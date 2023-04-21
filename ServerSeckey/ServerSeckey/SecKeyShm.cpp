#include "SecKeyShm.h"
#include <string.h>
#include <iostream>
using namespace std;

SecKeyShm::SecKeyShm(int key, int maxNode)
	: BaseShm(key, maxNode * sizeof(NodeSecKeyInfo))
	, m_maxNode(maxNode)
{
}

SecKeyShm::SecKeyShm(string pathName, int maxNode)
	: BaseShm(pathName, maxNode * sizeof(NodeSecKeyInfo))
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
		memset(m_shmAddr, 0, m_maxNode * sizeof(NodeSecKeyInfo));
	}
}

int SecKeyShm::shmWrite(NodeSecKeyInfo * pNodeInfo)
{
	int ret = -1;
	// 关联共享内存
	NodeSecKeyInfo* pAddr = static_cast<NodeSecKeyInfo*>(mapShm());
	if (pAddr == NULL)
	{
		return ret;
	}

	// 判断传入的网点密钥是否已经存在
	NodeSecKeyInfo	*pNode = NULL;
	for (int i = 0; i < m_maxNode; i++)
	{
		// pNode依次指向每个节点的首地址
		pNode = pAddr + i;
		cout << i << endl;
		cout << "clientID 比较: " << pNode->clientID << ", " << pNodeInfo->clientID << endl;
		cout << "serverID 比较: " << pNode->serverID << ", " << pNodeInfo->serverID << endl;
		cout << endl;
		if (strcmp(pNode->clientID, pNodeInfo->clientID) == 0 &&
			strcmp(pNode->serverID, pNodeInfo->serverID) == 0)
		{
			// 如果找到了该网点秘钥已经存在, 使用新秘钥覆盖旧的值
			memcpy(pNode, pNodeInfo, sizeof(NodeSecKeyInfo));
			unmapShm();
			cout << "写数据成功: 原数据被覆盖!" << endl;
			return 0;
		}
	}

	// 若没有找到对应的信息, 找一个空节点将秘钥信息写入
	int i = 0;
	NodeSecKeyInfo tmpNodeInfo; //空结点
	for (i = 0; i < m_maxNode; i++)
	{
		pNode = pAddr + i;
		if (memcmp(&tmpNodeInfo, pNode, sizeof(NodeSecKeyInfo)) == 0)
		{
			ret = 0;
			memcpy(pNode, pNodeInfo, sizeof(NodeSecKeyInfo));
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

NodeSecKeyInfo SecKeyShm::shmRead(string clientID, string serverID)
{
	int ret = 0;
	// 关联共享内存
	NodeSecKeyInfo *pAddr = NULL;
	pAddr = static_cast<NodeSecKeyInfo*>(mapShm());
	if (pAddr == NULL)
	{
		cout << "共享内存关联失败..." << endl;
		return NodeSecKeyInfo();
	}
	cout << "共享内存关联成功..." << endl;

	//遍历网点信息
	int i = 0;
	NodeSecKeyInfo info;
	NodeSecKeyInfo	*pNode = NULL;
	// 通过clientID和serverID查找节点
	cout << "maxNode: " << m_maxNode << endl;
	for (i = 0; i < m_maxNode; i++)
	{
		pNode = pAddr + i;
		cout << i << endl;
		cout << "clientID 比较: " << pNode->clientID << ", " << clientID.data() << endl;
		cout << "serverID 比较: " << pNode->serverID << ", " << serverID.data() << endl;
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
