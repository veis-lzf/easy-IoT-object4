#ifndef __NET_CEMERA_H
#define __NET_CEMERA_H

 #include "type.h"

#define NETCAM_BUF_SIZE  28 * 1024 //缓冲区大小(Bytes)

typedef struct
{	
	uint32_t		FIFO_Size;         //缓冲区大小
	 
	uint8_t*		pFIFO_Buffer;     //缓冲区
	
	uint32_t		FrameSize;        //数据帧大小
	
	uint8_t*		FrameBuffer;      //数据帧
	
	uint8_t*		FrameHeadPos;     //帧头
	
	uint8_t*		FrameTailPos;     //帧尾

}NETCAMERA_HandleTypeDef;


void netcamera_init(void);		   //网络摄像头初始化(创建网络摄像头线程)
void netcamera_task(void );
void  cemera_cofig(void);
void  rgb565_test(void);
void   jpeg_test(void);

#endif

