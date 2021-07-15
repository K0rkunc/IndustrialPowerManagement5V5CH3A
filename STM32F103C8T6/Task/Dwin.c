/**
  ******************************************************************************
  * File Name          : Dwin.c
  * Description        : Code for dwin display driver
  ******************************************************************************
  * @author  kmakise
  * @version V1.0.0
  * @date    2021-3-3
  * @brief   Dwin��ʾ����
	******************************************************************************
  * @attention
  * <h2><center>&copy; Copyright (c) kmakise
  * All rights reserved.
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "Dwin.h"
#include "string.h"
#include "cmsis_os.h"
#include "Task02.h"
#include "usart.h"

const uint8_t DispRst[] = {0x82,0x00,0x04,0x55,0xAA,0x5A,0xA5,};

void Disp_SendArr(uint8_t *p,uint8_t n)//��������(�Զ����֡ͷ)
{
	uint8_t str[3] = {0x5A,0xA5,n};
	memcpy(ComTx.buf , str, 3);
	memcpy(&ComTx.buf[3],p,n);
	ComTx.cnt = n + 3;
	//TxData(COMDW);
	HAL_UART_Transmit(&huart1,ComTx.buf,ComTx.cnt,10);
}

void dwinFloatToCh(void *p,float f)
{
	char *pch = (char *)p;
	char *pf = (char *)&f;
	pch[0] = pf[3];
	pch[1] = pf[2];
	pch[2] = pf[1];
	pch[3] = pf[0];
}

float dwinChToFloat(void *p)
{
	float f;
	char *pch = (char *)p;
	char *pf = (char *)&f;
	pf[3] = pch[0];
	pf[2] = pch[1];
	pf[1] = pch[2];
	pf[0] = pch[3];
	return f;
}

void dwinInt32ToCh(void *p,int32_t l)
{
	char *pch = (char *)p;
	char *pl = (char *)&l;
	pch[0] = pl[3];
	pch[1] = pl[2];
	pch[2] = pl[1];
	pch[3] = pl[0];
}
uint32_t dwinChToInt32(void *p)
{
	int32_t f;
	char *pch = (char *)p;
	char *pf = (char *)&f;
	pf[3] = pch[0];
	pf[2] = pch[1];
	pf[1] = pch[2];
	pf[0] = pch[3];
	return f;
}
//��λ��ʾ��
void Disp_Rst(void) 
{	
	//��������(�Զ����֡ͷ
	Disp_SendArr((uint8_t*)DispRst,sizeof(DispRst));
}

//����Ҫ��ʾ��ͼƬ
void Disp_SetPic(uint8_t pic)
{
	uint8_t arr[20],cnt;
	cnt = 0;
	arr[cnt++] = 0x82; 	
	arr[cnt++] = 0x00; 	
	arr[cnt++] = 0x84; 	
	arr[cnt++] = 0x5a; 	
	arr[cnt++] = 0x01; 	
	arr[cnt++] = 0x00; 	
	arr[cnt++] = pic; 	
	Disp_SendArr(arr,cnt);
}
 //��ȡ��ǰ��ʾ��ͼƬ
int16_t Disp_ReadPic(void)
{
	int16_t pic = -1;
	uint8_t arr[20],cnt;
	ComRx.cnt = 0;
	
	cnt = 0;
	arr[cnt++] = 0x83; 	
	arr[cnt++] = 0x00; 	
	arr[cnt++] = 0x14; 
	arr[cnt++] = 0x01; 
	Disp_SendArr(arr,cnt);
  osDelay(5);
	if(	(ComRx.buf[0] == 0x5A)&&
			(ComRx.buf[1] == 0xA5)&&
			(ComRx.buf[2] == 0x06))
	{
		pic = ComRx.buf[8];
	}
	return pic;
}
/*
//дRTC
void Disp_WrRtc(void)
{
	uint8_t arr[20],cnt;
	cnt = 0;
	arr[cnt++] = 0x82; 	
	arr[cnt++] = 0x00; 	
	arr[cnt++] = 0x10; 	
	arr[cnt++] = G_Date.Year;//��
	arr[cnt++] = G_Date.Month;//�� 	
	arr[cnt++] = G_Date.Date;//�� 		
	arr[cnt++] = 0;//����
	arr[cnt++] = G_Time.Hours;//ʱ	
	arr[cnt++] = G_Time.Minutes;//�� 		
	arr[cnt++] = G_Time.Seconds;//��
	arr[cnt++] = 0;//Ԥ��0 
	Disp_SendArr(arr,cnt);
}*/
//д16λ����
void Disp_WrInt16(uint16_t addr,uint16_t data)
{
	uint8_t arr[20],cnt;
	cnt = 0;
	arr[cnt++] = 0x82; 	
	arr[cnt++] = addr>>8; 	
	arr[cnt++] = addr&0xff; 	
	arr[cnt++] = data>>8; 	
	arr[cnt++] = data&0xff; 	
	Disp_SendArr(arr,cnt);
}
//д������
void Disp_WrFloat(uint16_t addr,float fdata)
{
	uint8_t arr[20],cnt;
	cnt = 0;
	arr[cnt++] = 0x82; 	
	arr[cnt++] = addr>>8; 	
	arr[cnt++] = addr&0xff; 	
	dwinFloatToCh(&arr[cnt],fdata); 
	cnt+=4;
	Disp_SendArr(arr,cnt);
}
//д32λ����
void Disp_WrInt32(uint16_t addr,int32_t ldata)
{
	uint8_t arr[20],cnt;
	cnt = 0;
	arr[cnt++] = 0x82; 	
	arr[cnt++] = addr>>8; 	
	arr[cnt++] = addr&0xff; 	
	dwinInt32ToCh(&arr[cnt],ldata); 
	cnt+=4;
	Disp_SendArr(arr,cnt);
}
//д�ַ���
void Disp_WrStr(uint16_t addr,char *p)
{
	uint8_t arr[100],cnt;
	cnt = 0;
	arr[cnt++] = 0x82; 	
	arr[cnt++] = addr>>8; 	
	arr[cnt++] = addr&0xff; 	
	while(*p)
	{
		arr[cnt++] = *p; 	
		p++;
	}	
	Disp_SendArr(arr,cnt);
}

//ɫ�����
void Disp_WrBlock(uint16_t addr,uint16_t x0,uint16_t y0,uint16_t w,uint16_t h,uint16_t color)
{
	uint8_t arr[30],cnt;
	cnt = 0;
	arr[cnt++] = 0x82; 	
	arr[cnt++] = addr>>8; 	
	arr[cnt++] = addr&0xff;
	arr[cnt++] = 0x00;
	arr[cnt++] = 0x04;//mod
	arr[cnt++] = 0x00;
	arr[cnt++] = 0x01;//num
	arr[cnt++] = x0>>8;
	arr[cnt++] = x0&0xff;//x0
	arr[cnt++] = y0>>8;
	arr[cnt++] = y0&0xff;//y0
	arr[cnt++] = (x0+w)>>8;
	arr[cnt++] = (x0+w)&0xff;//x1
	arr[cnt++] = (y0+h)>>8;
	arr[cnt++] = (y0+h)&0xff;//y1
	arr[cnt++] = color>>8;
	arr[cnt++] = color&0xff;//y1
	arr[cnt++] = 0xff;
	arr[cnt++] = 0x00;
	Disp_SendArr(arr,cnt);
}

void Disp_BeepContrl(uint16_t ms)
{
	//5AA5 07 82 00A0 03 01 40 00
	uint8_t arr[20],cnt;
	cnt = 0;
	arr[cnt++] = 0x82; 	
	arr[cnt++] = 0x00; 	
	arr[cnt++] = 0xA0; 	
	arr[cnt++] = 0x01; 	
	arr[cnt++] = ms / 8;
 	arr[cnt++] = 0x40;
	arr[cnt++] = 0x00;
	Disp_SendArr(arr,cnt);
}





