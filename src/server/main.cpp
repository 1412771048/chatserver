#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;

// 服务器异常退出(ctrl+c)，重置用户状态为offline
void resetHandler(int) {
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        cout << "failed! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }
    char* ip = argv[1];
    unsigned short port = atoi(argv[2]);

    signal(SIGINT, resetHandler); //监听SIGINT(ctrl+c)，后续即可回调处理
    EventLoop loop; 
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");
    server.start(); //开启服务器
    loop.loop(); //开启事件循环


}