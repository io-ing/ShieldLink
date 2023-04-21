#pragma once
#include <iostream>

class Codec
{
public:
	Codec();
	virtual std::string encodeMsg();
	virtual void* decodeMsg();

	virtual ~Codec();
};

