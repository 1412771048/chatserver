#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <functional>
#include <string>
using namespace std;
using namespace placeholders;
using json = nlohmann::json; //创建别名，与typedef类似

// 初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop) {
    // 注册连接回调，onConnection,onMessage是我们类的2个方法，把他们注册回调
    _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));
    // 注册消息回调
    _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));
    // 设置线程数量，一个主reactor负责用户的连接(onConnection)，3个子reactor负责处理事件(onMessage)
    _server.setThreadNum(4); 
}

// 启动服务
void ChatServer::start() {
    _server.start();
}

//连接回调的具体实现
void ChatServer::onConnection(const TcpConnectionPtr& conn) {
    // 客户端断开链接
    if (!conn->connected()) {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

//消息回调的具体实现
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time) { 
    json js = json::parse(buffer->retrieveAllAsString()); //反序列化拿到数据
    //获取业务类单例,调gethandler：根据msgid查map表得到对应的回调对象
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    //执行回调(一个接口，多种行为)
    msgHandler(conn, js, time);
}