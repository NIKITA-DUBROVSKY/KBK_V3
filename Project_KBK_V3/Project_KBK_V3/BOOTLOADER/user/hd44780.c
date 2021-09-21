//************************************************************************//
//	LCD HD44780
//	Alex_EXE 
//	http://alex-exe.ru/category/radio/stm32/stm32-lcd-hd44780-spl
//************************************************************************//
#include <hd44780.h>

const unsigned char lcd44780_addLUT[4] = {0x80, 0xC0, 0x94, 0xD4};//выбор адреса нулнвой позиции
unsigned char lcd44780_Address, lcd44780_Line;

const unsigned char ansii_table_for_epson[256]=
{
/*0*/ 0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*1*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*2*/ ' ','!','"','#','$','%','&',0x27,'(',')','*','+',',','-','.','/',
/* 0 1 2 3 4 5 6 7 8 9 : ; ?*/
/*3*/ '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
/*4*/ '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
/* P Q R S T U V W X Y Z [ \ ] ^ _ */
/* P Q R S T U V W X Y Z [ Ф ] ^ _ */
/*5*/ 'P','Q','R','S','T','U','V','W','X','Y','Z','[',0x4C,']','^','_',
/* ‘ a b c d e f g h i j k l m n o*/
/*6*/ 0x60,'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
/* p q r s t u v w x y z { | } ~ ~ */
/* p q r s t u v w x y z 10 12 15 cr ~ */
/*7*/ 'p','q','r','s','t','u','v','w','x','y','z',0x7b,0x7c,0x7d,0x7e,0xe9,
/*остальные спецсимволы перенесены в стобцы 8 и 9 */
/*<> » inv№ inv? integr ‘ e’ ii o O */
/*8*/ 0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,0xe7,0xe8,0xea,0xeb,0xec,0xed,0xee,0xef,
/* epsilon */
/*9*/ 0x7f,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
/*столбец D таблицы Epson перенесен в A-й*/
/*A*/ 0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
/*столбец F таблицы Epson перенесен в B-й т.е вместо 0xf2 указываем 0xb2*/
/*B*/ 0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff,
/* А Б В Г Д Е Ж З И Й К Л М Н О П*/
/*C*/ 'A',0xa0,'B',0xa1,0xe0,'E',0xa3,0xa4,0xa5,0xa6,'K',0xa7,'M','H','O',0xa8,
/* Р С Т У Ф Х Ц Ч Ш Щ Ъ Ы Ь Э Ю Я*/
/*D*/ 'P','C','T',0xa9,0xaa,'X',0xe1,0xab,0xac,0xe2,0xad,0xae,0xC4,0xaf,0xb0,0xb1,
/*а б в г д е ж з и й к л м н о п*/
/*E*/ 'a',0xb2,0xb3,0xb4,0xe3,'e',0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,'o',0xbe,
/* р с т у ф х ц ч ш щ ъ ы ь э ю я*/
/*F*/ 'p','c' ,0xbf,'y',0xe4,'x',0xe5,0xc0,0xc1,0xe6,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7

};

//************************************************************************//
GPIO_InitTypeDef lcd44780_GPIO_InitStructure;	//	структура для инициализации выводов используемых дисплеем
//пользовательские символы в дисплей
uint8_t symbol_degree[8] =  {
	0b01110,
	0b01010,
	0b01110,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000
};

uint8_t symbol_gamma[8] =  {
	0b01001,
	0b10101,
	0b00101,
	0b00101,
	0b00010,
	0b00010,
	0b00010,
	0b00100
};

uint8_t symbol_alpha[8] =  {
	0b00000,
	0b00000,
	0b01001,
	0b10101,
	0b10010,
	0b10010,
	0b01101,
	0b00000
};

uint8_t symbol_beta[8] =  {
	0b00110,
	0b01001,
	0b01001,
	0b01110,
	0b01001,
	0b01001,
	0b01110,
	0b10000
};

#define pause 50

void delay_micros(uint16_t us)
{
	TIM16->CNT = 0U; // обнуляем счётчик
	while(TIM16->CNT < us);
}

//	инициализация выводов дисплея
void lcd44780_init_pins(void)
{ //D7 - PC13, D6 - PC15  //D7 - PA2, D6 - PA3
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC, ENABLE);
	
	lcd44780_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	lcd44780_GPIO_InitStructure.GPIO_Pin = 	lcd44780_pin_EN | lcd44780_pin_D4 | lcd44780_pin_D5 | lcd44780_pin_D7 | lcd44780_pin_D6;
	lcd44780_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	lcd44780_GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	lcd44780_GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	GPIO_Init(lcd44780_port_EN, &lcd44780_GPIO_InitStructure);
	
	lcd44780_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	lcd44780_GPIO_InitStructure.GPIO_Pin = 	lcd44780_pin_RS;
	lcd44780_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	lcd44780_GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	lcd44780_GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	GPIO_Init(lcd44780_port_RS, &lcd44780_GPIO_InitStructure);
}
//************************************************************************//

//	Отправка полубайта дисплею
//	Input : data - пол байта данных
void lcd44780_WriteNibble(uint8_t data)
{
	uint8_t data_inv = ~data;
	lcd44780_EN_1;
//	if ((data & 0x01) == 1) GPIO_WriteBit(lcd44780_port_D4, lcd44780_pin_D4, Bit_SET);
//	else GPIO_WriteBit(lcd44780_port_D4, lcd44780_pin_D4, Bit_RESET);
//	
//	if ((data & 0x02) == 2) GPIO_WriteBit(lcd44780_port_D5, lcd44780_pin_D5, Bit_SET);
//	else GPIO_WriteBit(lcd44780_port_D5, lcd44780_pin_D5, Bit_RESET);
//	
//	if ((data & 0x04) == 4) GPIO_WriteBit(lcd44780_port_D6, lcd44780_pin_D6, Bit_SET);
//	else GPIO_WriteBit(lcd44780_port_D6, lcd44780_pin_D6, Bit_RESET);
//	
//	if ((data & 0x08) == 8) GPIO_WriteBit(lcd44780_port_D7, lcd44780_pin_D7, Bit_SET);
//	else GPIO_WriteBit(lcd44780_port_D7, lcd44780_pin_D7, Bit_RESET);
	GPIOA->BSRR = (data << 0) | (data_inv << 16); //выставляем данные на ножки контроллера (9 пин соответствует ножке данных D4, 10 пин - D5 и т.д.)
	delay_micros(10); //задержка для строба
	lcd44780_EN_0;
	delay_micros(50); //задержка для контроллера дисплея, чтобы тот успел записать данные в свою RAM
}

//	Отправка байта дисплею
//	Input : data - отправляемые данные
void lcd44780_WriteByte(uint8_t data)
{
	lcd44780_WriteNibble(data >> 4);
	lcd44780_WriteNibble(data & 0x0F);
}

//	Перейти на строчку
//	Input : LineNum - номер строки
void lcd44780_GoToLine(char LineNum)
{
	lcd44780_RS_0;
	lcd44780_Address = lcd44780_addLUT[LineNum-1];
	lcd44780_WriteByte(lcd44780_Address);
	lcd44780_RS_1;
	lcd44780_Address = 0;
	lcd44780_Line = LineNum;
}

//	Очистка дисплея
void lcd44780_ClearLCD(void)
{
	lcd44780_RS_0;
	lcd44780_WriteByte(0x01);
	delay_micros(pause*32);
	lcd44780_RS_1;
	lcd44780_GoToLine(1);
}

//	Установка курсора дисплея
//	Input : x , y - координаты курсора
void lcd44780_SetLCDPosition(char x, char y)
{
	lcd44780_RS_0;
	lcd44780_Address = lcd44780_addLUT[y] + x;
	lcd44780_WriteByte(lcd44780_Address);
	lcd44780_RS_1;
	lcd44780_Line = y+1;
}

//	Отправка символа
//	Input : c - символ
void lcd44780_ShowChar(unsigned char c)
{
	lcd44780_RS_1;
	lcd44780_WriteByte(c);
	lcd44780_Address++;
	switch (lcd44780_Address)
	{
		case 20: lcd44780_GoToLine(2); break;
		case 40: lcd44780_GoToLine(3); break;
		case 60: lcd44780_GoToLine(4); break;
		case 80: lcd44780_GoToLine(1); break;
	}
}

//	Отправка строки
//	Input : *s - ссылка на строку (массив символов)
//void lcd44780_ShowStr(unsigned char *s)
//{
//	while (*s != 0) lcd44780_ShowChar(*s++);
//}

void lcd44780_ShowStr(char *s)
{
while (*s != 0) lcd44780_ShowChar(ansii_table_for_epson[*s++]);
}

//	Инициализация дисплея
void lcd44780_init(void)
{
	lcd44780_EN_0;
	lcd44780_RS_0;
	delay_micros(pause*320);
	
	lcd44780_WriteNibble(0x3); //вход в инициализацию
	delay_micros(pause*82);
	lcd44780_WriteNibble(0x3);	//вход в инициализацию
	delay_micros(pause*2);
	lcd44780_WriteNibble(0x3);	//вход в инициализацию
	lcd44780_WriteNibble(0x2);	//ставим шину 4 бита
	
	lcd44780_WriteByte(0b00101100);
	lcd44780_WriteByte(0b00001000);	//000 дисплей - выкл, курсор - выкл, моргание курсора - выкл
	lcd44780_WriteByte(0b00000001);	//очистка экрана, счетчик адреса на 0 позицию DDRAM
	lcd44780_WriteByte(0b00000110);	//настройка сдвига экрана и курсора

	lcd44780_RS_1;
	lcd44780_ClearLCD();
	
	Write_command(0b00001100);//вкл. дисплей, курсор - не стоит и не мигет
	
	Set_CGRAM_Symbol(0, symbol_gamma);//записываем свой символ в дисплей
	Set_CGRAM_Symbol(0x08, symbol_degree);//записываем свой символ в дисплей
	Set_CGRAM_Symbol(0x10, symbol_alpha);//записываем свой символ в дисплей
	Set_CGRAM_Symbol(0x18, symbol_beta);//записываем свой символ в дисплей
}

/*функция для записи пользовательского символа в память дисплея
address может принимать значения 0x00 - 0 ячейка, 0x08 - 1 ячейка, 0x10 - 2 ячейка и т.д.
data принимает массив с символом
*/
void Set_CGRAM_Symbol(uint8_t address, uint8_t *data) 
{
	lcd44780_RS_0;
	lcd44780_WriteByte(address | 0x40);
	lcd44780_RS_1;
	for (uint8_t i = 0; i < 8; i++)
	{
		lcd44780_WriteByte(data[i]);
	}	
}

//отправка команды дисплею
void Write_command(uint8_t command)
{
	lcd44780_RS_0;
	lcd44780_WriteByte(command);
	lcd44780_RS_1;
}
