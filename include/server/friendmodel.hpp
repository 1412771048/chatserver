#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include <vector>
using namespace std;
#include "user.hpp"


//好友表操作类
class FriendModel {
public:
    void insert(int userid, int friendid); //添加好友
    vector<User> query(int userid);//返回好友列表
private:
};


#endif