#include "stdafx.h"

#include "LogHelper.h"
#include "common_function.h"
#include <string.h>
#include <malloc.h>
#include <share.h>
#include <time.h>
#include <cerrno>

CLogHelper* CLogHelper::m_pLogInstance = NULL;

CLogHelper::CLogHelper()
{
	Init();
}

CLogHelper::~CLogHelper()
{

}

CLogHelper* CLogHelper::GetInstance()
{
	if (m_pLogInstance == NULL)
	{
		m_pLogInstance = new CLogHelper();
	}
	return m_pLogInstance;
}

void CLogHelper::Init()
{
	//可配置成员变量
	m_iMaxSingleLogFileSize = MAX_SINGLE_LOG_FILE_SIZE;
	m_iLogRecordTimeInterval = LOG_RECORD_TIME_INTERVAL;
	m_pLogSavedPath = LOG_SAVED_PATH;

	m_tLogBeginTime = time(NULL);
	m_iCurrentLogFileSuffix = 1;	

	//初始化关键段结构，同时使用旋转锁
	if (InitializeCriticalSectionAndSpinCount(&m_cCriticalSection, 4000) != TRUE)
	{
		printf("初始化临界区失败！\n");
	}

	//堆里申请内存，作为Log缓存，并进行初始化
	m_iBufferUsedSize = 0;
	m_pBuffer = (char*)malloc(MAX_BUFFER_SIZE);
	memset(m_pBuffer, 0, MAX_BUFFER_SIZE);

	//程序初始化创建Log文件
	m_pCurrentLogFileFullPath = NULL;
	m_bCurrentLogFileIsOpen = false;
	m_pCurrentLogFile = NULL;
	CreateLogFile(GetLogFileName());

	//新建线程，用于设置定时器，定期刷新buffer
	//m_hFlushBufferThread = (HANDLE)_beginthreadex(NULL, 0, FlushBufferThreadFunc, NULL, 0, 0);
	//FlushBufferThreadFunc();
}

void CLogHelper::Uinit()
{
	//释放关键段
	DeleteCriticalSection(&m_cCriticalSection);
}

WN_INT32 CLogHelper::WriteLog(WN_INT32 iLogLevel, char* pLogStr)
{
	WN_INT32 iReturnResult = 1;

	switch (iLogLevel)
	{
	case WN_INFO:
		EnterCriticalSection(&m_cCriticalSection);
		OperationAsType("INFO", pLogStr);
		LeaveCriticalSection(&m_cCriticalSection);
		break;
	case WN_ERROR:
		EnterCriticalSection(&m_cCriticalSection);
		OperationAsType("**ERROR**", pLogStr);
		LeaveCriticalSection(&m_cCriticalSection);
		break;
	case WN_USER_OPERATE:
		printf("USER_OPERATE\n");
		break;
	default:
		printf("wrong type\n");
		iReturnResult = 0;
		break;
	}

	return iReturnResult;
}

void CLogHelper::OperationAsType(char* typeSignal, char* pLogContent)
{
	char* pLogContentPrefix = NULL;
	char* pFullLogContent = NULL;
	char* strLogDateTimeStamp = NULL;

	strLogDateTimeStamp = GetLogDateTimeStamp();
	pLogContentPrefix = CharPointerLink(typeSignal, strLogDateTimeStamp, false);
	pFullLogContent = CharPointerLink(pLogContentPrefix, pLogContent, true);

	if (CheckBufferCapability(pFullLogContent))
	{
		errno_t error = strcat_s(m_pBuffer, MAX_BUFFER_SIZE, pFullLogContent);
		if (error == 0)
		{
			m_iBufferUsedSize += strlen(pFullLogContent);
		}
		else if (error == EINVAL || error == ERANGE)
		{
			//todo
			printf("%s: source null or too small\n", typeSignal);
		}
	}
	else
	{
		WriteIntoLogFile(m_pBuffer);
		ClearBuffer();
		errno_t error = strcat_s(m_pBuffer, MAX_BUFFER_SIZE, pFullLogContent);
		if (error == 0)
		{
			m_iBufferUsedSize += strlen(pFullLogContent);
		}
		else if (error == EINVAL || error == ERANGE)
		{
			//todo
			printf("%s: source null or too small\n", typeSignal);
		}
	}
}

WN_INT32 CLogHelper::WriteIntoLogFile(char* content)
{
	if ( !m_bCurrentLogFileIsOpen )
	{
		OpenCurrentLogFile(m_pCurrentLogFileFullPath);
	}

	if( !CheckSizeOut() )
	{
		if( !CheckTimeOut() )
		{
			//如果时间和文件大小都符合，则：
			//1.向当前文件写入
			//2.写完后关闭文件
			if (content == NULL)
				printf("WriteIntoLogFile: content is null\n");
			fwrite(content, strlen(content), 1, m_pCurrentLogFile);
			CloseCurrentLogFile();
		}
		else
		{
			//如果时间范围超出，则：
			//1.关闭当前文件
			//2.新建文件
			//3.向新文件写入内容
			//4.写完后关闭文件
			CloseCurrentLogFile();
			CreateLogFile(GetLogFileName());
			fwrite(content, strlen(content), 1, m_pCurrentLogFile);
			CloseCurrentLogFile();
		}
	}
	else
	{
			//如果文件范围超出，则：
			//1.关闭当前文件
			//2.m_iCurrentLogFileSuffix自增
			//3.新建文件
			//4.向新文件写入内容
			//5.写完后关闭文件
			CloseCurrentLogFile();
			m_iCurrentLogFileSuffix++;
			CreateLogFile(GetLogFileName());
			fwrite(content, strlen(content), 1, m_pCurrentLogFile);
			CloseCurrentLogFile();
	}
	
	return 0;
}

bool CLogHelper::CheckBufferCapability(char* pContent)
{
	bool bResult = true;
	int iLength = strlen(pContent);
	if ((iLength + m_iBufferUsedSize) >= MAX_BUFFER_SIZE)
		bResult = false;
	
	return bResult;
}

bool CLogHelper::CheckTimeOut()
{
	bool bResult = false;
	
	//判断标准如下：
	//1.首先判断起始记录时间与当前时间是否在同一年
	//2.如果是同一年，则利用“日期是当年的第多少天[0, 365]”来判断两个日期的间隔
	//3.如果不是同一年，则计算“去年的第多少天”与“今年的第多少天”
	time_t currentTime = time(NULL);
	struct tm tCurrTime = { 0 };
	localtime_s(&tCurrTime, &currentTime);

	struct tm tBeginTime = { 0 };
    localtime_s(&tBeginTime, &m_tLogBeginTime);

	if(tCurrTime.tm_year == tBeginTime.tm_year)
	{
		if((tCurrTime.tm_yday - tBeginTime.tm_yday + 1) > m_iLogRecordTimeInterval)
		{
			bResult = true;
		}
	}
	else
	{
		WN_INT32 iBeginTimeYear = tBeginTime.tm_year;//起始记录日期的年份，判断是否为闰年
		if(iBeginTimeYear%4==0 && iBeginTimeYear%100!=0 || iBeginTimeYear%400==0) 
		{
			if((365 - tBeginTime.tm_yday +1) + (tCurrTime.tm_yday + 1) > m_iLogRecordTimeInterval)
			{
				bResult = true;
			}
		}
		else
		{
			if((364 - tBeginTime.tm_yday +1) + (tCurrTime.tm_yday + 1) > m_iLogRecordTimeInterval)
			{
				bResult = true;
			}
		}
	}

	return bResult;

}

bool CLogHelper::CheckSizeOut()
{
	bool bResult = false;
	WN_INT32 iFileSize = GetLogFileSize();
	//Buffer已经使用的空间size（即将写入log文件的内容size）与log文件当前size之和如果大于规定的最大文件Size，则需要新建log文件
	if((iFileSize + m_iBufferUsedSize) > m_iMaxSingleLogFileSize)
	{
		bResult = true;
	}

	return bResult;
}

char* CLogHelper::GetLogFileName()
{
	char* beginDate = NULL;
	char* endDate = NULL;
	char* fileName = NULL;

	beginDate = ChangeTimeType(m_tLogBeginTime);
	endDate = GetNewDateOffDays(m_tLogBeginTime, LOG_RECORD_TIME_INTERVAL - 1);//考虑到当日也算作1天，所以需要在间隔的基础上减一
	fileName = (char*)malloc(25);

	sprintf_s(fileName, 25, "%s_%s_%02d.txt", beginDate, endDate, m_iCurrentLogFileSuffix);

	return fileName;
}

WN_INT32 CLogHelper::CreateLogFile(char* pLogFileName)
{
	m_pCurrentLogFileFullPath = CharPointerLink(m_pLogSavedPath, pLogFileName, false);
	try{
		m_pCurrentLogFile = _fsopen(m_pCurrentLogFileFullPath, "a+", _SH_DENYNO);
		if (m_pCurrentLogFile == NULL)
		{
			//todo
			printf("CreateLogFile: Create file failure\n");
		}
		else
		{
			m_bCurrentLogFileIsOpen = true;
		}
	}
	catch (...){
		return 0;
	}

	return 1;
}

WN_INT32 CLogHelper::OpenCurrentLogFile(char* pLogFilePath)
{
	try{
		m_pCurrentLogFile = _fsopen(pLogFilePath, "a+", _SH_DENYNO);
		if (m_pCurrentLogFile == NULL)
		{
			//todo
			printf("OpenLogFile: open file failure\n");
		}
		else
		{
			m_bCurrentLogFileIsOpen = true;
		}
	}
	catch (...){
		return 0;
	}
	return 1;
}

void CLogHelper::CloseCurrentLogFile()
{
	fclose(m_pCurrentLogFile);
	m_pCurrentLogFile = NULL;
	m_bCurrentLogFileIsOpen = false;
}


void CLogHelper::ClearBuffer()
{
	memset(m_pBuffer, 0, sizeof(m_pBuffer));
	m_iBufferUsedSize = 0;
}

void CLogHelper::FlushBufferThreadFunc()
{
	/*SetTimer(0, 0, 30 * 1000, &TimerProc);
	MSG  msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_TIMER)
		{
			DispatchMessage(&msg);
		}
	}*/
	while (1)
	{
		EnterCriticalSection(&m_cCriticalSection);
		WriteIntoLogFile(m_pBuffer);
		ClearBuffer();
		LeaveCriticalSection(&m_cCriticalSection);
		Sleep(30 * 1000);
	}
}

//void CALLBACK CLogHelper::TimerProc(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime)
//{
//	EnterCriticalSection(&m_cCriticalSection);
//	WriteIntoLogFile(m_pBuffer);
//	ClearBuffer();
//	LeaveCriticalSection(&m_cCriticalSection);
//}


//void CLogHelper::FlushBuffer()
//{
//	EnterCriticalSection(&m_cCriticalSection);
//	WriteIntoLogFile(m_pBuffer);
//	ClearBuffer();
//	LeaveCriticalSection(&m_cCriticalSection);
//}

WN_INT32 CLogHelper::GetLogFileSize()
{
	int size = 0;
	
	fseek(m_pCurrentLogFile, 0, SEEK_END);
	size = ftell(m_pCurrentLogFile);
	
	return size;
}