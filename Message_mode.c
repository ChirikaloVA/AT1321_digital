//info_mode.c

#include <string.h>
#include "interrupts.h"
#include "syncObj.h"
#include "message_mode.h"
#include "stab_mode.h"
#include "display.h"



struct tagMessageModeControl MessageModeControl;


const struct tagMode modes_MessageMode=
{
	MessageMode_NameOnUpdate,//
	RGB(40,60,0),	//mode color
	MessageMode_LeftOnUpdate,//
	MessageMode_RightOnUpdate,//
	MessageMode_UpOnUpdate,//
	MessageMode_DownOnUpdate,//
	MessageMode_OnActivate,
	MessageMode_OnLeft,
	MessageMode_OnRight,
	MessageMode_OnUp,
	MessageMode_OnDown,
	MessageMode_OnIdle,
	MessageMode_OnShow,
	MessageMode_OnExit,
	MessageMode_OnPowerDown,
	MessageMode_OnWakeUp,
	MessageMode_OnTimer
};




const char* MessageMode_NameOnUpdate(void)//
{
	return "MESSAGE\0""MESSAGE\0""MESSAGE\0""—ќќЅў≈Ќ.";
}
const char* MessageMode_LeftOnUpdate(void)//
{
	return "exit\0""exit\0""exit\0""выход";
}
const char* MessageMode_RightOnUpdate(void)//
{
	return "exit\0""exit\0""exit\0""выход";
}
const char* MessageMode_UpOnUpdate(void)//
{
	return "exit\0""exit\0""exit\0""выход";
}
const char* MessageMode_DownOnUpdate(void)//
{
	return "exit\0""exit\0""exit\0""выход";
}









void MessageMode_Init(void)
{
	memset(&MessageModeControl,0,sizeof(MessageModeControl));
}



BOOL MessageMode_OnActivate(void)
{
	MessageModeControl.iTimer = 5;
	Modes_createTimer(1000);
	Modes_updateMode();	
	return 1;
}

BOOL MessageMode_OnTimer(void)
{
	Modes_OnShow();
	if(!--MessageModeControl.iTimer)
	{
		if(MessageModeControl.pRetFunction==NULL)exception(__FILE__, __FUNCTION__, __LINE__, "MessageModeControl internal error!");
		(*MessageModeControl.pRetFunction)();
	}
	return 1;
}


BOOL MessageMode_OnLeft(void)
{
	if(MessageModeControl.pRetFunction==NULL)exception(__FILE__, __FUNCTION__, __LINE__, "MessageModeControl internal error!");
	(*MessageModeControl.pRetFunction)();
	return 1;
}
BOOL MessageMode_OnRight(void)
{
	if(MessageModeControl.pRetFunction==NULL)exception(__FILE__, __FUNCTION__, __LINE__, "MessageModeControl internal error!");
	(*MessageModeControl.pRetFunction)();
	return 1;
}
BOOL MessageMode_OnUp(void)
{
	if(MessageModeControl.pRetFunction==NULL)exception(__FILE__, __FUNCTION__, __LINE__, "MessageModeControl internal error!");
	(*MessageModeControl.pRetFunction)();
	return 1;
}
BOOL MessageMode_OnDown(void)
{
	if(MessageModeControl.pRetFunction==NULL)exception(__FILE__, __FUNCTION__, __LINE__, "MessageModeControl internal error!");
	(*MessageModeControl.pRetFunction)();
	return 1;
}
BOOL MessageMode_OnIdle(void)
{
	return 1;
}
BOOL MessageMode_OnShow(void)
{
	MessageMode_showModeScreen();
	return 1;
}
BOOL MessageMode_OnExit(void)
{
	return 1;
}
BOOL MessageMode_OnWakeUp(void)
{
	return 1;
}
BOOL MessageMode_OnPowerDown(void)
{
	return 1;	//allow enter power down
}







void MessageMode_showModeScreen(void)
{
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_setTextLineClear(1);
	Display_checkForClearLine();
	
	if(MessageModeControl.pDrawFunction==NULL)exception(__FILE__, __FUNCTION__, __LINE__, "MessageModeControl internal error!");
	(*MessageModeControl.pDrawFunction)(MessageModeControl.pParam);
	
	Display_clearUserPart();
	
	Display_setTextLineClear(0);
}


