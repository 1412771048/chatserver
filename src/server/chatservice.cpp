#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>
#include <iostream>
using namespace std;
using namespace muduo;

// 获取单例对象的接口函数
ChatService* ChatService::instance() {
    static ChatService service;
    return &service;
}

//私有构造函数(保证单例)：为每一个msgid绑定一个回调
ChatService::ChatService() {
    // 用户基本业务管理相关事件处理回调注册
    _msgHandlerMap[LOGIN_MSG] = bind(&ChatService::login, this, _1, _2, _3);
    _msgHandlerMap[LOGINOUT_MSG] = bind(&ChatService::loginout, this, _1, _2, _3);
    _msgHandlerMap[REG_MSG] = bind(&ChatService::reg, this, _1, _2, _3);
    _msgHandlerMap[ONE_CHAT_MSG] = bind(&ChatService::oneChat, this, _1, _2, _3);
    _msgHandlerMap[ADD_FRIEND_MSG] = bind(&ChatService::addFriend, this,  _1, _2, _3);
    // 群组业务管理相关事件处理回调注册
    _msgHandlerMap[CREATE_GROUP_MSG] = bind(&ChatService::createGroup, this, _1, _2, _3);
    _msgHandlerMap[ADD_GROUP_MSG] = bind(&ChatService::addGroup, this, _1, _2, _3);
    _msgHandlerMap[GROUP_CHAT_MSG] = bind(&ChatService::groupChat, this, _1, _2, _3);
    //连接redis服务器
    if (_redis.connect()) {//私有成员，类外不能直接访问，所以才设置的public接口
        _redis.init_notify_handler(bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2)); //设置消息回调
    }
}

//都是调数据库操作类接口
void ChatService::reset() {
    // 把online状态的用户，设置成offline
    _userModel.resetState();
}

//客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn) {
    User user;
    {   //删除用户连接
        unique_lock<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it) {//可以删除迭代器/键，都能删掉这个元素
            if (it->second == conn) {
                user.setid(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    //用户state改为offline
    user.setstate("offline");
    _userModel.update(user);
}

// 获取消息对应的回调
MsgHandler ChatService::getHandler(int msgid) {
    if (_msgHandlerMap.find(msgid) == _msgHandlerMap.end()) {
        //没找到返回一个默认处理器，空操作
        return [=](const TcpConnectionPtr&, json&, Timestamp) {
            LOG_ERROR << "msgid:" << msgid << "can not find!";
        };
    } else {
        return _msgHandlerMap[msgid];
    }
}

// 登录业务:通过id查表返回user对象，看pwd是否等于pwd
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int id = js["id"];
    string pwd = js["password"];
    json response;
    response["msgid"] = LOGIN_MSG_ACK;

    User user = _userModel.query(id);
    //id!=-1说明有这个用户，且密码正确，且不在线，我才让你登录
    if (user.getid() != -1 && user.getpwd() == pwd && user.getstate() == "offline") {
        response["errno"] = 0;
        response["id"] = user.getid();
        response["name"] = user.getname();
        //更新用户state
        user.setstate("online");
        _userModel.update(user);

        //保存用户连接,加个括号即作用域，出括号自动解锁
        {
            unique_lock<mutex> lock(_connMutex);
            _userConnMap[id] = conn;
        }
        //登录成功，订阅自己的通道
        _redis.subscribe(id);
        //登录后查看离线消息
        vector<string> vec = _offlineMsgModel.query(id);
        if (!vec.empty()) {
            response["offlinemsg"] = vec; //json也可以序列化容器
            _offlineMsgModel.remove(id); //读取后删除离线消息
            vec.clear();
        }
        //用户登陆后给他返回好友列表
        vector<User> vec2 = _friendModel.query(id);
        if (!vec2.empty()) {
            for (User& user: vec2) {
                json js;
                js["id"] = user.getid();
                js["name"] = user.getname();
                js["state"] = user.getstate();
                vec.push_back(js.dump());
            }
            response["friend_lists"] = vec; 
        }
    } else {
        response["errno"] = 1;
        response["errmsg"] = "登录失败: 用户id不存在 or 密码错误 or 已在线";
    }
    conn->send(response.dump());
 
}

//一对一聊天
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int toid = js["to"]; //拿到对方id
    {
        unique_lock<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end()) {//连接在，说明在本台服务器上且在线
            it->second->send(js.dump());
            return;
        } 
    }
    //若连接不在，再查状态，在线即在其他服务器上
    if (_userModel.query(toid).getstate() == "online") {
        _redis.publish(toid, js.dump());
        return;
    }
    //不在线
    _offlineMsgModel.insert(toid, js.dump());
}

// 注册业务  name  password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    string name = js["name"];
    string pwd = js["password"];
    json response;
    response["msgid"] = REG_MSG_ACK;
    //设置好user表的信息，传给user表操作类
    User user; 
    user.setname(name);
    user.setpwd(pwd);
    //调用user表操作类方法
    bool state = _userModel.insert(user);
    if (state) { //注册成功
        response["errno"] = 0;
        response["id"] = user.getid();
    } else { // 注册失败
        response["errno"] = 1;
        response["errmsg"] = "用户名已存在";
    }
    conn->send(response.dump());
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    _friendModel.insert(js["id"], js["friendid"]);
}

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userid = js["id"];
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end()) {
            _userConnMap.erase(it);
        }
    }
    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid); 
    // // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _userModel.update(user);
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userid = js["id"];
    string name = js["groupname"], desc = js["groupdesc"];
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group)) { //调model层接口，传一个group对象进去
        _groupModel.addGroup(userid, group.getId(), "creator");//更新一下对象值
    }   
}
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"], groupid = js["groupid"];
    _groupModel.addGroup(userid, groupid, "normal");
}
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"], groupid = js["groupid"];
    vector<int> vec = _groupModel.queryGroupUsers(userid, groupid);    
    for (int id : vec) {
        //给每个id发一条消息，即与每个人单聊，直接抄
        {
            unique_lock<mutex> lock(_connMutex);
            auto it = _userConnMap.find(id);
            if (it != _userConnMap.end()) {//连接在，说明在本台服务器上且在线
                it->second->send(js.dump());
                return;
            } 
        }
        //若连接不在，再查状态，在线即在其他服务器上
        if (_userModel.query(id).getstate() == "online") {
            _redis.publish(id, js.dump());
            return;
        }
        //不在线
        _offlineMsgModel.insert(id, js.dump());
    }
}


//处理redis的消息：若在线就发给订阅者，不在线就发到它们的消息表
//但这里每个用户只需要订阅自己的通道(我们把通道与id设置一样号操作)，即可实现跨服通信
void ChatService::handleRedisSubscribeMessage(int userid, string msg) {
    unique_lock<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end()) {
        it->second->send(msg);
        return;
    }
    // 存储该用户的离线消息
    _offlineMsgModel.insert(userid, msg);
}



