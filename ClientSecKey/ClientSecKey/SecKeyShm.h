#pragma once
#include "BaseShm.h"
#include <string.h>

class NodeSecKeyInfo
{
public:
	NodeSecKeyInfo() : status(0), seckeyID(0)
	{
		bzero(clientID, sizeof(clientID));
		bzero(serverID, sizeof(serverID));
		bzero(seckey, sizeof(seckey));
	}
	int status;		// 秘钥状态: 1可用, 0:不可用
	int seckeyID;	// 秘钥的编号
	char clientID[12];	// 客户端ID, 客户端的标识
	char serverID[12];	// 服务器ID, 服务器标识
	char seckey[128];	// 对称加密的秘钥
};

class SecKeyShm : public BaseShm
{
public:
	// 打开或创建一块共享内存
	// 这个操作是在父类中做的
	SecKeyShm(int key, int maxNode);
	SecKeyShm(string pathName, int maxNode);
	~SecKeyShm();

	void shmInit();
	int shmWrite(NodeSecKeyInfo* pNodeInfo);
	NodeSecKeyInfo shmRead(string clientID, string serverID);

private:
	int m_maxNode;
};

