#include "util.h"
int calendar[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
TIMESTAMP_STRUCT addHour(const TimeStamp& t, size_t hour, size_t min){
		TIMESTAMP_STRUCT ts;
		if(t.minute+min >= 60) {
				ts.minute = (t.minute+min)%60;
				ts.hour = t.hour + 1;
		}else{
				ts.minute = t.minute + min;
				ts.hour = t.hour;
		}
		if(ts.hour + hour >= 24) {
				ts.hour = (ts.hour + hour)%24;
				ts.day = t.day + 1;
		}else{
				ts.hour = (ts.hour + hour);
				ts.day = t.day;
		}
		if(ts.day >= calendar[t.month-1]){
				ts.day = 1;
				ts.month = t.month + 1;
		}else{
				ts.month = t.month;
		}
		if(ts.month == 13){
				ts.year = 2015;
				ts.month = 1;
		}
		return ts;
}
string toStr(const TIMESTAMP_STRUCT& ts){
		char a[22]={0};
		sprintf(a, "%d-%d-%d %d:%d:%d", ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second);
		string s(a);
		return s;
}
TIMESTAMP_STRUCT addDay(const TimeStamp& t, size_t day){
		TIMESTAMP_STRUCT ts;
		ts.year = t.year;
		ts.day = t.day + day;
		ts.hour = t.hour;
		ts.minute = t.minute;
		ts.second = t.second;
		//FIXME: assume the end date is less tha 28 days of start date
		if(ts.day >= calendar[t.month-1]){
				ts.day = ts.day % calendar[t.month-1];
				ts.month = t.month + (t.day+day)/calendar[t.month-1];
		}else{
				ts.month = t.month;
		}
		if(ts.month >= 13){
				ts.year = 2015;
				ts.month = 1;
		}
		return ts;
}

