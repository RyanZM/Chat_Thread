#include <stdio.h>
#include <Windows.h>
/*
#ifndef 0
DWORD WINAPI ThreadFun(LPVOID p) {
	int iMym = *((int*)p);
	printf("我是子线程，PID = %d,iMym = %d\n", GetCurrentThreadId(), iMym);
	return 0;
}
int main(void) {
	HANDLE hThread;
	DWORD dwThreadID;

	int m = 100;
	hThread = CreateThread(NULL, NULL, ThreadFun, &m, NULL, &dwThreadID);
	printf("我是主线程，PID = %d\n", GetCurrentThreadId());
	CloseHandle(hThread);
	Sleep(200);
	system("pause");
	return 0;
}
#endif
*/
/*
unsigned long WINAPI ThreadFun(void* arg) {
	int count;
	count = *((int*)arg);
	for (int i = 0; i < count; i++) {
		Sleep(1000);
		printf("runing ThreadFun\n");
	}
	return 0;
}

int main(void) {
	HANDLE hThread;
	unsigned long threadID;
	int param = 5;

	hThread = CreateThread(NULL, 0, ThreadFun, (void*)&param, 0, &threadID);
	if (hThread == 0) {
		printf("thread error");
		return 0;
	}

	//等待
	if (WaitForSingleObject(hThread, INFINITE) == WAIT_FAILED) {
		puts("thread wait error");
		return 0;
	}


	return 0;
}*/

#include <stdio.h>
#include <Windows.h>
#include <process.h>

#define NUM_THREAD 50

long long num = 0;

HANDLE hMutex;

unsigned WINAPI ThreadDes(LPVOID arg) {
	WaitForSingleObject(hMutex, INFINITE);
	for (int i = 0; i < 500000; i++) {
		num -= 1;
	}
	ReleaseMutex(hMutex);
	return 0;
}

unsigned WINAPI ThreadAdd(LPVOID arg) {
	WaitForSingleObject(hMutex, INFINITE);
	for (int i = 0; i < 500000; i++) {
		num += 1;
	}
	ReleaseMutex(hMutex);
	return 0;
}

int main(int argc, char* argv[]) {
	HANDLE tHreadles[NUM_THREAD];

	hMutex = CreateMutex(NULL, FALSE, NULL);
	printf("long long num: %lld\n", num);
	for (int i = 0; i < NUM_THREAD; i++) {
		if (i % 2 ) {
			tHreadles[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadAdd, NULL, NULL, NULL);
		}
		else {
			tHreadles[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadDes, NULL, NULL, NULL);
		}
	}

	WaitForMultipleObjects(NUM_THREAD, tHreadles, TRUE, INFINITE);
	
	CloseHandle(hMutex);
	printf("long long i :%lld\n", num);
	system("pause");
	return 0;
}