#include "redis.hpp"
#include <iostream>
using namespace std;

Redis::Redis(): _publish_context(nullptr), _subcribe_context(nullptr) {}

Redis::~Redis() {
    if (_publish_context) {
        redisFree(_publish_context);
    }
    if (_subcribe_context) {
        redisFree(_subcribe_context);
    }
}

bool Redis::connect() {
    // 负责publish发布消息的上下文连接
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (!_publish_context) {
        cerr << "connect redis failed!" << endl;
        return false;
    }
    // 负责subscribe订阅消息的上下文连接
    _subcribe_context = redisConnect("127.0.0.1", 6379);
    if (!_subcribe_context) {
        cerr << "connect redis failed!" << endl;
        return false;
    }
    // 在单独的线程中，监听通道上的事件，有消息给业务层进行上报
    thread t([&]() {observer_channel_message();}); //lambda表达式
    t.detach();
    cout << "connect redis-server success!" << endl;
    return true;
}

// 向指定通道发布消息
bool Redis::publish(int channel, string message) {
    redisReply* reply = (redisReply*)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (!reply) {
        cerr << "publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 向redis指定的通道subscribe订阅消息
bool Redis::subscribe(int channel) {
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道，不接收通道消息
    // 通道消息的接收专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送命令，不阻塞接收redis server响应消息，由独立线程来回调处理
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "SUBSCRIBE %d", channel)) {
        cerr << "subscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done)) {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel) {
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "UNSUBSCRIBE %d", channel)) {
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done)) {
            cerr << "unsubscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

// 在独立线程中阻塞等待订阅的消息
void Redis::observer_channel_message() {
    redisReply* reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subcribe_context, (void**)&reply)) {
        // 订阅收到的消息是一个带三元素的数组：["message","通道号", "消息内容"]
        if (reply && reply->element[2] && reply->element[2]->str) {
            _notify_message_handler(atoi(reply->element[1]->str) , reply->element[2]->str); //有人publish，执行回调通知业务层
        }
        freeReplyObject(reply);
    }
    cout << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}

void Redis::init_notify_handler(function<void(int,string)> fn) {
    this->_notify_message_handler = fn;
}