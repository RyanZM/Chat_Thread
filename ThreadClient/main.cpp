#include <iostream>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

#define MAX_NAME 256			//���������


#define MAX_MSG 1024			//���յ�������ݳ���

char szMsg[MAX_MSG];
char szName[MAX_NAME] = "[xxxxx]";

//������Ϣ���߳�
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
//������Ϣ�߳�
unsigned WINAPI RecvMsg(void* arg) {
	SOCKET sockCli = *((SOCKET*)arg);

	char recv_buf[MAX_NAME + MAX_MSG] = { 0 };
	int len = 0;

	while (1)
	{
		
		//recv����
		len = recv(sockCli, recv_buf, (MAX_NAME + MAX_MSG - 1), 0);

		//����˶Ͽ�
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

	//�����׽���
	SOCKET sockCli = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(9090);

	sprintf(szName, "[%s]", argv[1]);

	//���ӷ�����
	if (connect(sockCli, (SOCKADDR*)&addrSrv, sizeof(addrSrv)) == SOCKET_ERROR) {
		cout << "connect error error code:" << GetLastError() << endl;
		return -1;
	}

	//���ͷ���˵���Ϣ������һ�����ˣ���һ���̷߳�����Ϣ
	HANDLE sendThread;
	sendThread = (HANDLE)_beginthreadex(NULL, 0, sendMsg, (void*)&sockCli, 0, NULL);

	//������Ϣ������ˣ�����һ�����ˣ���һ���߳̽�����Ϣ
	HANDLE recvThread;
	recvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&sockCli, 0, NULL);

	WaitForSingleObject(sendThread, INFINITE);
	WaitForSingleObject(recvThread, INFINITE);
	//�ر��׽���
	closesocket(sockCli);
	WSACleanup();

	return 0;
}