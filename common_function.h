#pragma once
#ifndef COMMON_FUNCTION_H
#define COMMON_FUNCTION_H


#define DAYSEC 3600*24

//@func: 将两个char* 字符串连接，结果传入pReturnResult
//@param: part_x(第一个字符串) 
//@param: part_y(第二个字符串)
//@param: end_with_Lf(false:表示不以\n结束;true:末尾添上换行符)
char* CharPointerLink(char* part_x, char* part_y, bool end_with_Lf);

char* GetCurrentStringDate();

//@func: 获取写入Log文件的时间戳(格式为"[20160313 10:02:05]---")
char* GetLogDateTimeStamp(); 

void GetCurrentStringDateTime(char* strDateTime);

//@func: 用于日期计算(返回的日期格式为"2016-03-13")
//@param: tBeginTime(初始日期)
//@param: off(间隔天数，可以为负)
char* GetNewDateOffDays(time_t tBeginTime, int off);

//@func: 将time_t类型的时间转化为char*类型(返回的日期格式为"2016-03-13")
//@param: date(需要转化的时间)
char* ChangeTimeType(time_t date);


#endif