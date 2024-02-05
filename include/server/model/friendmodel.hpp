#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include"user.hpp"
#include<vector>
using namespace std;

//维护好友信息的操作接口方法
class FriendModel
{
public:
    //添加好友
    void insert(int userid,int friendid);

    //返回用户好友列表，做一个user表和friend表的联合查询，通过userid查询该userid拥有的好友的friendid
    //再根据这个friendid去到friendid对应的user表中查询其详细信息
    vector<User>query(int userid);
};
#endif