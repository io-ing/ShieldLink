#pragma once
#include "Codec.h"
#include "Message.pb.h"
#include <iostream>
using namespace std;

struct RequestInfo
{
	int cmd;
	string clientID;
	string serverID;
	string sign;
	string data;
};

class RequestCodec :
	public Codec
{
public:
	// 空对象
	RequestCodec();
	// 构造出的对象用于 解码 的场景
	RequestCodec(string encstr);
	// 构造出的对象用于 编码 场景
	RequestCodec(RequestInfo *info);
	// init函数给空构造准备 的
	// 解码使用
	void initMessage(string encstr);
	// 编码时候使用
	void initMessage(RequestInfo *info);
	// 重写的父类函数 -> 序列化函数, 返回序列化的字符串
	string encodeMsg();
	// 重写的父类函数 -> 反序列化函数, 返回结构体/类对象
	void* decodeMsg();

	~RequestCodec();

private:
	string m_encStr;
	RequestMsg m_msg;
};

