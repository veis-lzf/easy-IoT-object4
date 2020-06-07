#include "api.h"
#include "atomic.h"

void enter_interrupt(void)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OSIntNestingCtr++;
#else
	OSIntNesting++;
#endif
#endif
#ifdef OS_FREE_RTOS
	OSIntNesting++;
#endif
}

void exit_interrupt(int need_sched)
{
#ifdef	OS_UCOS
	if (need_sched)
		OSIntExit();
	else{
#ifdef UCOS_V3
		OSIntNestingCtr--;
#else
		OSIntNesting--;
#endif
	}
#endif
#ifdef OS_FREE_RTOS
	OSIntNesting--;
	portYIELD_FROM_ISR(need_sched);
#endif
}


unsigned int local_irq_save(void)
{
	unsigned int cpu_sr;
#ifdef OS_UCOS
#ifdef UCOS_V3
	cpu_sr = CPU_SR_Save();
#else
	cpu_sr = OS_CPU_SR_Save();
#endif
#endif
#ifdef OS_FREE_RTOS
	vPortEnterCritical();
	cpu_sr = 0;
#endif
	return (unsigned int)cpu_sr;
}

void local_irq_restore(unsigned int cpu_sr)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	CPU_SR_Restore(cpu_sr);
#else
	OS_CPU_SR_Restore(cpu_sr);
#endif
#endif
#ifdef OS_FREE_RTOS
	vPortExitCritical();
#endif
}

void enter_critical(void)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR	err;
	OSSchedLock(&err);
#else
	OSSchedLock();
#endif
#endif
#ifdef OS_FREE_RTOS
	vPortEnterCritical();
#endif
}

void exit_critical(void)
{
#ifdef OS_UCOS	
#ifdef UCOS_V3
	OS_ERR	err;
	OSSchedUnlock(&err);
#else
	OSSchedUnlock();
#endif
#endif
#ifdef OS_FREE_RTOS
	vPortExitCritical();
#endif
}

int atomic_test_set(atomic *at, int val)
{
	uint32_t cpu_sr;
	cpu_sr = local_irq_save();
	if (at->val)
	{
		local_irq_restore(cpu_sr);
		return 1;
	}
	at->val = val;
	local_irq_restore(cpu_sr);
	return 0;
}

void atomic_set(atomic *at, int val)
{
	uint32_t cpu_sr;
	cpu_sr = local_irq_save();
	at->val = val;
	local_irq_restore(cpu_sr);
}


int atomic_read(volatile atomic_t *v)
{
	int val;
	uint32_t cpu_sr;
    	cpu_sr = local_irq_save();
	val = v->val;
	local_irq_restore(cpu_sr);
	return val;
}

void atomic_add(int i, volatile atomic_t *v)
{
	uint32_t cpu_sr;
    	cpu_sr = local_irq_save();
	v->val += i;
	local_irq_restore(cpu_sr);
}

void atomic_sub(int i, volatile atomic_t *v)
{
	uint32_t cpu_sr;
    	cpu_sr = local_irq_save();
	v->val -= i;
	local_irq_restore(cpu_sr);
}

int atomic_add_return(int i, atomic_t *v)
{
	int temp;
	uint32_t cpu_sr;
    	cpu_sr = local_irq_save();
	temp = v->val;
	temp += i;
	v->val = temp;
	local_irq_restore(cpu_sr);

	return temp;
}

int  atomic_sub_return(int i, atomic_t *v)
{
	unsigned long temp;
	uint32_t cpu_sr;
	cpu_sr = local_irq_save();
	temp = v->val;
	temp -= i;
	v->val = temp;
	local_irq_restore(cpu_sr);
	
	return temp;
}


