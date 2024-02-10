#ifndef OFFLINEMSGMODEL_H
#define OFFLINEMSGMODEL_H
#include <string>
using namespace std;
#include <vector>


//操作离线消息表
class OfflineMsgModel {
public:
    bool insert(int userid, string msg); //存储用户离线消息
    bool remove(int userid); //删除用户离线消息
    vector<string> query(int userid); //查询用户消息,不能返回局部变量的引用
};

#endif