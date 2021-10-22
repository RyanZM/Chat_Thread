#include <iostream>
#include <process.h>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

#define MAX_CLIENT 256			//���������
SOCKET clntSocks[MAX_CLIENT];	//���ӵ�����

#define MAX_MSG 1024			//���յ�������ݳ���
int cli_count = 0;				//��ǰ������

HANDLE hMutex;

//����
void send_msg(char* msg, int len) {
	WaitForSingleObject(hMutex, INFINITE);
	for (int i = 0; i < cli_count; i++) {
		send(clntSocks[i], msg, len, 0);
	}
	ReleaseMutex(hMutex);
}

unsigned WINAPI ThreadCln(void* arg) {
	//���ղ���
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

	std::cout << "��ǰ��������" << cli_count << std::endl;

	//ĳ�����ӶϿ�����Ҫ����Ͽ������ӣ�����
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
	std::cout << "�Ͽ���ʱ�������ǣ�" << cli_count;
	ReleaseMutex(hMutex);
	closesocket(hClnSock);
	return 0;
}

int main(void) {
	//�����׽��ֿ�
	WORD wVersionRequested;
	WSAData wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);
	//��ʼ���׽��ֿ�
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return err;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return -1;
	}

	//�����׽���
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htons(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(9090);

	//���׽��ֵ�����ip��ַ���˿ں�9090
	bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));

	//��ʼ����
	listen(sockSrv, 5);

	SOCKADDR_IN addrCli;
	int	len = sizeof(SOCKADDR);

	HANDLE hThread;

	hMutex = CreateMutex(NULL, false, NULL);
	while (1) {
		//���տͻ��˵�����
		SOCKET sockconnet = accept(sockSrv, (SOCKADDR*)&addrCli, &len);

		//�������
		WaitForSingleObject(hMutex, INFINITE);
		clntSocks[cli_count++] = sockconnet;
		ReleaseMutex(hMutex);
		//�����߳�
		hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadCln, (void*)&sockconnet, 0, NULL);
		std::cout << "��ǰ��������" << cli_count << std::endl;
		std::cout << "�µ����ӵ�ַ��" << inet_ntoa(addrCli.sin_addr) << std::endl;
	}

	closesocket(sockSrv);
	WSACleanup();
	return 0;
}