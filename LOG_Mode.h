#ifndef _LOGMODE_H
#define _LOGMODE_H

#include "types.h"
#include "modes.h"
#include "ini_control.h"



struct tagLOGModeControl
{
	int iPage;
	int iPos;
	int iPosCur;
	char txtPage[MAX_INI_STRING_LEN];
//	char log_name[FILE_NAME_SZ];	//name of auto saved log
	HFILE hfile_log;	//file log
};

extern struct tagLOGModeControl LOGModeControl;

extern const struct tagMode modes_LOGMode;





void LOGMode_Init(void);


BOOL LOGMode_OnActivate(void);
BOOL LOGMode_OnLeft(void);
BOOL LOGMode_OnRight(void);
BOOL LOGMode_OnUp(void);
BOOL LOGMode_OnDown(void);
BOOL LOGMode_OnIdle(void);
BOOL LOGMode_OnShow(void);
BOOL LOGMode_OnExit(void);
BOOL LOGMode_OnPowerDown(void);
BOOL LOGMode_OnWakeUp(void);
BOOL LOGMode_OnTimer(void);
void LOGMode_showModeScreen(void);

const char* LOGMode_NameOnUpdate(void);//"LOG\0""ќѕ÷»»",	//mode name
const char* LOGMode_LeftOnUpdate(void);//"back\0""назад",	//left
const char* LOGMode_RightOnUpdate(void);//"gps\0""gps",	//right
const char* LOGMode_UpOnUpdate(void);//"acquir\0""набор",//up
const char* LOGMode_DownOnUpdate(void);//"menu\0""меню",	//down




BOOL LOGMode_insertEventByLang(const char* pEvent);
BOOL LOGMode_insertEvent(const char* pEvent);
BOOL LOGMode_createLog(void);


BOOL LOGMode_insertDoserate(float val);
BOOL LOGMode_insertGPS(void);


#endif	//ifndef _LOGMODE_H
