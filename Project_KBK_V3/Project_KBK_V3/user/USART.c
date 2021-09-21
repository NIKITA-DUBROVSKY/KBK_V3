#include "stm32f0xx.h"
#include "USART.h"

extern uint16_t TxBuffer[1];
extern uint16_t RxBuffer[10];

void Init_USART_2(void){
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
//    NVIC_InitTypeDef NVIC_InitStructure;
//    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
//		NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
//		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//		NVIC_Init(&NVIC_InitStructure);	
		
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);
	

    GPIO_InitTypeDef GPIO_InitStructure;
		
	//TX UART
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		//RX UART
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure the USART2 */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
    USART_Init(USART2, &USART_InitStructure);
	USART_OverrunDetectionConfig(USART2, USART_OVRDetection_Disable);	
		  /*------------------------------- DMA---------------------------------------*/   
  /* Common DMA configuration */
	DMA_InitTypeDef  DMA_InitStructure;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  
  /* DMA2 Channel1 configuration */
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)TxBuffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->TDR);
  DMA_Init(DMA1_Channel4, &DMA_InitStructure);
  
  /* DMA remap Channel1 to USART1 Tx */
  DMA_RemapConfig(DMA1, DMA1_CH4_USART2_TX);
  
  /* DMA2 Channel2 configuration */
	DMA_InitStructure.DMA_BufferSize = 5;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RxBuffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->RDR);
  DMA_Init(DMA1_Channel5, &DMA_InitStructure);
  
  /* DMA remap Channel2 to USART2 Rx */
  DMA_RemapConfig(DMA1, DMA1_CH5_USART2_RX);
    
  /* Enable the USART2 Rx and Tx DMA2 requests */
  USART_DMACmd(USART2, USART_DMAReq_Rx|USART_DMAReq_Tx, ENABLE);
  
  /* Enable the DMA2 channels */
  DMA_Cmd(DMA1_Channel4, ENABLE);
  DMA_Cmd(DMA1_Channel5, ENABLE); 
  
  /* Enable Usart */
  USART_Cmd(USART2, ENABLE);
}
