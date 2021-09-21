#include "stm32f0xx.h"
#include <stdio.h>
#include <math.h>
#include "hd44780.h"
#include "buttons.h"
#include "CAN.h"
#include "ADC.h"
#include "led.h"
#include "USART.h"
#include "i2c.h"
#include "SPI_MSD_Driver.h"
#include "ff.h"
#include <string.h>

#include "stm32f0xx_hal.h"
#include "usbd_desc.h"
#include "usbd_storage_if.h"
#include "usbd_cdc_if.h"
#include "usbd_msc_cdc.h"

#include "MicroMenu.h"

#include "timers.h"
#include "messages.h"

#include "AT45DB.h"

#include <stdlib.h>

#include "crypto.h"

uint16_t password = 0xFFFF;
uint16_t password2 =  0xFFFF;
#define blink_time_on_TEST 	80
	
uint8_t MessageDigest[CRL_MD5_SIZE];
int32_t MessageDigestLength = 0;
char serial_num_buf[] = "    ";
uint8_t flag_serial;
uint32_t	serial = 0;
//uint32_t	serial = 4369; //pass = 6189

#define blink_time_on 	80
#define blink_time_off 	10

void save_params(void);
void Read_sensor_LEP(void);
void Read_sensor_USILIE(void);
void Read_sensor_DUG1(void);
void Read_sensor_DUG2(void);
void entry_password(void);
void Check_password(void);
void enter_params_F0(void);
void save_Q_sample(void);
void save_Calib (void); 
void Edit_Time (void);
void save_date(void);
void Edit_Number_KBK (void);
void TEST (void);
void (*Test_func) (void);

extern _uchar_menu Position;
extern uint8_t nParam;
extern uint8_t PRESS_Button, HOLD_Button, REPEAT_Button; 
uint8_t pass_flag = 1;

USBD_HandleTypeDef hUsbDeviceFS;
uint8_t USB_TX[11] = {0x24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x23};
uint8_t USB_TX_Fash[20] = {0};
uint8_t str_rx[21];
uint8_t SD_USB_Disp_in;


uint8_t BAN_REC_SD;
void loading_USB(void);

FIL fsrc;         
FRESULT res;
UINT br;
FATFS fs;
FATFS* fs_ptr = &fs;
DWORD fre_clust, fre_sect, tot_sect, avalible_SD, total_SD;
DIR dir;
FILINFO fileInfo;

char zagolovok_flash[] = "Вылет, дм;Макс. масса, т;Факт. масса, т;Загруженность, %;Угол стрелы, град;Режим работы;Сняты ограничения;Складывание КГ;Откидывание КГ;Подъем крюка;Опускание крюка;Подъем стрелы;Опускание стрелы;Время;Дата\r\n";
char zagolovok[] = "Вылет, мм;Макс. масса, кг;Факт. масса, кг;Загруженность, %;Датчик ЛЭП;Угол стрелы, град;Крен, град;Тангаж, град;Противовес;Режим работы;Сняты ограничения;Тип трубоукладчика;Время\r";
char textFileBuffer[]="                                                       "; 
char *limitations_str;
char *LEP_str;
char *mode_str;
char date_txt[]="0:/01_01_01.TXT";
uint8_t flag_delay_SD;
typedef struct 
{	
float					angle_alpha_setup;
float					angle_betta_setup;
float					angle_gamma_setup;
	
int8_t				cnt_lep; //Выбранный диапазон ЛЭП
uint8_t	      flag_TG; //указывает какой кран выбран (от 1 до последнего пункта меню с кранами) по порядку
	
uint8_t 			k_TG;// коэф для формулы расчета массы от вылета для разных кранов (70 - ТГ_35, 40 - ТГ_20, 24 - ТГ_12)

int16_t				F0; //Параметры калибровки Q
int16_t	      K;  //Параметры калибровки Q
	
uint32_t			AZK_N; //Номер АЗК

uint16_t			max_massa;//ТГ50 - 55000, ТГ35 - 45000, ТГ20 - 28000, ТГ12 - 18000;

uint16_t			L;// 			= 8600
uint16_t			L_og;// 		= 560, 
uint16_t			Hsh;// 		= 899, 
uint16_t			Lsh;// 		= 690;
uint16_t			rezerv;
uint32_t			rezerv2;

uint32_t 			angle_strely_min; //15

float					KPD_polispast;

uint8_t				flag_memory_card; //0 - карта паяти вставлена, 1 - карты памяти нет, и была нажата кнопка согласия работы без карты памяти
} FLASH_Params_def;
FLASH_Params_def params;//структура с переменными которые нужно хранить во флэше

typedef struct {

	uint32_t 			flag_device_first_turn_on; //0 - не было первого включения, после перепрошивки, 1 - было первое включение и был выбран трубач	
	
} FLASH_erasable_params_def;
FLASH_erasable_params_def erasable_params;

uint8_t protivoves = 0;  //флаг состояния противовеса

//регистры часов реального времени
DS3231_registers_TypeDef DS3231registers;

#define reg_sec 	0x00
#define reg_min 	0x01
#define reg_hour 	0x02
#define reg_day 	0x04
#define reg_month 0x05
#define reg_year 	0x06
uint16_t time_setup_count;
uint8_t 			ds3231_hour, ds3231_min, ds3231_sec, blinkflag,
							ds3231_day = 1, ds3231_month = 1;
uint16_t 			ds3231_year = 2020;
#define LENGHT 7
uint8_t DS3231_Buf[LENGHT];
uint32_t file_date = 4000000000; 
uint16_t file_err_num, file_err_max_num;
char file_name[13];

uint8_t 			beep_flag = 0; //состояние пищалки
uint8_t   		beep_off;

uint16_t 			led_status[31]; //массив состояний светодиодов

uint8_t 			flag_test = 0; //флаг нажатия кнопки ТЕСТ (работает только на главной странице)

float  				DT_ADC, DT_C;	//переменные для датчика температуры						
uint16_t 			ADCBuffer[] = {0xAAAA}; //буффер АЦП

float 				angle_alpha = 0,
							angle_gamma = 0, 
							angle_betta = 0,
							angle_gamma_rama;

int16_t				angle_alpha_CAN = 0,
							angle_gamma_CAN = 0, 
							angle_betta_CAN = 0;

uint8_t 			angle_alpha_CAN_USB,
							angle_betta_CAN_USB,
							USILIE_CAN_USB_L,
							USILIE_CAN_USB_H;
							
float 				angle_gamma_rad,  //углы в радианах для формулы вылета
							angle_betta_rad,
							angle_gamma_rad_setup,
							angle_betta_rad_setup;

uint16_t 			MzMAP,
							Mz = 0, 
							Mz_temp;

float 				R, R_temp, R_display, Q_display, Qf2_display;

uint8_t				mode = 140;
uint16_t      a = 1116;
int16_t				y0 = -2919;


#define max_massa_TG50 55000
#define max_massa_TG35 45000
#define max_massa_TG20 28000
#define max_massa_TG12 18000

#define L_og_TG35	215
#define Hsh_TG35	905
#define Lsh_TG35	512

#define L_og_TG20	295
#define Hsh_TG20	617
#define Lsh_TG20	522

#define L_og_TG12	300
#define Hsh_TG12	620
#define Lsh_TG12	463

#define L_strely_TG35_8m 		8650
#define L_strely_TG35_10m 	10500
#define L_strely_TG20_9m 		9000
#define L_strely_TG12_5m 		5000
#define L_strely_TG12_6m 		6000
#define L_strely_TG12_7m 		7000
#define L_strely_TG12_9m 		9000

#define KPD_polispast_4 	0.9506
#define KPD_polispast_5 	0.9408
#define KPD_polispast_6 	0.9318
#define KPD_polispast_8 	0.9139

#define k_TG50 100
#define k_TG35 70
#define k_TG20 40
#define k_TG12 24

typedef enum
{
	rezerv,
	TG_50_8m6,
	TG_35_strela_opora,
	TG_35_8m6,
	TG_35_10m,
	TG_20_9m,
	TG_20_7m,
	TG_12_9m,
	TG_12_7m,
	TG_12_6m,
	TG_12_5m,
}Type_TG_TypeDef;

uint32_t			Qmax;
float 				Q_massa_temp = 0;
int32_t				Q_massa, Qf2;
						
char 					io_buff[100];//буфера для строки с переменной
uint8_t				cnt_leds	=	0; //счетчик для светодиодов загруженности	
												
uint8_t 	CRC_DUG50_CAN = 0,
					CRC_DUG51_CAN = 0,
					CRC_LEP_CAN = 0,
					CRC_USILIE_CAN = 0,
					CRC_JOY_CAN = 0,
					NO_SENSORS = 0;

uint8_t LEP_CAN;
uint16_t USILIE_CAN;
uint8_t KONCEVIK_LEP, KONCEVIK_DUG_50, KONCEVIK_DUG_51;

uint8_t check_flash_params;
#define MAIN_PROGRAM_START_ADDRESS (uint32_t)0x08004000
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));

#define PARAMS_WORD_CNT 	sizeof(params) / sizeof(uint32_t) // Расчитывается исходя из размера структуры в памяти МК деленного на размер блока (4 байта)
#define PARAMS_WORD_CNT_erasable 	sizeof(erasable_params) / sizeof(uint32_t) // Расчитывается исходя из размера структуры в памяти МК деленного на размер блока (4 байта)
#define PAGE_62 	0x0801F000 //адрес предпоследней страницы для 128кБ, т.к. старницы по 2 кБ
#define PAGE_61 	0x0801E800
#define PAGE_60 	0x0801E000
#define PAGE_32 	0x08010000

uint32_t working_minutes;
#define Start_Search_Memory PAGE_61 //адрес с которого начнем писать моточасы
#define ENDMEMORY (Start_Search_Memory + 0x800) //(0x800 - размер страницы в stm32f072) условный конец памяти 62 страница (F072)
#define time_write_work_minutes 1 //через сколько минут обновляется значение моточасов во флэш памяти
uint32_t adr_last_record;

#define T_on 10
#define T_off 20
#define heated_on() 	GPIO_SetBits(GPIOB,GPIO_Pin_10);
#define heated_off() 	GPIO_ResetBits(GPIOB,GPIO_Pin_10);

uint8_t strela_up = 1, strela_up_CAN = 1, strela_down = 1, strela_down_CAN = 1, kruk_up = 1, kruk_up_CAN = 1, kruk_down = 1,kruk_down_CAN = 1, 
				KG_skladivanie = 1, KG_skladivanie_CAN = 1,  KG_otkidivanie = 1, KG_otkidivanie_CAN = 1, angle_strely_max = 88;

uint8_t MAX_Mz, MAX_LEP, MAX_ROLL, MAX_PITCH, MAX_UP_STRELY, MAX_DOWN_STRELY, GRUZ_NA_KRUKE;

uint8_t konc_kruka, konc_streli;
uint8_t	TS_kruka = 1, TS_strely = 1;

uint8_t flag_button_unlocking, flag_unlocking;


uint8_t CAN_MESSAGE, CAN_MESSAGE_1;

uint8_t ATT_LEFT, ATT_RIGHT, ATT_FWD, ATT_REV;

int8_t digit[6];
uint8_t pass[4];

uint8_t flag_lock_block;

uint8_t copy_data_to_SD = 0;

#define	TICKs_ms 48000000/1000 //вычисляем кол-во тиков в 1 мс

uint8_t status_RCC, status_RTC, status_SD, status_AT45, status_CAN_Transsiver;

extern PCD_HandleTypeDef hpcd_USB_FS;

uint16_t ChannelPulse = 0, TimerPeriod = 0;
extern int8_t str_number;
extern uint8_t list;
uint8_t m_number = 0;

uint8_t ComputedCRC = 0;
uint8_t buf_AT45DB[528];
uint8_t params_AT45DB[13];
uint8_t buf_1[528] = {0};
uint16_t page_write;

typedef enum
{
	AT45DB_time_hour = 0,
	AT45DB_time_min,
	AT45DB_time_sec,
	AT45DB_time_day,
	AT45DB_time_month,
	AT45DB_M_z,
	AT45DB_Fact_massa,
	AT45DB_Max_massa,
	AT45DB_angle_strely,
	AT45DB_R_strely,
	AT45DB_mode_OffLimit_EnLimit,
	AT45DB_CRC_params,
	AT45DB_NULL_params
} AT45DB_params_TypeDef;


static void m_print_skroll(void);
static void m_print_simple(void);
float edit_var(float var, int32_t Low, int32_t Hi, float incValue_PRESS, float incValue_HOLD);
static void m_init_f_sample(void);
void dunamic_f_sample(void);
void m_init_dunamic(void);
void init_dunamic_func(void (*dunamic_f)(void));
void calibration_Q1 (void);
void calibration_Q2 (void);

NVIC_InitTypeDef  NVIC_InitStructure;
long map(long x, long in_min, long in_max, long out_min, long out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void FLASH_WriteSettings(void){

	FLASH_Unlock();                                                              
	FLASH_ErasePage(PAGE_62); 
	
	uint32_t *source_adr = (void *)&params;
	uint32_t *dest_addr = (uint32_t *) PAGE_62;
	
	for (uint16_t i=0; i<PARAMS_WORD_CNT; i++) {
				FLASH_ProgramWord((uint32_t)dest_addr, *source_adr);
        source_adr++;
        dest_addr++;
    }
	
	FLASH_Lock();
}

void FLASH_Write_erasable_parameters(void){

	FLASH_Unlock();                                                              
	FLASH_ErasePage(PAGE_60); 
	
	uint32_t *source_adr = (void *)&erasable_params;
	uint32_t *dest_addr = (uint32_t *) PAGE_60;
	
	for (uint16_t i=0; i<PARAMS_WORD_CNT_erasable; i++) {
				FLASH_ProgramWord((uint32_t)dest_addr, *source_adr);
        source_adr++;
        dest_addr++;
    }
	
	FLASH_Lock();
}

void FLASH_Read_erasable_parameters(void){
	uint32_t *source_adr = (uint32_t *)(PAGE_60);   // Определяем адрес, откуда будем читать
	uint32_t *dest_adr = (void *)&erasable_params;                                           // Определяем адрес, куда будем писать

	for (uint16_t i=0; i < PARAMS_WORD_CNT_erasable; ++i) {                                  // В цикле производим чтение
		*(dest_adr + i) = *(__IO uint32_t*)(source_adr + i);                    // Само чтение
	}	
}

void Flash_ReadParams(void){
	uint32_t *source_adr = (uint32_t *)(PAGE_62);   // Определяем адрес, откуда будем читать
	uint32_t *dest_adr = (void *)&params;                                           // Определяем адрес, куда будем писать

	for (uint16_t i=0; i < PARAMS_WORD_CNT; ++i) {                                  // В цикле производим чтение
		*(dest_adr + i) = *(__IO uint32_t*)(source_adr + i);                    // Само чтение
	}
}

uint32_t flash_search_adress(uint32_t address, uint8_t cnt){ //функция Поиска последней записи
 uint8_t count_byte = cnt;

 while(count_byte)
 {
   if(0xFF == *(uint8_t*)address++) count_byte--;
   else count_byte = cnt;

   if(address == ENDMEMORY - 1) return 0;
 }

 return address -= cnt;
}

void Flash_Write_EepromEmulation(uint32_t date_flash){ 
	adr_last_record	= flash_search_adress(adr_last_record, 4); //ищем пока не будет четыре 0xFF подряд (значит ячейки пусты)
	if (adr_last_record == 0) { //память закончилась, нужно снова затирать всю память
		FLASH_Unlock();                                                              
		FLASH_ErasePage(Start_Search_Memory);
		adr_last_record	= Start_Search_Memory;
		FLASH_ProgramWord((uint32_t)Start_Search_Memory, date_flash);
		FLASH_Lock();
	}
	else{ //записываем данные по найденному пустому адресу
		FLASH_Unlock();                                                              
		FLASH_ProgramWord((uint32_t)adr_last_record, date_flash);
		FLASH_Lock();
	}
}

void blink_symbol(void){
	if(GetGTimer(TIMER_Blink_symbol) >= 1)
	{	
		time_setup_count++;
		ResetGTimer(TIMER_Blink_symbol);
	}
	
	if (time_setup_count > blink_time_on) { blinkflag = 1; time_setup_count = 0; }
	if (time_setup_count > blink_time_off && blinkflag) { blinkflag = 0; time_setup_count = 0; }
}

uint8_t DEC_to_BCD(uint8_t val){
	return (((val/10) << 4) | (val % 10));
}

uint8_t BCD_to_DEC(uint8_t val){
	return ((((val & 0xF0) >> 4) * 10) + (val & 0x0F));
}

void main_page_0 (void){ //вывод Mz, R, Qm, Q
	uint8_t	Qdiv, Qmod, Qmdiv, Qmmod;
	if (Qmax > params.max_massa) Qmax = params.max_massa;
	lcd44780_SetLCDPosition(0, 0);
	sprintf(io_buff, "Mз=%3d%%", Mz);
	lcd44780_ShowStr(io_buff);

	Qmdiv = Qmax/1000;
	sprintf(io_buff, " Qm=%2d.", Qmdiv);
	lcd44780_ShowStr(io_buff);
	Qmmod = Qmax%1000/100;
	sprintf(io_buff, "%1dт", Qmmod);
	lcd44780_ShowStr(io_buff);
	//sprintf(io_buff, "Qm=%.5d", Qm);
	
	R_display = R / 1000;
	lcd44780_SetLCDPosition(0, 1);
	sprintf(io_buff, "R=%1.2fm", R_display);
	lcd44780_ShowStr(io_buff);

	Qdiv = Q_massa/1000;
	sprintf(io_buff, "  Q=%2d.", Qdiv);
	lcd44780_ShowStr(io_buff);
	Qmod = Q_massa%1000/100;
	sprintf(io_buff, "%1dт", Qmod);
	lcd44780_ShowStr(io_buff);
	
	if (HOLD_Button == BUTTON_DOWN && flag_test) 
	{
		TEST();
		flag_test = 0;
	}
	
	
	if (HOLD_Button == BUTTON_DOWN && !flag_test) 
	{
		TEST();
		flag_test = 1;
	}

	if (HOLD_Button == BUTTON_UP){ //снятие ограничений
		if (flag_button_unlocking == 1) {
			if (flag_unlocking == 0) flag_unlocking = 1;
			else flag_unlocking = 0;
		}
	}	
}

void main_page_1 (void){ //вывод Углов
	//градусы - 
	lcd44780_SetLCDPosition(0, 1);
	if (CRC_DUG50_CAN == 1)	{
		sprintf(io_buff, "=%4.0f", angle_gamma_rama);//гамма - 
		lcd44780_ShowStr(io_buff);
	}
	else
		lcd44780_ShowStr("=----");
	
	if (CRC_DUG51_CAN == 1)	{
	lcd44780_SetLCDPosition(0, 0);
	sprintf(io_buff, "          =%3.0f     ", angle_alpha - params.angle_alpha_setup);//альфа - 
	lcd44780_ShowStr(io_buff);
  lcd44780_SetLCDPosition(7, 1);
	sprintf(io_buff, "   =%3.0f     ", angle_betta - params.angle_betta_setup);//бетта -
	lcd44780_ShowStr(io_buff);
	}
	else {
		lcd44780_SetLCDPosition(0, 0);
		lcd44780_ShowStr("          =---     ");
		lcd44780_SetLCDPosition(7, 1);
		lcd44780_ShowStr("   =---     ");
	}		
	
	if (HOLD_Button == BUTTON_DOWN && flag_test) 
	{
		TEST();
		flag_test = 0;
	}
	if (HOLD_Button == BUTTON_DOWN && !flag_test) 
	{
		TEST();
		flag_test = 1;
	}
	if (HOLD_Button == BUTTON_UP){ //снятие ограничений
		if (flag_button_unlocking == 1) {
			if (flag_unlocking == 0) flag_unlocking = 1;
			else flag_unlocking = 0;
		}
	}	
}

void main_page_2 (void){ //вывод LEP
	lcd44780_SetLCDPosition(0, 0);
	if (protivoves == 1)
	lcd44780_ShowStr("Противовес:отодв");
	else if (protivoves == 0) lcd44780_ShowStr("Противовес:придв");
	else lcd44780_ShowStr("Противовес:-----");
	lcd44780_SetLCDPosition(0, 1);
//	if (flag_heat == 1)
//		sprintf(io_buff, "t=%2.0f  Heat - ON ", DT_C);
//	else sprintf(io_buff, "t=%2.0f  Heat - OFF", DT_C);
//	lcd44780_ShowStr(io_buff);
	lcd44780_ShowStr("                ");
	
	if (HOLD_Button == BUTTON_DOWN && flag_test) 
	{
		TEST();
		flag_test = 0;
	}
	if (HOLD_Button == BUTTON_DOWN && !flag_test) 
	{
		TEST();
		flag_test = 1;
	}
	if (HOLD_Button == BUTTON_UP){ //снятие ограничений
		if (flag_button_unlocking == 1) {
			if (flag_unlocking == 0) flag_unlocking = 1;
			else flag_unlocking = 0;
		}
	}		
}


void calibration_Q (void){
	if (Qmax > params.max_massa) Qmax = params.max_massa;
	switch (nParam)
	{	
		case 1:
			lcd44780_SetLCDPosition(0, 0);
			Q_display = (float)Qmax / 1000;
			sprintf(io_buff, "1:Qm=%4.1f", Q_display);
			lcd44780_ShowStr(io_buff);

			R_display = R / 1000;
			sprintf(io_buff, " R=%1.1fm ", R_display);
			lcd44780_ShowStr(io_buff);
			
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("Qf= 0.0т");
				
			sprintf(io_buff, "  F=%.3d ", USILIE_CAN);
			lcd44780_ShowStr(io_buff);
		break;
		case 2:
			Qf2 = edit_var(Qf2, 0, 60000, 100, 100);
			lcd44780_SetLCDPosition(0, 0);
			Q_display = (float)Qmax / 1000;
			sprintf(io_buff, "2:Qm=%4.1f", Q_display);
			lcd44780_ShowStr(io_buff);

			sprintf(io_buff, " R=%1.1fm", R_display);
			lcd44780_ShowStr(io_buff);

			lcd44780_SetLCDPosition(0, 1);
			Qf2_display = (float)Qf2/1000;
			sprintf(io_buff, "Qf=%4.1fт", Qf2_display);
			lcd44780_ShowStr(io_buff);

			sprintf(io_buff, "  F=%.3d ", USILIE_CAN);
			lcd44780_ShowStr(io_buff);	
		break;
	}
}

void calibration_R (void){ //Калибровка Вылета
		params.angle_gamma_setup = edit_var(params.angle_gamma_setup, -90, 90, 0.1, 1);
		if (Qmax > params.max_massa) Qmax = params.max_massa;
		lcd44780_SetLCDPosition(0, 0);
		R_display = R / 1000;
		sprintf(io_buff, "Вылет: R=%1.2fm   ", R_display);
		lcd44780_ShowStr(io_buff);
	
		lcd44780_SetLCDPosition(0, 1);
		sprintf(io_buff, "=(%4.1f%+4.1f)   ", angle_gamma, params.angle_gamma_setup);//гамма - 
		lcd44780_ShowStr(io_buff);
}

void vvod_krena (void){ //Калибровка крена
	lcd44780_SetLCDPosition(0, 0);
	lcd44780_ShowStr("Ввод крена      ");
	lcd44780_SetLCDPosition(0, 1);
	sprintf(io_buff, "=%4.1f", angle_alpha);//альфа - 
	lcd44780_ShowStr(io_buff);
	sprintf(io_buff, " =%4.1f ", angle_betta);//бетта -
	lcd44780_ShowStr(io_buff);
	params.angle_alpha_setup = angle_alpha;
	params.angle_betta_setup = angle_betta;
}

							
uint8_t 			ds3231_hour_temp, ds3231_min_temp, ds3231_sec_temp, ds3231_day_temp, ds3231_month_temp;
uint16_t 			ds3231_year_temp;
void date(void){

	switch (nParam)
	{
		case 1:
      ds3231_hour_temp = ds3231_hour;
			ds3231_min_temp = ds3231_min;
			ds3231_sec_temp = ds3231_sec;
		  ds3231_day_temp = ds3231_day;
		  ds3231_month_temp = ds3231_month;
		  ds3231_year_temp = ds3231_year;
		
			lcd44780_SetLCDPosition(0, 0);
			lcd44780_ShowStr("Дата  ");
			sprintf(io_buff, "%.2d.", ds3231_day);
			lcd44780_ShowStr(io_buff);
			sprintf(io_buff, "%.2d.", ds3231_month);
			lcd44780_ShowStr(io_buff);
			sprintf(io_buff, "%.4d", ds3231_year);
			lcd44780_ShowStr(io_buff);
			
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("Время  ");
			sprintf(io_buff, "%.2d:", ds3231_hour_temp);
			lcd44780_ShowStr(io_buff);
			sprintf(io_buff, "%.2d:", ds3231_min_temp);
			lcd44780_ShowStr(io_buff);
			sprintf(io_buff, "%.2d  ", ds3231_sec_temp);
			lcd44780_ShowStr(io_buff);
		break;
		case 2:
			ds3231_day_temp = edit_var(ds3231_day_temp, 0, 31, 1, 1);
			blink_symbol();
			lcd44780_SetLCDPosition(6, 0);	
			if (blinkflag)lcd44780_ShowStr("  .");			
			else{
				sprintf(io_buff, "%.2d", ds3231_day_temp);
				lcd44780_ShowStr(io_buff);
			}
		break;
		case 3:
			ds3231_month_temp = edit_var(ds3231_month_temp, 0, 12, 1, 1);
			blink_symbol();
			lcd44780_SetLCDPosition(6, 0);
			sprintf(io_buff, "%.2d.", ds3231_day_temp);
			lcd44780_ShowStr(io_buff);
			if (blinkflag)lcd44780_ShowStr("  .");			
			else{
				sprintf(io_buff, "%.2d", ds3231_month_temp);
				lcd44780_ShowStr(io_buff);
			}
		break;
		case 4:
			ds3231_year_temp = edit_var(ds3231_year_temp, 2020, 2100, 1, 1);
			blink_symbol();
			lcd44780_SetLCDPosition(9, 0);
			sprintf(io_buff, "%.2d.", ds3231_month_temp);
			lcd44780_ShowStr(io_buff);
			if (blinkflag)lcd44780_ShowStr("    ");			
			else{
				sprintf(io_buff, "%.4d", ds3231_year_temp);
				lcd44780_ShowStr(io_buff);
			}
		break;			
		case 5:
			ds3231_hour_temp = edit_var(ds3231_hour_temp, 0, 23, 1, 1);
			blink_symbol();
			lcd44780_SetLCDPosition(12, 0);
			sprintf(io_buff, "%.4d", ds3231_year_temp);
			lcd44780_ShowStr(io_buff);
			lcd44780_SetLCDPosition(7, 1);	
			if (blinkflag)lcd44780_ShowStr("  :");			
			else{
				sprintf(io_buff, "%.2d", ds3231_hour_temp);
				lcd44780_ShowStr(io_buff);
			}
		break;
		case 6:
			ds3231_min_temp = edit_var(ds3231_min_temp, 0, 59, 1, 1);
			blink_symbol();
			lcd44780_SetLCDPosition(7, 1);
			sprintf(io_buff, "%.2d:", ds3231_hour_temp);
			lcd44780_ShowStr(io_buff);
			if (blinkflag)
			{
							lcd44780_ShowStr("  :");
			}				
			else 
			{
							sprintf(io_buff, "%.2d", ds3231_min_temp);
							lcd44780_ShowStr(io_buff);
			}
		break;
		case 7:
			ds3231_sec_temp = edit_var(ds3231_sec_temp, 0, 59, 1, 1);
			blink_symbol();
			lcd44780_SetLCDPosition(10, 1);
			sprintf(io_buff, "%.2d:", ds3231_min_temp);
			lcd44780_ShowStr(io_buff);
			if (blinkflag)
			{
							lcd44780_ShowStr("   ");
			}				
			else 
			{
							sprintf(io_buff, "%.2d", ds3231_sec_temp);
							lcd44780_ShowStr(io_buff);
			}
		break;			
	}

}

void service(void){
	blink_symbol();
	
	switch (nParam)
	{
		case 1:
			digit[0] = params.AZK_N / 100000;
			digit[1] = params.AZK_N % 100000 / 10000;
			digit[2] = params.AZK_N % 10000 / 1000;
			digit[3] = params.AZK_N % 1000 / 100;
			digit[4] = params.AZK_N % 100 / 10;
			digit[5] = params.AZK_N % 10;
			
			lcd44780_SetLCDPosition(0, 0);
			lcd44780_ShowStr("АЗК N");
			sprintf(io_buff, "%.1d", digit[0]);
			lcd44780_ShowStr(io_buff);
			sprintf(io_buff, "%.1d", digit[1]);
			lcd44780_ShowStr(io_buff);
			sprintf(io_buff, "%.1d", digit[2]);
			lcd44780_ShowStr(io_buff);
			sprintf(io_buff, "%.1d", digit[3]);
			lcd44780_ShowStr(io_buff);
			sprintf(io_buff, "%.1d", digit[4]);
			lcd44780_ShowStr(io_buff);
			sprintf(io_buff, "%.1d     ", digit[5]);
			lcd44780_ShowStr(io_buff);

			lcd44780_SetLCDPosition(0, 1);
			sprintf(io_buff, "Моточасы: %5dч", working_minutes/60);
			lcd44780_ShowStr(io_buff);
  break;
	case 2:		
		digit[nParam-2] = edit_var(digit[nParam-2], 0, 9, 1, 1);
		lcd44780_SetLCDPosition(5, 0);
		if (blinkflag) lcd44780_ShowStr(" ");
		else {
			sprintf(io_buff, "%.1d", digit[nParam-2]);
			lcd44780_ShowStr(io_buff);		
		}		
	break;
	case 3:
		digit[nParam-2] = edit_var(digit[nParam-2], 0, 9, 1, 1);
		lcd44780_SetLCDPosition(5, 0);
		sprintf(io_buff, "%.1d", digit[0]);
		lcd44780_ShowStr(io_buff);
		if (blinkflag) lcd44780_ShowStr(" ");
		else {
			sprintf(io_buff, "%.1d", digit[nParam-2]);
			lcd44780_ShowStr(io_buff);		
		}	
	break;
	case 4:
		digit[nParam-2] = edit_var(digit[nParam-2], 0, 9, 1, 1);
		lcd44780_SetLCDPosition(6, 0);
		sprintf(io_buff, "%.1d", digit[1]);
		lcd44780_ShowStr(io_buff);
		if (blinkflag) lcd44780_ShowStr(" ");
		else {
			sprintf(io_buff, "%.1d", digit[nParam-2]);
			lcd44780_ShowStr(io_buff);		
		}	
	break;
	case 5:
		digit[nParam-2] = edit_var(digit[nParam-2], 0, 9, 1, 1);
		lcd44780_SetLCDPosition(7, 0);
		sprintf(io_buff, "%.1d", digit[2]);
		lcd44780_ShowStr(io_buff);
		if (blinkflag) lcd44780_ShowStr(" ");
		else {
			sprintf(io_buff, "%.1d", digit[nParam-2]);
			lcd44780_ShowStr(io_buff);		
		}	
	break;
	case 6:
		digit[nParam-2] = edit_var(digit[nParam-2], 0, 9, 1, 1);
		lcd44780_SetLCDPosition(8, 0);
		sprintf(io_buff, "%.1d", digit[3]);
		lcd44780_ShowStr(io_buff);
		if (blinkflag) lcd44780_ShowStr(" ");
		else {
			sprintf(io_buff, "%.1d", digit[nParam-2]);
			lcd44780_ShowStr(io_buff);		
		}	
	break;
	case 7:
		digit[nParam-2] = edit_var(digit[nParam-2], 0, 9, 1, 1);
		lcd44780_SetLCDPosition(9, 0);
		sprintf(io_buff, "%.1d", digit[4]);
		lcd44780_ShowStr(io_buff);
		if (blinkflag) lcd44780_ShowStr(" ");
		else {
			sprintf(io_buff, "%.1d", digit[nParam-2]);
			lcd44780_ShowStr(io_buff);		
		}	
		params.AZK_N = digit[0]*100000+digit[1]*10000+digit[2]*1000+digit[3]*100+digit[4]*10+digit[5];
	break;
	}
}

void limitations (void){
	lcd44780_SetLCDPosition(0, 0);
	lcd44780_ShowStr("КГ");lcd44780_ShowChar(0xD9);
	lcd44780_ShowStr("КГ");lcd44780_ShowChar(0xDA);
	lcd44780_ShowStr(" К");lcd44780_ShowChar(0xD9);
	lcd44780_ShowStr("К");lcd44780_ShowChar(0xDA);
	lcd44780_ShowStr(" C");lcd44780_ShowChar(0xD9);
	lcd44780_ShowStr("C");lcd44780_ShowChar(0xDA);
	lcd44780_SetLCDPosition(0, 1);
	sprintf(io_buff, "  %d  %d  %d %d  %d %d", KG_skladivanie_CAN, KG_otkidivanie_CAN, kruk_up_CAN, kruk_down_CAN, strela_up_CAN, strela_down_CAN);
	lcd44780_ShowStr(io_buff);
}

void program_version(void){
	lcd44780_SetLCDPosition(0, 0);
	lcd44780_ShowStr("Версия ПО: 1.0  ");
	lcd44780_SetLCDPosition(0, 1);
	lcd44780_ShowStr("                ");
}	





// Функция задержки в миллисекундах работает от 2 до 65536 мс
void delay_ms(uint16_t n){
TIM17->PSC = TICKs_ms-1; //Загрузить кол-во тиков для одной мс в регистр предварительного делителя
TIM17->ARR = n-1; // Загрузить число отсчётов в регистр автозагрузки
TIM17->EGR |= TIM_EGR_UG; // Сгенерировать событие обновления для записи данных в регистры PSC и ARR
TIM17->CR1 |= TIM_CR1_CEN | TIM_CR1_OPM; // Пуск таймера
//путём записи бита разрешения счёта CEN
//и бита режима одного прохода OPM в регистр управления CR1
while ((TIM17->CR1 & TIM_CR1_CEN) != 0); // Ожидание окончания счёта
}


void Init_RCC(void){
	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);
	
	if (RCC_WaitForHSEStartUp() == SUCCESS){
		status_RCC = SUCCESS;
		RCC_PREDIV1Config(RCC_PREDIV1_Div1);
		RCC_PLLConfig(RCC_PLLSource_HSE, RCC_PLLMul_6);//кварц 8MHz*5=40мгц
		RCC_PLLCmd(ENABLE);
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		RCC_PCLKConfig(RCC_HCLK_Div2);
	}
	else{
		status_RCC = ERROR;
		RCC_PREDIV1Config(RCC_PREDIV1_Div1);
		RCC_PLLConfig(RCC_PLLSource_HSI, RCC_PLLMul_5);//кварц 8MHz*5=40мгц
		RCC_PLLCmd(ENABLE);
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		RCC_PCLKConfig(RCC_HCLK_Div1);		
	}

	FLASH_SetLatency(FLASH_Latency_1);
	FLASH_PrefetchBufferCmd(ENABLE);		

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
		
	RCC_HSI48Cmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_HSI48RDY) == RESET)
	{
	}

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOA,  ENABLE );
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN | RCC_APB1Periph_USB, ENABLE);
	
	RCC_USBCLKConfig(RCC_USBCLK_HSI48);
	
	RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
}

void GPIO_USB_Init (void){
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	delay_ms(10); // немного ждём
}

void MX_USB_DEVICE_Init(void){
	USBD_Init(&hUsbDeviceFS, &FS_Desc, 0);
	#ifdef USE_USB_COMPOSITE
		USBD_RegisterClass(&hUsbDeviceFS, &USBD_MSC_CDC_ClassDriver);
		USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);	
		USBD_MSC_RegisterStorage(&hUsbDeviceFS, &USBD_Storage_Interface_fops_FS);
	#elif defined(USE_USB_MSC)
		USBD_RegisterClass(&hUsbDeviceFS, &USBD_MSC);
		USBD_MSC_RegisterStorage(&hUsbDeviceFS, &USBD_Storage_Interface_fops_FS);
	#else
		USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
		USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
	#endif
	USBD_Start(&hUsbDeviceFS);
}

void USB_IRQHandler(void){
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
}

void loading_USB(void){
	blink_symbol();
		
	if (blinkflag) {
		lcd44780_SetLCDPosition(0, 0);
		lcd44780_ShowStr("                ");
		lcd44780_SetLCDPosition(0, 1);
		lcd44780_ShowStr("                ");
	}
	else {
	  lcd44780_SetLCDPosition(0, 0);
		lcd44780_ShowStr(" Чтение Памяти  ");
		lcd44780_SetLCDPosition(0, 1);
		lcd44780_ShowStr("                ");
	}
//	
//	lcd44780_SetLCDPosition(0, 0);
//	lcd44780_ShowStr(" Чтение Памяти  ");
//	if (pos_i == 0) {
//		lcd44780_SetLCDPosition(pos_i, 1);
//		lcd44780_ShowStr("                ");
//	}
//	if (flag_cnt_SD == 1 && pos_i <= 16) 
//	{	
//		lcd44780_SetLCDPosition(pos_i, 1);
//		pos_i++;
//		lcd44780_WriteByte(0xFF);
//		lcd44780_ShowStr("               ");
//		flag_cnt_SD = 0;
//	}
}

void Independent_Watchdog(void){
	RCC_LSICmd(ENABLE);

  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {}
		
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_16); // 4, 8, 16 ... 256
  IWDG_SetReload(0x0FFF);//This parameter must be a number between 0 and 0x0FFF.
  IWDG_ReloadCounter();
  IWDG_Enable();
}


void SD_Card_Handler(void){
	if(status_SD == SUCCESS){
		if(GetGTimer(TIMER_SD_Card_Handler) >= 9*ms_100)
		{	
			ResetGTimer(TIMER_SD_Card_Handler);
			if(BAN_REC_SD == 0){	
//				if(_card_insert() == 0 && flag_delay_SD == 0) //если карточка вставлена 
//				{
//					flag_delay_SD = 1;
//					delay_ms(1000); //задержка для вставки SD -карты на горячую 
//				}
//				else if (_card_insert() == 1 && flag_delay_SD == 1) flag_delay_SD = 0;
//				
//				if (flag_delay_SD == 1){
					if (Mz > 90 || led_status[LED_STOP] == 1) 
						{					 							
							if (flag_unlocking == 0) limitations_str = "-";
							else limitations_str = "+";
							
							if (mode == 140) mode_str = "О";
							else if (mode == 117) mode_str = "К";
							
							if (MAX_LEP == 1) LEP_str = "+";
							else LEP_str = "-";
							
							sprintf(textFileBuffer, "%5.0f;%05d;%05d;%03d;%s;%4.0f;%3.0f;%3.0f;%.1d;%s;%s;%.2d;%.2d:%.2d:%.2d\r\n", 
																			R, Qmax, Q_massa, Mz,
																			LEP_str,
																			angle_gamma_rama,
																			angle_betta - params.angle_betta_setup,
																			angle_alpha - params.angle_alpha_setup,
																			protivoves,
																			mode_str,
																			limitations_str,
																			params.flag_TG,
																			ds3231_hour, ds3231_min, ds3231_sec);
							
							if (status_RTC == SUCCESS)
								sprintf(date_txt, "0:/%02d_%02d_%02d.csv", ds3231_day, ds3231_month, (ds3231_year-2000));
							else if (status_RTC == ERROR) {
								f_mount(0,&fs);
								f_opendir(&dir, "/");
								for(;;) { //цикл
									res = f_readdir(&dir, &fileInfo); //считываем информацию о файле или директории
									if((res != FR_OK) || (fileInfo.fname[0] == '\0')) { //если закончились файлы и дир, выходим из цикла
										break;
									}

									if(fileInfo.fattrib & AM_DIR) { //если это папка то что-то делаем

									} else { //если это файл то
										if(strstr(fileInfo.fname, "ERR") != NULL){
											file_err_num = atoi(&fileInfo.fname[4]);
											if (file_err_num > file_err_max_num){
												file_err_max_num = file_err_num; 
											}
										}
									}
								}
								file_err_max_num += 1;
								sprintf(date_txt, "0:/ERR_%04d.csv", file_err_max_num);
								status_RTC = 3;
							}

							res = f_mount(0,&fs);
							
							if (res != FR_OK) {
								return;
							}
								
							res = f_getfree("/", &fre_clust, &fs_ptr); //запрашиваем свободное место

							if(res != FR_OK) { //если нет ответа от карты, то выходим из функции
								return;
							}
							
							tot_sect = (fs.n_fatent - 2) * fs.csize;
							fre_sect = fre_clust * fs.csize;

							avalible_SD = fre_sect/2;
							total_SD = tot_sect/2; //не очень верный расчет, нужно кол-во секторов (30334976) умножить на размер одного сектора (512)
							
							if (avalible_SD < 200000){ //если свободное место меньше определенного количества Kb
								file_date = 4000000000; //огромное значение даты, для сравнения и поиска наименьшего значения
								
								
								res = f_opendir(&dir, "/"); //открываем текущую директорию 
								if(res != FR_OK) {
									return;
								}

								
								for(;;) { //цикл
									res = f_readdir(&dir, &fileInfo); //считываем информацию о файле или директории
									if((res != FR_OK) || (fileInfo.fname[0] == '\0')) { //если закончились файлы и дир, выходим из цикла
										break;
									}

									if(fileInfo.fattrib & AM_DIR) { //если это папка то что-то делаем

									} else { //если это файл то
										if (fileInfo.fdate < file_date){ //сравниваем дату файлов и выбираем самый старый
											file_date = fileInfo.fdate; 
											strcpy(file_name, fileInfo.fname); //копируем имя наиболее старого файла в массив
										}
									}
								}
								
								f_unlink(file_name); //удаляем самый старый файл
							}
					

							res = f_open( &fsrc , date_txt , FA_CREATE_NEW | FA_WRITE);
												
							if ( res == FR_OK )
								{
									f_write(&fsrc, zagolovok, (sizeof(zagolovok)-1), &br);
									f_close(&fsrc);
								}
							else if ( res == FR_EXIST )
								{
									f_open( &fsrc , date_txt , FA_OPEN_EXISTING | FA_WRITE);
															res = f_lseek(&fsrc, f_size(&fsrc));
									res = f_write(&fsrc, textFileBuffer, sizeof(textFileBuffer), &br);
									f_close(&fsrc);
			 
								}
						}			
					
	//			}
			}		
			
		}
	}
}


static void Init_sound_PWM(void){
	GPIO_InitTypeDef  GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//задаём тактовую частоту порта
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //Включаем режим выхода
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
	GPIO_InitStructure.GPIO_PuPd= GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure); //вызов функции инициализации
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_2);

  TimerPeriod = (48000000 / 2000 ) - 1;
  ChannelPulse = (uint16_t) ((float) 0.5 * (TimerPeriod - 1));

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
  
  /* Time Base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = ChannelPulse;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

  TIM_OC2Init(TIM2, &TIM_OCInitStructure);
 // TIM2->CCR2 = ChannelPulse; //тут менять скважность ШИМ от 0 до 0xFFFF
  /* TIM1 counter enable */
  TIM_Cmd(TIM2, ENABLE);
}

void Init_heater(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//задаём тактовую частоту порта
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //Включаем режим выхода
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
	GPIO_InitStructure.GPIO_PuPd= GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure); //вызов функции инициализации
	
	heated_off();
}



void init_TIM3 (void){
	
		TIM_TimeBaseInitTypeDef TIMER_InitStructure;
 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
 
    TIM_TimeBaseStructInit(&TIMER_InitStructure);
    TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIMER_InitStructure.TIM_Prescaler = 48000-1;
    TIMER_InitStructure.TIM_Period = 10-1; //10 ms
    TIM_TimeBaseInit(TIM3, &TIMER_InitStructure);
	
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
		
		TIM_Cmd(TIM3, ENABLE);
	
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
}



void init_TIM6(void){
		TIM_TimeBaseInitTypeDef TIMER_InitStructure;
		
 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
 
    TIM_TimeBaseStructInit(&TIMER_InitStructure);
    TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIMER_InitStructure.TIM_Prescaler = 48000-1; //1000 раз в секунду
    TIMER_InitStructure.TIM_Period = 500-1; //500 ms
    TIM_TimeBaseInit(TIM6, &TIMER_InitStructure);
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
		
		TIM_Cmd(TIM6, ENABLE);
	
		NVIC_InitStructure.NVIC_IRQChannel =  TIM6_DAC_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);	
}



void init_TIM16(void){ //таймер для мкс задержек дисплея, инициализируется еще в бутлоадере, тут не обязательно
		RCC->APB2ENR |= RCC_APB2ENR_TIM16EN; //тактируем таймер
		TIM16->CR1 |= TIM_CR1_CEN; //включаем 
}



void program_work_minutes(void){
	if(GetGTimer(TIMER_work_minutes) >= time_write_work_minutes * minute_timer)
	{	 				
		working_minutes += time_write_work_minutes;
		Flash_Write_EepromEmulation(working_minutes);		
		
		ResetGTimer(TIMER_work_minutes);
	}		
}




void TIM3_IRQHandler(void){	
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		ProcessTimers();
	}
}

void program_CAN_Send	(void){
	if (status_SD == ERROR && params.flag_memory_card == 0){
		CAN_MESSAGE = 0xFF; CAN_MESSAGE_1 = 0xFF;
	}
	
	if(GetGTimer(TIMER_CAN_Send) >= ms_100)
	{	
		CAN_Send(); //посылка флагов в КАН
		
		ResetGTimer(TIMER_CAN_Send);
	}		
}

void program_USB_Transmit	(void){
	if(GetGTimer(TIMER_USB_Transmit) >= ms_100)
	{	
		if(!copy_data_to_SD)
			CDC_Transmit_FS((uint8_t*)USB_TX, sizeof(USB_TX));
		
		ResetGTimer(TIMER_USB_Transmit);
	}		
}	

void program_timeout_SD_USB_Disp	(void){
	if (SD_USB_Disp_in ==1){ //Началось чтение блока Карты
		if(GetGTimer(TIMER_timeout_SD_USB_Disp) >= sec)
		{
			SD_USB_Disp_in = 0;
		}	
	}

	if(GetGTimer(TIMER_BAN_REC_SD) >= 2*sec){ //таймаут запрета записи на Карту памяти
		BAN_REC_SD = 0; 
	}	
}

void program_CAN_timeout_CRC	(void){ //определения датчиков на линии CAN
	if(GetGTimer(TIMER_CAN_timeout_DUG50) > sec){			
		CRC_DUG50_CAN = 0;
		ResetGTimer(TIMER_CAN_timeout_DUG50);
	}
	
	if(GetGTimer(TIMER_CAN_timeout_DUG51) > sec){			
		CRC_DUG51_CAN = 0;
		ResetGTimer(TIMER_CAN_timeout_DUG51);
	}
	
	if(GetGTimer(TIMER_CAN_timeout_LEP) > sec){			
		CRC_LEP_CAN = 0;
		ResetGTimer(TIMER_CAN_timeout_LEP);
	}
		
	if(GetGTimer(TIMER_CAN_timeout_JOY) > sec){			
		CRC_JOY_CAN = 0;
		ResetGTimer(TIMER_CAN_timeout_JOY);
	}
}

 
void TIM6_DAC_IRQHandler(void){
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
		if (beep_off == 0){
			if (beep_flag == 1){		
				if ((GPIOC->ODR & GPIO_Pin_14) != 0x00u)
					GPIOC->BRR = GPIO_Pin_14;
				else
					GPIOC->BSRR = GPIO_Pin_14;		
			}
			else {
				GPIOC->BRR = GPIO_Pin_14;
			}
		}
		else GPIOC->BRR = GPIO_Pin_14;
	}		
}


void Remap_Table(void){
 // Copy interrupt vector table to the RAM.
 volatile uint32_t *VectorTable = (volatile uint32_t *)0x20000000;
 uint32_t ui32_VectorIndex = 0;
 for(ui32_VectorIndex = 0; ui32_VectorIndex < 48; ui32_VectorIndex++)
 {
   VectorTable[ui32_VectorIndex] = *(__IO uint32_t*)((uint32_t)MAIN_PROGRAM_START_ADDRESS + (ui32_VectorIndex << 2));
 }
 //  Enable SYSCFG peripheral clock
 RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, ENABLE);
 // Remap RAM into 0x0000 0000
 SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
}

void check_memory_empty_cells (void){
	Flash_ReadParams();
	if (isnan(params.angle_alpha_setup) == 1) 	{params.angle_alpha_setup = 0; check_flash_params = 1;}
	if (isnan(params.angle_betta_setup) == 1) 	{params.angle_betta_setup = 0; check_flash_params = 1;}
	if (isnan(params.angle_gamma_setup) == 1) 	{params.angle_gamma_setup = 0; check_flash_params = 1;}
	if (params.angle_strely_min == 0xFFFFFFFF) 	{params.angle_strely_min = 17; check_flash_params = 1;}
	if (params.cnt_lep == (int8_t)0xFF) 		{params.cnt_lep = 0; check_flash_params = 1;}
	if (params.flag_TG == 0xFF) 						{params.flag_TG = TG_12_5m; check_flash_params = 1;}
	if (params.k_TG == 0xFF) 								{params.k_TG = k_TG12; check_flash_params = 1;}
	if (params.F0 == (int16_t)0xFFFF) 			{params.F0 = 0; check_flash_params = 1;}
	if (params.K == (int16_t)0xFFFF) 				{params.K = 0; check_flash_params = 1;}
	if (params.AZK_N == 0xFFFFFFFF) 				{params.AZK_N = 0; check_flash_params = 1;}
	if (params.max_massa == 0xFFFF) 				{params.max_massa = 18000; check_flash_params = 1;}
	if (params.L == 0xFFFF) 								{params.L = L_strely_TG12_5m; check_flash_params = 1;}
	if (params.L_og == 0xFFFF) 							{params.L_og = L_og_TG12; check_flash_params = 1;}
	if (params.Hsh == 0xFFFF) 							{params.Hsh = Hsh_TG12; check_flash_params = 1;}
	if (params.Lsh == 0xFFFF) 							{params.Lsh = Lsh_TG12; check_flash_params = 1;}
	if (params.flag_memory_card == 0xFF)		{params.flag_memory_card = 0; check_flash_params = 1;}	
	if (check_flash_params == 1)	FLASH_WriteSettings();

	check_flash_params = 0;
	FLASH_Read_erasable_parameters();
	if (erasable_params.flag_device_first_turn_on == 0xFFFFFFFF)		{erasable_params.flag_device_first_turn_on = 0; check_flash_params = 1;}
	if (check_flash_params == 1) FLASH_Write_erasable_parameters();
	
//Проверка памяти на свободный адрес для записи моточасов
//==============================================================================================================	
	adr_last_record	= flash_search_adress(Start_Search_Memory, 4); //проверяем всю память с 32 страницы, и находим по какому адресу начинать писать
	if (adr_last_record == 0) { //память закончилась, нужно снова затирать всю память
		working_minutes =  *(__IO uint32_t*)(ENDMEMORY-8); //сохраняем моточасы из флэша в переменную
		if (working_minutes == 0xFFFFFFFF) working_minutes = 0;
		FLASH_Unlock();                                                              
		FLASH_ErasePage(Start_Search_Memory); //стираем нужные страницы
		FLASH_ProgramWord((uint32_t)Start_Search_Memory, working_minutes); //записываем в начало памяти моточасы
		FLASH_Lock();
		adr_last_record	= Start_Search_Memory; //запоминаем адрес по которому лежат моточасы
	}
	else {
		working_minutes =  *(__IO uint32_t*)(adr_last_record-4); //в предыдущем адресе хранятся ранее сохраненные моточасы
    if (working_minutes == 0xFFFFFFFF) working_minutes = 0;
	}
//==============================================================================================================
}

void temperature_reading(void){
//	while((DMA_GetFlagStatus(DMA1_FLAG_TC1)) == RESET ); //ожидаем флаг конца передачи	
//	DMA_ClearFlag(DMA1_FLAG_TC1); /* Clear DMA TC flag */
//	//измеренное напряжение от датчика Т
//	DT_ADC = (float)ADCBuffer[0] * 0.0008; //Опорное напряжение делим на разрядность 3.288/4096 = 0.0008
//  DT_C = (DT_ADC - 0.5) / 0.01; //Температура в цельсиях

	if(GetGTimer(TIMER_read_temperature_ds3231) >= sec){
		DT_C = (float)(DS3231registers.temp_msb<<2 | DS3231registers.temp_lsb>>6)*0.25;
		
		ds3231_write(0x0E, 0x20);	
		ResetGTimer(TIMER_read_temperature_ds3231);
	}

	if (flag_test == 0){
		//Включение подогрева
		if (DT_C < T_on) {
			heated_on();}
		if (DT_C > T_off){
			heated_off();}	
	}
	else heated_on();
}

void Mass_calculation(void){
	Q_massa_temp = params.K*(USILIE_CAN-params.F0);
		 
	if (CRC_JOY_CAN == 1){
		if ((ATT_LEFT == 1 && ATT_REV == 1) || (ATT_RIGHT == 1 && ATT_FWD == 1)) Q_massa_temp = Q_massa_temp;
		else if (ATT_LEFT == 1 || ATT_FWD == 1) Q_massa_temp /= params.KPD_polispast; //если Стрела опускается или Крюк опускается
		else  if (ATT_RIGHT == 1 || ATT_REV == 1) Q_massa_temp *= params.KPD_polispast; //если Стрела поднимается или Крюк поднимается
		else Q_massa_temp = Q_massa_temp;
	}
	else Q_massa_temp = Q_massa_temp;

	if (Q_massa_temp < 0) Q_massa = 0;
	else Q_massa = Q_massa_temp;
}





void m_init_dunamic(void){//ф. приготавливает данные для редактирования предустановок 
	init_dunamic_func(Menu_GetCurrentMenu()->SelectCallback);
}

float edit_var(float var, int32_t Low, int32_t Hi, float incValue_PRESS, float incValue_HOLD){
	float  Var = var;	

	if (PRESS_Button == BUTTON_UP) {
		Var	+=	incValue_PRESS;
		PRESS_Button = 0;
	}
		
	if (PRESS_Button == BUTTON_DOWN) {
		Var	-=	incValue_PRESS;
		PRESS_Button = 0;
	}
		
	if (REPEAT_Button == BUTTON_UP) {
		Var	+=	incValue_HOLD;
		REPEAT_Button = 0;
	}
		
	if (REPEAT_Button == BUTTON_DOWN) {
		Var	-=	incValue_HOLD;
		REPEAT_Button = 0;
	}
	
	if (Var<Low) Var=Low;
	if (Var>Hi)  Var=Hi;
			
	return Var;
}

void init_dunamic_func(void (*dunamic_f)(void)){
	dunamic_data_print=dunamic_f;//присвоить указателю на функцию адрес нужной функции
}


void dunamic_f_sample(void){
	if (list == 0)
	{
		lcd44780_SetLCDPosition(15, 0);
		if (CRC_LEP_CAN == 1)
		lcd44780_ShowStr("+");
		else
		lcd44780_ShowStr("-");
		
		lcd44780_SetLCDPosition(15, 1);
		if (CRC_LEP_CAN == 1 && CRC_USILIE_CAN == 1)
		lcd44780_ShowStr("+");
		else if (CRC_LEP_CAN == 0 || CRC_USILIE_CAN == 0)
		lcd44780_ShowStr("-");
	}
	
	if (list == 1)
	{		
		lcd44780_SetLCDPosition(15, 0);
		if (CRC_LEP_CAN == 1 && CRC_USILIE_CAN == 1)
		lcd44780_ShowStr("+");
		else if (CRC_LEP_CAN == 0 || CRC_USILIE_CAN == 0)
		lcd44780_ShowStr("-");
		
		lcd44780_SetLCDPosition(15, 1);
		if (CRC_DUG50_CAN)
		lcd44780_ShowStr("+");
		else
		lcd44780_ShowStr("-");
	}
	
	if (list == 2)
	{		
		lcd44780_SetLCDPosition(15, 0);
		if (CRC_DUG50_CAN)
		lcd44780_ShowStr("+");
		else
		lcd44780_ShowStr("-");
		
		lcd44780_SetLCDPosition(15, 1);
		if (CRC_DUG51_CAN)
		lcd44780_ShowStr("+");
		else
		lcd44780_ShowStr("-");
	}
}

void dunamic_f_TS_KOLONNA(void){
	if (list == 0)
	{	
		lcd44780_SetLCDPosition(15, 0);
		if (TS_strely == 1)
		lcd44780_ShowStr("-");
		else
		lcd44780_ShowStr("+");
		
		lcd44780_SetLCDPosition(15, 1);
		if (mode == 117)
		lcd44780_ShowStr("+");
		else if (mode == 140)
		lcd44780_ShowStr("-");
	}
	if (list == 1)
	{	
		lcd44780_SetLCDPosition(15, 0);
		if (mode == 117)
		lcd44780_ShowStr("+");
		else if (mode == 140)
		lcd44780_ShowStr("-");
	}	
}

void enter_f_KOLONNA(void){
	if (PRESS_Button == BUTTON_ENTER) {
		if (mode == 140)	{mode = 117;}
		else							{mode = 140;}
	}	
}

void enter_f_TS(void){
	if (PRESS_Button == BUTTON_ENTER) {
		if (TS_strely == 1){
			TS_strely = 0; TS_kruka = 0;
		}
		else{
			TS_strely = 1; TS_kruka = 1;
		}
	}	
}
uint8_t LEP_range_selection;
void enter_f_LEP(void){
	LEP_range_selection = params.cnt_lep;
}

void save_range_LEP(void){
	params.cnt_lep = LEP_range_selection;
	save_params();
}

static void m_init_f_sample(void){
	init_dunamic_func(Menu_GetCurrentMenu()->SelectCallback);	
}


void simple_item_print(uint8_t pos, const char *s){
	lcd44780_SetLCDPosition(0, pos);
	lcd44780_ShowStr((char *)s);
}

static void m_print_simple(void){
	menu_set_print_item_callback(simple_item_print);	// menu_item_print=item_skroll;
}


//---Главное меню, рисует только пункты(без параметров)------------
void item_skroll(uint8_t pos, const char *s){

	if (pos==menu_get_pos())
		{
		if (str_number == 0)
		{	
			lcd44780_SetLCDPosition(0, 0);
			lcd44780_ShowStr(">");
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr(" ");
		}
		if (str_number == 1)
		{
			
			lcd44780_SetLCDPosition(0, 0);
			lcd44780_ShowStr(" ");
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr(">");
		}
	}
	lcd44780_SetLCDPosition(1, pos);
	lcd44780_ShowStr((char *)s);

}



static void m_print_skroll(void){
	menu_set_print_item_callback(item_skroll);
}

void Generic_Write_Menu(void){
	menu_make_screen(menu_get_ptrGeneric());
}

void Setup_LEP_sample(void){
	LEP_range_selection = edit_var(LEP_range_selection, 0, 4, 1 , 1);
	
	switch (LEP_range_selection)
	{
		case 0:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("   220В - 1кВ   ");
			break;
		case 1:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("    1кВ - 20кВ   ");
			break;			
		case 2:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("   35кВ - 110кВ    ");
			break;
		case 3:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("  150кВ - 220кВ    ");
			break;
		case 4:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("  330кВ - 750кВ    ");
			break;			
	}
}

void Crane_selection(void){
	if(erasable_params.flag_device_first_turn_on == 0)
		params.flag_TG = edit_var(params.flag_TG, 0, 10, 1 , 1);
	
	switch (params.flag_TG){
		case TG_12_5m:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("   ТГ-12 (5м)   ");		
		
			params.k_TG = k_TG12;
			params.max_massa = max_massa_TG12;
			params.angle_strely_min = 17;
			params.L = L_strely_TG12_5m;
			params.L_og = L_og_TG12;
			params.Hsh = Hsh_TG12;
			params.Lsh = Lsh_TG12;
			params.KPD_polispast = KPD_polispast_4;
		break;
		
		case TG_12_6m:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("   ТГ-12 (6м)   ");		
		
			params.k_TG = k_TG12;
			params.max_massa = max_massa_TG12;
			params.angle_strely_min = 17;
			params.L = L_strely_TG12_6m;
			params.L_og = L_og_TG12;
			params.Hsh = Hsh_TG12;
			params.Lsh = Lsh_TG12;
			params.KPD_polispast = KPD_polispast_4;
		break;

		case TG_12_7m:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("   ТГ-12 (7м)   ");		
		
			params.k_TG = k_TG12;
			params.max_massa = max_massa_TG12;
			params.angle_strely_min = 17;
			params.L = L_strely_TG12_7m;
			params.L_og = L_og_TG12;
			params.Hsh = Hsh_TG12;
			params.Lsh = Lsh_TG12;
			params.KPD_polispast = KPD_polispast_4;
		break;

		case TG_12_9m:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("   ТГ-12 (9м)   ");		
		
			params.k_TG = k_TG12;
			params.max_massa = max_massa_TG12;
			params.angle_strely_min = 17;
			params.L = L_strely_TG12_9m;
			params.L_og = L_og_TG12;
			params.Hsh = Hsh_TG12;
			params.Lsh = Lsh_TG12;
			params.KPD_polispast = KPD_polispast_4;
		break;
		
		case TG_20_7m:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("   ТГ-20 (7м)   ");		

			params.k_TG = k_TG20;
			params.max_massa = max_massa_TG20;
			params.angle_strely_min = 17;
			params.L = 7000;
			params.L_og = L_og_TG20;
			params.Hsh = Hsh_TG20;
			params.Lsh = Lsh_TG20;
			params.KPD_polispast = KPD_polispast_6;
		break;
		
		case TG_20_9m:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("   ТГ-20 (9м)   ");		

			params.k_TG = k_TG20;
			params.max_massa = max_massa_TG20;
			params.angle_strely_min = 17;
			params.L = L_strely_TG20_9m;
			params.L_og = L_og_TG20;
			params.Hsh = Hsh_TG20;
			params.Lsh = Lsh_TG20;
			params.KPD_polispast = KPD_polispast_6;
		break;
		
		case TG_35_10m:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("   ТГ-35 (10м)  ");		

			params.k_TG = k_TG35;
			params.max_massa = max_massa_TG35;
			params.angle_strely_min = 17;
			params.L = L_strely_TG35_10m;
			params.L_og = L_og_TG35;
			params.Hsh = Hsh_TG35;
			params.Lsh = Lsh_TG35;
			params.KPD_polispast = KPD_polispast_6;
		break;
		
		case TG_35_8m6:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("   ТГ-35 (8,6м) ");		

			params.k_TG = k_TG35;
			params.max_massa = max_massa_TG35;
			params.angle_strely_min = 17;
			params.L = L_strely_TG35_8m;
			params.L_og = L_og_TG35;
			params.Hsh = Hsh_TG35;
			params.Lsh = Lsh_TG35;
			params.KPD_polispast = KPD_polispast_6;
		break;
		
		case TG_35_strela_opora:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("ТГ-35(Стрела-оп)");		

			params.k_TG = k_TG35;
			params.max_massa = max_massa_TG35;
			params.angle_strely_min = 48;
			params.L = L_strely_TG35_8m;
			params.L_og = L_og_TG35;
			params.Hsh = Hsh_TG35;
			params.Lsh = Lsh_TG35;
			params.KPD_polispast = KPD_polispast_6;	
		break;
		
		case TG_50_8m6:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("   ТГ-50        ");

			params.k_TG = k_TG50;
			params.max_massa = max_massa_TG50;
			params.angle_strely_min = 17;
			params.L = 8600;
			params.L_og = 560;
			params.Hsh = 899;
			params.Lsh = 690;
			params.KPD_polispast = KPD_polispast_8;
		break;

		case rezerv:
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("   Резерв       ");

			params.k_TG = k_TG50;
			params.max_massa = max_massa_TG50;
			params.angle_strely_min = 17;
			params.L = 8600;
			params.L_og = 560;
			params.Hsh = 899;
			params.Lsh = 690;
			params.KPD_polispast = KPD_polispast_8;		
		break;			
	}
}

extern void (*Enter_edit)(void);
void read_flash(void){
	Flash_ReadParams();
	Enter_edit = NULL;
}

void TIME_PRINT (void){		
	lcd44780_SetLCDPosition(0, 0);
	lcd44780_ShowStr("Дата  ");
	sprintf(io_buff, "%.2d.", ds3231_day);
	lcd44780_ShowStr(io_buff);
	sprintf(io_buff, "%.2d.", ds3231_month);
	lcd44780_ShowStr(io_buff);
	sprintf(io_buff, "%.4d", ds3231_year);
	lcd44780_ShowStr(io_buff);
	
	lcd44780_SetLCDPosition(0, 1);
	lcd44780_ShowStr("Время  ");
	sprintf(io_buff, "%.2d:", ds3231_hour);
	lcd44780_ShowStr(io_buff);
	sprintf(io_buff, "%.2d:", ds3231_min);
	lcd44780_ShowStr(io_buff);
	sprintf(io_buff, "%.2d  ", ds3231_sec);
	lcd44780_ShowStr(io_buff);
}

char string_lcd[] = "                ";
uint8_t state_scroll;
char *SD_strings = "Продолжить без карты памяти?";
char *SD_strings_1 = "Работа без карты памяти";

uint16_t len_str  = 0;

void gorizontal_scr(char *strings, char x, char y);

uint8_t state_string_SD;
void gorizontal_scroll(void){
	
	lcd44780_SetLCDPosition(0, 0);
	lcd44780_ShowStr("Карта памяти   ");
	if (_card_insert() == 0) {
		lcd44780_ShowStr("+");
		
		lcd44780_SetLCDPosition(0,1);
		lcd44780_ShowStr("                ");
		
		state_scroll = 0;		
	}
	else {
		lcd44780_ShowStr("-");
	
		switch(state_string_SD){
			case 0:	
				if (params.flag_memory_card == 1){
					state_string_SD = 2;
					break;
				}
				gorizontal_scr(SD_strings, 0, 1);
			break;
			case 1:
				params.flag_memory_card = 1;
				FLASH_WriteSettings();
				lcd44780_SetLCDPosition(0,1);
				lcd44780_ShowStr("      ОК!       ");
				if (GetGTimer(TIMER_string_SD)> 2 * sec){
					state_string_SD = 2;
				}
			break;
			case 2:
				gorizontal_scr(SD_strings_1, 0, 1);
			break;		
		}
	}
}

void SD_card_enter_f (void){
	if (_card_insert() != 0) {
		if (state_string_SD == 0) {
			ResetGTimer(TIMER_string_SD);
			state_string_SD = 1;
			state_scroll = 0;
		}
	}
}
char *temp_strings = "                                                                                           ";

void gorizontal_scr(char *strings, char x, char y){
	switch(state_scroll){
		case 0:
			temp_strings = strings;                                  
		
			for (uint8_t i=0 ; i < 16 ; i++){
				string_lcd[i] = *temp_strings;
				temp_strings++;
			}
			lcd44780_SetLCDPosition(x, y);
			lcd44780_ShowStr(string_lcd);
			
			state_scroll = 1;
			
			ResetGTimer(TIMER_scroll_str);
			StartGTimer(TIMER_scroll_str);
		break;
		case 1:	
			if (GetGTimer(TIMER_scroll_str) >= sec){
				ResetGTimer(TIMER_scroll_str);
				
				state_scroll = 2;
				StartGTimer(TIMER_scroll_str);	
			}
		break;	
		case 2:
			if (GetGTimer(TIMER_scroll_str) >= ms_200){
				ResetGTimer(TIMER_scroll_str);
								
				for (uint8_t i=0 ; i < 15 ; i++) 
					string_lcd[i] = string_lcd[i+1] ; // Производим побайтовый сдвиг строки влево		
				string_lcd[15] = *temp_strings;
				
				temp_strings++ ;		
				len_str++ ;
				
				lcd44780_SetLCDPosition(x, y);
				lcd44780_ShowStr(string_lcd);
				
				if (*temp_strings == 0) state_scroll = 3;
				StartGTimer(TIMER_scroll_str);
			}
		break;	
		case 3:
			if (GetGTimer(TIMER_scroll_str) >= sec){
				ResetGTimer(TIMER_scroll_str);
				
				temp_strings -= 16;
				for (uint16_t n = 0 ; n < len_str ; n++) temp_strings--;
				
				state_scroll = 0;
				StartGTimer(TIMER_scroll_str);
			}
		break;
	}
}

void SD_card_init_f(void){
	state_scroll = 0;
}

void Settings_init_f(void){
	flag_lock_block = 0;
}

#include "make_menu.h"

Menu_Item_t* main_CurrentMenu = &NULL_MENU;
Menu_Item_t* main_HidedMenu1 = &Menu_pass_entry1;
Menu_Item_t* main_HidedMenu2 = &Menu_pass_entry2;

void TEST (void){
	uint8_t l = 0;
	if (flag_test == 0) {
		while (l <= 30){
			led_status[l] = 1;
			l++;
		}
		Led_show();
	}
	else if (flag_test == 1) {
		while (l <= 30){
			led_status[l] = 0;
			l++;
		}
		Led_show();			
	}
	HOLD_Button = 0;
}

void Edit_Time (void){
	Vise_Versa_Menu(&Menu_Settings_Time);
	NextParam (7, save_date, &Menu_Time_Entry, 1);
}

void Edit_Number_KBK (void){
	Vise_Versa_Menu(&Menu_Settings_Service);
	NextParam (7, save_params, &Menu_Service_Entry, 1);
}

void save_Calib (void){
	Vise_Versa_Menu(&Menu_Settings_Calib_Q);
	if (nParam == 1) params.F0 = USILIE_CAN;
	else if (nParam == 2) params.K = Qf2/(USILIE_CAN-params.F0);
	NextParam (2, save_params, &Menu_Calib_Q_Entry, 1);
}


void Check_password(void){
	Vise_Versa_Menu(&Menu_1_4);
	if ((pass[0]*1000+pass[1]*100+pass[2]*10+pass[3]) == password) 
	{
		flag_lock_block = 1;
		Set_Hided_Menu(1);
		Back_Through_Pass(1);
	}
	else if ((pass[0]*1000+pass[1]*100+pass[2]*10+pass[3]) == password2) 
	{
		Set_Hided_Menu(2);
		Back_Through_Pass(1);
	}
	else 
	{
		Back_Through_Pass(0);
	}
	
	m_number++;
	if (m_number > 3) m_number = 0;
}

void Read_sensor_LEP(void){
	lcd44780_SetLCDPosition(0, 0);
	sprintf(io_buff, "Датчик ЛЭП:  %3d", LEP_CAN);
	lcd44780_ShowStr(io_buff);
	lcd44780_SetLCDPosition(0, 1);
	if (KONCEVIK_LEP == 1)
		lcd44780_ShowStr("Конц.:   замкнут");
	else if (KONCEVIK_LEP == 0) lcd44780_ShowStr("Конц.:не замкнут");	
}

void Read_sensor_USILIE(void){
	lcd44780_SetLCDPosition(0, 0);
	sprintf(io_buff, "Усилие:  F=%5d", USILIE_CAN);
	lcd44780_ShowStr(io_buff);
	lcd44780_SetLCDPosition(0, 1);
	lcd44780_ShowStr("                ");
}

void Read_sensor_DUG1(void){
	lcd44780_SetLCDPosition(0, 0);
	sprintf(io_buff, "ДУГ1:     =%3d", angle_gamma_CAN);
	lcd44780_ShowStr(io_buff);
	lcd44780_SetLCDPosition(0, 1);
	if (KONCEVIK_DUG_50 == 1)
		lcd44780_ShowStr("Конц.:   замкнут");
	else lcd44780_ShowStr("Конц.:не замкнут");	
}

void Read_sensor_DUG2(void){
	lcd44780_SetLCDPosition(0, 0);
	if (KONCEVIK_DUG_51 == 1)
		lcd44780_ShowStr("ДУГ2:    замкнут");
	else lcd44780_ShowStr("ДУГ2: не замкнут");
	lcd44780_SetLCDPosition(0, 1);
	sprintf(io_buff, "=%3d =%3d        ", angle_alpha_CAN, angle_betta_CAN);
	lcd44780_ShowStr(io_buff);	
}



void entry_password(void){
	
	pass[m_number] = edit_var(pass[m_number], 0, 9, 1, 1);
		
	if (m_number == 0){
	lcd44780_SetLCDPosition(0, 1);	
	lcd44780_ShowStr("     ");
	lcd44780_SetLCDPosition(6, 1);	
	lcd44780_ShowStr("***        ");	
	}
	else if (m_number == 1){
		lcd44780_SetLCDPosition(0, 1);	
		lcd44780_ShowStr("     *");
		lcd44780_SetLCDPosition(7, 1);	
		lcd44780_ShowStr("**        ");	
	}
	else if (m_number == 2){
		lcd44780_SetLCDPosition(0, 1);	
		lcd44780_ShowStr("     **");
		lcd44780_SetLCDPosition(8, 1);	
		lcd44780_ShowStr("*        ");	
	}
	else if (m_number == 3){
		lcd44780_SetLCDPosition(0, 1);	
		lcd44780_ShowStr("     ***");
		lcd44780_SetLCDPosition(9, 1);	
		lcd44780_ShowStr("        ");	
	}		
	
		
	lcd44780_SetLCDPosition(m_number+5, 1);
	
	blink_symbol();
	
	if (blinkflag) lcd44780_ShowStr(" "); 
	else {
		sprintf(io_buff, "%.1d", pass[m_number]);
		lcd44780_ShowStr(io_buff);		
	}
}

void save_params(void){
	lcd44780_SetLCDPosition(0, 0);
	lcd44780_ShowStr("   Сохранение   ");
	lcd44780_SetLCDPosition(0, 1);
	lcd44780_ShowStr("   параметров   ");
  FLASH_WriteSettings();
	delay_ms(1000);
}

void save_date(void){
	lcd44780_SetLCDPosition(0, 0);
	lcd44780_ShowStr("   Сохранение   ");
	lcd44780_SetLCDPosition(0, 1);
	lcd44780_ShowStr("      даты      ");
	
	ds3231_write(reg_hour, DEC_to_BCD(ds3231_hour_temp));
	ds3231_write(reg_min, DEC_to_BCD(ds3231_min_temp));
	ds3231_write(reg_sec, DEC_to_BCD(ds3231_sec_temp));

	ds3231_write(reg_day, DEC_to_BCD(ds3231_day_temp));
	ds3231_write(reg_month, DEC_to_BCD(ds3231_month_temp));
	ds3231_write(reg_year, DEC_to_BCD(ds3231_year_temp - 2000));
	
	delay_ms(1000);
}

static void CRC_Config(uint8_t poly){
  /* Enable CRC AHB clock interface */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);

  /* DeInit CRC peripheral */
  CRC_DeInit();
  
  /* Init the INIT register */
  CRC_SetInitRegister(0);
  
  /* Select 8-bit polynomial size */
  CRC_PolynomialSizeSelect(CRC_PolSize_8);
  
  /* Set the polynomial coefficients */
  CRC_SetPolynomial(poly);
}

static uint8_t CRC_8BitsCompute(uint8_t* data, uint32_t size){
  uint8_t* dataEnd = data+size;

  /* Reset CRC data register to avoid overlap when computing new data stream */
  CRC_ResetDR();

  while(data < dataEnd)
  {
    /* Write the input data in the CRC data register */
    CRC_CalcCRC8bits(*data++);
  }
  /* Return the CRC value */
  return (uint8_t)CRC_GetCRC();
}

void clear_RXBuffer(uint8_t *RX_BUFER, uint16_t size){
   uint16_t i = 0;
   for (i = 0; i < size; i++) RX_BUFER[i] = 0;
}

FRESULT flash_to_SD (void){
	res = f_lseek(&fsrc, f_size(&fsrc));
	if(res != FR_OK) {
		return res;
	}	
	uint8_t temp_bufer[13], temp_SD_bufer[52];	
	uint8_t temp_CRC;
	uint8_t old_procent = 0;
	for (uint16_t num_page_FlashToSD = Start_page_data; num_page_FlashToSD <= End_page_data; num_page_FlashToSD ++) //задаем диапазон чтения страниц флэша
	{         
		AT45DB161_Read_Data(num_page_FlashToSD, 0, 528, buf_AT45DB); //считываем страницу в буфер
		
		uint16_t count_byte = 0;
		for (uint16_t num_REC_FlashToSD = 0; num_REC_FlashToSD <= (528/Size_Buff_params_AT45DB); num_REC_FlashToSD ++){ //задаем количество записей на одной странице (528 байт / 13 байт(занимает одна запись))
			uint8_t cnt_temp_buf = 0;
			
			for (uint16_t i = count_byte; i <= count_byte + Size_Buff_params_AT45DB; i ++){ //копируем байты одной записи во временный буфер для анализа
				temp_bufer[cnt_temp_buf] = buf_AT45DB[i];	
				cnt_temp_buf++;						
			}
			count_byte += Size_Buff_params_AT45DB; 
			temp_CRC = CRC_8BitsCompute(temp_bufer, (countof(temp_bufer) - 2));
			if (temp_CRC == temp_bufer[11]) {
				temp_bufer[11] = 0x0D;  //возврат каретки
				temp_bufer[12] = 0x0A;  //перевод каретки на новую строку
				sprintf((char*)temp_SD_bufer, "%03d;%03d;%03d;%03d;%03d;%01d;%01d;%01d;%01d;%01d;%01d;%01d;%01d;%.2d:%.2d:%.2d;%.2d.%.2d\r\n", 
															temp_bufer[AT45DB_R_strely], 
															temp_bufer[AT45DB_Max_massa], 
															temp_bufer[AT45DB_Fact_massa], 
															temp_bufer[AT45DB_M_z],
															temp_bufer[AT45DB_angle_strely],
															((temp_bufer[AT45DB_mode_OffLimit_EnLimit] & 0x80)>>7), //0 - колонна, 1 - одиночный
															((temp_bufer[AT45DB_mode_OffLimit_EnLimit] & 0x40)>>6), //0 - огр. не сняты, 1 - огр. сняты по кнопке
															((temp_bufer[AT45DB_mode_OffLimit_EnLimit] & 0x20)>>5), //0 - Разрешено, 1 - запрет складывания КГ
															((temp_bufer[AT45DB_mode_OffLimit_EnLimit] & 0x10)>>4), //0 - Разрешено, 1 - запрет откидывания КГ
															((temp_bufer[AT45DB_mode_OffLimit_EnLimit] & 0x08)>>3), //0 - Разрешено, 1 - запрет подъема крюка
															((temp_bufer[AT45DB_mode_OffLimit_EnLimit] & 0x04)>>2), //0 - Разрешено, 1 - запрет опускания крюка
															((temp_bufer[AT45DB_mode_OffLimit_EnLimit] & 0x02)>>1), //0 - Разрешено, 1 - запрет подъема стрелы
															(temp_bufer[AT45DB_mode_OffLimit_EnLimit] & 0x01), 		//0 - Разрешено, 1 - запрет опускания стрелы					
															temp_bufer[AT45DB_time_hour],
															temp_bufer[AT45DB_time_min],
															temp_bufer[AT45DB_time_sec],
															temp_bufer[AT45DB_time_day],
															temp_bufer[AT45DB_time_month]
				);
				
				res = f_write(&fsrc, temp_SD_bufer, countof(temp_SD_bufer), &br);
			}
			else{
				__nop();
			}						
		}	
		uint16_t procent;
		procent = ((num_page_FlashToSD - End_page_NumPage) *100) / (End_page_data - Start_page_data);
		if (old_procent != procent){
			old_procent =  procent;
			sprintf((char*)USB_TX, "%03d%%\r\n", procent);
			CDC_Transmit_FS((uint8_t*)USB_TX, 6);
		}					
	}			
	if(res != FR_OK) {
		return res;
	}
	return FR_OK;
}

uint8_t copy_flash_to_SD (void){
  if (strstr((char*)str_rx, "load_flash")){
		copy_data_to_SD = 1;
		clear_RXBuffer(str_rx, countof(str_rx));
		
		sprintf((char*)USB_TX,"\r\nOK!\r\n");
		CDC_Transmit_FS((uint8_t*)USB_TX, 7);

		res = f_mount(0,&fs);
							
		if (res != FR_OK) {
      return res;
		}
		res = f_open( &fsrc , "FLASH.csv" , FA_CREATE_NEW | FA_WRITE);
		if ( res == FR_OK ){
			
			f_write(&fsrc, zagolovok_flash, countof(zagolovok_flash), &br);
      res = flash_to_SD();
			if(res != FR_OK) {
				return res;
			}
			f_close(&fsrc);
		}
		else if ( res == FR_EXIST ){
			res = f_open( &fsrc , "FLASH.csv" , FA_OPEN_EXISTING | FA_WRITE);
			if(res != FR_OK) {
				return res;
			}
			
      res = flash_to_SD();
			if(res != FR_OK) {
				return res;
			}
			
			f_close(&fsrc);
		}
		USB_TX[0] = 0x24;
	  USB_TX[10] = 0x23;
		copy_data_to_SD = 0;
	}
	return FR_OK;
}
extern uint16_t 	N_page_data, N_adrr_data,
									N_page_NumPage, N_adrr_NumPage;
void clear_flash (void){
	if (strstr((char*)str_rx, "clear_flash")){
		copy_data_to_SD = 1;
		clear_RXBuffer(str_rx, countof(str_rx));
		
		sprintf((char*)USB_TX_Fash,"\r\nOK!\r\n");
		CDC_Transmit_FS((uint8_t*)USB_TX_Fash, 7);	

		AT45DB_ChipErase();
		
		N_page_data = Start_page_data;
		N_adrr_data = 0;
		
		N_page_NumPage = Start_page_NumPage;
		N_adrr_NumPage = 0;
		
		sprintf((char*)USB_TX_Fash,"\r\nClear complete\r\n");
		CDC_Transmit_FS((uint8_t*)USB_TX_Fash, 18);
		
		USB_TX[0] = 0x24;
	  USB_TX[10] = 0x23;		
		copy_data_to_SD = 0;
	}	
}

void program_write_params_to_AT45DB (void){
	if (!BAN_REC_SD){
		if(GetGTimer(TIMER_write_AT45DB) >= ms_100){
			ds3231_read(DS3231_Buf, LENGHT);
			
			if (mode == 140) params_AT45DB[AT45DB_mode_OffLimit_EnLimit] =  (1 << 7);
			else if (mode == 117) params_AT45DB[AT45DB_mode_OffLimit_EnLimit] =  (0 << 7);
		
			params_AT45DB[AT45DB_time_sec] 		= BCD_to_DEC(DS3231_Buf[0]);
			params_AT45DB[AT45DB_time_min] 		= BCD_to_DEC(DS3231_Buf[1]);
			params_AT45DB[AT45DB_time_hour] 	= BCD_to_DEC(DS3231_Buf[2]);
			params_AT45DB[AT45DB_time_day] 		= BCD_to_DEC(DS3231_Buf[4]);
			params_AT45DB[AT45DB_time_month] 	= BCD_to_DEC(DS3231_Buf[5]);
		
			params_AT45DB[AT45DB_angle_strely] = (uint8_t)angle_gamma_rama;
			params_AT45DB[AT45DB_Fact_massa] = Q_massa/1000;		 
			params_AT45DB[AT45DB_Max_massa] = Qmax/1000;
			params_AT45DB[AT45DB_mode_OffLimit_EnLimit] =  (flag_unlocking << 6) | (KG_skladivanie << 5) | (KG_otkidivanie << 4) | 
																						(kruk_up << 3) | (kruk_down << 2) | (strela_up << 1) | strela_down;
			params_AT45DB[AT45DB_M_z] = Mz;
			params_AT45DB[AT45DB_R_strely] = R/100;
				
			params_AT45DB[AT45DB_CRC_params] = CRC_8BitsCompute(params_AT45DB, (countof(params_AT45DB) - 2));
		
			params_AT45DB[AT45DB_NULL_params] = 0;
			
			AT45DB_write_data();

//			AT45DB161_Read_Data(N_page_data, 0, 528, buf1);
//			AT45DB161_Read_Data(N_page_NumPage, 0, 528, buf2);
			ResetGTimer(TIMER_write_AT45DB);
		}
	}	
}
#define delay_check_error 200
void program_check_ERROR(void){
	ds3231_read(DS3231_Buf, LENGHT);
	if(BCD_to_DEC(DS3231_Buf[6]) == 00){
		status_RTC = ERROR;
	}
	else status_RTC = SUCCESS;
	
	if (_card_insert() == 0){
		if(MSD_Init() == 0){//	if (_card_insert() == 0){//if(MSD_Init() == 0){
			status_SD = SUCCESS;
			if (params.flag_memory_card == 1){
				params.flag_memory_card = 0;
				FLASH_WriteSettings();
			}
		}
		else status_SD = ERROR;
	}	
	else status_SD = ERROR;	
	
	CAN_MESSAGE = 0xFF;
	CAN_MESSAGE_1 = 0xFF;
	
//////////////////////////////////////////////////	
	if (status_RTC == ERROR){
		lcd44780_SetLCDPosition(0, 0);
		lcd44780_ShowStr("     Ошибка     ");
		lcd44780_SetLCDPosition(0, 1);
		lcd44780_ShowStr("     часов      ");	
		
		while (GPIO_ReadInputDataBit(SB3_GPIO_PORT, SB3_GPIO_PIN) ==  0) {
			program_CAN_Send();
		}
		delay_ms(delay_check_error);
	}
	if (status_RCC == ERROR){
		lcd44780_SetLCDPosition(0, 0);
		lcd44780_ShowStr("     Ошибка     ");
		lcd44780_SetLCDPosition(0, 1);
		lcd44780_ShowStr("  тактирования  ");	
		
		while (GPIO_ReadInputDataBit(SB3_GPIO_PORT, SB3_GPIO_PIN) ==  0) {
			program_CAN_Send();
		}
		delay_ms(delay_check_error);
	}	
	if (status_SD == ERROR){
		lcd44780_SetLCDPosition(0, 0);
		lcd44780_ShowStr("     Ошибка     ");
		lcd44780_SetLCDPosition(0, 1);
		lcd44780_ShowStr("  карты памяти  ");	
		
		while (GPIO_ReadInputDataBit(SB3_GPIO_PORT, SB3_GPIO_PIN) ==  0) {
			program_CAN_Send();
		}
		delay_ms(delay_check_error);
	}	
	if (status_AT45 == ERROR){
		lcd44780_SetLCDPosition(0, 0);
		lcd44780_ShowStr("     Ошибка     ");
		lcd44780_SetLCDPosition(0, 1);
		lcd44780_ShowStr("  flash памяти  ");	
		
		while (GPIO_ReadInputDataBit(SB3_GPIO_PORT, SB3_GPIO_PIN) ==  0) {
			program_CAN_Send();
		}
		delay_ms(delay_check_error);
	}	
	if (status_CAN_Transsiver == ERROR){
		lcd44780_SetLCDPosition(0, 0);
		lcd44780_ShowStr("     Ошибка     ");
		lcd44780_SetLCDPosition(0, 1);
		lcd44780_ShowStr("    CAN шины    ");	
		
		while (GPIO_ReadInputDataBit(SB3_GPIO_PORT, SB3_GPIO_PIN) ==  0) {}
		delay_ms(delay_check_error);
	}
	else {
		if (CRC_DUG50_CAN == ERROR){
			lcd44780_SetLCDPosition(0, 0);
			lcd44780_ShowStr(" ДУГ на стреле  ");
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("  не подключен  ");	
			
			while (GPIO_ReadInputDataBit(SB3_GPIO_PORT, SB3_GPIO_PIN) ==  0) {
				program_CAN_Send();
			}
			delay_ms(delay_check_error);
		}
		if (CRC_DUG51_CAN == ERROR){
			lcd44780_SetLCDPosition(0, 0);
			lcd44780_ShowStr("  ДУГ на раме   ");
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("  не подключен  ");	
			
			while (GPIO_ReadInputDataBit(SB3_GPIO_PORT, SB3_GPIO_PIN) ==  0) {
				program_CAN_Send();
			}
			delay_ms(delay_check_error);
		}
		if (CRC_LEP_CAN == ERROR){
			lcd44780_SetLCDPosition(0, 0);
			lcd44780_ShowStr("   Датчик ЛЭП   ");
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("  не подключен  ");	
			
			while (GPIO_ReadInputDataBit(SB3_GPIO_PORT, SB3_GPIO_PIN) ==  0) {
				program_CAN_Send();
			}
			delay_ms(delay_check_error);
		}
		if (CRC_USILIE_CAN == ERROR){
			lcd44780_SetLCDPosition(0, 0);
			lcd44780_ShowStr(" Датчик усилия  ");
			lcd44780_SetLCDPosition(0, 1);
			lcd44780_ShowStr("  не подключен  ");	
			
			while (GPIO_ReadInputDataBit(SB3_GPIO_PORT, SB3_GPIO_PIN) ==  0) {
				program_CAN_Send();
			}
			delay_ms(delay_check_error);
		}		
	}
}

void program_sensor_processing(void){
	//====================================ДАТЧИК УГЛА НА СТРЕЛЕ=======================================================
	if (CRC_DUG50_CAN == 0){
		KONCEVIK_DUG_50 = 0;
		angle_gamma_CAN = 0;
		USB_TX[1] = 0xFF; USB_TX[2] = 0xFF;
	}
	else if (CRC_DUG50_CAN == 1){
		USB_TX[1] = (uint8_t)(angle_gamma_CAN); USB_TX[2] = KONCEVIK_DUG_50;
	}
	konc_streli = KONCEVIK_DUG_50;
	angle_gamma = angle_gamma_CAN;
		
	//================================================================================================================

	//====================================ДАТЧИК УГЛА Бетта НА РАМЕ===================================================
	if (CRC_DUG51_CAN == 0){
		angle_betta_CAN = 0;
		angle_alpha_CAN = 0;
		KONCEVIK_DUG_51 = 0;
		USB_TX[3] = 0xFF; USB_TX[4] = 0xFF; USB_TX[5] = 0xFF;
	}
	else if (CRC_DUG51_CAN == 1){
	  USB_TX[3] = (uint8_t)(angle_alpha_CAN_USB); USB_TX[4] = (uint8_t)(angle_betta_CAN_USB); USB_TX[5] = KONCEVIK_DUG_51;
	}
	angle_betta = angle_betta_CAN;
	//================================================================================================================

		
	//====================================ДАТЧИК УГЛА Альфа НА РАМЕ===================================================
	angle_alpha = angle_alpha_CAN;
	//================================================================================================================
	angle_gamma_rama = (angle_gamma + params.angle_gamma_setup) - (angle_betta - params.angle_betta_setup);
	//====================================ДАТЧИК ЛЭП==================================================================
	if (CRC_LEP_CAN == 0){	
		LEP_CAN = 0;
		KONCEVIK_LEP = 0;
		USB_TX[8] = 0xFF; USB_TX[9] = 0xFF;
	}
	else if (CRC_LEP_CAN == 1){
		USB_TX[8] = LEP_CAN; USB_TX[9] = KONCEVIK_LEP;
	}
	konc_kruka = KONCEVIK_LEP;
	//================================================================================================================
	//====================================ДАТЧИК Усилия ДСТ5==================================================================
  if (CRC_LEP_CAN == 0 || CRC_USILIE_CAN == 0){
		USILIE_CAN = 0;
		USB_TX[6] = 0xFF; USB_TX[7] = 0xFF; 		
	}
	else if (CRC_LEP_CAN == 1 && CRC_USILIE_CAN == 1){
		USB_TX[6] = USILIE_CAN_USB_L; USB_TX[7] = USILIE_CAN_USB_H; 
	}
	//=================================================================================================	
	if (CRC_DUG51_CAN == 1){ //если концевик на линии CAN и подключен к ДУГ51
	 if (KONCEVIK_DUG_51 == 0){//сработал коцевик на ДУГ51
			protivoves = 0; //противовес придвинут		
	 }
	 else protivoves = 1; //противовес откинут
	}
	else protivoves = 2; //ДУГ51 не подключен то ошибка
	//=================================================================================================	
	if (protivoves == 1 || params.k_TG == k_TG12){
		a = 1323;
		y0 = -2920;
	}
	else {
		a = 1116;
		y0 = -2919;
	}	
}

void program_calculation_R(void){
	//пересчет угла в радианы для формулы	
	angle_gamma_rad = angle_gamma * 3.14 / 180;
	angle_betta_rad = angle_betta * 3.14 / 180;
	angle_gamma_rad_setup = params.angle_gamma_setup * 3.14 / 180;
	angle_betta_rad_setup = params.angle_betta_setup * 3.14 / 180;
	
	R_temp = (params.L * cos(angle_gamma_rad+angle_gamma_rad_setup)-params.Hsh*sin(angle_betta_rad - angle_betta_rad_setup)+params.Lsh*cos(angle_betta_rad - angle_betta_rad_setup)+params.L_og*sin(angle_gamma_rad+angle_gamma_rad_setup));
	if (R_temp < 1) R = 1; 
	else if (R_temp > 15000) R = 15000;
	else R = R_temp;	
}

void program_calculation_Qmax_and_Mz(void){
	//расчет макс массы и загруженности
	if (params.flag_TG != TG_35_strela_opora)
		Qmax = (a*100000/R+y0)/mode*params.k_TG;
	else Qmax =  45000;
	
	if (Qmax > params.max_massa) Qmax = params.max_massa;
	
	Mz_temp = 100*Q_massa/Qmax;
	if (Mz_temp > 250) Mz = 250;
	else Mz = Mz_temp;	
}

void program_LED_indication(void){
	if(flag_test == 0){
		//Индикация загруженности
		if (Mz > 10) {
			MzMAP = map(Mz, 10, 110, 0, 20); //пересчет Mz в кол-во горящих лампочек
			for (cnt_leds = 0; cnt_leds < 21; cnt_leds++){
				if (cnt_leds <= MzMAP) led_status[cnt_leds] = 1;
				else led_status[cnt_leds] = 0;
			}
		}
		else {
			for (cnt_leds = 0; cnt_leds < 21; cnt_leds++){
				led_status[cnt_leds] = 0;
			}
		}
	 
		//Индикация транспортной скорости стрелы и крюка
		if (TS_kruka == 1 || TS_strely == 1) led_status[LED_Transport_speed] = 0;                                                  
		else  led_status[LED_Transport_speed] = 1;                                                                    

		//РЕЖИМ КОЛОННА
		if (mode == 140) //ВЫКЛ Режим колонна
		{
			led_status[LED_KOLONNA] = 0;
			//beep_flag = 0;
			if (led_status[16] == 1) 	//Индикатор загруженности больше 90%
			{
				led_status[LED_DANGER] = 1;	//Зажигаем индикатор ПРЕДЕЛ
				//beep_flag = 1;
			}
			else led_status[LED_DANGER] = 0;
			
			if (led_status[19] == 1) 	//Индикатор загруженности больше 105% 
			{
				led_status[LED_STOP] = 1; 	//Зажигаем индикатор СТОП
				MAX_Mz = 1;
				//beep_flag = 2;
			}
			else	{
				led_status[LED_STOP] = 0;
				MAX_Mz = 0;
			}
		}
		
		if (mode == 117) //ВКЛ Режим колонна
		{
			led_status[LED_KOLONNA] = 1;
			//beep_flag = 0;
			if (led_status[14] == 1) //Индикатор загруженности больше 80%
			{
				led_status[LED_DANGER] = 1; //Зажигаем индикатор ПРЕДЕЛ
				//beep_flag = 1;
			}
			else led_status[LED_DANGER] = 0;

			
			if (led_status[17] == 1) //Индикатор загруженности больше 95%
			{
				led_status[LED_STOP] = 1; //Зажигаем индикатор СТОП
				MAX_Mz = 1;
				//beep_flag = 2;
			}
			else {
				led_status[LED_STOP] = 0;
				MAX_Mz = 0;
			}
		}
		
		//Диапазон ЛЭП - включение светодиода
		switch (params.cnt_lep){
				case -1: //если во флэш не записали никакие параметры лэп
					if (LEP_CAN > 50) {led_status[LED_LEP] = 1; led_status[LED_STOP] = 1; MAX_LEP = 1;} //ВКЛ индикатор ЛЭП и СТОП
					else {led_status[LED_LEP] = 0;  MAX_LEP = 0;}
					break;			
				case 0:
					if (LEP_CAN > 50) {led_status[LED_LEP] = 1; led_status[LED_STOP] = 1; MAX_LEP = 1;} //ВКЛ индикатор ЛЭП и СТОП
					else {led_status[LED_LEP] = 0;  MAX_LEP = 0;}
					break;
				case 1:
					if (LEP_CAN > 100) {led_status[LED_LEP] = 1; led_status[LED_STOP] = 1; MAX_LEP = 1;}
					else {led_status[LED_LEP] = 0;  MAX_LEP = 0;}
					break;
				case 2:
					if (LEP_CAN > 150) {led_status[LED_LEP] = 1; led_status[LED_STOP] = 1; MAX_LEP = 1;}
					else {led_status[LED_LEP] = 0;  MAX_LEP = 0;}
					break;
				case 3:
					if (LEP_CAN > 200) {led_status[LED_LEP] = 1; led_status[LED_STOP] = 1; MAX_LEP = 1;}
					else {led_status[LED_LEP] = 0;  MAX_LEP = 0;}
					break;
				case 4:
					if (LEP_CAN > 250) {led_status[LED_LEP] = 1; led_status[LED_STOP] = 1; MAX_LEP = 1;} 
					else {led_status[LED_LEP] = 0;  MAX_LEP = 0;}
					break;
		}	

		//индикация крена
		if (angle_betta - params.angle_betta_setup > 5 || angle_betta - params.angle_betta_setup < -5) {led_status[LED_BETTA] = 1; led_status[LED_STOP] = 1; MAX_ROLL = 1;} //отклонение по бетта на 5 град зажигаем светодиод
		else  {led_status[LED_BETTA] = 0; MAX_ROLL = 0;}
		if (angle_alpha - params.angle_alpha_setup > 10 || angle_alpha - params.angle_alpha_setup < -10) {led_status[LED_ALPHA] = 1; led_status[LED_STOP] = 1; MAX_PITCH = 1;} //отклонение по альфа на 10 град зажигаем светодиод
		else  {led_status[LED_ALPHA] = 0;	MAX_PITCH = 0;}		
		
		//индикация предельного подъема крюка и стрелы
		if (konc_kruka == 1) led_status[LED_KRUK] = 1;
		else led_status[LED_KRUK] = 0;
		
		if (konc_streli == 1) led_status[LED_STRELI] = 1;
		else led_status[LED_STRELI] = 0;
		
		//индикация при срабатывании ограничений
		if ((strela_up || strela_down || kruk_up || kruk_down) == 1)  
			led_status[LED_STOP] = 1;
		
		//индикация если любого датчика нет на линии
		if ((CRC_DUG51_CAN && CRC_DUG50_CAN && CRC_LEP_CAN) == 0){ 
			NO_SENSORS = 1;	
			led_status[LED_STOP] = 1;
		}
		else NO_SENSORS = 0;
	}
	
	if (led_status[LED_STOP] == 1) beep_flag = 1;
	else beep_flag = 0;	
}

void program_limitations_CAN (void){
	if (flag_test == 0){
		//ограничения максимального и минимального угла стрелы
		//=================================================================================================
		if (angle_gamma_rama >= angle_strely_max) MAX_UP_STRELY = 1;
		else MAX_UP_STRELY = 0;
			
		if (angle_gamma_rama <= params.angle_strely_min) MAX_DOWN_STRELY = 1;
		else MAX_DOWN_STRELY = 0;

		if (Q_massa < 500) GRUZ_NA_KRUKE	= 0; //груза НЕТ //if (USILIE_CAN < (params.F0 + (float)params.F0*0.01))
		else  GRUZ_NA_KRUKE	= 1; //груз ЕСТЬ	
		//=================================================================================================
		//ОГРАНИЧЕНИЯ для CAN посылки 
		
		//масса загруженности < 110% и разомкнут концевик крюка и любое из огранич. запрещено
		if ((MAX_Mz == 1 && Mz < 110 && mode == 140) || MAX_LEP == 1 || MAX_PITCH == 1 || MAX_ROLL == 1 || (MAX_DOWN_STRELY == 1 && GRUZ_NA_KRUKE == 0) || NO_SENSORS == 1) { //сработали ограничения и масса загрузки меньше 110%
			
			if (flag_unlocking == 0) flag_button_unlocking = 1; //если не стоит флаг разрешения разблокировки ограничений, 
		}                                             	//ставим флаг разрешения разблокировки ограничений при удержании кнопки 5
		else {
			flag_button_unlocking = 0;
			flag_unlocking = 0;
		}
		
		if (flag_button_unlocking == 1 && flag_unlocking == 0) led_status[LED_Limitations] = 1; //индикация того, что сработали ограничения и можно нажать на кнопку, чтобы их убрать
		else led_status[LED_Limitations] = 0;
		
		//ограничения для опускания стрелы
		if (MAX_Mz == 1 || MAX_LEP == 1 || MAX_PITCH == 1 || MAX_ROLL == 1 || MAX_DOWN_STRELY == 1 || konc_kruka == 1 || NO_SENSORS == 1) {
			strela_down = 1; //ЗАПРЕЩАЕМ Опускать СТРЕЛУ
			strela_down_CAN = 1;
		}
		else  strela_down = 0; //РАЗРЕШАЕМ Опускать СТРЕЛУ
		
		if (flag_unlocking == 1 && konc_kruka == 0 && Mz < 110){
			strela_down_CAN = 0; //если нажали на кнопку и концевик крюка разомкнут, то разрешаем опускать стрелу
			if (MAX_DOWN_STRELY == 1 && GRUZ_NA_KRUKE == 1) strela_down_CAN = 1; //абсолютное ограничение если при опускании стрелы меньше минимума, на крюке появился груз
			if (MAX_Mz == 1 && mode == 117) strela_down_CAN = 1; //абсолютное ограничение если степень загрузки >95% при работе в колонне
		}
		else if (strela_down == 1)  strela_down_CAN = 1; //проверяем само ограничение
		else strela_down_CAN = 0;	

		//ограничения для поднятия стрелы
		if ((MAX_Mz == 1 && mode == 117) || Mz > 110 || MAX_LEP == 1 || MAX_PITCH == 1 || MAX_ROLL == 1 || MAX_UP_STRELY == 1 || konc_streli == 1 || NO_SENSORS == 1) {
			strela_up = 1; //ЗАПРЕЩАЕМ Поднимать СТРЕЛУ
			strela_up_CAN = 1;
		}
		else	strela_up = 0; //РАЗРЕШАЕМ Поднимать СТРЕЛУ
		
		if (flag_unlocking == 1 && konc_streli == 0 && MAX_UP_STRELY == 0 && Mz < 110){
			strela_up_CAN = 0; //проверяем запрет посылки ограничений в КАН
			if (MAX_Mz == 1 && mode == 117) strela_up_CAN = 1; //абсолютное ограничение если степень загрузки >95% при работе в колонне
		}
		else if (strela_up == 1)  strela_up_CAN = 1; //проверяем само ограничение
		else strela_up_CAN = 0;
		
		//ограничения для опускания крюка
		if (MAX_LEP == 1 || NO_SENSORS == 1){
			kruk_down = 1;
			kruk_down_CAN = 1;
		}
		else kruk_down = 0;
		
		if (flag_unlocking == 1)  kruk_down_CAN = 0; //проверяем запрет посылки ограничений в КАН
		else if (kruk_down == 1)  kruk_down_CAN = 1; //проверяем само ограничение
		else kruk_down_CAN = 0;
			
		//ограничения для подъема крюка
		if (MAX_Mz == 1 || MAX_LEP == 1 || MAX_PITCH == 1 || MAX_ROLL == 1 || konc_kruka == 1 || NO_SENSORS == 1) {
			kruk_up = 1;
			kruk_up_CAN = 1;
		}
		else kruk_up = 0;
		
		if (flag_unlocking == 1 && konc_kruka == 0 && Mz < 110){ //абсолютное ограничение если сработал конц крюка или степень загрузки > 110
			kruk_up_CAN = 0; //проверяем запрет посылки ограничений в КАН
			if (MAX_Mz == 1 && mode == 117) kruk_up_CAN = 1; //абсолютное ограничение если степень загрузки >95% при работе в колонне
		}
		else if (kruk_up == 1)  kruk_up_CAN = 1; //проверяем само ограничение
		else kruk_up_CAN = 0;	
		

		//ограничения для складывания КГ (масса на крюке больше тонны)
		if (MAX_Mz == 1 || MAX_LEP == 1 || MAX_PITCH == 1 || MAX_ROLL == 1 || GRUZ_NA_KRUKE == 1 || NO_SENSORS == 1) {
			KG_skladivanie = 1;
			KG_skladivanie_CAN = 1;
		}
		else KG_skladivanie = 0;

		if (flag_unlocking == 1 && Mz < 110){  
			KG_skladivanie_CAN = 0; //проверяем запрет посылки ограничений в КАН 
			if (MAX_Mz == 1) KG_skladivanie_CAN = 1; //абсолютное ограничение если степень загрузки >95% при работе в колонне или > 105% при одиночном реж
			if (GRUZ_NA_KRUKE == 1) KG_skladivanie_CAN = 1; //абсолютное ограничение если на крюке висит груз
		}
		else if (KG_skladivanie == 1)  KG_skladivanie_CAN = 1; //проверяем само ограничение
		else KG_skladivanie_CAN = 0;
		
		//ограничения для откидывания КГ (Сработал концевик)
		if (MAX_LEP == 1 || MAX_PITCH == 1 || MAX_ROLL == 1 || protivoves != 0 || NO_SENSORS == 1) {
			KG_otkidivanie = 1;
			KG_otkidivanie_CAN = 1;
		}
		else KG_otkidivanie = 0;

		if (flag_unlocking == 1 && protivoves != 1)  KG_otkidivanie_CAN = 0; //проверяем запрет посылки ограничений в КАН
		else if (KG_otkidivanie == 1)  KG_otkidivanie_CAN = 1; //проверяем само ограничение
		else KG_otkidivanie_CAN = 0;	

		if (flag_lock_block == 0){
			CAN_MESSAGE = (TS_kruka << 7) | (TS_strely << 6) | (KG_skladivanie_CAN << 5) | (KG_otkidivanie_CAN << 4) | 
										(kruk_up_CAN << 3) | (kruk_down_CAN << 2) | (strela_up_CAN << 1) | strela_down_CAN;
			CAN_MESSAGE_1 = GRUZ_NA_KRUKE;
			//CAN_MESSAGE = ~CAN_MESSAGE;
		}     //если зашли в Настройки, то ограничения в КАН не шлем
		else {  
			CAN_MESSAGE = (TS_kruka << 7) | (TS_strely << 6) | 0 << 5 | 0 << 4 | 0 << 3 | 0 << 2 | 0 << 1 | 0;
			CAN_MESSAGE_1 = 0;
		}		
	}
	else{
		if (flag_lock_block == 0) {
			KG_otkidivanie_CAN = 1; KG_skladivanie_CAN = 1; kruk_up_CAN = 1; kruk_down_CAN = 1; strela_up_CAN = 1; strela_down_CAN = 1;
			CAN_MESSAGE = 0xFF;
			CAN_MESSAGE_1 = 0xFF;
		}
		else{ //если зашли в Настройки, то ограничения в КАН не шлем
			CAN_MESSAGE = (TS_kruka << 7) | (TS_strely << 6) | 0 << 5 | 0 << 4 | 0 << 3 | 0 << 2 | 0 << 1 | 0;
			CAN_MESSAGE_1 = 0;
		}
	}
}

extern unsigned int Load$$LR$$LR_IROM1$$Limit;

uint32_t* begin = (uint32_t*)MAIN_PROGRAM_START_ADDRESS;     
uint32_t calculated_crc;

static void CRC32_Config(uint32_t poly)
{
  /* Enable CRC AHB clock interface */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);

  /* DeInit CRC peripheral */
  CRC_DeInit();
  
  /* Init the INIT register */
  CRC_SetInitRegister(0xFFFFFFFF);
  
  /* Select 32-bit polynomial size */
  CRC_PolynomialSizeSelect(CRC_PolSize_32);
  
  /* Set the polynomial coefficients */
  CRC_SetPolynomial(poly);
}

void Check_CRC_program (void){
	
	CRC32_Config(0x4C11DB7);
	
	uint32_t firmware_end = (unsigned int) &(Load$$LR$$LR_IROM1$$Limit);
	uint32_t* end = (uint32_t*)firmware_end;      // адрес CRC
	
	while (begin != end)
	{
			CRC->DR = *begin++;
	}
	calculated_crc = CRC->DR;
	if(calculated_crc == *end){

	}
	else {
		lcd44780_SetLCDPosition(0, 0);		
		lcd44780_ShowStr("    Прошивка    ");
		lcd44780_SetLCDPosition(0, 1);
		lcd44780_ShowStr("   повреждена   ");
		while(1){} //ЗАВИСАЕМ
	}	
}

void program_read_time (void){
	if(GetGTimer(TIMER_read_time) >= ms_100){
		ResetGTimer(TIMER_read_time);
		if(status_RTC == SUCCESS){
			ds3231_read_ALL(&DS3231registers);
			
			ds3231_sec 		= BCD_to_DEC(DS3231registers.seconds);
			ds3231_min 		= BCD_to_DEC(DS3231registers.minutes);
			ds3231_hour 	= BCD_to_DEC(DS3231registers.hours);
			
			ds3231_day 		= BCD_to_DEC(DS3231registers.date);
			ds3231_month 	= BCD_to_DEC(DS3231registers.mon_cen);
			ds3231_year 	= BCD_to_DEC(DS3231registers.year)+2000;
			
			if (ds3231_day == 0){ 
				I2C_Cmd(DS3231_I2C, DISABLE); 
				delay_ms(5);
				I2C_Cmd(DS3231_I2C, ENABLE);
			} //перезапуск шины, если нет часов
		}
	}	
}

int32_t STM32_MD5_HASH_DigestCompute(uint8_t* InputMessage, uint32_t InputMessageLength,
                               uint8_t *MessageDigest, int32_t* MessageDigestLength)
{
  MD5ctx_stt P_pMD5ctx;
  uint32_t error_status = HASH_SUCCESS;

  /* Set the size of the desired hash digest */
  P_pMD5ctx.mTagSize = CRL_MD5_SIZE;

  /* Set flag field to default value */
  P_pMD5ctx.mFlags = E_HASH_DEFAULT;

  error_status = MD5_Init(&P_pMD5ctx);

  /* check for initialization errors */
  if (error_status == HASH_SUCCESS)
  {
    /* Add data to be hashed */
    error_status = MD5_Append(&P_pMD5ctx,
                              InputMessage,
                              InputMessageLength);

    if (error_status == HASH_SUCCESS)
    {
      /* retrieve */
      error_status = MD5_Finish(&P_pMD5ctx, MessageDigest, MessageDigestLength);
    }
  }

  return error_status;
}

uint32_t generatePassword_SN(char *serial_num){
	int32_t status = HASH_SUCCESS;

	char Message[19] =  "odmeN@";
	char Message_MD5_STR[33] = "";
	char Message_MD5_STR_OUT[33] = "";
	char Message_MD5_STR_OUT_PASS[5] = "";
	
	uint8_t Length = (sizeof(Message) - 1);
	
	char serial_num_[5];
	
	int indexIN = 0;
	int indexOUT = 0;

	strcpy(serial_num_, serial_num);
	
	for(int i = 0; serial_num_[i] != 0; i++)
	{
		if (serial_num_[i] == '0') serial_num_[i] = '1';
	}
	
	for(int i = 0; serial_num_[i] != 0; i++)
	{
		if (serial_num_[i] == '2') serial_num_[i] = '3';
	}
	
	for(int i = 0; serial_num_[i] != 0; i++)
	{
		if (serial_num_[i] == '4') serial_num_[i] = '5';
	}
	
	for(int i = 0; serial_num_[i] != 0; i++)
	{
		if (serial_num_[i] == '6') serial_num_[i] = '7';
	}

	for(int i = 0; serial_num_[i] != 0; i++)
	{
		if (serial_num_[i] == '8') serial_num_[i] = '9';
	}

	strcat(Message, serial_num_buf);
	strcat(Message, "#");
	strcat(Message, serial_num_);
	strcat(Message, ":");
	strcat(Message, "4");
	strcat(Message, "!");

	Crypto_DeInit();

  status = STM32_MD5_HASH_DigestCompute((uint8_t*)Message,
                                  Length,
                                  (uint8_t*)MessageDigest,
                                  &MessageDigestLength);
	sprintf(Message_MD5_STR, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", MessageDigest[0],MessageDigest[1],MessageDigest[2],MessageDigest[3],MessageDigest[4],MessageDigest[5],MessageDigest[6],
																MessageDigest[7],MessageDigest[8],MessageDigest[9],MessageDigest[10],MessageDigest[11],MessageDigest[12],MessageDigest[13],MessageDigest[14],MessageDigest[15]);  

  Message_MD5_STR[32] = '\0';

	while (Message_MD5_STR[indexIN] != '\0')
	{
		if (Message_MD5_STR[indexIN] < '0' || Message_MD5_STR[indexIN] > '9')
				indexIN++;
		else
		{
				Message_MD5_STR_OUT[indexOUT] = Message_MD5_STR[indexIN];
				indexIN++;
				indexOUT++;
		}
	}

	Message_MD5_STR_OUT[indexOUT] = '\0';
	
	strncpy(Message_MD5_STR_OUT_PASS, &Message_MD5_STR_OUT[4], 4); 
	
	return atoi(Message_MD5_STR_OUT_PASS);
}

void program_generate_password (void){
	if (serial != 0 && flag_serial == 0){
		flag_serial = 1;
		sprintf(serial_num_buf, "%04d", serial);
		password = generatePassword_SN(serial_num_buf);
	}
}

void program_sound_off (void){
	if (HOLD_Button == BUTTON_SOUND){
		if(beep_off) beep_off = 0;
		else beep_off = 1;
	}	
}

char *TG_selection_strings = "Выберите тип трубоукладчика";
void TG_selection_menu(void){
	gorizontal_scr(TG_selection_strings, 0, 0);
	
	Crane_selection();
	if (PRESS_Button == BUTTON_ENTER) {
		erasable_params.flag_device_first_turn_on = 1;
		FLASH_Write_erasable_parameters();
		save_params();
	}
}


int main(void)
{	
	Remap_Table();
	__enable_irq();
	
	Check_CRC_program(); //СДЕЛАТЬ REBUILD в конечном файле прошивки
		
	Init_RCC();
	CRC_Config(0xD5);
	
	AT45DB161_Init();
	Init_CAN();
	 	
//	for(page_write = 0; page_write < 8192; page_write++){
//		AT45DB161_PageProgram_without_BUF(page_write, 0, buf_1, 528);
//  }
	
	AT45DB_search_data();	
	
	MX_USB_DEVICE_Init();	 
	GPIO_USB_Init();
 
	I2C_init();
	ADC_DMA_init();
	
	Init_leds();
	Init_heater();
	 
	init_TIM6(); 	//Вкл и Выкл звука 
	Init_sound_PWM();
		  
	Init_button();

	check_memory_empty_cells();

	init_TIM3(); 	
	InitGTimers();
	Start_ALL_GTimer();
	
	program_check_ERROR();
	
	Menu_SetGenericWriteCallback(Generic_Write_Menu);
	menu_set_ptrGeneric(&Menu_1); //назначить первый пункт меню	
	menu_start();
	show_pop_up_windiw(limitations, program_version, loading_USB);

//	Independent_Watchdog();
	while(1)
{
	ProcessKeyFSM();
	ProcessMessages();
	program_key_click();
	
	if (erasable_params.flag_device_first_turn_on == 0){
		TG_selection_menu();
	}
	else {
		program_generate_password();
		
		program_read_time();//Каждые100МС
		
		program_write_params_to_AT45DB();//Каждые100МС
		copy_flash_to_SD();
		clear_flash();
		
		program_sensor_processing();
		

		
		menu_process(PRESS_Button);	
		
		program_sound_off();
				
		SD_Card_Handler();//каждую 1 сек
		program_work_minutes();//каждую 1 мин
		program_CAN_Send();//Каждые100МС
		program_USB_Transmit();//каждую 1 сек	
		program_timeout_SD_USB_Disp();//каждую 1 сек
		program_CAN_timeout_CRC();//каждую 1 сек
		
		
		
		temperature_reading();
		
		program_calculation_R();
		
		program_calculation_Qmax_and_Mz();
		
		program_LED_indication();

		program_limitations_CAN();

		Mass_calculation();	
		
		Led_show();	
	}

	PRESS_Button=0;
	HOLD_Button=0;
	REPEAT_Button = 0;	
	
	IWDG_ReloadCounter();
}
}
