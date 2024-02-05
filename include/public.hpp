// server和client的公共文件

#ifndef PUBLIC_H
#define PUBLIC_H

// 消息id
enum EnMsgType
{
    LOGIN_MSG = 1,    // 登录消息id 1
    LOGIN_MSG_ACK,    // 登录响应消息 2
    LOGINOUT_MSG,     // 注销消息 3
    REG_MSG,          // 注册消息id，由客户端发送给客户端 4
    REG_MSG_ACK,      // 注册响应消息，由服务端响应给客户端 5
    ONE_CHAT_MSG,     // 聊天消息 6
    ADD_FRIEND_MSG,   // 添加好友消息 7
    CREATE_GROUP_MSG, // 创建群组 8
    ADD_GROUP_MSG,    // 加入群组 9
    GROUP_CHAT_MSG    // 群聊天 10
};

#endif