#pragma once
#ifndef COMMON_FUNCTION_H
#define COMMON_FUNCTION_H


#define DAYSEC 3600*24

//@func: ������char* �ַ������ӣ��������pReturnResult
//@param: part_x(��һ���ַ���) 
//@param: part_y(�ڶ����ַ���)
//@param: end_with_Lf(false:��ʾ����\n����;true:ĩβ���ϻ��з�)
char* CharPointerLink(char* part_x, char* part_y, bool end_with_Lf);

char* GetCurrentStringDate();

//@func: ��ȡд��Log�ļ���ʱ���(��ʽΪ"[20160313 10:02:05]---")
char* GetLogDateTimeStamp(); 

void GetCurrentStringDateTime(char* strDateTime);

//@func: �������ڼ���(���ص����ڸ�ʽΪ"2016-03-13")
//@param: tBeginTime(��ʼ����)
//@param: off(�������������Ϊ��)
char* GetNewDateOffDays(time_t tBeginTime, int off);

//@func: ��time_t���͵�ʱ��ת��Ϊchar*����(���ص����ڸ�ʽΪ"2016-03-13")
//@param: date(��Ҫת����ʱ��)
char* ChangeTimeType(time_t date);


#endif