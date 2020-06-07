#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"
#include "dhcpd.h"
#include "web_cfg.h"
#include <cctype>


#define VIRSION		"T1.0"

char command;
int dbg_level = 1;

/*
*空函数,避免编译错误
*/
void send_work_event(uint32_t event)
{

}

/*
*空函数,避免编译错误
*/
void show_tcpip_info(struct netif *p_netif)
{
}

msg_q_t test_msg = 0;

void test_thread1(void *pdata)
{
	int cnt = 0;
	p_dbg("进入线程:%s", (char*)pdata);	
	while(cnt < 5)
	{
		sleep(1000);  //线程1的优先级高一些,我们设定每10ms打印一次
		p_dbg("发送消息");
		msgsnd(test_msg, "这是线程1的消息");	
		cnt++;
	}
	p_dbg("退出线程:%s", (char*)pdata);	
	thread_exit(thread_myself());
}

void test_thread2(void *pdata)
{
	int cnt = 0, ret;
	void *p_msg_recv;
	p_dbg("进入线程:%s", (char*)pdata);	
	
	while(cnt < 10)
	{
		ret = msgrcv(test_msg, &p_msg_recv, 2000);
		if(ret < 0)
			p_err("test_event err");
		else if(ret > 0)
			p_dbg("TIMEOUT");
		else
			p_dbg("接收到消息:%s", (char*)p_msg_recv);
		cnt++;
	}
	p_dbg("退出线程:%s", (char*)pdata);	
	thread_exit(thread_myself());
}

void main_thread(void *pdata)
{
	int cnt = 0;
	#ifdef DEBUG
	RCC_ClocksTypeDef RCC_ClocksStatus;
	#endif
	
	driver_misc_init(); //初始化一些杂项(驱动相关)
	usr_gpio_init(); //初始化GPIO
	//IND3_ON; //点亮LED
#ifdef OS_UCOS
#ifdef UCOS_V3
	OSStatTaskCPUUsageInit((OS_ERR*)&ret);
#else
	OSStatInit(); //初始化UCOS状态
#endif
#endif
	uart1_init(); //初始化串口1
	uart3_init();

	//打印MCU总线时钟
#ifdef DEBUG
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	p_dbg("SYSCLK_Frequency:%d,HCLK_Frequency:%d,PCLK1_Frequency:%d,PCLK2_Frequency:%d,ADCCLK_Frequency:%d\n", 
		RCC_ClocksStatus.SYSCLK_Frequency, 
		RCC_ClocksStatus.HCLK_Frequency, 
		RCC_ClocksStatus.PCLK1_Frequency, 
		RCC_ClocksStatus.PCLK2_Frequency, 
		0);
#endif

/*
*os测试例程之消息队列
*
*
*创建两个线程
*	线程2保持睡眠,消息到来
*	线程1每秒发送一个消息给线程2
*
*/
	cnt = msgget(&test_msg, 10); //创建一个互斥变量
	if(cnt)
		p_err("test_event 创建失败");
	else
		p_dbg("test_event 创建成");

	cnt = thread_create(test_thread1, 
		"参数1", 	
		TASK_MAIN_PRIO + 1, 	//线程优先级,请注意ucos2环境下每个线程的优先级不能相同
		0, 	//线程堆栈指针,为0代表自动分配
		256, //堆栈大小
		"线程名称1");
	if(cnt < 0)
		p_err("线程 1 创建失败");
	else
		p_dbg("线程 1 创建成功,线程ID:%d", cnt);
		
	cnt = thread_create(test_thread2, 
		"参数2", 
		TASK_MAIN_PRIO + 2, 
		0, 
		256, 
		"线程名称2");
	if(cnt < 0)
		p_err("线程 2 创建失败");
	else
		p_dbg("线程 2 创建成功,线程ID:%d", cnt);
	
	while(1)
	{
		sleep(2000);
		//p_dbg("%s %d", __FUNCTION__, cnt++);
	}
}

int main(void)
{
#ifdef OS_UCOS
#ifdef UCOS_V3
	OS_ERR err;
	OSInit(&err);
#else
	OSInit();
#endif
#endif
	_mem_init(); //初始化内存分配
	msg_q_init();//
	thread_create(main_thread, 0, TASK_MAIN_PRIO, 0, TASK_MAIN_STACK_SIZE, "main_thread");
#ifdef OS_UCOS
#ifdef UCOS_V3
	OSStart(&err);
#else
	OSStart();
#endif
#endif
#ifdef OS_FREE_RTOS
	vTaskStartScheduler();
#endif
	return 0;
}
