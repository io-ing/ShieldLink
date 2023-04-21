#pragma once
#include "TcpSocket.h"
// 超时的时间

class TcpServer
{
public:
	TcpServer();
	~TcpServer();

	// 服务器设置监听
	int setListen(unsigned short port);
	// 等待并接受客户端连接请求, 默认连接超时时间为10000s
	TcpSocket* acceptConn(int timeout = 10000);
	void closefd();

private:
	int m_lfd;	// 用于监听的文件描述符
};

