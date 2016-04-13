// TestLog.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "LogHelper.h"
#include <stdlib.h>
#include <string.h>

CLogHelper* logHelper;

UINT _stdcall FirstThread(PVOID pvParam)
{
	for (int i = 0; i<3000; i++)
	{	
		logHelper->WriteLog(0, "common info 1!");
		printf("In first thread...., current i is: %d\n", i);
	}
	return 0;
}

UINT _stdcall SecondThread(PVOID pvParam)
{
	for (int i = 0; i<3000; i++)
	{
		logHelper->WriteLog(1, "wrong error!");
		printf("In second thread...., current i is: %d\n", i);
	}

	return 0;
}

UINT _stdcall ThirdThread(PVOID pvParam)
{
	/*for (int i = 0; i<3000; i++)
	{
		logHelper->WriteLog(0, "common info 3!");
		printf("In third thread...., current i is: %d\n", i);
	}*/

	logHelper->FlushBufferThreadFunc();

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	logHelper = CLogHelper::GetInstance();

	HANDLE hThread1 = (HANDLE)_beginthreadex(NULL, 0, FirstThread, NULL, 0, 0);
	HANDLE hThread2 = (HANDLE)_beginthreadex(NULL, 0, SecondThread, NULL, 0, 0);
	HANDLE hThread3 = (HANDLE)_beginthreadex(NULL, 0, ThirdThread, NULL, 0, 0);

	if (NULL != hThread1)
		CloseHandle(hThread1);
	if (NULL != hThread2)
		CloseHandle(hThread2);
	if (NULL != hThread3)
		CloseHandle(hThread3);
	
	/*clock_t  t_begin, t_end;
	double total_time;
	t_begin = clock();
	t_end = clock();
	total_time = (double)(t_end - t_begin) / CLOCKS_PER_SEC;*/

	system("pause");
	return 0;
}



