#include "mysql.hpp"
#include "usermodel.hpp"
#include <iostream>

//User表的insert
bool UserModel::insert(User& user) {
    //组装sql,末尾不需要;
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')", 
    user.getname().c_str(), user.getpwd().c_str(), user.getstate().c_str());
    //连接数据库,执行sql
    MySQL mysql;
    if (mysql.connect() && mysql.update(sql)) {
            user.setid(mysql_insert_id(mysql.getConnection()));//反过来更新一下表对象的id
            return true;
    }
    return false;
}


User UserModel::query(int id) {
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id=%d", id);
    //连接数据库,执行sql
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) {
                User user(atoi(row[0]), row[1], row[2], row[3]);//id，name,pwd,state
                return user;
            }
        }
        mysql_free_result(res); //释放资源，空指针也可以释放
    }
    return User(); //返回id=-1
}

bool UserModel::update(User& user) {
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getstate().c_str(), user.getid());
    MySQL mysql;
    return mysql.connect() && mysql.update(sql);
}

void UserModel::resetState() {
    char sql[1024] = "update user set state='offline' where state='online'";
    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    } 
}