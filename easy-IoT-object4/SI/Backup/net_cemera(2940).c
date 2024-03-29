#define DEBUG
//#define DEBUG_TASK#include "app.h"
#include "drivers.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include  "debug.h"
#include "app.h"
#include "driver_misc.h"



extern wait_event_t asd;


extern void delay_us(uint32_t us);

sem_t sem_camera = NULL;
NETCAMERA_HandleTypeDef hcamera;


/*-----------------------------------------------------------------------------------*/
static err_t	netcamera_malloc(NETCAMERA_HandleTypeDef *phcamera);
static void 	netcamera_free(NETCAMERA_HandleTypeDef *phcamera);
static void 	netcamera_get_frame(void);
static void 	netcamera_open(void);
static void 	netcamera_close(void);

/*-----------------------------------------------------------------------------------*/
wait_event_t camera_mode_event = NULL;
uint8_t  *image_data = NULL;
int   cemera_rawlen = 0;
wait_event_t client_pic_event = NULL;


void netcamera_task(void )
{
    int b, irq_flag1, i, j;
    static int  K_FIFO_BUF = 0;
    u8 *p;
    camera_mode_event = init_event();  // 创建一个二值信号量     camera_mode_event    通过按键触发
    client_pic_event = init_event();   // 创建一个二值信号量     client_pic_event

    printf("网络摄像头内存申请前内存\r\n");
    printf("mem pool1 remain:%d/%dbytes\r\n", mem_get_free(), mem_get_size());
    printf("mem pool2 remain:%d/%dbytes\r\n", mem_get_free2(), mem_get_size2());

    if(netcamera_malloc(&hcamera) == ERR_OK) {
        hcamera.FIFO_Size 		= NETCAM_BUF_SIZE;
        hcamera.FrameHeadPos 	= hcamera.pFIFO_Buffer;
        hcamera.FrameTailPos 	= hcamera.pFIFO_Buffer;
    }
    else {
        netcamera_close();
    }

    while(1)    //等待连接
    {
        netcamera_open();
        printf("图片上传\r\n");
        b = 1;
		
        while(sem_wait(&sem_camera, 3000) == 0) {   
			printf("获取到帧头\r\n");
            netcamera_get_frame();
            if(hcamera.FrameBuffer != NULL) {
                sem_destory(&sem_camera);
                LCD_Init();
                rgb565_test();
				
                printf("捕获到图片\r\n");
                printf("%d\r\n", hcamera.FrameSize);
                mem_free(hcamera.pFIFO_Buffer);
                cemera_rawlen = hcamera.FrameSize;
                image_data = hcamera.FrameBuffer;                 //把收集到的图片指向发送的地址  image_data

                printf("mem pool1 remain:%d/%dbytes\r\n", mem_get_free(), mem_get_size());
                printf("mem pool2 remain:%d/%dbytes\r\n", mem_get_free2(), mem_get_size2());
                return;
            }
        }
    }
}



/*-----------------------------------------------------------------------------------*/
static err_t netcamera_malloc(NETCAMERA_HandleTypeDef *phcamera)
{
    phcamera->pFIFO_Buffer = (uint8_t *)mem_malloc(NETCAM_BUF_SIZE);

    if(phcamera->pFIFO_Buffer != NULL)
    {
        printf("网络摄像头内存申请成功\r\n");
        printf("mem pool1 remain:%d/%dbytes\r\n", mem_get_free(), mem_get_size());
        printf("mem pool2 remain:%d/%dbytes\r\n", mem_get_free2(), mem_get_size2());
        return ERR_OK;
    }

    printf("网络摄像头内存申请失败\r\n");
    printf("mem pool1 remain:%d/%dbytes\r\n", mem_get_free(), mem_get_size());
    printf("mem pool2 remain:%d/%dbytes\r\n", mem_get_free2(), mem_get_size2());
    return ERR_MEM;
}

static void netcamera_free(NETCAMERA_HandleTypeDef *phcamera)
{
    mem_free(phcamera->pFIFO_Buffer);
    printf("网络摄像头释放内存成功\r\n");
    printf("mem pool1 remain:%d/%dbytes\r\n", mem_get_free(), mem_get_size());
    printf("mem pool2 remain:%d/%dbytes\r\n", mem_get_free2(), mem_get_size2());
}
/*-----------------------------------------------------------------------------------*/

static void netcamera_open(void) // 调用处理函数，产生一个信号量
{
	jpeg_test();
}
/*-----------------------------------------------------------------------------------*/
static void netcamera_close(void)
{
    /*释放内存*/
    netcamera_free(&hcamera);
    /*关闭传输*/
    DCMI_Stop();
}
/*-----------------------------------------------------------------------------------*/
static void
netcamera_get_frame(void)
{
    /* Get a frame in the same dma transmitting procedure */
    if(hcamera.FrameTailPos > hcamera.FrameHeadPos &&
            *hcamera.FrameHeadPos == 0xFF &&
            *(hcamera.FrameHeadPos + 1) == 0xD8) {
        hcamera.FrameSize = hcamera.FrameTailPos - hcamera.FrameHeadPos;
        hcamera.FrameBuffer = mem_malloc(hcamera.FrameSize);
        if(hcamera.FrameBuffer != NULL) {
            memcpy(hcamera.FrameBuffer, hcamera.FrameHeadPos, hcamera.FrameSize);
            printf("成功捕获图片\r\n");
        }
        else {
            mem_free(hcamera.FrameBuffer);
            printf("1:FrameBuffer mem_malloc fail\r\n");
        }
    }

    /* Get a frame in two different dma transmitting procedures */
    else if(hcamera.FrameTailPos < hcamera.FrameHeadPos &&
            *hcamera.FrameHeadPos == 0xFF &&
            *(hcamera.FrameHeadPos + 1) == 0xD8) {
        hcamera.FrameSize = hcamera.FIFO_Size + hcamera.FrameTailPos - hcamera.FrameHeadPos;
        hcamera.FrameBuffer = mem_malloc(hcamera.FrameSize);
        if(hcamera.FrameBuffer != NULL) {
            uint32_t FramePart1, FramePart2;

            FramePart1 = hcamera.pFIFO_Buffer + hcamera.FIFO_Size - hcamera.FrameHeadPos;
            FramePart2 = hcamera.FrameSize - FramePart1;
            memcpy(hcamera.FrameBuffer, hcamera.FrameHeadPos, FramePart1);
            memcpy(hcamera.FrameBuffer + FramePart1, hcamera.pFIFO_Buffer, FramePart2);
        }
        else {
            mem_free(hcamera.FrameBuffer);
            printf("2:FrameBuffer mem_malloc fail\r\n");
        }
    }

    /* Point to the next frame header */
    if(hcamera.FrameTailPos == hcamera.FrameBuffer + hcamera.FIFO_Size)
        hcamera.FrameHeadPos = hcamera.FrameBuffer;
    else
        hcamera.FrameHeadPos = hcamera.FrameTailPos;

//	printf("FrameSize = %dBytes\r\n", hcamera.FrameSize);

    DCMI_CaptureCmd(ENABLE);//DCMI捕获使能
}
/*-----------------------------------------------------------------------------------*/
//DCMI帧中断服务函数
void DCMI_IRQHandler(void)
{
    int need_sched = 1;
    enter_interrupt();

	 //捕获到一帧图像
    if(DCMI_GetITStatus(DCMI_IT_FRAME) == SET) {
        hcamera.FrameTailPos = hcamera.pFIFO_Buffer + hcamera.FIFO_Size - DMA2_Stream1->NDTR * 4;

        /* Check whether capture a correct frame */
        if(*hcamera.FrameHeadPos == 0xFF &&
                *(hcamera.FrameHeadPos + 1) == 0xD8) {
            DCMI_CaptureCmd(DISABLE);//DCMI捕获关闭
            sem_post(&sem_camera);
        }
        else {
            if(hcamera.FrameTailPos == hcamera.FrameBuffer + hcamera.FIFO_Size) {
                hcamera.FrameHeadPos = hcamera.FrameBuffer;
			}
            else {
                hcamera.FrameHeadPos = hcamera.FrameTailPos;
            }
        }
        DCMI_ClearITPendingBit(DCMI_IT_FRAME);//清除帧中断
    }
    exit_interrupt(need_sched);
}
/**
 * @brief   jpeg
 * @param
 * @param
*/
void jpeg_test(void)
{
    u8 fac;
    DCMI_CaptureCmd(DISABLE);//DCMI捕获使关闭

    OV5640_RGB565_Mode();	//RGB565模式
    OV5640_JPEG_Mode();		//JPEG模式
    sem_init(&sem_camera, 0);
    /*DCMI配置*/
    DCMI_JPEGCmd(ENABLE);
    /*DCMI_DMA配置*/
    DCMI_DMA_Init((uint32_t*)hcamera.pFIFO_Buffer,  NETCAM_BUF_SIZE / 4, DMA_MemoryDataSize_Word, DMA_MemoryInc_Enable);
    OV5640_OutSize_Set(12, 4, 240, 180); //设置输出尺寸
    /*启动传输*/
    DCMI_Start();
    printf("图片上传模式\r\n");
}


/**
 * @brief   rgb565
 * @param
 * @param
*/
void rgb565_test(void)
{
    u8 fac;

    OV5640_RGB565_Mode();	//RGB565模式
    My_DCMI_Init();			//DCMI配置
    DCMI_DMA_Init((uint32_t *)&LCD->LCD_RAM, 1, DMA_MemoryDataSize_HalfWord, DMA_MemoryInc_Disable); //DCMI DMA配置
    DCMI_CaptureCmd(ENABLE);//DCMI捕获使关闭

    fac = 480 / lcddev.height; //得到比例因子
    OV5640_OutSize_Set((640 - fac * lcddev.width) / 2, (480 - fac * lcddev.height) / 2, lcddev.width, lcddev.height);

    DCMI_Start(); 		//启动传输
    printf("图片LCD模式\r\n");
}
/**
 * @brief   rgb565
 * @param
 * @param
*/
void cemera_cofig(void)
{

    while(OV5640_Init())//初始化OV2640
    {
        printf("ov5640初始化失败\r\n");

    }
    printf("ov5640初始化\r\n");
    //自动对焦初始化
    OV5640_RGB565_Mode();	//RGB565模式
    OV5640_Focus_Init();

    OV5640_Light_Mode(0);	//自动模式
    OV5640_Color_Saturation(3);//色彩饱和度0
    OV5640_Brightness(4);	//亮度0
    OV5640_Contrast(3);		//对比度0
    OV5640_Sharpness(33);	//自动锐度
    OV5640_Focus_Constant();//启动持续对焦
    printf("摄像头配置\r\n");
}
/*-----------------------------------------------------------------------------------*/
void netcamera_init(void)
{

}
/*-----------------------------------------------------------------------------------*/
//#endif

