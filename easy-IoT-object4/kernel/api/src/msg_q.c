#include "api.h"
#include "debug.h"
#include "msg_q.h"

struct _msg_q_manage
{
	msg_q_t msg;
	void **q_start;
};

#ifdef OS_UCOS
#ifdef UCOS_V3
#define OS_MAX_QS MSG_QUEUE_MAX
#define OS_TICKS_PER_SEC OS_CFG_TICK_RATE_HZ
#endif
#endif

#ifdef OS_UCOS
struct _msg_q_manage q_man[OS_MAX_QS];
#endif

#ifdef OS_FREE_RTOS
static int null_pointer = 0;
#define OS_TICKS_PER_SEC configTICK_RATE_HZ

#endif

int msgget(msg_q_t *msg_q, int q_size)          //创建一个消息队列
{
#ifdef OS_UCOS
#ifdef UCOS_V3
    OS_Q * msg;
    OS_ERR perr;
#else
	OS_EVENT *msg;
#endif
	void **q_start = NULL;
	int i;

	for (i = 0; i < OS_MAX_QS; i++)
	{
		if (q_man[i].msg == 0)
			break;
	} if (i >= OS_MAX_QS)
	{
		p_err("msgget: no more msg queen");
		return  - 1;
	}
#ifdef UCOS_V3
    msg = (OS_Q *)mem_calloc_ex("msgget", 1, sizeof(OS_Q));
    if(msg == NULL)
    {
        p_err("msgget: mem_calloc_ex err.");
        return -1;
	}

	OSQCreate(msg, "msgget", q_size, &perr);
	if(perr != OS_ERR_NONE)
	{
	    mem_free(msg);
        p_err("msgget: OSQCreate err.");
        return -1;
	}         
#else
	q_start = (void **)mem_malloc(q_size *(sizeof(void*)));
	if (q_start == 0)
	{
		p_err("msgget: malloc queen err.no enough malloc");
		return  - 1;
	}

	msg = OSQCreate(q_start, (INT16U)q_size);
	if (msg == 0)
	{
		*msg_q = 0;
		mem_free(q_start);
		return  - 1;
	}
#endif
	*msg_q = (msg_q_t)msg;
	q_man[i].msg = (msg_q_t)msg;
	q_man[i].q_start = q_start;

	return 0;
#endif

#ifdef OS_FREE_RTOS
    *msg_q = xQueueCreate(q_size, sizeof(void *));       //创建一个消息队列
    return 0;
#endif
}

int msgsnd(msg_q_t msgid, void *msgbuf)               //向消息队列中发送消息
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
#else
	INT8U perr;
#endif
	if (0 == msgid)
	{
		p_err("msgsnd: msgid err");
		return  - 1;
	}
	
#ifdef UCOS_V3
	OSQPost((OS_Q *)msgid, msgbuf, sizeof(void *), OS_OPT_POST_FIFO, &perr); 
#else
	perr = OSQPost((OS_EVENT*)msgid, (void*)msgbuf);
#endif

	if (perr == OS_ERR_NONE)
		return 0;
	else if (perr == OS_ERR_Q_FULL)
		return 1;

	p_err("msgsnd: err %d", perr);
	return  - 1;
#endif

#ifdef OS_FREE_RTOS
    BaseType_t ret;
    void *data = msgbuf;
    if(data == NULL)
    {
        data = (void *)&null_pointer;
	}

	ret = xQueueSend( msgid, &data, 1000);      //向消息队列中发送消息

	if(ret == pdTRUE)
	{
	    return 0;
	}
	else if(ret == pdFALSE)
	{
		p_err("msgsnd timeout");
		return 1;
	}
	else
	{
		p_err("msgsnd error [%d]", ret);
		return -1;
	}
#endif

}

int msgrcv(msg_q_t msgid, void **msgbuf, unsigned int timeout)                    //从消息队列中接收消息
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
    OS_MSG_SIZE  msg_size;

#else
	INT8U perr;
#endif
	if (0 == msgid)
	{
		p_err("msgrcv: msgid err");
		return  - 1;
	}

	if ((timeout > 0) && (timeout < 1000uL / OS_TICKS_PER_SEC))
		timeout = 1000uL / OS_TICKS_PER_SEC;

	timeout = timeout * OS_TICKS_PER_SEC / 1000uL;

#ifdef UCOS_V3
    *msgbuf = OSQPend ((OS_Q *)msgid, timeout, OS_OPT_PEND_BLOCKING, &msg_size, NULL, &perr);
#else
	*msgbuf = OSQPend((OS_EVENT*)msgid, (INT32U)timeout, &perr);
#endif

	if (perr == OS_ERR_NONE)
		return 0;
	else if (perr == OS_ERR_TIMEOUT)
		return 1;

	p_err("msgrcv: err %d", perr);
	return  - 1;
#endif

#ifdef OS_FREE_RTOS
    void *data = NULL;
	BaseType_t ret;

    if (0 == msgid)
	{
		p_err("msgrcv: msgid err");
		return  - 1;
	}

	if ((timeout > 0) && (timeout < 1000uL / OS_TICKS_PER_SEC))
		timeout = 1000uL / OS_TICKS_PER_SEC;

	timeout = timeout * OS_TICKS_PER_SEC / 1000uL;

	if(timeout == 0)
		timeout = portMAX_DELAY;
	
	ret = xQueueReceive( msgid, &data, timeout);             //从消息队列中接收消息
	if(ret == pdTRUE)
	{
	    if(data == (&null_pointer))
	    {
            *msgbuf = NULL;
		}
		else
		{
		    *msgbuf = data;
		}
	    return 0;
	}
	else if(ret == pdFALSE)
	{
		//p_err("msgrcv timeout");
		*msgbuf = data;
		return 1;
	}
	else
	{
		p_err("msgrcv error [%d]", ret);
		*msgbuf = data;
		return -1;
	}
#endif

}

int msgfree(msg_q_t msgid)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR perr;
#else
    INT8U perr;
#endif
	int i;
	
	for (i = 0; i < OS_MAX_QS; i++)
	{
		if (q_man[i].msg == msgid)
			break;
	}
	if (i >= OS_MAX_QS)
	{
		p_err("msgfree: err no match msg_q %x", msgid);
		return  - 1;
	}
#ifdef UCOS_V3
	OSQDel ((OS_Q *)msgid, OS_OPT_DEL_ALWAYS, &perr);
    mem_free(msgid);
#else
    OSQDel((OS_EVENT*)msgid, OS_DEL_ALWAYS, &perr);
#endif

	if (OS_ERR_NONE != perr)
	{
		p_err("msgfree: free err %d", perr);
		return  - 1;
	}

	if (q_man[i].q_start)
	{
		mem_free(q_man[i].q_start);
	}

	q_man[i].q_start = 0;
	q_man[i].msg = 0;

	return 0;
#endif

#ifdef OS_FREE_RTOS
    vQueueDelete(msgid);
    return 0;
#endif


}

void msg_q_init(void)
{
#ifdef OS_UCOS
	int i;
	for (i = 0; i < OS_MAX_QS; i++)
	{
		q_man[i].msg = 0;
		q_man[i].q_start = 0;
	}
#endif
}
