#include "api.h"
#include "debug.h"
#include "wait.h"

#ifdef OS_UCOS
#ifdef UCOS_V3
#define OS_TICKS_PER_SEC OS_CFG_TICK_RATE_HZ
#endif
#endif

#ifdef OS_FREE_RTOS
#define OS_TICKS_PER_SEC configTICK_RATE_HZ
#endif

wait_event_t init_event()                                  //创建一个二值信号量              
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
    OS_SEM * event = (OS_SEM *)mem_calloc_ex("init_event", 1, sizeof(OS_SEM));
    
	if(NULL == event)
	{
	    p_err("mem_calloc_ex err");
        return 0;
	}

	OSSemCreate (event, "init_event", 0, &perr);
	if(OS_ERR_NONE != perr)
	{
        p_err("OSSemCreate err");
		mem_free(event); 
		return 0;
	}
	
    return (wait_event_t)event;
#else
	OS_EVENT *event = OSSemCreate(0);
	if (!event)
		p_err("init_waitqueue_head err");

	return (wait_event_t)event;
#endif
#endif

#ifdef OS_FREE_RTOS
    SemaphoreHandle_t ret = xSemaphoreCreateBinary();            //创建一个二值信号量
    return (wait_event_t)ret;
#endif

}

int wait_event(wait_event_t wq)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
    OS_ERR perr;
	OS_SEM_CTR ret;
#else
    INT8U perr;
#endif
    if (wq == NULL)
	{
		return  - 1;
	}

  #ifdef UCOS_V3
    ret = OSSemPend (wq, 0, OS_OPT_PEND_BLOCKING, NULL, &perr);
  #else
    OSSemPend(wq, 0, &perr);
  #endif
  
	if(perr != OS_ERR_NONE)
	{
        p_err("wait_event err:%d\n", perr);
		return -1;
	}

	return 0;

#endif

#ifdef OS_FREE_RTOS
    if (wq == NULL)
	{
		return  - 1;
	}

	if(xSemaphoreTake( wq, portMAX_DELAY) == pdTRUE)
	{
        return 0;
	}

	p_err("wait_event xSemaphoreTake err");
	return -1;
#endif

}

int wait_event_timeout(wait_event_t wq, unsigned int timeout)      //等待信号量
{
#ifdef OS_UCOS
#ifdef UCOS_V3
    OS_ERR perr;
	OS_SEM_CTR ret;
#else
	INT8U perr;
#endif

	uint32_t ticks;
	if (wq == NULL)
	{
		//p_err("wait_event_timeout wq err");
		return  - 1;
	}
	ticks = (timeout *OS_TICKS_PER_SEC) / 1000;

	
    #ifdef UCOS_V3
	ret = OSSemPend (wq, ticks, OS_OPT_PEND_BLOCKING, NULL, &perr);

	#else
    OSSemPend(wq, ticks, &perr);
	#endif
	
	if (perr == OS_ERR_NONE)
		return 0;
	else if (perr == OS_ERR_TIMEOUT){
		return WAIT_EVENT_TIMEOUT;
	}
	return  - 1;
#endif

#ifdef OS_FREE_RTOS                                     //等待信号量
    BaseType_t ret;

    if (wq == NULL)
	{
		p_err("wait_event_timeout wq err");
		return  - 1;
	}
	
    if ((timeout > 0) && (timeout < 1000uL / OS_TICKS_PER_SEC))
		timeout = 1000uL / OS_TICKS_PER_SEC;

	timeout = timeout * OS_TICKS_PER_SEC / 1000uL;
	if(timeout == 0)
		timeout = portMAX_DELAY;
	
    ret = xSemaphoreTake( wq, timeout);
	if(ret == pdTRUE)
	{
        return 0;
	}
	else if(ret == pdFALSE)
	{
	    //p_err("wait_event_timeout timeout");
        return 1;
	}
	else
	{
        p_err("wait_event_timeout error [%d]", ret);
        return -1;
	}

#endif

}

void wake_up(wait_event_t wq)      //释放信号量  唤醒事件
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
    OS_SEM_CTR ret;
#else
	INT8U perr;
#endif
    if(wq == NULL)
    {
        return;
	}

    #ifdef UCOS_V3
	ret = OSSemPost (wq, OS_OPT_POST_ALL, &perr);
	#else
	perr = OSSemPost(wq);
	#endif
	
	if (perr != OS_ERR_NONE)
		p_err("wake_up err %d", perr);
#endif

#ifdef OS_FREE_RTOS
    BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdTRUE;

    if(in_interrupt())
	{
	    xSemaphoreGiveFromISR( wq, &xHigherPriorityTaskWoken);
	}
	else
	{
        xSemaphoreGive(wq);               //释放信号量  唤醒事件
	}

#endif

}

void del_event(wait_event_t wq)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
	OS_OBJ_QTY ret = OSSemDel (wq, OS_OPT_DEL_ALWAYS, &perr);
	mem_free(wq);
#else
	INT8U perr;
    OSSemDel (wq, OS_DEL_ALWAYS, &perr);
#endif

#endif

#ifdef OS_FREE_RTOS
    vSemaphoreDelete(wq);
#endif

}

void clear_wait_event(wait_event_t wq)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
#else
	INT8U perr;
#endif

	if (wq == 0)
	{
		p_err("clear_wait_event wq err");
		return ;
	}
	
	OSSemSet(wq, 0, &perr);
	if (perr != OS_ERR_NONE)
		p_err("clear_wait_event err %d", perr);

	return ;
#endif

#ifdef OS_FREE_RTOS
    while(prvIsQueueEmpty(wq) == pdFALSE)
    {
        if(xSemaphoreTake( wq, 0) != pdTRUE)
	    {
	        p_err("clear_wait_event xSemaphoreTake err");
            break;
	    }
	}

#endif

}
