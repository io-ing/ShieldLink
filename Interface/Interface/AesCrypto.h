#pragma once
#include <string>
#include <openssl/aes.h>
using namespace std;

class AesCrypto
{
public:
	// 可使用 16byte, 24byte, 32byte 的秘钥
	AesCrypto(string key);
	~AesCrypto();
	// 加密
	string aesCBCEncrypt(string text);
	// 解密
	string aesCBCDecrypt(string encStr);

private:
	string aesCrypto(string data, int crypto);
	void generateIvec(unsigned char* ivec);

private:
	AES_KEY m_encKey;
	AES_KEY m_decKey;
	string m_key;	// 秘钥
};

