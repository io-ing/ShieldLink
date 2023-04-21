#include <cstdio>
#include "ServerOP.h"

int main()
{
	ServerOP op("server.json");
	op.startServer();

    return 0;
}