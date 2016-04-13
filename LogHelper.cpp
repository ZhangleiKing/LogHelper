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
	//�����ó�Ա����
	m_iMaxSingleLogFileSize = MAX_SINGLE_LOG_FILE_SIZE;
	m_iLogRecordTimeInterval = LOG_RECORD_TIME_INTERVAL;
	m_pLogSavedPath = LOG_SAVED_PATH;

	m_tLogBeginTime = time(NULL);
	m_iCurrentLogFileSuffix = 1;	

	//��ʼ���ؼ��νṹ��ͬʱʹ����ת��
	if (InitializeCriticalSectionAndSpinCount(&m_cCriticalSection, 4000) != TRUE)
	{
		printf("��ʼ���ٽ���ʧ�ܣ�\n");
	}

	//���������ڴ棬��ΪLog���棬�����г�ʼ��
	m_iBufferUsedSize = 0;
	m_pBuffer = (char*)malloc(MAX_BUFFER_SIZE);
	memset(m_pBuffer, 0, MAX_BUFFER_SIZE);

	//�����ʼ������Log�ļ�
	m_pCurrentLogFileFullPath = NULL;
	m_bCurrentLogFileIsOpen = false;
	m_pCurrentLogFile = NULL;
	CreateLogFile(GetLogFileName());

	//�½��̣߳��������ö�ʱ��������ˢ��buffer
	//m_hFlushBufferThread = (HANDLE)_beginthreadex(NULL, 0, FlushBufferThreadFunc, NULL, 0, 0);
	//FlushBufferThreadFunc();
}

void CLogHelper::Uinit()
{
	//�ͷŹؼ���
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
			//���ʱ����ļ���С�����ϣ���
			//1.��ǰ�ļ�д��
			//2.д���ر��ļ�
			if (content == NULL)
				printf("WriteIntoLogFile: content is null\n");
			fwrite(content, strlen(content), 1, m_pCurrentLogFile);
			CloseCurrentLogFile();
		}
		else
		{
			//���ʱ�䷶Χ��������
			//1.�رյ�ǰ�ļ�
			//2.�½��ļ�
			//3.�����ļ�д������
			//4.д���ر��ļ�
			CloseCurrentLogFile();
			CreateLogFile(GetLogFileName());
			fwrite(content, strlen(content), 1, m_pCurrentLogFile);
			CloseCurrentLogFile();
		}
	}
	else
	{
			//����ļ���Χ��������
			//1.�رյ�ǰ�ļ�
			//2.m_iCurrentLogFileSuffix����
			//3.�½��ļ�
			//4.�����ļ�д������
			//5.д���ر��ļ�
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
	
	//�жϱ�׼���£�
	//1.�����ж���ʼ��¼ʱ���뵱ǰʱ���Ƿ���ͬһ��
	//2.�����ͬһ�꣬�����á������ǵ���ĵڶ�����[0, 365]�����ж��������ڵļ��
	//3.�������ͬһ�꣬����㡰ȥ��ĵڶ����족�롰����ĵڶ����족
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
		WN_INT32 iBeginTimeYear = tBeginTime.tm_year;//��ʼ��¼���ڵ���ݣ��ж��Ƿ�Ϊ����
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
	//Buffer�Ѿ�ʹ�õĿռ�size������д��log�ļ�������size����log�ļ���ǰsize֮��������ڹ涨������ļ�Size������Ҫ�½�log�ļ�
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
	endDate = GetNewDateOffDays(m_tLogBeginTime, LOG_RECORD_TIME_INTERVAL - 1);//���ǵ�����Ҳ����1�죬������Ҫ�ڼ���Ļ����ϼ�һ
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