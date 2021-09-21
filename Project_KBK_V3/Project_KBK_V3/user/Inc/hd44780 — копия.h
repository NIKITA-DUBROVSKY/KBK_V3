//************************************************************************//
//	LCD HD44780
//	Alex_EXE 
//	http://alex-exe.ru/category/radio/stm32/stm32-lcd-hd44780-spl
//************************************************************************//
#include "stm32f0xx.h"


#define	SysClock 48000000 //частота шины на которой сидит таймер
#define	TICKs_us SysClock/1000000 //вычисляем кол-во тиков в 1 мкс
//************************************************************************//
//			Конфигурация порта
#define lcd44780_port_RS		GPIOB
#define lcd44780_pin_RS			GPIO_Pin_14

#define lcd44780_port_EN		GPIOA
#define lcd44780_pin_EN			GPIO_Pin_8

#define lcd44780_port_D7		GPIOA
#define lcd44780_pin_D7   	GPIO_Pin_3

#define lcd44780_port_D6		GPIOA
#define lcd44780_pin_D6   	GPIO_Pin_2

#define lcd44780_port_D5		GPIOA
#define lcd44780_pin_D5   	GPIO_Pin_1 

#define lcd44780_port_D4		GPIOA
#define lcd44780_pin_D4   	GPIO_Pin_0         


#define lcd44780_RS_1 	lcd44780_port_RS->BSRR = lcd44780_pin_RS;
#define lcd44780_EN_1  	lcd44780_port_EN->BSRR = lcd44780_pin_EN;

#define lcd44780_RS_0 	lcd44780_port_RS->BRR = lcd44780_pin_RS;
#define lcd44780_EN_0  	lcd44780_port_EN->BRR = lcd44780_pin_EN;
//************************************************************************//

////************************************************************************//
////			Конфигурация порта
////	используемый порт
//#define lcd44780_port				GPIOA
////	используемые выводы
//#define lcd44780_pin_RS			GPIO_Pin_14
//#define lcd44780_pin_E			GPIO_Pin_8
////#define lcd44780_pins_data		GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15
//#define lcd44780_pin_D7   	GPIO_Pin_2        
//#define lcd44780_pin_D6   	GPIO_Pin_3          
//#define lcd44780_pin_D5   	GPIO_Pin_10          
//#define lcd44780_pin_D4   	GPIO_Pin_9         

////	включить тактирование порта
//#define lcd44780_RCC					RCC_AHBPeriph_GPIOA
////	смещение начала линии данных
////#define lcd44780_offset 			12

//#define lcd44780_RS_1 GPIO_SetBits(GPIOB, lcd44780_pin_RS);
//#define lcd44780_E_1  GPIOA->BSRR = lcd44780_pin_E;
//#define lcd44780_RS_0 GPIO_ResetBits(GPIOB, lcd44780_pin_RS);
//#define lcd44780_E_0  GPIOA->BRR = lcd44780_pin_E;
////************************************************************************//

//	Инициализация выводов дисплея
void lcd44780_init_pins(void);

//	Инициализация дисплея
void lcd44780_init(void);

//	задержка
//	Input : p - величина задержки
void lcd44780_delay(unsigned int p);

//	Очистка дисплея
void lcd44780_ClearLCD(void);

//	Установка курсора дисплея
//	Input : x , y - координаты курсора
void lcd44780_SetLCDPosition(char x, char y);

//	Отправка символа
//	Input : c - символ
void lcd44780_ShowChar(unsigned char c);

//	Отправка строки
//	Input : *s - ссылка на строку (массив символов)
void lcd44780_ShowStr(char *s);

void Set_CGRAM_Symbol(uint8_t address, uint8_t *data);

void Write_command(uint8_t command);
void lcd44780_WriteByte(uint8_t data);
