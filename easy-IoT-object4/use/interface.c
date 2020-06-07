#define DEBUG
#include <api.h>
#include <lwip/api.h>
#include <lwip/inet.h>
#include <lwip/sockets.h>
#include "debug.h"
#include "wifi.h"
#include  "usr_cfg.h"
#include  "app.h"
#include  "drivers.h"



wait_event_t interface_event = NULL;            //是否进入物体识别模式


void interface_thread(void *parameter)
{  
    int res;
	interface_event = init_event(); 
	while (1) {
		sleep(1000);
		printf("进入interface_thread \r\n");
		Show_Str(10,280,300,24,(unsigned char *)"WIFI设置:",24,0);	
		Show_Str(380,280,100,24,(unsigned char *)"识别模式:",24,0);   
		res = wait_event_timeout(interface_event, 5000);    
		if(res != 0) {       
			 continue;          
		}   
	}
}


void interface_display(void )
{
   font_init();     //字库初始化
   LCD_Init();		      //初始化lcd屏幕
   POINT_COLOR=RED;      //设置字体为红色 
   show_pic_init(1);
   Show_Str(200,130,150,24,(unsigned char *)"智能识别器",24,0);
   Show_Str(10,280,300,24,(unsigned char *)"WIFI设置:",24,0);	
   Show_Str(380,280,100,24,(unsigned char *)"识别模式:",24,0);   
}




void interface_init(void)
{

//         thread_create(interface_thread,
//                        0,
//                        TASK_NRF_PRIO,
//                        0,
//                        TASK_IMG_SEND_STACK_SIZE,
//                        "web_server_thread");

}
