//#define DEBUG
//#define DEBUG_TASK
#include "api.h"
#include "debug.h"

static struct workqueue_struct *timer_tasklet = NULL;

static void os_workqueue_thread(struct workqueue_struct *workqueue);

int thread_create(void(*task)(void *p_arg), void *p_arg, unsigned int prio, unsigned int *pbos, unsigned int stk_size, char *name)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR err;
	OS_TCB *TaskTCB=NULL;
	
	if (pbos == 0)
	{
		pbos = (CPU_STK *)mem_calloc2(stk_size, sizeof(CPU_STK));
		if (!pbos)
			return NULL;
		TaskTCB=(OS_TCB *)mem_calloc2(1, sizeof(OS_TCB));
		if (!TaskTCB)
			return NULL;
	}

	OSTaskCreate(	(OS_TCB 	*)TaskTCB,
								(CPU_CHAR 	*)name,
								(OS_TASK_PTR)task,
								(void 		*)p_arg,
								(OS_PRIO	) prio,
								(CPU_STK	*)&pbos[0],
								(CPU_STK_SIZE)stk_size/10,
								(CPU_STK_SIZE)stk_size,
								(OS_MSG_QTY)0,
								(OS_TICK	)0,
								(void		*)0,
								(OS_OPT		)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
								(OS_ERR		*)&err);
								
	if (err == OS_ERR_NONE)
	{
		return (int)TaskTCB;
	}else{
			if(pbos)
				mem_free2(pbos);
			if(TaskTCB)
				mem_free2(TaskTCB);
	}
#else
	INT8U ret, need_free_stack = 0;
	if (pbos == 0)
	{
		need_free_stack = 1;
		pbos = (OS_STK*)mem_calloc2(stk_size, sizeof(OS_STK));
		if (!pbos)
			return OS_ERR;
	}
	ret = OSTaskCreateExt(task, p_arg, &pbos[stk_size - 1], prio, prio, pbos, stk_size, NULL, (INT16U)(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR), need_free_stack);
	if (ret == OS_ERR_NONE)
	{
		OSTaskNameSet(prio, (INT8U*)name, (INT8U*) &ret);
		return prio;
	}else{
		if(need_free_stack && pbos)
			mem_free2(pbos);
	}
#endif
#endif
#ifdef OS_FREE_RTOS
	TaskHandle_t xHandle = NULL;
	xTaskCreate(task, name, stk_size, p_arg, prio, &xHandle);
	return (int)xHandle;
#endif

	return  - 1;
}

int thread_exit(int thread_id)
{
	int ret = 0;
	if(!thread_id)
		return -1;
#ifdef OS_UCOS
#ifdef UCOS_V3
	OSTaskDel((OS_TCB *)thread_id,(OS_ERR*)&ret);
	mem_free2(((OS_TCB *)thread_id)->StkBasePtr);//mark
	mem_free2((void*)thread_id);
#else
	
	ret = OSTaskDel(thread_id);
#endif
#endif
#ifdef OS_FREE_RTOS
	vTaskDelete((TaskHandle_t)thread_id);
#endif
	return ret;
}

//返回任务自身的ID号
int thread_myself()
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	return (int)OSTCBCurPtr;
#else
	return OSTCBCur->OSTCBId;
#endif
#endif
#ifdef OS_FREE_RTOS
	return (int)xTaskGetCurrentTaskHandle();
#endif
}


void initList(PLIST_HEADER pList)
{
	pList->pHead = pList->pTail = NULL;
	pList->size = 0;
	return ;
}

void insertTailList(PLIST_HEADER pList, PLIST_ENTRY pEntry)
{
	uint32_t cpu_sr;
	assert(pList);
    	cpu_sr = local_irq_save();
	pEntry->pNext = NULL;
	if (pList->pTail)
		pList->pTail->pNext = pEntry;
	else
		pList->pHead = pEntry;
	pList->pTail = pEntry;
	pList->size++;
	local_irq_restore(cpu_sr);
	return ;
}

PLIST_ENTRY removeHeadList(PLIST_HEADER pList)
{
	PLIST_ENTRY pNext;
	PLIST_ENTRY pEntry = 0;
	uint32_t cpu_sr;
	assert(pList);
    	cpu_sr = local_irq_save();
	pEntry = pList->pHead;
	if (pList->pHead != NULL)
	{
		pNext = pList->pHead->pNext;
		pList->pHead = pNext;
		if (pNext == NULL)
			pList->pTail = NULL;
		pList->size--;
		if(pList->size == 0)
			pList->pHead = pList->pTail = NULL;
	}else{
		pList->size = 0;
		pList->pHead = pList->pTail = NULL;
	}
	local_irq_restore(cpu_sr);
	return pEntry;
}

int getListSize(PLIST_HEADER pList)
{
	return pList->size;
}

PLIST_ENTRY delEntryList(PLIST_HEADER pList, PLIST_ENTRY pEntry)
{
	PLIST_ENTRY pCurEntry;
	PLIST_ENTRY pPrvEntry;

	if (pList->pHead == NULL)
		return NULL;

	if (pEntry == pList->pHead)
	{
		pCurEntry = pList->pHead;
		pList->pHead = pCurEntry->pNext;

		if (pList->pHead == NULL)
			pList->pTail = NULL;

		pList->size--;
		return pCurEntry;
	}

	pPrvEntry = pList->pHead;
	pCurEntry = pPrvEntry->pNext;
	while (pCurEntry != NULL)
	{
		if (pEntry == pCurEntry)
		{
			pPrvEntry->pNext = pCurEntry->pNext;

			if (pEntry == pList->pTail)
				pList->pTail = pPrvEntry;

			pList->size--;
			break;
		}
		pPrvEntry = pCurEntry;
		pCurEntry = pPrvEntry->pNext;
	}

	return pCurEntry;
}

struct workqueue_struct *create_workqueue(const char *name, uint32_t stack_size, int prio)
{
	struct workqueue_struct *new_queue = (struct workqueue_struct *)mem_calloc(sizeof(struct workqueue_struct ), 1);
	if(!new_queue)
		return NULL;

	new_queue->thread_event = init_event();
	if(!new_queue->thread_event)
		goto faild1;

	initList(&new_queue->list);
	new_queue->name = name;
	new_queue->task_id = prio; //thread_create调用的时候可能马上会切换到新线程，这里预先赋好值
	new_queue->task_id = thread_create((void (*)(void*))os_workqueue_thread,
                        new_queue,
                        prio,
                        NULL,
                        stack_size,
                        (char*)name);

	if(new_queue->task_id <= 0)
		goto faild;
	

	return new_queue;
faild:
	del_event(new_queue->thread_event);;
faild1:
	mem_free(new_queue);
//	OSSchedUnlock();
	return NULL;
}

void destroy_workqueue(struct workqueue_struct *workqueue)
{
	if(!workqueue)
		return;
	p_dbg("destroy %s", workqueue->name);
	del_event(workqueue->thread_event);
	thread_exit(workqueue->task_id);
	mem_free(workqueue);
}

/*
*工作线程
*参考linux的workqueue机制
*为异步事件的处理提供一种很好的方式
*/
static void os_workqueue_thread(struct workqueue_struct *workqueue)
{
	int ret;
	uint32_t irq_flag;
	p_dbg("start workqueue %s id:%d", workqueue->name, workqueue->task_id);
	while (1)
	{  p_dbg_enter_task;
		ret = wait_event_timeout(workqueue->thread_event, 1000);
		if (ret != 0)
		{
			if (ret != WAIT_EVENT_TIMEOUT)
			{
				p_err("os_tasklet_thread wait err:%d, %x\n", ret, workqueue->thread_event);
				sleep(2000);
				continue;
			}
			//continue;
		}
		
		if (workqueue->list.size > 0)
		{
			while (workqueue->list.size)
			{
				PLIST_ENTRY entry = NULL;
				irq_flag = local_irq_save();
				if(workqueue->list.size)
					entry = removeHeadList(&workqueue->list);
				local_irq_restore(irq_flag);
					
				if (entry)
				{
					struct work_struct *work;
					work = (struct work_struct*)((char*)entry - offsetof(struct work_struct, entry));
					if (work->fun){
						//while(work->ref_cnt--)
						{
							work->fun(work->data);
						}
					}
				}
				else
				{
					p_err("no entry:%d\n", workqueue->list.size);
					break;
				} 
			}
		}
        p_dbg_exit_task;
	}
}

/*
*定时器服务线程
*因ucos自带的定时器函数不允许使用信号等待操作
*这里单独建一个线程
*注意:已经对ucos内核做了一点修改
*/
int init_timer_tasklet()
{
	timer_tasklet = create_workqueue("timer_tasklet", TIMER_TASKLET_STACK_SIZE, TASK_TIMER_TASKLET_PRIO);
	if(timer_tasklet)
		return 0;
	else
		return -1;
}
void init_work(struct work_struct *work, void *fun, void *data)
{
	work->fun = (void (*)(void *))fun;
	work->data = data;
	work->ref_cnt = 0;
}


int schedule_work(struct workqueue_struct *queue, struct work_struct *work)
{
#if 1
	uint32_t irq_flg;
	LIST_ENTRY *entry;
	irq_flg = local_irq_save();
		
	for(entry = queue->list.pHead; entry != NULL; entry = entry->pNext)
	{
		if(entry == &work->entry)
		{
			break;
		}
	}
	if(entry == NULL){
		insertTailList(&queue->list, &work->entry);
		work->ref_cnt = 1;
	}else
		work->ref_cnt++;
	
	local_irq_restore(irq_flg);
#else
	insertTailList(&queue->list, &work->entry);
#endif
	wake_up(queue->thread_event);
	
	return 0;
}


int schedule_timer_tasklet(struct work_struct *work)
{
	int ret = -1;
	if(timer_tasklet)
		ret = schedule_work(timer_tasklet,  work);
	return ret;
}

void timer_tasklet_callback(struct work_struct *work)
{
	schedule_timer_tasklet(work);
}

void init_work_thread(void)
{
	init_timer_tasklet();		
}

