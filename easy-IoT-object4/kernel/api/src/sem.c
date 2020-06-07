#include "api.h"
#include "debug.h"
#include "sem.h"
#ifdef OS_UCOS
#ifdef UCOS_V3
#define OS_TICKS_PER_SEC OS_CFG_TICK_RATE_HZ
#endif
#endif

#ifdef OS_FREE_RTOS
#define OS_TICKS_PER_SEC configTICK_RATE_HZ
#endif

void sem_init(sem_t *sem, int value)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
    sem_t tmp = mem_calloc_ex("sem_init", 1, sizeof(OS_SEM));
    if(tmp == NULL)
    {
        *sem = NULL;
        return;
	}

	OSSemCreate (tmp, "sem_init", value, &perr);
	if(perr != OS_ERR_NONE)
	{
        p_err("sem_init OSSemCreate err");
		mem_free(tmp);
		tmp = NULL;
	}

	*sem = tmp;
	return;
#else

	*sem = (sem_t)OSSemCreate(value);
	if (*sem == 0)
		p_err("sem_init err");

	return;
#endif
#endif

#ifdef OS_FREE_RTOS
    *sem = xSemaphoreCreateCounting(8, value);

    if (*sem == 0)
	    p_err("sem_init err");

    return;
#endif

}

int sem_wait(sem_t *sem, unsigned int timeout)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
    OS_SEM_CTR ret;

#else
	INT8U perr;
#endif
	if ((timeout > 0) && (timeout < 1000uL / OS_TICKS_PER_SEC))
		timeout = 1000uL / OS_TICKS_PER_SEC;

	timeout = timeout * OS_TICKS_PER_SEC / 1000uL;

#ifdef UCOS_V3
	ret = OSSemPend ((OS_SEM *)*sem, timeout, OS_OPT_PEND_BLOCKING, NULL, &perr);			   
#else
	OSSemPend((OS_EVENT*) * sem, timeout, &perr);
#endif

	if (perr == OS_ERR_NONE)
		return 0;
	else if (perr == OS_ERR_TIMEOUT)
		return 1;

	p_err("sem_wait err %d", perr);
	return  - 1;
#endif

#ifdef OS_FREE_RTOS
    BaseType_t ret;

    if ((timeout > 0) && (timeout < 1000uL / OS_TICKS_PER_SEC))
		timeout = 1000uL / OS_TICKS_PER_SEC;

	timeout = timeout * OS_TICKS_PER_SEC / 1000uL;
	if(timeout == 0)
		timeout = portMAX_DELAY;
    ret = xSemaphoreTake( *sem, timeout);
	if( ret == pdTRUE)
	{
        return 0;
	}
	else if(ret == pdFALSE)
	{
	    //p_err("xSemaphoreTake timeout");
        return 1;
	}
	else
	{
        p_err("xSemaphoreTake error [%d]", ret);
        return -1;
	}
#endif


}

int sem_post(sem_t *sem)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
    OS_SEM_CTR ret;
    ret = OSSemPost ((OS_SEM *)*sem, OS_OPT_POST_ALL, &perr);
#else
	INT8U perr;
    perr = OSSemPost((OS_EVENT*) * sem);
#endif
	
	if (perr == OS_ERR_NONE)
		return 0;

	p_err("sem_post err %d", perr);
	return  - 1;
#endif
#ifdef OS_FREE_RTOS
    BaseType_t ret = xSemaphoreGive( *sem );
    if( ret == pdTRUE)
    {
        return 0;
	}
	else
	{
        p_err("xSemaphoreGive err %d", ret);
	    return  -1;
	}
#endif

}

int is_sem_empty(sem_t *sem)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
    sem_t tmp = *sem;
    if(((OS_SEM *)tmp)->Ctr != 0)
    {
        return 0;
	}
#else
    sem_t tmp = *sem;
    if(((OS_EVENT *)tmp)->OSEventCnt != 0)
    {
        return 0;
	}

	return 1;
#endif
#endif

#ifdef OS_FREE_RTOS
    return prvIsQueueEmpty(*sem);
#endif

}

void sem_destory(sem_t *sem)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
	OS_OBJ_QTY ret;
	ret = OSSemDel ((OS_SEM *) *sem, OS_OPT_DEL_ALWAYS, &perr);
	mem_free(*sem);
	*sem = NULL;
#else
	INT8U perr;
    OSSemDel((OS_EVENT*) * sem, OS_DEL_ALWAYS, &perr);
#endif

	if (perr != OS_ERR_NONE)
		p_err("sem_destory err %d", perr);
#endif

#ifdef OS_FREE_RTOS
    vSemaphoreDelete(*sem);
#endif
}
