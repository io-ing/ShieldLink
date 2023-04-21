#include "SecKeyShm.h"
#include <string.h>
#include <iostream>
using namespace std;

SecKeyShm::SecKeyShm(int key, int maxNode)
	: BaseShm(key, maxNode * sizeof(NodeSHMInfo))
	, m_maxNode(maxNode)
{
}

SecKeyShm::SecKeyShm(string pathName, int maxNode)
	: BaseShm(pathName, maxNode * sizeof(NodeSHMInfo))
	, m_maxNode(maxNode)
{
}

SecKeyShm::~SecKeyShm()
{
}

void SecKeyShm::shmInit()
{
	if (m_shmAddr != NULL)
	{
		memset(m_shmAddr, 0, m_maxNode * sizeof(NodeSHMInfo));
	}
}

int SecKeyShm::shmWrite(NodeSHMInfo * pNodeInfo)
{
	int ret = -1;
	// 关联共享内存
	NodeSHMInfo* pAddr = static_cast<NodeSHMInfo*>(mapShm());
	if (pAddr == NULL)
	{
		return ret;
	}

	// 判断传入的网点密钥是否已经存在
	NodeSHMInfo	*pNode = NULL;
	for (int i = 0; i < m_maxNode; i++)
	{
		// pNode依次指向每个节点的首地址
		pNode = pAddr + i;
		cout << i << endl;
		cout << pNode->clientID << ", " << pNodeInfo->clientID << endl;
		cout << pNode->serverID << ", " << pNodeInfo->serverID << endl;
		cout << endl;
		if (strcmp(pNode->clientID, pNodeInfo->clientID) == 0 &&
			strcmp(pNode->serverID, pNodeInfo->serverID) == 0)
		{
			// 如果找到了该网点秘钥已经存在, 使用新秘钥覆盖旧的值
			memcpy(pNode, pNodeInfo, sizeof(NodeSHMInfo));
			unmapShm();
			cout << "写数据成功: 原数据被覆盖!" << endl;
			return 0;
		}
	}

	// 若没有找到对应的信息, 找一个空节点将秘钥信息写入
	int i = 0;
	NodeSHMInfo tmpNodeInfo; //空结点
	for (i = 0; i < m_maxNode; i++)
	{
		pNode = pAddr + i;
		if (memcmp(&tmpNodeInfo, pNode, sizeof(NodeSHMInfo)) == 0)
		{
			ret = 0;
			memcpy(pNode, pNodeInfo, sizeof(NodeSHMInfo));
			cout << "写数据成功: 在新的节点上添加数据!" << endl;
			break;
		}
	}
	if (i == m_maxNode)
	{
		ret = -1;
	}

	unmapShm();
	return ret;
}

NodeSHMInfo SecKeyShm::shmRead(string clientID, string serverID)
{
	int ret = 0;
	// 关联共享内存
	NodeSHMInfo *pAddr = NULL;
	pAddr = static_cast<NodeSHMInfo*>(mapShm());
	if (pAddr == NULL)
	{
		cout << "共享内存关联失败..." << endl;
		return NodeSHMInfo();
	}
	cout << "共享内存关联成功..." << endl;

	//遍历网点信息
	int i = 0;
	NodeSHMInfo info;
	NodeSHMInfo	*pNode = NULL;
	// 通过clientID和serverID查找节点
	cout << "maxNode: " << m_maxNode << endl;
	for (i = 0; i < m_maxNode; i++)
	{
		pNode = pAddr + i;
		cout << i << endl;
		cout << pNode->clientID << ", " << clientID.data() << endl;
		cout << pNode->serverID << ", " << serverID.data() << endl;
		if (strcmp(pNode->clientID, clientID.data()) == 0 &&
			strcmp(pNode->serverID, serverID.data()) == 0)
		{
			// 找到的节点信息, 拷贝到传出参数
			info = *pNode;
			cout << "++++++++++++++" << endl;
			cout << info.clientID << " , " << info.serverID << ", "
				<< info.seckeyID << ", " << info.status << ", "
				<< info.seckey << endl;
			cout << "===============" << endl;
			break;
		}
	}

	unmapShm();
	return info;
}

NodeSHMInfo SecKeyShm::shmRead(int keyID)
{
	int ret = 0;
	// 关联共享内存
	NodeSHMInfo* pAddr = NULL;
	pAddr = static_cast<NodeSHMInfo*>(mapShm());
	if (pAddr == NULL)
	{
		cout << "共享内存关联失败..." << endl;
		return NodeSHMInfo();
	}
	cout << "共享内存关联成功..." << endl;

	//遍历网点信息
	int i = 0;
	NodeSHMInfo info;
	NodeSHMInfo* pNode = NULL;
	// 通过clientID和serverID查找节点
	cout << "maxNode: " << m_maxNode << endl;
	for (i = 0; i < m_maxNode; i++)
	{
		pNode = pAddr + i;
		cout << i << endl;
//		cout << pNode->clientID << ", " << clientID.data() << endl;
//		cout << pNode->serverID << ", " << serverID.data() << endl;
		if (keyID == pNode->seckeyID)
		{
			// 找到的节点信息, 拷贝到传出参数
			info = *pNode;
			cout << "++++++++++++++" << endl;
			cout << info.clientID << " , " << info.serverID << ", "
				<< info.seckeyID << ", " << info.status << ", "
				<< info.seckey << endl;
			cout << "===============" << endl;
			break;
		}
	}

	unmapShm();
	return info;
}

NodeSHMInfo SecKeyShm::shmFirstNode()
{
	int ret = 0;
	// 关联共享内存
	NodeSHMInfo* pAddr = NULL;
	pAddr = static_cast<NodeSHMInfo*>(mapShm());
	if (pAddr == NULL)
	{
		cout << "共享内存关联失败..." << endl;
		return NodeSHMInfo();
	}
	cout << "共享内存关联成功..." << endl;

	// 返回第一个节点
	cout << "++++++++++++++" << endl;
	cout << pAddr->clientID << " , " << pAddr->serverID << ", "
		<< pAddr->seckeyID << ", " << pAddr->status << ", "
		<< pAddr->seckey << endl;
	cout << "===============" << endl;


//	unmapShm();
	NodeSHMInfo* pNode = pAddr + 0;
	NodeSHMInfo info = *pNode;
	cout << "return info 成功..." << endl;
	return info;

	//unmapShm();
	//return *pAddr;
}
