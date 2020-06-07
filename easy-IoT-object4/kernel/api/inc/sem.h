#ifndef _SEM_H_
#define _SEM_H_
#include "api.h"

#ifdef OS_UCOS
#ifdef UCOS_V3
typedef OS_SEM *sem_t;
#else
typedef OS_EVENT *sem_t;
#endif
#endif
#ifdef OS_FREE_RTOS
typedef void* sem_t;
#endif

void sem_init(sem_t *sem, int value);
int sem_wait(sem_t *sem, unsigned int timeout);
int sem_post(sem_t *sem);
void sem_destory(sem_t *sem);
int is_sem_empty(sem_t *sem);
#endif
