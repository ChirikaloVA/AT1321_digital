//clock.h
//clock control

#ifndef _CLOCK_H
#define _CLOCK_H

#include <iolpc2388.h>
#include "types.h"

struct tagClock
{
  BOOL bMustBeInited;  //=1 then clock is not inited
  BYTE dayOfWeek;
  WORD dayOfYear;
  struct tagDateTime dateTime;
  BOOL mayUpdateDateTimeView;
  DWORD dwMiliseconds;	//used only for sleep function
  DWORD dwTotalSecondsFromStart;	//totalseconds from start device
};

extern struct tagClock volatile clockData;

void Clock_control_Init(void);
void Clock_retrieveDateTimeData_intcall(void);
__arm void _INT_Clock(void);
void Clock_showDateTime(void);

WORD Clock_Randomize(void);

char* Clock_getClockDateTimeStr(char* pStr);
char* Clock_getClockTimeStr(char* pStr);

void Clock_setDateTime(int sec, int min, int hour, int dom, int dow, int doy, int month, int year);


char* Clock_getClockDateTimeStrEx(char* pStr);

#endif  //#ifndef _CLOCK_H
