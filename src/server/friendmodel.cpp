#include "friendmodel.hpp"
#include "mysql.hpp"

void FriendModel::insert(int userid, int friendid) {
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d, %d)", userid, friendid);
    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
        sprintf(sql, "insert into friend values(%d, %d)",friendid, userid);
        mysql.update(sql);
    }
}

vector<User> FriendModel::query(int userid) {
    char sql[1024] = {0};
    //friend和uer表联合查询
    sprintf(sql, "select a.id,a.name,a.state from user a join friend b on a.id = b.friendid where b.userid = %d", userid);
    MySQL mysql;
    vector<User> vec;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res) {//若查询结果不空，则一行一行放入数组
            MYSQL_ROW row;
            while (row = mysql_fetch_row(res)) {
                vec.push_back(User(atoi(row[0]), row[1], "", row[2]));
            }
        }
        mysql_free_result(res);
    }
    return vec;
}