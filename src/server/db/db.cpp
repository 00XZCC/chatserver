#include "db.h"
#include <muduo/base/Logging.h>
#include<iostream>

// 数据库配置信息
static string server = "127.0.0.1";//这里要填数据库运行的地址，不能填服务器的ip地址，就是netstat -tanp里显示的地址，这个项目里的2003错误就是因为这个产生的
static string user = "root";
static string password = "123456";
static string dbname = "chat";

// 初始化数据库连接
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}
// 释放数据库连接资源
MySQL::~MySQL()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}
// 连接数据库
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                  password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        // c和C++代码默认的编码字符是ASCII，如果不设置，从MySQL上拉下来的中文显示?
        mysql_query(_conn, "set names gbk");
        LOG_INFO << "connect mysql success!";
    }
    else
    {
        LOG_INFO << "connect mysql fail!";
        std::cout<<mysql_errno(_conn)<<endl;
    }
    return p;
}
// 更新操作
bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << "更新失败!";
        return false;
    }
    return true;
}
// 查询操作
MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << "查询失败!";
        cout<<mysql_errno(_conn)<<endl;
        return nullptr;
    }
    return mysql_use_result(_conn);
}

// 获取链接
MYSQL *MySQL::getConnection()
{
    return _conn;
}