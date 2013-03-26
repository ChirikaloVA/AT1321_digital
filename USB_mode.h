#ifndef _USBMODE_H
#define _USBMODE_H

#include "types.h"
#include "modes.h"


struct tagUSBModeControl
{
	//number of received and transmitted bytes by bluetooth
	volatile DWORD dwReceived;
	volatile DWORD dwTransmitted;
	BOOL bEnterThisMode;	//���� 1 �� ������ � ���� ����� ��� ������, ���� 0 �� �� ������
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





const char* USBMode_NameOnUpdate(void);//"COMM\0""�����",	//mode name
const char* USBMode_LeftOnUpdate(void);//"back\0""�����",	//left
const char* USBMode_RightOnUpdate(void);//"comm\0""�����",	//right
const char* USBMode_UpOnUpdate(void);//"on\0""���",//up
const char* USBMode_DownOnUpdate(void);//"menu\0""����",	//down

void USBMode_showTransmitted(void);
void USBMode_showReceived(void);



#endif	//ifndef _USBMODE_H
