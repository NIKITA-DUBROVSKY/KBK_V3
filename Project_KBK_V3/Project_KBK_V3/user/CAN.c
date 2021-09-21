#include "CAN.h"
#include "stm32f0xx.h"
#include "timers.h"

#define CAN_IDE_32            0b00000100            // Для 32-х битного масштаба
#define joystick_CAN_ID				0x18FDD603  //0x18FE2103 id джойстика для навесного оборудования
#define serial_num_CAN_ID			0x18021003

extern int16_t	angle_alpha_CAN,
								angle_gamma_CAN, 
								angle_betta_CAN;

extern uint8_t 				CRC_DUG50_CAN,
											CRC_DUG51_CAN,
											CRC_LEP_CAN,
											CRC_USILIE_CAN,
											CRC_JOY_CAN;

extern uint8_t LEP_CAN;
extern uint16_t USILIE_CAN;

extern uint8_t angle_alpha_CAN_USB,
							angle_betta_CAN_USB,
							USILIE_CAN_USB_L,
							USILIE_CAN_USB_H;

extern uint8_t KONCEVIK_LEP, KONCEVIK_DUG_50, KONCEVIK_DUG_51;

extern uint8_t CAN_MESSAGE, CAN_MESSAGE_1;
extern uint8_t ATT_LEFT, ATT_RIGHT, ATT_FWD, ATT_REV;

extern uint8_t status_CAN_Transsiver;

extern uint32_t	serial;

void Init_CAN(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;

	/*CAN Интерфейс на 4 альтернативной функции*/
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_4);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_4);

	/* Конфигурация CAN RX и TX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* CAN rikystir init */
	CAN_DeInit(CAN);
	CAN_StructInit(&CAN_InitStructure);
	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = ENABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;

	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1  = CAN_BS1_13tq; 
	CAN_InitStructure.CAN_BS2  = CAN_BS2_2tq; 
	CAN_InitStructure.CAN_Prescaler = 6; //12 - 48 Мгц, 6 - 24 Мгц
	if (CAN_Init(CAN, &CAN_InitStructure) == CAN_InitStatus_Success){
		status_CAN_Transsiver = SUCCESS;
	}
	else status_CAN_Transsiver = ERROR;	

	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdList;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 			(uint16_t)(0x18050303>>13); 						// Старшая часть идентификатора №1
	CAN_FilterInitStructure.CAN_FilterIdLow = 			(uint16_t)((uint16_t)0x18050303<<3) | CAN_IDE_32;	// Младшая часть идентификатора №1
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh =	(uint16_t)(0x18050403>>13);            	// Старшая часть идентификатора №2
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 	(uint16_t)((uint16_t)0x18050403<<3) | CAN_IDE_32;	// Младшая часть идентификатора №2
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);

	CAN_FilterInitStructure.CAN_FilterNumber = 1;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 			(uint16_t)(0x18050503>>13); 						// Старшая часть идентификатора №3
	CAN_FilterInitStructure.CAN_FilterIdLow = 			(uint16_t)((uint16_t)0x18050503<<3) | CAN_IDE_32;	// Младшая часть идентификатора №3
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh =	(uint16_t)(joystick_CAN_ID>>13);            	// Старшая часть идентификатора №4
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 	(uint16_t)((uint16_t)joystick_CAN_ID<<3) | CAN_IDE_32;	// Младшая часть идентификатора №4
	CAN_FilterInit(&CAN_FilterInitStructure);

	CAN_FilterInitStructure.CAN_FilterNumber = 2;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 			(uint16_t)(serial_num_CAN_ID>>13); 						// Старшая часть идентификатора №5
	CAN_FilterInitStructure.CAN_FilterIdLow = 			(uint16_t)(serial_num_CAN_ID<<3) | CAN_IDE_32;	// Младшая часть идентификатора №5
	CAN_FilterInit(&CAN_FilterInitStructure);

	CAN_ITConfig(CAN, CAN_IT_FMP0, ENABLE); // Прерывание получения пакета в буфер FIFO 0

	NVIC_InitStructure.NVIC_IRQChannel = CEC_CAN_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void CAN_Send()
{
		CanTxMsg TxMessage;
		TxMessage.ExtId 	= 0x18050203;
		TxMessage.RTR 		= CAN_RTR_DATA;
		TxMessage.IDE 		= CAN_ID_EXT;				//CAN_ID_STD;
		TxMessage.DLC 		= 8;
	
		TxMessage.Data[0] = CAN_MESSAGE;
		TxMessage.Data[1] = CAN_MESSAGE_1;
		TxMessage.Data[2] = 0;
		TxMessage.Data[3] = 0;
		TxMessage.Data[4] = 0;
		TxMessage.Data[5] = 0;
		TxMessage.Data[6] = 0;
		TxMessage.Data[7] = 0;
		
		CAN_Transmit(CAN,&TxMessage);
}		

void CEC_CAN_IRQHandler(void){
	CanRxMsg RxMessage;

	// Обработаем прерывания приемного буфера FIFO 0
	if (CAN_GetITStatus(CAN, CAN_IT_FMP0) == SET) {                   // Прерывание получения пакета в буфер FIFO 0
	// Обнулим данные пакета
	RxMessage.DLC = 	0x00;
	RxMessage.ExtId = 	0x00;
	RxMessage.FMI = 	0x00;
	RxMessage.IDE = 	0x00;
	RxMessage.RTR = 	0x00;
	RxMessage.StdId = 	0x00;
	RxMessage.Data [0] = 0x00;
	RxMessage.Data [1] = 0x00;
	RxMessage.Data [2] = 0x00;
	RxMessage.Data [3] = 0x00;
	RxMessage.Data [4] = 0x00;
	RxMessage.Data [5] = 0x00;
	RxMessage.Data [6] = 0x00;
	RxMessage.Data [7] = 0x00;
	CAN_Receive(CAN, CAN_FIFO0, &RxMessage);
		
		if (RxMessage.ExtId == 0x18050303) {  //датчик на стреле
			CRC_DUG50_CAN = 1;
			ResetGTimer(TIMER_CAN_timeout_DUG50);
			
			angle_gamma_CAN = RxMessage.Data [0];
			KONCEVIK_DUG_50 = RxMessage.Data [2];
		}
		
		if (RxMessage.ExtId == 0x18050403) {	//датчик на раме
			CRC_DUG51_CAN = 1;
			ResetGTimer(TIMER_CAN_timeout_DUG51);
			
			angle_alpha_CAN_USB = RxMessage.Data [0]; 
			angle_betta_CAN_USB = RxMessage.Data [1];
			angle_alpha_CAN = (RxMessage.Data [0]-55) * (-1); //выставить 0, когда он лежит горизонтально 
			angle_betta_CAN = (RxMessage.Data [1]-55) * (-1); //выставить 0, когда он лежит горизонтально
			KONCEVIK_DUG_51 = RxMessage.Data [2];
		}
			
		if (RxMessage.ExtId == 0x18050503) {	//датчик усилия и ЛЭП
			CRC_LEP_CAN = 1;
			ResetGTimer(TIMER_CAN_timeout_LEP);
			
			if (RxMessage.Data [0] != 0xFF && RxMessage.Data [1] != 0xFF){
				CRC_USILIE_CAN = 1;
				USILIE_CAN_USB_L = RxMessage.Data [0];
				USILIE_CAN_USB_H = RxMessage.Data [1];	
				USILIE_CAN = RxMessage.Data [0] + (RxMessage.Data [1] * 256);
			}
			else {
				CRC_USILIE_CAN = 0;
				USILIE_CAN_USB_L = 0;
				USILIE_CAN_USB_H = 0;	
				USILIE_CAN = RxMessage.Data [0] + (RxMessage.Data [1] * 256);
			}
			LEP_CAN = RxMessage.Data [3];
			KONCEVIK_LEP = RxMessage.Data [2];			
		}

		if (RxMessage.ExtId == joystick_CAN_ID) {
			CRC_JOY_CAN = 1;
			ResetGTimer(TIMER_CAN_timeout_JOY);
			
			ATT_LEFT	 	= (RxMessage.Data [0] & 0x04) >> 2;
			ATT_RIGHT	 	= (RxMessage.Data [0] & 0x10) >> 4;
			ATT_FWD			= (RxMessage.Data [2] & 0x04) >> 2;
			ATT_REV	 		= (RxMessage.Data [2] & 0x10) >> 4;
		}
		
		if (RxMessage.ExtId == serial_num_CAN_ID) 
			serial = (RxMessage.Data[3] << 8) + RxMessage.Data[2];
	}
}
