//这里是客户端！！

#include "winsock2.h"
#include <WS2tcpip.h>
#include <iostream>
#include<Windows.h>
#include<time.h>
#include<thread>
#include <cassert>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
using namespace std;

static double send_time;
static SOCKET			sClient;			//客户端套接字
SOCKADDR_IN		servAddr;			//服务器地址
char pid[10];
bool havegotten = false;

void helpmenu();

void sendcheck()
{
	char	bufSend[64] = { "CHCK" };	//发送数据缓冲区
	strcat_s(bufSend, pid);
	cout << "send check";
	sendto(sClient, bufSend, strlen(bufSend), 0, (sockaddr*)&servAddr, sizeof(servAddr));
	while (true)
	{
		Sleep(108000000);  //暂时先设定为5s发送一次，便于测试，108000000为30min
		cout << "send check";
		sendto(sClient, bufSend, strlen(bufSend), 0, (sockaddr*)&servAddr, sizeof(servAddr));
	}
}
int main(int argc, char* argv[])
{
	const int		BUF_SIZE = 64;
	const int		COMMAND_LENGTH = 4;
	WSADATA			wsd;				//WSADATA变量
	char			bufSend[BUF_SIZE];	//发送数据缓冲区
	char			bufRecv[BUF_SIZE];  //接收数据缓冲区
	int				retVal;				//返回值
	int temppid		= (int)_getpid();
	char* closeSymbol = (char*)"00";//结束通信的标志
									  // 设置服务端地址
	servAddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", (void*)&servAddr.sin_addr.S_un.S_addr);
	servAddr.sin_port = htons((short)4999);
	int	nServAddlen = sizeof(servAddr);
	// 初始化套接字动态库
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		cout << "WSAStartup failed!" << endl;
		return -1;
	}
	// 创建服务端套接字
	sClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == sClient)
	{
		cout << "Socket failed !" << endl;
		WSACleanup();               //释放套接字资源
		return  -1;
	}
	else
	{
		cout << "原则上本次过程中的ID值为" << temppid << " (如需测试可自行修改)"<<endl;
		char t[10];
		cout << "本次过程中的ID值为:";
		cin >> t;
		ZeroMemory(pid, 10);
		strcat_s(pid, t);
		//cout << "若发送的数据是'0 0'，则表示想结束此次UDP通信\n";
		//cout << "Client UDP Socket init !" << endl;
		helpmenu();
	}
	//std::thread t(sendcheck);
	//std::cout << "sendcheck thread\n";
	//t.detach();     //该线程可以放飞自我

	// 循环等待，向服务端发送数据 & 从服务端接收数据
	while (true)
	{
		// 初始化buf空间
		ZeroMemory(bufSend, BUF_SIZE);
		// 向服务端发送数据buf
		cout << "\nData send: ";
		cin >> bufSend;
		if (!strcmp(bufSend, closeSymbol))
		{
			cout << "Client UDP Socket wants to finish this communication" << endl;
			break;
		}
		else if (!strcmp(bufSend, "LISC"))
		{
			char a[20];
			cout << "请输入你的许可证号：";
			cin >> a;
			strcat_s(bufSend, a);
			strcat_s(bufSend, ",");
			strcat_s(bufSend, pid);
		}
		else if (!strcmp(bufSend, "HELO"))
		{
			strcat_s(bufSend, pid);
			if (havegotten == true) {
				cout << "你已经拥有了许可，请勿重复申请。" << endl;
				continue;
			}
			cout << "许可请求已发送，请稍等片刻" << endl;
		}
		else if (!strcmp(bufSend, "GBYE"))
		{
			strcat_s(bufSend, pid);
			cout << "许可归还请求已发送，请稍等片刻" << endl;
		}
		else if (!strcmp(bufSend, "STOP"))
		{
			return 0;
		}
		else if (!strcmp(bufSend, "HELP"))
		{
			helpmenu();
			continue;
		}
		else
		{
			cout << "输入不符合规范，请重新输入" << endl;
			continue;
		}
		
		retVal = sendto(sClient, bufSend, strlen(bufSend), 0, (sockaddr*)&servAddr, nServAddlen);
		if (SOCKET_ERROR == retVal)
		{
			cout << "send failed!" << endl;
			closesocket(sClient);	//关闭服务端套接字			
			WSACleanup();		    //释放套接字资源
			return -1;
		}
		

		// 从服务端接收数据bufRecv
		retVal = recvfrom(sClient, bufRecv, BUF_SIZE, 0, (sockaddr*)&servAddr, &nServAddlen);
		bufRecv[retVal] = '\0';
		char str[INET_ADDRSTRLEN];
		//cout << "Server IP：" << inet_ntop(AF_INET, &servAddr.sin_addr, str, sizeof(str)) << endl;
		cout << "Data recv from Server: " << bufRecv << endl;

		/*成功获得确认*/
		if (!strncmp(bufRecv, "TICK", COMMAND_LENGTH))    
		{
			cout << "说明：您已获得许可钥匙，可正常使用\n";
			havegotten = true;
			char* ticketid;
			ticketid = &bufRecv[COMMAND_LENGTH];

			thread t(sendcheck);
			//cout << "sendcheck thread\n";
			t.detach();     //该线程可以放飞自我,与主线程并发进行
		}
		//服务器拒绝请求
		else if (!strncmp(bufRecv, "FAIL", COMMAND_LENGTH)) {
			cout << "非常抱歉，该许可证的所有钥匙已被占用，请稍后再试\n";

		}
		else if (!strncmp(bufRecv, "THNX", COMMAND_LENGTH)) {
			cout << "说明：您归还许可钥匙，感谢您的使用\n";
			havegotten = false;

		}
		// 若服务端发送的数据是'0'，则表示服务端想结束此次UDP通信	
		if (!strcmp(bufRecv, closeSymbol))
		{
			cout << "Server UDP Socket wants to finish this communication" << endl;
			break;
		}

	}
	//退出
	closesocket(sClient);	//关闭服务端套接字
	WSACleanup();
	//释放套接字资源
	Sleep(5000);
	return 0;
}

void helpmenu() {
	cout << "\n本程序运行需要按照如下步骤进行。" << endl;
	cout << "1.首先请输入 LISC 验证你的许可证。" << endl;
	cout << "2.其次请输入 HELO 请求获得对应许可。" << endl;
	cout << "3.然后请输入 GBYE 返还对应许可。 " << endl;
	cout << "4.最后请输入 STOP 安全退出程序。 " << endl;
	cout << "使用过程中有问题请输入 HELP 获得帮助" << endl;
}
