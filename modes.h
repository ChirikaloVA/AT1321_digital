//modes.h
#ifndef _MODES_H
#define _MODES_H

#include "types.h"

//minimum timer interval of this mode
#define MODES_TIMER_VAL 50

//coords of standart mode
//user area top, bottom and height
#define MODE_USER_TOP (Y_SCREEN_MAX-266)
#define MODE_USER_BOTTOM (Y_SCREEN_MAX-33)
#define MODE_USER_HEIGHT (MODE_USER_BOTTOM-MODE_USER_TOP+1)
//button area top and height
#define MODE_BUTTONS_TOP (MODE_USER_BOTTOM+1)
#define MODE_BUTTONS_HEIGHT 33


//coords of standart menu
#define MENU_TOP (Y_SCREEN_MAX-266)
#define MENU_HEIGHT 234
#define MENU_WIDTH X_SCREEN_SIZE
#define MENU_LEFT 0
//смещение маркера от края меню
#define MENU_MARKER_OFFSET 2
//высота маркера
#define MENU_MARKER_HEIGHT 20


//////////////////////////////////
//режимы

//description of mode
struct tagMode
{
	const char*(*pNameOnUpdate)(void);
	COLORREF modeNameClr;	//mode name color
	//след обр исп для получения наименования кнопки
	//функц может возвр NULL , но поле не должно быть NULL
	const char*(*pLeftOnUpdate)(void);
	const char*(*pRightOnUpdate)(void);
	const char*(*pUpOnUpdate)(void);
	const char*(*pDownOnUpdate)(void);
	BOOL(*pOnActivate)();	//first activate mode, show system and user mode screen
	BOOL(*pOnLeft)();
	BOOL(*pOnRight)();
	BOOL(*pOnUp)();
	BOOL(*pOnDown)();
	BOOL(*pOnIdle)();	//периодическая функция
	BOOL(*pOnShow)();	//show mode screen, can be called by user. system call it intime of onactivate and onwakeup
	BOOL(*pOnExit)();	//called if exit mode
	BOOL(*pOnPowerDown)();	//called if need to power down, ret 0 mean disable power down
	BOOL(*pOnWakeUp)();	//called when waked up from power down
	BOOL(*pOnTimer)();	//timer must be adjusted before in onActivate, work only for active mode
};

//menu item struct

#define MAX_MENU_ITEMS 10


/*
принцип построения меню
- задаем имя меню
- задаем кол пунктов меню
- прописываем обработчики события выбора, если он возвр 0 то меню перерисовывается (для пунктов параметров), если 1 то закрывается
- прописываем обработчики события получения текста пункта, если NULL то пункт не виден и не исп.
*/


//menu struct
struct tagMenu
{
	const char name[80];
	int iItemsNum;	//items number of this menu

	//array of procedures for items in menu
	//ret 1 if exit menu
	//ret 0 if redraw menu item
	//ret -1 if do nothing
	BOOL(*pArOnClick[MAX_MENU_ITEMS])(void);
	//array of procedures of item text update in menu if NULL then item is disabled
	const char*(*pArOnUpdate[MAX_MENU_ITEMS])(void);
};


//number of lang
#define MAX_ENU_LANG 4
enum ENU_LANG
{
	enu_lang_english,
	enu_lang_german,
	enu_lang_french,
	enu_lang_russian,
};


//current mode data
struct tagModeControl
{
	const struct tagMode * pMode;	//current mode
	const struct tagMenu * pMenu;	//current menu
	//
	BOOL bSysDefault;	//true is system.ini is deafult
	BOOL bNoSystemBak;	//true if no file system.bak
	BOOL bNoEnergyCal;	//true if no file energy.cal
	BOOL bNoSigmaCal;	//true if no file sigma.cal
	BOOL bNoLibrary;	//true if no file selected .lib file
	BOOL bIdentDefault;	//true if file identify.ini became default
	//
	int bLang;	//enum ENU_LANG
	BOOL bTimerON;	//=1 then timer if ON
	volatile DWORD dwTimerPeriod;
	volatile DWORD dwTimerReach;	//=1 then main cycle can call timer proc
	BOOL bMenuON;	//=1 then menu mode is activated
	DWORD dwCurMenuItem;
	DWORD dwCurMenuItemNumber;	//amount of items calced by pArOnUpdate!=NULL
};


extern struct tagModeControl modeControl;


void Modes_Init(void);

BOOL Modes_OnActivate(void);
BOOL Modes_OnShow(void);
BOOL Modes_OnExit(void);
BOOL Modes_OnPowerDown(void);
BOOL Modes_OnLeft(void);
BOOL Modes_OnRight(void);
BOOL Modes_OnUp(void);
BOOL Modes_OnDown(void);
BOOL Modes_OnIdle(void);
BOOL Modes_OnWakeUp(void);
BOOL Modes_OnTimer(void);
void Modes_showModeName(void);
void Modes_setActiveMode(const struct tagMode* pMode);

void Modes_Timer_turnON(DWORD ms);
void Modes_Timer_turnOFF(void);
void Modes_createTimer(DWORD ms);
void Modes_killTimer(void);

void Modes_menuDown(void);
void Modes_menuUp(void);
void Modes_activateMenu(const struct tagMenu * pMenu);
void Modes_showMenu();
void Modes_showMarker(void);
void Modes_showButtons(void);
void Modes_clearModeArea(void);
void Modes_updateMode(void);

const char* Modes_menu_DownOnUpdate(void);
const char* Modes_menu_UpOnUpdate(void);
const char* Modes_menu_LeftOnUpdate(void);
const char* Modes_menu_RightOnUpdate(void);

#endif	//_MODES_H
