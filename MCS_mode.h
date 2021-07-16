#ifndef _MCSMODE_H
#define _MCSMODE_H

#include "types.h"
#include "modes.h"
#include "filesystem.h"

//time of spectrum acquiring in MCS mode
#define AUTO_SPEC_TIME 10

#define MAX_MCS 30

#pragma pack(1)
struct tagMCSData
{
	DWORD dwMomCPS;	//32bit = 1 means alarm case
	float fDR;	//gamma dose rate in uSv/h
	BYTE btError;	//statistical error in %
	float fNCPS;	//neutron cps
};
#pragma pack()

struct tagMCSModeControl
{
//	BOOL bSaveSpectrum;	//TRUe means its time to save spectrum
	struct tagMCSData mcs[MAX_MCS];	//временный буфер скоростей счета
	int iNumberOfCps;
	BYTE savedSeconds;	//to get spectrum every second
//	int iNumberOfCpsTotal;
	HFILE hfile_tmp;	//mcs temp file
	BOOL bSavingON;	//true means save all data in memory
unsigned char specar_name[FILE_NAME_SZ];	//name of auto saved spectrum array file
//	struct tagDateTime dateTime;	//date and time of start data collection
//	struct tagCommonGPS commonGPS;	//gps of start data collection
};

extern struct tagMCSModeControl MCSModeControl;

extern const struct tagMode modes_MCSMode;

extern const struct tagMenu MCS_menu;


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
BOOL MCSMode_menu1_SaveRowValues(void);
const char* MCSMode_menu1_SaveRowValues_onUpdate(void);

BOOL MCSMode_saveArSpec(void);
void MCSMode_saveValue(void);
BOOL MCSMode_menu1_SaveValue(void);

//BOOL MCSMode_menu1_SUBmode(void);
//const char* MCSMode_menu1_SUBmode_onUpdate(void);

#endif	//ifndef _MCSMODE_H
