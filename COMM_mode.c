//comm_mode.c



//#include <string.h>
#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>

#include "types.h"
//#include "syncObj.h"
#include "ini_control.h"
#include "display.h"
#include "comm_mode.h"
#include "sprd_mode.h"
#include "bluetooth.h"
#include "setup_mode.h"
#include "USBRS.h"

#include "powerControl.h"



struct tagCOMMModeControl COMMModeControl;


#ifndef GPS_BT_FREE	


const struct tagMode modes_COMMMode=
{
	COMMMode_NameOnUpdate,//"COMM\0""ÑÂßÇÜ",	//mode name
	RGB(0,63,30),	//mode color
	COMMMode_LeftOnUpdate,//"back\0""íàçàä",	//left
	COMMMode_RightOnUpdate,//"comm\0""îáìåí",	//right
	COMMMode_UpOnUpdate,//"on\0""âêë",//up
	COMMMode_DownOnUpdate,//"menu\0""ìåíþ",	//down
	COMMMode_OnActivate,
	COMMMode_OnLeft,
	COMMMode_OnRight,
	COMMMode_OnUp,
	COMMMode_OnDown,
	COMMMode_OnIdle,
	COMMMode_OnShow,
	COMMMode_OnExit,
	COMMMode_OnPowerDown,
	COMMMode_OnWakeUp,
	COMMMode_OnTimer
};








/*
---------
SPRD > modem
PC > SPRD
---------
Manually on/off
ON while active
----------

*/




const struct tagMenu comm_menu1=
{
	//CONNECTION COMMUNICATION LINK BLUETOOTH
	"MENU: LINK\0""MENU: LINK\0""MENU: LINK\0""ÌÅÍÞ: ÑÂßÇÜ",	//menu name
	1,	//number of items
	{/*COMMMode_menu1_change_state,*/COMMMode_menu1_power_control},
	{/*COMMMode_menu1_change_state_onUpdate,*/COMMMode_menu1_power_control_onUpdate}
};





const char* COMMMode_NameOnUpdate(void)//"COMM\0""ÑÂßÇÜ",	//mode name
{
	if(bluetoothControl.bBluetooth_ON)
		return "LINK ON\0""LINK ON\0""LINK ON\0""Ñâÿçü âêë";
	else
		return "LINK OFF\0""LINK OFF\0""LINK OFF\0""Ñâÿçü âûê";
}
const char* COMMMode_LeftOnUpdate(void)//"back\0""íàçàä",	//left
{
	return "back\0""back\0""back\0""íàçàä";
}
const char* COMMMode_RightOnUpdate(void)//"comm\0""îáìåí",	//right
{
	return "setup\0""setup\0""setup\0""îïöèè";
}
const char* COMMMode_UpOnUpdate(void)//"on\0""âêë",//up
{
	if(bluetoothControl.bBluetooth_ON)
		return "off\0""off\0""off\0""âûêë";
	else
		return "on\0""on\0""on\0""âêë";
}
const char* COMMMode_DownOnUpdate(void)//"menu\0""ìåíþ",	//down
{
	return "menu\0""menu\0""menu\0""ìåíþ";
}






void COMMMode_Init(void)
{
	COMMModeControl.dwReceived = 0;
	COMMModeControl.dwTransmitted = 0;
}

BOOL COMMMode_OnActivate(void)
{
	Modes_createTimer(1000);
	Modes_updateMode();
	return 1;
}

BOOL COMMMode_OnTimer(void)
{
	Modes_OnShow();
	return 1;
}


BOOL COMMMode_OnLeft(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return 1;
}
BOOL COMMMode_OnRight(void)
{
	//must be here with  Modes_setActiveMode(&modes_SETUPMode);
	//reset expert mode
//	SETUPModeControl.bExpertMode = FALSE;
	Modes_setActiveMode(&modes_SETUPMode);
	return 1;
}
BOOL COMMMode_OnUp(void)
{
	if(bluetoothControl.bBluetooth_ON)
		Bluetooth_turnOFF();
	else
	{
		Bluetooth_turnON();
	}
	Modes_updateMode();
	return 1;
}
BOOL COMMMode_OnDown(void)
{
	Modes_activateMenu(&comm_menu1);
	return 1;
}
BOOL COMMMode_OnIdle(void)
{
	return 1;
}
BOOL COMMMode_OnShow(void)
{
	COMMMode_showModeScreen();
	return 1;
}
BOOL COMMMode_OnExit(void)
{
	return 1;
}
BOOL COMMMode_OnWakeUp(void)
{
	return 1;
}
BOOL COMMMode_OnPowerDown(void)
{
	return 0;	//disable enter power down
}


void COMMMode_showModeHeaders(void)
{
	Display_setTextColor(GREEN);	//set text color
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(10);
	Display_outputTextByLang("Received:\0""Bekommen:\0""Received:\0""Ïðèíÿòî:");
	Display_drawHLine(0,Y_SCREEN_MAX-216,X_SCREEN_MAX,RGB(0,32,0));

	Display_setTextColor(GREEN);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-214,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_clearTextWin(250);
	Display_outputTextByLang("Transmitted:\0""Absendung:\0""Transmitted:\0""Ïåðåäàíî:");
	Display_drawHLine(0,Y_SCREEN_MAX-164,X_SCREEN_MAX,RGB(0,32,0));
}

void COMMMode_showModeScreen(void)
{
	char buf[100];
	
	COMMMode_showModeHeaders();
	COMMMode_showReceived();
	COMMMode_showTransmitted();
	
	//show module name
	
/*	if(USBRSControl.bBridgeMode)
	{
		Display_outputText("\r");
		Display_outputText("\r");
		Display_setTextColor(YELLOW);	//set text color
		Display_setTextWrap(1);
		Display_setTextSteps(1,1);
		Display_setTextDoubleHeight(0);
		Display_setTextJustify(NONE);
		Display_setCurrentFont(fnt8x16);
		Display_outputText("> ");
		for(int i=0;i<10;i++)
		{
			sprintf(buf, "%0x ", (UINT)bluetoothControl.uart.trmBuff[i]);
			Display_outputText((char*)buf);
		}		
		Display_outputText("\r< ");
		for(int i=0;i<10;i++)
		{
			sprintf(buf, "%0x ", (UINT)bluetoothControl.uart.rcvBuff_safe[i]);
			Display_outputText((char*)buf);
		}
	}else*/
	{
		Display_setTextColor(YELLOW);	//set text color
		Display_setTextWrap(0);
		Display_setTextSteps(1,1);
		Display_setTextDoubleHeight(0);
		Display_setTextJustify(LEFT);
		Display_setCurrentFont(fnt8x16);
		Display_outputText("\r");
		Display_outputText("\r");
		Display_outputTextByLang("Module name:\r\0""Module name:\r\0""Module name:\r\0""Èìÿ ìîäóëÿ:\r");
		Display_outputText(bluetoothControl.moduleName);
	}
}


//set active mode depends on comm state
void COMMMode_setActiveMode(void)
{
	Modes_setActiveMode(&modes_COMMMode);
}



//ret menu item text, if item disabled then ret NULL
const char* COMMMode_menu1_change_state_onUpdate(void)
{
	switch(bluetoothControl.comm_state)
	{
	case enu_comm_state_modem:
		return "MODE: modem\0""MODE: modem\0""MODE: modem\0""ÐÅÆÈÌ: ìîäåì";
	case enu_comm_state_pc:
		return "MODE: PC\0""MODE: PC\0""MODE: PC\0""ÐÅÆÈÌ: ÏÊ";
	case enu_comm_state_du:
		return "MODE: DU\0""MODE: DU\0""MODE: DU\0""ÐÅÆÈÌ: ÁÄ";
	default:
		return "error\0""error\0""error\0""error";
	}
}

//change state of comm
BOOL COMMMode_menu1_change_state(void)
{
	if(bluetoothControl.comm_state==enu_comm_state_modem)
		bluetoothControl.comm_state=enu_comm_state_pc;
	else if(bluetoothControl.comm_state==enu_comm_state_pc)
		bluetoothControl.comm_state=enu_comm_state_du;
	else 
		bluetoothControl.comm_state=enu_comm_state_modem;

	ini_write_system_ini_int("bluetoothControl", "comm_state", bluetoothControl.comm_state);

	return 0;	//just redraw menu
}


//change state of comm power
BOOL COMMMode_menu1_power_control(void)
{
	if(bluetoothControl.comm_power_state==enu_comm_power_state_manually)
		bluetoothControl.comm_power_state=enu_comm_power_state_on;
	else
		bluetoothControl.comm_power_state=enu_comm_power_state_manually;
	ini_write_system_ini_int("bluetoothControl", "comm_power_state", bluetoothControl.comm_power_state);
	return 0;	//just redraw menu
}




//ret menu item text, if item disabled then ret NULL
const char* COMMMode_menu1_power_control_onUpdate(void)
{
	if(bluetoothControl.comm_power_state==enu_comm_power_state_on)
		return "ON: active\0""ON: active\0""ON: active\0""ÂÊË: àêòèâíî";
	else
		return "ON: manually\0""ON: manually\0""ON: manually\0""ÂÊË: âðó÷íóþ";
}




void COMMMode_showReceived(void)
{
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-249,X_SCREEN_SIZE,32);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt32x32);
	
	char buf[20];
	if(COMMModeControl.dwReceived<1024)
		sprintf(buf,"%u",(UINT)COMMModeControl.dwReceived);
	else
		sprintf(buf,"%u",(UINT)COMMModeControl.dwReceived/1024);
	
	Display_clearTextWin(250);
	Display_outputText(buf);
	Display_setTextXY(190,16);	//set start coords in window
	Display_setCurrentFont(fnt16x16);
	if(COMMModeControl.dwReceived<1024)
		Display_outputText(" B");
	else
		Display_outputText(" kB");
}

void COMMMode_showTransmitted(void)
{
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-197,X_SCREEN_SIZE,32);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt32x32);
	
	char buf[20];
	if(COMMModeControl.dwTransmitted<1024)
		sprintf(buf,"%u",(UINT)COMMModeControl.dwTransmitted);
	else
		sprintf(buf,"%u",(UINT)COMMModeControl.dwTransmitted/1024);
	Display_clearTextWin(200);
	Display_outputText(buf);
	Display_setTextXY(190,16);	//set start coords in window
	Display_setCurrentFont(fnt16x16);
	if(COMMModeControl.dwTransmitted<1024)
		Display_outputText(" B");
	else
		Display_outputText(" kB");
}

#endif	//#ifndef GPS_BT_FREE	
