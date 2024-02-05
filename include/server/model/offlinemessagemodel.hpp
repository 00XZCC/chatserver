#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>
using namespace std;

// 提供离线消息表的操作接口方法
class offlineMsgModel
{
public:
    // 存储用户的离线消息
    void insert(int userid, string msg);
    // 删除用户的离线消息，用户上线之后就可以把离线消息转发给用户后就删除掉
    void remove(int userid);
    // 查询用户的离线消息
    vector<string> query(int userid);
};
#endif