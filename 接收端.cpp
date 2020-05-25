//这里是服务端！！
//336行需要修改 其他好像还能工作
//74-76处理线程
#include "winsock2.h"
#include <WS2tcpip.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string.h>
#include <time.h>
#include <ctime>
#include <thread>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)
using namespace std;
const int		BUF_SIZE = 64;
const int		COMMAND_LENGTH = 4;
WSADATA			wsd;			    //WSADATA变量
SOCKET			sServer;		    //服务器套接字
SOCKET			sClient;		    //客户端套接字
SOCKADDR_IN		servAddr;		    //服务器地址
SOCKADDR_IN     clientAddr;         //客户端地址
int				nAddrLen = sizeof(clientAddr);
char			bufSend[BUF_SIZE];	//发送数据缓冲区
char			bufRecv[BUF_SIZE];  //接收数据缓冲区
int				retVal;			    //返回值
char* closeSymbol = (char*)"00";  //结束通信的标志
									// 服务端套接字地址 


void writefile(const char* , map<string, string> );
void autocheck();

int main(int argc, char* argv[])
{
	servAddr.sin_family = AF_INET;        //协议
	servAddr.sin_port = htons(4999);      //端口
	inet_pton(AF_INET, "127.0.0.1", (void*)&servAddr.sin_addr.S_un.S_addr);

	// 初始化套接字动态库	
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		cout << "WSAStartup failed !" << endl;
		return 1;
	}
	// 创建服务端套接字
	sServer = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == sServer)
	{
		cout << "socket failed!" << endl;
		WSACleanup();			 //释放套接字资源;
		return  -1;
	}
	else
	{
		cout << "若发送的数据是'0'，则表示想结束此次UDP通信\n";
		cout << "Server UDP Socket init!" << endl;
		cout << "Server UDP Socket IP: 127.0.0.1" << endl;
		cout << "Server UDP Socket Port: 4999" << endl;
	}
	// 套接字绑定IP和端口
	if (SOCKET_ERROR == bind(sServer, (LPSOCKADDR)&servAddr, sizeof(SOCKADDR_IN)))
	{
		cout << "bind failed!" << endl;
		closesocket(sServer);	//关闭服务端套接字
		WSACleanup();			//释放套接字资源;
		return -1;
	}
	else
	{
		cout << "Server UDP Socket bind IP & Port !" << endl;
	}
	// 循环等待其他端口发送数据，从客户端接收数据 & 向客户端发送数据
	while (true) 
	{
		thread t(autocheck);
		//cout << "sendcheck thread\n";
		t.detach();     //该线程可以放飞自我,与主线程并发进行
		bool			sendFlag;			//自动发送
		map<string, string> clientkey;		//存取钥匙csv文件
		map<string, string> liscpid;		//存取csv文件
		string liscfilename;
		string tmpStr;
		ifstream inFile("total.csv", ios::in);
		while (getline(inFile, tmpStr))
		{
			// 分割字符串
			int index = tmpStr.find(",");
			string tmppid = tmpStr.substr(0, index);
			string tmplisc = tmpStr.substr(index + 1, tmpStr.size() - 1);
			// 存入map
			liscpid[tmppid] = tmplisc;
		}
		inFile.close();

		sendFlag = true;
		// 初始化缓冲空间
		ZeroMemory(bufRecv, BUF_SIZE);
		// 接收客户端发送的buf信息
		retVal = recvfrom(sServer, bufRecv, BUF_SIZE, 0, (sockaddr*)&clientAddr, &nAddrLen);
		if (SOCKET_ERROR == retVal)
		{// 接收失败则关闭服务端客户端套接字
			cout << "Recv Failed!" << endl;
			closesocket(sServer);	//关闭服务端套接字
			WSACleanup();			//释放套接字资源;
			return -1;
		}
		

		// 确认客户端发送的信息

		bufRecv[retVal] = '\0';			// 接收的最后一位设为\0，避免烫烫的问题
		char str[INET_ADDRSTRLEN];
		cout << "Client IP：" << inet_ntop(AF_INET, &clientAddr.sin_addr, str, sizeof(str)) << endl;
		cout << "Data recv from Client UDP Socket: " << bufRecv << endl;

		const char* pid="0";
		if (!strncmp(bufRecv, "HELO", COMMAND_LENGTH))
		{
			cout << "GOT HEL0\n";
			
			pid = &bufRecv[COMMAND_LENGTH];
			string liscname;
			//得到pid以后去寻找total.csv里面对应的lisc
			if (liscpid.count(pid) > 0)
			{
				liscname = liscpid[pid];
				const char* license = liscname.c_str();
				//打开对应的LISC文件
				const char* autoname = ".csv";
				char temp[40];
				strcpy_s(temp, license);
				strcat_s(temp, autoname);
				liscfilename = temp;
				ifstream inFile(liscfilename, ios::in);
				string lineStr;

				while (getline(inFile, lineStr))
				{
					// 分割字符串
					int index = lineStr.find(",");
					string tick = lineStr.substr(0, index);
					string pid = lineStr.substr(index + 1, lineStr.size() - 1);
					// 存入map
					clientkey[tick] = pid;
				}
				inFile.close();

				int pidlength = retVal - COMMAND_LENGTH;
				//cout << pid<< endl;
				bool keyflag = false;//分配钥匙是否成功
				for (map<string, string>::iterator iter = clientkey.begin(); iter != clientkey.end(); iter++)
				{
					const char* tmp = iter->second.c_str();
					if (!strcmp(tmp, "0"))
					{
						iter->second = pid;
						//cout << iter->second;

						const char* a = iter->first.c_str();
						char autoSend[10] = "TICK";
						strcat_s(autoSend, a);
						cout << "Data send to Client UDP Socket: " << autoSend;
						sendto(sServer, autoSend, strlen(autoSend), 0, (sockaddr*)&clientAddr, nAddrLen);
						keyflag = true;
						break;
					}
				}
				if (!keyflag)
				{
					const char* a = "no tickets";
					char autoSend[40] = "FAIL";
					strcat_s(autoSend, a);
					cout << "Data send to Client UDP Socket: " << autoSend;
					sendto(sServer, autoSend, strlen(autoSend), 0, (sockaddr*)&clientAddr, nAddrLen);
				}
				else
				{
					const char* tmp = liscfilename.c_str();
					writefile(tmp, clientkey);
				}
				sendFlag = false;
			}
			else
			{
				const char* a = " IS INVALID";
				char autoSend[50] = "YOUR ID";
				strcat_s(autoSend, a);
				cout << "Data send to Client UDP Socket: " << autoSend;
				sendto(sServer, autoSend, strlen(autoSend), 0, (sockaddr*)&clientAddr, nAddrLen);
				sendFlag = false;
			}
		}
		else if (!strncmp(bufRecv, "GBYE", COMMAND_LENGTH))
		{
			cout << "GOT GBYE\n";
			pid = &bufRecv[COMMAND_LENGTH];
			//得到pid以后要去寻找total.csv里面对应的lisc
			if (liscpid.count(pid) > 0)
			{
				string liscname = liscpid[pid];
				cout << liscname;
				const char* license = liscname.c_str();
				//打开存储文件1111111111.csv
				const char* autoname = ".csv";
				char temp[40];
				strcpy_s(temp, license);
				strcat_s(temp, autoname);
				liscfilename = temp;
				ifstream inFile(liscfilename, ios::in);
				string lineStr;

				while (getline(inFile, lineStr))
				{
					// 分割字符串
					int index = lineStr.find(",");
					string tick = lineStr.substr(0, index);
					string pid = lineStr.substr(index + 1, lineStr.size() - 1);
					// 存入map
					clientkey[tick] = pid;
				}
				inFile.close();
				bool findflag=false;
				for (std::map<string, string>::iterator it = clientkey.begin(); it != clientkey.end(); it++)
				{
					if (!strcmp(it->second.c_str(), pid))
					{
						findflag = true;
						it->second = "0";
						break;
					}
				}
				//成功返还钥匙
				if (findflag)
				{
					const char* a = "YOU";
					char autoSend[10] = "THNX";
					strcat_s(autoSend, a);
					cout << "Data send to Client UDP Socket: " << autoSend;
					sendto(sServer, autoSend, strlen(autoSend), 0, (sockaddr*)&clientAddr, nAddrLen);

					const char* wtmp = liscfilename.c_str();
					writefile(wtmp, clientkey);
				}
				else
				{
					const char* a = " ";
					char autoSend[10] = "FAIL";
					strcat_s(autoSend, a);
					cout << "Data send to Client UDP Socket: " << autoSend;
					sendto(sServer, autoSend, strlen(autoSend), 0, (sockaddr*)&clientAddr, nAddrLen);
				}
				sendFlag = false;
			}
			else
			{
				const char* a = " IS INVALID";
				char autoSend[50] = "YOUR ID";
				strcat_s(autoSend, a);
				cout << "Data send to Client UDP Socket: " << autoSend;
				sendto(sServer, autoSend, strlen(autoSend), 0, (sockaddr*)&clientAddr, nAddrLen);
				sendFlag = false;
			}
		}
		else if (!strncmp(bufRecv, "LISC", COMMAND_LENGTH)) {
			cout << "GOT LISC\n";
			string left = &bufRecv[COMMAND_LENGTH];
			int index = left.find(",");
			string lisc = left.substr(0, index);
			string pid = left.substr(index + 1, left.size() - 1);
			
			const char* license = lisc.c_str();
			const char* autoname = ".csv";
			char temp[40];
			strcpy_s(temp, license);
			strcat_s(temp,autoname);
			liscfilename = temp;
			
			ifstream inFile(liscfilename, ios::in);
			const char* a;
			if (!inFile.is_open())
			{
				a = " IS INVALID";
			}
			else
			{
				a = "IS VALID";
				//将有关信息写入到total.csv
				FILE* fp = fopen("total.csv", "a+");//文件已存在 指针指到文件末尾继续写
				fprintf(fp, "%s,%s\n\0", pid.c_str(), lisc.c_str());//输出到文件
				fclose(fp);
			}
			inFile.close();
			char autoSend[50] = "YOUR LICENSE";
			strcat_s(autoSend, a);
			cout << "Data send to Client UDP Socket: " << autoSend;
			sendto(sServer, autoSend, strlen(autoSend), 0, (sockaddr*)&clientAddr, nAddrLen);
			sendFlag = false;
		}
		
		else if (!strncmp(bufRecv, "CHCK", COMMAND_LENGTH)) {
		sendFlag = false;
		cout << "GOT CHCK\n";
		
		string checkpid = &bufRecv[COMMAND_LENGTH];
		//打开存储文件check.csv并把信息存储到pidtime中
		map<string, string> pidtime;
		ifstream inFile("check.csv", ios::in);
		while (getline(inFile, tmpStr))
		{
			// 分割字符串
			int index = tmpStr.find(",");
			string _pid = tmpStr.substr(0, index);
			string _time = tmpStr.substr(index + 1, tmpStr.size() - 1);
			// 存入map
			pidtime[_pid] = _time;
		}
		inFile.close();
		
		long long now = time(0);
		ostringstream os;
		os << now;
		string result;
		istringstream is(os.str());
		is >> result;
		if (pidtime.count(checkpid) > 0)
		{
			pidtime[checkpid] = result;
		}
		else
		{
			//pidtime.insert(checkpid, pidtime[checkpid]);
			pidtime.insert(map<string, string>::value_type(checkpid, result));
		}
		//输出到文件
		writefile("check.csv", pidtime);
		}

		// 若客户端发送的数据是'0'，则表示客户端想结束此次UDP通信		
		if (!strcmp(bufRecv, closeSymbol))
		{
			cout << "Client UDP Socket wants to finish this communication" << endl;
			break;
		}
		if (sendFlag)
		{
			// 将sendBuf信息发送到客户端
			cout << "Data send to Client UDP Socket: ";
			char a[20];
			cin >> bufSend >> a;
			strcat_s(bufSend, a);
			sendto(sServer, bufSend, strlen(bufSend), 0, (sockaddr*)&clientAddr, nAddrLen);
			// 若服务端发送的数据是'0'，则表示服务端想结束此次UDP通信	
			if (!strcmp(bufSend, closeSymbol))
			{
				cout << "Server UDP Socket wants to finish this communication" << endl;
				break;
			}
		}
	}
	// 退出
	closesocket(sServer);	//关闭服务端套接字
	WSACleanup();			//释放套接字资源;
	Sleep(5000);
	return 0;
}

//写入到csv文件
void writefile(const char* myfile, map<string, string> clientkey)
{
	FILE* fp = fopen(myfile, "w");//文件已存在 清空并重写
	for (map<string, string>::iterator iter = clientkey.begin(); iter != clientkey.end(); iter++)
	{
		fprintf(fp, "%s,%s\n\0", iter->first.c_str(), iter->second.c_str());//输出map到文件
	}
	fclose(fp);
}

//用于检测
void autocheck()
{
	while (true)
	{
		Sleep(900000);//每15分钟检查一次
		map<string, string> checktime;
		string tmpStr;
		ifstream inFile("check.csv", ios::in);
		while (getline(inFile, tmpStr))
		{
			// 分割字符串
			int index = tmpStr.find(",");
			string _pid = tmpStr.substr(0, index);
			string _time = tmpStr.substr(index + 1, tmpStr.size() - 1);
			// 存入map
			checktime[_pid] = _time;
		}
		inFile.close();
		long long now = time(0);
		//pid-time
		for (map<string, string>::iterator iter = checktime.begin(); iter != checktime.end(); iter++)
		{
			//超过半小时崩溃了
			if (now - atoi(iter->second.c_str()) > 1800)
			{
				//同GBYE操作
				//得到lisc
				map<string, string> getlisc;
				ifstream inFile("total.csv", ios::in);
				while (getline(inFile, tmpStr))
				{
					// 分割字符串
					int index = tmpStr.find(",");
					string _pid = tmpStr.substr(0, index);
					string _lisc = tmpStr.substr(index + 1, tmpStr.size() - 1);
					// 存入map
					//pid-lisc
					getlisc[_pid] = _lisc;
				}
				inFile.close();
				string tmplisc;
				if (getlisc.count(iter->first) > 0)
				{
					tmplisc= getlisc[iter->first];
				}
				//得到key
				const char* license = tmplisc.c_str();
				const char* autoname = ".csv";
				char temp[40];
				strcpy_s(temp, license);
				strcat_s(temp, autoname);
				map<string, string> getkey;
				ifstream inkeyFile(temp, ios::in);
				if (inkeyFile.is_open())
				{
					while (getline(inkeyFile, tmpStr))
					{
						// 分割字符串
						int index = tmpStr.find(",");
						string _key = tmpStr.substr(0, index);
						string _pid = tmpStr.substr(index + 1, tmpStr.size() - 1);
						// 存入map
						//key-pid
						getkey[_key] = _pid;
					}
				}
				inkeyFile.close();
				FILE* fp = fopen(temp, "w");//文件已存在 清空并重写
				for (map<string, string>::iterator it = getkey.begin(); it != getkey.end(); it++)
				{
					if (!strcmp(it->second.c_str(), iter->first.c_str()))
					{
						it->second = "0";
					}
					fprintf(fp, "%s,%s\n\0", it->second.c_str(), it->first.c_str());//输出map到文件
				}
				fclose(fp);
			}
			
		}
		
	}
}

