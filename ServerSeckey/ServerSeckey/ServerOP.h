#pragma once
#include <map>
#include "TcpServer.h"
#include "Message.pb.h"
#include "OCCIOP.h"
#include "SecKeyShm.h"

// 处理客户端请求
class ServerOP
{
public:
	enum KeyLen {Len16=16, Len24=24, Len32=32};
	ServerOP(string json);
	~ServerOP();

	// 启动服务器
	void startServer();

	// 线程工作函数，推荐使用
	static void* working(void* arg);
	// 不推荐使用，友元破坏了类的封装
	friend void* workHard(void* arg);

	// 秘钥协商
	string seckeyAgree(RequestMsg* reqMsg);

	// 密钥校验
	string seckeyCheck(RequestMsg* reqMsg);

	// 密钥注销
	string seckeyLogoff(RequestMsg* reqMsg);


private:
	string getRandKey(KeyLen len);

private:
	string m_serverID;	// 当前服务器的ID
	string m_dbUser;
	string m_dbPwd;
	string m_dbConnStr;
	unsigned short m_port;
	map<pthread_t, TcpSocket*> m_list;
	TcpServer *m_server = NULL;
	OCCIOP m_occi;		// 数据库实例对象
	SecKeyShm* m_shm;	// 共享内存实例对象
};

void* workHard(void* arg);

