#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>
#include <iostream>
using namespace std;
using namespace muduo;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的handler回调操作，也就是初始化msgHandlerMap_
ChatService::ChatService()
{
    msgHandlerMap_.insert({LOGIN_MSG, bind(&ChatService::login, this, _1, _2, _3)});
    msgHandlerMap_.insert({REG_MSG, bind(&ChatService::reg, this, _1, _2, _3)});
    msgHandlerMap_.insert({ONE_CHAT_MSG, bind(&ChatService::onechat, this, _1, _2, _3)});
    msgHandlerMap_.insert({ADD_FRIEND_MSG, bind(&ChatService::addFriend, this, _1, _2, _3)});
    msgHandlerMap_.insert({LOGINOUT_MSG, bind(&ChatService::loginout, this, _1, _2, _3)});

    msgHandlerMap_.insert({CREATE_GROUP_MSG, bind(&ChatService::createGroup, this, _1, _2, _3)});
    msgHandlerMap_.insert({ADD_GROUP_MSG, bind(&ChatService::addGroup, this, _1, _2, _3)});
    msgHandlerMap_.insert({GROUP_CHAT_MSG, bind(&ChatService::groupChat, this, _1, _2, _3)});

    // 连接redis服务器
    if (redis_.connect())
    {
        // 设置上报消息的回调
        redis_.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

// 处理登录业务，登录业务就是检查用户id和对应的密码password是否能正确对应上
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>(); // 默认是字符串类型，必须用get<>转换成整型
    string pwd = js["password"];

    User user = userModel_.query(id);
    LOG_INFO << "do login service";

    // id不为-1，这个用户存在，且密码正确，就登录成功
    if (user.getId() != -1 && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 如果登录的用户已经在线，则不允许重复登录，登录失败
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该用户已登录，不允许重复登录，请重新输入账号";
            conn->send(response.dump());
        }
        else
        {
            { // 登陆成功，记录用户连接信息，保证线程安全
                lock_guard<mutex> lock(connMutex_);
                userConnMap_.insert({id, conn});
            }

            // id用户登录成功后，向redis订阅channel(id)
            redis_.subscribe(id);

            // 登陆成功，更新用户状态信息 state->offline->online
            user.setState("online");
            userModel_.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0; // 类似系统编程中，成功返回0，失败返回非0，这么一个错误码
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询用户是否有离线消息，有的话通过json给带过去
            vector<string> vec = offlineMsgModel_.query(id);
            if (!vec.empty())
            {
                // 不为空说明有离线消息，读取离线消息后删除该离线消息
                response["offlinemsg"] = vec;
                offlineMsgModel_.remove(id);
            }

            // 登陆成功，查询该用户的好友信息并返回
            vector<User> uservec = friendModel_.query(id);
            if (!uservec.empty())
            {
                vector<string> vec2;
                for (User &user : uservec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
            conn->send(response.dump());
        }
    }
    else
    {
        // 否则就登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误";
        conn->send(response.dump());
    }
}

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(connMutex_);
        auto it = userConnMap_.find(userid);
        if (it != userConnMap_.end())
            userConnMap_.erase(it);
    }
    // 用户注销，相当于就是下线，在redis中取消订阅通道
    redis_.unsubscribe(userid);

    // 更新用户的状态
    User user(userid, "", "", "");
    userModel_.updateState(user);
}

// 处理注册业务，注册只需要用户名和密码
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // LOG_INFO<<"do reg service";
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);

    bool state = userModel_.insert(user);
    if (state)
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0; // 类似系统编程中，成功返回0，失败返回非0，这么一个错误码
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，表示msgid这个消息id没有对应的处理回调函数
    auto it = msgHandlerMap_.find(msgid);
    if (it == msgHandlerMap_.end())
    {
        // 返回一个默认的空处理回调函数，什么都不做，并使用muduo库的输出错误日志
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << "can't find handler!";
        };
    }
    else
        return msgHandlerMap_[msgid];
}

// 处理客户端异常退出
// 通过conn去userConnMap_中查找对应用户的id，查找到后要做两件事
// 把该用户的链接信息从userConnMap_中删除，把该用户在数据库中的state修改成offline
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        // 涉及到对userConnMap_的查找操作，要注意线程安全问题，所以第一步就是要加锁
        // 遍历userConnMap_，删除对应的链接信息的键值对
        lock_guard<mutex> lock(connMutex_);
        for (auto it = userConnMap_.begin(); it != userConnMap_.end(); ++it)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                userConnMap_.erase(it);
                break;
            }
        }
    }
    // 异常退出也是下线，在redis中取消订阅通道
    redis_.unsubscribe(user.getId());

    // 更新用户的state
    // 确保是一个存在的用户，如果是一个不存在的用户，就不用向数据库发送请求了
    if (user.getId() != -1)
    {
        user.setState("offline");
        userModel_.updateState(user);
    }
}

void ChatService::onechat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["to"].get<int>(); // 发送给目标用户的id

    {
        // 先在当前服务器上的userConnMap_中查找是否有目标用户的连接信息
        lock_guard<mutex> lock(connMutex_);
        auto it = userConnMap_.find(toid);

        if (it != userConnMap_.end())
        {
            // 查找成功，用户在线，服务器转发消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    // 本地userConnMap_如果没有toid用户信息，则查询toid是否在线
    // 在线说明toid用户连接到了另一台服务器，则进行跨服务器通信，向通道发布消息
    User user = userModel_.query(toid);
    if (user.getState() == "online")
    {
        redis_.publish(toid, js.dump());
        return;
    }
    // 查找失败，用户不在线，存储离线消息
    offlineMsgModel_.insert(toid, js.dump());
}

void ChatService::reset()
{
    // 把online状态的用户，设置成offline
    userModel_.resetState();
}

// msgid id:要添加好友的用户id friendid：被加为好友的用户id
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    friendModel_.insert(userid, friendid);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if (groupModel_.createGroup(group))
    {
        // 存储群组创建人信息
        groupModel_.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    groupModel_.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = groupModel_.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(connMutex_);
    for (int id : useridVec)
    {
        auto it = userConnMap_.find(id);
        if (it != userConnMap_.end())
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            User user = userModel_.query(id);
            if (user.getState() == "online")
            {
                redis_.publish(id, js.dump());
            }
            // 存储离线群消息
            else
            {
                offlineMsgModel_.insert(id, js.dump());
            }
        }
    }
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(connMutex_);
    auto it = userConnMap_.find(userid);
    if (it != userConnMap_.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    offlineMsgModel_.insert(userid, msg);
}