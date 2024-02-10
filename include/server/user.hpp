#ifndef USER_H
#define USER_H
#include <string>
using namespace std;

//User表的ORM类，操作表变成操作对象
class User {
public:
    //构造函数若不赋值就按默认值
    User(int id=-1, string name="", string pwd="", string state="offline"): id(id), name(name), pwd(pwd), state(state) {}
    void setid(int id) {this->id = id;}
    void setname(string name) {this->name = name;}
    void setpwd(string pwd) {this->pwd = pwd;}
    void setstate(string state) {this->state = state;}
    int getid() {return this->id;}
    string getname() {return this->name;}
    string getpwd() {return this->pwd;}
    string getstate() {return this->state;}
private:
    int id;
    string name, pwd, state;
};

#endif