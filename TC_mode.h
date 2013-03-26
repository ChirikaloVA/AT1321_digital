#ifndef _TCMODE_H
#define _TCMODE_H

#include "types.h"
#include "modes.h"


struct tagTCModeControl
{
	DWORD dwCount;
	DWORD dwTimer;
	float fsigm;
	BOOL bRun;	//if TRUE then count
};

extern struct tagTCModeControl TCModeControl;

extern const struct tagMode modes_TCMode;



void TCMode_Init(void);


BOOL TCMode_OnActivate(void);
BOOL TCMode_OnLeft(void);
BOOL TCMode_OnRight(void);
BOOL TCMode_OnUp(void);
BOOL TCMode_OnDown(void);
BOOL TCMode_OnIdle(void);
BOOL TCMode_OnShow(void);
BOOL TCMode_OnExit(void);
BOOL TCMode_OnPowerDown(void);
BOOL TCMode_OnWakeUp(void);
BOOL TCMode_OnTimer(void);
void TCMode_showModeScreen(void);
void TCMode_showModeHeaders(void);
void TCMode_showTimer(void);
void TCMode_showCount(void);


const char* TCMode_NameOnUpdate(void);
const char* TCMode_LeftOnUpdate(void);
const char* TCMode_RightOnUpdate(void);
const char* TCMode_UpOnUpdate(void);
const char* TCMode_DownOnUpdate(void);


void TCMode_showCps(void);

void TCMode_showDose(void);
void TCMode_showBkg(void);
void TCMode_TCSearch(void);


#endif	//ifndef _TCMODE_H
