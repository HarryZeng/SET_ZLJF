
///**
//  ********************************  STM32F0x1  *********************************
//  * @文件名     ： differentialDC.c
//  * @作者       ： HarryZeng
//  * @库版本     ： V1.5.0
//  * @文件版本   ： V1.0.0
//  * @日期       ： 2017年05月11日
//  * @摘要       ： 数据处理
//  ******************************************************************************/
///*----------------------------------------------------------------------------
//  更新日志:
//  2017-05-11 V1.0.0:初始版本
//  ----------------------------------------------------------------------------*/
///* 包含的头文件 --------------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "project.h"
#include "bsp_init.h"
#include "key.h"
#include "display.h"
#include "SelfStudy.h"
#include "menu.h"
#include "flash.h"
#include "stm32f0xx_dma.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_dac.h"

/*DSP库宏定义：ARM_MATH_CM0*/

uint32_t DealyBaseTime = 8;
uint16_t DEL = 50;

int16_t HI = 1000;
int16_t LO = 500;
bool timeflag = 0;
uint8_t LastOUT1 = 1;
uint8_t RegisterA = 0;
uint8_t LastRegisterA = 0;
uint8_t RegisterB = 1;
uint8_t RegisterC = 0;
uint8_t OUT1 = 0;
uint8_t OUT2 = 0;
uint8_t OUT3 = 0;
int16_t OUT2_TimerCounter = 0;
uint16_t OUT2_Timer = 0;
int16_t OUT3_TimerCounter = 0;
uint16_t OUT3_Timer = 0;
uint32_t ADCRawValue = 0;
int32_t ADC_Display = 0;
int32_t DACOUT1 = 1000;
int32_t DACOUT2 = 1000;
uint32_t CPV = 0;

Button_STATUS KEY = ULOC;
//uint8_t ConfirmShortCircuit = 0;
//uint32_t ShortCircuitCounter = 0;
//uint32_t ShortCircuitLastTime = 0;
uint8_t KeyMappingFlag = 0;
uint8_t EventFlag = 0x00;
uint8_t ShortCircuit = 0;
//uint32_t ShortCircuitTimer = 0;
uint8_t displayModeONE_FLAG = 0;
uint8_t DisplayModeNo = 0;

void SetRegisterA(uint32_t GetADCValue);
void DisplayMODE(void);
void DisplayModeONE(void);
void DisplayModeTWO(void);
void DisplayModeTHIRD(void);
void DisplayModeFour(void);
void ShortCircuitProtection(void);
void SetOUT1Status(void);
void SetOUT2Status(void);
void SetOUT3Status(void);
void ButtonMapping(void);
void DEL_Set(void);
void DisplayModeONE_STD(void);
void DisplayModeONE_AREA(void);
void ResetParameter(void);
void Test_Delay(uint32_t ms);
uint32_t ADCDispalyProcess(uint32_t *ADC_RowValue, uint16_t Length);
uint8_t CheckDust(void);

extern int8_t PERCENTAGE;
extern int16_t ATT100;
extern uint16_t FSV;
extern int32_t SV;
extern uint8_t SelftStudyflag;
extern int8_t DSC;
/*----------------------------------宏定义-------------------------------------*/

uint8_t DustFlag = 0;

int32_t SA = 0;
int32_t SA_Sum = 0;
float Final_1 = 0;
float Final = 0;

int32_t SX[4];
int32_t SX_Final[32];
uint8_t SX_Flag;
uint8_t SX_Index = 0;

int32_t S_Final = 0; /*S-SET*/
uint8_t S_Final_FinishFlag = 0;

uint32_t S32 = 0;
uint8_t S32_Flag = 0;


uint32_t S_Selft = 0;

int16_t Threshold = 1000;

int16_t DX = 0;
int16_t DX_Index = 0;
int16_t Last_DX = 0;
int16_t Min_DX = 4095;
int16_t Max_DX = 0;
uint8_t DX_Flag = 1;

int32_t TX_Index = 0;
int32_t TX = 0;
int32_t TX_Temp[4]={4096,4096,4096,4096};
int32_t TX_Sum=0;

int32_t 	SET_VREF = 0;
int16_t 	FX = 0;
uint8_t 	FX_Flag = 1;

//uint32_t Display_Signal[256];
//uint32_t Display_Signal_Index = 0;
//uint32_t Display_Signal_Flag = 0;
//uint32_t DisplayADCValue_Sum = 0;
///*去除最大最小值后剩下的数据求平均*/
//uint32_t DeleteMaxAndMinGetAverage(uint32_t *ary, uint8_t Length, uint32_t *Max, uint32_t *Min)
//{
//	int j;
//	uint32_t Sum = 0;
//	*Max = ary[0];
//	*Min = ary[0];

//	for (j = 1; j < Length; j++)
//	{
//		if (*Max < ary[j])
//			*Max = ary[j];

//		if (*Min > ary[j])
//			*Min = ary[j];
//	}

//	for (j = 0; j < Length; j++)
//	{
//		Sum += ary[j];
//	}
//	//return (Sum-*Max-*Min)/(Length-2);
//	return (Sum) / (Length);
//}
/*求总和*/
void GetSum(uint32_t *SUM, uint32_t *arry, uint8_t arryLength)
{
	int j;
	for (j = 0; j < arryLength; j++)
	{
		*SUM += arry[j];
	}
}

/*求平均*/
void GetAverage(uint32_t *Average, uint32_t *arry, uint8_t arryLength)
{
	int j;
	uint32_t sum;
	for (j = 0; j < arryLength; j++)
	{
		sum += arry[j];
	}
	*Average = sum / j;
}

///*清零函数*/
//void ClearData(uint32_t *ary, uint8_t Length)
//{
//	int j;
//	for (j = 0; j < Length; j++)
//	{
//		*ary = 0x00;
//		ary++; /*指针移位*/
//	}
//}

///*记录CPV并设置OUT的输出*/
//void CPV_SET_OUT(void)
//{
//	//uint8_t OUT1_LED,OUT2_LED,OUT3_LED;
//	if (LastOUT1 == 0 && OUT1 == 1)
//	{
//		CPV++;
//		if (CPV >= SV) /*如果计数器达到预先设定的SV，清零，OUT2输出一个高电平*/
//		{
//			OUT2 = 1;
//			//CPV = 0;
//		}
//		if (CPV >= FSV) /*如果计数器达到预先设定的FSV，清零，OUT3输出一个高电平*/
//		{
//			OUT3 = 1;
//			CPV = 0;
//		}
//	}
//	LastOUT1 = OUT1;
//	//		/*显示OUT1和OUT2的状态*/
//	SMG_DisplayOUT_STATUS(OUT1, OUT2);
//}

//uint8_t PWMCounter = 0;
//uint32_t S1024 = 0;
//uint16_t S1024_Index = 0;
//uint32_t S1024_Sum = 0;

#define S1024_Or_S8192 1024
void JudgeDX(void)
{
//	/*判断灰层影响程度*/
//	if (displayModeONE_FLAG == 0)
//	{
//		if (RegisterA) //STD——Mode
//		{

//			//S1024_Sum = S1024_Sum + S_Final;
//			S1024_Sum = S1024_Sum + SX_Final[SX_Index];
//			S1024_Index++;
//			if (S1024_Index >= S1024_Or_S8192)
//			{
//				S1024_Index = 0;
//				S1024 = S1024_Sum / S1024_Or_S8192; //求得S1024

//				if (DSC)				//modifiy 20180105
//					DX = S_SET - S1024; //modifiy 20171230
//				else
//					DX = 0;

//				S1024_Sum = 0;
//			}
//			if (DX <= -2000) //有疑问，当最小是-1500时，没有比它更小的了
//				DX = -2000;
//			else if (DX >= 2000)
//				DX = 2000;
//			//
//			//			Last_DX = DX;
//			//
//			//			if(Last_DX<Min_DX) 	/*用于记录最小的DX值*/
//			//				Min_DX = DX;

//			//DustFlag = CheckDust(); /*灰层积聚严重，DUST*/
//		}
//	}
//	else if (displayModeONE_FLAG == 1) /*Area Mode*/
//	{
//		if (RegisterC)
//		{
//			//S1024_Sum = S1024_Sum + S_Final;
//			S1024_Sum = S1024_Sum + SX_Final[SX_Index];
//			S1024_Index++;
//			if (S1024_Index >= S1024_Or_S8192)
//			{
//				S1024_Index = 0;
//				S1024 = S1024_Sum / S1024_Or_S8192; //求得S1024
//				if (DSC)							//modifiy 20180105
//					DX = S_SET - S1024;				//modifiy 20171230
//				else
//					DX = 0;
//				S1024_Sum = 0;
//			}
//			if (DX <= -2000) //有疑问，当最小是-1500时，没有比它更小的了
//				DX = -2000;
//			else if (DX >= 2000)
//				DX = 2000;
//			//
//			//			Last_DX = DX;
//			//
//			//			if(Last_DX<Min_DX) 	/*用于记录最小的DX值*/
//			//				Min_DX = DX;
//			//
//			//				//DustFlag = CheckDust(); /*灰层积聚严重，DUST*/
//		}
//	}
}

///*冒泡排序*/
//void bubbleSort(int *arr, int n) 
//{
//	int i;
//	int j;
//	int temp;
//	for ( i= 0; i<n - 1; i++)
//			for (j = 0; j < n - i - 1; j++)
//			{
//					if (arr[j] > arr[j + 1]) 
//					{
//							temp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = temp;
//					}
//			}
//}


void JudgeTX(void)
{
	if(RegisterA==0)
	{
		if(TX_Temp[0]>Final_1)
			TX_Temp[0] = Final_1;
		else if(TX_Temp[1]>Final_1)
			TX_Temp[1] = Final_1;
		else if(TX_Temp[2]>Final_1)
			TX_Temp[2] = Final_1;
		else if(TX_Temp[3]>Final_1)
			TX_Temp[3] = Final_1;

		TX_Sum = TX_Temp[0]+TX_Temp[1]+TX_Temp[2]+TX_Temp[3];
		//TX_Index = 0;
		TX = TX_Sum / 4;
		TX_Sum = 0;
	
	}
}

uint8_t sample_finish = 0;
extern int32_t CSV;

uint32_t DMA_Counter = 0;
uint8_t RegisterA_1_Counter = 0;
uint8_t RegisterA_0_Counter = 0;
uint8_t TempRegisterA = 0;
int16_t DMA_ADC_Counter = 0;
uint8_t CheckCounter = 0;
uint8_t StartFlag=0;
void DMA1_Channel1_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA_IT_TC)) //判断DMA传输完成中断
	{
			SA = 4095 - adc_dma_tab[0];
			SA_Sum = SA_Sum+SA;
			DMA_ADC_Counter++;
			if (DMA_ADC_Counter >= 4)
			{
				DMA_ADC_Counter = 0;
				Final_1 = SA_Sum / 4;
				SA_Sum = 0;
				DMA_Counter++;
				if (StartFlag==1)  //用作延时200ms，50us*4次=200us，所以200ms = 200us*1000
				{
						if(displayModeONE_FLAG==1 || FX_Flag==0)	/**AREA模式下或者自学习情况下，FX=0，TX=0**/  //2018-7-12 去掉|| SelftStudyflag==1 
						{
							FX = 0;
							//TX = 0;
						}
						else/*STD模式下*/
						{					
							/******TX******/
							JudgeTX();
							/*******FX*******/
							FX = SET_VREF - TX;     /*求FX*/
							if(FX>=600) FX = 600;		/**FX范围**/
							else if(FX<=-600) FX = -600;
						}
						
						Final = Final_1 - FX ;  //最终信号值
						
						if(Final>=9999)
								Final = 9999;
						if(Final<=0)
								Final = 0;
						
						/***************DX*************/
						
						DX_Index++;
						if(Final>=Max_DX)
							Max_DX = Final;
						if(Final<=Min_DX)
							Min_DX = Final;
						if(DX_Index>6)
						{
							DX_Index = 0;
							DX = Max_DX - Min_DX;
							Max_DX = 0;    /*初始化变量*/
							Min_DX = 4095; /*初始化变量*/
						}
						
						/***********Register A**********/
						if(displayModeONE_FLAG==1)	/**AREA模式**/
						{
							if(Final>=LO+2+DX && Final<=HI-2-DX)
								RegisterA = 1;
							else if((Final>=0&&Final<=LO-20-LO/128-0.125*DX)||(Final>=HI+20+HI/128+0.125*DX && Final<=9999)) //2018-06-18
								RegisterA = 0;
						}
						else if(displayModeONE_FLAG==0)/**STD模式**/
						{
							if(Final>=Threshold+2+DX)  //2018-06-18  TH+2+DX
								RegisterA = 1;
							else if(Final<=Threshold-4-Threshold/128-0.125*DX) //2018-06-19 TH-4-TH/128-0.125*DX
								RegisterA = 0;
						}		

				sample_finish = 1;


				/*设置OUT1的状态*/
				SetOUT1Status();
					/*OUT2输出*/
				SetOUT2Status();
					
				/*显示OUT1和OUT2的状态*/
				SMG_DisplayOUT_STATUS(OUT1, OUT2);
			}
			else 
			{
				JudgeTX();//TX = Final_1;   /*200ms前，使用Final_1作为TX*/ 2018-7-11修改成JudgeTX();
				if(DMA_Counter >=1000)
				{
					StartFlag = 1;
					FX = SET_VREF;
				}
			}
		}
		DMA_ClearITPendingBit(DMA_IT_TC); //清楚DMA中断标志位
	}
}

/*PG120-ADC-DMA*/
//void DMA1_Channel1_IRQHandler(void)
//{
//	uint32_t 	SX_Max=0;
//	uint32_t 	SX_Min=0;

//	uint32_t 	TX_Max=0;
//	uint32_t 	TX_Min=0;

//	uint32_t 	Dispaly_Max=0;
//	uint32_t 	Dispaly_Min=0;
//
//	int32_t tempdata;
//
// 	/*判断DMA传输完成中断*/
//	if(DMA_GetITStatus(DMA1_IT_TC1) != RESET)
//	{
//		if(PWMCounter<100)
//			PWMCounter++;
//		S[0] = 4095-adc_dma_tab[0];
//		S[1] = 4095-adc_dma_tab[1];
//		S[2] = 4095-adc_dma_tab[2];
//		S[3] = 4095-adc_dma_tab[3];
//		GetSum(&SX[S_Index++],S,4);/*四个通道总和*/
//		S_Flag = 1;
//
//		if(S_Index>3)
//		{
//				S_Index = 0;
//				/*SX,FX*/
//				tempdata = DeleteMaxAndMinGetAverage(SX,4,&SX_Max,&SX_Min);
//				SX_Final[SX_Index] = tempdata - 3000;/*求得并去掉最大最小值，求剩下数据的平均值,需要求32组*/
//				/*数据限位*/
//				if(SX_Final[SX_Index]>=9999)
//					SX_Final[SX_Index] = 9999;
//				if(SX_Final[SX_Index]<=0)
//					SX_Final[SX_Index] = 0;
//
//				/*求得S最终信号*/
//				S_Final = SX_Final[SX_Index] + DX;	/*获得最终信号值 信号值加上DX，得到的数据作为最终信号值*/
//				if(S_Final>=9999)
//					S_Final=9999;
//				if(S_Final<=0)
//					S_Final=0;
//				S_Final_FinishFlag = 1;
//
//
//				if(PWMCounter>50)/*大于等于50个PWM脉冲，才开始计算RegisterA*/
//					SetRegisterA(S_Final);/*判断RegisterA的状态*/
//				if(PWMCounter>80)
//					SetOUT1Status();/*大于等于50个PWM脉冲，才开始设置OUT输出*/
//
//				FX = (SX_Max-SX_Min);  /*求得FX*/
//				ClearData(SX,4);/*清零*/
//
//				/*TX*/
//				GetSum(&TX_Signal[TX_Index++],&S_Final,1);/*六次总和,TX*///20180106
//				if(TX_Index>=6)
//				{
//					TX_Index = 0;
//					DeleteMaxAndMinGetAverage(TX_Signal,6,&TX_Max,&TX_Min);
//					TX = (TX_Max-TX_Min);/*求得TX*/
//					//TX = 0;
//					ClearData(TX_Signal,8);/*清零*/
//				}
//
//				/*显示数据计数*/
//				GetSum(&DisplayADCValue_Sum,&S_Final,1);
//				Display_Signal_Index++;
//				if(Display_Signal_Index>=256)
//				{
//					ADCRawValue = DisplayADCValue_Sum/256;
//					Display_Signal_Index = 0;
//					Display_Signal_Flag	=	1;
//					DisplayADCValue_Sum = 0;
//
//				}

//				/*根据CPV，设置OUT输出*/
//				CPV_SET_OUT();
//				/*判断灰层*/
//				if(DX_Flag==1)
//					JudgeDX();
//				/*累计32组数据*/
//				SX_Index++;
//				if(SX_Index>31)
//				{
//					SX_Index = 0;
//					SX_Flag = 1;
//					/*自学习*/
//					if(SelftStudyflag)
//					{
//						DX = 0;
//						S1024 = 0;
//						//GetAverage(&S_SET,SX_Final,32); /*自学习，求得S-SET*/
//						//Threshold = S_SET-80;   /*更新阀值*/
//						S_SET = S_Final;
//
//						if(displayModeONE_FLAG)//区域模式
//						{
//							if(DisplayModeNo==0)
//							{
//								HI = S_SET*(100-PERCENTAGE)/100;   /*更新阀值,=S-SET*(1-1%)=S-SET*0.99=S-SET*99/100*/;
//								WriteFlash(HI_FLASH_DATA_ADDRESS,HI);
//							}
//							else if(DisplayModeNo==1)
//							{
//								LO =S_SET*(100-PERCENTAGE)/100;   /*更新阀值,=S-SET*(1-1%)=S-SET*0.99=S-SET*99/100*/;
//								WriteFlash(LO_FLASH_DATA_ADDRESS,LO);
//							}
//						}
//						else    //标准模式
//						{
//							Threshold = S_SET*(100-PERCENTAGE)/100;   /*更新阀值,=S-SET*(1-1%)=S-SET*0.99=S-SET*99/100*/
//							WriteFlash(Threshold_FLASH_DATA_ADDRESS,Threshold);
//						}
//						SelftStudyflag = 0;
//
//						WriteFlash(S_SET_FLASH_DATA_ADDRESS,S_SET);
//					}
//				}
//				/*计算时间*/
//				RunCounter[RunIndex] = TIM_GetCounter(TIM4);
//				if(RunCounter[RunIndex] >=3200)
//					Runflag = 1;
//				RunIndex++;
//		}
//	}
//	/*清除DMA中断标志位*/
//	DMA_ClearITPendingBit(DMA1_IT_TC1);
//}

/*采样完成后，ADC数据处理*/
uint32_t ADCDispalyProcess(uint32_t *ADC_RowValue, uint16_t Length)
{
	uint32_t DisplayADCValue = 0;
	uint32_t SumADCValue = 0;
	uint16_t k;
	for (k = 0; k < Length; k++)
		SumADCValue += ADC_RowValue[k];

	DisplayADCValue = SumADCValue / (k);

	return DisplayADCValue;
}

/*计算OUT1*/
/********************
*
*判断出RegisterA状态
*
**********************/
uint8_t GetRegisterAState(uint32_t ADCValue)
{
	uint8_t A;

	if (ADCValue >= Threshold)
		A = 1;
	else if (ADCValue <= Threshold - 50)
		A = 0;
	return A;
}

/*	正常显示*/
/********************
*
*显示ADC的值处理
*
**********************/

void GetTotalADCValue(void)
{
	uint32_t Dispaly_Max = 0;
	uint32_t Dispaly_Min = 0;

	ADC_Display = Final;
}

//void Get_SA_Value(uint32_t *SAvalue)
//{
//	/*正常显示*/
//	//*SAvalue = SA_Final;
//}

//void Get_SB_Value(uint32_t *SBvalue)
//{
//	/*正常显示*/
//	//*SBvalue =SB_Final;
//}

//void Get_S1_Value(uint32_t *S1value)
//{
//	/*正常显示*/
//	//*S1value =S1_Final;
//}
//void Get_S2_Value(uint32_t *S2value)
//{
//	/*正常显示*/
//	//*S2value =S2_Final;
//}

//uint16_t Dac1_Get_Vol(void)
//{
//	return DAC_GetDataOutputValue(DAC_Channel_2);
//}

uint16_t RealDACOUT = 0;

void Main_Function(void)
{
	GetEEPROM();
	
	while (1)
	{
		if (0)
		{
			Dust_Display();
			GPIO_WriteBit(OUT1_GPIO_Port, OUT1_Pin, Bit_RESET); /*一直将OUT1拉低*/
		}
		else
		{

			//RealDACOUT = Dac1_Get_Vol();
			/*正常显示模式*/
			DisplayMODE();

			/*按键复用*/
			ButtonMapping();

			if (KEY == ULOC) /*判断按键是否上锁*/
			{
				/*SET自学习模式*/
				if ((DisplayModeNo == 0 && displayModeONE_FLAG == 0) || (DisplayModeNo <= 1 && displayModeONE_FLAG == 1))
					selfstudy();
				/*Mode菜单模式*/
				menu();
			}
		}
	}
}

/*********************************
灰层积聚严重，DUST
**********************************/
uint8_t CheckDust(void)
{
	//if(DX>(S32/2))
	if (DX >= 1501)
	{
		return 1;
	}
	else
		return 0;
}

/*******************************
*
*显示模式切换
*
*******************************/

void DisplayMODE(void)
{
	if (ModeButton.Status == Press && (DownButton.Status == Press || SetButton.Status == Press || UpButton.Status == Press))
	{
	}
	else
	{
		if (KEY == ULOC)
		{
			if (ModeButton.PressCounter == 0)
			{
				DisplayModeNo = 0;
			}
			else if (ModeButton.Effect == PressShort && ModeButton.PressCounter == 1 && DownButton.Status == Release && UpButton.Status == Release)
			{
				DisplayModeNo = 1;
			}
			else if (ModeButton.Effect == PressShort && ModeButton.PressCounter == 2 && DownButton.Status == Release && UpButton.Status == Release)
			{
				DisplayModeNo = 2;
			}
//			else if (ModeButton.Effect == PressShort && ModeButton.PressCounter == 3 && DownButton.Status == Release && UpButton.Status == Release)
//			{
//				DisplayModeNo = 3;
//			}
			//			else if(ModeButton.Effect == PressShort && ModeButton.PressCounter==4 &&DownButton.Status==Release&&UpButton.Status==Release)
			//			{
			//				DisplayModeNo = 4;
			//			}
			/********mode 按键 循环*******/
			if (ModeButton.Effect == PressShort && ModeButton.PressCounter == 2 && DownButton.Status == Release && displayModeONE_FLAG == 0) //if need to display Mode_Four,PressCounter=4
			{
				ModeButton.PressCounter = 0;
				DisplayModeNo = 0;
			}
			else if (ModeButton.Effect == PressShort && ModeButton.PressCounter == 3 && DownButton.Status == Release && displayModeONE_FLAG == 1) //if need to display Mode_Four,PressCounter=5
			{
				ModeButton.PressCounter = 0;
				DisplayModeNo = 0;
			}
		}
		else
		{
			while (ModeButton.Status == Press || SetButton.Status == Press || DownButton.Status == Press || UpButton.Status == Press)
			{
				if (ModeButton.Effect == PressLong && UpButton.Status == Release && DownButton.Effect == PressLong && DownButton.Status == Press)
				{
					if (KEY == ULOC)
						KEY = LOC;
					else
					{
						KEY = ULOC;
						KeyMappingFlag = 0;
					}
					ModeButton.PressCounter = 0;
					while (ModeButton.Effect == PressLong && UpButton.Status == Release && DownButton.Effect == PressLong && DownButton.Status == Press)
					{
						ButtonMappingDisplay(1);
					}
					WriteFlash(KEY_FLASH_DATA_ADDRESS, KEY);
					ModeButton.Effect = PressNOEffect;
					ModeButton.PressCounter = 0;
					DownButton.PressCounter = 0;
					DownButton.Effect = PressNOEffect;
				}
				else
					ButtonMappingDisplay(1);
			}
		}
		/*显示*/
		if ((DisplayModeNo == 0 && displayModeONE_FLAG == 0) || (DisplayModeNo <= 1 && displayModeONE_FLAG == 1))
		{
			DisplayModeONE();
		}
		else if ((DisplayModeNo == 1 && displayModeONE_FLAG == 0) || (DisplayModeNo == 2 && displayModeONE_FLAG == 1))
		{
			//DisplayModeTWO();
			DisplayModeTHIRD();
		}
		else if ((DisplayModeNo == 2 && displayModeONE_FLAG == 0) || (DisplayModeNo == 3 && displayModeONE_FLAG == 1))
		{
			//DisplayModeTHIRD();
			//DisplayModeFour();
		}
		else if ((DisplayModeNo == 3 && displayModeONE_FLAG == 0) || (DisplayModeNo == 4 && displayModeONE_FLAG == 1))
		{
			//DisplayModeFour();
		}
	}
}

/*******************************
*
*显示模式1
*
*******************************/
int8_t LastDSC;
void DisplayModeONE(void)
{
	if (displayModeONE_FLAG)
	{
		if (DSC)
		{
			LastDSC = DSC;
			DSC = 0; //区域模式下，DX一直为0
		}
		DisplayModeONE_AREA();
	}
	else
	{
		if (LastDSC)
		{
			LastDSC = 0;
			DSC = 1;
		}
		DisplayModeONE_STD();
	}
}

/*******************************
*
*显示模式1_DETECT_STD
*
*******************************/
uint32_t tempPress = 0;
void DisplayModeONE_STD(void)
{
	static uint8_t lastCounter;
	static int16_t LastThreshold;

	/*数码管显示*/
	SMG_DisplayModeONE(Threshold, ADC_Display);

	if (ModeButton.Status == Release && KeyMappingFlag == 0 && KEY == ULOC)
	{
		/*Up Button*/
		LastThreshold = Threshold;
		if (UpButton.PressCounter != lastCounter && UpButton.Effect == PressShort)
		{
			lastCounter = UpButton.PressCounter;
			UpButton.PressCounter = 0;
			Threshold = Threshold + 1;
		}
		else if (UpButton.Status == Press && (UpButton.Effect == PressLong))
		{ /*还按着按键，并且时间超过长按时间*/
			UpButton.PressCounter = 0;
			if (UpButton.PressTimer <= KEY_LEVEL_1)
			{
				if (UpButton.PressTimer % KEY_LEVEL_1_SET == 0 && tempPress == 1)
				{
					Threshold = Threshold + 1;
					tempPress = 0;
				}
			}
			else if (UpButton.PressTimer > KEY_LEVEL_1 && UpButton.PressTimer <= KEY_LEVEL_2)
			{
				if (UpButton.PressTimer % KEY_LEVEL_2_SET == 0 && tempPress == 1)
				{
					tempPress = 0;
					Threshold = Threshold + 2;
				}
			}
			else
			{
				if (UpButton.PressTimer % KEY_LEVEL_3_SET == 0 && tempPress == 1)
				{
					tempPress = 0;
					Threshold = Threshold + 5;
				}
			}
		}
		else
		{
			UpButton.Effect = PressShort;
		}
		/*Down Button*/
		if (DownButton.PressCounter != lastCounter && DownButton.Effect == PressShort)
		{
			DownButton.PressCounter = 0;
			Threshold = Threshold - 1;
		}
		else if (DownButton.Status == Press && (DownButton.Effect == PressLong))
		{
			DownButton.PressCounter = 0;
			if (DownButton.PressTimer < KEY_LEVEL_1)
			{
				if (DownButton.PressTimer % KEY_LEVEL_1_SET == 0 && tempPress == 1)
				{
					Threshold = Threshold - 1;
					tempPress = 0;
				}
			}
			else if (DownButton.PressTimer > KEY_LEVEL_1 && DownButton.PressTimer < KEY_LEVEL_2)
			{
				if (DownButton.PressTimer % KEY_LEVEL_2_SET == 0 && tempPress == 1)
				{
					Threshold = Threshold - 2;
					tempPress = 0;
				}
			}
			else
			{
				if (DownButton.PressTimer % KEY_LEVEL_3_SET == 0 && tempPress == 1)
				{
					tempPress = 0;
					Threshold = Threshold - 5;
				}
			}
		}
		else
		{
			DownButton.Effect = PressShort;
		}
		if (LastThreshold != Threshold && DownButton.Status == Release && UpButton.Status == Release)
		{
			WriteFlash(Threshold_FLASH_DATA_ADDRESS, Threshold);
		}
	}
	if (Threshold >= 9999)
		Threshold = 9999;
	else if (Threshold <= 0)
		Threshold = 0;
}

/*******************************
*
*显示模式1_DETECT_AREA
*
*******************************/
void DisplayModeONE_AREA(void)
{
	static uint8_t lastCounter;
	static int16_t LastHIValue;
	static int16_t LastLOValue;

	/*HI display mode*/
	if (DisplayModeNo == 0)
	{
		if (ModeButton.Status == Release && KeyMappingFlag == 0 && KEY == ULOC)
		{
			/*Up Button*/
			LastHIValue = HI;
			if (UpButton.PressCounter != lastCounter && UpButton.Effect == PressShort)
			{
				lastCounter = UpButton.PressCounter;
				UpButton.PressCounter = 0;
				HI = HI + 1;
				if (HI >= 9999)
					HI = 9999;
				SMG_DisplayModeONE_Detect_AREA_HI(1, HI, ADC_Display); /*显示阀值*/
			}
			else if (UpButton.Status == Press && (UpButton.Effect == PressLong))
			{ /*还按着按键，并且时间超过长按时间*/
				UpButton.PressCounter = 0;
				if (UpButton.PressTimer <= KEY_LEVEL_1)
				{
					if (UpButton.PressTimer % KEY_LEVEL_1_SET == 0 && tempPress == 1)
					{
						tempPress = 0;
						HI = HI + 1;
					}
				}
				else if (UpButton.PressTimer > KEY_LEVEL_1 && UpButton.PressTimer <= KEY_LEVEL_2)
				{
					if (UpButton.PressTimer % KEY_LEVEL_2_SET == 0 && tempPress == 1)
					{
						tempPress = 0;
						HI = HI + 2;
					}
				}
				else
				{
					if (UpButton.PressTimer % KEY_LEVEL_3_SET == 0 && tempPress == 1)
					{
						tempPress = 0;
						HI = HI + 5;
					}
				}
				if (HI >= 9999)
					HI = 9999;
				SMG_DisplayModeONE_Detect_AREA_HI(1, HI, ADC_Display); /*显示阀值*/
			}
			//					else
			//					{
			//						UpButton.Effect = PressShort;
			//						SMG_DisplayModeONE_Detect_AREA_HI(timeflag,HI,ADC_Display); /*交替显示HI与阀值*/
			//					}
			/*Down Button*/
			else if (DownButton.PressCounter != lastCounter && DownButton.Effect == PressShort)
			{
				DownButton.PressCounter = 0;
				HI = HI - 1;
				if (HI <= 0)
					HI = 0;
				SMG_DisplayModeONE_Detect_AREA_HI(1, HI, ADC_Display); /*显示阀值*/
			}
			else if (DownButton.Status == Press && (DownButton.Effect == PressLong))
			{
				DownButton.PressCounter = 0;
				if (DownButton.PressTimer < KEY_LEVEL_1)
				{
					if (DownButton.PressTimer % KEY_LEVEL_1_SET == 0 && tempPress == 1)
					{
						tempPress = 0;
						HI = HI - 1;
					}
				}
				else if (DownButton.PressTimer > KEY_LEVEL_1 && DownButton.PressTimer < KEY_LEVEL_2)
				{
					if (DownButton.PressTimer % KEY_LEVEL_2_SET == 0 && tempPress == 1)
					{
						tempPress = 0;
						HI = HI - 2;
					}
				}
				else
				{
					if (DownButton.PressTimer % KEY_LEVEL_3_SET == 0 && tempPress == 1)
					{
						tempPress = 0;
						HI = HI - 5;
					}
				}
				if (HI <= 0)
					HI = 0;
				SMG_DisplayModeONE_Detect_AREA_HI(1, HI, ADC_Display); /*显示阀值*/
			}
			else
			{
				DownButton.Effect = PressShort;
				SMG_DisplayModeONE_Detect_AREA_HI(timeflag, HI, ADC_Display); /*交替显示HI与阀值*/
			}
			if (LastHIValue != HI && DownButton.Status == Release && UpButton.Status == Release)
			{
				WriteFlash(HI_FLASH_DATA_ADDRESS,HI);
			}
		}
	}

	/*LO display mode*/
	else if (DisplayModeNo == 1)
	{
		if (ModeButton.Status == Release && KeyMappingFlag == 0 && KEY == ULOC)
		{
			/*Up Button*/
			LastLOValue = LO;
			if (UpButton.PressCounter != lastCounter && UpButton.Effect == PressShort)
			{
				lastCounter = UpButton.PressCounter;
				UpButton.PressCounter = 0;
				LO = LO + 1;
				if (LO >= 9999)
					LO = 9999;
				SMG_DisplayModeONE_Detect_AREA_LO(1, LO, ADC_Display); /*显示阀值*/
			}
			else if (UpButton.Status == Press && (UpButton.Effect == PressLong))
			{ /*还按着按键，并且时间超过长按时间*/
				UpButton.PressCounter = 0;
				if (UpButton.PressTimer <= KEY_LEVEL_1)
				{
					if (UpButton.PressTimer % KEY_LEVEL_1_SET == 0 && tempPress == 1)
					{
						tempPress = 0;
						LO = LO + 1;
					}
				}
				else if (UpButton.PressTimer > KEY_LEVEL_1 && UpButton.PressTimer <= KEY_LEVEL_2)
				{
					if (UpButton.PressTimer % KEY_LEVEL_2_SET == 0 && tempPress == 1)
					{
						tempPress = 0;
						LO = LO + 2;
					}
				}
				else
				{
					if (UpButton.PressTimer % KEY_LEVEL_3_SET == 0 && tempPress == 1)
					{
						tempPress = 0;
						LO = LO + 5;
					}
				}
				if (LO >= 9999)
					LO = 9999;
				SMG_DisplayModeONE_Detect_AREA_LO(1, LO, ADC_Display); /*显示阀值*/
			}
			//					else
			//					{
			//						UpButton.Effect = PressShort;
			//						SMG_DisplayModeONE_Detect_AREA_LO(timeflag,LO,ADC_Display);/*交替显示LO与阀值*/
			//					}
			/*Down Button*/
			else if (DownButton.PressCounter != lastCounter && DownButton.Effect == PressShort)
			{
				DownButton.PressCounter = 0;
				LO = LO - 1;
				if (LO <= 0)
					LO = 0;
				SMG_DisplayModeONE_Detect_AREA_LO(1, LO, ADC_Display); /*显示阀值*/
			}
			else if (DownButton.Status == Press && (DownButton.Effect == PressLong))
			{
				DownButton.PressCounter = 0;
				if (DownButton.PressTimer < KEY_LEVEL_1)
				{
					if (DownButton.PressTimer % KEY_LEVEL_1_SET == 0 && tempPress == 1)
					{
						tempPress = 0;
						LO = LO - 1;
					}
				}
				else if (DownButton.PressTimer > KEY_LEVEL_1 && DownButton.PressTimer < KEY_LEVEL_2)
				{
					if (DownButton.PressTimer % KEY_LEVEL_2_SET == 0 && tempPress == 1)
					{
						tempPress = 0;
						LO = LO - 2;
					}
				}
				else
				{
					if (DownButton.PressTimer % KEY_LEVEL_3_SET == 0 && tempPress == 1)
					{
						tempPress = 0;
						LO = LO - 5;
					}
				}
				if (LO <= 0)
					LO = 0;
				SMG_DisplayModeONE_Detect_AREA_LO(1, LO, ADC_Display); /*显示阀值*/
			}
			else
			{
				DownButton.Effect = PressShort;
				SMG_DisplayModeONE_Detect_AREA_LO(timeflag, LO, ADC_Display); /*交替显示LO与阀值*/
			}
			if (LastLOValue != LO && DownButton.Status == Release && UpButton.Status == Release)
			{
				WriteFlash(LO_FLASH_DATA_ADDRESS,LO);
			}
		}
	}
}

/*******************************
*
*显示模式2
*
*******************************/
void DisplayModeTWO(void)
{
	static uint8_t lastCounter;
	/*数码管显示*/
	SMG_DisplayModeTWO(RegisterB);

	/*Up Button*/
	if (UpButton.PressCounter != lastCounter && UpButton.Effect == PressShort)
	{
		lastCounter = UpButton.PressCounter;
		UpButton.PressCounter = 0;
		if (RegisterB == 0)
			RegisterB = 1;
		else
			RegisterB = 0;
		WriteFlash(RegisterB_FLASH_DATA_ADDRESS, RegisterB);
	}

	/*Down Button*/
	if (DownButton.PressCounter != lastCounter && DownButton.Effect == PressShort)
	{
		DownButton.PressCounter = 0;
		if (RegisterB == 0)
			RegisterB = 1;
		else
			RegisterB = 0;
		WriteFlash(RegisterB_FLASH_DATA_ADDRESS, RegisterB);
	}
}

/*******************************
*
*显示模式3
*
*******************************/
void DisplayModeTHIRD(void)
{
	/*数码管显示*/
	SMG_DisplayModeTHIRD(CPV);
	/*以下为清楚按键计数，防止会影响到显示模式4*/
	DownButton.PressCounter = 0;
	UpButton.PressCounter = 0;
}

/*******************************
*
*显示模式4
*
*******************************/
void DisplayModeFour(void)
{
	DEL_Set();
}

/*******************************
*
*OUT1_DEL 应差值设置
*
*******************************/
void DEL_Set(void)
{
	static uint8_t lastCounter;
	uint16_t LastDEL;

	/*显示SHOT,闪烁*/
	SMG_DisplayMenuTwo_DEL_SET(DEL, 0);

	/*Up Button*/
	LastDEL = DEL;
	if (UpButton.PressCounter != lastCounter && UpButton.Effect == PressShort)
	{
		lastCounter = UpButton.PressCounter;
		UpButton.PressCounter = 0;
		DEL = DEL + 1;
	}
	else if (UpButton.Status == Press && (UpButton.Effect == PressLong))
	{ /*还按着按键，并且时间超过长按时间*/
		UpButton.PressCounter = 0;
		if (UpButton.PressTimer < KEY_LEVEL_1)
		{
			if (UpButton.PressTimer % KEY_LEVEL_1_SET == 0)
				DEL = DEL + 1;
		}
		else if (UpButton.PressTimer > KEY_LEVEL_1 && UpButton.PressTimer < KEY_LEVEL_2)
		{
			if (UpButton.PressTimer % KEY_LEVEL_2_SET == 0)
				DEL = DEL + 1;
		}
		else
		{
			if (UpButton.PressTimer % KEY_LEVEL_3_SET == 0)
				DEL = DEL + 1;
		}
	}
	else
	{
		UpButton.Effect = PressShort;
	}
	/*Down Button*/
	if (DownButton.PressCounter != lastCounter && DownButton.Effect == PressShort)
	{
		DownButton.PressCounter = 0;
		DEL = DEL - 1;
	}
	else if (DownButton.Status == Press && (DownButton.Effect == PressLong))
	{ /*还按着按键，并且时间超过长按时间*/
		DownButton.PressCounter = 0;
		if (DownButton.PressTimer < KEY_LEVEL_1)
		{
			if (DownButton.PressTimer % KEY_LEVEL_1_SET == 0)
				DEL = DEL - 1;
		}
		else if (DownButton.PressTimer > KEY_LEVEL_1 && DownButton.PressTimer < KEY_LEVEL_2)
		{
			if (DownButton.PressTimer % KEY_LEVEL_2_SET == 0)
				DEL = DEL - 1;
		}
		else
		{
			if (DownButton.PressTimer % KEY_LEVEL_3_SET == 0)
				DEL = DEL - 1;
		}
	}
	else
	{
		DownButton.Effect = PressShort;
	}
	if (LastDEL != DEL && DownButton.Status == Release && UpButton.Status == Release)
	{
		//WriteFlash(DEL_FLASH_DATA_ADDRESS,DEL);
	}
	if (DEL >= 500)
		DEL = 500;
	else if (DEL <= 4)
		DEL = 4;
	//	}
}

/*******************************
*
*Set RegisterA value
*
*******************************/
void SetRegisterA(uint32_t ADCTestValue)
{
	//	TX = 0; //debug
	//	DX = 0;
	if (displayModeONE_FLAG) /*AREA Mode*/
	{
		if (ADCTestValue >= LO + TX && ADCTestValue <= HI - TX - 80 - HI / 128)
			RegisterA = 1;
		else if (((ADCTestValue >= (HI + TX)) && (ADCTestValue <= 9999)) || (ADCTestValue <= (LO - TX - 80 - LO / 128))) //20180106
			RegisterA = 0;

		/*RegisterC*/
		if (ADCTestValue >= HI + TX)
			RegisterC = 1;
		else if (ADCTestValue <= HI - TX - HI / 128)
			RegisterC = 0;
	}
	else /*STD Mode*/
	{
		if (ADCTestValue >= Threshold + TX) //20171231
			RegisterA = 1;
		else if (ADCTestValue <= (Threshold - TX - 40 - Threshold / 256)) /*20171223*/ //2018-1-5 修改成-80,//20180106 改成-40 /256
			RegisterA = 0;
	}
}

/*******************************
*
*判断OUT1的输出状态
*
*******************************/
uint8_t SHOTflag = 0;
uint8_t CPV_Status=0;

void SetOUT1Status(void)
{
	if (ShortCircuit != 1) /*不是短路保护的情况下才判断OUT1的输出*/
	{
		/*同或运算*/
//		if (GPIO_ReadInputDataBit(BUTTON_SWITCH_GPIO_Port, BUTTON_SWITCH_Pin))
//			RegisterB = 0;
//		else
//			RegisterB = 1;

		OUT1 = !(RegisterB ^ RegisterA);
		if (OUT1_Mode.DelayMode == TOFF)
		{
			//GPIOA->ODR ^= GPIO_Pin_9;
			if (OUT1 == 0)
			{
				GPIO_WriteBit(OUT1_GPIO_Port, OUT1_Pin, Bit_RESET);
				OUT1_Mode.DelayCounter = 0;
			}
			else
			{
				GPIO_WriteBit(OUT1_GPIO_Port, OUT1_Pin, Bit_SET);
				OUT1_Mode.DelayCounter = 0;
				CPV_Status = 1;
			}
		}
		/*OFFD*/
		else if (OUT1_Mode.DelayMode == OFFD)
		{
			if (OUT1 == 0)
			{
				if (OUT1_Mode.DelayCounter > (OUT1_Mode.DelayValue * DealyBaseTime))
				{
					GPIO_WriteBit(OUT1_GPIO_Port, OUT1_Pin, Bit_RESET);
					CPV_Status = 1;
				}
			}
			else
			{
				GPIO_WriteBit(OUT1_GPIO_Port, OUT1_Pin, Bit_SET);
				OUT1_Mode.DelayCounter = 0;
			}			
		}
		/*ON_D*/
		else if (OUT1_Mode.DelayMode == ON_D)
		{
			if (OUT1 == 0)
			{
				GPIO_WriteBit(OUT1_GPIO_Port, OUT1_Pin, Bit_RESET);
				OUT1_Mode.DelayCounter = 0;
			}
			else
			{
				if (OUT1_Mode.DelayCounter > (OUT1_Mode.DelayValue * DealyBaseTime))
				{
					GPIO_WriteBit(OUT1_GPIO_Port, OUT1_Pin, Bit_SET);
					CPV_Status = 1;
				}
			}
		}
		/*SHOT*/
		else if (OUT1_Mode.DelayMode == SHOT)
		{
			if (OUT1 == 0 && SHOTflag ==0)
			{
				OUT1_Mode.DelayCounter = 0;
				GPIO_WriteBit(OUT1_GPIO_Port, OUT1_Pin, Bit_RESET);
			}
			else if(OUT1 ==1 || SHOTflag == 1)
			{
				if (OUT1_Mode.DelayCounter < (OUT1_Mode.DelayValue * DealyBaseTime))
				{
					GPIO_WriteBit(OUT1_GPIO_Port, OUT1_Pin, Bit_SET);
					SHOTflag = 1;
					
				}
				else
				{
					GPIO_WriteBit(OUT1_GPIO_Port, OUT1_Pin, Bit_RESET);
					SHOTflag = 0;
					CPV_Status = 1;
				}
			}
		}
		
		/*********************CPV**************************/
			if(LastOUT1 == 0 && OUT1 == 1 && CPV_Status == 1)
			{
				CPV++;
				CPV_Status = 0;
				if (CPV >= CSV) /*如果计数器达到预先设定的CSV，清零，OUT2输出一个高电平*/
				{
					OUT2 = 1;
					CPV = 0;
				}
			}		
			LastOUT1 = OUT1;
	}
}
/*******************************
*
*判断OUT2的输出状态
*
*******************************/
void SetOUT2Status(void)
{
	if (ShortCircuit != 1) /*不是短路保护的情况下才判断OUT2的输出*/
	{
		if (OUT2)
		{
			//GPIO_WriteBit(OUT2_GPIO_Port, OUT2_Pin, Bit_SET); /*拉高*/
		}
		if (OUT2_TimerCounter >= 80)
		{
			OUT2 = 0;
			OUT2_TimerCounter = 0;								/*获取当前时间*/
			//GPIO_WriteBit(OUT2_GPIO_Port, OUT2_Pin, Bit_RESET); /*80ms后拉低*/
		}
	}
}

/*******************************
*
*判断OUT3的输出状态
*
*******************************/
void SetOUT3Status(void)
{
	if (ShortCircuit != 1) /*不是短路保护的情况下才判断OUT2的输出*/
	{
		if (OUT3)
		{
			//GPIO_WriteBit(OUT3_GPIO_Port,OUT3_Pin,Bit_SET);/*拉高*/
		}
		if (OUT3_TimerCounter >= 160)
		{
			OUT3 = 0;
			OUT3_TimerCounter = 0; /*获取当前时间*/
								   //GPIO_WriteBit(OUT3_GPIO_Port,OUT3_Pin,Bit_RESET);/*80ms后拉低*/
		}
	}
}

/*******************************
*
*短路保护
*
*******************************/
void ShortCircuitProtection(void)
{
//	uint8_t SCState;

//	/*读取SC引脚的状态*/
//	if (ShortCircuit != 1)
//	{
//		//SCState = GPIO_ReadInputDataBit(SC_GPIO_Port ,SC_Pin);
//		if ((BitAction)SCState == Bit_RESET)
//		{
//			/*拉低FB_SC*/
//			ShortCircuit = 1;
//		}
//		else
//		{
//			ShortCircuit = 0;
//			ConfirmShortCircuit = 0;
//		}
//	}
//	if (ShortCircuit && ShortCircuitCounter >= 100)
//	{
//		ConfirmShortCircuit = 1;
//		GPIO_WriteBit(OUT1_GPIO_Port, OUT1_Pin, Bit_RESET);
//		//GPIO_WriteBit(OUT2_GPIO_Port, OUT2_Pin, Bit_RESET); /*马上拉低OUT*/
//		//GPIO_WriteBit(OUT3_GPIO_Port,OUT3_Pin,Bit_RESET);/*马上拉低OUT*/
//		ShortCircuitTimer = ShortCircuitLastTime;
//	}
}

/*******************************
*
*按键复用
*
*******************************/
void ButtonMapping(void)
{
	/*按键上锁*/
	if (ModeButton.Effect == PressLong && UpButton.Status == Release && DownButton.Effect == PressLong && DownButton.Status == Press)
	{
		if (KEY == ULOC)
			KEY = LOC;
		else
			KEY = ULOC;
		ModeButton.PressCounter = 0;
		while (ModeButton.Effect == PressLong && UpButton.Status == Release && DownButton.Effect == PressLong && DownButton.Status == Press)
		{
			ButtonMappingDisplay(1);
			KeyMappingFlag = 1;
		}
		WriteFlash(KEY_FLASH_DATA_ADDRESS, KEY);
		ModeButton.Effect = PressNOEffect;
		ModeButton.PressCounter = 0;
		DownButton.PressCounter = 0;
		DownButton.Effect = PressNOEffect;
	}
	/*软件初始化*/
	else if (ModeButton.Effect == PressLong && SetButton.Effect == PressLong && SetButton.Status == Press && UpButton.Status == Release && DownButton.Status == Release)
	{
		takeoffLED();
		while ((ReadButtonStatus(&ModeButton)) == Press && (ReadButtonStatus(&SetButton) == Press))
		{
			EraseFlash();
			while ((ReadButtonStatus(&ModeButton)) == Press && (ReadButtonStatus(&SetButton) == Press))
				ButtonMappingDisplay(2);
		}

		ResetParameter();

		ModeButton.PressCounter = 0;
		ModeButton.Effect = PressNOEffect;
		DownButton.PressCounter = 0;
		DownButton.Effect = PressNOEffect;
		UpButton.PressCounter = 0;
		UpButton.Effect = PressNOEffect;
		Test_Delay(1000);
	}
	/*计算器清零*/
	else if (ModeButton.Effect == PressLong && UpButton.Status == Press && UpButton.Effect == PressLong && DownButton.Status == Release)
	{
		CPV = 0;
		while (ModeButton.Effect == PressLong && UpButton.Status == Press && UpButton.Effect == PressLong && DownButton.Status == Release)
		{
			ButtonMappingDisplay(3);
		}
		ModeButton.Effect = PressNOEffect;
		ModeButton.PressTimer = 0;
		ModeButton.PressCounter = 0;
		UpButton.PressCounter = 0;
		UpButton.Effect = PressNOEffect;
		Test_Delay(1000);
	}
}

void Test_Delay(uint32_t ms)
{
	uint32_t i;

	/*　
		CPU主频168MHz时，在内部Flash运行, MDK工程不优化。用台式示波器观测波形。
		循环次数为5时，SCL频率 = 1.78MHz (读耗时: 92ms, 读写正常，但是用示波器探头碰上就读写失败。时序接近临界)
		循环次数为10时，SCL频率 = 1.1MHz (读耗时: 138ms, 读速度: 118724B/s)
		循环次数为30时，SCL频率 = 440KHz， SCL高电平时间1.0us，SCL低电平时间1.2us

		上拉电阻选择2.2K欧时，SCL上升沿时间约0.5us，如果选4.7K欧，则上升沿约1us

		实际应用选择400KHz左右的速率即可
	*/
	for (i = 0; i < ms * 100; i++)
		;
}
/*******************************
*
*获取EEPROM参数
*
*******************************/

//	uint32_t _test_send_buf[30];
//	uint32_t _test_recv_buf[30];
//	short _test_size = 30;

//	char statusW=2;
//	char statusR=2;

uint32_t ProgramRUNcounter = 0;

void GetEEPROM(void)
{

	//			//OUT1_Mode.DelayMode 	= *(__IO uint32_t*)(OUT1_Mode_FLASH_DATA_ADDRESS);
	//			OUT1_Mode.DelayValue 	= *(__IO uint32_t*)(OUT1_Value_FLASH_DATA_ADDRESS);
	//			CSV 									= *(__IO uint32_t*)(CSV_FLASH_DATA_ADDRESS);
	//			Threshold 						= *(__IO uint32_t*)(Threshold_FLASH_DATA_ADDRESS);
	//			DACOUT 								= *(__IO uint32_t*)(DACOUT_FLASH_DATA_ADDRESS);
	//			KEY 									= *(__IO uint32_t*)(KEY_FLASH_DATA_ADDRESS);
	//			RegisterB 						= *(__IO uint32_t*)(RegisterB_FLASH_DATA_ADDRESS);
	//			DEL 									= *(__IO uint32_t*)(DEL_FLASH_DATA_ADDRESS);
	//			HI 										= *(__IO uint32_t*)(HI_FLASH_DATA_ADDRESS);
	//			LO 										= *(__IO uint32_t*)(LO_FLASH_DATA_ADDRESS);
	//			displayModeONE_FLAG 	= *(__IO uint32_t*)(DETECT_FLASH_DATA_ADDRESS);

	OUT1_Mode.DelayMode = ReadFlash(OUT1_Mode_FLASH_DATA_ADDRESS);
	OUT1_Mode.DelayValue = ReadFlash(OUT1_Value_FLASH_DATA_ADDRESS);
	CSV = ReadFlash(CSV_FLASH_DATA_ADDRESS);
	Threshold = ReadFlash(Threshold_FLASH_DATA_ADDRESS);
	DACOUT1 = ReadFlash(DACOUT1_FLASH_DATA_ADDRESS);
	KEY = ReadFlash(KEY_FLASH_DATA_ADDRESS);
	RegisterB = ReadFlash(RegisterB_FLASH_DATA_ADDRESS);
	DEL = ReadFlash(DEL_FLASH_DATA_ADDRESS);
	SET_VREF = ReadFlash(SET_VREF_FLASH_DATA_ADDRESS);
	//SB_MaxValue = ReadFlash(SB_MAX_FLASH_DATA_ADDRESS);
	displayModeONE_FLAG = ReadFlash(DETECT_FLASH_DATA_ADDRESS);
	HI = ReadFlash(HI_FLASH_DATA_ADDRESS);
	LO = ReadFlash(LO_FLASH_DATA_ADDRESS);
	DSC = ReadFlash(DSC_FLASH_DATA_ADDRESS);

//	DAC_SetChannel1Data(DAC_Align_12b_R, (uint16_t)DACOUT1);
//	DAC_SoftwareTriggerCmd(DAC_Channel_1, ENABLE);
//	DAC_SetChannel2Data(DAC_Align_12b_R, (uint16_t)DACOUT2);
//	DAC_SoftwareTriggerCmd(DAC_Channel_2, ENABLE);
}

/*****************************
*
*初始化所有参数
*
****************************/
void ResetParameter(void)
{
	CSV = 10;
	Threshold = 1000;
	KEY = ULOC;
	OUT1_Mode.DelayMode = TOFF;
	OUT1_Mode.DelayValue = 10;
	ATT100 = 100;
	DEL = 4;
	RegisterB = 1;
	HI = 1000;
	LO = 500;
	displayModeONE_FLAG = 0;
	PERCENTAGE = 1;
	DSC = 1;
	
	TX = SET_VREF;
	TX_Sum=0;
	TX_Index = 0;
	FX = 0;
	
	WriteFlash(OUT1_Mode_FLASH_DATA_ADDRESS, OUT1_Mode.DelayMode);
	Test_Delay(50);
	WriteFlash(OUT1_Value_FLASH_DATA_ADDRESS, OUT1_Mode.DelayValue);
	Test_Delay(50);
	WriteFlash(CSV_FLASH_DATA_ADDRESS, CSV);
	Test_Delay(50);
	WriteFlash(Threshold_FLASH_DATA_ADDRESS, Threshold);
	Test_Delay(50);
	WriteFlash(DACOUT1_FLASH_DATA_ADDRESS, DACOUT1);
	Test_Delay(50);
	WriteFlash(KEY_FLASH_DATA_ADDRESS, KEY);
	Test_Delay(50);
	WriteFlash(RegisterB_FLASH_DATA_ADDRESS, RegisterB);
	Test_Delay(50);
	WriteFlash(DEL_FLASH_DATA_ADDRESS, DEL);
	Test_Delay(50);
	WriteFlash(SET_VREF_FLASH_DATA_ADDRESS, SET_VREF);
	Test_Delay(50);
//	WriteFlash(SB_MAX_FLASH_DATA_ADDRESS, SB_MaxValue);
	Test_Delay(50);
	WriteFlash(DETECT_FLASH_DATA_ADDRESS, displayModeONE_FLAG);
	Test_Delay(50);
	WriteFlash(HI_FLASH_DATA_ADDRESS, HI);
	Test_Delay(50);
	WriteFlash(LO_FLASH_DATA_ADDRESS, LO);
	Test_Delay(50);
	WriteFlash(DSC_FLASH_DATA_ADDRESS, DSC);

	ModeButton.Effect = PressNOEffect;
	ModeButton.PressTimer = 0;
	ModeButton.PressCounter = 0;
	SetButton.Effect = PressNOEffect;
	SetButton.PressCounter = 0;
}
