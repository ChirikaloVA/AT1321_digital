//yesno_mode.c



//#include <string.h>
//#include <stdio.h>
//#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>

#include "types.h"
#include "syncObj.h"
#include "display.h"
#include "keyboard.h"
#include "powerControl.h"
#include "yesno_mode.h"
//#include "sound.h"
#include "interrupts.h"
#include "clock.h"
#include "SPRD_mode.h"


struct tagYESNOModeControl YESNOModeControl;


const struct tagMode modes_YESNOMode=
{
	YESNOMode_NameOnUpdate,//"YESNO\0""СПРД",	//mode name
	RGB(63,0,0),	//mode color
	YESNOMode_LeftOnUpdate,//"setup\0""опции",	//right
	YESNOMode_RightOnUpdate,//"rid\0""рид",	//left
	YESNOMode_UpOnUpdate,//"bkg\0""фон",//up
	YESNOMode_DownOnUpdate,//"menu\0""меню",	//down
	YESNOMode_OnActivate,
	YESNOMode_OnLeft,
	YESNOMode_OnRight,
	YESNOMode_OnUp,
	YESNOMode_OnDown,
	YESNOMode_OnIdle,
	YESNOMode_OnShow,
	YESNOMode_OnExit,
	YESNOMode_OnPowerDown,
	YESNOMode_OnWakeUp,
	YESNOMode_OnTimer
};

const char* YESNOMode_NameOnUpdate(void)//"YESNO\0""СПРД",	//mode name
{
	if(!YESNOModeControl.pModeName)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"pModeName must not be NULL");
	}
	return YESNOModeControl.pModeName;
}
const char* YESNOMode_LeftOnUpdate(void)//"setup\0""опции",	//right
{
	return "yes\0""yes\0""yes\0""да";
}
const char* YESNOMode_RightOnUpdate(void)//"rid\0""рид",	//left
{
	return "no\0""no\0""no\0""нет";
}
const char* YESNOMode_UpOnUpdate(void)//"acquir\0""набор",//up
{
	return NULL;
}
const char* YESNOMode_DownOnUpdate(void)//"menu\0""меню",	//down
{
	return NULL;
}



void YESNOMode_Init(void)
{
	YESNOModeControl.pModeName = NULL;
	YESNOModeControl.pModeQuestion = NULL;
	YESNOModeControl.pReturnFunction = NULL;
	YESNOModeControl.pReturnMode = NULL;
	YESNOModeControl.clr = RED;
	YESNOModeControl.bExitModal = FALSE;
}

BOOL YESNOMode_OnActivate(void)
{
	Modes_updateMode();
	return 1;
}

BOOL YESNOMode_OnTimer(void)
{
	return 1;
}

//YES
BOOL YESNOMode_OnLeft(void)
{
	if(!YESNOModeControl.pReturnFunction)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"pReturnFunction must not be NULL");
	}
	Modes_setActiveMode(YESNOModeControl.pReturnMode);
	(*YESNOModeControl.pReturnFunction)(1);
	YESNOModeControl.bExitModal = TRUE;
	return 1;
}
//NO
BOOL YESNOMode_OnRight(void)
{
	if(!YESNOModeControl.pReturnFunction)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"pReturnFunction must not be NULL");
	}
	Modes_setActiveMode(YESNOModeControl.pReturnMode);
	(*YESNOModeControl.pReturnFunction)(0);
	YESNOModeControl.bExitModal = TRUE;
	return 1;
}
BOOL YESNOMode_OnUp(void)
{
	return 1;
}
BOOL YESNOMode_OnDown(void)
{
	return 1;
}
BOOL YESNOMode_OnIdle(void)
{
	return 1;
}
BOOL YESNOMode_OnShow(void)
{
	YESNOMode_showModeScreen();
	return 1;
}
BOOL YESNOMode_OnExit(void)
{
	return 1;
}
BOOL YESNOMode_OnWakeUp(void)
{
//	YESNOMode_showModeScreen();
	return 1;
}
BOOL YESNOMode_OnPowerDown(void)
{
	return 0;	//disable enter power down
}




void YESNOMode_showModeScreen(void)
{
	if(!YESNOModeControl.pModeQuestion)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"pModeQuestion must not be NULL");
	}
	Display_setTextColor(YESNOModeControl.clr);	//set text color
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(10);
	Display_outputTextByLang(YESNOModeControl.pModeQuestion);
}


//start modal mode of question
void YESNOMode_DoModal(COLORREF clr, const char* pModeName, const char* pModeQuestion, void (*pReturnFunction)(BOOL bYes))
{
	YESNOModeControl.bExitModal = FALSE;
	YESNOModeControl.clr = clr;
	YESNOModeControl.pModeName =  pModeName;
	YESNOModeControl.pModeQuestion = pModeQuestion;
	YESNOModeControl.pReturnFunction = pReturnFunction;
	YESNOModeControl.pReturnMode = modeControl.pMode;
	Modes_setActiveMode(&modes_YESNOMode);
	
	YESNOMode_modalProcedure();
}

void YESNOMode_modalProcedure(void)
{
	while(!YESNOModeControl.bExitModal)
	{
		if(clockData.mayUpdateDateTimeView)
		{//once a second
			PowerControl_kickWatchDog();
			///////////////
			if(powerControl.dwBatteryAlarmCnt>BAT_LOW_TIME_TO_OFF)
			{//turn off device by bat low status
				PowerControl_turnOFF_device("Battery low!\0""Battery low!\0""Battery low!\0""Батареи разряжены!");
			}
			//show status line and reset clockData.mayUpdateDateTimeView to 0
			Display_showStatusLine();
		}
		Modes_OnIdle();
		//Sys Timer processing
		if(modeControl.bTimerON &&
		   modeControl.dwTimerReach)
		{
			modeControl.dwTimerReach = 0;
			Modes_OnTimer();
		}
		if(KeyboardControl_keyboardControl())
		{//some key was processed
			//after key processing
			powerControl.dwIdleTime = 0;	//reset idle time counter after key processing
		}
	}
}


//start modal mode of question
void YESNOMode_DoNotModal(COLORREF clr, const char* pModeName, const char* pModeQuestion, void (*pReturnFunction)(BOOL bYes))
{
	YESNOModeControl.bExitModal = FALSE;
	YESNOModeControl.clr = clr;
	YESNOModeControl.pModeName =  pModeName;
	YESNOModeControl.pModeQuestion = pModeQuestion;
	YESNOModeControl.pReturnFunction = pReturnFunction;
	if(modeControl.pMode!=&modes_YESNOMode)
		YESNOModeControl.pReturnMode = modeControl.pMode;
	Modes_setActiveMode(&modes_YESNOMode);
}


