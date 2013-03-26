#ifndef _USBMODE_H
#define _USBMODE_H

#include "types.h"
#include "modes.h"


struct tagUSBModeControl
{
	//number of received and transmitted bytes by bluetooth
	volatile DWORD dwReceived;
	volatile DWORD dwTransmitted;
	BOOL bEnterThisMode;	//если 1 то войдет в этот режим при обмене, если 0 то не войдет
};

extern struct tagUSBModeControl USBModeControl;

extern const struct tagMode modes_USBMode;



void USBMode_Init(void);


BOOL USBMode_OnActivate(void);
BOOL USBMode_OnLeft(void);
BOOL USBMode_OnRight(void);
BOOL USBMode_OnUp(void);
BOOL USBMode_OnDown(void);
BOOL USBMode_OnIdle(void);
BOOL USBMode_OnShow(void);
BOOL USBMode_OnExit(void);
BOOL USBMode_OnPowerDown(void);
BOOL USBMode_OnWakeUp(void);
BOOL USBMode_OnTimer(void);
void USBMode_showModeScreen(void);
void USBMode_showModeHeaders(void);





const char* USBMode_NameOnUpdate(void);//"COMM\0""—¬я«№",	//mode name
const char* USBMode_LeftOnUpdate(void);//"back\0""назад",	//left
const char* USBMode_RightOnUpdate(void);//"comm\0""обмен",	//right
const char* USBMode_UpOnUpdate(void);//"on\0""вкл",//up
const char* USBMode_DownOnUpdate(void);//"menu\0""меню",	//down

void USBMode_showTransmitted(void);
void USBMode_showReceived(void);



#endif	//ifndef _USBMODE_H
