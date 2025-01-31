#include "sys/syscall.h"

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