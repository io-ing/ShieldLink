#pragma once
#include <string>
#include "SecKeyShm.h"
using namespace std;

struct ClientInfo
{
	string ServerID;
	string ClientID;
	string ip;
	unsigned short port;
};

class ClientOP
{
public:
	ClientOP(string jsonFile);
	~ClientOP();

	// 秘钥协商
	bool seckeyAgree();

	// 秘钥校验
	void seckeyCheck();

	// 秘钥注销
	void seckeyLogoff();

private:
	ClientInfo m_info;
	SecKeyShm* m_shm;
};

