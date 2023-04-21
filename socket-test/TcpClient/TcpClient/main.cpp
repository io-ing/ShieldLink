#include <cstdio>
#include "TcpSocket.h"
#include <string.h>
#include <iostream>
#include <unistd.h>
#include "Interface.h"

using namespace std;

int main()
{
	// 创建套接字
	TcpSocket socket;
	// 连接服务器
	cout << "开始连接..." << endl;
	socket.connectToHost("127.0.0.1", 8888);
	cout << "连接成功..." << endl;

	// 创建接口对象
	Interface inter("tcpClient.json");
	// 通信
	while (1)
	{
		string sendmsg = "hello server ...";
		sendmsg = inter.encryptData(sendmsg);
		if (sendmsg.empty())
		{
			cout << "sendmsg == " << sendmsg << endl;
			break;
		}
		cout << "发送数据：" << sendmsg << endl;
		int ret = socket.sendMsg(sendmsg);
		cout << "发送数据成功：ret = "<< ret << endl;

		// 接收数据
		int recvLen = -1;
		string recvMsg = socket.recvMsg();
		// 数据解密
		recvMsg = inter.decryptData(recvMsg);
		cout << "接收数据：" << recvMsg << endl;
		sleep(1);
	}
    return 0;
}