#include "friendmodel.hpp"
#include "db.h"

// 添加好友
void FriendModel::insert(int userid, int friendid)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d,%d)", userid, friendid);

    // 创建MySql对象，链接到数据库
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 返回用户好友列表，做一个user表和friend表的联合查询
vector<User> FriendModel:: query(int userid)
{   
     // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid=%d", userid);

    // 创建MySql对象，链接到数据库
    MySQL mysql;
    vector<User> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec; // 失败也返回一个空的vec
}