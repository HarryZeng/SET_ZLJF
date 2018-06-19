
///**
//  ********************************  STM32F0x1  *********************************
//  * @�ļ���     �� SelftStudy.c
//  * @����       �� HarryZeng
//  * @��汾     �� V1.5.0
//  * @�ļ��汾   �� V1.0.0
//  * @����       �� 2017��11��24��
//  * @ժҪ       �� ���ݴ���
//  ******************************************************************************/
///*----------------------------------------------------------------------------
//  ������־:
//  2017-11-24 V1.0.0:��ʼ�汾
//  ----------------------------------------------------------------------------*/
///* ������ͷ�ļ� --------------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "project.h"
#include "bsp_init.h"
#include "key.h"
#include "SelfStudy.h"
#include "flash.h"
#include "display.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_dac.h"

/*��һ��SET�������´�����*/
void SelfStudy_SET1(void);;

/*�ڶ���SET�������´�����*/
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
extern uint32_t ADC_Display;//ADC��ʾ
extern uint8_t 	FX_Flag;




/*��һϵ�е�ADCvalue��Ѱ������ADV MAX*/
int32_t 			ADCMAX=0;
int32_t 		NewThreshold=0;
//int32_t 		S_MaxValue=0;
int32_t    	S1_MaxValue=0;
int32_t    	S2_MaxValue=0;
extern uint8_t displayModeONE_FLAG ;
extern uint8_t DisplayModeNo;
extern int16_t HI ;
extern int16_t LO ;

/*ADCIN�����ݵ���*/

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
			/*��һ�ν���SETģʽ*/
			while(SetButton.Status == Press && SetButton.PressTimer >= SetButton.LongTime)     //ֻҪ����ʾģʽ�µ�һ�ΰ���SET����
			{	
				SelftStudyflag = 1; //��ǽ�����ѧϰ״̬
				sample_finish = 0;  //��������������
					/*����OUT1��״̬*/
				OUT1_STATUS = GPIO_ReadInputDataBit(OUT1_GPIO_Port,OUT1_Pin);/*��ȡ��ǰ��OUT1״̬*/
				GPIO_WriteBit(OUT1_GPIO_Port,OUT1_Pin,(BitAction)OUT1_STATUS);/*������OUT1״̬*/
				OUT2_STATUS = GPIO_ReadInputDataBit(OUT2_GPIO_Port,OUT2_Pin);/*��ȡ��ǰ��OUT2״̬*/
				GPIO_WriteBit(OUT2_GPIO_Port,OUT2_Pin,(BitAction)OUT2_STATUS);/*������OUT1״̬*/

				SelfStudy_SET1();
				SMG_DisplaySET_Step_1_Mode(2,S1_MaxValue);  //��ʾSET1���ź�ֵ
				
				while(SetButton.PressCounter==1)
				{				
				SMG_DisplaySET_Step_1_Mode(2,S1_MaxValue);  //��ʾSET1���ź�ֵ
				} //�ȴ�Set�����ͷ�
				
				while(SetButton.PressCounter==2)	
				{	
					SelfStudy_SET2();/*���µ��������ڣ����ϲ������ֵ*/
					SMG_DisplaySET_Step_2_Mode(2,0,Threshold);  //��ʾSET2����ֵ
				}

					if(SetButton.PressCounter>=3) /*�����ﵽ3��󣬵�һ�ν�����ѧϰ���ȴ��ڶ��ΰ���SET 3��*/
					{
						
						SelfStudy_End();
						
						FX_Flag = 1;  //������ѧϰ������FX
						/*һֱ�ȴ��ڶ���SET�İ���*
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
						SelftStudyflag = 0;//�����ѧϰ���-- ��������ѧϰ
					}
				
			}
	}
}





void SelfStudy_End(void)
{					
	int32_t S_SET = 0;

			S_SET = Final;

			if(displayModeONE_FLAG)//����ģʽ
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
			else    //��׼ģʽ
			{
				NewThreshold = S_SET; 

				if(NewThreshold<=20) NewThreshold=20;
				if(NewThreshold>=4095) NewThreshold=4095;	

				Threshold = NewThreshold;
				WriteFlash(Threshold_FLASH_DATA_ADDRESS,Threshold);
			}
		
		GPIO_WriteBit(OUT1_GPIO_Port,OUT1_Pin,(BitAction)GPIO_ReadInputDataBit(OUT1_GPIO_Port,OUT1_Pin));
		GPIO_WriteBit(OUT2_GPIO_Port,OUT2_Pin,(BitAction)GPIO_ReadInputDataBit(OUT2_GPIO_Port,OUT2_Pin));
		
		
		if(S1_MaxValue <= S2_MaxValue)  /*SET_VREFΪSET1��SET2�н�С��ֵ*/
			SET_VREF = S1_MaxValue;
		else 
			SET_VREF = S2_MaxValue;
		
		Threshold = NewThreshold;
		
		selfDisplayEndFlay = 1;
		SetButton.PressCounter = 0;					/*�����������*/
		SetButton.Status = Release;					/*�ͷŰ���*/
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
				
				if(SET2_ADCValue>=S1_MaxValue)   //����Ѱ�����ֵ
				{
					S1_MaxValue = SET2_ADCValue;
				}
	}
}

/*��ȡ�ĸ�ADCͨ����������ƽ����ֵ*/
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

/*ADCIN�����ݵ���*/

void SelfStudy_SET1(void) 
{
		uint32_t 		SET1_ADCValue=0;

//	while(CalibrateS1Value>=1400 || CalibrateS1Value<=1000 || CalibrateS2Value>=1400 || CalibrateS2Value<=1000)
//	{
		if(sample_finish)
		{
			sample_finish = 0;

//			Get_S1_Value(&CalibrateS1Value);					//��ʱADC����
//			DACOUT1 = CalibrateS1Value;
//			
//			if(DACOUT1>=4095)
//				DACOUT1 = 4095;
//			else if(DACOUT1<=0)
//				DACOUT1 = 0;
//			DAC_SetChannel1Data(DAC_Align_12b_R,(uint16_t)DACOUT1);
//			DAC_SoftwareTriggerCmd(DAC_Channel_1,ENABLE);
//			/*******************************************************/
//			Get_S2_Value(&CalibrateS2Value);					//��ʱADC����

//			DACOUT2 = CalibrateS2Value;
//			if(DACOUT2>=4095)
//				DACOUT2 = 4095;
//			else if(DACOUT2<=0)
//				DACOUT2 = 0;
//			DAC_SetChannel2Data(DAC_Align_12b_R,(uint16_t)DACOUT2);
//			DAC_SoftwareTriggerCmd(DAC_Channel_2,ENABLE);
//			

				SET1_ADCValue = 	Final ;
				
				if(SET1_ADCValue>=S1_MaxValue)   //����Ѱ�����ֵ
				{
					S1_MaxValue = SET1_ADCValue;
				}

	}
}

