#ifndef AT45DB
#define AT45DB

#include <stdint.h>

#define GPIO_Pin_CS_AT45  GPIO_Pin_15
#define PORT_CS_AT45  		GPIOA

#define GPIO_Pin_CS_SD  GPIO_Pin_4
#define PORT_CS_SD 			GPIOA

#define GPIO_Pin_Card_detect  GPIO_Pin_13
#define PORT_Card_detect 			GPIOC

#define CS_AT45_LOW  GPIO_ResetBits(PORT_CS_AT45,	GPIO_Pin_CS_AT45)
#define CS_AT45_HIGH  GPIO_SetBits(PORT_CS_AT45,	GPIO_Pin_CS_AT45)

#define Start_page_NumPage 0
#define End_page_NumPage 	31

#define Start_page_data 	32
#define End_page_data 		8191

#define countof(a)		(uint8_t)(sizeof(a) / sizeof(*(a)))

#define Size_Buff_params_AT45DB countof(params_AT45DB)

void AT45DB161_Init(void);
uint8_t AT45DB161_Read_Status(void);
void AT45DB_search_data(void);
void AT45DB_ChipErase(void);
void AT45DB161_Read_Data(uint16_t page, uint16_t addr, uint32_t length, uint8_t *out);
void AT45DB_write_data(void);
void AT45DB161_PageProgram_without_BUF(uint16_t page, uint16_t addr, uint8_t *data, uint16_t length);

#endif 
