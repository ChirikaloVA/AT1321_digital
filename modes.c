//modes.c

//init all modes by default


#include "types.h"
#include "syncObj.h"
#include "spectrum.h"
#include "display.h"
#include "interrupts.h"
#include "sprd_mode.h"
#include "tc_mode.h"
#include "mcs_mode.h"
#include "setup_mode.h"
#include "gps_mode.h"
#include "comm_mode.h"
#include "interProc.h"
#include "powerControl.h"
#include "eeprom.h"
#include "YESNO_Mode.h"
#include "edit.h"
#include "USB_mode.h"
#include "LOG_mode.h"
#include "stab_mode.h"
#include "file_list.h"
#include "info_mode.h"
#include "message_mode.h"
#include "rid.h"
#include "keyboard.h"


struct tagModeControl modeControl;

void Modes_Init(void)
{
	modeControl.bLang = enu_lang_english;	//english by default
	modeControl.bMenuON = 0;
	modeControl.pMode = &modes_SPRDMode;
	modeControl.pMenu = NULL;	//current menu
	modeControl.bTimerON = 0;	//=1 then timer if ON
	modeControl.dwTimerPeriod = 0;
	modeControl.dwTimerReach = 0;	//=1 then main cycle can call timer proc
	modeControl.dwCurMenuItem = 0;
	modeControl.dwCurMenuItemNumber = 0;	//amount of items calced by pArOnUpdate!=NULL
	/////////////////////
	//make first init all the modes
	Spectrum_Init();
	SPRDMode_Init();
	MCSMode_Init();
	SPRDMode_Init();
	TCMode_Init();
	SETUPMode_Init();
	GPSMode_Init();
	COMMMode_Init();
	YESNOMode_Init();
	EditMode_Init();
	USBMode_Init();
	LOGMode_Init();
	STABMode_Init();
	FileListMode_Init();
	INFOMode_Init();
	MessageMode_Init();
	RID_Init();
	/////////////////////
}

//set active mode
void Modes_setActiveMode(const struct tagMode* pMode)
{
	//allow NULL mode (power down)
/*	if(!pMode)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"pMode must not be NULL");
	}*/
	
	if(modeControl.pMode != pMode)
		Modes_OnExit();
	
	modeControl.pMode = pMode;
	modeControl.bMenuON = 0;	//turn off menu if it was
	Modes_killTimer();	//kill timer if it was
	//activate mode by default

	Modes_OnActivate();	//call onactivate for the new mode
	
	//здесь надо очистить буфер клавы чтобы случ нажатия не обработать
	KeyboardControl_restoreKeyboard();
}

//clear user area of mode
void Modes_clearModeArea(void)
{
	RECT rect = {0,MODE_USER_TOP,X_SCREEN_MAX,MODE_USER_BOTTOM};
	Display_clearRect(rect,200);
}

//update view of current mode
void Modes_updateMode(void)
{
	if(!modeControl.bMenuON)
		Modes_clearModeArea();	//clear screen area for mode
	Modes_showModeName();
	Modes_showButtons();
	Modes_OnShow();
}

//here whole mode screen update too
BOOL Modes_OnActivate(void)
{
	if(!display.bLCDON)
	{
		Display_turnON();
	}
	
	if(!modeControl.pMode)return 1;
	if(!modeControl.pMode->pOnActivate)return 1;
	return modeControl.pMode->pOnActivate();
}

//on show of the mode, but if we are in menu so no onshow will be called
BOOL Modes_OnShow(void)
{
	if(!modeControl.pMode)return 1;
	if(!modeControl.bMenuON)
	{
		//if not menu mode then show mode show proc
		if(!modeControl.pMode->pOnShow)return 1;
		return modeControl.pMode->pOnShow();
	}
	return 1;
}

BOOL Modes_OnExit(void)
{
	if(!modeControl.pMode)return 1;
	if(!modeControl.pMode->pOnExit)return 1;
	return modeControl.pMode->pOnExit();
}

//call on power down of the mode, if it ret 0 then it will block enter power down
BOOL Modes_OnPowerDown(void)
{
	if(!modeControl.pMode)return 1;
	if(powerControl.bBatteryAlarm)return 0;	//no power down if bat alarm
	if(!modeControl.pMode->pOnPowerDown)return 1;
	BOOL bpd = modeControl.pMode->pOnPowerDown();
	if(bpd)
		if(EEPROM_Busy())return 0;	//can not enter power down if eeprom busy
	return bpd;
}

BOOL Modes_OnLeft(void)
{
	if(!modeControl.pMode)return 1;
	if(modeControl.bMenuON)
	{
		Modes_menuUp();
		return 1;
	}else
	{
		if(!modeControl.pMode->pLeftOnUpdate())return 1;
		if(!modeControl.pMode->pOnLeft)return 1;
		return modeControl.pMode->pOnLeft();
	}
}

BOOL Modes_OnRight(void)
{
	if(!modeControl.pMode)return 1;
	if(modeControl.bMenuON)
	{
		Modes_menuDown();
		return 1;
	}else
	{
		if(!modeControl.pMode->pRightOnUpdate())return 1;
		if(!modeControl.pMode->pOnRight)return 1;
		return modeControl.pMode->pOnRight();
	}
}

BOOL Modes_OnUp(void)
{
	if(!modeControl.pMode)return 1;
	if(modeControl.bMenuON)
	{//click on menu item
		DWORD it = modeControl.dwCurMenuItem;
		//find real item index
		int idx = 0;
		for(;idx<modeControl.pMenu->iItemsNum;idx++)
		{
			if(modeControl.pMenu->pArOnUpdate[idx]()!=NULL)
			{
				if(it==0)break;
				--it;
			}
		}
		//check item
		if(!modeControl.pMenu->pArOnClick[idx])
		{
			exception(__FILE__,__FUNCTION__,__LINE__,"void menu item handler");
		}
		BOOL ret = modeControl.pMenu->pArOnClick[idx]();
		if(ret==TRUE)
		{//close menu
			if(modeControl.bMenuON)	//check if menu was opened
				Modes_OnDown();	//close menu
		}else if(ret==FALSE)
		{//just redraw menu
			Modes_showMenu();
		}
		return ret;
	}else
	{
		if(!modeControl.pMode->pUpOnUpdate())return 1;
		if(!modeControl.pMode->pOnUp)return 1;
		return modeControl.pMode->pOnUp();
	}
}

BOOL Modes_OnDown(void)
{
	if(!modeControl.pMode)return 1;
	if(modeControl.bMenuON)
	{//exit menu
		modeControl.bMenuON = 0;
		Modes_updateMode();
		return 1;
	}else
	{
		if(!modeControl.pMode->pDownOnUpdate())return 1;
		if(!modeControl.pMode->pOnDown)return 1;
		return modeControl.pMode->pOnDown();
	}
}

BOOL Modes_OnIdle(void)
{
	if(!modeControl.pMode)return 1;
	if(!modeControl.pMode->pOnIdle)return 1;
	return modeControl.pMode->pOnIdle();
}

BOOL Modes_OnTimer(void)
{
	if(!modeControl.pMode)return 1;
	if(!modeControl.pMode->pOnTimer)return 1;
	return modeControl.pMode->pOnTimer();
}


//onwakeup is used after wakeup from power down
// it needs to show mode name, mode buttons and mode user screen
BOOL Modes_OnWakeUp(void)
{
	if(!modeControl.pMode)return 1;
	modeControl.bMenuON = 0;
	Modes_updateMode();
	if(!modeControl.pMode->pOnWakeUp)return 1;
	return modeControl.pMode->pOnWakeUp();
}


void Modes_showModeName(void)
{
	if(!modeControl.pMode)return;
	//show mode name
	COLORREF clr = modeControl.pMode->modeNameClr;
	Display_setTextColor(clr);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-300,X_SCREEN_SIZE,34);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt32x32);	//set current font
	const char* pName = (char*)modeControl.pMode->pNameOnUpdate();
	Display_clearTextWin(110);
	Display_outputTextByLang(pName);
	Display_drawHLine(0,MODE_USER_TOP-2,X_SCREEN_MAX,clr);
}

void Modes_showButtons(void)
{
	if(!modeControl.pMode)return;
	//show bottom menu buttons
	COLORREF clr = modeControl.pMode->modeNameClr;
	Display_setTextColor(clr);	//set text color
	Display_setTextSteps(1,-1);
	Display_setTextWrap(0);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);	//set current font
	Display_setTextWin(0,MODE_BUTTONS_TOP,X_SCREEN_SIZE,MODE_BUTTONS_HEIGHT);	//set text window
	Display_clearTextWin(100);
	Display_setTextWin(0,MODE_BUTTONS_TOP,80,15);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_outputTextByLang((char*)(modeControl.bMenuON?Modes_menu_LeftOnUpdate():(modeControl.pMode->pLeftOnUpdate()?modeControl.pMode->pLeftOnUpdate():"\0""")));
	Display_setTextWin(80,MODE_BUTTONS_TOP,80,15);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_outputTextByLang((char*)(modeControl.bMenuON?Modes_menu_UpOnUpdate():(modeControl.pMode->pUpOnUpdate()?modeControl.pMode->pUpOnUpdate():"\0""")));
	Display_setTextWin(80,MODE_BUTTONS_TOP+15,80,15);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_outputTextByLang((char*)(modeControl.bMenuON?Modes_menu_DownOnUpdate():(modeControl.pMode->pDownOnUpdate()?modeControl.pMode->pDownOnUpdate():"\0""")));
	Display_setTextWin(160,MODE_BUTTONS_TOP,80,15);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_outputTextByLang((char*)(modeControl.bMenuON?Modes_menu_RightOnUpdate():(modeControl.pMode->pRightOnUpdate()?modeControl.pMode->pRightOnUpdate():"\0""")));
	clr = RGB(63,63,0);
	Display_drawHLine(0,Y_SCREEN_MAX-0,X_SCREEN_MAX,clr);
	Display_drawHLine(0,Y_SCREEN_MAX-30,X_SCREEN_MAX,clr);
	Display_drawHLine(81,Y_SCREEN_MAX-15,159,clr);
	Display_drawVLine(80,Y_SCREEN_MAX-29,Y_SCREEN_MAX-0,clr);
	Display_drawVLine(160,Y_SCREEN_MAX-29,Y_SCREEN_MAX-0,clr);
}

const char* Modes_menu_DownOnUpdate(void)
{
	return "exit\0""exit\0""exit\0""выход";
}

const char* Modes_menu_UpOnUpdate(void)
{
	return "select\0""select\0""select\0""выбор";
}

const char* Modes_menu_LeftOnUpdate(void)
{
	return "up\0""up\0""up\0""вверх";
}

const char* Modes_menu_RightOnUpdate(void)
{
	return "down\0""down\0""down\0""вниз";
}


void Modes_killTimer(void)
{
	Modes_Timer_turnOFF();
	modeControl.bTimerON = 0;	//turn off timer by default
	SAFE_SET(modeControl.dwTimerReach, 0);
}


void Modes_createTimer(DWORD ms)
{
	if(ms<INTERPROC_TIMER_VAL)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"timer delay is out of range");
	}
	Modes_Timer_turnOFF();
	modeControl.bTimerON = 1;	//turn on timer
	modeControl.dwTimerPeriod = ms;
	SAFE_SET(modeControl.dwTimerReach, 0);
	Modes_Timer_turnON(ms);
}




void Modes_Timer_turnON(DWORD ms)
{
	if(ms<INTERPROC_TIMER_VAL)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"timer delay is out of range");
	}
	//adjust comparators 0
	SAFE_DECLARE;
	DISABLE_VIC;
	T2TCR_bit.CE = 0;
	T2MR1 = T2TC+ms/INTERPROC_TIMER_VAL;
	T2MCR_bit.MR1I = 1; //enable interrupt for interproc
	T2TCR_bit.CE = 1;
	ENABLE_VIC;
}

void Modes_Timer_turnOFF(void)
{
	T2MCR_bit.MR1I = 0; //disable interrupt for modes
}


//активировать режим меню и отобразить его
//it called to activate menu first time
//!!!!! modeControl.dwCurMenuItemAr must be set before
void Modes_activateMenu(const struct tagMenu * pMenu)
{
	if(!pMenu)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"pMenu must not be NULL");
	}
	modeControl.bMenuON = 1;
	modeControl.dwCurMenuItem = 0;
	modeControl.pMenu = pMenu;
	Modes_showMenu();
	Modes_showButtons();
}



void Modes_showMenu()
{
	if(!modeControl.pMode)return;
	
	if(!modeControl.pMenu->pArOnClick)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"pArOnClick must not be NULL");
	}
	


	Display_setTextWin(MENU_LEFT,MENU_TOP,MENU_WIDTH,MENU_HEIGHT);	//set text window
	Display_setTextWrap(0);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);	//set current font
	Display_setTextColor(YELLOW_DARK);	//set text color
	Display_clearTextWin(10);
	

	//calc number of items visible
	modeControl.dwCurMenuItemNumber = 0;
	for(int i=0;i<modeControl.pMenu->iItemsNum;i++)
	{
		if(modeControl.pMenu->pArOnUpdate[i]()!=NULL)
			modeControl.dwCurMenuItemNumber++;
	}

	int y,dy;
	dy = (MAX_MENU_ITEMS-modeControl.dwCurMenuItemNumber)*MENU_MARKER_HEIGHT;
	y = MENU_TOP+dy;
	dy = MENU_HEIGHT-dy;
	Display_setTextWin(MENU_LEFT+4,y,MENU_WIDTH-4,dy);	//set text window
	Display_setTextSteps(1,4);
	Display_setTextXY(3,3);	//set start coords in window
	
	if(modeControl.pMenu->name[0])
	{
	Display_outputTextByLang((char*)modeControl.pMenu->name);
	Display_outputText("\r");
	Display_drawHLine(1,y+MENU_MARKER_HEIGHT,238,YELLOW_DARK);
	}
	Display_setTextJustify(LEFT);
	Display_setTextColor(modeControl.pMode->modeNameClr);	//set text color
	for(int i=0;i<modeControl.pMenu->iItemsNum;i++)
	{
		const char* pTxt = modeControl.pMenu->pArOnUpdate[i]();//Return Item Text
		if(pTxt)
		{
			Display_outputTextByLang(pTxt);	
			Display_outputText("\r");	
		}
	}
//	Display_outputTextByLang((char*)modeControl.pMenu->pMenuItemAr[modeControl.pMenu->pGetMenuItemsAr()]);
		
	Display_drawRect(MENU_LEFT,y,MENU_LEFT+MENU_WIDTH-1,y+dy-1,YELLOW_DARK);
	Modes_showMarker();
}

void Modes_showMarker(void)
{
	if(!modeControl.pMode)return;
	int ym;
	ym = MENU_TOP+(MAX_MENU_ITEMS-modeControl.dwCurMenuItemNumber)*MENU_MARKER_HEIGHT;

	int y = ym+MENU_MARKER_OFFSET+(modeControl.dwCurMenuItem+1)*MENU_MARKER_HEIGHT;

	//invers line
	for(int i=0;i<MENU_MARKER_HEIGHT-1;i++)
		Display_drawHLine_xor(MENU_LEFT+MENU_MARKER_OFFSET,
						 y+i,
						 MENU_LEFT+MENU_WIDTH-1-MENU_MARKER_OFFSET,
						 modeControl.pMode->modeNameClr);
	//just rect
/*	
	Display_drawRect_xor(MENU_LEFT+MENU_MARKER_OFFSET,
						 y,
						 MENU_LEFT+MENU_WIDTH-1-MENU_MARKER_OFFSET,
						 y-MENU_MARKER_HEIGHT,
						 WHITE);
	*/
}

void Modes_menuUp(void)
{
	Modes_showMarker();
	if(modeControl.dwCurMenuItem>0)
	{
		modeControl.dwCurMenuItem--;
	}else
	{
		modeControl.dwCurMenuItem=modeControl.dwCurMenuItemNumber-1;
	}
	Modes_showMarker();
}

void Modes_menuDown(void)
{
	Modes_showMarker();
	if(modeControl.dwCurMenuItem<modeControl.dwCurMenuItemNumber-1)
	{
		modeControl.dwCurMenuItem++;
	}else
	{
		modeControl.dwCurMenuItem=0;
	}
	Modes_showMarker();
}
