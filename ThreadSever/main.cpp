#include <iostream>
#include <process.h>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

#define MAX_CLIENT 256			//最大连接数
SOCKET clntSocks[MAX_CLIENT];	//连接的数组

#define MAX_MSG 1024			//接收的最大内容长度
int cli_count = 0;				//当前连接数

HANDLE hMutex;

//发送
void send_msg(char* msg, int len) {
	WaitForSingleObject(hMutex, INFINITE);
	for (int i = 0; i < cli_count; i++) {
		send(clntSocks[i], msg, len, 0);
	}
	ReleaseMutex(hMutex);
}

unsigned WINAPI ThreadCln(void* arg) {
	//接收参数
	SOCKET hClnSock = *((SOCKET*)arg);

	char szMsg[MAX_MSG] = { 0 };
	int len = 0;
	while (1) {
		//WaitForSingleObject(hMutex, INFINITE);
		memcpy(szMsg, "0",MAX_MSG);
		len = recv(hClnSock, szMsg, MAX_MSG, 0);
		if (len > 0) {
			send_msg(szMsg, len);
		}
		else {
			break;
		}
		//ReleaseMutex(hMutex);
	}

	std::cout << "当前连接数：" << cli_count << std::endl;

	//某个连接断开，需要处理断开的连接，遍历
	WaitForSingleObject(hMutex, INFINITE);
	for (int i = 0; i < cli_count; i++) {
		if(hClnSock == clntSocks[i]) {
			while (i++ < cli_count) {
				clntSocks[i] = clntSocks[i + 1];
			}
			break;
		}
	}
	cli_count--;
	std::cout << "断开此时连接数是：" << cli_count;
	ReleaseMutex(hMutex);
	closesocket(hClnSock);
	return 0;
}

int main(void) {
	//加载套接字库
	WORD wVersionRequested;
	WSAData wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);
	//初始化套接字库
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return err;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return -1;
	}

	//创建套接字
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htons(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(9090);

	//绑定套接字到本地ip地址，端口号9090
	bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));

	//开始监听
	listen(sockSrv, 5);

	SOCKADDR_IN addrCli;
	int	len = sizeof(SOCKADDR);

	HANDLE hThread;

	hMutex = CreateMutex(NULL, false, NULL);
	while (1) {
		//接收客户端的连接
		SOCKET sockconnet = accept(sockSrv, (SOCKADDR*)&addrCli, &len);

		//添加连接
		WaitForSingleObject(hMutex, INFINITE);
		clntSocks[cli_count++] = sockconnet;
		ReleaseMutex(hMutex);
		//创建线程
		hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadCln, (void*)&sockconnet, 0, NULL);
		std::cout << "当前连接数：" << cli_count << std::endl;
		std::cout << "新的连接地址：" << inet_ntoa(addrCli.sin_addr) << std::endl;
	}

	closesocket(sockSrv);
	WSACleanup();
	return 0;
}