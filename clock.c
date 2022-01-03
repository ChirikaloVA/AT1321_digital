//clock.c
//clock control

#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>

#include "types.h"
#include "syncObj.h"

#include "clock.h"
#include "display.h"
#include "powerControl.h"
#include "geiger.h"
#include "bluetooth.h"
#include "gps.h"


struct tagClock volatile clockData;

__arm void _INT_Clock(void)
{
  if(ILR_bit.RTCCIF)
  {//seconds interrupts from clock
    clockData.dwTotalSecondsFromStart++;
    //start adc to get voltage
    //result we will have in interrupt
    PowerControl_startADC_intcall();
    //retrieve date time from clock
    Clock_retrieveDateTimeData_intcall();
    //LED control
    //orange for gps
    //green for working
    
#ifndef GPS_BT_FREE	
    if(GPSControl.bGPS_ON)
    {
      if(GPSControl.bGPS_Fix)
        Display_flashOrangeLED();	//flash orange show we work with GPS
      else
        Display_turnONOrangeLED();	//turn off orange led
    }else
      Display_turnOFFOrangeLED();	//turn off orange led
#endif	//#ifndef GPS_BT_FREE	
    
    
    Display_flashGreenLED(); //flash green LED show we work
    ILR_bit.RTCCIF = 1;
    powerControl.bAwakedByClock = 1;	//mean that processor is awaked by clock (1 in a second)
    Geiger_GetCount_intcall();
  }else
    if(ILR_bit.RTCALF)
    {//alarm clock interrupts
      ILR_bit.RTCALF = 1;
    }else
      if(ILR_bit.RTSSF)
      {//subseconds interrupts from clock
	clockData.dwMiliseconds++;
	ILR_bit.RTSSF = 1;
      }
}



//init clock control, check does clock need adjusting
void Clock_control_Init(void)
{
  CCR_bit.CLKEN = 0;
  CCR_bit.CLKSRC = 1; //CLK source from external oscillator
  AMR = 0xff; //signal mask (disable all comparators)
  //no interruptions for beginning
  __ciir_bits ciir;
  ciir.IMSEC = 1;//select seconds interrupt from clock
  ciir.IMMIN = 0;
  ciir.IMHOUR = 0;
  ciir.IMDOM = 0;
  ciir.IMDOW = 0;
  ciir.IMDOY = 0;
  ciir.IMMON = 0;
  ciir.IMYEAR = 0;
  CIIR_bit = ciir;
  CISS_bit.SUBSECENA = 0; //disable subseconds int

  Clock_retrieveDateTimeData_intcall();


  if(clockData.dateTime.second>59 || clockData.dateTime.minute>59 || clockData.dateTime.hour>23 ||
	clockData.dayOfWeek>6 ||
	clockData.dateTime.dayOfMonth<1 || clockData.dateTime.dayOfMonth>31 ||
	clockData.dateTime.month<1 || clockData.dateTime.month>12 ||
	clockData.dayOfYear<1 || clockData.dayOfYear>366)
  {//clock is never initialized
	Clock_setDateTime(0,0,0,1,0,1,1,2011);
	clockData.bMustBeInited = 1;
  }else
  {
    clockData.bMustBeInited = 0;
  }

  CCR_bit.CTCRST = 0;
  CCR_bit.CTTEST = 0;
  CCR_bit.CLKEN = 1;

  clockData.dwMiliseconds = 0;
  clockData.dwTotalSecondsFromStart = 0;
}



void Clock_retrieveDateTimeData_intcall(void)
{
  DWORD ctime0;
  DWORD ctime1;
  DWORD ctime2;
  ctime0 = CTIME0;
  ctime1 = CTIME1;
  ctime2 = CTIME2;
  clockData.dateTime.second = LOBYTE(ctime0)&0x3f;
  clockData.dateTime.minute = LO2BYTE(ctime0)&0x3f;
  clockData.dateTime.hour = LO3BYTE(ctime0)&0x1f;
  clockData.dayOfWeek = HIBYTE(ctime0)&0x07;
  clockData.dateTime.dayOfMonth = LOBYTE(ctime1)&0x1f;
  clockData.dateTime.month = LO2BYTE(ctime1)&0x0f;
  clockData.dateTime.year = ((WORD)(HIBYTE(ctime1)&0x0f)<<8) | LO3BYTE(ctime1);
  clockData.dayOfYear = ((WORD)(LO2BYTE(ctime2)&0x01)<<8) | LOBYTE(ctime2);
  clockData.mayUpdateDateTimeView = 1;
}


char* Clock_getClockDateTimeStr(char* pStr)
{
	SAFE_DECLARE;
	DISABLE_VIC;
	sprintf((char*)pStr, "%4u.%02u.%02u %02u:%02u:%02u",
			(UINT)clockData.dateTime.year, (UINT)clockData.dateTime.month, (UINT)clockData.dateTime.dayOfMonth,
			(UINT)clockData.dateTime.hour, (UINT)clockData.dateTime.minute, (UINT)clockData.dateTime.second);
	ENABLE_VIC;
	return pStr;
}


char* Clock_getClockTimeStr(char* pStr)
{
	SAFE_DECLARE;
	DISABLE_VIC;
	//only tmie
	sprintf((char*)pStr, "%02u:%02u:%02u",
			(UINT)clockData.dateTime.hour, (UINT)clockData.dateTime.minute, (UINT)clockData.dateTime.second);
	clockData.mayUpdateDateTimeView = 0;
	ENABLE_VIC;
	return pStr;
}


void Clock_showDateTime(void)
{
	Display_setCurrentFont(fnt8x16);	//set current font

	if(clockData.bMustBeInited && (clockData.dateTime.second&0x1))
		Display_setTextColor(RED);	//set text color of clock if it needs to be inited
	else
		Display_setTextColor(YELLOW);	//set text color
	
	char str[100];
	const char* pStr;
	pStr = Clock_getClockTimeStr(str);
	Display_setTextSteps(2,1);//set steps
	Display_setTextWin(0,0,130,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextJustify(LEFT);
	Display_setTextDoubleHeight(0);
	Display_setTextWrap(0);
	
	Display_drawHLine(0,18,X_SCREEN_MAX, ORANGE);
	
	Display_clearTextWin(100);
	
	Display_outputText(pStr);
}



//get randomize number
WORD Clock_Randomize(void)
{
	DWORD dw1 = (CTIME0&0x071f3f3f)^(CTIME1&0x0fff0f1f)^(CTIME2&0xfff);
	return ((DWORD)(CTC&0xfffe)^HIWORD(dw1)^LOWORD(dw1));
}


//get string monthdayhourminutesecond
char* Clock_getClockDateTimeStrEx(char* pStr)
{
	SAFE_DECLARE;
	DISABLE_VIC;
	sprintf((char*)pStr, "%02u%02u%02u%02u%02u",
			(UINT)clockData.dateTime.month, (UINT)clockData.dateTime.dayOfMonth,
			(UINT)clockData.dateTime.hour, (UINT)clockData.dateTime.minute, (UINT)clockData.dateTime.second);
	ENABLE_VIC;
	return pStr;
}

//adjust clock
void Clock_setDateTime(int sec, int min, int hour, int dom, int dow, int doy, int month, int year)
{
	CCR_bit.CLKEN = 0;
	SEC_bit.SEC = sec;
	MIN_bit.MIN = min;
	HOUR_bit.HOUR = hour;
	DOM_bit.DOM = dom;
	DOW_bit.DOW = dow;
	DOY_bit.DOY = doy;
	MONTH_bit.MON = month;
	YEAR_bit.YEAR = year;
	CCR_bit.CLKEN = 1;
	clockData.bMustBeInited = 0;	//reset have to be inited flag
}


