/**
  ******************************************************************************
  * File Name          : Task01.c
  * Description        : Code for freeRTOS Task01 Application
  ******************************************************************************
  * @author  kmakise
  * @version V1.0.0
  * @date    2021-6-2
  * @brief   Power Management Main Logic
	******************************************************************************
  * @attention
  * <h2><center>&copy; Copyright (c) kmakise
  * All rights reserved.
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"
#include "Task01.h"
#include "Task02.h"
#include "Task03.h"
#include "Task04.h"

/*Global Data Space ----------------------------------------------------------*/

uint16_t g_mA[5] = {0,0,0,0,0};
uint16_t g_chst[5] = {WARITING,WARITING,WARITING,WARITING,WARITING};
uint16_t g_state = 0; //0 �ȴ�״̬ 1����״̬ 2����״̬ 3��ͣ״̬

/*Driver Function ------------------------------------------------------------*/

uint8_t getRunPress(void)
{
	return (SW2_GPIO_Port->IDR & SW2_Pin) == 0;
}
uint8_t getStopPress(void)
{
	return (SW1_GPIO_Port->IDR & SW1_Pin) == 0;
}

//ch(1-6Chnnal),times(��ȡ����)
uint32_t ADC_Get_Average(uint8_t ch,uint8_t times)
{
	ADC_ChannelConfTypeDef sConfig;		//ͨ����ʼ��
	uint32_t value_sum=0;	
	uint8_t i;
	switch(ch)							//ѡ��ADCͨ��
	{
		case 1:sConfig.Channel = ADC_CHANNEL_1;break;	
		case 2:sConfig.Channel = ADC_CHANNEL_2;break;
		case 3:sConfig.Channel = ADC_CHANNEL_3;break;
		case 4:sConfig.Channel = ADC_CHANNEL_4;break;
		case 5:sConfig.Channel = ADC_CHANNEL_5;break;
		case 6:sConfig.Channel = ADC_CHANNEL_6;break;
	}
	sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	sConfig.Rank = 1;
	HAL_ADC_ConfigChannel(&hadc1,&sConfig);											
	for(i=0;i<times;i++)
	{
		HAL_ADC_Start(&hadc1);		
		HAL_ADC_PollForConversion(&hadc1,30);	
		value_sum += HAL_ADC_GetValue(&hadc1);			
		HAL_ADC_Stop(&hadc1);	
	}
	return value_sum/times;
}

//���㵱ǰ����
void getADCtoCurrent(void)
{
	g_mA[0] = ADC_Get_Average(2,1) * 1.0;
	g_mA[1] = ADC_Get_Average(3,1) * 1.506993006993006993006993006993;
	g_mA[2] = ADC_Get_Average(4,1) * 1.5297691373025516403402187120292;
	g_mA[3] = ADC_Get_Average(5,1) * 1.5244047619047619047619047619048;
	g_mA[4] = ADC_Get_Average(6,1) * 1.5451263537906137184115523465704;
}

//��Դ����
void setPowerEnable(uint8_t ch,uint8_t st)
{
	switch(ch)
	{
		case 1:PWEN_1_GPIO_Port->BSRR = PWEN_1_Pin << 16 * (st == 0);break;
		case 2:PWEN_2_GPIO_Port->BSRR = PWEN_2_Pin << 16 * (st == 0);break;
		case 3:PWEN_3_GPIO_Port->BSRR = PWEN_3_Pin << 16 * (st == 0);break;
		case 4:PWEN_4_GPIO_Port->BSRR = PWEN_4_Pin << 16 * (st == 0);break;
		case 5:PWEN_5_GPIO_Port->BSRR = PWEN_5_Pin << 16 * (st == 0);break;
	}
}

//�������
void setOutputEnable(uint8_t ch,uint8_t st)
{
	switch(ch)
	{
		case 1:INP_1_GPIO_Port->BSRR = INP_1_Pin << 16 * (st == 0);break;
		case 2:INP_2_GPIO_Port->BSRR = INP_2_Pin << 16 * (st == 0);break;
		case 3:INP_3_GPIO_Port->BSRR = INP_3_Pin << 16 * (st == 0);break;
		case 4:INP_4_GPIO_Port->BSRR = INP_4_Pin << 16 * (st == 0);break;
		case 5:INP_5_GPIO_Port->BSRR = INP_5_Pin << 16 * (st == 0);break;
	}
}

void PowerEN(uint8_t st)
{
	setPowerEnable(1,st);
	setPowerEnable(2,st);
	setPowerEnable(3,st);
	setPowerEnable(4,st);
	setPowerEnable(5,st);
}

void OutputEN(uint8_t st)
{
	setOutputEnable(1,st);
	setOutputEnable(2,st);
	setOutputEnable(3,st);
	setOutputEnable(4,st);
	setOutputEnable(5,st);
}
/*State function --------------------------------------------------------------*/

//�ȴ�״̬�İ����¼�
void ST0_Wait_keyEvent(void)
{
	//��ͣ
	if(!getStopPress())
	{
		if(getRunPress())
		{
			setBeepState(1);			//�������������
			while(getRunPress());	//�ȴ��ͷŰ���
			setTimeVal(CfgData.dt);	//���õ���ʱʱ��
			setTimeState(SET);		//��ʼ����ʱ
			g_state = 1;					//��ת״̬
			for(int i = 0;i < 5;i++) 
			{
				g_chst[i] = RUNNING;//����ͨ��״̬
			}
			OutputEN(ENABLE);			//�������
		}
	}
	else
	{
		if(getRunPress())
		{
			setBeepState(3);
		}
	}

}

//����״̬ �ȴ�ʱ����� �������� ��ͣ
void ST1_Run_JumpEvent(void)
{
	//ֹͣ
	if(getRunPress())
	{
		setBeepState(1);				//�������������
		while(getRunPress());		//�ȴ��ͷŰ���
		OutputEN(DISABLE);			//�ر����
		setTimeState(RESET);		//�رյ���ʱ
		for(int i = 0;i < 5;i++) 
		{
			g_chst[i] = WARITING;	//����ͨ��״̬
		}
		g_state = 3;						//��ת״̬
	}
	
	//�ȴ�ʱ�����
	if(getTimeVal() == 0)
	{
		setBeepState(1);				//�������������
		OutputEN(DISABLE);			//�ر����
		setTimeState(RESET);		//�رյ���ʱ
		setTimeVal(CfgData.dt);		//�������õ���ʱʱ��
		for(int i = 0;i < 5;i++) 
		{
			g_chst[i] = WARITING;	//����ͨ��״̬
		}
		g_state = 0;						//��ת״̬
	}
	
	//������������
	uint8_t oc = 0;
	for(int i = 0;i < 5;i++)
	{
		if(g_mA[i] > CfgData.octh[i])
		{
			oc = 1;
		}
	}
	if(oc)
	{
		setBeepState(3);				//�������������
		OutputEN(DISABLE);			//�ر����
		setTimeState(RESET);		//�رյ���ʱ
		for(int i = 0;i < 5;i++) 
		{
			g_chst[i] = WARITING;	//����ͨ��״̬
		}
		for(int i = 0;i < 5;i++)
		{
			if(g_mA[i] > CfgData.octh[i])
			{
				g_chst[i] = CAUTION;	//����ͨ��״̬
			}
		}
		g_state = 2;						//��ת״̬
	}
	
	//��ͣ
	if(getStopPress())
	{
		setBeepState(1);				//�������������
		OutputEN(DISABLE);			//�ر����
		setTimeState(RESET);		//�رյ���ʱ
		for(int i = 0;i < 5;i++) 
		{
			g_chst[i] = WARITING;	//����ͨ��״̬
		}
		while(getStopPress())		//�ȴ��ͷ�
		{
			if(getRunPress())
			{
				setBeepState(3);
			}
			osDelay(20);
		}
		g_state = 3;						//��ת״̬
	}	
}

//�ȴ��������ò���ʼ
void ST2_OC_JumpEvent(void)
{
	if(getRunPress())
	{
		setBeepState(2);				//�������������
		while(getRunPress());		//�ȴ��ͷŰ���
		OutputEN(DISABLE);			//�ر����
		setTimeState(RESET);		//�رյ���ʱ
		setTimeVal(CfgData.dt);		//�������õ���ʱʱ��
		for(int i = 0;i < 5;i++) 
		{
			g_chst[i] = WARITING;	//����ͨ��״̬
		}
		g_state = 0;						//��ת״̬
	}
}
//�ȴ��������ò���ʼ
void ST3_STOP_JumpEvent(void)
{
	if(getRunPress())
	{
		setBeepState(2);				//�������������
		while(getRunPress());		//�ȴ��ͷŰ���
		OutputEN(DISABLE);			//�ر����
		setTimeState(RESET);		//�رյ���ʱ
		setTimeVal(CfgData.dt);		//�������õ���ʱʱ��
		for(int i = 0;i < 5;i++) 
		{
			g_chst[i] = WARITING;	//����ͨ��״̬
		}
		g_state = 0;						//��ת״̬
	}
}

/*State Machine ---------------------------------------------------------------*/

//����״̬���״̬��
void runningStateMachine(void)
{
	switch(g_state)//0 �ȴ�״̬ 1����״̬ 2����״̬ 3��ͣ״̬
	{
		case 0://�ȴ�״̬ 
		{
			setTimeVal(CfgData.dt);	//���õ���ʱʱ��
			getADCtoCurrent();
			ST0_Wait_keyEvent();
			break;
		}
		case 1://����״̬ 
		{
			getADCtoCurrent();
			ST1_Run_JumpEvent();
			break;
		}
		case 2://����״̬
		{
			//�ȴ��������ò���ʼ
			ST2_OC_JumpEvent();
			break;
		}
		case 3://��ͣ״̬
		{
			//�ȴ��������ò���ʼ
			ST3_STOP_JumpEvent();
			break;
		}
	}
}

void Task01Main(void)
{
	osDelay(100);
	//outputTest();
	
	//�رո���ͨ�����
	OutputEN(DISABLE);
	
	//�򿪸���ͨ����Դ
	PowerEN(ENABLE);
	
	//��ʾ����ʱ
	setTimeState(RESET);	//�رյ���ʱ
	setTimeVal(CfgData.dt);	//���õ���ʱʱ��
	
	setTimeState(RESET);
	for(;;)
	{
		runningStateMachine();
		osDelay(10);
	}
}



