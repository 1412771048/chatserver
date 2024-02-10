#include "groupmodel.hpp"
#include "mysql.hpp"

// 创建群组
bool GroupModel::createGroup(Group &group) {
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s', '%s')", group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect() && mysql.update(sql)) {
        group.setId(mysql_insert_id(mysql.getConnection()));//更新一个组id
        return true;
    }
    return false;
}

// 加入群组
void GroupModel::addGroup(int userid, int groupid, string role) {
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values(%d, %d, '%s')", groupid, userid, role.c_str());
    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}

// 查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userid) {
    //1. 先根据userid在groupuser表中查询出该用户所在的所有群
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from allgroup a join groupuser b on a.id = b.groupid where b.userid=%d", userid);
    vector<Group> groupVec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res) {
            MYSQL_ROW row;
            // 查出userid所有的群组信息
            while ((row = mysql_fetch_row(res))) {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
        }
        mysql_free_result(res);

        //2. 再遍历所有的groupid查询属于每个群组的所有用户信息(联合查询：user和groupuser)
        for (Group &group : groupVec) {
            sprintf(sql, "select a.id,a.name,a.state,b.grouprole from user a join groupuser b on b.userid = a.id where b.groupid=%d", group.getId());
            MYSQL_RES *res = mysql.query(sql);
            if (res) {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res))) {
                    GroupUser user;
                    user.setid(atoi(row[0]));
                    user.setname(row[1]);
                    user.setstate(row[2]);
                    user.setRole(row[3]);
                    group.getUsers().push_back(user);
                }
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}

//群聊：给群组其它所有成员发消息, 通过groupid和userid查到该组其他成员id
vector<int> GroupModel::queryGroupUsers(int userid, int groupid) {
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupid, userid);
    vector<int> idVec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                idVec.push_back(atoi(row[0]));
            }
        }
        mysql_free_result(res);
    }
    return idVec;
}