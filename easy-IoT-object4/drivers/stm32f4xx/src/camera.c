#define DEBUG
//#define DEBUG_TASK
#include "drivers.h"
#include "app.h"
#include "api.h"
#include  "mq2.h"
#include  "debug.h"






#define CAPTURE_MODE_CIR		 			0	

/*
*捕获模式，我们提供两种捕获列子，
*循环模式:图片通过dma自动读取，循环保存到缓冲区内存，我们可以在帧中断里面把图片读出来
*双缓冲方式:分配两块缓冲区，用于轮流保存图片，这种模式很简单，但是浪费更多内存，并且捕获到一个图片后
*需要暂停捕获以重新配置DMA
*
*/
#define CAPTURE_MODE 			CAPTURE_MODE_CIR    //使用循环模式



#define CAMERA_IMG_QUEUE_LEN	1

extern  u8 * p;
    

extern void  cemera_cofig(void);
extern void  rgb565_test(void);


int   camera_mode( int  CAMERA_MODE  );

struct CAMERA_CFG camera_cfg;
extern wait_event_t client_camera_event;

void img_send_thread(void *arg);
int img_send_thread_id;
                



int deinit_camera()
{
	DCMI_Cmd(DISABLE);

	DMA_Cmd(DMA2_Stream1, DISABLE);

	DCMI_DeInit();

	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, DISABLE);
	
	return 0;
}


extern void web_server_init(void);
int open_camera()
{ 
	cemera_cofig();
    web_server_init();
	return 0;
}


void DMA2_Stream1_IRQHandler() 
{
	enter_interrupt();

	if(DMA_GetFlagStatus(DMA2_Stream1, DMA_FLAG_HTIF1))
	{
		DMA_ClearFlag(DMA2_Stream1,  DMA_FLAG_HTIF1);
		
	}

	if(DMA_GetFlagStatus(DMA2_Stream1, DMA_FLAG_TCIF1))
	{
         printf("\r\n ****进入DMA接收中断2  *****\r\n");
		DMA_ClearFlag(DMA2_Stream1,  DMA_FLAG_TCIF1);
	}

	if(DMA_GetFlagStatus(DMA2_Stream1, DMA_FLAG_TEIF1))
	{
		p_err("dcmi dma err");
		DMA_ClearFlag(DMA2_Stream1,  DMA_FLAG_TEIF1);
	}

 	exit_interrupt(0);
}





extern int mqtt_connected;

uint8_t *web_data = NULL;
mutex_t web_sig = NULL;





void img_send_thread(void *arg)
{

 
    while(1)
    {   
           p_dbg_enter_task;
           
   
        p_dbg_exit_task;
    }
}



