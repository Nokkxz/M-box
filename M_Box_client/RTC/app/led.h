#ifndef __LED_H
#define __LED_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
#define LEDA0 PAout(0)
#define LEDA1 PAout(1)
#define LEDA2 PAout(2)
#define LEDA3 PAout(3)
#define LEDA4 PAout(4)
#define LEDA5 PAout(5)
#define LEDC13 PCout(13)
#define BEEPB12 PBout(12)

void LED_Init(void);//��ʼ��

		 				    
#endif
