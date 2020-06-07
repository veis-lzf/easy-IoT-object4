#ifndef _ATOMIC_H_
#define _ATOMIC_H_
#include "api.h"

#ifdef OS_UCOS
#ifdef UCOS_V3
#define in_interrupt() (OSIntNestingCtr)
#else
#define in_interrupt() (OSIntNesting)
#endif
#endif
#ifdef OS_FREE_RTOS
extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );
#define in_interrupt() (OSIntNesting)
#endif
#define atomic_t atomic


typedef struct _atomic
{
	volatile unsigned char val;
} atomic;

void enter_interrupt(void);
void exit_interrupt(int need_sched);

unsigned int local_irq_save(void);
void local_irq_restore(unsigned int cpu_sr);
void enter_critical(void);
void exit_critical(void);



int atomic_test_set(atomic *at, int val);
void atomic_set(atomic *at, int val);
int atomic_read(volatile atomic_t *v);
void atomic_add(int i, volatile atomic_t *v);
void atomic_sub(int i, volatile atomic_t *v);
int atomic_add_return(int i, atomic_t *v);
int  atomic_sub_return(int i, atomic_t *v);

#endif
