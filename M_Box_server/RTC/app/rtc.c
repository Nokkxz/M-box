#include "led.h"
#include "includes.h"
#include "rtc.h" 		    

//第一次配置的时候更改这个值，并且断开电池和电源一段时间，保证电容电量也放完，或者短接vbat和GND
/*
    其实，在32.768K的晶振引脚上并个上1M(>1m也可以)的电阻就没事了，哪有那么多问题啊。
    主要是STM32的LSE反馈电阻有点大，并个电阻，就能减小反馈电阻，提高启振系数。相应的，LSE的功耗就会大一点。
    不过，没关系，STM32在使用外部电源时，内部软开关会把LSE的电源切换到外部电源的。
    1个40mAH的小电池，用3年应该没问题。
    3年，你这个产品还会不会还在使用都是个问题了。说不定早升级了。

*/
#define data_change 0x4456
#define RTCAlarm_Way 0  //0中断线方案，1 原子方案


_calendar_obj calendar;//时钟结构体 
 
static void RTC_NVIC_Config(void)
{	
		 NVIC_InitTypeDef NVIC_InitStructure;
		 NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;  //RTC全局中断
		 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; 
		 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
		 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  //使能该通道中断
		 NVIC_Init(&NVIC_InitStructure);
#if  (RTCAlarm_Way==0)
		 NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;  //闹钟中断
		 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //比RTC全局中断的优先级高
		 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 
		 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
		 NVIC_Init(&NVIC_InitStructure);
#endif
}
#if    (RTCAlarm_Way==0)
static void RTC_Alarm_EXIT(void)
{ 
		EXTI_InitTypeDef EXTI_InitStructure;
		EXTI_ClearITPendingBit(EXTI_Line17);
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Line = EXTI_Line17;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
}
#endif
//实时时钟配置
//初始化RTC时钟,同时检测时钟是否工作正常
//BKP->DR1用于保存是否第一次配置的设置
//返回0:正常
//其他:错误代码

u8 RTC_Init(void)
{
		//检查是不是第一次配置时钟
		u8 temp=0;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟
    RCC_RTCCLKCmd(ENABLE); 	
		PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问  
		if (BKP_ReadBackupRegister(BKP_DR1) != data_change)		//从指定的后备寄存器中读出数据:读出了与写入的指定数据不相乎
		{	 			
				BKP_DeInit();	//复位备份区域 	
				RCC_LSEConfig(RCC_LSE_ON);	//设置外部低速晶振(LSE),使用外设低速晶振
				while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET&&temp<250)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
				{
						temp++;
						delay_ms(10);
				}
				if(temp>=250)
			  {
          while(1)
          {
            printf("jingzhen-error\r\n");
            delay_ms(1000);
          }
					  return 1;//初始化时钟失败,晶振有问题
				}
				RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
				RCC_RTCCLKCmd(ENABLE);	                  //使能RTC时钟 
        RTC_WaitForSynchro();										  //等待RTC寄存器(RTC_CNT,RTC_ALR和RTC_PRL)与RTC APB时钟同步				
				RTC_WaitForLastTask();	                  //等待最近一次对RTC寄存器的写操作完成
				
				RTC_ITConfig(RTC_IT_SEC, ENABLE);		//使能RTC秒中断
				RTC_ITConfig(RTC_IT_ALR, ENABLE);   //使能RTC闹钟秒中断
				RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
				
				RTC_EnterConfigMode();  // 允许配置	
				RTC_SetPrescaler(32767); //设置RTC预分频的值
				RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
        RTC_ExitConfigMode(); //退出配置模式	
				
        BKP_WriteBackupRegister(BKP_DR1, data_change);	//向指定的后备寄存器中写入用户程序数据
				printf("time_set_ok\r\n"); 
				
				RTC_Set(2017,5,22,22,31,59);        //设置时间
				RTC_Alarm_Set(2017,5,22,22,31,0);  //设置闹钟 								
		}
		else//系统继续计时
		{
				RTC_ITConfig(RTC_IT_SEC, ENABLE);	  //使能RTC秒中断
			  RTC_ITConfig(RTC_IT_ALR, ENABLE);   //使能RTC闹钟秒中断
				RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
			  printf("time_ok\r\n"); 
		}
		RTC_NVIC_Config();//RCT中断分组设置
#if (RTCAlarm_Way==0)		
    RTC_Alarm_EXIT();	
#endif		
		RTC_Get();//更新时间	
		return 0; //ok
}	


#if (RTCAlarm_Way==0)
//RTC时钟中断
//每秒触发一次  
void RTC_IRQHandler(void)
{
		if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
		{
				RTC_Get();//更新时间
		}
		RTC_ClearITPendingBit(RTC_IT_SEC); 
		RTC_WaitForLastTask();
}
//闹钟中断
//必须先开启中断线中断  RTC_Alarm_EXIT();
//或者采用原子的查询中断方案
void RTCAlarm_IRQHandler(void)
{     
		if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
		{
        printf("**********中断线方案闹钟**********\r\n");//输出闹铃时间
		}
		EXTI_ClearITPendingBit(EXTI_Line17);
		RTC_WaitForLastTask();
		RTC_ClearITPendingBit(RTC_IT_ALR);
		RTC_WaitForLastTask();
}
#else
//RTC时钟中断
//每秒触发一次  
void RTC_IRQHandler(void)
{		 
		if (RTC_GetITStatus(RTC_IT_SEC) != RESET)//秒钟中断
		{							
				RTC_Get();//更新时间   
		}
		if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)//闹钟中断
		{
				RTC_ClearITPendingBit(RTC_IT_ALR);		//清闹钟中断	  	  
				printf("**********查询方案闹钟**********\r\n");//输出闹铃时间
		} 				  								 
		RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);		//清闹钟中断
		RTC_WaitForLastTask();	  	    						 	   	 
}
#endif




//得到当前的时间
//返回值:0,成功;其他:错误代码.
u8 RTC_Get(void)
{
			u32 timecount=0; 
			timecount=RTC_GetCounter();	 
			sec_2_time(&calendar,timecount) ;   
			return 0;
}	 
u8 Time_Get(_calendar_obj* time)
{
 			u32 timecount=0; 
			timecount=RTC_GetCounter();	
			sec_2_time(time,timecount) ;   
			return 0;  
}
//以1970年1月1日为基准,1970~2099年为合法年份
//syear,smon,sday,hour,min,sec：闹钟的年月日时分秒
u8 RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
			u32 seccount=0;
			seccount=time_2_sec(syear,smon,sday,hour,min,sec);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟  
			PWR_BackupAccessCmd(ENABLE);	//使能RTC和后备寄存器访问 
			RTC_SetCounter(seccount);	//设置RTC计数器的值
			RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成  	
			return 0;	    
}

//初始化闹钟		  
//以1970年1月1日为基准,1970~2099年为合法年份
//syear,smon,sday,hour,min,sec：闹钟的年月日时分秒
u8 RTC_Alarm_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
		u32 seccount=0;
		seccount=time_2_sec(syear,smon,sday,hour,min,sec);		    
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟 	
		PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问  
		RTC_SetAlarm(seccount);
		RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成  	
		return 0;	    
			
}
void RTC_Alarm_Set_after(u8 sday,u8 hour,u8 min,u8 sec)
{
		u32 seccount=0;
	  u32 time_go=time_2_sec_go(sday,hour,min,sec);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟   
		PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问  	
		seccount=RTC_GetCounter();
		RTC_SetAlarm(seccount+time_go);
		RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成  	
}


//用于闹钟
u32 time_2_sec_go(u8 sday,u8 hour,u8 min,u8 sec)
{
    return (sday*24*3600+hour*3600+min*60+sec);
}
u32 time_2_sec(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
	u16 t;
	u32 seccount=0;
	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)	//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t))seccount+=31622400;//闰年的秒钟数
		else seccount+=31536000;			  //平年的秒钟数
	}
	smon-=1;
	for(t=0;t<smon;t++)	   //把前面月份的秒钟数相加
	{
		seccount+=(u32)mon_table[t]*86400;//月份秒钟数相加
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;//闰年2月份增加一天的秒钟数	   
	}
	seccount+=(u32)(sday-1)*86400;//把前面日期的秒钟数相加 
	seccount+=(u32)hour*3600;//小时秒钟数
  seccount+=(u32)min*60;	 //分钟秒钟数
	seccount+=sec;//最后的秒钟加上去
  return seccount;
}
u8 sec_2_time(_calendar_obj* time,u32 timecount)
{
	u16 daycnt=0;
	u32 temp=0;
	u16 temp1=0;	  
 	temp=timecount/86400;   //得到天数(秒钟数对应的)
	if(daycnt!=temp)//超过一天了
	{	  
		daycnt=temp;
		temp1=1970;	//从1970年开始
		while(temp>=365)
		{				 
			if(Is_Leap_Year(temp1))//是闰年
			{
				if(temp>=366)temp-=366;//闰年的秒钟数
				else {temp1++;break;}  
			}
			else temp-=365;	  //平年 
			temp1++;  
		}   
		(* time).w_year=temp1;//得到年份
		temp1=0;
		while(temp>=28)//超过了一个月
		{
			if(Is_Leap_Year((* time).w_year)&&temp1==1)//当年是不是闰年/2月份
			{
				if(temp>=29)temp-=29;//闰年的秒钟数
				else break; 
			}
			else 
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];//平年
				else break;
			}
			temp1++;  
		}
		(* time).w_month=temp1+1;	//得到月份
		(* time).w_date=temp+1;  	//得到日期 
	}
	temp=timecount%86400;     		//得到秒钟数   	   
	(* time).hour=temp/3600;     	//小时
	(* time).min=(temp%3600)/60; 	//分钟	
	(* time).sec=(temp%3600)%60; 	//秒钟
	(* time).week=RTC_Get_Week((* time).w_year,(* time).w_month,(* time).w_date);//获取星期   
	return 0;
}

s32 compare_time(_calendar_obj time1,_calendar_obj time2)
{
   u32 seccount1=0,seccount2=0;
	 seccount1=time_2_sec(time1.w_year,time1.w_month,time1.w_date,time1.hour,time1.min,time1.sec);
	 seccount2=time_2_sec(time2.w_year,time2.w_month,time2.w_date,time2.hour,time2.min,time2.sec);
	 return (seccount2-seccount1);
}



//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
u8 Is_Leap_Year(u16 year)
{			  
	if(year%4==0) //必须能被4整除
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//如果以00结尾,还要能被400整除 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	 			   
//设置时钟
//把输入的时钟转换为秒钟
//以1970年1月1日为基准
//1970~2099年为合法年份
//返回值:0,成功;其他:错误代码.
//月份数据表											 
u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
//平年的月份日期表
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日 
//返回值：星期号																						 
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{	
	u16 temp2;
	u8 yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// 如果为21世纪,年份数加100  
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}			  


