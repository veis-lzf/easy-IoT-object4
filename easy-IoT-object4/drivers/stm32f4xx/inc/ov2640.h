#ifndef __OV2640_H
#define __OV2640_H
#include "drivers.h"
#include "sys.h"


//#define OV2640_RST PDout(6) //RST低电平有效
//#define OV2640_PWDN  PEout(5) //PWDN高电平有效

//#define OV2640_RA_DLMT 0XFF   //寄存器bank选择地址
//#define OV2640_DSP     0X00   //选择DSP
//#define OV2640_SENSOR  0X01   //选择SENSOR
//#define OV2640_MID				0X7FA2
//#define OV2640_PID				0X2642
// 

////当选择DSP地址(0XFF=0X00)时,OV2640的DSP寄存器地址映射表
//#define OV2640_DSP_R_BYPASS     0x05
//#define OV2640_DSP_Qs           0x44
//#define OV2640_DSP_CTRL         0x50
//#define OV2640_DSP_HSIZE1       0x51
//#define OV2640_DSP_VSIZE1       0x52
//#define OV2640_DSP_XOFFL        0x53
//#define OV2640_DSP_YOFFL        0x54
//#define OV2640_DSP_VHYX         0x55
//#define OV2640_DSP_DPRP         0x56
//#define OV2640_DSP_TEST         0x57
//#define OV2640_DSP_ZMOW         0x5A
//#define OV2640_DSP_ZMOH         0x5B
//#define OV2640_DSP_ZMHH         0x5C
//#define OV2640_DSP_BPADDR       0x7C
//#define OV2640_DSP_BPDATA       0x7D
//#define OV2640_DSP_CTRL2        0x86
//#define OV2640_DSP_CTRL3        0x87
//#define OV2640_DSP_SIZEL        0x8C
//#define OV2640_DSP_HSIZE2       0xC0
//#define OV2640_DSP_VSIZE2       0xC1
//#define OV2640_DSP_CTRL0        0xC2
//#define OV2640_DSP_CTRL1        0xC3
//#define OV2640_DSP_R_DVP_SP     0xD3
//#define OV2640_DSP_IMAGE_MODE   0xDA
//#define OV2640_DSP_RESET        0xE0
//#define OV2640_DSP_MS_SP        0xF0
//#define OV2640_DSP_SS_ID        0x7F
//#define OV2640_DSP_SS_CTRL      0xF8
//#define OV2640_DSP_MC_BIST      0xF9
//#define OV2640_DSP_MC_AL        0xFA
//#define OV2640_DSP_MC_AH        0xFB
//#define OV2640_DSP_MC_D         0xFC
//#define OV2640_DSP_P_STATUS     0xFE
//#define OV2640_DSP_RA_DLMT      0xFF 

////当选择传感器地址(0XFF=0X01)时,OV2640的DSP寄存器地址映射表
//#define OV2640_SENSOR_GAIN       0x00
//#define OV2640_SENSOR_COM1       0x03
//#define OV2640_SENSOR_REG04      0x04
//#define OV2640_SENSOR_REG08      0x08
//#define OV2640_SENSOR_COM2       0x09
//#define OV2640_SENSOR_PIDH       0x0A
//#define OV2640_SENSOR_PIDL       0x0B
//#define OV2640_SENSOR_COM3       0x0C
//#define OV2640_SENSOR_COM4       0x0D
//#define OV2640_SENSOR_AEC        0x10
//#define OV2640_SENSOR_CLKRC      0x11
//#define OV2640_SENSOR_COM7       0x12
//#define OV2640_SENSOR_COM8       0x13
//#define OV2640_SENSOR_COM9       0x14
//#define OV2640_SENSOR_COM10      0x15
//#define OV2640_SENSOR_HREFST     0x17
//#define OV2640_SENSOR_HREFEND    0x18
//#define OV2640_SENSOR_VSTART     0x19
//#define OV2640_SENSOR_VEND       0x1A
//#define OV2640_SENSOR_MIDH       0x1C
//#define OV2640_SENSOR_MIDL       0x1D
//#define OV2640_SENSOR_AEW        0x24
//#define OV2640_SENSOR_AEB        0x25
//#define OV2640_SENSOR_W          0x26
//#define OV2640_SENSOR_REG2A      0x2A
//#define OV2640_SENSOR_FRARL      0x2B
//#define OV2640_SENSOR_ADDVSL     0x2D
//#define OV2640_SENSOR_ADDVHS     0x2E
//#define OV2640_SENSOR_YAVG       0x2F
//#define OV2640_SENSOR_REG32      0x32
//#define OV2640_SENSOR_ARCOM2     0x34
//#define OV2640_SENSOR_REG45      0x45
//#define OV2640_SENSOR_FLL        0x46
//#define OV2640_SENSOR_FLH        0x47
//#define OV2640_SENSOR_COM19      0x48
//#define OV2640_SENSOR_ZOOMS      0x49
//#define OV2640_SENSOR_COM22      0x4B
//#define OV2640_SENSOR_COM25      0x4E
//#define OV2640_SENSOR_BD50       0x4F
//#define OV2640_SENSOR_BD60       0x50
//#define OV2640_SENSOR_REG5D      0x5D
//#define OV2640_SENSOR_REG5E      0x5E
//#define OV2640_SENSOR_REG5F      0x5F
//#define OV2640_SENSOR_REG60      0x60
//#define OV2640_SENSOR_HISTO_LOW  0x61
//#define OV2640_SENSOR_HISTO_HIGH 0x62


//								
//	    				 
//u8 OV2640_Init(void);//OV2640摄像头初始化
//void OV2640_JPEG_Mode(void);//OV2640切换为JPEG模式
//void OV2640_RGB565_Mode(void);//OV2640切换为RGB565模式
//void OV2640_Auto_Exposure(u8 level);//OV2640自动曝光等级设置
//void OV2640_Light_Mode(u8 mode);//白平衡设置
//void OV2640_Color_Saturation(u8 sat);//色度设置
//void OV2640_Brightness(u8 bright);//亮度设置
//void OV2640_Contrast(u8 contrast);//对比度设置
//void OV2640_Special_Effects(u8 eft);//特效设置
//void OV2640_Color_Bar(u8 sw);//彩条测试

//void OV2640_Window_Set(u16 sx,u16 sy,u16 width,u16 height);//设置图像输出窗口
//u8 OV2640_OutSize_Set(u16 width,u16 height);//设置图像输出大小
//u8 OV2640_ImageWin_Set(u16 offx,u16 offy,u16 width,u16 height);//设置图像开窗大小
//u8 OV2640_ImageSize_Set(u16 width,u16 height);//设置图像尺寸大小

//#endif


#define OV5640_PWDN  	PEout(5)			//POWER DOWN控制信号
#define OV5640_RST  	PDout(6)			//复位控制信号 
////////////////////////////////////////////////////////////////////////////////// 
#define OV5640_ID               0X5640  	//OV5640的芯片ID
 

#define OV5640_ADDR        		0X78		//OV5640的IIC地址
 
//OV5640相关寄存器定义  
#define OV5640_CHIPIDH          0X300A  	//OV5640芯片ID高字节
#define OV5640_CHIPIDL          0X300B  	//OV5640芯片ID低字节
 

								
u8 OV5640_WR_Reg(u16 reg,u8 data);
u8 OV5640_RD_Reg(u16 reg);
void OV5640_PWDN_Set(u8 sta);
u8 OV5640_Init(void);  
void OV5640_JPEG_Mode(void);
void OV5640_RGB565_Mode(void);
void OV5640_Exposure(u8 exposure);
void OV5640_Light_Mode(u8 mode);
void OV5640_Color_Saturation(u8 sat);
void OV5640_Brightness(u8 bright);
void OV5640_Contrast(u8 contrast);
void OV5640_Sharpness(u8 sharp);
void OV5640_Special_Effects(u8 eft);
void OV5640_Test_Pattern(u8 mode);
void OV5640_Flash_Ctrl(u8 sw);
u8 OV5640_OutSize_Set(u16 offx,u16 offy,u16 width,u16 height);
u8 OV5640_ImageWin_Set(u16 offx,u16 offy,u16 width,u16 height); 
u8 OV5640_Focus_Init(void);
u8 OV5640_Focus_Single(void);
u8 OV5640_Focus_Constant(void);
#endif


