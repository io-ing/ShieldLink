#include "AesCrypto.h"
#include <iostream>

AesCrypto::AesCrypto(string key)
{
	if (key.size() == 16 || key.size() == 24 || key.size() == 32)
	{
		const unsigned char* aesKey = (const unsigned char*)key.data();
		AES_set_encrypt_key(aesKey, key.size() * 8, &m_encKey);
		AES_set_decrypt_key(aesKey, key.size() * 8, &m_decKey);
		m_key = key;
	}
}

AesCrypto::~AesCrypto()
{
}

string AesCrypto::aesCBCEncrypt(string text)
{
	//return string();
	return aesCrypto(text, AES_ENCRYPT);
}

string AesCrypto::aesCBCDecrypt(string encStr)
{
	return aesCrypto(encStr, AES_DECRYPT);
}

string AesCrypto::aesCrypto(string data, int crypto)
{

	AES_KEY* key = crypto == AES_ENCRYPT ? &m_encKey : &m_decKey;

	unsigned char ivec[AES_BLOCK_SIZE];
	int length = data.size() + 1;	// +1 是添加\0的长度

	if (length % 16)
	{
		length = (length / 16 + 1) * 16;
	}
	char* out = new char[length];
	generateIvec(ivec);

	AES_cbc_encrypt((const unsigned char*)data.c_str(), 
		(unsigned char*)out, length, key, ivec, crypto);

	string retStr = string(out);
	delete[]out;
	return string(retStr);
}

void AesCrypto::generateIvec(unsigned char* ivec)
{
	for (int i = 0; i < AES_BLOCK_SIZE; ++i)
	{
		ivec[i] = m_key.at(AES_BLOCK_SIZE - i - 1);
	}
}
