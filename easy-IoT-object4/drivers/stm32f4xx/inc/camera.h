#ifndef _CAMERA_H
#define _CAMERA_H


/* Exported constants --------------------------------------------------------*/
#define DCMI_DR_ADDRESS       0x50050028
#define FSMC_LCD_ADDRESS      0x68000002
#define NOKEY                 0
#define SEL                   1
#define UP                    2
#define DOWN                  3

#define Delay sleep

#define JPEG_FRAME_SIZE	        250

#define SEND_PENDING_TIMEOUT    3000

/* Exported types ------------------------------------------------------------*/
/* Camera devices enumeration */
typedef enum   
{
  OV9655_CAMERA            =   0x00,	 /* Use OV9655 Camera */
  OV2640_CAMERA            =   0x01      /* Use OV2640 Camera */
}Camera_TypeDef;

/* Image Sizes enumeration */
typedef enum   
{
  BMP_QQVGA,
  BMP_QVGA,
  JPEG_160x120,
  JPEG_176x144,
  JPEG_320x240,
  JPEG_352x288,
  JPEG_640x480,
  JPEG_800x600,
  JPEG_1024x768,
  JPEG_FORMAT_NUM
}ImageFormat_TypeDef;


struct CAMERA_CFG
{
	char *dma_buff[2];
	int dma_index;
	int jpeg_size;
	int frame_size;
	int frame_num;
    
  	int reomote_frame_size;
	int reomote_frame_num;

	int send_pending_timeout;
	int cur_dev_index;
	wait_event_t img_event;
	msg_q_t img_queue;
	
	mutex_t img_mutex;
	int pos;
	bool remote_capture_img;
};

//void start_capture_img(void);
//void stop_capture_img(void);
int open_camera(void);
int close_camera(void);
void camera_button_event(int event);
int switch_jpeg_size(void);
int send_img_local(uint8_t *buff, int size);
extern void rgb565_test(void);

extern struct CAMERA_CFG camera_cfg;

#endif

