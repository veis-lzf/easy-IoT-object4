#ifndef _SENSERS_H
#define _SENSERS_H

struct _sensers_stat
{
	uint16_t year;
	uint8_t mon;
	uint8_t date;
	uint8_t week;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t temperature;
	uint8_t humidity;
}__packed;

struct _sensers_pkg
{
	struct mqtt_pkg_head pkg_head;
	struct _sensers_stat sensers_stat;
}__packed;

extern struct _sensers_pkg sensers_pkg;
void senser_poll_callback(void *arg);

void init_sensers(void);
void start_senser_poll(void);

#endif
