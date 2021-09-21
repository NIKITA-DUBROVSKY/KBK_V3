#ifndef __I2C
#define __I2C

#include <stdio.h>
#include "stm32f0xx.h"

#define DS3231_LONG_TIMEOUT         ((uint16_t)0x1000)

#define DS3231_I2C                       		I2C1
#define DS3231_I2C_CLK                      RCC_APB1Periph_I2C1
   
#define DS3231_I2C_SCL_PIN                  GPIO_Pin_6                  /* PB.06 */
#define DS3231_I2C_SCL_GPIO_PORT            GPIOB                       /* GPIOB */
#define DS3231_I2C_SCL_GPIO_CLK             RCC_AHBPeriph_GPIOB
#define DS3231_I2C_SCL_SOURCE               GPIO_PinSource6
#define DS3231_I2C_SCL_AF                   GPIO_AF_1

#define DS3231_I2C_SDA_PIN                  GPIO_Pin_7                  /* PB.07 */
#define DS3231_I2C_SDA_GPIO_PORT            GPIOB                       /* GPIOB */
#define DS3231_I2C_SDA_GPIO_CLK             RCC_AHBPeriph_GPIOB
#define DS3231_I2C_SDA_SOURCE               GPIO_PinSource7
#define DS3231_I2C_SDA_AF                   GPIO_AF_1

#define DS3231_I2C_TIMING          0x0090194B
//I2C speed 400kHz, I2C clock F 48000kHz
//Analog filter delay ON
//Rise Time 100 ns
//Fall Time 100 ns
//рассчитывается из I2C_Timing_Configuration_V1.0.1.xls

#define DS3231_ADRESS  				0xD0 //адрес смещен влево на 1 бит

#define LENGHT_BUF 19

typedef struct
{
	uint8_t seconds;              
	uint8_t minutes;              
	uint8_t hours;                
	uint8_t day;                  
	uint8_t date;                  
	uint8_t mon_cen;              
	uint8_t year;                 
	uint8_t alrm1_sec;            
	uint8_t alrm1_min;            
	uint8_t alrm1_hr;             
	uint8_t alrm1_doa;            
	uint8_t alrm2_min;            
	uint8_t alrm2_hr;             
	uint8_t alrm2_doa;            
	uint8_t ctrl;                 
	uint8_t stat;                 
	uint8_t aging;                
	uint8_t temp_msb;             
	uint8_t temp_lsb;             
} DS3231_registers_TypeDef;

void I2C_init(void);
void ds3231_read(uint8_t *Buf, uint8_t size);
void ds3231_write(uint8_t RegName, uint8_t RegValue);
float ds3231_read_temperature(void);
void ds3231_read_ALL(DS3231_registers_TypeDef* DS3231_reg);

#endif //__I2C
