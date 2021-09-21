//************************************************************************//
//	LCD HD44780
//	Alex_EXE 
//	http://alex-exe.ru/category/radio/stm32/stm32-lcd-hd44780-spl
//************************************************************************//
#include "stm32f0xx.h"
//************************************************************************//
//			Конфигурация порта
//	используемые выводы
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
//	Инициализация выводов дисплея
void lcd44780_init_pins(void);

//	Инициализация дисплея
void lcd44780_init(void);

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
void delay_micros(uint16_t us);
