#include "RsaCrypto.h"
#include <openssl/bio.h>
#include <openssl/err.h>
#include <iostream>
#include <openssl/buffer.h>
#include <string.h>

RsaCrypto::RsaCrypto()
{
	m_publicKey = RSA_new();
	m_privateKey = RSA_new();
}

RsaCrypto::RsaCrypto(string fileName, bool isPrivate)
{
	m_publicKey = RSA_new();
	m_privateKey = RSA_new();
	if (isPrivate)
	{
		initPrivateKey(fileName);
	}
	else
	{
		initPublicKey(fileName);
	}
}


RsaCrypto::~RsaCrypto()
{
	RSA_free(m_publicKey);
	RSA_free(m_privateKey);
}

// 将公钥/私钥字符串数据解析到 RSA 对象中
void RsaCrypto::parseKeyString(string keystr, bool pubKey)
{
	// 字符串数据 -> BIO对象中
	BIO* bio = BIO_new_mem_buf(keystr.data(), keystr.size());
	// 公钥字符串
	if (pubKey)
	{
		PEM_read_bio_RSAPublicKey(bio, &m_publicKey, NULL, NULL);
	}
	else
	{
		// 私钥字符串
		PEM_read_bio_RSAPrivateKey(bio, &m_privateKey, NULL, NULL);
	}
	BIO_free(bio);
}

void RsaCrypto::generateRsakey(int bits, string pub, string pri)
{
	RSA* r = RSA_new();
	// 生成RSA密钥对
	// 创建bignum对象
	BIGNUM* e = BN_new();
	// 初始化bignum对象
	BN_set_word(e, 456787);
	RSA_generate_key_ex(r, bits, e, NULL);

	// 创建bio文件对象
	BIO* pubIO = BIO_new_file(pub.data(), "w");
	// 公钥以pem格式写入到文件中
	PEM_write_bio_RSAPublicKey(pubIO, r);
	// 缓存中的数据刷到文件中
	BIO_flush(pubIO);
	BIO_free(pubIO);

	// 创建bio对象
	BIO* priBio = BIO_new_file(pri.data(), "w");
	// 私钥以pem格式写入文件中
	PEM_write_bio_RSAPrivateKey(priBio, r, NULL, NULL, 0, NULL, NULL);
	BIO_flush(priBio);
	BIO_free(priBio);

	// 得到公钥和私钥
	m_privateKey = RSAPrivateKey_dup(r);
	m_publicKey = RSAPublicKey_dup(r);

	// 释放资源
	BN_free(e);
	RSA_free(r);
}

bool RsaCrypto::initPublicKey(string pubfile)
{
	// 通过BIO读文件
	BIO* pubBio = BIO_new_file(pubfile.data(), "r");
	// 将bio中的pem数据读出
	if (PEM_read_bio_RSAPublicKey(pubBio, &m_publicKey, NULL, NULL) == NULL)
	{
		ERR_print_errors_fp(stdout);
		return false;
	}
	BIO_free(pubBio);
	return true;
}

bool RsaCrypto::initPrivateKey(string prifile)
{
	// 通过bio读文件
	BIO* priBio = BIO_new_file(prifile.data(), "r");
	// 将bio中的pem数据读出
	if (PEM_read_bio_RSAPrivateKey(priBio, &m_privateKey, NULL, NULL) == NULL)
	{
		ERR_print_errors_fp(stdout);
		return false;
	}
	BIO_free(priBio);
	return true;
}


string RsaCrypto::rsaPubKeyEncrypt(string data)
{
	cout << "加密数据长度: " << data.size() << endl;
	// 计算公钥长度
	int keyLen = RSA_size(m_publicKey);
	cout << "pubKey len: " << keyLen << endl;
	// 申请内存空间
	char* encode = new char[keyLen + 1];
	// 使用公钥加密
	int ret = RSA_public_encrypt(data.size(), (const unsigned char*)data.data(),
		(unsigned char*)encode, m_publicKey, RSA_PKCS1_PADDING);
	string retStr = string();
	if (ret >= 0)
	{
		// 加密成功
		cout << "ret: " << ret << ", keyLen: " << keyLen << endl;
		retStr = toBase64(encode, ret);
	}
	else
	{
		ERR_print_errors_fp(stdout);
	}
	// 释放资源
	delete[]encode;
	return retStr;
}

string RsaCrypto::rsaPriKeyDecrypt(string encData)
{
	// text指向的内存需要释放
	char* text = fromBase64(encData);
	// 计算私钥长度
	//cout << "解密数据长度: " << text.size() << endl;
	int keyLen = RSA_size(m_privateKey);
	// 使用私钥解密
	char* decode = new char[keyLen + 1];
	// 数据加密完成之后, 密文长度 == 秘钥长度
	int ret = RSA_private_decrypt(keyLen, (const unsigned char*)text,
		(unsigned char*)decode, m_privateKey, RSA_PKCS1_PADDING);
	string retStr = string();
	if (ret >= 0)
	{
		retStr = string(decode, ret);
	}
	else
	{
		cout << "私钥解密失败..." << endl;
		ERR_print_errors_fp(stdout);
	}
	delete[]decode;
	delete[]text;
	return retStr;
}

string RsaCrypto::rsaSign(string data, SignLevel level)
{
	unsigned int len;
	char* signBuf = new char[1024];
	memset(signBuf, 0, 1024);
	int ret = RSA_sign(level, (const unsigned char*)data.data(), data.size(), (unsigned char*)signBuf,
		&len, m_privateKey);
	if (ret == -1)
	{
		ERR_print_errors_fp(stdout);
	}
	cout << "sign len: " << len << ", ret: " << ret << endl;
	string retStr = toBase64(signBuf, len);
	delete[]signBuf;
	return retStr;
}

bool RsaCrypto::rsaVerify(string data, string signData, SignLevel level)
{
	// 验证签名
	int keyLen = RSA_size(m_publicKey);
	char* sign = fromBase64(signData);
	int ret = RSA_verify(level, (const unsigned char*)data.data(), data.size(),
		(const unsigned char*)sign, keyLen, m_publicKey);
	delete[]sign;
	if (ret == -1)
	{
		ERR_print_errors_fp(stdout);
	}
	if (ret != 1)
	{
		return false;
	}
	return true;
}
string RsaCrypto::toBase64(const char* str, int len)
{
	BIO* mem = BIO_new(BIO_s_mem());
	BIO* bs64 = BIO_new(BIO_f_base64());
	// mem添加到bs64中
	bs64 = BIO_push(bs64, mem);
	// 写数据
	BIO_write(bs64, str, len);
	BIO_flush(bs64);
	// 得到内存对象指针
	BUF_MEM *memPtr;
	BIO_get_mem_ptr(bs64, &memPtr);
	string retStr = string(memPtr->data, memPtr->length - 1);
	BIO_free_all(bs64);
	return retStr;
}

char* RsaCrypto::fromBase64(string str)
{
	int length = str.size();
	BIO* bs64 = BIO_new(BIO_f_base64());
	BIO* mem = BIO_new_mem_buf(str.data(), length);
	BIO_push(bs64, mem);
	char* buffer = new char[length];
	memset(buffer, 0, length);
	BIO_read(bs64, buffer, length);
	BIO_free_all(bs64);

	return buffer;
}
