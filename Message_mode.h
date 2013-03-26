#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "types.h"
#include "modes.h"



struct tagMessageModeControl
{
	int iTimer;	//timer to close dialog
	void(*pRetFunction)(void);	//ret function after edit, it must switch to neccessary mode
	void(*pDrawFunction)(const void* pParam);
	const void* pParam;	//parameter of draw function
};

extern struct tagMessageModeControl MessageModeControl;

extern const struct tagMode modes_MessageMode;

void MessageMode_Init(void);

BOOL MessageMode_OnActivate(void);
BOOL MessageMode_OnLeft(void);
BOOL MessageMode_OnRight(void);
BOOL MessageMode_OnUp(void);
BOOL MessageMode_OnDown(void);
BOOL MessageMode_OnIdle(void);
BOOL MessageMode_OnShow(void);
BOOL MessageMode_OnExit(void);
BOOL MessageMode_OnPowerDown(void);
BOOL MessageMode_OnWakeUp(void);
BOOL MessageMode_OnTimer(void);
void MessageMode_showModeScreen(void);

const char* MessageMode_NameOnUpdate(void);
const char* MessageMode_LeftOnUpdate(void);
const char* MessageMode_RightOnUpdate(void);
const char* MessageMode_UpOnUpdate(void);
const char* MessageMode_DownOnUpdate(void);







#endif	//ifndef _MESSAGE_H
