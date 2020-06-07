#ifndef _MSG_Q_H_
#define _MSG_Q_H_
#include "api.h"
#ifdef OS_UCOS
#ifdef UCOS_V3
typedef OS_Q *msg_q_t;
#else
typedef OS_EVENT *msg_q_t;
#endif
#endif
#ifdef OS_FREE_RTOS
typedef void *msg_q_t;
#endif
int msgget(msg_q_t *msg_q, int q_size);
int msgsnd(msg_q_t msgid, void *msgbuf);
int msgrcv(msg_q_t msgid, void **msgbuf, unsigned int timeout);
int msgfree(msg_q_t msg_q);
void msg_q_init(void);

#endif
