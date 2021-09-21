//messages.h
#ifndef MESSAGES_h
#define MESSAGES_h

#include "stm32f0xx.h"

#define MAX_MESSAGES 10	

#define MAX_BROADCAST_MESSAGES 10

#define MSG_SOME_MESSAGE 0

#define MSG_KEY_PRESSED 1
#define MSG_KEY_HOLD 2
#define MSG_KEY_REPEAT 3
#define MSG_Beep 4
#define MSG_Oven_ON 5
#define MSG_Err_Termocouple 6
#define MSG_overheat_boiler 7
#define MSG_overheat_screw 8

#define MSG_LEDON_ACTIVATE 2
void ProcessMessages(void);
void InitMessages(void);
void SendMessage(char Msg, char params);
char GetMessage(char Msg);
void SendBroadcastMessage(char Msg);
char GetMessageParams( char Msg);
void SendParams(char Msg, char params);
#endif
