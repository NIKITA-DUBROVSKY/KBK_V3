#include "timers.h"
#include "stdint.h"

uint32_t  GTimers[MAX_GTIMERS];			//� ������� �������� ������� �������� ���������� ��������
//��������� �������
//=========================================
#define TIMER_STOPPED	0					//������ ����������
#define TIMER_RUNNING	1					//������ ��������
#define TIMER_PAUSED	2					//������ �� �����

char GTStates[MAX_GTIMERS];					//� ������� �������� ������� ��������� ���������� ��������
//########################################
void InitGTimers(void)						//������� ������������� ���������� ��������
{
 char i;
 for(i=0; i<MAX_GTIMERS; i++)
 	GTStates[i]=TIMER_STOPPED;
}
//########################################

void StartGTimer(TIMERS_TypeDef GTimerID)		// ������� ������� �������
{
  if(GTStates[GTimerID]==TIMER_STOPPED)
  {
  	GTimers[GTimerID]=0;
		GTStates[GTimerID]= TIMER_RUNNING;
  }
}
//#########################################
void StopGTimer(TIMERS_TypeDef GTimerID)		//��������� �������
{
	GTStates[GTimerID]=TIMER_STOPPED;
}
//##########################################
void ResetGTimer(TIMERS_TypeDef GTimerID)		//��������� �������
{
	GTimers[GTimerID]=0;
}
//##########################################
void PauseGTimer(TIMERS_TypeDef GTimerID)		//������������ ������ �������
{
	if(GTStates[GTimerID]==TIMER_RUNNING)
		GTStates[GTimerID]=TIMER_PAUSED;
}
//###########################################
void ReleaseGTimer(TIMERS_TypeDef GTimerID)	//����������� ������ �������
{
	if(GTStates[GTimerID]==TIMER_PAUSED)
		GTStates[GTimerID]=TIMER_RUNNING;
}
//###########################################
unsigned int GetGTimer(TIMERS_TypeDef GTimerID)	//��������� �������� �������� �������
{
	return GTimers[GTimerID];
}
//###########################################
void ProcessTimers(void)						//����������� ���������� �������/��������
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
