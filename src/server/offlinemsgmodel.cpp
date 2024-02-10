#include "offlinemsgmodel.hpp"
#include "mysql.hpp"


 //与操作user表一样，但这里没有把offlinemsg表给变成类
bool OfflineMsgModel::insert(int userid, string msg) {
   char sql[1024] = {0};
   sprintf(sql, "insert into offlinemessage(userid, message) values('%d','%s')", userid, msg.c_str());
   MySQL mysql;
   return mysql.connect() && mysql.update(sql);
}

bool OfflineMsgModel::remove(int userid) {
   char sql[1024] = {0};
   sprintf(sql, "delete from offlinemessage where userid=%d", userid);
   MySQL mysql;
   return mysql.connect() && mysql.update(sql);
}

vector<string> OfflineMsgModel::query(int userid) {
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid=%d", userid);
    vector<string> vec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res) {
            MYSQL_ROW row; 
            while ((row = mysql_fetch_row(res))) {//把查询结果一行一行的赋给row
                vec.push_back(row[0]); //把第一个字段message放入vec
            }
        }
        mysql_free_result(res);
    }
    return vec;
}




