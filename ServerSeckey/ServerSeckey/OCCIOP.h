#pragma once
#include <string>
#include <occi.h>
#include "SeckKeyNodeInfo.h"
using namespace std;
using namespace oracle::occi;


class OCCIOP
{
public:
	OCCIOP();
	~OCCIOP();

	// 初始化环境连接数据库
	bool connectDB(string user, string passwd, string connstr);
	// 得到keyID -> 根据实际业务需求封装的小函数
	int getKeyID();
	bool updataKeyID(int keyID);
	bool writeSecKey(NodeSecKeyInfo *pNode);
	bool updateState(int keyID);
	void closeDB();

private:
	// 获取当前时间, 并格式化为字符串
	string getCurTime();

private:
	Environment* m_env;
	Connection* m_conn;
};

