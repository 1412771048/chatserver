#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;
#include "redis.hpp"
#include "groupmodel.hpp"
#include "friendmodel.hpp"
#include "usermodel.hpp"
#include "offlinemsgmodel.hpp"
#include "json.hpp"
using json = nlohmann::json;

//function函数返回一种函数的类型，实现了函数的多态，即为所有的消息处理函数提供一个统一的接口，
//网络层通过msgid得到对应的回调函数对象，执行即可，解耦了网络层业务层，后续业务层随便改，网络模块都不用动
using MsgHandler = function<void(const TcpConnectionPtr&, json&, Timestamp)>;

//业务类，单例
class ChatService {
public:
    static ChatService* instance();  // 获取单例对象的静态接口
    MsgHandler getHandler(int msgid); // 获取消息对应的回调,就是查map表
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time); //登录业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time); //注册业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time); // 一对一聊天业务
    void reset(); //服务器异常退出
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time); //添加好友业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time); // 创建群组
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time); // 加入群组业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time); // 群组聊天业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time); // 处理注销业务
    void clientCloseException(const TcpConnectionPtr &conn); // 处理客户端异常退出，因为客户端发一个错误的json会导致服务端退出
    void handleRedisSubscribeMessage(int, string); //从redis消息队列中获取订阅的消息
private:
    ChatService(); //私有构造函数，单例
    unordered_map<int, MsgHandler> _msgHandlerMap; // 存储msgid和其对应的回调
    unordered_map<int, TcpConnectionPtr> _userConnMap;//存储在线用户的连接，用于服务器主动推送消息给用户
    mutex _connMutex; //c++11的互斥锁，保证_userConnMap的线程安全

    // 数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    Redis _redis;
};

#endif