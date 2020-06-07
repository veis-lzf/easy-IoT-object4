#include "api.h"
#include "debug.h"
#include "mutex.h"

mutex_t mutex_init(const char *name)              //创建一个互斥量
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
    mutex_t ret = (mutex_t)mem_calloc_ex("mutex_init", 1, sizeof(OS_MUTEX));
    if(ret == NULL)
    {
        p_err("mutex_init mem_calloc_ex error");
        return NULL;
	}

	OSMutexCreate (ret, (CPU_CHAR  *)name, &perr);
	if(perr != OS_ERR_NONE)
	{
        p_err("mutex_init OSMutexCreate err");
		mem_free(ret);
		ret = NULL;
	}

	return ret;
#else
	INT8U err;
	mutex_t ret;
	ret =  (mutex_t)OSSemCreate(1);
	if(ret){
		OSEventNameSet(ret, (INT8U*)name, &err);
	}
	return ret;
#endif
#endif

#ifdef OS_FREE_RTOS

    mutex_t ret = xSemaphoreCreateMutex();      //创建一个互斥量
    return ret;
#endif

}

int mutex_lock(mutex_t mutex)               //等待一个互斥信号
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;

	OSMutexPend (mutex, 0, OS_OPT_PEND_BLOCKING, NULL, &perr);
#else
	INT8U perr;

	OSSemPend((OS_EVENT*)mutex, 0, &perr);
#endif

	if (perr == OS_ERR_NONE)
		return 0;

	p_err("mutex_lock err %d", perr);
	return  - 1;

#endif

#ifdef OS_FREE_RTOS

    if(xSemaphoreTake( mutex, portMAX_DELAY) == pdTRUE)      //等待一个互斥信号
	{
        return 0;
	}

	p_err("mutex_lock xSemaphoreTake err");
	return -1;
#endif

}

int mutex_unlock(mutex_t mutex)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;

    mutex->OwnerTCBPtr = OSTCBCurPtr;
	
	OSMutexPost (mutex, OS_OPT_POST_NONE, &perr);

#else
	INT8U perr;
	perr = OSSemPost((OS_EVENT*)mutex);
#endif

	if (perr == OS_ERR_NONE)
		return 0;

	p_err("mutex_unlock err %d", perr);
	return  - 1;
#endif

#ifdef OS_FREE_RTOS
    if(xSemaphoreGive(mutex) == pdTRUE)
    {
        return 0;
	}
	else
	{
        p_err("mutex_unlock xSemaphoreGive err");
	    return  - 1;
	}
	
#endif
}

void mutex_destory(mutex_t mutex)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
    OS_OBJ_QTY  ret;
	ret = OSMutexDel (mutex, OS_OPT_DEL_ALWAYS, &perr);
	mem_free(mutex);
#else
	INT8U perr;
	OSSemDel((OS_EVENT*)mutex, OS_DEL_ALWAYS, &perr);
#endif

	if (perr != OS_ERR_NONE)
		p_err("mutex_destory err %d", perr);
#endif

#ifdef OS_FREE_RTOS
    vSemaphoreDelete(mutex);
#endif
}
