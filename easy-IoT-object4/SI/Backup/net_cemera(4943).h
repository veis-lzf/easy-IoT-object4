#ifndef __NET_CEMERA_H
#define __NET_CEMERA_H

 #include "type.h"

#define NETCAM_BUF_SIZE  28 * 1024 //��������С(Bytes)

typedef struct
{	
	uint32_t		FIFO_Size;         //��������С
	 
	uint8_t*		pFIFO_Buffer;     //������
	
	uint32_t		FrameSize;        //����֡��С
	
	uint8_t*		FrameBuffer;      //����֡
	
	uint8_t*		FrameHeadPos;     //֡ͷ
	
	uint8_t*		FrameTailPos;     //֡β

}NETCAMERA_HandleTypeDef;


void netcamera_init(void);		   //��������ͷ��ʼ��(������������ͷ�߳�)
void netcamera_task(void );
void  cemera_cofig(void);
void  rgb565_test(void);
void   jpeg_test(void);

#endif

