#ifndef TIMERS_h
#define TIMERS_h
//----------------------------------



//определение единиц времени
#define ms_40 4
#define ms_50 5 
#define ms_100 10
#define ms_200 20
#define ms_500 50
#define sec 100  		//период работы таймера 10мс, т.е. 1с соответствует 100 периодам
#define sec_10 sec*10
#define minute_timer (60*sec)
#define hour (60*minute_timer)
#define day 24*hour

#define MAX_GTIMERS 30	//максимальное количество таймеров 
//в зтом разделе обьявляются константы , служацие идентификаторами таймеров.

typedef enum
{
	TIMER_CAN_Send = 0,
	TIMER_CAN_timeout_DUG50,
	TIMER_CAN_timeout_DUG51,
	TIMER_CAN_timeout_LEP,
	TIMER_CAN_timeout_JOY,
	KEYB_TIMER,
	TIMER_Blink_symbol,
	TIMER_SD_Card_Handler,
	TIMER_work_minutes,
	TIMER_USB_Transmit,
	TIMER_timeout_SD_USB_Disp,
	TIMER_BAN_REC_SD,
	TIMER_write_AT45DB,
	TIMER_read_time,
	TIMER_read_temperature_ds3231,
	TIMER_scroll_str,
	TIMER_string_SD,
	
	TOTAL_NUMBER_OF_TIMERS
}TIMERS_TypeDef;




//функции работы с локальными  таймерами
/*************************************************
void InitTimers(void);

unsigned int GetTimer(char Timer);

void ResetTimer(char Timer);
***************************************************/
//функции работы с локальными таймерами
//#################################################
void InitGTimers(void);							//Инициализация глобальных таймеров		
void StartGTimer(TIMERS_TypeDef GTimerID);		//Запуск таймера
void StopGTimer(TIMERS_TypeDef GTimerID);			//Остановка таймера
void PauseGTimer(TIMERS_TypeDef GTimerID);		//Приостановка работы таймера
void ReleaseGTimer(TIMERS_TypeDef GTimerID);		//Продолжение работы таймера
unsigned int GetGTimer(TIMERS_TypeDef GTimerID);	//Получение текущего значения таймера
void ProcessTimers(void);						//Обработмчик прерыванмя таймера/счетчика		
void ResetGTimer(TIMERS_TypeDef GTimerID);
void Start_ALL_GTimer(void);

#endif
