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
	//@func: 将日志内容写入内存buffer
	//@param: iLogLevel(日志类型)
	//@param: pLogContent(日志内容)
	WN_INT32 WriteLog(WN_INT32 iLogLevel, char* pLogContent);

	inline void SetMaxSingleLogFileSize(WN_INT32 size){ m_iMaxSingleLogFileSize = size; }

	inline void SetLogRecordTimeInterval(WN_INT32 time_interval){ m_iLogRecordTimeInterval = time_interval; }

	inline void SetLogSavedPath(char* path){ m_pLogSavedPath = path; }

	void FlushBufferThreadFunc();

private:

	//@func: 将buffer中的内容写入file中
	//@param: pLogContent(写入的内容)
	WN_INT32 WriteIntoLogFile(char* pLogContent);

	void OperationAsType(char* typeSignal, char* pLogContent);

	//@func: 检查buffer是否还有足够的空间写入pLogContent
	//@return: true,足够；false,不足
	bool CheckBufferCapability(char* pLogContent);

	//@func: 检查当前时间是否在日志文件范围内，以确定是否需要新建文件
	//@return: true,超出范围；false,在范围内
	bool CheckTimeOut();

	//@func: 检查当前日志文件的大小是否超过定义范围，以确定是否需要新建文件
	//@return: true,超出范围；false,在范围内
	bool CheckSizeOut();

	//每次创建新文件时，需要获取日志文件名称
	char* GetLogFileName();

	//@func: 创建新的Log文件
	//@param: pLogFileName(将要创建的文件名称)
	WN_INT32 CreateLogFile(char* pLogFileName);

	//@func: 打开当前需要操作的Log文件
	//@param: pLogFilePath(需要操作的文件全路径，包括文件名)
	WN_INT32 OpenCurrentLogFile(char* pLogFilePath);

	//@func: 关闭当前操作的Log文件
	void CloseCurrentLogFile();

	//@func: 清空buffer
	void ClearBuffer();

	//void FlushBuffer();

	//@func: SetTimer的回调函数，定期刷新buffer（先将buffer内容写入文件，再清空）
	//void CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime);

	int GetLogFileSize();

private:
	//单例模式
	static CLogHelper* m_pLogInstance;

	//临界区，用于写buffer时加锁
	CRITICAL_SECTION m_cCriticalSection;

	//刷新buffer的线程句柄
	//HANDLE m_hFlushBufferThread;
private:
	//单个Log文件最大的size
	WN_INT32 m_iMaxSingleLogFileSize;

	//每隔n天生成一个Log文件
	WN_INT32 m_iLogRecordTimeInterval;

	//Log文件存储路径
	char* m_pLogSavedPath;

	//当前保存的Log文件后缀，如_1,_2等
	WN_INT32 m_iCurrentLogFileSuffix;

	//当前写入Log的文件
	FILE* m_pCurrentLogFile;

	//当前写入Log的文件的全路径（包括文件名）
	char* m_pCurrentLogFileFullPath;

	//当前写入Log的文件打开状态
	bool m_bCurrentLogFileIsOpen;

	//申请的Buffer,写入Log,之后再写入文件
	char* m_pBuffer;

	//当前buffer已经使用的空间size
	WN_INT32 m_iBufferUsedSize;

	//当前写入的log文件起始创建时间
	time_t m_tLogBeginTime;

};

#endif