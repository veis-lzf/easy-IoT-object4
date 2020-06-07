#ifndef __RTC_DEV_H
#define	__RTC_DEV_H

int init_rtc(void);
void RTC_Time_Set(int y, int m, int d, int wd, int hour, int min, int sec);

#endif
