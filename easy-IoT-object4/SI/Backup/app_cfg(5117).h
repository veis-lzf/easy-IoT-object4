#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "type.h"

#ifndef SUPPORT_AUDIO
#define SUPPORT_AUDIO 			0
#endif

#ifndef SUPPORT_CAM
#define SUPPORT_CAM 			1
#endif

#ifndef ONENET_ENABLE
#define ONENET_ENABLE            0
#endif

#ifndef ALI_IOT_ENABLE
#define ALI_IOT_ENABLE            0
#endif

#ifndef BAIDU_AI_FACE_ENABLE
#define BAIDU_AI_FACE_ENABLE         SUPPORT_CAM
#endif

#ifndef BAIDU_AI_VOICE_ENABLE
#define BAIDU_AI_VOICE_ENABLE  SUPPORT_AUDIO
#endif

//红外发射器D4可以用作简单的LED指示灯
#ifndef USE_D4_AS_LED
#define USE_D4_AS_LED      1
#endif


//LCD和RFID共用一个插座，不能同时使用
#if ENABLE_LCD == 1
#define ENABLE_RFID 0
#else
#define ENABLE_RFID 1	
#endif

#if (BAIDU_AI_FACE_ENABLE && ALI_IOT_ENABLE) || (BAIDU_AI_FACE_ENABLE && ONENET_ENABLE)|| (ALI_IOT_ENABLE && ONENET_ENABLE)
#error "Only support one IoT client"
#endif

#if (BAIDU_AI_VOICE_ENABLE && ALI_IOT_ENABLE) || (BAIDU_AI_VOICE_ENABLE && ONENET_ENABLE)|| (ALI_IOT_ENABLE && ONENET_ENABLE)
#error "Only support one client"
#endif
/*
*********************************************************************************************************
*                                            TASK PRIORITIES
* UCOS每个线程优先级不能相同，放在这里统一管理
*********************************************************************************************************
*/
#define WORK_QUEUE_MAX_SIZE 4
#ifdef OS_UCOS
enum TASK_PRIO{
TASK_UNUSED_PRIO = 10,
//TASK_IMG_SEND_PRIO, 	 //add
//TASK_TCPIP_THREAD_PRIO,  //add
TASK_MOAL_WROK_PRIO,
TASK_SDIO_THREAD_PRIO,

TASK_ETH_INT_PRIO,
OS_TASK_TMR_PRIO,
TASK_TCPIP_THREAD_PRIO,
TASK_TIMER_TASKLET_PRIO,
TASK_MONITOR_PRIO,
REASSOCIATION_THREAD_PRIO, 
//以上顺序勿随意改动

//user task

TASK_MAIN_PRIO,
TASK_MUSIC_PRIO,
TASK_IMG_SEND_PRIO,

TASK_ADC_RECV_PKG_PRIO,
TASK_TCP_RECV_PRIO,
TASK_TCP_ACCEPT_PRIO,
TASK_IOT_CLIENT_PRIO,


TASK_AI_CLIENT_PRIO,
//NETCAMERA_TASK_PRIO,

TASK_DHCP_RECV_PRIO,
TASK_SUB_MQTT_PRIO,
TASK_TCP_SEND_PRIO,
TASK_GUI_APP_PRIO,
TASK_NRF_PRIO,
TASK_TEST_BASE_PRIO,
NETCAMERA_TASK_PRIO,
};
#endif

#ifdef OS_FREE_RTOS
enum TASK_PRIO{
//user task
TASK_GUI_APP_PRIO = 2,
TASK_NRF_PRIO,
TASK_TCP_SEND_PRIO,
TASK_AI_CLIENT_PRIO,
TASK_IOT_CLIENT_PRIO,
TASK_DHCP_RECV_PRIO,


TASK_TCP_ACCEPT_PRIO,
TASK_TCP_RECV_PRIO,
TASK_ADC_RECV_PKG_PRIO,
TASK_MAIN_PRIO,
TASK_IMG_SEND_PRIO,
TASK_MUSIC_PRIO,
TASK_SUB_MQTT_PRIO,

//以下
REASSOCIATION_THREAD_PRIO, 
TASK_MONITOR_PRIO,
TASK_TIMER_TASKLET_PRIO,
TASK_TCPIP_THREAD_PRIO,

OS_TASK_TMR_PRIO,
TASK_ETH_INT_PRIO,
TASK_SDIO_THREAD_PRIO,	
TASK_MOAL_WROK_PRIO,

//OS_TASK_TMR_PRIO,
TASK_UNUSED_PRIO,
};
#endif



/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*单位WORD
*********************************************************************************************************
*/
#define TASK_TCPIP_THREAD_STACK_SIZE		512
#define TIMER_TASKLET_STACK_SIZE			256
#define MOAL_WROK_STACK_SIZE				256
#define TASK_MONITOR_STACK_SIZE			128
#define REASSOCIATION_THREAD_STACK_SIZE     	400

//以上数值勿随意改动

#define TASK_MAIN_STACK_SIZE				512
#define TASK_ADC_RECV_PKG_STACK_SIZE		1024
#define TASK_TCP_RECV_STACK_SIZE			512
#define TASK_TCP_SEND_STACK_SIZE			256
#define TASK_ACCEPT_STACK_SIZE				256
#define TASK_IOT_CLIENT_STK_SIZE            2048

#define TASK_AI_CLIENT_STK_SIZE				2048
#define TASK_IMG_SEND_STACK_SIZE			512
#define TASK_GUI_APP_STACK_SIZE				512
#define TASK_NRF_STACK_SIZE					256
#define TASK_MUSIC_STACK_SIZE				2048	
#define TASK_DHCP_SERVER_STACK_SIZE         256
#define NETCAMERA_STK_SIZE                  1024

/*
*********************************************************************************************************
*                               		IRQ_PRIORITY
*如果中断函数里需要打印信息，则此中断优先级要低于uart中断
*systick中断设为最低优先级15，其他都应该在0-14
*********************************************************************************************************
*/

enum IRQ_PRIORITY{
	
	TIM2_IRQn_Priority = 0,   //ir control
	OTG_HS_IRQn_Priority=1,
	USART1_IRQn_Priority = 1,
	USART3_IRQn_Priority = 2,
	SDIO_IRQn_Priority,
	EXTI2_IRQn_Priority,
	EXTI9_5_IRQn_Priority,
	ETH_IRQn_Priority,
	EXTI15_10_IRQn_Priority,
	DMA2_Stream5_IRQn_Priority = 8,
	DMA2_Stream7_IRQn_Priority = 9,
	SPI3_IRQn_Priority = 9,
	DMA2_Stream1_IRQn_Priority = 10,	
	DCMI_IRQn_Priority = 10,
	TIM3_IRQn_Priority = 13,
	TIM4_IRQn_Priority = 13,
	ADC_IRQn_Priority = 14,
	DMA1_Stream6_IRQn_Priority = 14,
	DMA2_Stream0_IRQn_Priority = 14,
	
	SysTick_IRQ_Priority = 15	//
};

/*
*********************************************************************************************************
*                                      kernel 里面常用资源值定义
*建议配合monitor软件确定合适的数值
*********************************************************************************************************
*/
#define EVENTS_MAX			100		//EVENTS_MAX是所有事件的数量之和，包括邮箱、消息队列、信号量、互斥量等
#define TIMER_MAX       		20		//定时器
#define MSG_QUEUE_MAX		16		//消息队列
#define TASKS_MAX			20		//任务
#define OS_TICK_RATE_HZ	100

/*
*********************************************************************************************************
*                                      定义中断向量表地址
* 因低16k地址用于bootloader，我们的代码需要从16k开始，所以需要在启动时将中断向量重新映射到16k地址
* 目前只对F205有效，F103版本不使用bootloader
*********************************************************************************************************
*/
//#define IVECTOR_ADDR 					(16*1024)
#define IVECTOR_ADDR 					(0)		//如果不使用bootloaer（同时将IROM1的start改为0x8000000）

#endif
