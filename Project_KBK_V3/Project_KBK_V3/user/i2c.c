#include "stm32f0xx.h"
#include "i2c.h"

uint8_t seconds, minutes, hours;
__IO uint32_t  Timeout = DS3231_LONG_TIMEOUT;

void I2C_init(void)
{
		GPIO_InitTypeDef  GPIO_InitStructure;
		I2C_InitTypeDef  I2C_InitStructure;
		
		RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);
		RCC_AHBPeriphClockCmd(DS3231_I2C_SCL_GPIO_CLK | DS3231_I2C_SDA_GPIO_CLK, ENABLE);
		RCC_APB1PeriphClockCmd(DS3231_I2C_CLK, ENABLE);

		GPIO_PinAFConfig(DS3231_I2C_SCL_GPIO_PORT, DS3231_I2C_SCL_SOURCE, DS3231_I2C_SCL_AF);
		GPIO_PinAFConfig(DS3231_I2C_SDA_GPIO_PORT, DS3231_I2C_SDA_SOURCE, DS3231_I2C_SDA_AF);

		GPIO_InitStructure.GPIO_Pin = DS3231_I2C_SCL_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(DS3231_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = DS3231_I2C_SDA_PIN;
		GPIO_Init(DS3231_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);

		I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
		I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
		I2C_InitStructure.I2C_DigitalFilter = 0x00;
		I2C_InitStructure.I2C_OwnAddress1 = 0x00;
		I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
		I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
		I2C_InitStructure.I2C_Timing = DS3231_I2C_TIMING;
		I2C_Init(DS3231_I2C, &I2C_InitStructure);

		I2C_Cmd(DS3231_I2C, ENABLE);		
}

void ds3231_read(uint8_t *Buf, uint8_t size)
{
	uint8_t i = 0;

  Timeout = DS3231_LONG_TIMEOUT;
	while((DS3231_I2C->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY)
	{if((Timeout--) == 0)  break;}		
  
  /* Configure slave address, nbytes, reload, end mode and start or stop generation */
  DS3231_I2C->CR2 = (1 << 16) | I2C_CR2_START | DS3231_ADRESS; //передаем адрес устройства
  
  Timeout = DS3231_LONG_TIMEOUT;
	while((DS3231_I2C->ISR & I2C_ISR_TXIS) != I2C_ISR_TXIS) //Wait until TXIS flag is set
  {if((Timeout--) == 0) break;}

  DS3231_I2C->TXDR = (uint8_t)0; //передаем адрес регистра

  Timeout = DS3231_LONG_TIMEOUT;
  while((DS3231_I2C->ISR & I2C_ISR_TC) != I2C_ISR_TC)	//Wait until TC flag is set	
  {if((Timeout--) == 0) break;}  
  
  /* Configure slave address, nbytes, reload, end mode and start or stop generation */
  DS3231_I2C->CR2 = I2C_CR2_AUTOEND | size << 16 | I2C_CR2_START | I2C_CR2_RD_WRN | DS3231_ADRESS; //передаем адрес устройства, но теперь для чтения
	for(i = 0;i < size;i++)
	{ 
		Timeout = DS3231_LONG_TIMEOUT;
		while((DS3231_I2C->ISR & I2C_ISR_RXNE) != I2C_ISR_RXNE)
		{if((Timeout--) == 0) break;}
			/* Read data from RXDR */
		Buf[i] = (uint8_t)DS3231_I2C->RXDR; //читаем
  }

	Timeout = DS3231_LONG_TIMEOUT;
	while((DS3231_I2C->ISR & I2C_ISR_STOPF) != I2C_ISR_STOPF)	//Wait until STOPF flag is set
  {if((Timeout--) == 0) break;}
  
  /* Clear STOPF flag */
	DS3231_I2C->ICR = I2C_ICR_STOPCF;
}


void ds3231_write(uint8_t RegName, uint8_t RegValue)
{ 
  /* Test on BUSY Flag */
  Timeout = DS3231_LONG_TIMEOUT; 
  while(I2C_GetFlagStatus(DS3231_I2C, I2C_ISR_BUSY) != RESET)
	{
		if((Timeout--) == 0) break;
	}

  /* Configure slave address, nbytes, reload, end mode and start or stop generation */
  I2C_TransferHandling(DS3231_I2C, DS3231_ADRESS, 1, I2C_Reload_Mode, I2C_Generate_Start_Write);
  
  /* Wait until TXIS flag is set */
  Timeout = DS3231_LONG_TIMEOUT; 
  while(I2C_GetFlagStatus(DS3231_I2C, I2C_ISR_TXIS) == RESET)
	{
	  if((Timeout--) == 0) break;
	}
  
  /* Send Register address */
  I2C_SendData(DS3231_I2C, (uint8_t)RegName);
  
  /* Wait until TCR flag is set */
  Timeout = DS3231_LONG_TIMEOUT; 
  while(I2C_GetFlagStatus(DS3231_I2C, I2C_ISR_TCR) == RESET)
	{
		if((Timeout--) == 0) break;
	}
  
  /* Configure slave address, nbytes, reload, end mode and start or stop generation */
  I2C_TransferHandling(DS3231_I2C, DS3231_ADRESS, 1, I2C_AutoEnd_Mode, I2C_No_StartStop);
  
	/* Wait until TXIS flag is set */
	Timeout = DS3231_LONG_TIMEOUT; 
	while(I2C_GetFlagStatus(DS3231_I2C, I2C_ISR_TXIS) == RESET)
	{
		if((Timeout--) == 0) break;
	} 
	
	/* Write data to TXDR */
	I2C_SendData(DS3231_I2C, (uint8_t)(RegValue));
  
  /* Wait until STOPF flag is set */
  Timeout = DS3231_LONG_TIMEOUT; 
  while(I2C_GetFlagStatus(DS3231_I2C, I2C_ISR_STOPF) == RESET)
	{
		if((Timeout--) == 0) break;	
	} 
  
  /* Clear STOPF flag */
  I2C_ClearFlag(DS3231_I2C, I2C_ICR_STOPCF);
}

float ds3231_read_temperature(void)
{
	uint8_t i = 0;
	uint8_t Buf_temp[2];
	float temp;
	
  Timeout = DS3231_LONG_TIMEOUT;
	while((DS3231_I2C->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY)
	{if((Timeout--) == 0)  break;}		
  
  /* Configure slave address, nbytes, reload, end mode and start or stop generation */
  DS3231_I2C->CR2 = (1 << 16) | I2C_CR2_START | DS3231_ADRESS; //передаем адрес устройства
  
  Timeout = DS3231_LONG_TIMEOUT;
	while((DS3231_I2C->ISR & I2C_ISR_TXIS) != I2C_ISR_TXIS) //Wait until TXIS flag is set
  {if((Timeout--) == 0) break;}

  DS3231_I2C->TXDR = (uint8_t)0x11; //передаем адрес регистра

  Timeout = DS3231_LONG_TIMEOUT;
  while((DS3231_I2C->ISR & I2C_ISR_TC) != I2C_ISR_TC)	//Wait until TC flag is set	
  {if((Timeout--) == 0) break;}  
  
  /* Configure slave address, nbytes, reload, end mode and start or stop generation */
  DS3231_I2C->CR2 = I2C_CR2_AUTOEND | 2 << 16 | I2C_CR2_START | I2C_CR2_RD_WRN | DS3231_ADRESS; //передаем адрес устройства, но теперь для чтения
	for(i = 0;i < 2;i++)
	{ 
		Timeout = DS3231_LONG_TIMEOUT;
		while((DS3231_I2C->ISR & I2C_ISR_RXNE) != I2C_ISR_RXNE)
		{if((Timeout--) == 0) break;}
			/* Read data from RXDR */
		Buf_temp[i] = (uint8_t)DS3231_I2C->RXDR; //читаем
  }

	Timeout = DS3231_LONG_TIMEOUT;
	while((DS3231_I2C->ISR & I2C_ISR_STOPF) != I2C_ISR_STOPF)	//Wait until STOPF flag is set
  {if((Timeout--) == 0) break;}
  
  /* Clear STOPF flag */
	DS3231_I2C->ICR = I2C_ICR_STOPCF;
	
	temp = (float)(Buf_temp[0]<<2 | Buf_temp[1]>>6)*0.25;
	
	return temp;
}



void ds3231_read_ALL(DS3231_registers_TypeDef* DS3231_reg)
{
	uint8_t i = 0;
  uint8_t Buf[LENGHT_BUF];
	
  Timeout = DS3231_LONG_TIMEOUT;
	while((DS3231_I2C->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY)
	{if((Timeout--) == 0)  break;}		
  
  /* Configure slave address, nbytes, reload, end mode and start or stop generation */
  DS3231_I2C->CR2 = (1 << 16) | I2C_CR2_START | DS3231_ADRESS; //передаем адрес устройства
  
  Timeout = DS3231_LONG_TIMEOUT;
	while((DS3231_I2C->ISR & I2C_ISR_TXIS) != I2C_ISR_TXIS) //Wait until TXIS flag is set
  {if((Timeout--) == 0) break;}

  DS3231_I2C->TXDR = (uint8_t)0; //передаем адрес регистра

  Timeout = DS3231_LONG_TIMEOUT;
  while((DS3231_I2C->ISR & I2C_ISR_TC) != I2C_ISR_TC)	//Wait until TC flag is set	
  {if((Timeout--) == 0) break;}  
  
  /* Configure slave address, nbytes, reload, end mode and start or stop generation */
  DS3231_I2C->CR2 = I2C_CR2_AUTOEND | LENGHT_BUF << 16 | I2C_CR2_START | I2C_CR2_RD_WRN | DS3231_ADRESS; //передаем адрес устройства, но теперь для чтения
	for(i = 0; i < LENGHT_BUF; i++)
	{ 
		Timeout = DS3231_LONG_TIMEOUT;
		while((DS3231_I2C->ISR & I2C_ISR_RXNE) != I2C_ISR_RXNE)
		{if((Timeout--) == 0) break;}
			/* Read data from RXDR */
		Buf[i] = (uint8_t)DS3231_I2C->RXDR; //читаем
  }
	
	DS3231_reg->seconds 	= Buf[0];              
	DS3231_reg->minutes 	= Buf[1];              
	DS3231_reg->hours 		= Buf[2];                
	DS3231_reg->day 			= Buf[3];                  
	DS3231_reg->date 			= Buf[4];                  
	DS3231_reg->mon_cen 	= Buf[5];              
	DS3231_reg->year 			= Buf[6];                 
	DS3231_reg->alrm1_sec = Buf[7];            
	DS3231_reg->alrm1_min = Buf[8];            
	DS3231_reg->alrm1_hr 	= Buf[9];             
	DS3231_reg->alrm1_doa = Buf[10];            
	DS3231_reg->alrm2_min = Buf[11];            
	DS3231_reg->alrm2_hr 	= Buf[12];             
	DS3231_reg->alrm2_doa = Buf[13];            
	DS3231_reg->ctrl 			= Buf[14];                 
	DS3231_reg->stat 			= Buf[15];                 
	DS3231_reg->aging 		= Buf[16];                
	DS3231_reg->temp_msb 	= Buf[17];             
	DS3231_reg->temp_lsb 	= Buf[18]; 

	Timeout = DS3231_LONG_TIMEOUT;
	while((DS3231_I2C->ISR & I2C_ISR_STOPF) != I2C_ISR_STOPF)	//Wait until STOPF flag is set
  {if((Timeout--) == 0) break;}
  
  /* Clear STOPF flag */
	DS3231_I2C->ICR = I2C_ICR_STOPCF;
}
