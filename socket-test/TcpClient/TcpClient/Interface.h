#pragma once
#include <string>
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
	string m_key;	// 秘钥
};

