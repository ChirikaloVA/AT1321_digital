//tc_mode.c



#include <string.h>
#include <stdio.h>
//#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <math.h>

#include "types.h"
//#include "syncObj.h"
//#include "spectrum.h"
#include "display.h"
#include "interProc.h"
//#include "keyboard.h"
//#include "powerControl.h"
#include "mcs_mode.h"
#include "tc_mode.h"
#include "sprd_mode.h"
#include "SETUP_Mode.h"
#include "sound.h"
#include "rid.h"


struct tagTCModeControl TCModeControl;


const struct tagMode modes_TCMode=
{
	TCMode_NameOnUpdate,//"TC\0""—◊≈“",	//mode name
	RGB(0,63,0),	//mode color
	TCMode_LeftOnUpdate,//"back\0""Ì‡Á‡‰",	//left
	TCMode_RightOnUpdate,//"mcs\0""‰Ë‡„",	//right
	TCMode_UpOnUpdate,//"acquir\0""Ì‡·Ó",//up
	TCMode_DownOnUpdate,//"menu\0""ÏÂÌ˛",	//down
	TCMode_OnActivate,
	TCMode_OnLeft,
	TCMode_OnRight,
	TCMode_OnUp,
	TCMode_OnDown,
	TCMode_OnIdle,
	TCMode_OnShow,
	TCMode_OnExit,
	TCMode_OnPowerDown,
	TCMode_OnWakeUp,
	TCMode_OnTimer
};




const char* TCMode_NameOnUpdate(void)//"TC\0""—◊≈“",	//mode name
{
	return "TC\0""TC\0""TC\0""—◊≈“";
}
const char* TCMode_LeftOnUpdate(void)//"back\0""Ì‡Á‡‰",	//left
{
	return "back\0""back\0""back\0""Ì‡Á‡‰";
}
const char* TCMode_RightOnUpdate(void)//"mcs\0""‰Ë‡„",	//right
{
#ifdef _IAEA
	return "expert\0""expert\0""expert\0""˝ÍÒÔÂÚ";
#else
	return "rid\0""rid\0""rid\0""Ë‰";
#endif
}
const char* TCMode_UpOnUpdate(void)//"acquir\0""Ì‡·Ó",//up
{
	if(TCModeControl.bRun)
		return "stop\0""stop\0""stop\0""ÒÚÓÔ";
	else
		return "start\0""start\0""start\0""ÒÚ‡Ú";
}
const char* TCMode_DownOnUpdate(void)//"menu\0""ÏÂÌ˛",	//down
{
	return NULL;//"menu\0""ÏÂÌ˛";
}




void TCMode_Init(void)
{
	TCModeControl.dwCount = 0;
	TCModeControl.dwTimer = 0;
	TCModeControl.bRun = TRUE;
	TCModeControl.fsigm = SPRDModeControl.operation_search_sigma[0];//SETUPMode_calcSigmaEx(SPRDModeControl.false_alarm_period_oper, 0.3);
	SPRDModeControl.fDose = 0;
}

BOOL TCMode_OnActivate(void)
{
	Modes_createTimer(333);
	TCMode_Init();
	InterProc_resetDose();
	Modes_updateMode();
	return 1;
}



//control of Timer Counter search mode
void TCMode_TCSearch(void)
{
	float sigm, bkg, cur, ftime = (float)TCModeControl.dwTimer/3.0;
	//background
	//interProcControl.rsModbus.fBkgCPS
	bkg = interProcControl.rsModbus.fBkgCPS*ftime;
	if(bkg==0)return;
	//current count
	cur = (float)TCModeControl.dwCount;
	sigm = (cur-bkg)/sqrt(bkg);
	if(sigm>0)
	{
		if(sigm>=TCModeControl.fsigm)
		{//alarm!
			SET_SND_AU;
			SoundControl_Alarm_intcall(50, 1000);
		}else
		{//no alarm
			;		
		}
	}else
	{//no alarm
		;
	}
}


BOOL TCMode_OnTimer(void)
{
	if(TCModeControl.bRun)
	{
		InterProc_readMeasurementRegs();
		TCMode_TCSearch();
		if((TCModeControl.dwTimer%3)==0)
		{
			Modes_OnShow();
		}
	}
	return 1;
}


BOOL TCMode_OnLeft(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return 1;
}
BOOL TCMode_OnRight(void)
{
	Modes_setActiveMode(&modes_RID_Mode);
	return 1;
}
BOOL TCMode_OnUp(void)
{
	if(TCModeControl.bRun)
	{//stop
		TCModeControl.bRun = FALSE;
		Modes_showButtons();
		return 1;	//just exit without screen update
	}else
	{//reset
		InterProc_resetDose();
		TCMode_Init();
	}
	Modes_updateMode();
	return 1;
}
BOOL TCMode_OnDown(void)
{
	return 1;
}
BOOL TCMode_OnIdle(void)
{
	return 1;
}
BOOL TCMode_OnShow(void)
{
	TCMode_showModeScreen();
	return 1;
}
BOOL TCMode_OnExit(void)
{
	return 1;
}
BOOL TCMode_OnWakeUp(void)
{
	return 1;
}
BOOL TCMode_OnPowerDown(void)
{
	return 0;	//disable enter power down
}


void TCMode_showModeHeaders(void)
{
	Display_setTextColor(ORANGE);	//set text color
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setCurrentFont(fnt16x16);
	Display_setTextJustify(CENTER);

	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_clearTextWin(70);
	Display_outputTextByLang("TIMER\0""TIMER\0""TIMER\0""“¿…Ã≈–");
	Display_drawHLine(0,Y_SCREEN_MAX-216,X_SCREEN_MAX,RGB(0,32,0));

	Display_setTextWin(0,Y_SCREEN_MAX-214,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_clearTextWin(250);
	Display_outputTextByLang("Å COUNTER\0""Å COUNTER\0""Å COUNTER\0""Å —◊≈“◊» ");
	Display_drawHLine(0,Y_SCREEN_MAX-164,X_SCREEN_MAX,RGB(0,32,0));
	
	Display_setTextWin(0,Y_SCREEN_MAX-162,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_clearTextWin(200);
	Display_outputTextByLang("Å COUNTS PER SECOND\0""Å COUNTS PER SECOND\0""Å COUNTS PER SECOND\0""Å — Œ–Œ—“‹ —◊≈“¿");
	Display_drawHLine(0,Y_SCREEN_MAX-112,X_SCREEN_MAX,RGB(0,32,0));

	Display_setTextWin(0,Y_SCREEN_MAX-110,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_clearTextWin(50);
	Display_outputTextByLang("Å DOSE\0""Å DOSE\0""Å DOSE\0""Å ƒŒ«¿");
	Display_drawHLine(0,Y_SCREEN_MAX-60,X_SCREEN_MAX,RGB(0,32,0));
	
}

void TCMode_showModeScreen(void)
{
	TCMode_showModeHeaders();
	
	TCMode_showTimer();
	TCMode_showCount();
	TCMode_showCps();
	TCMode_showDose();
	TCMode_showBkg();
}




void TCMode_showBkg(void)
{
	Display_setTextColor(ORANGE);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-51,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt16x16);
	
	char buf[20];
	Display_clearTextWin(200);
	Display_outputTextByLang("Background: \0""Background: \0""Background: \0""‘ÓÌ: ");
	sprintf(buf,"%.1f ",interProcControl.rsModbus.fBkgCPS);
	Display_outputText(buf);
	Display_outputText("cps");
}


void TCMode_showCps(void)
{
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-145,X_SCREEN_SIZE,32);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt32x32);
	
	char buf[20];
	if(TCModeControl.dwTimer)
		sprintf(buf,"%.1f",(float)TCModeControl.dwCount/((float)TCModeControl.dwTimer/3.0));
	else
		strcpy(buf, "-");
	Display_clearTextWin(200);
	Display_outputText(buf);
	Display_setTextXY(180,16);	//set start coords in window
	Display_setCurrentFont(fnt16x16);
	Display_outputText("cps");
	Display_setTextXY(180,0);
	UINT erval = 200;
	if(TCModeControl.dwCount)
		erval = (UINT)(200.0/sqrt(TCModeControl.dwCount)+0.5);
	if(erval<1)
		erval = 1;
	sprintf(buf,"%u%%",erval);
	Display_outputText(buf);
}

void TCMode_showCount(void)
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
	sprintf(buf,"%u",TCModeControl.dwCount);
	Display_clearTextWin(200);
	Display_outputText(buf);
	Display_setTextXY(180,16);	//set start coords in window
	Display_setCurrentFont(fnt16x16);
	Display_outputText("cnt");
}

void TCMode_showTimer(void)
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
	sprintf(buf,"%u",(DWORD)(TCModeControl.dwTimer/3));
	Display_clearTextWin(250);
	Display_outputText(buf);
	Display_setTextXY(180,16);	//set start coords in window
	Display_setCurrentFont(fnt16x16);
	Display_outputText("s");
}

void TCMode_showDose(void)
{
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-93,X_SCREEN_SIZE,32);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt32x32);
	
	char buf[20];
		
	float val =SPRDModeControl.fDose;
	
	const char* pFltMsk;
	if(val<1)
		pFltMsk = "%.3f";
	else if(val<10)
		pFltMsk = "%.2f";
	else if(val<100)
		pFltMsk = "%.1f";
	else
		pFltMsk = "%.0f";
	sprintf(buf,pFltMsk,SPRDModeControl.fDose);
	Display_clearTextWin(100);
	Display_outputText(buf);
	Display_setTextXY(180,16);	//set start coords in window
	Display_setCurrentFont(fnt16x16);
	Display_outputTextByLang(SPRDMode_getDimensionDose());
}
