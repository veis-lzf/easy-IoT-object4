#ifndef _MBOX_H_
#define _MBOX_H_
#include "api.h"
#ifdef OS_UCOS
#ifdef UCOS_V3
typedef OS_Q *mbox_t;
#else
typedef OS_EVENT *mbox_t;
#endif
void mbox_new(mbox_t *mbox, void *pmsg);

void *mbox_get(mbox_t *mbox, unsigned int timeout);

int mbox_post(mbox_t *mbox, void *pmsg);

int mbox_destory(mbox_t *mbox);
#endif
#endif
