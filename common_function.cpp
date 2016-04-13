#include "stdafx.h"

#include "common_function.h"
#include<stdlib.h>
#include <string.h>
#include<windows.h>
#include<winbase.h>
#include<time.h>

char* CharPointerLink(char* part_x, char* part_y, bool end_with_Lf = false)
{
	char* pReturnResult = NULL;
	unsigned int iPartXLength = strlen(part_x);
	unsigned int iPartYLength = strlen(part_y);
	if (end_with_Lf)
	{
		unsigned int iSumLength = (iPartXLength + iPartYLength + 2)*sizeof(char);
		pReturnResult = (char*)malloc(iSumLength);
		memset(pReturnResult, 0, iSumLength);
		strcat_s(pReturnResult, iSumLength, part_x);
		strcat_s(pReturnResult, iSumLength, part_y);
		strcat_s(pReturnResult, iSumLength, "\n");
	}
	else
	{
		unsigned int iSumLength = (iPartXLength + iPartYLength + 1)*sizeof(char);
		pReturnResult = (char*)malloc(iSumLength);
		memset(pReturnResult, 0, iSumLength);
		strcat_s(pReturnResult, iSumLength, part_x);
		strcat_s(pReturnResult, iSumLength, part_y);
	}
	return pReturnResult;
}

char* GetCurrentStringDate()
{
	char* strDate = NULL;
	strDate = (char*)malloc(11);

	SYSTEMTIME stTime;
	GetLocalTime(&stTime);

	sprintf_s(strDate, 9, "%04d%02d%02d",
		stTime.wYear, stTime.wMonth, stTime.wDay);

	return strDate;
}

char* GetLogDateTimeStamp()
{
	char* strDateTime = NULL;
	strDateTime = (char*)malloc(23);

	SYSTEMTIME stTime;
	GetLocalTime(&stTime);

	//22¸ö×Ö½Ú
	sprintf_s(strDateTime, 23, "[%04d%02d%02d %02d:%02d:%02d]---",
		stTime.wYear, stTime.wMonth, stTime.wDay,
		stTime.wHour, stTime.wMinute, stTime.wSecond);

	return strDateTime;
}

void GetCurrentStringDateTime(char* strDateTime)
{
	SYSTEMTIME stTime;
	GetLocalTime(&stTime);

	sprintf_s(strDateTime, 18, "%04d%02d%02d %02d:%02d:%02d",
		stTime.wYear, stTime.wMonth, stTime.wDay,
		stTime.wHour, stTime.wMinute, stTime.wSecond);
}

char* GetNewDateOffDays(time_t tBeginTime, int off)
{
	char* result_date = NULL;
	result_date = (char*)malloc(9);

	struct tm pansctime = { 0 };
	time_t new_time = tBeginTime + off*DAYSEC;
	localtime_s(&pansctime, &new_time);

	sprintf_s(result_date, 9, "%04d%02d%02d",
		(pansctime.tm_year) + 1900, (pansctime.tm_mon) + 1, pansctime.tm_mday);

	return result_date;
}

char* ChangeTimeType(time_t date)
{
	char* result_date = NULL;
	result_date = (char*)malloc(9);

	struct tm pansctime = { 0 };
	localtime_s(&pansctime, &date);

	sprintf_s(result_date, 9, "%04d%02d%02d",
		(pansctime.tm_year) + 1900, (pansctime.tm_mon) + 1, pansctime.tm_mday);

	return result_date;
}