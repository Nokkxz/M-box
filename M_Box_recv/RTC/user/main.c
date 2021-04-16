#include "delay.h"
#include "sys.h"
#include "usart1.h"
#include "package.h"
#include "DL_LN3X.h"
#include "usart3.h"			 	 
#include "string.h"	  
#include "stdio.h"
#include "lee.h"
#include "led.h"
#include "key.h"
#include "includes.h"

/*
接收端
*/

extern void uartRevieveByte(u8 data);

int almList[100][4]={{0,0x1F,0x01,1},{5,0x1F,0x01,1},{10,0x1F,0x01,2},{15,0x1F,0x01,2},{20,0x1F,0x01,3},
{25,0x1F,0x01,3},{30,0x1F,0x01,4},{35,0x1F,0x01,4},{40,0x1F,0x01,5},{45,0x1F,0x01,5},{50,0x1F,0x01,6},{55,0x1F,0x01,6}};


//----------------------------InitAll
void initAll()
{
  	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
		delay_init();	    	 //延时函数初始化
		USART1_Init(115200,0);	 //串口初始化为115200
  	usart3_init(115200);//串口3初始化为115200
//		RTC_Init();	  			//RTC初始化 
    LED_Init();         //LED初始化
    KEY_Init();         //KEY初始化
	  delay_ms(1000);
}


//----------------------------读取STM32ID
#define STM32_UNIQUE_ID_SIZE 12
u8     stm32_uniqueId[STM32_UNIQUE_ID_SIZE] = {0}; 
u8* readStm32UniqueID(void) 
{        
    vu8* addr = (vu8*)(0x1ffff7e8);   //UID首地址
    for(u8 i = 0; i < STM32_UNIQUE_ID_SIZE; ++i)
    {
        u8 id= *addr;
        stm32_uniqueId[i] = id;
        ++addr;
    }
    return stm32_uniqueId;
}
void testID(void)
{
    readStm32UniqueID();
  while(1)
  {
    delay_ms(500);
    for(int i=0;i<12;i++)
    {
      printf("%d",stm32_uniqueId[i]);
    }
    printf("\n");
  }
}


//-----------------------------测试通讯
void testCM(int t)//0发1收
{
  newPkg(1) THPkg_Out={5,0x90,0x90,0x1F,0x01,{0x01}};//发送包
  newPkg(1) THPkg_Test={5,0x90,0x20,0x00,0x00,{0x01}};//本地点亮
  sPkg* pkg;
  while(1)
  {
    if(t==0)
    {
      sendPkg((sPkg*)(&THPkg_Out));
    }else if(t==1)
    {
      pkg = getNextPkg();
      if(pkg!=NULL) sendPkg((sPkg*)(&THPkg_Test));
    }
  }
}


//-----------------------------测试LED
void testLED(void)
{
  while(1)
  {
    LEDA0=1;LEDA1=1;LEDA2=1;LEDA3=1;LEDA4=1;LEDA5=1;LEDC13=1;
    delay_ms(500);
    LEDA0=0;LEDA1=0;LEDA2=0;LEDA3=0;LEDA4=0;LEDA5=0;LEDC13=0;
    delay_ms(500);
  }
}


//-----------------------------测试KEY
void testKEY(void)
{  
  while(1)
  {
    printf("%d",KEYB13);
    if(KEYB13==0)
    {
      BEEPB12=1;
    }else
    {
      BEEPB12=0;
    }
  }
}


//------------------------------Stop
void stop(void)
{
    BEEPB12=0;
  LEDA0=0; LEDA1=0; LEDA2=0; LEDA3=0; LEDA4=0; LEDA5=0;
}


//------------------------------Remind
void remind(int t)
{
  switch(t){
    case 1: LEDA0=1; break;
    case 2: LEDA0=1; LEDA1=1; break;
    case 3: LEDA0=1; LEDA1=1; LEDA2=1; break;
    case 4: LEDA0=1; LEDA1=1; LEDA2=1; LEDA3=1; break;
    case 5: LEDA0=1; LEDA1=1; LEDA2=1; LEDA3=1; LEDA4=1; break;
    case 6: LEDA0=1; LEDA1=1; LEDA2=1; LEDA3=1; LEDA4=1; LEDA5=1; break;
    default: printf("Wrong number"); break;
  }
  BEEPB12=1;
  printf("\nHello Alarm!\n");
  for(int i=0;i<2;i++)delay_ms(1000);
  stop();
}


//------------------------------SendRemind
void sendRemind(int t[])
{
  newPkg(1) action={5,0x90,0x20,t[1],t[2],{t[3]}};
  sendPkg((sPkg*)(&action));
}


//------------------------------GetRemind
void getRemind(void)
{
  sPkg* pkg;
  pkg = getNextPkg();
  if(pkg!=NULL)
  {
    printf("GetRemind\n");
    remind(pkg->data[0]);
  }
}


//------------------------------判断闹钟
void setAlm(int t[])
{
  if(t[0]!=0 || t[1]!= 0)
  {
    if(/*t[0]==calendar.hour && t[1]==calendar.min*/t[1]==calendar.sec) sendRemind(t);
    else stop();
  }
}


//------------------------------检测开关
int keyState=0;
void checkKey(void)
{
  if(keyState==0 && KEYB13==1)
  {
    printf("open");
    keyState=1;
    newPkg(1) key={5,0x90,0x90,0x1F,0x00,{9}};
    sendPkg((sPkg*)(&key));
  }else if(keyState==1 && KEYB13==0)
  {
    printf("close");
    keyState=0;
  }
}


//------------------------------开始循环
void loopAll()
{
  while(1)
  {
    getRemind();
    checkKey();
	}
}


int main()
{
//  while(1);
  initAll();
  loopAll();
}
