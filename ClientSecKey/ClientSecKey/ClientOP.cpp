#include "ClientOP.h"
#include <json/json.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "RequestFactory.h"
#include "RequestCodec.h"
#include "RsaCrypto.h"
#include "TcpSocket.h"
#include "RespondFactory.h"
#include "RespondCodec.h"
#include "Message.pb.h"
#include "Hash.h"

using namespace std;
using namespace Json;

ClientOP::ClientOP(string jsonFile)
{
	// 解析 json 文件到 root
	ifstream ifs(jsonFile);
	Reader r;
	Value root;
	r.parse(ifs, root);

	// 初始化成员变量
	m_info.ServerID = root["ServerID"].asString();
	m_info.ClientID = root["ClientID"].asString();
	m_info.ip = root["ServerIP"].asString();
	m_info.port = root["Port"].asInt();

	// 实例化共享内存对象
	string shmKey = root["ShmKey"].asString();
	m_shm = new SecKeyShm(shmKey, 1);		// 客户端的秘钥只有一个

	//int maxNode = root["ShmMaxNode"].asInt();
	//m_shm = new SecKeyShm(shmKey, maxNode);
}

ClientOP::~ClientOP()
{
	delete m_shm;
}

bool ClientOP::seckeyAgree()
{
	// 生成密钥对，读取公钥
	RsaCrypto rsa;
	rsa.generateRsakey(1024);
	ifstream ifs("public.pem");
	stringstream pubKey;
	pubKey << ifs.rdbuf();

	// 组织要发送的数据
	// 创建哈希对象
	Hash sha1(T_SHA1);
	sha1.addData(pubKey.str());

	RequestInfo reqInfo;
	reqInfo.clientID = m_info.ClientID;
	reqInfo.serverID = m_info.ServerID;
	reqInfo.cmd = 1;							// 1 代表秘钥协商
	reqInfo.data = pubKey.str();				// 非对称加密的公钥
	reqInfo.sign = rsa.rsaSign(sha1.result());	// 公钥的的哈希值签名
	cout << "签名完成..." << endl;

	// 使用工厂类创建序列化的类对象，数据序列化
	CodecFactory* factory = new RequestFactory(&reqInfo);
	Codec* c =  factory->createCodec();
	string encstr = c->encodeMsg();

	// 释放资源
	delete factory;
	delete c;

	// 套接字通信，连接服务器
	TcpSocket* tcp = new TcpSocket;
	int ret = tcp->connectToHost(m_info.ip, m_info.port);
	if (ret != 0)
	{
		cout << "连接服务器失败！" << endl;
		return false;
	}
	cout << "连接服务器成功..." << endl;

	// 发送序列化数据，等待服务端回复
	tcp->sendMsg(encstr);
	string msg = tcp->recvMsg();

	// 反序列化接收到的数据
	factory = new RespondFactory(msg);
	c = factory->createCodec();
	RespondMsg* resData = (RespondMsg*)c->decodeMsg();

	// 判断状态，协商失败就返回
	if (!resData->status())
	{
		cout << "秘钥协商失败！" << endl;
		return false;
	}
	// 私钥解密得到对称加密的密钥
	string key = rsa.rsaPriKeyDecrypt(resData->data());
	cout << "对称加密的秘钥: " << key << endl;

	// 秘钥写入共享内存中
	NodeSecKeyInfo info;
	strcpy(info.clientID, m_info.ClientID.data());
	strcpy(info.serverID, m_info.ServerID.data());
	strcpy(info.seckey, key.data());
	info.seckeyID = resData->seckeyid();
	info.status = true;
	m_shm->shmWrite(&info);

	delete factory;
	delete c;
	tcp->disConnect();	// 短连接，通信完成，断开连接
	delete tcp;

	return true;
}

void ClientOP::seckeyCheck()
{
	// 读共享内存
	NodeSecKeyInfo secInfo = m_shm->shmRead(m_info.ClientID, m_info.ServerID);

	if (secInfo.status == 0)
	{
		cout << "密钥已注销，请更新密钥！" << endl;
		return;
	}

	// 组织要发送的数据
	// 生成对称加密密钥的哈希值
	Hash sha1(T_SHA1);
	sha1.addData(secInfo.seckey);
	string seckeyHash = sha1.result();
	cout << "seckey = " << secInfo.seckey << endl;
	cout << "sha1.result() = " << seckeyHash << endl;

	RequestInfo reqInfo;
	reqInfo.cmd = 2;							// 2 代表秘钥校验
	reqInfo.clientID = m_info.ClientID;
	reqInfo.serverID = m_info.ServerID;
	reqInfo.sign = seckeyHash;					// 对称加密密钥的哈希值

	// 使用工厂类创建序列化的类对象，数据序列化
	CodecFactory* factory = new RequestFactory(&reqInfo);
	Codec* c = factory->createCodec();
	string encstr = c->encodeMsg();

	// 释放资源
	delete factory;
	delete c;

	// 套接字通信
	TcpSocket* tcp = new TcpSocket;
	int ret = tcp->connectToHost(m_info.ip, m_info.port);
	if (ret != 0)
	{
		cout << "连接服务器失败！" << endl;
		return;
	}
	cout << "连接服务器成功..." << endl;

	// 发送序列化数据，等待服务端回复
	tcp->sendMsg(encstr);
	string msg = tcp->recvMsg();

	// 反序列化接收到的数据
	factory = new RespondFactory(msg);
	c = factory->createCodec();
	RespondMsg* resData = (RespondMsg*)c->decodeMsg();

	// 判断状态，协商失败就返回
	cout << "resData->status() = " << resData->status() << endl;
	if (resData->status())
	{
		cout << "密钥相同，无需更新密钥！" << endl;
	}
	else
	{
		cout << "密钥不同，请更新密钥！" << endl;
	}
	delete factory;
	delete c;
	tcp->disConnect();	// 短连接，通信完成，断开连接
	delete tcp;
}

void ClientOP::seckeyLogoff()
{
	// 读共享内存
	NodeSecKeyInfo node = m_shm->shmRead(m_info.ClientID, m_info.ServerID);

	if (node.status == 0)
	{
		cout << "密钥已注销！无需重复注销。" << endl;
		return;
	}
	
	// 组织要发送的数据
	RequestInfo reqInfo;
	reqInfo.clientID = m_info.ClientID;
	reqInfo.serverID = m_info.ServerID;
	reqInfo.cmd = 3;							// 3 代表秘钥注销

	// 使用工厂类创建序列化的类对象，数据序列化
	CodecFactory* factory = new RequestFactory(&reqInfo);
	Codec* c = factory->createCodec();
	string encstr = c->encodeMsg();

	// 释放资源
	delete factory;
	delete c;

	// 套接字通信，连接服务器
	TcpSocket* tcp = new TcpSocket;
	int ret = tcp->connectToHost(m_info.ip, m_info.port);
	if (ret != 0)
	{
		cout << "连接服务器失败！" << endl;
		return;
	}
	cout << "连接服务器成功..." << endl;

	// 发送序列化数据，等待服务端回复
	tcp->sendMsg(encstr);
	string msg = tcp->recvMsg();

	// 反序列化接收到的数据
	factory = new RespondFactory(msg);
	c = factory->createCodec();
	RespondMsg* resData = (RespondMsg*)c->decodeMsg();

	// 判断状态，注销失败就返回
	if (resData->status())
	{
		// 服务端注销成功后把本地的密钥也注销
		node.status = 0;
		m_shm->shmWrite(&node);
		cout << "密钥注销成功！" << endl;
	}
	else
	{
		cout << "秘钥注销失败！" << endl;
	}

	delete factory;
	delete c;
	tcp->disConnect();	// 短连接，通信完成，断开连接
	delete tcp;
}
