#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"

#include "web_cfg.h"
#include "text.h"



#define VIRSION		"V2.0"
  u8 * p;
  
extern void baidu_ai_image_client(void);
extern void baidu_ai_voice_client(void);

void main_thread(void *pdata)
{
	int ret;
	#ifdef DEBUG
	RCC_ClocksTypeDef RCC_ClocksStatus;
	#endif
	//init RNG,Timer,Systick
	driver_misc_init();
	//init LED, Button, relays for ONENET
	usr_gpio_init();
    

  
#ifdef OS_UCOS
#ifdef UCOS_V3
	OSStatTaskCPUUsageInit((OS_ERR*)&ret);
#else
	OSStatInit();
#endif
#endif
	uart1_init(); 
//	uart3_init();

#ifdef DEBUG
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	p_dbg("SYSCLK_Frequency:%d,HCLK_Frequency:%d,PCLK1_Frequency:%d,PCLK2_Frequency:%d,ADCCLK_Frequency:%d\n", 
		RCC_ClocksStatus.SYSCLK_Frequency, 
		RCC_ClocksStatus.HCLK_Frequency, 
		RCC_ClocksStatus.PCLK1_Frequency, 
		RCC_ClocksStatus.PCLK2_Frequency, 
		0);
#endif
    //init the SPI flash
	m25p80_init();  
    LCD_Init();		      //初始化lcd屏幕
    POINT_COLOR=RED;      
     
     
//     show_pic_init();
    interface_display();
#if 0
	NRF24L01_Init();    //not support
#endif
#if SUPPORT_AUDIO
    //reset audio control structure
	audio_dev_init();
#endif
    //init a thread for timeout task 
	init_work_thread();


	//read the configurations 
//	load_web_cfg(&web_cfg);     //把flash里面对应的的参数读出来，放到全局变量web_cfg里面
    read_web_cfg(&web_cfg);     //把flash里面对应的的参数读出来，放到全局变量web_cfg里面
#if ENABLE_LCD == 1
	test_gui();
#endif
    //init LwIP kernel
	init_lwip();          //初始化  LWIP 协议栈   工作于多线程，不能基于回调去完成   同一接口，用socket编程
#if SUPPORT_WIFI
	ret = init_wifi();    //初始化wifi芯片  
	if(ret == 0)
	{
	    //add wifi netif
        lwip_netif_init();  //创建无线网卡对应的netif结构，netif：lwip用来管理网线网卡的一个结构
	
	}else{
	p_err("init wifi faild!"); 
#if ENABLE_LCD == 1
	gui_show_info("wifi invalid");
#endif
	}
#endif


#if SUPPORT_AUDIO
	open_audio();       //音频相关的初始化
#endif
	init_key_dev();
//	init_rtc();

#if	!SUPPORT_AUDIO
	lamp_pwm_init(); //init PWM to control lamp, use TIM4 which is conflict with AUDIO
#endif


    misc_init();

//    interface_init();
   
    init_sensers(); 

//   my_hanzi_display();
//    show_pic_init();

    
    

#if SUPPORT_CAM	
	open_camera();     //摄像头的初始化
    
     MUTE_ON;
  
#endif
#if SURPPORT_USB
	init_usb();
#endif
	p_dbg("code version:%s", VIRSION);
	p_dbg("startup time:%d.%d S", os_time_get() / 1000, os_time_get() % 1000);
	p_dbg("mem pool1 remain:%d/%d", mem_get_free() , mem_get_size());
	p_dbg("mem pool2 remain:%d/%d", mem_get_free2() , mem_get_size2());
	
#if SUPPORT_WIFI
        //low power mode for WIFI
	test_power_save_enable();        //如果想降低WIFI模块功耗，请咨询这里
#endif

 
//    init_sensers(); 
#if ONENET_ENABLE
    //update senser value automatically when Onenet enabled
    start_senser_poll();
#endif

#if ONENET_ENABLE
    init_onenet_edpclient();
#endif

#if ALI_IOT_ENABLE
    aliyun_mqtt();
#endif

//  web_server_init();

#if BAIDU_AI_FACE_ENABLE
    baidu_ai_image_client();
#endif

#if BAIDU_AI_VOICE_ENABLE
    baidu_ai_voice_client();
#endif
    
    main_process();

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
        //init memory configurations
	_mem_init();
	msg_q_init();
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

/**********************
=================================================
任务名      任务状态 优先级   剩余栈 任务序号
printf_thread  	R				1		318				2
IDLE           	R				0		81				3
tcpip_thread   	B				18		319				6
eth_int_thread 	B				20		165				9
MOAL_WORK_QUEUE	B				22		66				7
main_thread    	B				11		162				1
timer_tasklet  	B				17		218				5
adc recv       	B				10		955				10
AI audio client	B				6		1975			13
Tmr Svc        	B				19		59				4
baidu AI client	B				5		417			    12
woal_reassoc_se	B				15		315				8
img_snd        	B				12		470			    11





B:  BLOCK       阻塞
R:  READY       就绪
S:  SUSPENDED   挂起
D:  DELETE      删除

 printf("\r\n ****退出 img_send_thread *****\r\n");
 
 
**********************/



