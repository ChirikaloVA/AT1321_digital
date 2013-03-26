#ifndef _YESNOMODE_H
#define _YESNOMODE_H

#include "types.h"
#include "modes.h"




struct tagYESNOModeControl
{
	COLORREF clr;
	const char* pModeName;
	const char* pModeQuestion;
	void (*pReturnFunction)(BOOL bYes);
	const struct tagMode * pReturnMode;
	BOOL bExitModal;	//if true then exec will exit from dead cycle
};

extern struct tagYESNOModeControl YESNOModeControl;

extern const struct tagMode modes_YESNOMode;



void YESNOMode_Init(void);


BOOL YESNOMode_OnActivate(void);
BOOL YESNOMode_OnLeft(void);
BOOL YESNOMode_OnRight(void);
BOOL YESNOMode_OnUp(void);
BOOL YESNOMode_OnDown(void);
BOOL YESNOMode_OnIdle(void);
BOOL YESNOMode_OnShow(void);
BOOL YESNOMode_OnExit(void);
BOOL YESNOMode_OnPowerDown(void);
BOOL YESNOMode_OnWakeUp(void);
BOOL YESNOMode_OnTimer(void);
void YESNOMode_showModeScreen(void);


const char* YESNOMode_NameOnUpdate(void);//"YESNO\0""СПРД",	//mode name
const char* YESNOMode_LeftOnUpdate(void);//"setup\0""опции",	//right
const char* YESNOMode_RightOnUpdate(void);//"rid\0""рид",	//left
const char* YESNOMode_UpOnUpdate(void);//"acquir\0""набор",//up
const char* YESNOMode_DownOnUpdate(void);//"menu\0""меню",	//down

void YESNOMode_DoModal(COLORREF clr, const char* pModeName, const char* pModeQuestion, void (*pReturnFunction)(BOOL bYes));
void YESNOMode_DoNotModal(COLORREF clr, const char* pModeName, const char* pModeQuestion, void (*pReturnFunction)(BOOL bYes));
void YESNOMode_modalProcedure(void);

#endif	//ifndef _YESNOMODE_H
