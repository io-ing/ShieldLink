#include "Interface.h"
#include <json/json.h>
#include <fstream>
#include "SecKeyShm.h"
#include "AesCrypto.h"
#include <arpa/inet.h>
using namespace std;

Interface::Interface(string json)
{
	// 解析json文件
	// 1. 得到流对象 -> 读文件
	ifstream ifs(json);
	// 2. 创建json Reader 
	Json::Reader rd;
	// 3. 调用Reader对象 parse, 初始化一个Value对象
	Json::Value root;
	rd.parse(ifs, root);

	// 4. 对Value对象中的数据
	string shmkey = root["shmkey"].asString();
	string serverID = root["serverID"].asString();
	string clientID = root["clientID"].asString();
	int maxNode = root["maxNode"].asInt();

	cout << "shmkey="<< shmkey <<endl;
	cout << "serverID="<< serverID <<endl;
	cout << "clientID="<< clientID <<endl;
	cout << "maxNode="<< maxNode <<endl;

#if 0
	// 读共享内存
	m_shm = new SecKeyShm(shmkey, maxNode);
	// 得到秘钥
	//NodeSHMInfo node = shm.shmRead(clientID, serverID);
	NodeSHMInfo node = m_shm->shmFirstNode();

	m_key = string(node.seckey);
	m_secKeyID = node.seckeyID;
	cout << "m_key = " << m_key << endl;
	cout << "m_secKeyID = " << m_secKeyID << endl;
#else

	// 读共享内存
	SecKeyShm shm(shmkey, maxNode);
	// 得到秘钥
	NodeSHMInfo node = shm.shmRead(clientID, serverID);
	m_key = string(node.seckey);

	if (node.status == 0)
	{
		status = false;
	}
#endif // 0
}

Interface::~Interface()
{
}

// "hello,world"
string Interface::encryptData(string data)
{
#if 0
	AesCrypto aes(m_key);
	string ret = aes.aesCBCEncrypt(data);

	int header = m_secKeyID;
	cout << "header = " << header << endl;

	string result;
	result.append(reinterpret_cast<const char*>(&header), sizeof(header));
	result.append(ret);

	cout << "result = " << result << endl;
	return result;

#elif 0
	//cout << "data = " << data << endl;
	//AesCrypto aes(m_key);
	//string ret = aes.aesCBCEncrypt(data);
	//cout << "ret = " << ret << endl;

	//int dataLen = ret.size() + 4;
	//// 添加的4字节作为数据头, 存储数据块长度
	//char* netdata = (char*)malloc(dataLen);
	//if (netdata == NULL)
	//{
	//	printf("func sckClient_send() mlloc Err:\n");
	//	return "error";
	//}

	//int header = 6666;
	//memcpy(netdata, &header, 4);					// 加包头
	//cout << "netdata = " << netdata << endl;
	//memcpy(netdata + 4, ret.data(), ret.size());	// 加包体
	//cout << "netdata = " << netdata << endl;

	//string str = string(netdata);
	//cout << "encryptData str = " << str << endl;

	//return str;

	AesCrypto aes(m_key);
	string ret = aes.aesCBCEncrypt(data);

	int header = 6666;
	// 将int类型包头转换为网络字节序
	uint32_t network_header = htonl(header);
	cout << "header = " << header << endl;
	cout << "network_header = " << network_header << endl;

	// 将包头转换为char类型的字符串
	char header_str[sizeof(network_header)];
	cout << "header_str = " << header_str << "size = " << sizeof(header_str) << endl;
	memcpy(header_str, &network_header, sizeof(network_header));
	cout << "header_str = " << header_str << "size = " << sizeof(header_str) << endl;

	// 将包头字符串和原始字符串拼接起来
	std::string result(header_str, sizeof(header_str));
	result.append(ret);

	cout << "result = " << result << endl;
	return result;
#else
	if (status == false)
	{
		cout << "密钥已注销，请更新密钥！" << endl;
		return string();
	}

	// data -> 要加密的数据
	string head = "666 ";
	//string str = head + data;	// 666helloworld
	string str = data;	// 666helloworld
	AesCrypto aes(m_key);
	string ret = aes.aesCBCEncrypt(str);
	return ret;
#endif // 0

}

string Interface::decryptData(string data)
{
#if 0
	// 读接收到的前四个字节
	int header = 0;

	memcpy(&header, data.data(), sizeof(header));
	cout << "header = " << header << endl;
	m_secKeyID = header;

	// 读共享内存
	NodeSHMInfo node = m_shm->shmRead(m_secKeyID);
	m_key = string(node.seckey);


	string str = data.substr(sizeof(header));

	AesCrypto aes(m_key);
	return aes.aesCBCDecrypt(str);

#elif 0

	int header = 0;

	// 从字符串中读取int类型的包头
	uint32_t network_header;
	memcpy(&network_header, data.c_str(), sizeof(network_header));

	// 将包头转换为主机字节序
	header = ntohl(network_header);
	cout << "header = " << header << endl;

	string str = data.substr(sizeof(network_header));

	AesCrypto aes(m_key);
	cout << "aes.aesCBCDecrypt(str) = " << aes.aesCBCDecrypt(str) << endl;
	return aes.aesCBCDecrypt(str);

#else

	AesCrypto aes(m_key);
	return aes.aesCBCDecrypt(data);
#endif // 0
}
