#ifndef TIMERS_h
#define TIMERS_h
//----------------------------------



//����������� ������ �������
#define ms_40 4
#define ms_50 5 
#define ms_100 10
#define ms_200 20
#define ms_500 50
#define sec 100  		//������ ������ ������� 10��, �.�. 1� ������������� 100 ��������
#define sec_10 sec*10
#define minute_timer (60*sec)
#define hour (60*minute_timer)
#define day 24*hour

#define MAX_GTIMERS 30	//������������ ���������� �������� 
//� ���� ������� ����������� ��������� , �������� ���������������� ��������.

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




//������� ������ � ����������  ���������
/*************************************************
void InitTimers(void);

unsigned int GetTimer(char Timer);

void ResetTimer(char Timer);
***************************************************/
//������� ������ � ���������� ���������
//#################################################
void InitGTimers(void);							//������������� ���������� ��������		
void StartGTimer(TIMERS_TypeDef GTimerID);		//������ �������
void StopGTimer(TIMERS_TypeDef GTimerID);			//��������� �������
void PauseGTimer(TIMERS_TypeDef GTimerID);		//������������ ������ �������
void ReleaseGTimer(TIMERS_TypeDef GTimerID);		//����������� ������ �������
unsigned int GetGTimer(TIMERS_TypeDef GTimerID);	//��������� �������� �������� �������
void ProcessTimers(void);						//����������� ���������� �������/��������		
void ResetGTimer(TIMERS_TypeDef GTimerID);
void Start_ALL_GTimer(void);

#endif
