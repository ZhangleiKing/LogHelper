#pragma once
#ifndef LOGHELPER_H
#define LOGHELPER_H

#include "common_function.h"
#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <time.h>

#define MAX_SINGLE_LOG_FILE_SIZE 1024*60
#define MAX_BUFFER_SIZE 1024
#define LOG_RECORD_TIME_INTERVAL 2
#define LOG_SAVED_PATH "D:\\"

enum LogLevel{
	WN_INFO = 0,
	WN_ERROR,
	WN_USER_OPERATE,
};

class CLogHelper{

private:
	CLogHelper();

	~CLogHelper();

	void Init();

	void Uinit();

public:
	static CLogHelper* GetInstance();

public:
	//@func: ����־����д���ڴ�buffer
	//@param: iLogLevel(��־����)
	//@param: pLogContent(��־����)
	WN_INT32 WriteLog(WN_INT32 iLogLevel, char* pLogContent);

	inline void SetMaxSingleLogFileSize(WN_INT32 size){ m_iMaxSingleLogFileSize = size; }

	inline void SetLogRecordTimeInterval(WN_INT32 time_interval){ m_iLogRecordTimeInterval = time_interval; }

	inline void SetLogSavedPath(char* path){ m_pLogSavedPath = path; }

	void FlushBufferThreadFunc();

private:

	//@func: ��buffer�е�����д��file��
	//@param: pLogContent(д�������)
	WN_INT32 WriteIntoLogFile(char* pLogContent);

	void OperationAsType(char* typeSignal, char* pLogContent);

	//@func: ���buffer�Ƿ����㹻�Ŀռ�д��pLogContent
	//@return: true,�㹻��false,����
	bool CheckBufferCapability(char* pLogContent);

	//@func: ��鵱ǰʱ���Ƿ�����־�ļ���Χ�ڣ���ȷ���Ƿ���Ҫ�½��ļ�
	//@return: true,������Χ��false,�ڷ�Χ��
	bool CheckTimeOut();

	//@func: ��鵱ǰ��־�ļ��Ĵ�С�Ƿ񳬹����巶Χ����ȷ���Ƿ���Ҫ�½��ļ�
	//@return: true,������Χ��false,�ڷ�Χ��
	bool CheckSizeOut();

	//ÿ�δ������ļ�ʱ����Ҫ��ȡ��־�ļ�����
	char* GetLogFileName();

	//@func: �����µ�Log�ļ�
	//@param: pLogFileName(��Ҫ�������ļ�����)
	WN_INT32 CreateLogFile(char* pLogFileName);

	//@func: �򿪵�ǰ��Ҫ������Log�ļ�
	//@param: pLogFilePath(��Ҫ�������ļ�ȫ·���������ļ���)
	WN_INT32 OpenCurrentLogFile(char* pLogFilePath);

	//@func: �رյ�ǰ������Log�ļ�
	void CloseCurrentLogFile();

	//@func: ���buffer
	void ClearBuffer();

	//void FlushBuffer();

	//@func: SetTimer�Ļص�����������ˢ��buffer���Ƚ�buffer����д���ļ�������գ�
	//void CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime);

	int GetLogFileSize();

private:
	//����ģʽ
	static CLogHelper* m_pLogInstance;

	//�ٽ���������дbufferʱ����
	CRITICAL_SECTION m_cCriticalSection;

	//ˢ��buffer���߳̾��
	//HANDLE m_hFlushBufferThread;
private:
	//����Log�ļ�����size
	WN_INT32 m_iMaxSingleLogFileSize;

	//ÿ��n������һ��Log�ļ�
	WN_INT32 m_iLogRecordTimeInterval;

	//Log�ļ��洢·��
	char* m_pLogSavedPath;

	//��ǰ�����Log�ļ���׺����_1,_2��
	WN_INT32 m_iCurrentLogFileSuffix;

	//��ǰд��Log���ļ�
	FILE* m_pCurrentLogFile;

	//��ǰд��Log���ļ���ȫ·���������ļ�����
	char* m_pCurrentLogFileFullPath;

	//��ǰд��Log���ļ���״̬
	bool m_bCurrentLogFileIsOpen;

	//�����Buffer,д��Log,֮����д���ļ�
	char* m_pBuffer;

	//��ǰbuffer�Ѿ�ʹ�õĿռ�size
	WN_INT32 m_iBufferUsedSize;

	//��ǰд���log�ļ���ʼ����ʱ��
	time_t m_tLogBeginTime;

};

#endif