#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"
#include <string>
#include <vector>
using namespace std;

// group表的ORM类，与user.hpp同理
class Group {
public:
    Group(int id = -1, string name = "", string desc = ""): id(id), name(name), desc(desc) {}
    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setDesc(string desc) { this->desc = desc; }

    int getId() { return this->id; }
    string getName() { return this->name; }
    string getDesc() { return this->desc; }
    vector<GroupUser> &getUsers() { return this->users; } //返回组员数组

private:
    int id;
    string name;
    string desc;
    vector<GroupUser> users;
};

#endif