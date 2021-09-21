#include "messages.h"
 char Messages[MAX_MESSAGES];
 char Parametrs[MAX_MESSAGES];
 char BroadcastMessages[MAX_BROADCAST_MESSAGES];

void InitMessages(void)
{
 unsigned char i;
 for(i=0; i<MAX_MESSAGES; i++)
 	Messages[i]=0;
 for(i=0; i<MAX_BROADCAST_MESSAGES; i++)
 	BroadcastMessages[i]=0;
}

void SendMessage(char Msg, char params)
{
	if(Messages[Msg] == 0){
		Messages[Msg]=1;
		Parametrs[Msg] = params;
	}
}

void SendParams(char Msg, char params)
{
		Parametrs[Msg] = params;
}

void SendBroadcastMessage(char Msg)
{
 BroadcastMessages[Msg]=1;
}

void ProcessMessages(void)
{
 unsigned char i;
 for(i=0; i<MAX_MESSAGES; i++)
 {
  if(Messages[i]==2) Messages[i]=0;
  if(Messages[i]==1) Messages[i]=2;
 }
 for(i=0; i<MAX_BROADCAST_MESSAGES; i++)
 {
  if(BroadcastMessages[i]==2) BroadcastMessages[i]=0;
  if(BroadcastMessages[i]==1) BroadcastMessages[i]=2;
 }
}



 char GetMessage( char Msg)
{
 if(Messages[Msg]==2)
  {
   Messages[Msg]=0;
   return 1;
  };
  return 0;
}

unsigned char GetBroadcastMessage(char Msg)
{
 if(Messages[Msg]==2)
   return 1;
  return 0;
}

char GetMessageParams( char Msg)
{
  return Parametrs[Msg];
}
