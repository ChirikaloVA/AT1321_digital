#ifndef _INFO_H
#define _INFO_H

#include "types.h"
#include "modes.h"



struct tagINFOModeControl
{
	int iLine;	//line to output
	char text[600];
};

extern struct tagINFOModeControl INFOModeControl;

extern const struct tagMode modes_INFOMode;

void INFOMode_Init(void);

BOOL INFOMode_OnActivate(void);
BOOL INFOMode_OnLeft(void);
BOOL INFOMode_OnRight(void);
BOOL INFOMode_OnUp(void);
BOOL INFOMode_OnDown(void);
BOOL INFOMode_OnIdle(void);
BOOL INFOMode_OnShow(void);
BOOL INFOMode_OnExit(void);
BOOL INFOMode_OnPowerDown(void);
BOOL INFOMode_OnWakeUp(void);
BOOL INFOMode_OnTimer(void);
void INFOMode_showModeScreen(void);

const char* INFOMode_NameOnUpdate(void);
const char* INFOMode_LeftOnUpdate(void);
const char* INFOMode_RightOnUpdate(void);
const char* INFOMode_UpOnUpdate(void);
const char* INFOMode_DownOnUpdate(void);







#endif	//ifndef _INFO_H
