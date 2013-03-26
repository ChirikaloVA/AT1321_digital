#ifndef _COMMMODE_H
#define _COMMMODE_H

#include "types.h"
#include "modes.h"


struct tagCOMMModeControl
{
	//number of received and transmitted bytes by bluetooth
	volatile DWORD dwReceived;
	volatile DWORD dwTransmitted;
};

extern struct tagCOMMModeControl COMMModeControl;

extern const struct tagMode modes_COMMMode;



void COMMMode_Init(void);


BOOL COMMMode_OnActivate(void);
BOOL COMMMode_OnLeft(void);
BOOL COMMMode_OnRight(void);
BOOL COMMMode_OnUp(void);
BOOL COMMMode_OnDown(void);
BOOL COMMMode_OnIdle(void);
BOOL COMMMode_OnShow(void);
BOOL COMMMode_OnExit(void);
BOOL COMMMode_OnPowerDown(void);
BOOL COMMMode_OnWakeUp(void);
BOOL COMMMode_OnTimer(void);
void COMMMode_showModeScreen(void);
void COMMMode_showModeHeaders(void);




void COMMMode_setActiveMode(void);
//int COMMMode_getMenuItemsAr(void);
BOOL COMMMode_menu1_power_control(void);
BOOL COMMMode_menu1_change_state(void);


const char* COMMMode_menu1_change_state_onUpdate(void);
const char* COMMMode_menu1_power_control_onUpdate(void);

const char* COMMMode_NameOnUpdate(void);//"COMM\0""—¬я«№",	//mode name
const char* COMMMode_LeftOnUpdate(void);//"back\0""назад",	//left
const char* COMMMode_RightOnUpdate(void);//"comm\0""обмен",	//right
const char* COMMMode_UpOnUpdate(void);//"on\0""вкл",//up
const char* COMMMode_DownOnUpdate(void);//"menu\0""меню",	//down

void COMMMode_showTransmitted(void);
void COMMMode_showReceived(void);



#endif	//ifndef _COMMMODE_H
