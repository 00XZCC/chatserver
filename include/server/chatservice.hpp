#ifndef CHATSERVICE_HPP
#define CHATSERVICE_HPP

#include "json.hpp"
#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include"redis.hpp"

using namespace std;
using namespace muduo::net;
using namespace muduo;

using json = nlohmann::json;
// 表示处理消息的事件回调方法类型
using MsgHandler = function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

// 聊天服务器业务类，通过单例模式来实现
class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService *instance();
    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 一对一聊天业务
    void onechat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 获取消息id对应的处理函数
    MsgHandler getHandler(int msgid);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    // 服务器异常后，业务重置方法
    void reset();
    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 群组相关业务模块接口
    // 创建群组
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 加入群组
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 群组聊天
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    
    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, string);

private:
    ChatService();
    // 存储消息id和其对应的业务处理方法的映射表
    unordered_map<int, MsgHandler> msgHandlerMap_;

    // 存储在线用户的通信连接信息,不在线的用户不会存储在这里面，或者说下线的用户会被删除
    unordered_map<int, TcpConnectionPtr> userConnMap_;

    // 定义互斥锁，保证userConnMap_的线程安全
    mutex connMutex_;

    // 数据操作类对象
    UserModel userModel_;

    offlineMsgModel offlineMsgModel_;

    FriendModel friendModel_;

    GroupModel groupModel_;

    //redis操作类对象
    Redis redis_;
};

#endif