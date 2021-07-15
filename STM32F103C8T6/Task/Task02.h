#ifndef __TASK02_H
#define __TASK02_H

#include "main.h"

typedef struct
{
	uint8_t buf[256];
	uint8_t cnt;
}ComBufTypedef;

//ͨ�Ż�����
extern ComBufTypedef ComTx;
extern ComBufTypedef ComRx;

//������״̬����
void setBeepState(uint8_t st);


//�ж����ݽ���
void RecToBuf(void);

void Task02Main(void);

#endif /*__TASK02_H*/
