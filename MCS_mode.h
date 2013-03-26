#ifndef _MCSMODE_H
#define _MCSMODE_H

#include "types.h"
#include "modes.h"

#define MAX_MCS 30

struct tagMCSModeControl
{
	BOOL bSaveSpectrum;
	DWORD mcs[MAX_MCS];	//временный буфер скоростей счета
	int iNumberOfCps;
	int iNumberOfCpsTotal;
	HFILE hfile_tmp;	//mcs temp file
};

extern struct tagMCSModeControl MCSModeControl;

extern const struct tagMode modes_MCSMode;



void MCSMode_Init(void);

BOOL MCSMode_OnExit(void);

void MCSMode_createTempFile(void);

BOOL MCSMode_OnActivate(void);
BOOL MCSMode_OnLeft(void);
BOOL MCSMode_OnRight(void);
BOOL MCSMode_OnDown(void);
BOOL MCSMode_OnShow(void);
BOOL MCSMode_OnTimer(void);
BOOL MCSMode_OnPowerDown(void);
BOOL MCSMode_OnUp(void);

const char* MCSMode_NameOnUpdate(void);//"MCS\0""ƒ»ј√–јћћј",	//mode name
const char* MCSMode_LeftOnUpdate(void);//"back\0""назад",	//left
const char* MCSMode_RightOnUpdate(void);//"setup\0""опции",	//right
const char* MCSMode_DownOnUpdate(void);//"menu\0""меню",	//down


void MCSMode_saveMCS(void);
void MCSMode_prepare(void);




#endif	//ifndef _MCSMODE_H
