#include"chatserver.hpp"
#include"chatservice.hpp"
#include<iostream>
#include<signal.h>
using namespace std;

//处理服务器ctrl+c异常终止后，重置user的state信息
//后续还要处理各种服务器的异常终止
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main()
{
    signal(SIGINT,resetHandler);

    EventLoop loop;
    InetAddress addr("192.168.0.111",6000);
    ChatServer server(&loop,addr,"chatserver");

    server.start();
    loop.loop();

    return 0;
}