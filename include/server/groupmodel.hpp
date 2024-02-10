#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include "group.hpp"
#include <string>
#include <vector>
using namespace std;

// 群组业务的接口
class GroupModel {
public:
    bool createGroup(Group &group);                       // 创建群组
    void addGroup(int userid, int groupid, string role);  // 加入群组
    vector<Group> queryGroups(int userid);                // 查询用户所在群组信息
    vector<int> queryGroupUsers(int userid, int groupid); // 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
};

#endif