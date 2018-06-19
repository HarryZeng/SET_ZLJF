
///**
//  ********************************  STM32F0x1  *********************************
//  * @文件名     ： SelftStudy.c
//  * @作者       ： HarryZeng
//  * @库版本     ： V1.5.0
//  * @文件版本   ： V1.0.0
//  * @日期       ： 2017年11月24日
//  * @摘要       ： 数据处理
//  ******************************************************************************/
///*----------------------------------------------------------------------------
//  更新日志:
//  2017-11-24 V1.0.0:初始版本
//  ----------------------------------------------------------------------------*/
///* 包含的头文件 --------------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "project.h"
#include "bsp_init.h"
#include "key.h"
#include "SelfStudy.h"
#include "flash.h"
#include "display.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_dac.h"

/*第一次SET按键按下处理函数*/
void SelfStudy_SET1(void);;

/*第二次SET按键按下处理函数*/
void SelfStudy_SET2(void);;

uint8_t SelftStudyflag=0;
uint8_t ATTcalibrationFlag=0;
uint8_t 	selfDisplayEndFlay=0;
uint32_t  SET1ADC_Value=0;
uint32_t CalibrateADCValue=0;
uint32_t CalibrateSAValue=0;
uint32_t CalibrateSBValue=0;

uint32_t CalibrateS1Value=0;
uint32_t CalibrateS2Value=0;


extern int16_t adc_dma_tab[DMA_BUFFER_SIZE];
extern  uint8_t DX_Flag;
extern uint8_t sample_finish;
extern int32_t 	SET_VREF;
extern uint32_t ADC_Display;//ADC显示
extern uint8_t 	FX_Flag;




/*在一系列的ADCvalue中寻找最大的ADV MAX*/
int32_t 			ADCMAX=0;
int32_t 		NewThreshold=0;
//int32_t 		S_MaxValue=0;
int32_t    	S1_MaxValue=0;
int32_t    	S2_MaxValue=0;
extern uint8_t displayModeONE_FLAG ;
extern uint8_t DisplayModeNo;
extern int16_t HI ;
extern int16_t LO ;

/*ADCIN的数据调零*/

extern uint8_t S_Final_FinishFlag;
extern uint32_t 	S[4];
extern uint32_t 	S_Final;

void SelfStudy_End(void);

void selfstudy(void)
{
	uint8_t OUT1_STATUS,OUT2_STATUS;
	

	if(SetButton.Status == Press && ModeButton.Status==Press)
	{

	}
	else
	{
			/*第一次进入SET模式*/
			while(SetButton.Status == Press && SetButton.PressTimer >= SetButton.LongTime)     //只要在显示模式下第一次按下SET按键
			{	
				SelftStudyflag = 1; //标记进入自学习状态
				sample_finish = 0;  //清除采样结束标记
					/*保持OUT1的状态*/
				OUT1_STATUS = GPIO_ReadInputDataBit(OUT1_GPIO_Port,OUT1_Pin);/*获取当前的OUT1状态*/
				GPIO_WriteBit(OUT1_GPIO_Port,OUT1_Pin,(BitAction)OUT1_STATUS);/*保持着OUT1状态*/
				OUT2_STATUS = GPIO_ReadInputDataBit(OUT2_GPIO_Port,OUT2_Pin);/*获取当前的OUT2状态*/
				GPIO_WriteBit(OUT2_GPIO_Port,OUT2_Pin,(BitAction)OUT2_STATUS);/*保持着OUT1状态*/

				SelfStudy_SET1();
				SMG_DisplaySET_Step_1_Mode(2,S1_MaxValue);  //显示SET1和信号值
				
				while(SetButton.PressCounter==1)
				{				
				SMG_DisplaySET_Step_1_Mode(2,S1_MaxValue);  //显示SET1和信号值
				} //等待Set按键释放
				
				while(SetButton.PressCounter==2)	
				{	
					SelfStudy_SET2();/*按下的三秒钟内，不断查找最大值*/
					SMG_DisplaySET_Step_2_Mode(2,0,Threshold);  //显示SET2和阈值
				}

					if(SetButton.PressCounter>=3) /*按键达到3秒后，第一次进入自学习，等待第二次按下SET 3秒*/
					{
						
						SelfStudy_End();
						
						FX_Flag = 1;  //结束自学习，重启FX
						/*一直等待第二次SET的按下*
						**************************/
						SetButton.LastCounter = SetButton.PressCounter;
						SetButton.PressCounter = 0;
						SetButton.Effect = PressNOEffect;
						UpButton.PressCounter=0;
						UpButton.Effect=PressNOEffect;
						DownButton.PressCounter=0;
						DownButton.Effect=PressNOEffect;
						ModeButton.PressCounter=0;
						ModeButton.Effect=PressNOEffect;
						
						selfDisplayEndFlay =0;
						SelftStudyflag = 0;//清除自学习标记-- 结束了自学习
					}
				
			}
	}
}





void SelfStudy_End(void)
{					
	int32_t S_SET = 0;

			S_SET = Final;

			if(displayModeONE_FLAG)//区域模式
			{
				if(DisplayModeNo==0)
				{
					HI = S_SET;   
					WriteFlash(HI_FLASH_DATA_ADDRESS,HI);
				}
				else if(DisplayModeNo==1)
				{
					LO = S_SET;  
					WriteFlash(LO_FLASH_DATA_ADDRESS,LO);
				}
			}
			else    //标准模式
			{
				NewThreshold = S_SET; 

				if(NewThreshold<=20) NewThreshold=20;
				if(NewThreshold>=4095) NewThreshold=4095;	

				Threshold = NewThreshold;
				WriteFlash(Threshold_FLASH_DATA_ADDRESS,Threshold);
			}
		
		GPIO_WriteBit(OUT1_GPIO_Port,OUT1_Pin,(BitAction)GPIO_ReadInputDataBit(OUT1_GPIO_Port,OUT1_Pin));
		GPIO_WriteBit(OUT2_GPIO_Port,OUT2_Pin,(BitAction)GPIO_ReadInputDataBit(OUT2_GPIO_Port,OUT2_Pin));
		
		
		if(S1_MaxValue <= S2_MaxValue)  /*SET_VREF为SET1和SET2中较小的值*/
			SET_VREF = S1_MaxValue;
		else 
			SET_VREF = S2_MaxValue;
		
		Threshold = NewThreshold;
		
		selfDisplayEndFlay = 1;
		SetButton.PressCounter = 0;					/*清楚按键次数*/
		SetButton.Status = Release;					/*释放按键*/
		SetButton.Effect = PressNOEffect;
		
		WriteFlash(SET_VREF_FLASH_DATA_ADDRESS,SET_VREF);
}

void SelfStudy_SET2(void) 
{
		uint32_t 		SET2_ADCValue=0;
		
		if(sample_finish)
		{
				sample_finish = 0;

				SET2_ADCValue = 	Final ;
				
				if(SET2_ADCValue>=S1_MaxValue)   //不断寻找最大值
				{
					S1_MaxValue = SET2_ADCValue;
				}
	}
}

/*获取四个ADC通道采样后，求平均的值*/
uint8_t  JudgeSvalue(uint32_t *S_Value)
{
	uint8_t flag=0x00;
		if(S_Value[0]>4000) flag |= 0x01;/*0000 0001*/
	else flag &= 0xFE;/*1111 1110*/
	
		if(S_Value[1]>4000) flag |= 0x02;/*0000 0010*/
	else flag &= 0xFD;/*1111 1101*/
	
		if(S_Value[2]>4000) flag |= 0x04;/*0000 0100*/
	else flag &= 0xFB;/*1111 1011*/
	
		if(S_Value[3]>4000) flag |= 0x08;/*0000 1000*/
	else flag &= 0xF7;/*1111 0111*/
	
	if(flag==0x0f)
		return 1;
	else 
		return 0;
}

/*ADCIN的数据调零*/

void SelfStudy_SET1(void) 
{
		uint32_t 		SET1_ADCValue=0;

//	while(CalibrateS1Value>=1400 || CalibrateS1Value<=1000 || CalibrateS2Value>=1400 || CalibrateS2Value<=1000)
//	{
		if(sample_finish)
		{
			sample_finish = 0;

//			Get_S1_Value(&CalibrateS1Value);					//定时ADC采样
//			DACOUT1 = CalibrateS1Value;
//			
//			if(DACOUT1>=4095)
//				DACOUT1 = 4095;
//			else if(DACOUT1<=0)
//				DACOUT1 = 0;
//			DAC_SetChannel1Data(DAC_Align_12b_R,(uint16_t)DACOUT1);
//			DAC_SoftwareTriggerCmd(DAC_Channel_1,ENABLE);
//			/*******************************************************/
//			Get_S2_Value(&CalibrateS2Value);					//定时ADC采样

//			DACOUT2 = CalibrateS2Value;
//			if(DACOUT2>=4095)
//				DACOUT2 = 4095;
//			else if(DACOUT2<=0)
//				DACOUT2 = 0;
//			DAC_SetChannel2Data(DAC_Align_12b_R,(uint16_t)DACOUT2);
//			DAC_SoftwareTriggerCmd(DAC_Channel_2,ENABLE);
//			

				SET1_ADCValue = 	Final ;
				
				if(SET1_ADCValue>=S1_MaxValue)   //不断寻找最大值
				{
					S1_MaxValue = SET1_ADCValue;
				}

	}
}

