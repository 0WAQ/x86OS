/**
 * 
 * 时间相关源文件
 * 
 */
#include "tools/time.h"
#include "os_cfg.h"

// 每个月的起始天数
static int month[12] = {
    DAY * (0),
	DAY * (31),
	DAY * (31+29),
	DAY * (31+29+31),
	DAY * (31+29+31+30),
	DAY * (31+29+31+30+31),
	DAY * (31+29+31+30+31+30),
	DAY * (31+29+31+30+31+30+31),
	DAY * (31+29+31+30+31+30+31+31),
	DAY * (31+29+31+30+31+30+31+31+30),
	DAY * (31+29+31+30+31+30+31+31+30+31),
	DAY * (31+29+31+30+31+30+31+31+30+31+30)
};

// TODO: change func-signature
int get_tm(struct tm* tm) {
	
    do {
		tm->tm_sec = CMOS_READ(0);
		tm->tm_min = CMOS_READ(2);
		tm->tm_hour = CMOS_READ(4);
		tm->tm_mday = CMOS_READ(7);
		tm->tm_mon = CMOS_READ(8);
		tm->tm_year = CMOS_READ(9);
	} while (tm->tm_sec != CMOS_READ(0));

	BCD_TO_BIN(tm->tm_sec);
	BCD_TO_BIN(tm->tm_min);
	BCD_TO_BIN(tm->tm_hour);
	BCD_TO_BIN(tm->tm_mday);
	BCD_TO_BIN(tm->tm_mon);
	BCD_TO_BIN(tm->tm_year);
    
    // 月份从0开始
    tm->tm_mon--;

    // 东八区
    tm->tm_hour += 8;

    // 判断世纪
    if(CMOS_READ(0x32) == 0x20) {
        tm->tm_year += 100;
    }

    return 0;
}

// TODO: debug
time_t kernel_mktime(struct tm * tm)
{
	long res;
	int year;

	year = tm->tm_year - 70;
/* magic offsets (y+1) needed to get leapyears right.*/
	res = YEAR*year + DAY*((year+1)/4);
	res += month[tm->tm_mon];
/* and (y+2) here. If it wasn't a leap-year, we have to adjust */
	if (tm->tm_mon>1 && ((year+2)%4))
		res -= DAY;
	res += DAY*(tm->tm_mday-1);
	res += HOUR*tm->tm_hour;
	res += MINUTE*tm->tm_min;
	res += tm->tm_sec;
	return res;
}

int sys_gettimeofday(struct timeval* tv, struct timezone* tz) {
	
	struct tm tm;
	get_tm(&tm);

	if(tv != NULL) {
		tv->tv_sec = kernel_mktime(&tm);
		tv->tv_usec = 0;
	}

	if(tz != NULL) {
		tz->tz_minuteswest = 0;
		tz->tz_dsttime = 0;
	}
	return 0;
}