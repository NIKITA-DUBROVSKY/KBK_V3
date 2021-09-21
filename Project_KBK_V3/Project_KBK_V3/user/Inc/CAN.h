#ifndef __CAN
#define __CAN

#include <stdint.h>

void Init_CAN(void);
void CAN_Send(void);
void CEC_CAN_IRQHandler(void);

#endif //__CAN
