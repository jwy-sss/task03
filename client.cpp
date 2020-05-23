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

bool inputcheck(char * bufSend);

static double send_time;
static SOCKET			sClient;			//客户端套接字
SOCKADDR_IN		servAddr;			//服务器地址

const int		COMMAND_LENGTH = 4;
bool			gottenkey = false;
void sendcheck()
{
	while (true)
	{
		Sleep(108000000);  //暂时先设定为5s发送一次，便于测试，108000000为30min
		//double current_time = clock();
		//double durationTime = ((double)(current_time - send_time)) / CLK_TCK;
		//if (durationTime > 20)     //如果时间大于30分钟
		//{
			char	bufSend[64] = { "CHCK1234" };	//发送数据缓冲区
			cout << "send check";
			sendto(sClient, bufSend, strlen(bufSend), 0, (sockaddr*)&servAddr, sizeof(servAddr));
			send_time = clock();
			
		//}
	}
}
int main(int argc, char* argv[])
{
	const int		BUF_SIZE = 64;
	WSADATA			wsd;				//WSADATA变量
	char			bufSend[BUF_SIZE];	//发送数据缓冲区
	char			bufRecv[BUF_SIZE];  //接收数据缓冲区
	int				retVal;				//返回值

	char* closeSymbol = (char*)"00";//结束通信的标志
									  // 设置服务端地址
	servAddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", (void*)&servAddr.sin_addr.S_un.S_addr);
	servAddr.sin_port = htons((short)4999);
	int	nServAddlen = sizeof(servAddr);
	int pid = (int)_getpid();
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
		cout << "本次过程中的PID值为" <<pid <<endl;
		cout << "若发送的数据是'0'，则表示想结束此次UDP通信\n";
		cout << "Client UDP Socket init !" << endl;
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
		cout << "Data send to Server UDP Socket: ";
		char a[10];
		cin >> bufSend >> a;
		strcat_s(bufSend, a);
		cout << bufSend;
		if (!inputcheck(bufSend)) {
			ZeroMemory(bufSend, BUF_SIZE);
			// 向服务端发送数据buf
			cout << "Data send to Server UDP Socket: ";
			char a[10];
			cin >> bufSend >> a;
			strcat_s(bufSend, a);
			cout << bufSend;
		}
		retVal = sendto(sClient, bufSend, strlen(bufSend), 0, (sockaddr*)&servAddr, nServAddlen);
		if (SOCKET_ERROR == retVal)
		{
			cout << "send failed!" << endl;
			closesocket(sClient);	//关闭服务端套接字			
			WSACleanup();		    //释放套接字资源
			return -1;
		}
		if (!strcmp(bufSend, closeSymbol))
		{
			cout << "Client UDP Socket wants to finish this communication" << endl;
			break;
		}

		// 从服务端接收数据bufRecv
		retVal = recvfrom(sClient, bufRecv, BUF_SIZE, 0, (sockaddr*)&servAddr, &nServAddlen);
		bufRecv[retVal] = '\0';
		char str[INET_ADDRSTRLEN];
		cout << "Server IP：" << inet_ntop(AF_INET, &servAddr.sin_addr, str, sizeof(str)) << endl;
		cout << "Data recv from Server UDP Socket: " << bufRecv << endl;

		/*成功获得确认*/
		if (!strncmp(bufRecv, "TICK", COMMAND_LENGTH))    
		{
			cout << "GOT TICK\n";
			gottenkey = true;
			char* ticketid;
			ticketid = &bufRecv[COMMAND_LENGTH];

			thread t(sendcheck);
			cout << "sendcheck thread\n";
			t.detach();     //该线程可以放飞自我,与主线程并发进行
		}
		//服务器拒绝请求
		else if (!strncmp(bufRecv, "FAIL", COMMAND_LENGTH)) {
			cout << "GOT FAIL\n";

		}
		else if (!strncmp(bufRecv, "THNX", COMMAND_LENGTH)) {
			cout << "GOT THANK\n";
			gottenkey = false;

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

bool inputcheck(char * bufSend) {
	bool iscorrect = true;
	if (gottenkey == true && !strncmp(bufSend, "HELO", COMMAND_LENGTH)) {
		cout << "此时你已经拥有了key，请勿重复申请" << endl;
		iscorrect = false;
	}
	else if (strncmp(bufSend, "HELO", COMMAND_LENGTH) && strncmp(bufSend, "LISC", COMMAND_LENGTH) && strncmp(bufSend, "GBYE", COMMAND_LENGTH)) {
		cout << "WRONG INPUT" << endl;
		iscorrect = false;
	}
	return iscorrect;
}
