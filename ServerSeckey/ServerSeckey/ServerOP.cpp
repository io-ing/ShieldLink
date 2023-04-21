#include "ServerOP.h"
#include "TcpSocket.h"
#include "RequestFactory.h"
#include "RequestCodec.h"
#include "RespondCodec.h"
#include "RespondFactory.h"
#include "RsaCrypto.h"
#include <string>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <unistd.h>
#include "Hash.h"

using namespace std;
using namespace Json;

ServerOP::ServerOP(string json)
{
	// 解析 json 文件到 root
	ifstream ifs(json);
	Reader r;
	Value root;
	r.parse(ifs, root);

	// 初始化成员变量
	m_port = root["Port"].asInt();
	m_serverID = root["ServerID"].asString();

	// 实例化连接 oracle 的对象
	m_dbUser = root["UserDB"].asString();
	m_dbPwd = root["PwdDB"].asString();
	m_dbConnStr = root["ConnStrDB"].asString();
	m_occi.connectDB(m_dbUser, m_dbPwd, m_dbConnStr);

	// 实例化共享内存对象
	string shmKey = root["ShmKey"].asString();
	int maxNode = root["ShmMaxNode"].asInt();
	m_shm = new SecKeyShm(shmKey, maxNode);
}


void ServerOP::startServer()
{
	m_server = new TcpServer;
	m_server->setListen(m_port);
	while (1)
	{
		cout << "等待客户端连接..." << endl;
		TcpSocket* tcp = m_server->acceptConn();
		if (tcp == NULL)
		{
			continue;	// 项目不是练习，程序出问题不能直接挂
		}
		cout << "与客户端连接成功..." << endl;

		// pthread_create 中的回调函数能传 普通函数、类的静态函数、友元函数
        // 在回调函数中发送数据，参数需要传 tcp
        // 在回调函数中调用成员函数需要通过对象调用，参数需要传 this
        // 回调函数属于某个子线程，通过线程id tid 能找到 tcp
        // 普通函数无法访问私有成员 m_list，要么提供接口，要么使用友元
		pthread_t tid;
		//pthread_create(&tid, NULL, workHard, this);
		pthread_create(&tid, NULL, working, this);
		m_list.insert(make_pair(tid, tcp));
	}
}

void * ServerOP::working(void * arg)
{
	sleep(1);

	string data = string();

	ServerOP* op = (ServerOP*)arg;
	// 将通信的套接字对象取出
	TcpSocket* tcp = op->m_list[pthread_self()];

	// 接收客户端数据
	string msg = tcp->recvMsg();

	// 反序列化
	CodecFactory* fac = new RequestFactory(msg);
	Codec* c = fac->createCodec();
	RequestMsg* req = (RequestMsg*)c->decodeMsg();

	// 根据 cmd 判断客户端的请求
	switch (req->cmdtype())
	{
	case 1:	data = op->seckeyAgree(req);	break;	// 秘钥协商
	case 2:	data = op->seckeyCheck(req);	break;	// 秘钥校验
	case 3:	data = op->seckeyLogoff(req);	break;	// 秘钥注销
	case 4:									break;	// 秘钥查看
	default:								break;
	}


	// 发送返回的数据
	tcp->sendMsg(data);

	// 释放资源
	tcp->disConnect();
	op->m_list.erase(pthread_self());
	delete tcp;
	delete fac;
	delete c;

	return NULL;
}

string ServerOP::seckeyAgree(RequestMsg* reqMsg)
{
	// 将收到的公钥写到本地
	ofstream ofs("public.pem");
	ofs << reqMsg->data();
	ofs.close();

	// 创建非对称加密对象
	RespondInfo info;
	RsaCrypto rsa("public.pem", false);
	
	// 创建哈希对象
	Hash sha(T_SHA1);
	sha.addData(reqMsg->data());

	// 对签名的哈希值进行校验
	bool bl = rsa.rsaVerify(sha.result(), reqMsg->sign());
	if (bl == false)
	{
		cout << "签名校验失败..." << endl;
		info.status = false;
	}
	else
	{
		cout << "签名校验成功..." << endl;
		// 生成对称加密的秘钥, 使用对称加密算法 aes, 秘钥长度: 16、24、32byte
		string key = getRandKey(Len16);
		cout << "生成的随机秘钥: " << key << endl;

		// 通过公钥加密对称加密密钥
		string seckey = rsa.rsaPubKeyEncrypt(key);
		cout << "加密之后的秘钥: " << seckey << endl;

		// 组织要发送的数据
		info.clientID = reqMsg->clientid();
		info.data = seckey;
		info.serverID = m_serverID;
		info.status = true;	

		// 组织要写入数据库的数据
		NodeSecKeyInfo node;
		strcpy(node.clientID, reqMsg->clientid().data());
		strcpy(node.serverID, reqMsg->serverid().data());
		strcpy(node.seckey, key.data());
		node.seckeyID = m_occi.getKeyID();	// 秘钥的 ID
		info.seckeyID = node.seckeyID;		// 要发送的数据 seckeyID 由读数据库获得
		node.status = 1;					// 1 为可用

		// 写入到数据库 SECKEYINFO 表中
		bool bl = m_occi.writeSecKey(&node);
		if(bl)	// 成功
		{
			m_occi.updataKeyID(node.seckeyID + 1);
			// 写共享内存
			m_shm->shmWrite(&node);
		}
		else	// 失败
		{
			info.status = false;
		}
	}

	// 序列化
	CodecFactory* fac = new RespondFactory(&info);
	Codec* c = fac->createCodec();
	string encMsg = c->encodeMsg();

	// 返回序列化后的数据，由回调函数发送
	return encMsg;
}

string ServerOP::seckeyCheck(RequestMsg* reqMsg)
{
	// 读共享内存
	cout << "clientid = "<< reqMsg->clientid().data() <<endl;
	cout << "serverid = "<< reqMsg->serverid().data() <<endl;
	NodeSecKeyInfo secInfo = m_shm->shmRead(reqMsg->clientid().data(), reqMsg->serverid().data());

	// 生成对称加密密钥的哈希值
	Hash sha1(T_SHA1);
	sha1.addData(secInfo.seckey);
	string sha1Res = sha1.result().data();
	string reqSin = reqMsg->sign();
	
	cout << "sha1.result()         = " << sha1Res << endl;
	cout << "reqMsg->sign()        = " << reqSin << endl;

	// 组织要发送的数据
	RespondInfo info;

	//cout << "sha1Res               = ";
	//for (auto ch : sha1Res)
	//{
	//	cout << static_cast<int>(ch) << " ";
	//}
	//cout << endl;

	//cout << "reqMsg->sign()        = ";
	//for (auto ch : reqMsg->sign())
	//{
	//	cout << static_cast<int>(ch) << " ";
	//}
	//cout << endl;

	// 如果密钥哈希值相同
	if (reqSin.compare(sha1Res) == 0)
	//if (reqMsg->sign() == sha1Res)
	{
		info.status = true;
		cout << "info.status = true;" << endl;
	}
	else
	{
		info.status = false;
		cout << "info.status = false;" << endl;
	}

	// 序列化
	CodecFactory* fac = new RespondFactory(&info);
	Codec* c = fac->createCodec();
	string encMsg = c->encodeMsg();

	// 返回序列化后的数据，由回调函数发送
	return encMsg;
}

string ServerOP::seckeyLogoff(RequestMsg* reqMsg)
{	
	// 读共享内存
	cout << "clientid = " << reqMsg->clientid().data() << endl;
	cout << "serverid = " << reqMsg->serverid().data() << endl;
	NodeSecKeyInfo node = m_shm->shmRead(reqMsg->clientid().data(), reqMsg->serverid().data());

	// 共享内存中的密钥标记为不可用
	node.status = 0;

	// 组织返回的数据
	RespondInfo info;

	// SECKEYINFO 表中的 state 置为 0
	bool bl = m_occi.updateState(node.seckeyID);
	if (bl)	// 成功
	{
		// 写共享内存
		m_shm->shmWrite(&node);
		info.status = true;
	}
	else	// 失败
	{
		info.status = false;
	}

	// 序列化
	CodecFactory* fac = new RespondFactory(&info);
	Codec* c = fac->createCodec();
	string encMsg = c->encodeMsg();

	// 返回序列化后的数据，由回调函数发送
	return encMsg;
}

ServerOP::~ServerOP()
{
	if (m_server)
	{
		delete m_server;
	}
	delete m_shm;
}

// 要求字符串中包含：a-z、A-Z、0-9、特殊字符
string ServerOP::getRandKey(KeyLen len)
{
	// 设置随机数种子
	srand(time(NULL));

	int flag = 0;
	string randStr = string();
	char *cs = "~!@#$%^&*()_+}{|\';[]";

	for (int i = 0; i < len; ++i)
	{
		flag = rand() % 4;
		switch (flag)
		{
		case 0:	randStr.append(1, 'a' + rand() % 26);		break;	// a-z
		case 1:	randStr.append(1, 'A' + rand() % 26);		break;	// A-Z
		case 2:	randStr.append(1, '0' + rand() % 10);		break;	// 0-9
		case 3:	randStr.append(1, cs[rand() % strlen(cs)]);	break;	// 特殊字符
		default:											break;
		}
	}

	return randStr;
}

void* workHard(void * arg)
{
	sleep(1);

	string data = string();

	ServerOP* op = (ServerOP*)arg;
	// 将通信的套接字对象取出
	TcpSocket* tcp = op->m_list[pthread_self()];

	// 接收客户端数据
	string msg = tcp->recvMsg();

	// 反序列化
	CodecFactory* fac = new RequestFactory(msg);
	Codec* c = fac->createCodec();
	RequestMsg* req = (RequestMsg*)c->decodeMsg();

	// 根据 cmd 判断客户端的请求
	switch (req->cmdtype())
	{
	case 1:	data = op->seckeyAgree(req);	break;	// 秘钥协商
	case 2:	data = op->seckeyCheck(req);	break;	// 秘钥校验
	case 3:	data = op->seckeyLogoff(req);	break;	// 秘钥注销
	case 4:									break;	// 秘钥查看
	default:								break;
	}


	// 发送返回的数据
	tcp->sendMsg(data);

	// 释放资源
	tcp->disConnect();
	op->m_list.erase(pthread_self());
	delete tcp;
	delete fac;
	delete c;

	return NULL;
}
