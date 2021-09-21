#include "timers.h"
#include "stdint.h"

uint32_t  GTimers[MAX_GTIMERS];			//В массиве хранятся текущие значения глобальных таймеров
//Состояние тацмера
//=========================================
#define TIMER_STOPPED	0					//Таймер остановлен
#define TIMER_RUNNING	1					//Таймер работает
#define TIMER_PAUSED	2					//Таймер на паузе

char GTStates[MAX_GTIMERS];					//В массиве хранятся текущие состояния глобальных таймеров
//########################################
void InitGTimers(void)						//Функция инициализация глобальных таймеров
{
 char i;
 for(i=0; i<MAX_GTIMERS; i++)
 	GTStates[i]=TIMER_STOPPED;
}
//########################################

void StartGTimer(TIMERS_TypeDef GTimerID)		// Функция запуска таймера
{
  if(GTStates[GTimerID]==TIMER_STOPPED)
  {
  	GTimers[GTimerID]=0;
		GTStates[GTimerID]= TIMER_RUNNING;
  }
}
//#########################################
void StopGTimer(TIMERS_TypeDef GTimerID)		//Остановка таймера
{
	GTStates[GTimerID]=TIMER_STOPPED;
}
//##########################################
void ResetGTimer(TIMERS_TypeDef GTimerID)		//Остановка таймера
{
	GTimers[GTimerID]=0;
}
//##########################################
void PauseGTimer(TIMERS_TypeDef GTimerID)		//Приостановка работы таймера
{
	if(GTStates[GTimerID]==TIMER_RUNNING)
		GTStates[GTimerID]=TIMER_PAUSED;
}
//###########################################
void ReleaseGTimer(TIMERS_TypeDef GTimerID)	//Продолжение работы таймера
{
	if(GTStates[GTimerID]==TIMER_PAUSED)
		GTStates[GTimerID]=TIMER_RUNNING;
}
//###########################################
unsigned int GetGTimer(TIMERS_TypeDef GTimerID)	//Получение текущего значения таймера
{
	return GTimers[GTimerID];
}
//###########################################
void ProcessTimers(void)						//Обработмчик прерыванмя таймера/счетчика
{
	char i;

	for(i=0; i<MAX_GTIMERS; i++)
		if(GTStates[i]==TIMER_RUNNING)
			GTimers[i]++;
}
//##########################################
void Start_ALL_GTimer(void)
{
	char i;
	
	for(i=0; i < TOTAL_NUMBER_OF_TIMERS; i++){
  	GTimers[i] = 0;
		GTStates[i] = TIMER_RUNNING;
	}
}
