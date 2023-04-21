#pragma once
#include "BaseShm.h"
#include <string.h>

class NodeSHMInfo
{
public:
	NodeSHMInfo() : status(0), seckeyID(0)
	{
		bzero(clientID, sizeof(clientID));
		bzero(serverID, sizeof(serverID));
		bzero(seckey, sizeof(seckey));
	}
	int status;
	int seckeyID;
	char clientID[12];
	char serverID[12];
	char seckey[128];
};

class SecKeyShm : public BaseShm
{
public:
	SecKeyShm(int key, int maxNode);
	SecKeyShm(string pathName, int maxNode);
	~SecKeyShm();

	void shmInit();
	int shmWrite(NodeSHMInfo* pNodeInfo);
	NodeSHMInfo shmRead(string clientID, string serverID);
	NodeSHMInfo shmRead(int keyID);
	// 这个函数给客户端使用，通过这个函数读共享内存中第一个 NodeSHMInfo
	NodeSHMInfo shmFirstNode();

private:
	int m_maxNode;
};

