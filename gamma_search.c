//sprd_mode.c



#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <inarm.h>
#include <iolpc2378.h>
#include <math.h>

#include "types.h"
#include "syncObj.h"
#include "ports.h"
#include "spectrum.h"
#include "display.h"
#include "interProc.h"
#include "keyboard.h"
#include "powerControl.h"
#include "sprd_mode.h"


struct tagSPRDModeControl SPRDModeControl;


const struct tagMode modes_SPRDMode=
{
	"sprd\0""ÒÔ‰",
	RGB(0,63,0),
	"rid\0""Ë‰",
	"mcs\0""‰Ë‡„",
	"acquir\0""Ì‡·Ó",
	"menu\0""ÏÂÌ˛",
	SPRDMode_OnActivate,
	SPRDMode_OnLeft,
	SPRDMode_OnRight,
	SPRDMode_OnUp,
	SPRDMode_OnDown,
	SPRDMode_OnIdle,
	SPRDMode_OnShow,
	SPRDMode_OnExit,
	SPRDMode_OnPowerDown,
	SPRDMode_OnWakeUp,
	SPRDMode_OnTimer
};




void SPRDMode_Init(void)
{
	;
}

BYTE SPRDMode_OnActivate(void)
{
	Modes_createTimer(300);
	return 1;
}

BYTE SPRDMode_OnTimer(void)
{
	InterProc_readAcqTime();
	Modes_OnShow();
	return 1;
}


BYTE SPRDMode_OnLeft(void)
{
	return 1;
}
BYTE SPRDMode_OnRight(void)
{
	return 1;
}
BYTE SPRDMode_OnUp(void)
{
	InterProc_resetSpectrum();
	InterProc_setAcqTime(64800);
	InterProc_startSpectrumAcq();
	return 1;
}
BYTE SPRDMode_OnDown(void)
{
	Modes_activateMenu(&spectrum_menu1);
	return 1;
}
BYTE SPRDMode_OnIdle(void)
{
	return 1;
}
BYTE SPRDMode_OnShow(void)
{
	SPRDMode_showModeScreen();
	return 1;
}
BYTE SPRDMode_OnExit(void)
{
	return 1;
}
BYTE SPRDMode_OnWakeUp(void)
{
	SPRDMode_showModeScreen();
	return 1;
}
BYTE SPRDMode_OnPowerDown(void)
{
	return 1;	//allow enter power down
}


void SPRDMode_showModeHeaders(void)
{
	Display_setTextColor(GREEN);	//set text color
	Display_setTextWin(0,267,240,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(100);
	Display_outputTextByLang("Å DOSE RATE\0""Å ÃŒŸÕŒ—“‹ ƒŒ«€");

	Display_setTextWin(0,203,240,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_clearTextWin(100);
	Display_outputTextByLang("Å COUNT PER SECOND\0""Å — Œ–Œ—“‹ —◊≈“¿");
}

void SPRDMode_showModeScreen(void)
{
	SPRDMode_showModeHeaders();
	
	SPRDMode_showDR();
	SPRDMode_showCps();
}

void SPRDMode_showCps(void)
{
	Display_setTextColor(GREEN);	//set text color
	Display_setTextWin(0,250,240,32);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(LEFT);
	Display_setCurrentFont(fnt32x32);
	
	char buf[100];
	float cps =(float)spectrumControl.dwCount;
	if(spectrumControl.acqTime)
		cps /= (float)spectrumControl.acqTime;
	
	char* pFltMsk;
	if(cps<1)
		pFltMsk = "%.3f";
	else if(cps<10)
		pFltMsk = "%.2f";
	else if(cps<100)
		pFltMsk = "%.1f";
	else
		pFltMsk = "%.0f";
	
	sprintf(buf,pFltMsk,cps);
	Display_clearTextWin(100);
	Display_outputText(buf);
	Display_setTextXY(120,0);	//set start coords in window
	Display_setCurrentFont(fnt16x16);
	Display_outputTextByLang(" cps\0"" ËÏÔ/Ò");
}

void SPRDMode_showDR(void)
{
	Display_setTextColor(GREEN);	//set text color
	Display_setTextWin(0,186,240,32);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(LEFT);
	Display_setCurrentFont(fnt32x32);
	
	char buf[100];
	float cps =(float)spectrumControl.dwCount;
	if(spectrumControl.acqTime)
		cps /= (float)spectrumControl.acqTime;
	
	char* pFltMsk;
	if(cps<1)
		pFltMsk = "%.3f";
	else if(cps<10)
		pFltMsk = "%.2f";
	else if(cps<100)
		pFltMsk = "%.1f";
	else
		pFltMsk = "%.0f";
	
	sprintf(buf,pFltMsk,cps);
	Display_clearTextWin(100);
	Display_outputText(buf);
	Display_setTextXY(120,0);	//set start coords in window
	Display_setCurrentFont(fnt16x16);
	Display_outputTextByLang(" µSv/h\0"" µ«‚/˜");
}
