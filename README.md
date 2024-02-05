#项目介绍
  基于moduo库实现的，可以工作在nginx tcp负载均衡环境中的集群聊天服务器和客户端源码，基于redis实现客户端跨服务器通信


#环境依赖
  Json序列化和反序列化
  muduo网络库开发
  nginx源码编译安装和环境部署
  nginx的tcp负载均衡器配置
  中间件redis消息队列编程
  MySQL数据库
  CMake构建编译环境


#目录结构
  bin：存放编译后的客户端和服务端可执行文件
  build：存放编译过程中产生的中间文件
  include：存放项目所需头文件
  src：存放项目源代码
  thirdparty：存放项目所需第三方文件

