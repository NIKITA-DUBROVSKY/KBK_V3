#include "stm32f0xx.h"
#include "AT45DB.h"
#include "SPI_MSD_Driver.h"

extern uint8_t params_AT45DB[13];

uint16_t 	N_page_data, N_adrr_data,
					N_page_NumPage = 0, N_adrr_NumPage;

uint8_t buf_search[528];

#define SPI_DR8     *(uint8_t *)0x4001300C

extern uint8_t status_AT45;

uint8_t AT45DB_Read_Manufacturer_ID(void);

void AT45DB161_Init(void)
{	
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); 

  /* SPI1 Remap enable */
//  GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE );

  /**
  *	SPI1_SCK -> PA5 , SPI1_MISO -> PA6 , SPI1_MOSI ->	PA7
  */		
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  /**
  *	AT45_CS -> PA4 
  */		
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CS_AT45;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(PORT_CS_AT45, &GPIO_InitStructure);
  /**
  *	SD_CS -> PA4 
  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CS_SD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(PORT_CS_SD, &GPIO_InitStructure);
	/**
  * SD_CD -> 
  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_Card_detect;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(PORT_Card_detect, &GPIO_InitStructure);	
	 
	 _card_disable();
	 
    /* Connect PXx to SD_SPI_SCK */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_0);

  /* Connect PXx to SD_SPI_MISO */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_0); 

  /* Connect PXx to SD_SPI_MOSI */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_0);  
	
  CS_AT45_HIGH;
	
  SPI_InitTypeDef SPI_InitStructure;

  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;


  SPI_Init(SPI1, &SPI_InitStructure);
  SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);		

  SPI_Cmd(SPI1, ENABLE);
	
	if(AT45DB_Read_Manufacturer_ID() == 0x1F){
		status_AT45 = SUCCESS;
	}
	else status_AT45 = ERROR;
}
 
uint8_t SPI_SendByte (uint8_t byte){
//Ждем пока передающий буфер не пуст
    while(!(SPI1-> SR & SPI_SR_TXE));
//Передаем байт
    SPI_DR8 = byte;
//Ждем пока приемный буфер пуст
    while(!(SPI1-> SR & SPI_SR_RXNE));
//Возвращаем принятое значение
    return SPI_DR8;
}

uint8_t AT45DB161_Read_Status(void){
    uint8_t temp;
//CS = 0 
    CS_AT45_LOW;
//Отправляем код команды
    SPI_SendByte(0xD7);
//Читаем байт статуса
    temp = SPI_SendByte(0x00);
//Ждем пока завершится работа SPI
    while(SPI1 -> SR & SPI_SR_BSY);
//CS = 1
    CS_AT45_HIGH;
//Возвращаем байт статуса
    return temp;
}

uint8_t AT45DB_Read_Manufacturer_ID(void){
    uint8_t temp;
//CS = 0 
    CS_AT45_LOW;
//Отправляем код команды
    SPI_SendByte(0x9F);
//Читаем байт статуса
    temp = SPI_SendByte(0x00);
//Ждем пока завершится работа SPI
    while(SPI1 -> SR & SPI_SR_BSY);
//CS = 1
    CS_AT45_HIGH;
//Возвращаем байт статуса
    return temp;	
}

/* uint16_t page – номер читаемой страницы (0 – 4095)
   uint16_t addr – адрес байта внутри страницы (0 – 511)
   uint32_t length – количество читаемых байт
   uint8_t *out – указатель, куда ложить прочитанные байты
*/
void AT45DB161_Read_Data(uint16_t page, uint16_t addr, uint32_t length, uint8_t *out){
	uint32_t i;
	uint8_t temp;
	do {
		temp = AT45DB161_Read_Status();
	} while (!(temp & 0x80));
	if (temp & 0x01){	//512
		i = ((page << 9) | (addr & 0x1FF));
	} else {			//528
		i = ((page << 10) | (addr & 0x3FF));
	}
	CS_AT45_LOW;
	SPI_SendByte(0x0B);
	SPI_SendByte(i >> 16);
	SPI_SendByte(i >> 8);
	SPI_SendByte(i);
	SPI_SendByte(0x00);
	for (i = 0; i < length; i++){
		out[i] = SPI_SendByte(0xFF);
	}
	while(SPI1 -> SR & SPI_SR_BSY);
	CS_AT45_HIGH;
}

void AT45DB161_PageProgram(uint16_t page, uint8_t *data, uint16_t length){
	uint16_t i;
	uint8_t temp;
	temp = AT45DB161_Read_Status();
	CS_AT45_LOW;
	SPI_SendByte(0x84);
	SPI_SendByte(0x00);
	SPI_SendByte(0x00);
	SPI_SendByte(0x00);
	for (i = 0; i < length; i++){
		SPI_SendByte(data[i]);
	}
	while(SPI1 -> SR & SPI_SR_BSY);
	CS_AT45_HIGH;
	CS_AT45_LOW;
	SPI_SendByte(0x83);
	if (temp & 0x01){	//512
		SPI_SendByte((uint8_t)(page >> 7));
		SPI_SendByte((uint8_t)((page & 0x7F) << 1));
		SPI_SendByte(0x00);
	} else {			//528
		SPI_SendByte((uint8_t)(page >> 6));
		SPI_SendByte((uint8_t)((page & 0x3F) << 2));
		SPI_SendByte(0x00);
	}
	while(SPI1 -> SR & SPI_SR_BSY);
	CS_AT45_HIGH;
	while (!(AT45DB161_Read_Status() & 0x80));
}

void AT45DB161_PageProgram_without_BUF(uint16_t page, uint16_t addr, uint8_t *data, uint16_t length){
	uint16_t i;
	uint8_t temp;
	temp = AT45DB161_Read_Status();
	CS_AT45_LOW;
	SPI_SendByte(0x02);
	if (temp & 0x01){	//512
		SPI_SendByte((uint8_t)(page >> 7));
		SPI_SendByte((uint8_t)((page & 0x7F) << 1));
		SPI_SendByte(0x00);
	} else {			//528
		SPI_SendByte((uint8_t)(page >> 6));
		SPI_SendByte((uint8_t)((page & 0x3F) << 2) | (addr >> 8));
		SPI_SendByte((uint8_t)(addr & 0xFF));
	}
	for (i = 0; i < length; i++){
		SPI_SendByte(data[i]);
	}
	while(SPI1 -> SR & SPI_SR_BSY);
	CS_AT45_HIGH;
	while (!(AT45DB161_Read_Status() & 0x80));
}

void AT45DB_ChipErase(void){
	CS_AT45_LOW;
	SPI_SendByte(0xC7);
	SPI_SendByte(0x94);
	SPI_SendByte(0x80);
	SPI_SendByte(0x9A);	
	while(SPI1 -> SR & SPI_SR_BSY);
	CS_AT45_HIGH;
	while (!(AT45DB161_Read_Status() & 0x80));
}

void AT45DB_PageErase(uint16_t page){
	uint8_t temp;
	temp = AT45DB161_Read_Status();
	CS_AT45_LOW;
	SPI_SendByte(0x81);
	if (temp & 0x01){	//512
		SPI_SendByte((uint8_t)(page >> 7));
		SPI_SendByte((uint8_t)((page & 0x7F) << 1));
		SPI_SendByte(0x00);
	} else {			//528
		SPI_SendByte((uint8_t)(page >> 6));
		SPI_SendByte((uint8_t)((page & 0x3F) << 2));
		SPI_SendByte(0x00);
	}
	while(SPI1 -> SR & SPI_SR_BSY);
	CS_AT45_HIGH;
	while (!(AT45DB161_Read_Status() & 0x80));
}

uint32_t AT45DB_search_adress(uint16_t page, uint16_t address, uint8_t cnt) //функция Поиска последней записи на странице
{
	uint8_t count_byte = cnt;
	AT45DB161_Read_Data(page, address, 528, buf_search);

	while(count_byte){
	 if(0xFF == buf_search[address++]) 
		 count_byte--;
	 else count_byte = cnt;

	 if(address == 528 - 1) 
		 return 0xFFFF;
	}
	return address -= cnt;
}

void AT45DB_write_number_page(uint16_t data){
	uint8_t buf[2];
	
	buf[0] = data >> 8;   //записываем номер страницы в два байта
	buf[1] = data & 0xFF;
	N_adrr_NumPage = AT45DB_search_adress(N_page_NumPage, 0, 2); //ищем пустой адрес куда записать следующий номер страницы
	if (N_adrr_NumPage == 0xFFFF){ //Страница кончилась, переходим на следующую 
		N_adrr_NumPage = 0;
		N_page_NumPage++;
		if (N_page_NumPage > End_page_NumPage){ //если дошли до конца выделенной страницы
			N_page_NumPage = Start_page_NumPage; //начинаем сначала
		}
		AT45DB_PageErase(N_page_NumPage); //стираем страницу с которой начнем писать
		AT45DB161_PageProgram_without_BUF(N_page_NumPage, N_adrr_NumPage, buf, 2);
	}
	else{
		AT45DB161_PageProgram_without_BUF(N_page_NumPage, N_adrr_NumPage, buf, 2);
	}	
}

void AT45DB_write_data(void){
	N_adrr_data = AT45DB_search_adress(N_page_data, 0, Size_Buff_params_AT45DB); //ищем адрес последних записаных данных
	if (N_adrr_data == 0xFFFF) { //если кончилась страница
		AT45DB161_PageProgram_without_BUF(N_page_data, N_adrr_data, params_AT45DB, Size_Buff_params_AT45DB);
		N_adrr_data	= 0;           //обнуляем адрес на новой странице
		N_page_data++;             //прибавляем номер страницы
		
		if (N_page_data > End_page_data){ //если дошли до конца выделенной страницы
			N_page_data = Start_page_data; //начинаем сначала
		}
		AT45DB_PageErase(N_page_data); //стираем страницу с которой начнем писать		

		AT45DB_write_number_page(N_page_data);		//записываем номер страницы в начало флеша
		
		AT45DB161_PageProgram_without_BUF(N_page_data, N_adrr_data, params_AT45DB, Size_Buff_params_AT45DB);
	}
	else{ //записываем данные по найденному пустому адресу
		AT45DB161_PageProgram_without_BUF(N_page_data, N_adrr_data, params_AT45DB, Size_Buff_params_AT45DB);
	}	
}

void AT45DB_search_data(void){
	//поиск страницы с номером страницы
	N_adrr_NumPage = 0xFFFF; 
	while (N_adrr_NumPage == 0xFFFF){ 
		N_adrr_NumPage = AT45DB_search_adress(N_page_NumPage, 0, 2); 
		if (N_adrr_NumPage != 0xFFFF) 
			break;
		N_page_NumPage++;
		
		if (N_page_NumPage > End_page_NumPage){ //если достигли конца выделенных страниц
			N_page_NumPage = Start_page_NumPage;  //начинаем с начала
			AT45DB_PageErase(N_page_NumPage);
		}
  }	
	
	//вычисляем номер страницы на которой начинаются данные 
	if (N_page_NumPage == 0)
		N_page_data = Start_page_data;
	else
		N_page_data = (buf_search[N_adrr_NumPage-2] << 8) | buf_search[N_adrr_NumPage-1];
	
	//вычисляем адрес байта с которого начинаются данные
	N_adrr_data = 0xFFFF; 
	while (N_adrr_data == 0xFFFF){ //Страница кончилась, переходим на следующую 
		N_adrr_data = AT45DB_search_adress(N_page_data, 0, Size_Buff_params_AT45DB);
		if (N_adrr_data != 0xFFFF) 
			break;
		N_page_data++;
		
		if (N_page_data > End_page_data){ //если достигли конца выделенных страниц
			N_page_data = Start_page_data;  //начинаем с начала
			AT45DB_PageErase(N_page_data);
		}		
  }	
}
