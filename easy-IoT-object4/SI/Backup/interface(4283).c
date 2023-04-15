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



wait_event_t interface_event = NULL;            //�Ƿ��������ʶ��ģʽ


void interface_thread(void *parameter)
{  
    int res;
	interface_event = init_event(); 
	while (1) {
		sleep(1000);
		printf("����interface_thread \r\n");
		Show_Str(10,280,300,24,(unsigned char *)"WIFI����:",24,0);	
		Show_Str(380,280,100,24,(unsigned char *)"ʶ��ģʽ:",24,0);   
		res = wait_event_timeout(interface_event, 5000);    
		if(res != 0) {       
			 continue;          
		}   
	}
}


void interface_display(void )
{
   font_init();     //�ֿ��ʼ��
   LCD_Init();		      //��ʼ��lcd��Ļ
   POINT_COLOR=RED;      //��������Ϊ��ɫ 
   show_pic_init(1);
   Show_Str(200,130,150,24,(unsigned char *)"����ʶ����",24,0);
   Show_Str(10,280,300,24,(unsigned char *)"WIFI����:",24,0);	
   Show_Str(380,280,100,24,(unsigned char *)"ʶ��ģʽ:",24,0);   
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
