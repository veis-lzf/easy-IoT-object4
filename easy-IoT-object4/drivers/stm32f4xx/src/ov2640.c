#define DEBUG
#include "drivers.h"
//#include "ov2640_reg.h"
#include "ov5640af.h"
#include "ov2640_reg.h"
#include "debug.h"

extern void delay_ms(uint32_t us);
//OV2640初始化
//u8 OV2640_Init(void)
//{
//	u8 i;
//	u16 reg;
//
//   u16 OV2640_ID;//记录制造商ID或产品ID
//
//	GPIO_InitTypeDef  GPIO_InitStructure;
//
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOD,E时钟
//	//OV_RESET OV_PWDN 设置
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //输出模式
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;//25MHz
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
//
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;//GPIOD6推挽输出
//  GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//GPIOE5推挽输出
//  GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化
//	OV2640_PWDN=0;	//POWER ON
//	sleep(10);
//	OV2640_RST=0;	//复位OV2640
//	sleep(10);
//	OV2640_RST=1;	//结束复位
//    SCCB_Init();        		//初始化SCCB 的IO口
//	SCCB_WR_Reg(OV2640_DSP_RA_DLMT, 0x01);	//操作sensor寄存器
// 	SCCB_WR_Reg(OV2640_SENSOR_COM7, 0x80);	//软复位OV2640
//	sleep(50);
//	reg=SCCB_RD_Reg(OV2640_SENSOR_MIDH);	//读取厂家ID 高八位
//	reg<<=8;
//	reg|=SCCB_RD_Reg(OV2640_SENSOR_MIDL);	//读取厂家ID 低八位
//	if(reg!=OV2640_MID)
//	{
//		printf("MID:%4x\r\n",reg);
//		return 1;
//	}
//	reg=SCCB_RD_Reg(OV2640_SENSOR_PIDH);	//读取厂家ID 高八位
//	reg<<=8;
//	reg|=SCCB_RD_Reg(OV2640_SENSOR_PIDL);	//读取厂家ID 低八位
//	if(reg!=OV2640_PID)
//	{
//		printf("HID:%4x\r\n",reg);
//		return 2;
//	}
// 	//初始化 OV2640,采用SXGA分辨率(1600*1200)
//	for(i=0;i<sizeof(ov2640_svga_init_reg_tbl)/2;i++)
//	{
//	   	SCCB_WR_Reg(ov2640_svga_init_reg_tbl[i][0],ov2640_svga_init_reg_tbl[i][1]);
// 	}
//  	return 0x00; 	//ok
//}


////OV2640切换为JPEG模式
//void OV2640_JPEG_Mode(void)
//{
//	u16 i=0;
//	//设置:YUV422格式
//	for(i=0;i<(sizeof(ov2640_yuv422_reg_tbl)/2);i++)
//	{
//		SCCB_WR_Reg(ov2640_yuv422_reg_tbl[i][0],ov2640_yuv422_reg_tbl[i][1]);
//	}
//
//	//设置:输出JPEG数据
//	for(i=0;i<(sizeof(ov2640_jpeg_reg_tbl)/2);i++)
//	{
//		SCCB_WR_Reg(ov2640_jpeg_reg_tbl[i][0],ov2640_jpeg_reg_tbl[i][1]);
//	}
//}


////OV2640切换为RGB565模式
//void OV2640_RGB565_Mode(void)
//{
//	u16 i=0;
//	//设置:RGB565输出
//	for(i=0;i<(sizeof(ov2640_rgb565_reg_tbl)/2);i++)
//	{
//		SCCB_WR_Reg(ov2640_rgb565_reg_tbl[i][0],ov2640_rgb565_reg_tbl[i][1]);
//	}
//}


////自动曝光设置参数表,支持5个等级
//const static u8 OV2640_AUTOEXPOSURE_LEVEL[5][8]=
//{
//	{
//		0xFF,0x01,
//		0x24,0x20,
//		0x25,0x18,
//		0x26,0x60,
//	},
//	{
//		0xFF,0x01,
//		0x24,0x34,
//		0x25,0x1c,
//		0x26,0x00,
//	},
//	{
//		0xFF,0x01,
//		0x24,0x3e,
//		0x25,0x38,
//		0x26,0x81,
//	},
//	{
//		0xFF,0x01,
//		0x24,0x48,
//		0x25,0x40,
//		0x26,0x81,
//	},
//	{
//		0xFF,0x01,
//		0x24,0x58,
//		0x25,0x50,
//		0x26,0x92,
//	},
//};
////OV2640自动曝光等级设置
////level:0~4
//void OV2640_Auto_Exposure(u8 level)
//{
//	u8 i;
//	u8 *p=(u8*)OV2640_AUTOEXPOSURE_LEVEL[level];
//	for(i=0;i<4;i++)
//	{
//		SCCB_WR_Reg(p[i*2],p[i*2+1]);
//	}
//}
////白平衡设置
////0:自动
////1:太阳sunny
////2,阴天cloudy
////3,办公室office
////4,家里home
//void OV2640_Light_Mode(u8 mode)
//{
//	u8 regccval=0X5E;//Sunny
//	u8 regcdval=0X41;
//	u8 regceval=0X54;
//	switch(mode)
//	{
//		case 0://auto
//			SCCB_WR_Reg(0XFF,0X00);
//			SCCB_WR_Reg(0XC7,0X10);//AWB ON
//			return;
//		case 2://cloudy
//			regccval=0X65;
//			regcdval=0X41;
//			regceval=0X4F;
//			break;
//		case 3://office
//			regccval=0X52;
//			regcdval=0X41;
//			regceval=0X66;
//			break;
//		case 4://home
//			regccval=0X42;
//			regcdval=0X3F;
//			regceval=0X71;
//			break;
//	}
//	SCCB_WR_Reg(0XFF,0X00);
//	SCCB_WR_Reg(0XC7,0X40);	//AWB OFF
//	SCCB_WR_Reg(0XCC,regccval);
//	SCCB_WR_Reg(0XCD,regcdval);
//	SCCB_WR_Reg(0XCE,regceval);
//}
////色度设置
////0:-2
////1:-1
////2,0
////3,+1
////4,+2
//void OV2640_Color_Saturation(u8 sat)
//{
//	u8 reg7dval=((sat+2)<<4)|0X08;
//	SCCB_WR_Reg(0XFF,0X00);
//	SCCB_WR_Reg(0X7C,0X00);
//	SCCB_WR_Reg(0X7D,0X02);
//	SCCB_WR_Reg(0X7C,0X03);
//	SCCB_WR_Reg(0X7D,reg7dval);
//	SCCB_WR_Reg(0X7D,reg7dval);
//}
////亮度设置
////0:(0X00)-2
////1:(0X10)-1
////2,(0X20) 0
////3,(0X30)+1
////4,(0X40)+2
//void OV2640_Brightness(u8 bright)
//{
//  SCCB_WR_Reg(0xff, 0x00);
//  SCCB_WR_Reg(0x7c, 0x00);
//  SCCB_WR_Reg(0x7d, 0x04);
//  SCCB_WR_Reg(0x7c, 0x09);
//  SCCB_WR_Reg(0x7d, bright<<4);
//  SCCB_WR_Reg(0x7d, 0x00);
//}
////对比度设置
////0:-2
////1:-1
////2,0
////3,+1
////4,+2
//void OV2640_Contrast(u8 contrast)
//{
//	u8 reg7d0val=0X20;//默认为普通模式
//	u8 reg7d1val=0X20;
//  	switch(contrast)
//	{
//		case 0://-2
//			reg7d0val=0X18;
//			reg7d1val=0X34;
//			break;
//		case 1://-1
//			reg7d0val=0X1C;
//			reg7d1val=0X2A;
//			break;
//		case 3://1
//			reg7d0val=0X24;
//			reg7d1val=0X16;
//			break;
//		case 4://2
//			reg7d0val=0X28;
//			reg7d1val=0X0C;
//			break;
//	}
//	SCCB_WR_Reg(0xff,0x00);
//	SCCB_WR_Reg(0x7c,0x00);
//	SCCB_WR_Reg(0x7d,0x04);
//	SCCB_WR_Reg(0x7c,0x07);
//	SCCB_WR_Reg(0x7d,0x20);
//	SCCB_WR_Reg(0x7d,reg7d0val);
//	SCCB_WR_Reg(0x7d,reg7d1val);
//	SCCB_WR_Reg(0x7d,0x06);
//}
////特效设置
////0:普通模式
////1,负片
////2,黑白
////3,偏红色
////4,偏绿色
////5,偏蓝色
////6,复古
//void OV2640_Special_Effects(u8 eft)
//{
//	u8 reg7d0val=0X00;//默认为普通模式
//	u8 reg7d1val=0X80;
//	u8 reg7d2val=0X80;
//	switch(eft)
//	{
//		case 1://负片
//			reg7d0val=0X40;
//			break;
//		case 2://黑白
//			reg7d0val=0X18;
//			break;
//		case 3://偏红色
//			reg7d0val=0X18;
//			reg7d1val=0X40;
//			reg7d2val=0XC0;
//			break;
//		case 4://偏绿色
//			reg7d0val=0X18;
//			reg7d1val=0X40;
//			reg7d2val=0X40;
//			break;
//		case 5://偏蓝色
//			reg7d0val=0X18;
//			reg7d1val=0XA0;
//			reg7d2val=0X40;
//			break;
//		case 6://复古
//			reg7d0val=0X18;
//			reg7d1val=0X40;
//			reg7d2val=0XA6;
//			break;
//	}
//	SCCB_WR_Reg(0xff,0x00);
//	SCCB_WR_Reg(0x7c,0x00);
//	SCCB_WR_Reg(0x7d,reg7d0val);
//	SCCB_WR_Reg(0x7c,0x05);
//	SCCB_WR_Reg(0x7d,reg7d1val);
//	SCCB_WR_Reg(0x7d,reg7d2val);
//}
////彩条测试
////sw:0,关闭彩条
////   1,开启彩条(注意OV2640的彩条是叠加在图像上面的)
//void OV2640_Color_Bar(u8 sw)
//{
//	u8 reg;
//	SCCB_WR_Reg(0XFF,0X01);
//	reg=SCCB_RD_Reg(0X12);
//	reg&=~(1<<1);
//	if(sw)reg|=1<<1;
//	SCCB_WR_Reg(0X12,reg);
//}
////设置图像输出窗口
////sx,sy,起始地址
////width,height:宽度(对应:horizontal)和高度(对应:vertical)
//void OV2640_Window_Set(u16 sx,u16 sy,u16 width,u16 height)
//{
//	u16 endx;
//	u16 endy;
//	u8 temp;
//	endx=sx+width/2;	//V*2
// 	endy=sy+height/2;
//
//	SCCB_WR_Reg(0XFF,0X01);
//	temp=SCCB_RD_Reg(0X03);				//读取Vref之前的值
//	temp&=0XF0;
//	temp|=((endy&0X03)<<2)|(sy&0X03);
//	SCCB_WR_Reg(0X03,temp);				//设置Vref的start和end的最低2位
//	SCCB_WR_Reg(0X19,sy>>2);			//设置Vref的start高8位
//	SCCB_WR_Reg(0X1A,endy>>2);			//设置Vref的end的高8位
//
//	temp=SCCB_RD_Reg(0X32);				//读取Href之前的值
//	temp&=0XC0;
//	temp|=((endx&0X07)<<3)|(sx&0X07);
//	SCCB_WR_Reg(0X32,temp);				//设置Href的start和end的最低3位
//	SCCB_WR_Reg(0X17,sx>>3);			//设置Href的start高8位
//	SCCB_WR_Reg(0X18,endx>>3);			//设置Href的end的高8位
//}
////设置图像输出大小
////OV2640输出图像的大小(分辨率),完全由改函数确定
////width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
////返回值:0,设置成功
////    其他,设置失败
//u8 OV2640_OutSize_Set(u16 width,u16 height)
//{
//	u16 outh;
//	u16 outw;
//	u8 temp;
//	if(width%4)return 1;
//	if(height%4)return 2;
//	outw=width/4;
//	outh=height/4;
//	SCCB_WR_Reg(0XFF,0X00);
//	SCCB_WR_Reg(0XE0,0X04);
//	SCCB_WR_Reg(0X5A,outw&0XFF);		//设置OUTW的低八位
//	SCCB_WR_Reg(0X5B,outh&0XFF);		//设置OUTH的低八位
//	temp=(outw>>8)&0X03;
//	temp|=(outh>>6)&0X04;
//	SCCB_WR_Reg(0X5C,temp);				//设置OUTH/OUTW的高位
//	SCCB_WR_Reg(0XE0,0X00);
//	return 0;
//}
////设置图像开窗大小
////由:OV2640_ImageSize_Set确定传感器输出分辨率从大小.
////该函数则在这个范围上面进行开窗,用于OV2640_OutSize_Set的输出
////注意:本函数的宽度和高度,必须大于等于OV2640_OutSize_Set函数的宽度和高度
////     OV2640_OutSize_Set设置的宽度和高度,根据本函数设置的宽度和高度,由DSP
////     自动计算缩放比例,输出给外部设备.
////width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
////返回值:0,设置成功
////    其他,设置失败
//u8 OV2640_ImageWin_Set(u16 offx,u16 offy,u16 width,u16 height)
//{
//	u16 hsize;
//	u16 vsize;
//	u8 temp;
//	if(width%4)return 1;
//	if(height%4)return 2;
//	hsize=width/4;
//	vsize=height/4;
//	SCCB_WR_Reg(0XFF,0X00);
//	SCCB_WR_Reg(0XE0,0X04);
//	SCCB_WR_Reg(0X51,hsize&0XFF);		//设置H_SIZE的低八位
//	SCCB_WR_Reg(0X52,vsize&0XFF);		//设置V_SIZE的低八位
//	SCCB_WR_Reg(0X53,offx&0XFF);		//设置offx的低八位
//	SCCB_WR_Reg(0X54,offy&0XFF);		//设置offy的低八位
//	temp=(vsize>>1)&0X80;
//	temp|=(offy>>4)&0X70;
//	temp|=(hsize>>5)&0X08;
//	temp|=(offx>>8)&0X07;
//	SCCB_WR_Reg(0X55,temp);				//设置H_SIZE/V_SIZE/OFFX,OFFY的高位
//	SCCB_WR_Reg(0X57,(hsize>>2)&0X80);	//设置H_SIZE/V_SIZE/OFFX,OFFY的高位
//	SCCB_WR_Reg(0XE0,0X00);
//	return 0;
//}
////该函数设置图像尺寸大小,也就是所选格式的输出分辨率
////UXGA:1600*1200,SVGA:800*600,CIF:352*288
////width,height:图像宽度和图像高度
////返回值:0,设置成功
////    其他,设置失败
//u8 OV2640_ImageSize_Set(u16 width,u16 height)
//{
//	u8 temp;
//	SCCB_WR_Reg(0XFF,0X00);
//	SCCB_WR_Reg(0XE0,0X04);
//	SCCB_WR_Reg(0XC0,(width)>>3&0XFF);		//设置HSIZE的10:3位
//	SCCB_WR_Reg(0XC1,(height)>>3&0XFF);		//设置VSIZE的10:3位
//	temp=(width&0X07)<<3;
//	temp|=height&0X07;
//	temp|=(width>>4)&0X80;
//	SCCB_WR_Reg(0X8C,temp);
//	SCCB_WR_Reg(0XE0,0X00);
//	return 0;
//}

//OV5640写寄存器
//返回值:0,成功;1,失败.
u8 OV5640_WR_Reg(u16 reg, u8 data)
{
    u8 res = 0;
    SCCB_Start(); 					//启动SCCB传输
    if(SCCB_WR_Byte(OV5640_ADDR))res = 1;	//写器件ID
    if(SCCB_WR_Byte(reg >> 8))res = 1;	//写寄存器高8位地址
    if(SCCB_WR_Byte(reg))res = 1;		//写寄存器低8位地址
    if(SCCB_WR_Byte(data))res = 1; 	//写数据
    SCCB_Stop();
    return	res;
}
//OV5640读寄存器
//返回值:读到的寄存器值
u8 OV5640_RD_Reg(u16 reg)
{
    u8 val = 0;
    SCCB_Start(); 				//启动SCCB传输
    SCCB_WR_Byte(OV5640_ADDR);	//写器件ID
    SCCB_WR_Byte(reg >> 8);	  //写寄存器高8位地址
    SCCB_WR_Byte(reg);			//写寄存器低8位地址
    SCCB_Stop();
    //设置寄存器地址后，才是读
    SCCB_Start();
    SCCB_WR_Byte(OV5640_ADDR | 0X01); //发送读命令
    val = SCCB_RD_Byte();		 	//读取数据
    SCCB_No_Ack();
    SCCB_Stop();
    return val;
}
//初始化OV5640

//返回值:0,成功
//    其他,错误代码
u8 OV5640_Init(void)
{   p_dbg_enter;
    u16 i = 0;
    u16 reg;
    //设置IO
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE); //使能GPIOD,E时钟
    //OV_RESET OV_PWDN 设置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;//25MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;//GPIOD6推挽输出
    GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//GPIOE5推挽输出
    GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化

    OV5640_RST = 0;			//必须先拉低OV5640的RST脚,再上电
    delay_ms(20);
    OV5640_PWDN = 0;		//POWER ON
    delay_ms(5);
    OV5640_RST = 1;			//结束复位
    delay_ms(20);
    SCCB_Init();			//初始化SCCB 的IO口
    delay_ms(5);
    reg = OV5640_RD_Reg(OV5640_CHIPIDH);	//读取ID 高八位
    reg <<= 8;
    reg |= OV5640_RD_Reg(OV5640_CHIPIDL);	//读取ID 低八位
    if(reg != OV5640_ID)
    {
        printf("ID:%d\r\n", reg);
        return 1;
    }
    OV5640_WR_Reg(0x3103, 0X11);	//system clock from pad, bit[1]
    OV5640_WR_Reg(0X3008, 0X82);	//软复位
    delay_ms(10);
    //初始化 OV5640
    for(i = 0; i < sizeof(ov5640_init_reg_tbl) / 4; i++)
    {
        OV5640_WR_Reg(ov5640_init_reg_tbl[i][0], ov5640_init_reg_tbl[i][1]);
    }
    //检查闪光灯是否正常
//	OV5640_Flash_Ctrl(1);//打开闪光灯
//	delay_ms(50);
//	OV5640_Flash_Ctrl(0);//关闭闪光灯
    p_dbg_exit;
    return 0x00; 	//ok
}
//OV5640切换为JPEG模式
void OV5640_JPEG_Mode(void)
{
    u16 i = 0;
    //设置:输出JPEG数据
    for(i = 0; i < (sizeof(OV5640_jpeg_reg_tbl) / 4); i++)
    {
        OV5640_WR_Reg(OV5640_jpeg_reg_tbl[i][0], OV5640_jpeg_reg_tbl[i][1]);
    }
}
//OV5640切换为RGB565模式
void OV5640_RGB565_Mode(void)
{   p_dbg_enter;
    u16 i = 0;
    //设置:RGB565输出
    for(i = 0; i < (sizeof(ov5640_rgb565_reg_tbl) / 4); i++)
    {
        OV5640_WR_Reg(ov5640_rgb565_reg_tbl[i][0], ov5640_rgb565_reg_tbl[i][1]);
    }
    p_dbg_exit ;
}
//EV曝光补偿设置参数表，支持7个等级
const static u8 OV5640_EXPOSURE_TBL[7][6] =
{
    0x10, 0x08, 0x10, 0x08, 0x20, 0x10, //-3
    0x20, 0x18, 0x41, 0x20, 0x18, 0x10, //-
    0x30, 0x28, 0x61, 0x30, 0x28, 0x10, //-1
    0x38, 0x30, 0x61, 0x38, 0x30, 0x10, //0
    0x40, 0x38, 0x71, 0x40, 0x38, 0x10, //+1
    0x50, 0x48, 0x90, 0x50, 0x48, 0x20, //+2
    0x60, 0x58, 0xa0, 0x60, 0x58, 0x20, //+3
};

//EV曝光补偿
//exposure:0~6,代表补偿-3~3.
void OV5640_Exposure(u8 exposure)
{
    OV5640_WR_Reg(0x3212, 0x03);	//start group 3
    OV5640_WR_Reg(0x3a0f, OV5640_EXPOSURE_TBL[exposure][0]);
    OV5640_WR_Reg(0x3a10, OV5640_EXPOSURE_TBL[exposure][1]);
    OV5640_WR_Reg(0x3a1b, OV5640_EXPOSURE_TBL[exposure][2]);
    OV5640_WR_Reg(0x3a1e, OV5640_EXPOSURE_TBL[exposure][3]);
    OV5640_WR_Reg(0x3a11, OV5640_EXPOSURE_TBL[exposure][4]);
    OV5640_WR_Reg(0x3a1f, OV5640_EXPOSURE_TBL[exposure][5]);
    OV5640_WR_Reg(0x3212, 0x13); //end group 3
    OV5640_WR_Reg(0x3212, 0xa3); //launch group 3
}

//灯光模式参数表,支持5个模式
const static u8 OV5640_LIGHTMODE_TBL[5][7] =
{
    0x04, 0X00, 0X04, 0X00, 0X04, 0X00, 0X00, //Auto,自动
    0x06, 0X1C, 0X04, 0X00, 0X04, 0XF3, 0X01, //Sunny,日光
    0x05, 0X48, 0X04, 0X00, 0X07, 0XCF, 0X01, //Office,办公室
    0x06, 0X48, 0X04, 0X00, 0X04, 0XD3, 0X01, //Cloudy,阴天
    0x04, 0X10, 0X04, 0X00, 0X08, 0X40, 0X01, //Home,室内
};
//白平衡设置
//0:自动
//1:日光sunny
//2,办公室office
//3,阴天cloudy
//4,家里home
void OV5640_Light_Mode(u8 mode)
{   p_dbg_enter;
    u8 i;
    OV5640_WR_Reg(0x3212, 0x03);	//start group 3
    for(i = 0; i < 7; i++)OV5640_WR_Reg(0x3400 + i, OV5640_LIGHTMODE_TBL[mode][i]); //设置饱和度
    OV5640_WR_Reg(0x3212, 0x13); //end group 3
    OV5640_WR_Reg(0x3212, 0xa3); //launch group 3
    p_dbg_exit ;
}
//色彩饱和度设置参数表,支持7个等级
const static u8 OV5640_SATURATION_TBL[7][6] =
{
    0X0C, 0x30, 0X3D, 0X3E, 0X3D, 0X01, //-3
    0X10, 0x3D, 0X4D, 0X4E, 0X4D, 0X01, //-2
    0X15, 0x52, 0X66, 0X68, 0X66, 0X02, //-1
    0X1A, 0x66, 0X80, 0X82, 0X80, 0X02, //+0
    0X1F, 0x7A, 0X9A, 0X9C, 0X9A, 0X02, //+1
    0X24, 0x8F, 0XB3, 0XB6, 0XB3, 0X03, //+2
    0X2B, 0xAB, 0XD6, 0XDA, 0XD6, 0X04, //+3
};
//色度设置
//sat:0~6,代表饱和度-3~3.
void OV5640_Color_Saturation(u8 sat)
{   p_dbg_enter;
    u8 i;
    OV5640_WR_Reg(0x3212, 0x03);	//start group 3
    OV5640_WR_Reg(0x5381, 0x1c);
    OV5640_WR_Reg(0x5382, 0x5a);
    OV5640_WR_Reg(0x5383, 0x06);
    for(i = 0; i < 6; i++)OV5640_WR_Reg(0x5384 + i, OV5640_SATURATION_TBL[sat][i]); //设置饱和度
    OV5640_WR_Reg(0x538b, 0x98);
    OV5640_WR_Reg(0x538a, 0x01);
    OV5640_WR_Reg(0x3212, 0x13); //end group 3
    OV5640_WR_Reg(0x3212, 0xa3); //launch group 3
    p_dbg_exit;
}
//亮度设置
//bright:0~8,代表亮度-4~4.
void OV5640_Brightness(u8 bright)
{   p_dbg_enter;
    u8 brtval;
    if(bright < 4)brtval = 4 - bright;
    else brtval = bright - 4;
    OV5640_WR_Reg(0x3212, 0x03);	//start group 3
    OV5640_WR_Reg(0x5587, brtval << 4);
    if(bright < 4)OV5640_WR_Reg(0x5588, 0x09);
    else OV5640_WR_Reg(0x5588, 0x01);
    OV5640_WR_Reg(0x3212, 0x13); //end group 3
    OV5640_WR_Reg(0x3212, 0xa3); //launch group 3
    p_dbg_exit;
}
//对比度设置
//contrast:0~6,代表亮度-3~3.
void OV5640_Contrast(u8 contrast)
{   p_dbg_enter;
    u8 reg0val = 0X00; //contrast=3,默认对比度
    u8 reg1val = 0X20;
    switch(contrast)
    {
    case 0://-3
        reg1val = reg0val = 0X14;
        break;
    case 1://-2
        reg1val = reg0val = 0X18;
        break;
    case 2://-1
        reg1val = reg0val = 0X1C;
        break;
    case 4://1
        reg0val = 0X10;
        reg1val = 0X24;
        break;
    case 5://2
        reg0val = 0X18;
        reg1val = 0X28;
        break;
    case 6://3
        reg0val = 0X1C;
        reg1val = 0X2C;
        break;
    }
    OV5640_WR_Reg(0x3212, 0x03); //start group 3
    OV5640_WR_Reg(0x5585, reg0val);
    OV5640_WR_Reg(0x5586, reg1val);
    OV5640_WR_Reg(0x3212, 0x13); //end group 3
    OV5640_WR_Reg(0x3212, 0xa3); //launch group 3
    p_dbg_exit;
}
//锐度设置
//sharp:0~33,0,关闭;33,auto;其他值,锐度范围.
void OV5640_Sharpness(u8 sharp)
{   p_dbg_enter;
    if(sharp < 33) //设置锐度值
    {
        OV5640_WR_Reg(0x5308, 0x65);
        OV5640_WR_Reg(0x5302, sharp);
    } else	//自动锐度
    {
        OV5640_WR_Reg(0x5308, 0x25);
        OV5640_WR_Reg(0x5300, 0x08);
        OV5640_WR_Reg(0x5301, 0x30);
        OV5640_WR_Reg(0x5302, 0x10);
        OV5640_WR_Reg(0x5303, 0x00);
        OV5640_WR_Reg(0x5309, 0x08);
        OV5640_WR_Reg(0x530a, 0x30);
        OV5640_WR_Reg(0x530b, 0x04);
        OV5640_WR_Reg(0x530c, 0x06);
    }
    p_dbg_exit;

}
//特效设置参数表,支持7个特效
const static u8 OV5640_EFFECTS_TBL[7][3] =
{
    0X06, 0x40, 0X10, //正常
    0X1E, 0xA0, 0X40, //冷色
    0X1E, 0x80, 0XC0, //暖色
    0X1E, 0x80, 0X80, //黑白
    0X1E, 0x40, 0XA0, //泛黄
    0X40, 0x40, 0X10, //反色
    0X1E, 0x60, 0X60, //偏绿
};
//特效设置
//0:正常
//1,冷色
//2,暖色
//3,黑白
//4,偏黄
//5,反色
//6,偏绿
void OV5640_Special_Effects(u8 eft)
{
    OV5640_WR_Reg(0x3212, 0x03); //start group 3
    OV5640_WR_Reg(0x5580, OV5640_EFFECTS_TBL[eft][0]);
    OV5640_WR_Reg(0x5583, OV5640_EFFECTS_TBL[eft][1]); // sat U
    OV5640_WR_Reg(0x5584, OV5640_EFFECTS_TBL[eft][2]); // sat V
    OV5640_WR_Reg(0x5003, 0x08);
    OV5640_WR_Reg(0x3212, 0x13); //end group 3
    OV5640_WR_Reg(0x3212, 0xa3); //launch group 3
}
//测试序列
//mode:0,关闭
//     1,彩条
//     2,色块
void OV5640_Test_Pattern(u8 mode)
{
    if(mode == 0)OV5640_WR_Reg(0X503D, 0X00);
    else if(mode == 1)OV5640_WR_Reg(0X503D, 0X80);
    else if(mode == 2)OV5640_WR_Reg(0X503D, 0X82);
}
//闪光灯控制
//mode:0,关闭
//     1,打开
void OV5640_Flash_Ctrl(u8 sw)
{
    OV5640_WR_Reg(0x3016, 0X02);
    OV5640_WR_Reg(0x301C, 0X02);
    if(sw)OV5640_WR_Reg(0X3019, 0X02);
    else OV5640_WR_Reg(0X3019, 0X00);
}
//设置图像输出大小
//OV5640输出图像的大小(分辨率),完全由该函数确定
//offx,offy,为输出图像在OV5640_ImageWin_Set设定窗口(假设长宽为xsize和ysize)上的偏移
//由于开启了scale功能,用于输出的图像窗口为:xsize-2*offx,ysize-2*offy
//width,height:实际输出图像的宽度和高度
//实际输出(width,height),是在xsize-2*offx,ysize-2*offy的基础上进行缩放处理.
//一般设置offx和offy的值为16和4,更小也是可以,不过默认是16和4
//返回值:0,设置成功
//    其他,设置失败
u8 OV5640_OutSize_Set(u16 offx, u16 offy, u16 width, u16 height)
{
    OV5640_WR_Reg(0X3212, 0X03);  	//开始组3
    //以下设置决定实际输出尺寸(带缩放)
    OV5640_WR_Reg(0x3808, width >> 8);	//设置实际输出宽度高字节
    OV5640_WR_Reg(0x3809, width & 0xff); //设置实际输出宽度低字节
    OV5640_WR_Reg(0x380a, height >> 8); //设置实际输出高度高字节
    OV5640_WR_Reg(0x380b, height & 0xff); //设置实际输出高度低字节
    //以下设置决定输出尺寸在ISP上面的取图范围
    //范围:xsize-2*offx,ysize-2*offy
    OV5640_WR_Reg(0x3810, offx >> 8);	//设置X offset高字节
    OV5640_WR_Reg(0x3811, offx & 0xff); //设置X offset低字节

    OV5640_WR_Reg(0x3812, offy >> 8);	//设置Y offset高字节
    OV5640_WR_Reg(0x3813, offy & 0xff); //设置Y offset低字节

    OV5640_WR_Reg(0X3212, 0X13);		//结束组3
    OV5640_WR_Reg(0X3212, 0Xa3);		//启用组3设置
    return 0;
}

//设置图像开窗大小(ISP大小),非必要,一般无需调用此函数
//在整个传感器上面开窗(最大2592*1944),用于OV5640_OutSize_Set的输出
//注意:本函数的宽度和高度,必须大于等于OV5640_OutSize_Set函数的宽度和高度
//     OV5640_OutSize_Set设置的宽度和高度,根据本函数设置的宽度和高度,由DSP
//     自动计算缩放比例,输出给外部设备.
//width,height:宽度(对应:horizontal)和高度(对应:vertical)
//返回值:0,设置成功
//    其他,设置失败
u8 OV5640_ImageWin_Set(u16 offx, u16 offy, u16 width, u16 height)
{
    u16 xst, yst, xend, yend;
    xst = offx;
    yst = offy;
    xend = offx + width - 1;
    yend = offy + height - 1;
    OV5640_WR_Reg(0X3212, 0X03);		//开始组3
    OV5640_WR_Reg(0X3800, xst >> 8);
    OV5640_WR_Reg(0X3801, xst & 0XFF);
    OV5640_WR_Reg(0X3802, yst >> 8);
    OV5640_WR_Reg(0X3803, yst & 0XFF);
    OV5640_WR_Reg(0X3804, xend >> 8);
    OV5640_WR_Reg(0X3805, xend & 0XFF);
    OV5640_WR_Reg(0X3806, yend >> 8);
    OV5640_WR_Reg(0X3807, yend & 0XFF);
    OV5640_WR_Reg(0X3212, 0X13);		//结束组3
    OV5640_WR_Reg(0X3212, 0Xa3);		//启用组3设置
    return 0;
}
//初始化自动对焦
//返回值:0,成功;1,失败.
u8 OV5640_Focus_Init(void)
{   p_dbg_enter;
    u16 i;
    u16 addr = 0x8000;
    u8 state = 0x8F;
    OV5640_WR_Reg(0x3000, 0x20);			//reset MCU
    for(i = 0; i < sizeof(OV5640_AF_Config); i++) //发送配置数组
    {
        OV5640_WR_Reg(addr, OV5640_AF_Config[i]);
        addr++;
    }
    OV5640_WR_Reg(0x3022, 0x00);
    OV5640_WR_Reg(0x3023, 0x00);
    OV5640_WR_Reg(0x3024, 0x00);
    OV5640_WR_Reg(0x3025, 0x00);
    OV5640_WR_Reg(0x3026, 0x00);
    OV5640_WR_Reg(0x3027, 0x00);
    OV5640_WR_Reg(0x3028, 0x00);
    OV5640_WR_Reg(0x3029, 0x7f);
    OV5640_WR_Reg(0x3000, 0x00);
    i = 0;
    do
    {
        state = OV5640_RD_Reg(0x3029);
        delay_ms(5);
        i++;
        if(i > 1000)return 1;
    } while(state != 0x70);
    p_dbg_exit ;
    return 0;
}
//执行一次自动对焦
//返回值:0,成功;1,失败.
u8 OV5640_Focus_Single(void)
{   p_dbg_enter;
    u8 temp;
    u16 retry = 0;
    OV5640_WR_Reg(0x3022, 0x03);		//触发一次自动对焦
    while(1)
    {
        retry++;
        temp = OV5640_RD_Reg(0x3029);	//检查对焦完成状态
        if(temp == 0x10)break;		// focus completed
        delay_ms(5);
        if(retry > 1000)return 1;
    }
    p_dbg_exit;
    return 0;
}
//持续自动对焦,当失焦后,会自动继续对焦
//返回值:0,成功;其他,失败.
u8 OV5640_Focus_Constant(void)
{
    u8 temp = 0;
    u16 retry = 0;
    OV5640_WR_Reg(0x3023, 0x01);
    OV5640_WR_Reg(0x3022, 0x08); //发送IDLE指令
    do
    {
        temp = OV5640_RD_Reg(0x3023);
        retry++;
        if(retry > 1000)return 2;
        delay_ms(5);
    } while(temp != 0x00);
    OV5640_WR_Reg(0x3023, 0x01);
    OV5640_WR_Reg(0x3022, 0x04); //发送持续对焦指令
    retry = 0;
    do
    {
        temp = OV5640_RD_Reg(0x3023);
        retry++;
        if(retry > 1000)return 2;
        delay_ms(5);
    } while(temp != 0x00); //0,对焦完成;1:正在对焦
    return 0;
}

