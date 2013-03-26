//usb_mode.c



//#include <string.h>
#include <stdio.h>
//#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
//#include <math.h>

#include "types.h"
//#include "syncObj.h"
#include "display.h"
#include "interProc.h"
//#include "keyboard.h"
//#include "powerControl.h"
#include "usb_mode.h"
#include "usbrs.h"
#include "interrupts.h"
#include "SPRD_mode.h"

struct tagUSBModeControl USBModeControl;


const struct tagMode modes_USBMode=
{
	USBMode_NameOnUpdate,//"COMM\0""—¬я«№",	//mode name
	RGB(40,60,0),	//mode color
	USBMode_LeftOnUpdate,//"back\0""назад",	//left
	USBMode_RightOnUpdate,//"comm\0""обмен",	//right
	USBMode_UpOnUpdate,//"on\0""вкл",//up
	USBMode_DownOnUpdate,//"menu\0""меню",	//down
	USBMode_OnActivate,
	USBMode_OnLeft,
	USBMode_OnRight,
	USBMode_OnUp,
	USBMode_OnDown,
	USBMode_OnIdle,
	USBMode_OnShow,
	USBMode_OnExit,
	USBMode_OnPowerDown,
	USBMode_OnWakeUp,
	USBMode_OnTimer
};








/*
---------
SPRD > modem
PC > SPRD
---------
Manually on/off
ON while active
----------

*/






const char* USBMode_NameOnUpdate(void)//"COMM\0""—¬я«№",	//mode name
{
	return "PC LINK\0""PC LINK\0""PC LINK\0""ѕ  —¬я«№";
}
const char* USBMode_LeftOnUpdate(void)//"back\0""назад",	//left
{
	return "exit\0""exit\0""exit\0""выход";
}
const char* USBMode_RightOnUpdate(void)//"comm\0""обмен",	//right
{
	return NULL;
}
const char* USBMode_UpOnUpdate(void)//"on\0""вкл",//up
{
	return NULL;
}
const char* USBMode_DownOnUpdate(void)//"menu\0""меню",	//down
{
	return NULL;
}






void USBMode_Init(void)
{
	USBModeControl.dwReceived = 0;
	USBModeControl.dwTransmitted = 0;
	USBModeControl.bEnterThisMode = TRUE;
}

BOOL USBMode_OnActivate(void)
{
	Modes_createTimer(250);
	USBMode_Init();
	Modes_updateMode();
	return 1;
}

BOOL USBMode_OnTimer(void)
{
	Modes_OnShow();
	return 1;
}


BOOL USBMode_OnLeft(void)
{
	USBModeControl.bEnterThisMode = FALSE;
	Modes_setActiveMode(&modes_SPRDMode);
	return 1;
}
BOOL USBMode_OnRight(void)
{
	return 1;
}
BOOL USBMode_OnUp(void)
{
	return 1;
}
BOOL USBMode_OnDown(void)
{
	return 1;
}
BOOL USBMode_OnIdle(void)
{
	return 1;
}
BOOL USBMode_OnShow(void)
{
	USBMode_showModeScreen();
	return 1;
}
BOOL USBMode_OnExit(void)
{
	return 1;
}
BOOL USBMode_OnWakeUp(void)
{
	return 1;
}
BOOL USBMode_OnPowerDown(void)
{
	return 0;	//disable enter power down
}


void USBMode_showModeHeaders(void)
{
	Display_setTextColor(GREEN);	//set text color
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(10);
	Display_outputTextByLang("Received:\0""Received:\0""Received:\0""ѕрин€то:");
	Display_drawHLine(0,Y_SCREEN_MAX-216,X_SCREEN_MAX,RGB(0,32,0));

	Display_setTextColor(GREEN);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-214,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_clearTextWin(250);
	Display_outputTextByLang("Transmitted:\0""Transmitted:\0""Transmitted:\0""ѕередано:");
	Display_drawHLine(0,Y_SCREEN_MAX-164,X_SCREEN_MAX,RGB(0,32,0));
}

void USBMode_showModeScreen(void)
{
	USBMode_showModeHeaders();
	USBMode_showReceived();
	USBMode_showTransmitted();
}







void USBMode_showReceived(void)
{
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-249,X_SCREEN_SIZE,32);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt32x32);
	
	char buf[20];
	if(USBModeControl.dwReceived<1024)
		sprintf(buf,"%u",(UINT)USBModeControl.dwReceived);
	else
		sprintf(buf,"%u",(UINT)USBModeControl.dwReceived/1024);
	Display_clearTextWin(250);
	Display_outputText(buf);
	Display_setTextXY(190,16);	//set start coords in window
	Display_setCurrentFont(fnt16x16);
	if(USBModeControl.dwReceived<1024)
		Display_outputText(" B");
	else
		Display_outputText(" kB");
}

void USBMode_showTransmitted(void)
{
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-197,X_SCREEN_SIZE,32);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt32x32);
	
	char buf[20];
	if(USBModeControl.dwTransmitted<1024)
		sprintf(buf,"%u",(UINT)USBModeControl.dwTransmitted);
	else
		sprintf(buf,"%u",(UINT)USBModeControl.dwTransmitted/1024);
	Display_clearTextWin(200);
	Display_outputText(buf);
	Display_setTextXY(190,16);	//set start coords in window
	Display_setCurrentFont(fnt16x16);
	if(USBModeControl.dwTransmitted<1024)
		Display_outputText(" B");
	else
		Display_outputText(" kB");
}
