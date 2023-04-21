#include <iostream>
#include <string>
#include "ClientOP.h"
using namespace std;

int usage();
int main()
{
	// 创建客户端操作类对象
	ClientOP op("client.json");
	while (1)
	{
		int sel = usage();
		switch (sel)
		{
		case 0:
			break;				break;	// 退出系统
		case 1:
			op.seckeyAgree();	break;	// 秘钥协商
		case 2:
			op.seckeyCheck();	break;	// 秘钥校验
		case 3:
			op.seckeyLogoff();	break;	// 秘钥注销
		default:
								break;
		}
	}

	cout << "客户端退出！" << endl;
	return 0;
}

int usage()
{
	int nSel = -1;
	printf("\n  /=============================================================/");
	printf("\n  /=============================================================/");
	printf("\n  /*     1.密钥协商                                            */");
	printf("\n  /*     2.密钥校验                                            */");
	printf("\n  /*     3.密钥注销                                            */");
	//printf("\n  /*     4.密钥查看                                            */");
	printf("\n  /*     0.退出系统                                            */");
	printf("\n  /=============================================================/");
	printf("\n  /=============================================================/");
	printf("\n\n  选择:");

	scanf("%d", &nSel);
	while (getchar() != '\n');

	return nSel;
}