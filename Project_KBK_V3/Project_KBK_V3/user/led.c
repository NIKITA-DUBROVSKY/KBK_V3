#include "stm32f0xx.h"
#include "led.h"
//PB15 - DS, PB12 - защелка, PB13 - сдвиг
extern uint16_t led_status[31];

#define cs_set()		GPIO_ResetBits(GPIOB, GPIO_Pin_12)
#define cs_reset() 	GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define cs_strob() 	cs_reset();cs_set()
#define	SPI_HC595 SPI2

void Init_leds(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); 
		
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_0);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_0);

  cs_set(); 

  SPI_InitTypeDef SPI_InitStructure;

  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  SPI_Init(SPI_HC595, &SPI_InitStructure);
  SPI_RxFIFOThresholdConfig(SPI_HC595, SPI_RxFIFOThreshold_HF);
	SPI_Cmd(SPI_HC595, ENABLE);
}

uint16_t data;
void Led_show(void){
	cs_strob();
	
	while ((SPI_HC595->SR & SPI_SR_BSY) == 1){}
	while ((SPI_HC595->SR & SPI_SR_TXE) == 1){}		
		
	data = 0 << 15 | led_status[30]<<14 | led_status[29]<<13 | led_status[28]<<12 | led_status[27]<<11 | led_status[26]<<10 | led_status[25]<<9 | led_status[24] << 8 |
					led_status[23]<<7 | led_status[22]<<6 | led_status[21]<<5 | led_status[20]<<4 | led_status[19]<<3 | led_status[18]<<2 | led_status[17]<<1 | led_status[16];
	SPI_I2S_SendData16(SPI_HC595, data);
	data = led_status[15] << 15 | led_status[14]<<14 | led_status[13]<<13 | led_status[12]<<12 | led_status[11]<<11 | led_status[10]<<10 | led_status[9]<<9 | led_status[8] << 8 |
					led_status[7]<<7 | led_status[6]<<6 | led_status[5]<<5 | led_status[4]<<4 | led_status[3]<<3 | led_status[2]<<2 | led_status[1]<<1 | led_status[0];
	SPI_I2S_SendData16(SPI_HC595, data);
}
