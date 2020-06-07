#include "api.h"
#include "debug.h"
#include "timer.h"
#include "sys_misc.h"

#ifdef OS_UCOS
#ifdef UCOS_V3
#define OS_TMR_CFG_TICKS_PER_SEC OS_TICK_RATE_HZ
#endif
#endif

#ifdef OS_FREE_RTOS
#define OS_TMR_CFG_TICKS_PER_SEC configTICK_RATE_HZ
#endif

int mstoticks(int time_val)
{
    if(time_val <= 0)
    {
        return 0;
	}

	if(time_val < 1000/OS_TMR_CFG_TICKS_PER_SEC)
	{
        time_val = 1000/OS_TMR_CFG_TICKS_PER_SEC;
	}

    return time_val * OS_TMR_CFG_TICKS_PER_SEC / 1000;
}

#ifdef OS_UCOS
void comm_timer_callback(void *p_tmr, void *p_arg)
{
    OS_TMR *tmr = (OS_TMR *)p_tmr;
    if(tmr == NULL)
    {
        return;
	}

	timer_tasklet_callback(&tmr->work);
}
#endif

#ifdef OS_FREE_RTOS
void comm_timer_callback(void *p_tmr)
{
    xTIMER *tmr = (xTIMER *)p_tmr;
    if(tmr == NULL)
    {
        return;
	}

	timer_tasklet_callback(&tmr->work);
}
#endif


timer_t timer_setup(int time_val, int type, timer_callback_func callback, void *callback_arg)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
    OS_ERR  perr;
	OS_TMR *tmr = NULL;

    time_val = mstoticks(time_val);

	if ((callback == 0))
	{
		p_err("setup_timer err arg\n");
		return 0;
	}

	tmr = (OS_TMR *)mem_calloc_ex("timer_setup", 1, sizeof(OS_TMR));

	if(NULL == tmr)
	{
        p_err("mem_calloc_ex err\n");
		return 0;
	}
	
	OSTmrCreate(tmr,
                   "Timer",
                   time_val,
                   time_val,
                   type?OS_OPT_TMR_PERIODIC : OS_OPT_TMR_ONE_SHOT,
                   comm_timer_callback,
                   callback_arg,
                   &perr);
	if(perr != OS_ERR_NONE)
	{
        p_err("OSTmrCreate err\n");
		mem_free(tmr);
		return 0;
	}

	init_work(&tmr->work, callback, callback_arg);
	return (timer_t)tmr;
#else
	INT8U perr;
	OS_TMR *tmr;

	time_val = mstoticks(time_val);

	if ((callback == 0))
	{
		p_err("setup_timer err arg\n");
		return 0;
	}
	if (type)
	//== 1 repeat
	{
		p_dbg("repeat:%d\n", time_val);
		tmr = OSTmrCreate(time_val, time_val, OS_TMR_OPT_PERIODIC, comm_timer_callback, callback_arg, "", &perr);
	}
	else
	{
		p_dbg("one short:%d\n", time_val);
		tmr = OSTmrCreate(time_val, time_val, OS_TMR_OPT_ONE_SHOT, comm_timer_callback, callback_arg, "", &perr);
	}

	if (perr != OS_ERR_NONE)
	{
		tmr = 0;
		p_err("setup_timer err\n");
	}

	init_work(&tmr->work, callback, callback_arg);
	
	return (timer_t)tmr;
#endif
//#else  //for freertos
#endif
#ifdef OS_FREE_RTOS
    xTIMER *tmr = NULL;
    UBaseType_t autoreload = type > 0 ? pdTRUE : pdFALSE;
	
    time_val = mstoticks(time_val);
    
	tmr = xTimerCreate("Timer", time_val, autoreload, callback_arg, comm_timer_callback);
    if(tmr != NULL)
    {
        init_work(&tmr->work, callback, callback_arg);
	}

	return tmr;
#endif
}

int timer_pending(timer_t tmr)
{
#ifdef OS_UCOS
    OS_TMR *tmr_t = (OS_TMR*)tmr;

	if (tmr_t == 0)
	{
		p_err("timer_pending tmr err");
		return  0;
	}
#ifdef UCOS_V3
	if(tmr_t->State == OS_TMR_STATE_RUNNING)
#else
	if(tmr_t->OSTmrState == OS_TMR_STATE_RUNNING)
#endif	
	{
		return 1;
	}

	return 0;
#endif

#ifdef OS_FREE_RTOS
    void *timer_t = tmr;
	BaseType_t ret;

    if (timer_t == 0)
	{
		p_err("timer_pending tmr == NULL");
		return  0;
	}
	
    ret = xTimerIsTimerActive(timer_t);
    if(ret == pdFALSE)
    {
        return 0;
	}
	else
	{
        return 1;
	}
#endif

}

//expires ----n*ms
int mod_timer(timer_t tmr, unsigned int expires)
{
#ifdef OS_UCOS
	INT8U ret;
  #ifdef UCOS_V3
	OS_ERR  perr;
  #else
	INT8U perr;
  #endif
	
	OS_TMR *tmr_t = tmr;

	if (tmr_t == 0)
	{
		p_err("mod_timer tmr err");
		return  0;
	}

	if (expires < 100)
		expires = 100;

	expires = expires * OS_TMR_CFG_TICKS_PER_SEC / 1000;

	if(timer_pending(tmr)/* && (expires == tmr_t->OSTmrPeriod)*/)
		return 1;

#ifdef UCOS_V3

	tmr_t->Dly = expires;								/* Delay before start of repeat 						  */
	tmr_t->Period = expires;
#else
	tmr_t->OSTmrDly = expires;
	tmr_t->OSTmrPeriod = expires;
#endif
	ret = OSTmrStart(tmr_t, &perr);

#ifdef UCOS_V3
    if (ret == DEF_TRUE && perr == OS_ERR_NONE)
#else
	if (ret == OS_TRUE && perr == OS_ERR_NONE)
#endif
	{
		return 0;
	}
    p_err("mod_timer OSTmrStart err");
	return  0;
#endif

#ifdef OS_FREE_RTOS

    if (expires < 100)
		expires = 100;

	expires = expires * OS_TMR_CFG_TICKS_PER_SEC / 1000;

	if(timer_pending(tmr)/* && (expires == tmr_t->OSTmrPeriod)*/)
		return 1;

    if(xTimerChangePeriod(tmr, expires, 1000) == pdPASS)
    {
        return 0;
	}
	
    p_err("mod_timer xTimerChangePeriod err");
    return 0;
#endif

}

int add_timer(timer_t tmr)
{
#ifdef OS_UCOS
	INT8U ret;
  #ifdef UCOS_V3
	OS_ERR  perr;
  #else
	INT8U perr;
  #endif
	
	OS_TMR *tmr_t = tmr;

	if (tmr_t == 0)
	{
		p_err("add_timer tmr err");
		return  - 1;
	}
	ret = OSTmrStart(tmr_t, &perr);

#ifdef UCOS_V3
	if (ret == DEF_TRUE && perr == OS_ERR_NONE)
#else
	if (ret == OS_TRUE && perr == OS_ERR_NONE)
#endif
	{
		return OS_ERR_NONE;
	}

	return  - 1;
#endif

#ifdef OS_FREE_RTOS
    if (tmr == 0)
	{
		p_err("add_timer tmr err");
		return  - 1;
	}
	
    if(xTimerStart(tmr, 1000) == pdPASS)
    {
        return 0;
	}

    p_err("add_timer xTimerStart err");
	return -1;
#endif
}

//停止定时器
int del_timer(timer_t tmr)
{
#ifdef OS_UCOS

  #ifdef UCOS_V3
	OS_ERR  perr;
  #else
	INT8U perr;
  #endif
	
	OS_TMR *tmr_t = tmr;
	
	if (tmr_t == 0)
	{
		p_err("del_timer tmr err");
		return 0;
	}

	if(timer_pending(tmr))
	{

	#ifdef UCOS_V3
	    OSTmrStop(tmr_t, OS_OPT_TMR_NONE, 0, &perr);
    #else
	    OSTmrStop(tmr_t, OS_TMR_OPT_NONE, 0, &perr);
    #endif
		
		return 1;
	}

	#ifdef UCOS_V3
	    OSTmrStop(tmr_t, OS_OPT_TMR_NONE, 0, &perr);
    #else
	    OSTmrStop(tmr_t, OS_TMR_OPT_NONE, 0, &perr);
    #endif

	return  0;
#endif

#ifdef OS_FREE_RTOS
    if (tmr == 0)
	{
		p_err("del_timer tmr err");
		return 0;
	}

	if(timer_pending(tmr))
	{
        xTimerStop(tmr, 1000);
		return 1;
	}

	xTimerStop(tmr, 1000);
    return 0;
	
#endif

}

//释放删除定时器
int timer_free(timer_t tmr)
{
#ifdef OS_UCOS
	
  #ifdef UCOS_V3
	OS_ERR	perr;
  #else
	INT8U perr;
  #endif

	INT8U ret;
	OS_TMR *tmr_t = tmr;

	ret = OSTmrDel(tmr_t, &perr);

	#ifdef UCOS_V3
	mem_free(tmr);
	tmr = NULL;
	#endif

    #ifdef UCOS_V3
	if ((ret == DEF_TRUE) && (perr == OS_ERR_NONE))
    #else
	if ((ret == OS_TRUE) && (perr == OS_ERR_NONE))
	#endif
		return OS_ERR_NONE;

	p_err("timer_free err %d", perr);
	return  - 1;
#endif

#ifdef OS_FREE_RTOS
    if(xTimerDelete(tmr, 1000) == pdPASS)
    {
        return 0;
	}
	else
	{
	    p_err("timer_free xTimerDelete err");
        return -1;
	}
#endif

}

void sleep(uint32_t ms)
{
#ifdef OS_UCOS
		
  #ifdef UCOS_V3
    OS_ERR  perr;
  
    if(in_interrupt()){
		
		delay_us(ms*1000);
		return;
	}
	
    ms = mstoticks(ms);
	OSTimeDly (ms, OS_OPT_TIME_DLY, &perr);
	
  #else

	u32 s = 0;
	/*
	if (ms < 1000uL / OS_TICKS_PER_SEC)
	//最小睡眠分辨率
	{
		ms = 1000uL / OS_TICKS_PER_SEC;
	}
	ms = ms * OS_TICKS_PER_SEC / 1000uL;
	OSTimeDly(ms);*/
	if(in_interrupt()){
		
		delay_us(ms*1000);
		return;
	}
	if(ms < 10)
		ms = 10;
	if(ms > 1000)
	{
		s = ms/1000;
		ms = ms%1000;
	}
	OSTimeDlyHMSM (0,0,s,ms);
  #endif
#endif

#ifdef OS_FREE_RTOS
    if(in_interrupt())
    {
		delay_us(ms*1000);
		return;
    }
	
	if (ms < 100)
        ms = 100;
	
    ms = ms * OS_TMR_CFG_TICKS_PER_SEC / 1000;

	vTaskDelay(ms);
#endif
}

//ms
unsigned int os_time_get(void)
{
	//return (unsigned int)(OSTimeGet()*(1000uL / OS_TICKS_PER_SEC));
	return tick_ms;
}

