#include "RequestCodec.h"
#include <iostream>

RequestCodec::RequestCodec()
{
}

RequestCodec::RequestCodec(string encstr)
{
	initMessage(encstr);
}

RequestCodec::RequestCodec(RequestInfo *info)
{
	initMessage(info);
}

void RequestCodec::initMessage(string encstr)
{
	m_encStr = encstr;
}

void RequestCodec::initMessage(RequestInfo *info)
{
	m_msg.set_cmdtype(info->cmd);
	m_msg.set_clientid(info->clientID);
	m_msg.set_serverid(info->serverID);
	m_msg.set_sign(info->sign);
	m_msg.set_data(info->data);
}

string RequestCodec::encodeMsg()
{
	string output;
	m_msg.SerializeToString(&output);
	return output;
}

void* RequestCodec::decodeMsg()
{
	m_msg.ParseFromString(m_encStr);
	return &m_msg;
}

RequestCodec::~RequestCodec()
{
}
