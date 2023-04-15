#ifndef __FONTUPD_H__
#define __FONTUPD_H__	 
#include <stm32f4xx.h>

//������Ϣ�����ַ,ռ33���ֽ�,��1���ֽ����ڱ���ֿ��Ƿ����.����ÿ8���ֽ�һ��,�ֱ𱣴���ʼ��ַ���ļ���С														   
extern u32 FONTINFOADDR;	
//�ֿ���Ϣ�ṹ�嶨��
//���������ֿ������Ϣ����ַ����С��
__packed typedef struct 
{
	u8 fontok;				//�ֿ���ڱ�־��0XAA���ֿ��������������ֿⲻ����
	u32 ugbkaddr; 			//unigbk�ĵ�ַ
	u32 ugbksize;			//unigbk�Ĵ�С	 
	u32 f48addr;			//gbk48��ַ	
	u32 gbk48size;			//gbk48�Ĵ�С	 
	u32 f32addr;			//gbk40��ַ
	u32 gbk32size;			//gbk40�Ĵ�С		 
	u32 f24addr;			//gbk32��ַ
	u32 gbk24size;			//gbk32�Ĵ�С 
}_font_info; 


extern _font_info ftinfo;	//�ֿ���Ϣ�ṹ��


u8 font_init(void);										//��ʼ���ֿ�
#endif


















