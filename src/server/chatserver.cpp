#include "chatserver.hpp"
#include "chatservice.hpp"
#include "json.hpp"
#include "friendmodel.hpp"

#include <functional>
#include <string>

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : server_(loop, listenAddr, nameArg), loop_(loop)
{
    // 注册用户连接的创建和断开回调
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    // 注册用户读写事件的消息的回调
    server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置服务器端的线程数量，1个IO线程，n-1个worker线程
    server_.setThreadNum(4);
}

void ChatServer::start()
{
    server_.start();
}

// 上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // 如果连接失败，或者客户端断开链接
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        // 释放socket连接资源
        conn->shutdown();
    }
}

// 上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    // 数据的反序列化
    json js = json::parse(buf);

    // 达到目的：完全解耦网络模块和业务模块的代码
    // 通过js["msgid"]获取到
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    // 回调消息绑定好的事件处理函数，来执行相应的业务处理
    msgHandler(conn, js, time);
}