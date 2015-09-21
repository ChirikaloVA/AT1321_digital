//spectrum.c


#include <string.h>
#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <math.h>

#include "types.h"
#include "sound.h"
#include "rid.h"
#include "display.h"
#include "keyboard.h"
#include "powerControl.h"
#include "sprd_mode.h"
#include "tc_mode.h"
#include "eeprom.h"
#include "clock.h"
#include "interrupts.h"
#include "edit.h"
#include "spectrum.h"
#include "mcs_mode.h"
#include "setup_mode.h"
#include "message_mode.h"






struct tagRIDControl RIDControl;



const struct tagMenu rid_menu=
{
	"MENU: EXPERT\0""MENU: EXPERT\0""MENU: EXPERT\0""МЕНЮ: ЭКСПЕРТ",	//menu name
	2,	//number of items
	{RID_menu1_TC, RID_menu1_exitExpertMode},
	{RID_menu1_TC_onUpdate, RID_menu1_exitExpertMode_onUpdate}
};




const struct tagMode modes_RID_Mode=
{
	RID_NameOnUpdate,//"RID\0""РИД",
	RGB(63,40,0),
	RID_LeftOnUpdate,//"marker\rleft\0""маркер\rвлево",
	RID_RightOnUpdate,//"marker\rright\0""маркер\rвправо",
	RID_UpOnUpdate,//"acquir\0""набор",
	RID_DownOnUpdate,//"menu\0""меню",
	RID_OnActivate,
	RID_OnLeft,
	RID_OnRight,
	RID_OnUp,
	RID_OnDown,
	RID_OnIdle,
	RID_OnShow,
	RID_OnExit,
	RID_OnPowerDown,
	RID_OnWakeUp,
	RID_OnTimer
};



BOOL RID_OnTimer(void)
{
	return TRUE;
}



const char* RID_NameOnUpdate(void)//"RID adv\0""РИД расш",
{
	return "RID\0""RID\0""RID\0""РИД";
}



//режимы работы со спектром
//отображаемое слово и есть текущий режим
const char* RID_UpOnUpdate(void)
{
	if(RIDControl.bExpertMode )
		return "spectr\0""spectr\0""spectr\0""спектр";
	else
		return "login\0""login\0""login\0""пароль";
}
const char* RID_DownOnUpdate(void)//"menu\0""меню",
{
	return "menu\0""menu\0""menu\0""меню";
}




const char* RID_menu1_TC_onUpdate(void)
{
	return "TC mode\0""TC mode\0""TC mode\0""Режим СЧЕТ";
}

BOOL RID_menu1_TC(void)
{
	Modes_setActiveMode(&modes_TCMode);
	return 1;
}




/*
exit expert mode
*/

BOOL RID_menu1_exitExpertMode(void)
{
	RIDControl.bExpertMode = FALSE;
	Modes_setActiveMode(&modes_RID_Mode);
	return 1;
}


const char* RID_menu1_exitExpertMode_onUpdate(void)
{
	return "Exit Expert Mode\0""Exit Expert Mode\0""Exit Expert Mode\0""Выйти из экспертного режима";
}













void RID_Init(void)
{
	memset(&RIDControl,0,sizeof(RIDControl));
	RIDControl.bExpertMode = FALSE;
}







BOOL RID_OnIdle(void)
{
	return 1;
}
BOOL RID_OnExit(void)
{
	return 1;
}
BOOL RID_OnWakeUp(void)
{
	return 1;
}
BOOL RID_OnPowerDown(void)
{
	return 1;
}





///////////////////////////////////////////////////////////////////////////////
//RID PSW mode

const char* RID_LeftOnUpdate(void)//"marker\rleft\0""маркер\rвлево",
{
	return "back\0""back\0""back\0""назад";
}
const char* RID_RightOnUpdate(void)//"marker\rright\0""маркер\rвправо",
{
#ifdef _IAEA
	return "meas\0""meas\0""meas\0""измер";
#else
	return "mcs\0""mcs\0""mcs\0""мкд";
#endif
}



BOOL RID_OnActivate(void)
{
	Modes_updateMode();
	return 1;
}


BOOL RID_OnLeft(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return 1;
}

BOOL RID_OnRight(void)
{
	Modes_setActiveMode(&modes_MCSMode);
	return 1;
}

BOOL RID_OnUp(void)
{
	if(RIDControl.bExpertMode )
		Modes_setActiveMode(&modes_SpectrumMode);
	else
	{
		//enter expert mode
		EditMode_EditInt("Expert mode\rEnter password\0""Expert mode\rEnter password\0""Expert mode\rEnter password\0""Экспертный режим\rВведите пароль",
						  0,
						  0,
						  9999,
						 "\0""\0""\0""",
						  RID_done);
	}
	return (BOOL)-1;
}

BOOL RID_OnDown(void)
{
	if(!RIDControl.bExpertMode )
		return 1;
	Modes_activateMenu(&rid_menu);
	return 1;
}



BOOL RID_OnShow(void)
{
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,1);
	
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(10);
	if(RIDControl.bExpertMode )
		Display_outputTextByLang("Expert mode. Access granted. Press SPECTR or Menu.\0""Expert mode. Access granted. Press SPECTR or Menu.\0""Expert mode. Access granted. Press SPECTR or Menu.\0""Экспертный режим. Доступ открыт. Нажмите СПЕКТР или меню.");
	else
		Display_outputTextByLang("Expert mode. Password protected. Press LOGIN.\0""Expert mode. Password protected. Press LOGIN.\0""Expert mode. Password protected. Press LOGIN.\0""Экспертный режим. Защищен паролем. Нажмите ПАРОЛЬ.");
	return 1;
}







void RID_done(BOOL bOK)
{
	if(bOK)
	{

//in debug mode dont need to enter psw
#ifdef DEBUG
		SETUPMode_accessGranted();
		RIDControl.bExpertMode = TRUE;
		Modes_setActiveMode(&modes_RID_Mode);
#else
		char pStr[5];
		memset(pStr,0,5);
		sprintf((char*)pStr, "%02u%02u",
				(UINT)clockData.dateTime.hour, (UINT)clockData.dateTime.minute);

		if(!strcmp(pStr,EditModeControl.edit_buf))
		{
			SETUPMode_accessGranted();
			RIDControl.bExpertMode = TRUE;
			Modes_setActiveMode(&modes_RID_Mode);
		}else
		{
			SETUPMode_accessDenied();
			Modes_setActiveMode(&modes_RID_Mode);
		}
#endif		//#ifdef DEBUG
	}else
		Modes_setActiveMode(&modes_RID_Mode);
}
	