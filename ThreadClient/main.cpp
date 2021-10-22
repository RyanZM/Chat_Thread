#include <iostream>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

#define MAX_NAME 256			//最大连接数


#define MAX_MSG 1024			//接收的最大内容长度

char szMsg[MAX_MSG];
char szName[MAX_NAME] = "[xxxxx]";

//发送信息的线程
unsigned WINAPI sendMsg(void* arg) {
	SOCKET sockCli = *((SOCKET*)arg);

	char send_buf[MAX_NAME + MAX_MSG] = { 0 };
	while (1)
	{
		cin >> szMsg;
		if (!strcmp(szMsg, "Q") || !strcmp(szMsg, "q")) {
			closesocket(sockCli);
			exit(0);
		}

		sprintf(send_buf, "%s %s", szName, szMsg);
		send(sockCli, send_buf, strlen(send_buf),0);
		memcpy(send_buf, "0", MAX_NAME + MAX_MSG);
	}
	return 0;
}
//接收信息线程
unsigned WINAPI RecvMsg(void* arg) {
	SOCKET sockCli = *((SOCKET*)arg);

	char recv_buf[MAX_NAME + MAX_MSG] = { 0 };
	int len = 0;

	while (1)
	{
		
		//recv阻塞
		len = recv(sockCli, recv_buf, (MAX_NAME + MAX_MSG - 1), 0);

		//服务端断开
		if (len == -1) {
			return -1;
		}
		recv_buf[len] = 0;
		cout << recv_buf << endl;
	}
	return 0;
}

int main(int argc, char* argv[]) {
	WORD wVersion;
	WSADATA wsaData;
	int err;

	wVersion = MAKEWORD(1, 1);
	err = WSAStartup(wVersion, &wsaData);
	if (err != 0) {
		return err;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return -1;
	}

	//创建套接字
	SOCKET sockCli = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(9090);

	sprintf(szName, "[%s]", argv[1]);

	//连接服务器
	if (connect(sockCli, (SOCKADDR*)&addrSrv, sizeof(addrSrv)) == SOCKET_ERROR) {
		cout << "connect error error code:" << GetLastError() << endl;
		return -1;
	}

	//发送服务端的消息，安排一个工人，起一个线程发送信息
	HANDLE sendThread;
	sendThread = (HANDLE)_beginthreadex(NULL, 0, sendMsg, (void*)&sockCli, 0, NULL);

	//接收消息给服务端，安排一个工人，起一个线程接收信息
	HANDLE recvThread;
	recvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&sockCli, 0, NULL);

	WaitForSingleObject(sendThread, INFINITE);
	WaitForSingleObject(recvThread, INFINITE);
	//关闭套接字
	closesocket(sockCli);
	WSACleanup();

	return 0;
}