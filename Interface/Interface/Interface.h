#pragma once
#include <string>
#include "SecKeyShm.h"
using namespace std;
class Interface
{
public:
	Interface(string json);
	~Interface();

	// 数据加密
	string encryptData(string data);
	// 数据解密
	string decryptData(string data);

private:
	string m_key;		// 秘钥
	int m_secKeyID = 0;	// 共享内存中密钥 ID
	bool status = true;	// 密钥是否可用 1可用 0注销

	SecKeyShm* m_shm;
};

