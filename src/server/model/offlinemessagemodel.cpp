#include "db.h"
#include "offlinemessagemodel.hpp"

// 存储用户的离线消息
void offlineMsgModel::insert(int userid, string msg)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values('%d','%s')", userid, msg.c_str());

    // 创建MySql对象，链接到数据库
    MySQL mysql;
    if (mysql.connect())
        mysql.update(sql);
}
// 删除用户的离线消息，用户上线之后就可以把离线消息转发给用户后就删除掉
void offlineMsgModel::remove(int userid)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = %d", userid);

    // 创建MySql对象，链接到数据库
    MySQL mysql;
    if (mysql.connect())
        mysql.update(sql);
}
// 查询用户的离线消息
vector<string> offlineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userid);

    // 把userid用户的所有离线消息放入vec中返回
    vector<string> vec;

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            while ((row = mysql_fetch_row(res)) != nullptr)
                vec.push_back(row[0]);
            mysql_free_result(res);
            return vec;
        }
    }
    return vec; // 失败也返回一个空的vec
}
