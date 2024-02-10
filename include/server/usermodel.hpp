#ifndef USERMODEL_H
#define USERMODEL_H
#include "user.hpp"

//user表的操作类
class UserModel {
public:
    bool insert(User& user); //user表的增加
    User query(int id); //根据主键id返回user对象，不要返回引用
    bool update(User& user); //更新用户state
    void resetState();
};

#endif