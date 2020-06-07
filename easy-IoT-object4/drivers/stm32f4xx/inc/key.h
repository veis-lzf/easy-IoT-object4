#ifndef __DEV_KEY_H
#define	__DEV_KEY_H

#include "stdbool.h"
#include "type.h"

typedef int  (*key_recv_fn)(int arg);

enum _KEY_TYPE{
	KEY_USR,
    KEY2,
	KEY_MAX
};

struct usr_ctl_key{
	bool key_stat[KEY_MAX];
    key_recv_fn  rev;
	int press_time_cnt[KEY_MAX];
	uint32_t last_detect_time[KEY_MAX];
};

void init_key_dev(void);

#endif
