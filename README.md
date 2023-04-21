ServerSeckey服务端，配置文件为server.json和.sln文件放在一块
ClientSecKey客户端，配置文件为client.json和.sln文件放在一块
Interface内联接口
socket-test主程序，业务代码，配置文件为tcpServer.json\tcpClient.json和.sln文件放在一块

oracle-centos.sql为数据库文件




（二）项目工程介绍
What is xxx?

安全传输平台是一个 C++ 编写的程序，服务端和客户端可以进行不同等级的密钥协商，记录密钥信息到 Oracle 数据库，为其他程序通信提供外联接口进行加密通信。

此 README 文件包含 aaa 的编译说明，要想成功编译程序，需要安装 protobuf、openSSL 和 JSON。

（四）项目特点
- 使用openSSL进行加密，支持多种级别的加密
- 简单易用、结构清晰

部署
开始部署之前，确保你的电脑上安装了 protobuf、openSSL 和 JSON。

在客户端和服务端源码目录中使用下面的命令编译客户端和服务端
```
# 编译服务端
g++ *.cpp *.cc -ljson -lprotobuf -lcrypto -std=c++11

# 编译客户端
g++ *.cpp *.cc -ljson -lprotobuf -lcrypto -std=c++11

# 编译外联接口
g++ -c *.cpp -fpic -std=c++11
# 生成动态库
g++ -shared *.o -o libinterface.so
# 配置动态库环境
cp libInterface.so /usr/lib
echo export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib >> ∼/.bashrc
. ~/.bashrc
```

（十一）鸣谢
protobuf、openssl

（十二）版权信息
Mozilla

