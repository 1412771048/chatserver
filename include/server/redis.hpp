#ifndef REDIS_H
#define REDIS_H
#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
using namespace std;
// redis作为集群服务器通信的基于发布-订阅消息队列时，会遇到两个难搞的bug问题，参考：https://blog.csdn.net/QIANGWEIYUAN/article/details/97895611

class Redis {
public:
    Redis();
    ~Redis();
    bool connect();                                           // 连接redis服务器
    bool publish(int channel, string message);                // 向指定的通道发布消息
    bool subscribe(int channel);                              // 向指定的通道subscribe订阅消息
    bool unsubscribe(int channel);                            // 向redis指定的通道unsubscribe取消订阅消息
    void observer_channel_message();                          // 在独立线程中接收订阅通道中的消息
    void init_notify_handler(function<void(int, string)> fn); // 初始化向业务层上报通道消息的回调对象
private:
    // 因为subscribe是阻塞的，所以publish和subscribe不能再一个上下文
    redisContext *_publish_context;                      // hiredis同步上下文对象，负责publish消息
    redisContext *_subcribe_context;                     // hiredis同步上下文对象，负责subscribe消息，
    function<void(int, string)> _notify_message_handler; // 回调操作，收到订阅的消息，给service层上报
};

#endif