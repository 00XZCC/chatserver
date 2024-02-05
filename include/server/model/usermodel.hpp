#ifndef USERMODEL_H
#define USERMODEL_H

#include"user.hpp"

//这个类是直接去操作user表的，这个类的方法内部封装sql语句去操作数据库中的表
class UserModel
{
public:
    //User表的增加方法
    bool insert(User &user);

    //根据用户号码查询用户信息
    //返回值为指针类型是为了可以用空指针表示没有查到
    User query(int id);

    //更新用户的状态信息
    bool updateState(User user); 

    //重置用户的state信息
    void resetState();
};
#endif