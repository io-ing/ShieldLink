#include "OCCIOP.h"
#include <iostream>
#include <string.h>
#include <time.h>
using namespace std;
using namespace oracle::occi;

OCCIOP::OCCIOP()
{
}


OCCIOP::~OCCIOP()
{
}

bool OCCIOP::connectDB(string user, string passwd, string connstr)
{
	// 1. 初始化环境
	m_env = Environment::createEnvironment("utf8", "utf8");
	// 2. 创建连接
	m_conn = m_env->createConnection(user, passwd, connstr);
	if (m_conn == NULL)
	{
		return false;
	}
	cout << "数据库连接成功..." << endl;
	return true;
}

int OCCIOP::getKeyID()
{
	// 查询数据库
	// for update: 临时对数据表加锁
	string sql = "select ikeysn from SECMNG.KEYSN for update";
	Statement* stat = m_conn->createStatement(sql);
	// 执行数据查询
	ResultSet* resSet = stat->executeQuery();
	// 该表只有一条记录
	int keyID = -1;
	if (resSet->next())
	{
		keyID = resSet->getInt(1);
	}
	stat->closeResultSet(resSet);
	m_conn->terminateStatement(stat);

	return keyID;
}

// 秘钥ID在插入的时候回自动更新, 也可以手动更新
bool OCCIOP::updataKeyID(int keyID)
{
	// 更新数据库
	string sql = "update SECMNG.KEYSN set ikeysn = " + to_string(keyID);
	Statement* stat = m_conn->createStatement(sql);
	// 设置自动提交
	stat->setAutoCommit(true);	
	// 执行数据查询
	int ret = stat->executeUpdate();
	m_conn->terminateStatement(stat);
	if (ret <= 0)
	{
		return false;
	}
	return true;
}

// 将生成的秘钥写入数据库
// 更新秘钥编号
bool OCCIOP::writeSecKey(NodeSecKeyInfo *pNode)
{
	// 组织待插入的sql语句
	char sql[1024] = { 0 };
	sprintf(sql, "Insert Into SECMNG.SECKEYINFO(clientid, serverid, keyid, createtime, state, seckey) \
					values ('%s', '%s', %d, to_date('%s', 'yyyy-mm-dd hh24:mi:ss') , %d, '%s') ", 
		pNode->clientID, pNode->serverID, pNode->seckeyID, 
		getCurTime().data(), 1, pNode->seckey);
	cout << "insert sql: " << sql << endl;
	Statement* stat = m_conn->createStatement();
	// 数据初始化
	stat->setSQL(sql);
	// 执行sql
	stat->setAutoCommit(true);
	bool bl = stat->executeUpdate(sql);
	m_conn->terminateStatement(stat);
	if (bl)
	{
		return true;
	}
	return false;
}

// 将密钥状态设为不可用
bool OCCIOP::updateState(int keyID)
{
	// 更新数据库
	string sql = "update SECMNG.SECKEYINFO set state=0 where keyid = " + to_string(keyID);
	cout << "sql = " << sql << endl;

	Statement* stat = m_conn->createStatement(sql);
	// 设置自动提交
	stat->setAutoCommit(true);
	// 执行数据查询
	int ret = stat->executeUpdate();
	m_conn->terminateStatement(stat);
	if (ret <= 0)
	{
		return false;
	}
	return true;
}

void OCCIOP::closeDB()
{
	m_env->terminateConnection(m_conn);
	Environment::terminateEnvironment(m_env);
}

string OCCIOP::getCurTime()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));

	return tmp;
}
