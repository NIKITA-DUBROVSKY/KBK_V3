#include "buttons.h"
#include "stm32f0xx.h"
#include "timers.h"
#include "messages.h"
#include "MicroMenu.h"

uint8_t PRESS_Button, HOLD_Button, REPEAT_Button;

uint8_t _key_state,						//предвдущее состо€ние автомата опроса кнопок
				key_state, 						//текущее состо€ние автомата опроса кнопок
				key_code, 						//код кнопки
				_key_code,						//код кнопки на предыдущем цикле
				debounce = ms_40,			//задержка дл€ подавлени€ дребезга
				first_delay = sec,	//задержка перед первым повтором нажати€ клавиши
				auto_repeat = ms_100		//период автоповтора нажатий клавиш
;

void Init_button(void){
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
//кофигурируем пины дл€ кнопок
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = SB1_GPIO_PIN | SB2_GPIO_PIN | SB3_GPIO_PIN | SB4_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; //вход
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//void Init_button(void){
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);
////кофигурируем пины дл€ кнопок
//	GPIO_InitTypeDef  GPIO_InitStructure;
//	GPIO_InitStructure.GPIO_Pin = SB3_GPIO_PIN;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; //вход
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
//	GPIO_InitStructure.GPIO_Pin = SB1_GPIO_PIN | SB2_GPIO_PIN | SB4_GPIO_PIN;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; //вход
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//}


//функци€ переводит массив с линий данных шифратора в номер нажатой кнопки
uint8_t NumbPressButton(void){
	uint8_t j = 0;

	if (GPIO_ReadInputDataBit(SB1_GPIO_PORT, SB1_GPIO_PIN) ==  1) j = 1;
	else if  (GPIO_ReadInputDataBit(SB2_GPIO_PORT, SB2_GPIO_PIN) ==  1) j = 2;
	else if  (GPIO_ReadInputDataBit(SB3_GPIO_PORT, SB3_GPIO_PIN) ==  1) j = 3;
	else if  (GPIO_ReadInputDataBit(SB4_GPIO_PORT, SB4_GPIO_PIN) ==  1) j = 4;
	else  j = 0;  //ничего не нажато
	
	if  ((GPIO_ReadInputDataBit(SB4_GPIO_PORT, SB4_GPIO_PIN) ==  1) && (GPIO_ReadInputDataBit(SB3_GPIO_PORT, SB3_GPIO_PIN) ==  1)) j = 5;
	
	return j;//переменна€ с номером нажатой кнопки
}
	
void ProcessKeyFSM(void){
	key_code = NumbPressButton();
	switch (key_state){
		//состо€ние 0. ¬ этом состо€нии автомат ожидает нажати€ на клавишу*/
		case 0: //клавиша не нажата 
			if (_key_state == 2){ //если пришли из состо€ни€ нажатой кнопки 
				SendMessage(MSG_KEY_PRESSED, GetMessageParams(MSG_KEY_PRESSED));				
				_key_state = 0;
			}
			if (key_code > 0){
				_key_code = key_code;
				ResetGTimer(KEYB_TIMER);
				key_state = 1;
			}
		break;
			
		/*состо€ние 1. ¬ этом состо€нии вводитс€ задержка на врем€ debounce
			(ожидание завершени€ переходного процесса при замыкании контактов клавиши)*/
		case 1: //задержка подавлени€ дребезга
			if (GetGTimer(KEYB_TIMER) > debounce){
				_key_state = 1;
				key_state = 2;
			}
		break;

		/*состо€ние 2. ¬ этом состо€нии фиксируетс€ факт нажати€ клавиши и формируетс€ сообщение 
		MSG_KEY_PRESSED. ≈сли клавиша не нажата, возврат в состо€ние 0*/ 
		case 2: //если клавиша нажата, посылаем сообщение
			if (key_code == _key_code){
				ResetGTimer(KEYB_TIMER);
				SendParams(MSG_KEY_PRESSED, key_code);
				_key_state = 2;
				key_state = 3;
			}
			else
				key_state = 0; 
		break;

		/*состо€ние 3. ¬ этом состо€нии автомат формирует сообщение MSG_KEY_PRESSED, 
		если пользователь удерживает клавишу в течение времени first_delay. ≈сли клавиша отпущена, возврат в состо€ние 0*/ 
		case 3:
			if (key_code == _key_code){
				if (GetGTimer(KEYB_TIMER) >= first_delay){
					ResetGTimer(KEYB_TIMER);
					SendMessage(MSG_KEY_HOLD, key_code);
					_key_state = 3;					
					key_state = 4;
				}
			}
			else
				key_state = 0; 
		break;

		/*состо€ние 4. ¬ этом состо€нии автомат формирует последовательность сообщений MSG_KEY_PRESSED 
		с периодом auto_repeat, если пользователь продолжает удерживать клавишу. ≈сли клавиша отпущена, возврат в состо€ние 0*/ 
		case 4:
			if (key_code == _key_code){
				if (GetGTimer(KEYB_TIMER) >= auto_repeat){
					ResetGTimer(KEYB_TIMER);
					_key_state = 4;
					SendMessage(MSG_KEY_REPEAT, key_code);
				}
			}
			else
				key_state = 0; 
		break;
	}
}

uint8_t GetKeyCode(void){
	return GetMessageParams(MSG_KEY_PRESSED);
}

void program_key_click (void){
	if(GetMessage(MSG_KEY_PRESSED))
	{
		switch (GetKeyCode()){
			
			case BUTTON_BACK:
				PRESS_Button = BUTTON_BACK;
			break;
			
			case BUTTON_ENTER:
				PRESS_Button = BUTTON_ENTER;
				
			break;
			
			case BUTTON_UP:
				PRESS_Button = BUTTON_UP;
			break;

			case BUTTON_DOWN:
				PRESS_Button = BUTTON_DOWN;
			break;		
		}
	}
	
	if(GetMessage(MSG_KEY_HOLD))
	{
		switch (GetKeyCode()){			
			case BUTTON_BACK:
				HOLD_Button = BUTTON_BACK;
			break;
			
			case BUTTON_ENTER:
				HOLD_Button = BUTTON_ENTER;				
			break;
			
			case BUTTON_UP:
				HOLD_Button = BUTTON_UP;
			break;

			case BUTTON_DOWN:
				HOLD_Button = BUTTON_DOWN;
			break;

			case BUTTON_SOUND:
				HOLD_Button = BUTTON_SOUND;
			break;			
		}
	}	
	
	if(GetMessage(MSG_KEY_REPEAT))
	{
		switch (GetKeyCode()){							
			case BUTTON_UP:
				REPEAT_Button = BUTTON_UP;
			break;

			case BUTTON_DOWN:
				REPEAT_Button = BUTTON_DOWN;
			break;
		}
	}
}
