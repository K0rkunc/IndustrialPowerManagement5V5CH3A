/**
  ******************************************************************************
  * File Name          : Task04.c
  * Description        : Code for freeRTOS Task05 Application
  ******************************************************************************
  * @author  kmakise
  * @version V1.0.0
  * @date    2021-2-4
  * @brief   ���ò������ݴ洢����
	******************************************************************************
  * @attention
  * <h2><center>&copy; Copyright (c) kmakise
  * All rights reserved.
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "Task04.h"
#include "crc.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
/*Golbal data space ----------------------------------------------------------*/

#define FLASH_START_ADDR 		0x0800F000
#define FLASH_BUFFER_SIZE 	100
uint8_t datatfbuf[FLASH_BUFFER_SIZE];

CfgDataTypedef CfgData;

void ReadBufFromFlash(void)
{
	uint32_t Robot_Num_Flash_Add = FLASH_START_ADDR; 
	
	for(int i = 0;i < FLASH_BUFFER_SIZE/2;i++)
	{
		uint16_t readdata = *(__IO uint16_t*)( Robot_Num_Flash_Add );
		datatfbuf[i * 2 + 1	] = readdata >> 8;
		datatfbuf[i * 2 		] = readdata & 0xFF;
		Robot_Num_Flash_Add+=2;
	}
}

void WriteBufToFlash(void)
{
	uint32_t Robot_Num_Flash_Add = FLASH_START_ADDR; 

	FLASH_EraseInitTypeDef My_Flash;  //���� FLASH_EraseInitTypeDef �ṹ��Ϊ My_Flash
	HAL_FLASH_Unlock();               //����Flash
			
	My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;  //����Flashִ��ҳ��ֻ����������
	My_Flash.PageAddress = Robot_Num_Flash_Add;  //����Ҫ�����ĵ�ַ
	My_Flash.NbPages = 1;                        //˵��Ҫ������ҳ�����˲���������Min_Data = 1��Max_Data =(���ҳ��-��ʼҳ��ֵ)֮���ֵ
			
	uint32_t PageError = 0;                    //����PageError,������ִ�����������ᱻ����Ϊ�����FLASH��ַ
	HAL_FLASHEx_Erase(&My_Flash, &PageError);  //���ò�����������
	
	for(int i = 0;i < FLASH_BUFFER_SIZE/2;i++)
	{
		uint16_t Write_Flash_Data = datatfbuf[i * 2 + 1] << 8 | datatfbuf[i * 2];
		 //��Flash������д��FLASH_TYPEPROGRAM_HALFWORD ����������Flash��ַ��16λ
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,Robot_Num_Flash_Add, Write_Flash_Data);
		Robot_Num_Flash_Add+=2;
	}
	HAL_FLASH_Lock(); //��סFlash
}


//����Ĭ�ϲ������ò�д��flash
void DefultParam(void)
{
	CfgData.octh[0] = 3000;
	CfgData.octh[1] = 3000;
	CfgData.octh[2] = 3000;
	CfgData.octh[3] = 3000;
	CfgData.octh[4] = 3000;
	CfgData.dt = 3000;
	CfgData.update = 0;
}
void INT16ToCh(void *p,int16_t Int)
{
	char *pch = p;
	pch[0] = Int>>8;
	pch[1] = Int&0xff;
}

//�������ò����浽flash
void ConfigUpdate(void)
{
	if(CfgData.update != 0)
	{
		//copy to buf
		uint8_t len = sizeof(CfgData);
		memcpy(datatfbuf,&CfgData,sizeof(CfgData));
		uint16_t SendCrc = CalCRC16(datatfbuf,len);
		INT16ToCh(&datatfbuf[len],SendCrc);
		
		//save to flash
		WriteBufToFlash();
		CfgData.update = 0;
	}
}

//��flash�������ò�У��
void LoadParam(void)
{
	ReadBufFromFlash();
	//У�����
	if((CalCRC16(datatfbuf,sizeof(CfgData)+2)==0))
	{
		memcpy(&CfgData,datatfbuf,sizeof(CfgData));
	}
	else //ʧ��
	{
		DefultParam();
		CfgData.update = 255;
		ConfigUpdate();
	}
}

void Task04Main(void)
{
	LoadParam();
	for(;;)
	{
		ConfigUpdate();
		osDelay(100);
	}
}



