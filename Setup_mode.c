//setup_mode.c



#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <iolpc2388.h>
#include <math.h>
#include <stdlib.h>
#include "main.h"

#include "types.h"
#include "syncObj.h"
#include "spectrum.h"
#include "display.h"
#include "powerControl.h"
#include "sprd_mode.h"
#include "tc_mode.h"
#include "setup_mode.h"
#include "gps_mode.h"
#include "clock.h"
#include "filesystem.h"
#include "interrupts.h"
#include "identify.h"
#include "YESNO_mode.h"
#include "LOG_mode.h"
#include "ini_control.h"
#include "geiger.h"
#include "edit.h"
#include "interProc.h"
#include "file_list.h"
#include "puasson.h"
#include "USBRS.h"
#include "sound.h"
#include "bluetooth.h"
#include "gps.h"
#include "keyboard.h"
#include "rid.h"








struct tagSETUPModeControl SETUPModeControl;


const struct tagMode modes_SETUPMode=
{
	SETUPMode_NameOnUpdate,//"SETUP\0""ОПЦИИ",	//mode name
	RGB(10,63,20),	//mode color
	SETUPMode_LeftOnUpdate,//"back\0""назад",	//left
	SETUPMode_RightOnUpdate,//"gps\0""gps",	//right
	SETUPMode_UpOnUpdate,//"acquir\0""набор",//up
	SETUPMode_DownOnUpdate,//"menu\0""меню",	//down
	SETUPMode_OnActivate,
	SETUPMode_OnLeft,
	SETUPMode_OnRight,
	SETUPMode_OnUp,
	SETUPMode_OnDown,
	SETUPMode_OnIdle,
	SETUPMode_OnShow,
	SETUPMode_OnExit,
	SETUPMode_OnPowerDown,
	SETUPMode_OnWakeUp,
	SETUPMode_OnTimer
};



const char* SETUPMode_NameOnUpdate(void)//"SETUP\0""ОПЦИИ",	//mode name
{
	return "SETUP\0""SETUP\0""SETUP\0""ОПЦИИ";
}
const char* SETUPMode_LeftOnUpdate(void)//"back\0""назад",	//left
{
	return "back\0""back\0""back\0""назад";
}
const char* SETUPMode_RightOnUpdate(void)//"gps\0""gps",	//right
{
	return "log\0""log\0""log\0""журнал";
}
const char* SETUPMode_UpOnUpdate(void)//"acquir\0""набор",//up
{
	switch(SETUPModeControl.iModeScreen)
	{
	case ENU_SETUP_MODE_SCR_VER:
		return "memory\0""memory\0""memory\0""память";
	case ENU_SETUP_MODE_SCR_MEM:
		return "specif\0""specif\0""specif\0""специф";
	case ENU_SETUP_MODE_SCR_SPEC:
		return "librar\0""librar\0""librar\0""библ";
	case ENU_SETUP_MODE_SCR_LIB:
		return "geiger\0""geiger\0""geiger\0""гейгер";
	case ENU_SETUP_MODE_SCR_GEIGER:
		return "power\0""power\0""power\0""заряд";
	case ENU_SETUP_MODE_SCR_BAT:
		return "versio\0""versio\0""versio\0""версия";
	default:
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error (iModeScreen)");
	}
	return NULL;
}
const char* SETUPMode_DownOnUpdate(void)//"menu\0""меню",	//down
{
	//!!!!!!!!!!!!!!1 to get splash screens
/*	if(SETUPModeControl.iJustLeaveScreen)
	{
		--SETUPModeControl.iJustLeaveScreen;
		return NULL;
	}*/
	//!!!!!!!!!!!!!!!!
	if(RIDControl.bExpertMode)
		return "menu\0""menu\0""menu\0""меню";
	else
		return "login\0""login\0""login\0""пароль";
}





/////////////////////////////////////////////////////////////////////////////////////////
/*
//боевое
const struct tagMenu setup_menu_version=
{
	"MENU: COMMON\0""MENU: COMMON\0""MENU: COMMON\0""МЕНЮ: ОБЩЕЕ",	//menu name
	4,	//number of items
	{SETUPMode_menu1_datetime,SETUPMode_menu1_lang,SETUPMode_menu1_testscreen,SETUPMode_menu1_idleTime},
	{SETUPMode_menu1_datetime_onUpdate,SETUPMode_menu1_lang_onUpdate,SETUPMode_menu1_testscreen_onUpdate,SETUPMode_menu1_idleTime_onUpdate}
};*/
//для тестов со звуком
const struct tagMenu setup_menu_version=
{
	"MENU: VERSION\0""MENU: VERSION\0""MENU: VERSION\0""МЕНЮ: ВЕРСИЯ",	//menu name
	2,	//number of items
	{SETUPMode_menu1_idleTime, SETUPMode_menu1_testscreen /*, SETUPMode_menu1_showSplash, SETUPMode_menu1_playSound, SETUPMode_menu1_stopRecord, SETUPMode_menu1_placeMarkers*/},
	{SETUPMode_menu1_idleTime_onUpdate, SETUPMode_menu1_testscreen_onUpdate /*, SETUPMode_menu1_showSplash_onUpdate, SETUPMode_menu1_playSound_onUpdate, SETUPMode_menu1_stopRecord_onUpdate, SETUPMode_menu1_placeMarkers_onUpdate*/}
};
const struct tagMenu setup_menu_library=
{
	"MENU: LIBRARY\0""MENU: LIBRARY\0""MENU: LIBRARY\0""МЕНЮ: БИБЛ.",	//menu name
	4,	//number of items
	{SETUPMode_menu1_select_lib,SETUPMode_menu1_IDthreshold,SETUPMode_menu1_instability,SETUPMode_menu1_identtime},
	{SETUPMode_menu1_select_lib_onUpdate,SETUPMode_menu1_IDthreshold_onUpdate,SETUPMode_menu1_instability_onUpdate,SETUPMode_menu1_identtime_onUpdate}
};
const struct tagMenu setup_menu_memory=
{
	"MENU: MEMORY\0""MENU: MEMORY\0""MENU: MEMORY\0""МЕНЮ: ПАМЯТЬ",	//menu name
	6,	//number of items
	{SETUPMode_menu1_del_spz, SETUPMode_menu1_del_autospz, SETUPMode_menu1_del_log, SETUPMode_menu1_del_mcs, SETUPMode_menu1_del_lib, SETUPMode_menu1_defaults},
	{SETUPMode_menu1_del_spz_onUpdate, SETUPMode_menu1_del_autospz_onUpdate, SETUPMode_menu1_del_log_onUpdate, SETUPMode_menu1_del_mcs_onUpdate, SETUPMode_menu1_del_lib_onUpdate, SETUPMode_menu1_defaults_onUpdate}
};
const struct tagMenu setup_menu_geiger=
{
	"MENU: GM\0""MENU: GM\0""MENU: GM\0""МЕНЮ: ГМ",	//menu name
	5,	//number of items
	{SETUPMode_menu1_prirabotka, SETUPMode_menu1_setCoef, SETUPMode_menu1_setDT,
	SETUPMode_menu1_setSF,SETUPMode_menu1_SafetyThreshold},
	{SETUPMode_menu1_prirabotka_onUpdate, SETUPMode_menu1_setCoef_onUpdate, SETUPMode_menu1_setDT_onUpdate,
	SETUPMode_menu1_setSF_onUpdate, SETUPMode_menu1_SafetyThreshold_onUpdate}
};
const struct tagMenu setup_menu_spec=
{
	"MENU: SPECIFIC\0""MENU: SPECIFIC\0""MENU: SPECIFIC\0""МЕНЮ: СПЕЦИФИК",	//menu name
	2,	//number of items
	{SETUPMode_menu1_calcOperSigma, SETUPMode_menu1_editDTCOEF},
	{SETUPMode_menu1_calcOperSigma_onUpdate, SETUPMode_menu1_editDTCOEF_onUpdate}
};


const struct tagMenu setup_menu_bat=
{
	"MENU: BATTERY\0""MENU: BATTERY\0""MENU: BATTERY\0""МЕНЮ: БАТАРЕЯ",	//menu name
	1,	//number of items
	{SETUPMode_menu1_batcoef},
	{SETUPMode_menu1_batcoef_onUpdate}
};







const char* SETUPMode_menu1_batcoef_onUpdate(void)
{
	return "Calibration\0""Calibration\0""Calibration\0""Калибровка";
}










BOOL SETUPMode_menu1_SafetyThreshold(void)
{
	EditMode_EditFloat("Radiation safety alarm threshold\0""Radiation safety alarm threshold\0""Radiation safety alarm threshold\0""Порог радиационной безопасности",
					 SPRDModeControl.fDRThreshold,
#ifdef BNC					 
					 0.1,
					 100.0,
					 SPRDMode_getDimension(),
					 "%.2f",
#else
					 1.0,
					 1000.0,
					 SPRDMode_getDimension(),
					 "%.1f",
#endif					 
					 SETUPMode_menu1_SafetyThreshold_done);
	return (BOOL)-1;
}

void SETUPMode_menu1_SafetyThreshold_done(BOOL bOK)
{
	if(bOK)
	{
		SPRDModeControl.fDRThreshold  = atof(EditModeControl.edit_buf);
		if(!ini_write_system_ini_float("SPRDModeControl","fDRThreshold",SPRDModeControl.fDRThreshold))
		{
			;//!!!!!!!!!error
		}
	}
	SETUPMode_setModeOnSelf();
}


const char* SETUPMode_menu1_SafetyThreshold_onUpdate(void)
{
	return "Safety threshold\0""Safety Threshold\0""Safety threshold\0""Порог безопасности";
}




BOOL SETUPMode_menu1_identtime(void)
{
	EditMode_EditInt("Time to identify radionuclides\0""Time to identify radionuclides\0""Time to identify radionuclides\0""Время идентификации радионуклидов",
					 identifyControl.identifyStartDeadTime,
					 30.0,
					 300.0,
					 "second\0""second\0""second\0""секунд",
					 SETUPMode_menu1_identtime_done);
	return (BOOL)-1;
}

void SETUPMode_menu1_identtime_done(BOOL bOK)
{
	if(bOK)
	{
		identifyControl.identifyStartDeadTime  = atoi(EditModeControl.edit_buf);
		if(!identify_write_identify_ini_int("identifyControl","identifyStartDeadTime",identifyControl.identifyStartDeadTime))
		{
			;//!!!!!!!!error
		}
	}
	SETUPMode_setModeOnSelf();
}


const char* SETUPMode_menu1_identtime_onUpdate(void)
{
	return "Identification time\0""Identification time\0""Identification time\0""Время идентификации";
}









const char* SETUPMode_menu1_idleTime_onUpdate(void)
{
	return "Sleep mode timeout\0""Sleep mode timeout\0""Sleep mode timeout\0""Таймаут дежурный";
}


BOOL SETUPMode_menu1_idleTime(void)
{
	EditMode_EditInt("Time to enter sleep mode\0""Time to enter sleep mode\0""Time to enter sleep mode\0""Время до перехода в дежурный режим",
					 (int)powerControl.dwPowerDownDeadTime,
					 30,
					 600,
					 "\0""\0""\0""",
					 SETUPMode_menu1_idleTime_edit_done);
	return (BOOL)-1;
}

void SETUPMode_menu1_idleTime_edit_done(BOOL bOK)
{
	if(bOK)
	{
		powerControl.dwPowerDownDeadTime = (int)atoi(EditModeControl.edit_buf);
		if(!ini_write_system_ini_int("powerControl", "dwPowerDownDeadTime", powerControl.dwPowerDownDeadTime))
		{
			;//!!!!!!!!!error
		}
	}
	SETUPMode_setModeOnSelf();
}

void SETUPMode_setModeOnSelf(void)
{
	int imode = SETUPModeControl.iModeScreen;
	Modes_setActiveMode(&modes_SETUPMode);
	SETUPModeControl.iModeScreen = imode;
	Modes_updateMode();
}


#define FRAME_SCR_PAUS 10
#define INTER_SCR_PAUS 500
#define TONE_PAUS 100

BOOL SETUPMode_menu1_testscreen(void)
{
	int i;
	RECT rect = {0,0,X_SCREEN_SIZE-1, Y_SCREEN_SIZE-1};
	Display_clearScreen();
	
	for(i=0;i<0x40;i++)
	{
		Display_fillRect(rect, RGB(0,0,i));
		PowerControl_sleep(FRAME_SCR_PAUS);
	}
	PowerControl_sleep(INTER_SCR_PAUS);
	

	for(i=0;i<0x40;i++)
	{
		Display_fillRect(rect, RGB(0,i,0));
		PowerControl_sleep(FRAME_SCR_PAUS);
	}
	PowerControl_sleep(INTER_SCR_PAUS);


	for(i=0;i<0x40;i++)
	{
		Display_fillRect(rect, RGB(i,0,0));
		PowerControl_sleep(FRAME_SCR_PAUS);
	}
	PowerControl_sleep(INTER_SCR_PAUS);


	for(i=0;i<0x40;i++)
	{
		Display_fillRect(rect, RGB(i,i,0));
		PowerControl_sleep(FRAME_SCR_PAUS);
	}
	PowerControl_sleep(INTER_SCR_PAUS);


	for(i=0;i<0x40;i++)
	{
		Display_fillRect(rect, RGB(0,i,i));
		PowerControl_sleep(FRAME_SCR_PAUS);
	}
	PowerControl_sleep(INTER_SCR_PAUS);


	for(i=0;i<0x40;i++)
	{
		Display_fillRect(rect, RGB(i,0,i));
		PowerControl_sleep(FRAME_SCR_PAUS);
	}
	PowerControl_sleep(INTER_SCR_PAUS);


	for(i=0;i<0x40;i++)
	{
		Display_fillRect(rect, RGB(i,i,i));
		PowerControl_sleep(FRAME_SCR_PAUS);
	}
	PowerControl_sleep(INTER_SCR_PAUS);


	Display_clearScreen();
	PowerControl_sleep(INTER_SCR_PAUS);

	Modes_updateMode();
	return 1;
}

const char* SETUPMode_menu1_testscreen_onUpdate(void)
{
	return "Screen test\0""Screen test\0""Screen test\0""Тест экрана";
}


/*
//to get splash screens by USB
const char* SETUPMode_menu1_showSplash_onUpdate(void)
{
	return "Splash screens\0""Splash screens\0""Splash screens\0""Заставки";
}

BOOL SETUPMode_menu1_showSplash(void)
{
	switch(SETUPModeControl.isplash)
	{
	case 0:
		Display_startup_display_start();
		break;
	case 1:
		PowerControl_turnOFF_device_show_screen("RELEASE THE KEY\0""RELEASE THE KEY\0""RELEASE THE KEY\0""ОТПУСТИТЕ КНОПКУ");
		break;
	case 2:
		PowerControl_turnOFF_device_show_screen("Battery low!\0""Battery low!\0""Battery low!\0""Батареи разряжены!");
		break;
	case 3:
		PowerControl_powerDown_show_screen();
		break;
	}
	if(++SETUPModeControl.isplash>3)SETUPModeControl.isplash=0;
	modeControl.bMenuON = FALSE;	//to exit menu properly without redraw
	SETUPModeControl.iJustLeaveScreen = 5;
	return 1;
}
*/








BOOL SETUPMode_menu1_IDthreshold(void)
{
	EditMode_EditInt("Peak search threshold\0""Peak search threshold\0""Peak search threshold\0""Порог поиска пиков",
					 identifyControl.threshold,
					 2,
					 6,
					 "\0""\0""\0""",
					 SETUPMode_menu1_IDthreshold_edit_done);
	return (BOOL)-1;
}

void SETUPMode_menu1_IDthreshold_edit_done(BOOL bOK)
{
	if(bOK)
	{
		identifyControl.threshold = atoi(EditModeControl.edit_buf);		
		if(!identify_write_identify_ini_int("identifyControl","threshold",identifyControl.threshold))
		{
			;//!!!!!!!error
		}
	}
	SETUPMode_setModeOnSelf();
}





BOOL SETUPMode_menu1_instability(void)
{
	identifyControl.fSCALEINSTABILITY = (float)identifyControl.SCALEINSTABILITY/10.0;
	EditMode_EditFloat("Scale instability factor\0""Scale instability factor\0""Scale instability factor\0""Фактор нестабильности шкалы",
					 identifyControl.fSCALEINSTABILITY,
					 1.0,
					 3.0,
					 "%\0""%\0""%\0""%",
					 "%.1f",
					 SETUPMode_menu1_instability_edit_done);
	return (BOOL)-1;
}

void SETUPMode_menu1_instability_edit_done(BOOL bOK)
{
	if(bOK)
	{
		identifyControl.fSCALEINSTABILITY  = atof(EditModeControl.edit_buf);
		identifyControl.SCALEINSTABILITY = identifyControl.fSCALEINSTABILITY*10;
		if(!identify_write_identify_ini_int("identifyControl","SCALEINSTABILITY",identifyControl.SCALEINSTABILITY))
		{
			;//!!!!!!!!!error
		}
	}
	SETUPMode_setModeOnSelf();
}



const char* SETUPMode_menu1_select_lib_onUpdate(void)
{
	return "Nuclide library\0""Nuclide library\0""Nuclide library\0""Библиотека нуклидов";
}
const char* SETUPMode_menu1_IDthreshold_onUpdate(void)
{
	return "Threshold\0""Threshold\0""Threshold\0""Порог";
}
const char* SETUPMode_menu1_instability_onUpdate(void)
{
	return "Instability\0""Instability\0""Instability\0""Нестабильность";
}



BOOL SETUPMode_menu1_del_spz(void)
{
	YESNOMode_DoNotModal(RED, "REQUEST\0""REQUEST\0""REQUEST\0""ЗАПРОС",
		"You are going to DELETE ALL SPECTRA FILES. Please confirm...\0"
		"You are going to DELETE ALL SPECTRA FILES. Please confirm...\0"
		"You are going to DELETE ALL SPECTRA FILES. Please confirm...\0"
	"Вы собираетесь УДАЛИТЬ ВСЕ ФАЙЛЫ СПЕКТРОВ. Пожалуйста подтвердите...", SETUPMode_menu1_del_spz_confirm);
	return TRUE;
}
void SETUPMode_menu1_del_spz_confirm(BOOL bYes)
{
	if(bYes)
	{
		int progress=0;
		//delete all spz files
		HFILE hfile=NULL;
		do
		{
			hfile = filesystem_open_first("spz");
			if(hfile!=NULL)
			{
				if(filesystem_delete_file(hfile)==E_FAIL)
				{
					if(powerControl.bBatteryAlarm)//when battery discharged no file operation is allowed
						break;
				}
				SETUPMode_pleaseWait(++progress);
			}
			PowerControl_kickWatchDog();
		}while(hfile!=NULL);
	}
	Modes_clearModeArea();	//clear screen area for mode
	Modes_updateMode();
}






BOOL SETUPMode_menu1_del_autospz(void)
{
	YESNOMode_DoNotModal(RED, "REQUEST\0""REQUEST\0""REQUEST\0""ЗАПРОС",
		"You are going to DELETE ALL AUTOMATICALLY CREATED SPECTRA FILES. Please confirm...\0"
		"You are going to DELETE ALL AUTOMATICALLY CREATED SPECTRA FILES. Please confirm...\0"
		"You are going to DELETE ALL AUTOMATICALLY CREATED SPECTRA FILES. Please confirm...\0"
	"Вы собираетесь УДАЛИТЬ ВСЕ АВТОМАТИЧЕСКИ СОЗДАННЫЕ ФАЙЛЫ СПЕКТРОВ. Пожалуйста подтвердите...", SETUPMode_menu1_del_autospz_confirm);
	return TRUE;
}
void SETUPMode_menu1_del_autospz_confirm(BOOL bYes)
{
	if(bYes)
	{
		int progress = 0;
		//delete all autospz files
		HFILE hfile=NULL;
		do
		{
			hfile = filesystem_open_firstEx("спек_", "spz");
			if(hfile!=NULL)
			{
				if(filesystem_delete_file(hfile)==E_FAIL)
				{
					if(powerControl.bBatteryAlarm)//when battery discharged no file operation is allowed
						break;
				}
				SETUPMode_pleaseWait(++progress);
			}
			PowerControl_kickWatchDog();
		}while(hfile!=NULL);
		do
		{
			hfile = filesystem_open_firstEx("spec_", "spz");
			if(hfile!=NULL)
			{
				if(filesystem_delete_file(hfile)==E_FAIL)
				{
					if(powerControl.bBatteryAlarm)//when battery discharged no file operation is allowed
						break;
				}
			}
			PowerControl_kickWatchDog();
		}while(hfile!=NULL);
	}
	Modes_clearModeArea();	//clear screen area for mode
	Modes_updateMode();
}






BOOL SETUPMode_menu1_del_log(void)
{
	YESNOMode_DoNotModal(RED, "REQUEST\0""REQUEST\0""REQUEST\0""ЗАПРОС",
		"You are going to DELETE ALL LOG FILES. Please confirm...\0"
		"You are going to DELETE ALL LOG FILES. Please confirm...\0"
		"You are going to DELETE ALL LOG FILES. Please confirm...\0"
			"Вы собираетесь УДАЛИТЬ ВСЕ ФАЙЛЫ ЖУРНАЛОВ. Пожалуйста подтвердите...", SETUPMode_menu1_del_log_confirm);
	return TRUE;
}
void SETUPMode_menu1_del_log_confirm(BOOL bYes)
{
	if(bYes)
	{
		int progress=0;
		//delete all spz files
		HFILE hfile =NULL;
		do
		{
			hfile = filesystem_open_first("log");
			if(hfile!=NULL)
			{
				if(filesystem_delete_file(hfile)==E_FAIL)
				{
					if(powerControl.bBatteryAlarm)//when battery discharged no file operation is allowed
						break;
				}
				SETUPMode_pleaseWait(++progress);
			}
			PowerControl_kickWatchDog();
		}while(hfile!=NULL);
		//создать новй лог
		LOGMode_createLog();
	}
	Modes_clearModeArea();	//clear screen area for mode
	Modes_updateMode();
}
BOOL SETUPMode_menu1_del_mcs(void)
{
	YESNOMode_DoNotModal(RED, "REQUEST\0""REQUEST\0""REQUEST\0""ЗАПРОС",
		"You are going to DELETE ALL MCS FILES. Please confirm...\0"
		"You are going to DELETE ALL MCS FILES. Please confirm...\0"
		"You are going to DELETE ALL MCS FILES. Please confirm...\0"
			"Вы собираетесь УДАЛИТЬ ВСЕ ФАЙЛЫ MCS. Пожалуйста подтвердите...", SETUPMode_menu1_del_mcs_confirm);
	return TRUE;
}
void SETUPMode_menu1_del_mcs_confirm(BOOL bYes)
{
	if(bYes)
	{
		int progress=0;
		//delete all spz files
		HFILE hfile =NULL;
		do
		{
			hfile = filesystem_open_first("mcs");
			if(hfile!=NULL)
			{
				if(filesystem_delete_file(hfile)==E_FAIL)
				{
					if(powerControl.bBatteryAlarm)//when battery discharged no file operation is allowed
						break;
				}
				SETUPMode_pleaseWait(++progress);
			}
			PowerControl_kickWatchDog();
		}while(hfile!=NULL);
	}
	Modes_clearModeArea();	//clear screen area for mode
	Modes_updateMode();
}

BOOL SETUPMode_menu1_del_lib(void)
{
	YESNOMode_DoNotModal(RED, "REQUEST\0""REQUEST\0""REQUEST\0""ЗАПРОС",
		"You are going to DELETE ALL NUC.LIB FILES. Please confirm...\0"
		"You are going to DELETE ALL NUC.LIB FILES. Please confirm...\0"
		"You are going to DELETE ALL NUC.LIB FILES. Please confirm...\0"
			"Вы собираетесь УДАЛИТЬ ВСЕ ФАЙЛЫ БИБЛ.НУКЛ. Пожалуйста подтвердите...", SETUPMode_menu1_del_lib_confirm);
	return TRUE;
}
void SETUPMode_menu1_del_lib_confirm(BOOL bYes)
{
	if(bYes)
	{
		int progress=0;
		//delete all spz files
		HFILE hfile=NULL;
		do
		{
			hfile = filesystem_open_first("lib");
			if(hfile!=NULL)
			{
				if(filesystem_delete_file(hfile)==E_FAIL)
				{
					if(powerControl.bBatteryAlarm)//when battery discharged no file operation is allowed
						break;
				}
			}
			SETUPMode_pleaseWait(++progress);
			PowerControl_kickWatchDog();
		}while(hfile!=NULL);
		Modes_clearModeArea();	//clear screen area for mode
		Modes_OnShow();
	}
}


BOOL SETUPMode_menu1_hardreset(void)
{
	YESNOMode_DoModal(RED, "REQUEST\0""REQUEST\0""REQUEST\0""ЗАПРОС",
	"You are going to DELETE ALL FILES and SET UP ALL SETTINGS TO FACTORY DEFAULTS. Please confirm to pass through security procedure...\0"
	"You are going to DELETE ALL FILES and SET UP ALL SETTINGS TO FACTORY DEFAULTS. Please confirm to pass through security procedure...\0"
	"You are going to DELETE ALL FILES and SET UP ALL SETTINGS TO FACTORY DEFAULTS. Please confirm to pass through security procedure...\0"
		"Вы собираетесь УДАЛИТЬ ВСЕ ФАЙЛЫ и УСТАНОВИТЬ ЗАВОДСКИЕ НАСТРОЙКИ. Пожалуйста, подтвердите, чтобы перейти к процедуре безопасности...", SETUPMode_menu1_hardreset_confirm);
	return TRUE;
}


void SETUPMode_menu1_hardreset_confirm(BOOL bYes)
{
	if(bYes)
	{
		//enter expert mode
		EditMode_EditInt("Security procedure.\rEnter password\0""Security procedure.\rEnter password\0""Security procedure.\rEnter password\0""Процедура безопасности.\rВведите пароль",
						  0,
						  0,
						  9999,
						 "\0""\0""\0""",
						  SETUPMode_menu1_hardreset_confirm_done);
		
		YESNOModeControl.bExitModal = FALSE;
		YESNOMode_modalProcedure();
	}
}


void SETUPMode_menu1_hardreset_confirm_done(BOOL bOK)
{
	if(bOK)
	{
		char pStr[5];
		memset(pStr,0,5);
		SAFE_DECLARE;
		DISABLE_VIC;
		sprintf((char*)pStr, "%02u%02u",
				(UINT)clockData.dateTime.hour, (UINT)clockData.dateTime.minute);
		ENABLE_VIC;

		if(!strcmp(pStr,EditModeControl.edit_buf))
		{
			//reset everything
			filesystem_format_filetable(bOK);
#ifdef DEBUG
			WDMOD_bit.WDRESET = 1;	//must be =1 for RELEASE MODE
#endif	//#ifdef DEBUG
			while(1);
			//recreate files
//			filesystem_check_ini_files();
		}
	}
	YESNOModeControl.bExitModal = TRUE;
}



BOOL SETUPMode_menu1_defaults(void)
{
	YESNOMode_DoNotModal(RED, "REQUEST\0""REQUEST\0""REQUEST\0""ЗАПРОС",
	"You are going to SET ALL SETTINGS TO FACTORY DEFAULTS. Please confirm...\0"
	"You are going to SET ALL SETTINGS TO FACTORY DEFAULTS. Please confirm...\0"
	"You are going to SET ALL SETTINGS TO FACTORY DEFAULTS. Please confirm...\0"
		"Вы собираетесь УСТАНОВИТЬ ЗАВОДСКИЕ НАСТРОЙКИ. Пожалуйста подтвердите...", SETUPMode_menu1_defaults_confirm);
	return TRUE;
}
void SETUPMode_menu1_defaults_confirm(BOOL bYes)
{
	if(bYes)
	{
		SETUPMode_pleaseWait(25);
		//reset ini only
		filesystem_restore_identify_ini();
		SETUPMode_pleaseWait(50);
		filesystem_restore_system_ini_from_backup();
		SETUPMode_pleaseWait(75);
		filesystem_restore_main_lib();
		SETUPMode_pleaseWait(99);
		//open new files
		filesystem_check_ini_files();
	}
	Modes_clearModeArea();	//clear screen area for mode
	Modes_updateMode();
}






const char* SETUPMode_menu1_defaults_onUpdate(void)
{
	return "Reset to defaults\0""Reset to defaults\0""Reset to defaults\0""По-умолчанию";
}
const char* SETUPMode_menu1_del_spz_onUpdate(void)
{
	return "Delete spectra\0""Delete spectra\0""Delete spectra\0""Удалить спектры";
}
const char* SETUPMode_menu1_del_autospz_onUpdate(void)
{
	return "Delete autospectra\0""Delete autospectra\0""Delete autospectra\0""Удалить автоспектры";
}
const char* SETUPMode_menu1_del_log_onUpdate(void)
{
	return "Delete log\0""Delete log\0""Delete log\0""Удалить журнал";
}
const char* SETUPMode_menu1_del_mcs_onUpdate(void)
{
	return "Delete mcs's\0""Delete mcs's\0""Delete mcs's\0""Удалить все mcs";
}
const char* SETUPMode_menu1_del_lib_onUpdate(void)
{
	return "Delete nuc.libs\0""Delete nuc.libs\0""Delete nuc.libs\0""Удалить библ.нукл";
}

/*
const char* SETUPMode_menu1_hardreset_onUpdate(void)
{
	return "Hard reset\0""Hard reset\0""Hard reset\0""Жесткий сброс";
}
*/


















///////////////////////////////////////////////////////////////////////////////





void SETUPMode_Init(void)
{
	SETUPModeControl.iModeScreen = 0;
	SETUPModeControl.uiCalcFreeMem = 0;
	SETUPModeControl.uiAllFiles = 0;
	SETUPModeControl.uiSPZFiles = 0;
	SETUPModeControl.uiLIBFiles = 0;
	SETUPModeControl.uiMCSFiles = 0;
	SETUPModeControl.uiLOGFiles = 0;
	SETUPMode_clearBuffers();
//	SETUPModeControl.bExpertMode = FALSE;
}

void SETUPMode_clearBuffers(void)
{
	memset(SETUPModeControl.buf1,0,sizeof(SETUPModeControl.buf1));
	memset(SETUPModeControl.buf2,0,sizeof(SETUPModeControl.buf2));
	memset(SETUPModeControl.buf3,0,sizeof(SETUPModeControl.buf3));
	memset(SETUPModeControl.buf4,0,sizeof(SETUPModeControl.buf4));
	memset(SETUPModeControl.buf5,0,sizeof(SETUPModeControl.buf5));
	memset(SETUPModeControl.buf6,0,sizeof(SETUPModeControl.buf6));
	memset(SETUPModeControl.buf7,0,sizeof(SETUPModeControl.buf7));
	memset(SETUPModeControl.buf8,0,sizeof(SETUPModeControl.buf8));
/*	SETUPModeControl.buf1[0]='\r';
	SETUPModeControl.buf2[0]='\r';
	SETUPModeControl.buf3[0]='\r';
	SETUPModeControl.buf4[0]='\r';
	SETUPModeControl.buf5[0]='\r';
	SETUPModeControl.buf6[0]='\r';
	SETUPModeControl.buf7[0]='\r';
	SETUPModeControl.buf8[0]='\r';*/
//	SETUPModeControl.bExpertMode = FALSE;
//	SETUPModeControl.isplash = 0;	//to get splash screens
//	SETUPModeControl.iJustLeaveScreen = 0;	//to get splash screens
}

BOOL SETUPMode_OnActivate(void)
{
	SETUPMode_clearBuffers();
	Modes_createTimer(1000);
	SETUPModeControl.iModeScreen = ENU_SETUP_MODE_SCR_VER;
	Modes_updateMode();
	return 1;
}

BOOL SETUPMode_OnTimer(void)
{
	UINT erval;
	int ret;
	switch(SETUPModeControl.iModeScreen)
	{
	case ENU_SETUP_MODE_SCR_VER://screen of 	
//		if(!SETUPModeControl.iJustLeaveScreen)//!!!!!!!!!!!! to get splash screens
			SETUPMode_showDateTime();
		break;
	case ENU_SETUP_MODE_SCR_GEIGER:
#ifdef BNC		
		ret = sprintf(SETUPModeControl.buf1, "%.3f", geigerControl.esentVals_safe.fDoserate);
#else
		ret = sprintf(SETUPModeControl.buf1, "%.2f", geigerControl.esentVals_safe.fDoserate);
#endif
		if(ret>=sizeof(SETUPModeControl.buf1))
			exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
		erval = (UINT)(geigerControl.esentVals_safe.fCpsErr+0.5);
		if(erval<1)
			erval = 1;
		ret = sprintf(SETUPModeControl.buf2, "%u%%", erval);
		if(ret>=sizeof(SETUPModeControl.buf2))
			exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
		ret = sprintf(SETUPModeControl.buf3, "%.3f", geigerControl.esentVals_safe.fCps);
		if(ret>=sizeof(SETUPModeControl.buf3))
			exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
		ret = sprintf(SETUPModeControl.buf4, "%u", (UINT)geigerControl.dwTotalCount);
		if(ret>=sizeof(SETUPModeControl.buf4))
			exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
		ret = sprintf(SETUPModeControl.buf5, "%u", (UINT)geigerControl.dwTotalTime);
		if(ret>=sizeof(SETUPModeControl.buf5))
			exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
		ret = sprintf(SETUPModeControl.buf6, "%f", (float)geigerControl.fDrCoef);
		if(ret>=sizeof(SETUPModeControl.buf6))
			exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
		ret = sprintf(SETUPModeControl.buf7, "%g", (float)geigerControl.fDrDeadTime);
		if(ret>=sizeof(SETUPModeControl.buf7))
			exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
		ret = sprintf(SETUPModeControl.buf8, "%f", (float)geigerControl.fDrSelfCps);
		if(ret>=sizeof(SETUPModeControl.buf8))
			exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
#ifdef BNC
		ret = sprintf(SETUPModeControl.buf9, "%.2f mrem/h", (float)SPRDModeControl.fDRThreshold);
#else		
		ret = sprintf(SETUPModeControl.buf9, "%.1f µSv/h", (float)SPRDModeControl.fDRThreshold);
#endif
		if(ret>=sizeof(SETUPModeControl.buf9))
			exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
		Modes_OnShow();
	break;
	case ENU_SETUP_MODE_SCR_BAT://screen of battery
		ret = sprintf(SETUPModeControl.buf1, "%u%%", (UINT)powerControl.batCapacity);
		if(ret>=sizeof(SETUPModeControl.buf1))
			exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
		ret = sprintf(SETUPModeControl.buf2, "%.1f", (float)powerControl.batV_aver);
		if(ret>=sizeof(SETUPModeControl.buf2))
			exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
		ret = sprintf(SETUPModeControl.buf3, "%.1f", (float)powerControl.fBatCoef);
		if(ret>=sizeof(SETUPModeControl.buf3))
			exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
		Modes_OnShow();
		break;
	default:
		;
	};
	return 1;
}


BOOL SETUPMode_OnLeft(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return 1;
}
BOOL SETUPMode_OnRight(void)
{
	Modes_setActiveMode(&modes_LOGMode);
	return 1;
}
BOOL SETUPMode_OnUp(void)
{
	if(++SETUPModeControl.iModeScreen==MAX_SETUP_MODE_SCREENS)
		SETUPModeControl.iModeScreen = 0;
	
	SETUPMode_clearBuffers();
	Modes_updateMode();
	
	if(SETUPModeControl.iModeScreen==ENU_SETUP_MODE_SCR_MEM)
	{//get mem data, have to wait for

		SETUPMode_pleaseWait(100);
		
		filesystem_calc_special_files_number(&SETUPModeControl.uiAllFiles,
							&SETUPModeControl.uiSPZFiles,
							&SETUPModeControl.uiLIBFiles,
							&SETUPModeControl.uiMCSFiles,
							&SETUPModeControl.uiLOGFiles);
		
		Modes_clearModeArea();	//clear screen area for mode
	}
	
	Modes_OnShow();
	return 1;
}

void SETUPMode_pleaseWait(int progress)
{
	Modes_clearModeArea();	//clear screen area for mode
	Display_setCurrentFont(fnt16x16);
	Display_setTextWin(0,Y_SCREEN_MAX-200,X_SCREEN_SIZE,16);	//set text window
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_outputTextByLang("Please wait...\0""Bitte warten...\0""Please wait...\0""Пожалуйста ждите...");
	char text[10];
	sprintf(text, "\r%d", progress);
	Display_outputText(text);
}



void SETUPMode_PSW_done(BOOL bOK)
{
	if(bOK)
	{

//in debug mode dont need to enter psw
#ifdef DEBUG
		RIDControl.bExpertMode = TRUE;
#else
		char pStr[5];
		memset(pStr,0,5);
		SAFE_DECLARE;
		DISABLE_VIC;
		sprintf((char*)pStr, "%02u%02u",
				(UINT)clockData.dateTime.hour, (UINT)clockData.dateTime.minute);
		ENABLE_VIC;

		if(!strcmp(pStr,EditModeControl.edit_buf))
{
			RIDControl.bExpertMode = TRUE;
			SETUPMode_accessGranted();
}else
{
			SETUPMode_accessDenied();
}
#endif		//#ifdef DEBUG
	}
	SETUPMode_setModeOnSelf();
}

//показать сообщение что доступ закрыт и ожидать 2 сек или надатия кнопки
void SETUPMode_accessDenied(void)
{
	SoundControl_BeepSeq(beepSeq_NOK);

	
	Modes_clearModeArea();	//clear screen area for mode
	Display_setCurrentFont(fnt16x16);
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextXY(0,20);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextJustify(CENTER);
	Display_outputTextByLang("Access denied\0""Access denied\0""Access denied\0""В доступе отказано");
	PowerControl_sleep(1000);
	PowerControl_sleep(1000);
/*	int cnt = 20;
	do
	{
		PowerControl_sleep(100);
	}while(!KeyboardControl_anyKeyPressed_hardware_intcall() && --cnt);*/
}


//показать сообщение что доступ открыт и ожидать 2 сек или надатия кнопки
void SETUPMode_accessGranted(void)
{
	Modes_clearModeArea();	//clear screen area for mode
	Display_setCurrentFont(fnt16x16);
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextXY(0,20);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextJustify(CENTER);
	Display_outputTextByLang("Access granted\0""Access granted\0""Access granted\0""Доступ открыт");
	PowerControl_sleep(1000);
	PowerControl_sleep(1000);
/*	int cnt = 20;
	do
	{
		PowerControl_sleep(100);
	}while(!KeyboardControl_anyKeyPressed_hardware_intcall() && --cnt);*/
}


BOOL SETUPMode_OnDown(void)
{
	if(!RIDControl.bExpertMode)
	{
		//protected mode, password enter
	//enter expert mode
		EditMode_EditInt("Expert mode\rEnter password\0""Expert mode\rEnter password\0""Expert mode\rEnter password\0""Экспертный режим\rВведите пароль",
							  0,
							  0,
							  9999,
							 "\0""\0""\0""",
							  SETUPMode_PSW_done);
		return 1;
	}
	switch(SETUPModeControl.iModeScreen)
	{
	case ENU_SETUP_MODE_SCR_VER:
		Modes_activateMenu(&setup_menu_version);
		break;
	case ENU_SETUP_MODE_SCR_MEM:
		Modes_activateMenu(&setup_menu_memory);
		break;
	case ENU_SETUP_MODE_SCR_LIB:
		Modes_activateMenu(&setup_menu_library);
		break;
	case ENU_SETUP_MODE_SCR_GEIGER:
		Modes_activateMenu(&setup_menu_geiger);
		break;
	case ENU_SETUP_MODE_SCR_SPEC:
		Modes_activateMenu(&setup_menu_spec);
		break;
	case ENU_SETUP_MODE_SCR_BAT:
		Modes_activateMenu(&setup_menu_bat);
		break;
	default:
		;
	}
//	
	return 1;
}
BOOL SETUPMode_OnIdle(void)
{
	return 1;
}
BOOL SETUPMode_OnShow(void)
{
	SETUPMode_showModeScreen();
	return 1;
}
BOOL SETUPMode_OnExit(void)
{
	return 1;
}
BOOL SETUPMode_OnWakeUp(void)
{
//	SETUPMode_showModeScreen();
	return 1;
}
BOOL SETUPMode_OnPowerDown(void)
{
	return 1;	//allow enter power down
}


void SETUPMode_showDateTime(void)
{
	if(clockData.bMustBeInited && (clockData.dateTime.second&0x1))
		Display_setTextColor(RED);	//set text color of clock if it needs to be inited
	else
		Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(10);
	//show date and time
	char buf[30];
	Display_outputText(Clock_getClockDateTimeStr(buf));
	///////////////////////
}



void SETUPMode_showVersion(int y)
{
	//show version number
	char buf[10];
	Display_setCurrentFont(fnt16x16);
	Display_setTextWin(0,Y_SCREEN_MAX-y,X_SCREEN_SIZE,y-MODE_USER_BOTTOM);	//set text window
	Display_setTextColor(ORANGE);	//set text color
	Display_setTextWrap(1);
#ifdef BNC	
	Display_outputText("palmRAD (Model 920)\r");
#else	
	Display_outputText("RadSearcher (AT1321)\r");
#endif
	Display_outputTextByLang("Serial: \0""Serial: \0""Serial: \0""Зав.ном.: ");
	sprintf(buf,"%u\r",(UINT)SETUPModeControl.Serial);
	Display_outputText(buf);
	Display_outputTextByLang("Manufact.: \0""Manufact.: \0""Manufact.: \0""Выпуск: ");
	sprintf(buf,"%u.%u\r",(UINT)SETUPModeControl.ManufacturedMonth,(UINT)SETUPModeControl.ManufacturedYear);
	Display_outputText(buf);
	Display_setTextColor(BROWN);	//set text color
	Display_outputTextByLang(txtVersion);	//"ОТПУСТИТЕ КНОПКУ"
	Display_outputText("\r");
	Display_outputTextByLang(txtCompileDate);
	Display_outputText("\r");
	Display_setCurrentFont(fnt6x16);
	Display_outputText("CRC32: ");
	Display_outputText(SETUPModeControl.lpzCalcCRC32);
	Display_setCurrentFont(fnt16x16);
	Display_outputText("\r");
	Display_setTextColor(YELLOW);	//set text color
	Display_outputText("NaI(Tl) D25x40mm\r");
	Display_outputTextByLang("MCA: 1024 channels\r\0""MCA: 1024 channels\r\0""MCA: 1024 channels\r\0""АЦП: 1024 канала\r");
	Display_outputTextByLang("GM counter, USB, Bluetooth, GPS\r\0""GM counter, USB, Bluetooth, GPS\r\0""GM counter, USB, Bluetooth, GPS\r\0""Счетчик ГМ, USB, Bluetooth, GPS\r");
//	Display_outputTextByLang("GM counter: yes\r\0""GM counter: yes\r\0""GM counter: yes\r\0""Счетчик ГМ: да\r");
//	Display_outputTextByLang("Neutron detect.: no\r\0""Neutron detect.: no\r\0""Neutron detect.: no\r\0""Дет.нейтронов: нет\r");
//	Display_outputTextByLang("USB: yes\r\0""USB: yes\r\0""USB: yes\r\0""USB: да\r");
//	Display_outputTextByLang("Bluetooth: yes\r\0""Bluetooth: yes\r\0""Bluetooth: yes\r\0""Bluetooth: да\r");
//	Display_outputTextByLang("GPS: yes\r\0""GPS: yes\r\0""GPS: yes\r\0""GPS: да\r");
//	Display_outputTextByLang("Memory rewrites:\0""Memory rewrites:\0""Memory rewrites:\0""Перезаписей:");
//	sprintf(buf,"%u\r",(UINT)EEPROMControl.wdEepromWritesCounter);
//	Display_outputText(buf);
#ifdef BNC	
	Display_outputText("2015 © Manufacturer\r");
#else
	Display_outputText("2015 © ATOMTEX SPE\r");
#endif
	///////////////////////
}


void SETUPMode_showSpecification(void)
{
	Display_setCurrentFont(fnt8x16);
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextColor(YELLOW);	//set text color
	Display_outputTextByLang("Gamma radiation dose rate\r\0""Gamma radiation dose rate\r\0""Gamma radiation dose rate\r\0""Мощность дозы гамма излуч.\r");
#ifdef BNC	
	Display_outputTextByLang("Range: 0.003-10000 mrem/h\r\0""Range: 0.003-10000 mrem/h\r\0""Range: 0.003-10000 mrem/h\r\0""Диапазон: 0.003-10000 mrem/h\r");
#else	
	Display_outputTextByLang("Range: 0.03-100000 µSv/h\r\0""Range: 0.03-100000 µSv/h\r\0""Range: 0.03-100000 µSv/h\r\0""Диапазон: 0.03-100000 µSv/h\r");
#endif
	Display_outputTextByLang("Basic error: 20%\r\0""Basic error: 20%\r\0""Basic error: 20%\r\0""Основная погрешность: 20%\r");
	Display_outputTextByLang("Energy dependence: 25%\r\0""Energy dependence: 25%\r\0""Energy dependence: 25%\r\0""Энергетич.зависимость: 25%\r");
	Display_outputTextByLang("NaI max cps: 100000\r\0""NaI max  cps: 100000\r\0""NaI max  cps: 100000\r\0""Макс.ск.счета NaI: 100000 cps\r");
#ifdef BNC	
	Display_outputTextByLang("Sens. on Cs137: 4000 cps*h/mrem\r\0""Sens. on Cs137: 4000 cps*h/mrem\r\0""Sens. on Cs137: 4000 cps*h/mrem\r\0""Чувств. на Cs137: 4000 cps*h/mrem\r");
#else	
	Display_outputTextByLang("Sens. on Cs137: 400 cps*h/µSv\r\0""Sens. on Cs137: 400 cps*h/µSv\r\0""Sens. on Cs137: 400 cps*h/µSv\r\0""Чувств. на Cs137: 400 cps*h/µSv\r");
#endif
	Display_outputTextByLang("Energy range: 20-3000 keV\r\0""Energy range: 20-3000 keV\r\0""Energy range: 20-3000 keV\r\0""Энергет.диапазон: 20-3000 keV\r");
	Display_outputTextByLang("\r\0""\r\0""\r\0""\r");
//	Display_outputTextByLang("Operational sigma: \0""Operational sigma: \0""Operational sigma: \0""Оперативная сигма: ");
	Display_outputTextByLang("False alarm period (min): \0""False alarm period, min: \0""False alarm period, min: \0""Период ложн.тревог, мин: ");
	char buf[20];
	sprintf(buf,"%u\r",SPRDModeControl.false_alarm_period_oper);
	Display_outputText(buf);
//	Display_outputTextByLang("Sleep mode FAP (minutes): \0""Sleep mode FAP (minutes): \0""Sleep mode FAP: (minutes) \0""Дежурный ПЛТ (минут): ");
	//sprintf(buf,"%u\r",SPRDModeControl.false_alarm_period_sleep);
//	Display_outputText(buf);
	
	///////////////////////
}


void SETUPMode_showGeigerInfo(void)
{
	Display_setCurrentFont(fnt16x16);
	Display_setTextColor(YELLOW);	//set text color
	/////////////////////////////////////////////////////////////////////////////
	//show free memory
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,171);	//set text window
//	Display_clearTextWin(300);
	Display_setTextLineClear(1);
	Display_checkForClearLine();
	Display_outputTextByLang("GM counter\r\0""GM counter\r\0""GM counter\r\0""Счетчик ГМ\r");
	Display_outputTextByLang("Dose rate: \0""Dose rate: \0""Dose rate: \0""Мощн.дозы: ");
	Display_outputText(SETUPModeControl.buf1);
	Display_outputText("\r");
	Display_outputTextByLang("Error: \0""Messfehler: \0""Error: \0""Погрешность: ");
	Display_outputText(SETUPModeControl.buf2);
	Display_outputText("\r");
	Display_outputTextByLang("Count rate: \0""Count rate: \0""Count rate: \0""Скор.счета: ");
	Display_outputText(SETUPModeControl.buf3);
	Display_outputText("\r");

//	Display_setTextWin(0,MODE_USER_TOP-76,X_SCREEN_SIZE,95);	//set text window
//	Display_clearTextWin(200);
	Display_outputTextByLang("Total counts: \0""Total counts: \0""Total counts: \0""Общий счет: ");
	Display_outputText(SETUPModeControl.buf4);
	Display_outputText("\r");
	Display_outputTextByLang("Total time: \0""Total time: \0""Total time: \0""Общее время: ");
	Display_outputText(SETUPModeControl.buf5);
	Display_outputText("\r");
	Display_outputTextByLang("Coef: \0""Coef: \0""Coef: \0""Коэф: ");
	Display_outputText(SETUPModeControl.buf6);
	Display_outputText("\r");

	Display_outputTextByLang("DeadTm: \0""DeadTm: \0""DeadTm: \0""МертВр: ");
	Display_outputText(SETUPModeControl.buf7);
	Display_outputText("\r");

	Display_outputTextByLang("SelfCps: \0""SelfCps: \0""SelfCps: \0""СобсФон: ");
	Display_outputText(SETUPModeControl.buf8);
	Display_outputText("\r");
	Display_setCurrentFont(fnt8x16);
	Display_outputTextByLang("Safety threshold: \0""Safety threshold: \0""Safety threshold: \0""Порог безопасности: ");
	Display_outputText(SETUPModeControl.buf9);
	Display_setTextLineClear(0);
}


void SETUPMode_showBatteryInfo(void)
{
	Display_setCurrentFont(fnt16x16);
	Display_setTextColor(YELLOW);	//set text color
	/////////////////////////////////////////////////////////////////////////////
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,76);	//set text window
//	Display_clearTextWin(10);
	Display_setTextLineClear(1);
	Display_checkForClearLine();
	Display_outputTextByLang("Battery\r\0""Battery\r\0""Battery\r\0""Батарея\r");
	Display_outputTextByLang("Charge: \0""Charge: \0""Charge: \0""Заряд: ");
	Display_outputText(SETUPModeControl.buf1);
	Display_outputText("\r");

//	Display_setTextWin(0,MODE_USER_TOP-38,X_SCREEN_SIZE,38);	//set text window
//	Display_clearTextWin(300);
	
	if(!powerControl.bBatteryAlarm || powerControl.batCapacity<=VREF_BAT_MIN_CRITICAL)
		Display_outputTextByLang("Status: charged\r\0""Status: charged\r\0""Status: charged\r\0""Состояние: заряжены\r");
	else
		Display_outputTextByLang("Status: discharged\r\0""Status: discharged\r\0""Status: discharged\r\0""Состояние: разряжены\r");
	///////////////////////
	
	Display_outputTextByLang("Voltage: \0""Voltage: \0""Voltage: \0""Напряжение: ");
	Display_outputText(SETUPModeControl.buf2);
	Display_outputText("\r");

	Display_outputTextByLang("Calibr.coef: \0""Calibr.coef: \0""Calibr.coef: \0""Калибр.коэф: ");
	Display_outputText(SETUPModeControl.buf3);
	Display_setTextLineClear(0);

}


void SETUPMode_showLibrary(void)
{
	char buf[60];
	Display_setCurrentFont(fnt16x16);
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	//library chapter
	/////////////////////////////////////////////////////
	//show lib version
	Display_setTextColor(ORANGE);	//set text color
	Display_outputTextByLang("Nuc.lib: \0""Nuc.lib: \0""Nuc.lib: \0""Библ.нук: ");
	Display_outputText(identifyControl.libraryFileName);
	Display_outputText("\r");
	int ret = sprintf(buf, "%d\r", identifyControl.dwLibVer);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Version: \0""Version: \0""Version: \0""Версия: ");
	Display_outputText(buf);
	ret = sprintf(buf, "%d\r", identifyControl.NUCLNUM);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Nuclides: \0""Nuclides: \0""Nuclides: \0""Нуклидов: ");
	Display_outputText(buf);
	ret = sprintf(buf, "%d\r", identifyControl.threshold);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Threshold: \0""Threshold: \0""Threshold: \0""Порог: ");
	Display_outputText(buf);
/*	ret = sprintf(buf, "%d\r", identifyControl.MINENERGY);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Min.energy, keV: \0""Min.energy, keV: \0""Min.energy, keV: \0""Мин.энергия, кэВ: ");
	Display_outputText(buf);*/
	ret = sprintf(buf, "%.1f%%\r", (float)identifyControl.SCALEINSTABILITY/10);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Scale instab.: \0""Scale instab.: \0""Scale instab.: \0""Нестаб.шкалы: ");
	Display_outputText(buf);
	///////////////////////
	
}


void SETUPMode_showMemory(void)
{
	char buf[60];
	Display_setCurrentFont(fnt16x16);
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextColor(BROWN);	//set text color
	//memory chapter
	/////////////////////////////////////////////////////////////////////////////
	//show free memory
	UINT fmem = SETUPModeControl.uiCalcFreeMem; //in KB
	UINT totmem = TOTAL_MEMORY;
	int ret = sprintf(buf, "%u kB\r", (UINT)totmem);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Memory: \0""Memory: \0""Memory: \0""Память: ");
	Display_outputText(buf);
	
	ret = sprintf(buf, "%u kB\r", (UINT)totmem-fmem);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("In use: \0""In use: \0""In use: \0""Занято: ");
	Display_outputText(buf);
	
	ret = sprintf(buf, "%u%%\r", (UINT)(fmem*100/totmem));
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Free: \0""Free: \0""Free: \0""Свободно: ");
	Display_outputText(buf);
	///////////////////////
	
	//show number of files
	UINT files = SETUPModeControl.uiAllFiles;//filesystem_calc_files_number(NULL);
	ret = sprintf(buf, "%u\r", (UINT)files);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Files: \0""Files: \0""Files: \0""Файлов: ");
	Display_outputText(buf);
	
	ret = sprintf(buf, "%u%%\r", (UINT)(100-files*100/MAX_FILES));
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Free: \0""Free: \0""Free: \0""Свободно: ");
	Display_outputText(buf);
	///////////////////////
	
	//show number of spectra
	files = SETUPModeControl.uiSPZFiles;//filesystem_calc_files_number("spz");
	ret = sprintf(buf, "%u\r", (UINT)files);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Spectra: \0""Spectra: \0""Spectra: \0""Спектров: ");
	Display_outputText(buf);
	///////////////////////
	
	//show number of libraries
	files = SETUPModeControl.uiLIBFiles;//filesystem_calc_files_number("lib");
	ret = sprintf(buf, "%u\r", (UINT)files);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Nuc.libs: \0""Nuc.libs: \0""Nuc.libs: \0""Библ.нукл: ");
	Display_outputText(buf);
	///////////////////////

	//show number of mcs files
	files = SETUPModeControl.uiMCSFiles;//filesystem_calc_files_number("mcs");
	ret = sprintf(buf, "%u\r", (UINT)files);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("MCS files: \0""MCS files: \0""MCS files: \0""MCS файлов: ");
	Display_outputText(buf);
	///////////////////////

	//show log size
	if(LOGModeControl.hfile_log)
		files = filesystem_get_length(LOGModeControl.hfile_log);
	else 
		files = 0;
	ret = sprintf(buf, "%u kB\r", (UINT)files/1000+1);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Log file: \0""Log file: \0""Log file: \0""Журнал: ");
	Display_outputText(buf);
	///////////////////////

	//show eeprom write counter
	ret = sprintf(buf, "%u", (UINT)EEPROMControl.wdEepromWritesCounter);
	if(ret>=sizeof(buf))
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error");
	Display_outputTextByLang("Rewrites: \0""Rewrites: \0""Rewrites: \0""Перезаписей: ");
	Display_outputText(buf);
	///////////////////////
}


void SETUPMode_showModeScreen(void)
{
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,3);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt16x16);

	switch(SETUPModeControl.iModeScreen)
	{
	case ENU_SETUP_MODE_SCR_VER:
		SETUPMode_showVersion(245);
		SETUPMode_showDateTime();
		break;
	case ENU_SETUP_MODE_SCR_MEM:
		SETUPMode_showMemory();
		break;
	case ENU_SETUP_MODE_SCR_LIB:
		SETUPMode_showLibrary();
		break;
	case ENU_SETUP_MODE_SCR_SPEC:
		SETUPMode_showSpecification();
		break;
	case ENU_SETUP_MODE_SCR_BAT:
		SETUPMode_showBatteryInfo();
		break;
	case ENU_SETUP_MODE_SCR_GEIGER:
		SETUPMode_showGeigerInfo();
		break;
	default:
		exception(__FILE__,__FUNCTION__,__LINE__,"internal error (iModeScreen)");
	}

}













BOOL SETUPMode_menu1_batcoef(void)
{
	EditMode_EditFloat("Battery calibration coefficient\0""Battery calibration coefficient\0""Battery calibration coefficient\0""Калибровочный коэффициент батареи",
					 powerControl.fBatCoef,
					 MIN_BAT_COEF,
					 MAX_BAT_COEF,
					 "\0""\0""\0""",
					 "%f",
					 SETUPMode_menu1_batcoef_edit_done);
	return (BOOL)-1;
}


void SETUPMode_menu1_batcoef_edit_done(BOOL bOK)
{
	if(bOK)
	{
		powerControl.fBatCoef  = atof(EditModeControl.edit_buf);
		BOOL bbatal = powerControl.bBatteryAlarm;	//to allow file save
		powerControl.bBatteryAlarm = 0;	//to allow file save
		if(!ini_write_system_ini_float("powerControl", "fBatCoef", powerControl.fBatCoef))
		{
			;//!!!!!!!!!error
		}
		powerControl.bBatteryAlarm = bbatal;
	}
	SETUPMode_setModeOnSelf();
}









//включить режим приработки
BOOL SETUPMode_menu1_prirabotka(void)
{
	YESNOMode_DoModal(GREEN, "Breaking-in\0""Breaking-in\0""Breaking-in\0""Приработ.", 
					  "GM counter Breaking-in. Device will go in deep power saving mode. LCD will off. Press POWER to turn off device.\0""GM counter Breaking-in. Device will go in deep power saving mode. LCD will off. Press POWER to turn off device.\0""GM counter Breaking-in. Device will go in deep power saving mode. LCD will off. Press POWER to turn off device.\0""Приработка счетчика Гейгера-Мюллера. Прибор перейдет в режим максимального энергосбережения. Экран будет отключен. Нажмите POWER для выключения прибора.", 
					  SETUPMode_prirabotka);
	return (BOOL)-1;
}

void SETUPMode_prirabotka(BOOL bYes)
{
	if(bYes)
	{//breaking-in
		PowerControl_turnOFF_MAM();
		PowerControl_turboModeOFF();
#ifndef GPS_BT_FREE	
		Bluetooth_turnOFF();
		GPS_turnOFF();
#endif	//#ifndef GPS_BT_FREE	
		PowerControl_sendAllCommands();
		SoundControl_StopBeep();
		SoundControl_StopVibro();

		Modes_setActiveMode(NULL);
		
		//turn off battery low to allow eeprom operations	
		powerControl.bBatteryAlarm = 0;
		EEPROM_UpdateEssentialDataInEeprom();

//		CIIR_bit.IMSEC = 0;	//turn off clock interrupts
		Display_turnOFF_LEDs();

		Display_turnOFF();
		

		//adjust wakeup interrupts
		INTWAKE_bit.EXTWAKE2 = 1; //key pushing
		INTWAKE_bit.EXTWAKE1 = 0; //waking up from second proc
		INTWAKE_bit.GPIO0WAKE = 0; //geiger
		INTWAKE_bit.RTCWAKE = 1; //clock
		
		do
		{
			PowerControl_kickWatchDog();
			
			//reset all awaking statuses before entering power down
			powerControl.bAwakedByKeyboard = 0;	//mean that processor is awaked by key pressing
			powerControl.bAwakedByClock = 0;	//mean that processor is awaked by clock (1 in a second)
			powerControl.bAwakedByGeiger = 0;
			powerControl.bAwakedByInterProc = 0;	//mean that processor is awaked by second processor
			
			powerControl.bInPowerDownMode = 1;
	
//			Display_flashGreenLED();
			Display_flashRedLED();
//			Display_flashOrangeLED();
			
			PowerControl_enterPowerDownMode();
		}while(!powerControl.bAwakedByKeyboard);
	
		//turn off device
		while(KeyboardControl_anyKeyPressed_hardware_intcall())PowerControl_kickWatchDog();
		CLR_AN_ON;	//turn off second proc
		CLR_DG_ON; //Выключение питания of first proc
		while(1)PowerControl_kickWatchDog();
	}
	
}


const char* SETUPMode_menu1_prirabotka_onUpdate(void)
{
	return "Breaking-in\0""Breaking-in\0""Breaking-in\0""Приработка";
}





BOOL SETUPMode_menu1_setCoef(void)
{
	EditMode_EditFloat("Dose rate coefficient\0""Dose rate coefficient\0""Dose rate coefficient\0""Коэффициент мощности дозы",
					 geigerControl.fDrCoef,
					 0.000001,
					 10.00000,
					 "\0""\0""\0""",
					 "%f",
					 SETUPMode_menu1_setCoef_edit_done);
	return (BOOL)-1;
}


void SETUPMode_menu1_setCoef_edit_done(BOOL bOK)
{
	if(bOK)
	{
		geigerControl.fDrCoef  = atof(EditModeControl.edit_buf);
		if(!ini_write_system_ini_float("geigerControl", "fDrCoef", geigerControl.fDrCoef))
		{
			;//!!!!!!!!!error
		}
	}
	SETUPMode_setModeOnSelf();
}


BOOL SETUPMode_menu1_setDT(void)
{
	EditMode_EditFloat("Deadtime of counter\0""Deadtime of counter\0""Deadtime of counter\0""Мертвое время счетчика",
					 geigerControl.fDrDeadTime*1000000,
					 0.00000,
					 1000.00000,
					 "µs\0""µs\0""µs\0""µs",
					 "%f",
					 SETUPMode_menu1_setDT_edit_done);
	return (BOOL)-1;
}

void SETUPMode_menu1_setDT_edit_done(BOOL bOK)
{
	if(bOK)
	{
		geigerControl.fDrDeadTime  = atof(EditModeControl.edit_buf)/1000000.0;
		if(!ini_write_system_ini_float("geigerControl", "fDrDeadTime", geigerControl.fDrDeadTime))
		{
			;//!!!!!!!!!error
		}
	}
	SETUPMode_setModeOnSelf();
}



BOOL SETUPMode_menu1_setSF(void)
{
	EditMode_EditFloat("Self cps of counter\0""Self cps of counter\0""Self cps of counter\0""Собственный фон счетчика",
					 geigerControl.fDrSelfCps,
					 0.000000,
					 9.00000,
					 "cps\0""cps\0""cps\0""cps",
					 "%f",
					 SETUPMode_menu1_setSF_edit_done);
	return (BOOL)-1;
}

void SETUPMode_menu1_setSF_edit_done(BOOL bOK)
{
	if(bOK)
	{
		geigerControl.fDrSelfCps  = atof(EditModeControl.edit_buf);
		if(!ini_write_system_ini_float("geigerControl", "fDrSelfCps", geigerControl.fDrSelfCps))
		{
			;//!!!!!!!!!error
		}
	}
	SETUPMode_setModeOnSelf();
}



const char* SETUPMode_menu1_setCoef_onUpdate(void)
{
	return "Set coefficient\0""Set coefficient\0""Set coefficient\0""Задать коэф.";
}
const char* SETUPMode_menu1_setDT_onUpdate(void)
{
	return "Set deadtime\0""Set deadtime\0""Set deadtime\0""Задать мертв.время";
}
const char* SETUPMode_menu1_setSF_onUpdate(void)
{
	return "Set self cps\0""Set self cps\0""Set self cps\0""Задать собств.фон";
}












/////////////////////////////////////////////////////////////////////////////////////////////
//specification settings

/*
const char* SETUPMode_menu1_setSigmaOperation_onUpdate(void)
{
	return "Operational sigma\0""Operational sigma\0""Operational sigma\0""Оперативная сигма";
}


BOOL SETUPMode_menu1_setSigmaOperation(void)
{
	EditMode_EditFloat("Operational mode alarm sigma threshold\0""Operational mode alarm sigma threshold\0""Operational mode alarm sigma threshold\0""Порог тревоги в сигмах для оперативного режима",
					 SPRDModeControl.operation_search_sigma,
					 2.00000,
					 10.00000,
					 "\0""\0""\0""",
					 "%f",
					 SETUPMode_menu1_setSigmaOperation_edit_done);
	return (BOOL)-1;
}


void SETUPMode_menu1_setSigmaOperation_edit_done(BOOL bOK)
{
	if(bOK)
	{
		SPRDModeControl.operation_search_sigma  = atof(EditModeControl.edit_buf);
		ini_write_system_ini_float("SPRDModeControl", "operation_search_sigma", SPRDModeControl.operation_search_sigma);
		InterProc_setSigmaSearchThresholds(SPRDModeControl.operation_search_sigma);
	}
	SETUPMode_setModeOnSelf();
}









const char* SETUPMode_menu1_setSigmaSleep_onUpdate(void)
{
	return "Sleepmode sigma\0""Sleepmode sigma\0""Sleepmode sigma\0""Дежурная сигма";
}

BOOL SETUPMode_menu1_setSigmaSleep(void)
{
	EditMode_EditFloat("Sleep mode alarm sigma threshold\0""Sleep mode alarm sigma threshold\0""Sleep mode alarm sigma threshold\0""Порог тревоги в сигмах для дежурного режима",
					 SPRDModeControl.sleepmode_search_sigma,
					 2.00000,
					 10.00000,
					 "\0""\0""\0""",
					 "%f",
					 SETUPMode_menu1_setSigmaSleep_edit_done);
	return (BOOL)-1;
}


void SETUPMode_menu1_setSigmaSleep_edit_done(BOOL bOK)
{
	if(bOK)
	{
		SPRDModeControl.sleepmode_search_sigma  = atof(EditModeControl.edit_buf);
		ini_write_system_ini_float("SPRDModeControl", "sleepmode_search_sigma", SPRDModeControl.sleepmode_search_sigma);
		InterProc_setSigmaSleepThresholds(SPRDModeControl.sleepmode_search_sigma);
	}
	SETUPMode_setModeOnSelf();
}

*/




BOOL SETUPMode_menu1_select_lib(void)
{
	FileListModeControl.pOnNextPage = SETUPMode_menu1_select_lib_onNextPage;
	FileListModeControl.pOnPrevPage = SETUPMode_menu1_select_lib_onPrevPage;
	FileListModeControl.pRetFunction = SETUPMode_menu1_select_lib_done;
	FileListModeControl.iItemsNum = filesystem_get_dir(
					"lib",
					FileListModeControl.listItems,
					MAX_ITEMS,
					0);
	Modes_setActiveMode(&modes_FileListMode);
	return (BOOL)-1;
}

void SETUPMode_menu1_select_lib_done(BOOL bOK)
{
	if(bOK)
	{
		//FileListModeControl.iMarkerPos
		strncpy(identifyControl.libraryFileName,
				FileListModeControl.listItems[FileListModeControl.iMarkerPos].name,
				FILE_NAME_SZ);
		if(identify_write_identify_ini_string("identifyControl", "libraryFileName",
										   identifyControl.libraryFileName))
		{
			int ret = identify_open_library();
			if(ret == E_FAIL)
			{
				YESNOMode_DoModal(RED,"ERROR!\0""ERROR!\0""ERROR!\0""ОШИБКА!",
				"Invalid Selected Library File. Press any allowed key\0"
				"Invalid Selected Library File. Press any allowed key\0"
				"Invalid Selected Library File. Press any allowed key\0"
				"Выбран Неправильный Файл Библиотеки. Нажмите любую разрешенную кнопку",
				&SETUPMode_menu1_select_lib_done_err);
				;	//must be error
			}
		}else
		{
			;//error
		}
	}
	SETUPMode_setModeOnSelf();
}

void SETUPMode_menu1_select_lib_done_err(BOOL bOK)
{
	;
}



BOOL SETUPMode_menu1_select_lib_onNextPage(void)
{
	if(MAX_ITEMS!=FileListModeControl.iItemsNum)return FALSE;
	FileListModeControl.iItemsNum = filesystem_get_dir(
					"lib",
					FileListModeControl.listItems,
					MAX_ITEMS,
					FileListModeControl.iPage*MAX_ITEMS+FileListModeControl.iItemsNum);
	if(FileListModeControl.iItemsNum)
		FileListModeControl.iPage++;
	else
	{
	FileListModeControl.iItemsNum = filesystem_get_dir(
					"lib",
					FileListModeControl.listItems,
					MAX_ITEMS,
					FileListModeControl.iPage*MAX_ITEMS);
	}
	return (FileListModeControl.iItemsNum>0);
}


BOOL SETUPMode_menu1_select_lib_onPrevPage(void)
{
	if(FileListModeControl.iPage==0)return FALSE;
	FileListModeControl.iItemsNum = filesystem_get_dir(
					"lib",
					FileListModeControl.listItems,
					MAX_ITEMS,
					FileListModeControl.iPage*MAX_ITEMS-MAX_ITEMS);
	if(FileListModeControl.iItemsNum)
		FileListModeControl.iPage--;
	else
	{
	FileListModeControl.iItemsNum = filesystem_get_dir(
					"lib",
					FileListModeControl.listItems,
					MAX_ITEMS,
					FileListModeControl.iPage*MAX_ITEMS);
	}
	return (FileListModeControl.iItemsNum>0);
}


/*
BOOL SETUPMode_menu1_dimension(void)
{
	if(++SPRDModeControl.iDimension>=MAX_DIMS)
		SPRDModeControl.iDimension=0;
	if(!ini_write_system_ini_int("SPRDModeControl", "iDimension", SPRDModeControl.iDimension))
	{
		;//!!!!!!!!!error
	}
	return (BOOL)0;
}

const char* SETUPMode_menu1_dimension_onUpdate(void)
{
	switch(SPRDModeControl.iDimension)
	{
		case enu_dim_sv:
			return "DIMENSION: Sv\0""DIMENSION: Sv\0""DIMENSION: Sv\0""РАЗМЕРНОСТЬ: Зв";
		case enu_dim_gy:
			return "DIMENSION: Gy\0""DIMENSION: Gy\0""DIMENSION: Gy\0""РАЗМЕРНОСТЬ: Гр";
		case enu_dim_r:
			return "DIMENSION: R\0""DIMENSION: R\0""DIMENSION: R\0""РАЗМЕРНОСТЬ: Рем";
	default:
		exception(__FILE__,__FUNCTION__,__LINE__,"Internal error");
	}
	return NULL;
}

*/













////////////////////////////////sigmas calculations///////////////////////////////////////////////
/*

BOOL SETUPMode_menu1_calcSleepSigma(void)
{
	EditMode_EditInt("False alarm period in sleep mode\0""False alarm period in sleep mode\0""False alarm period in sleep mode\0""Период ложных тревог в дежурном режиме",
					 SPRDModeControl.false_alarm_period_sleep,
					 1.0,
					 14400.0,
					 "minutes\0""minutes\0""minutes\0""минут",
					 SETUPMode_menu1_calcSleepSigma_edit_done);
	return (BOOL)-1;
}

const char* SETUPMode_menu1_calcSleepSigma_onUpdate(void)
{
	return "Sleep mode FAP\0""Sleep mode FAP\0""Sleep mode FAP\0""Дежурный ПЛТ";
}


void SETUPMode_menu1_calcSleepSigma_edit_done(BOOL bOK)
{
	if(bOK)
	{
		SPRDModeControl.false_alarm_period_sleep = atoi(EditModeControl.edit_buf);
		//compute
		SPRDModeControl.sleepmode_search_sigma = SETUPMode_calcSigma(SPRDModeControl.false_alarm_period_sleep);
//			ini_write_system_ini_float("SPRDModeControl", "sleepmode_search_sigma", SPRDModeControl.sleepmode_search_sigma);
		InterProc_setSigmaSleepThresholds(SPRDModeControl.sleepmode_search_sigma);
		if(!ini_write_system_ini_int("SPRDModeControl", "false_alarm_period_sleep", SPRDModeControl.false_alarm_period_sleep))
		{
			;//!!!!!!!!!error
		}
	}		
	SETUPMode_setModeOnSelf();
}
*/

//just calc sigma for proposed period for any search mode
void SETUPMode_calcSigma(UINT period)
{
	SPRDModeControl.operation_search_sigma[0] = SETUPMode_calcSigmaEx(period, 0.33);
	SPRDModeControl.operation_search_sigma[1] = SETUPMode_calcSigmaEx(period, 0.66);
	SPRDModeControl.operation_search_sigma[2] = SETUPMode_calcSigmaEx(period, 1.00);
	SPRDModeControl.operation_search_sigma[3] = SETUPMode_calcSigmaEx(period, 1.33);
	SPRDModeControl.operation_search_sigma[4] = SETUPMode_calcSigmaEx(period, 1.66);
	SPRDModeControl.operation_search_sigma[5] = SETUPMode_calcSigmaEx(period, 2.00);

/*	char buf[20];
	sprintf(buf,"s1=%.1f",SPRDModeControl.operation_search_sigma[0]);
	LOGMode_insertEvent(buf);
	sprintf(buf,"s2=%.1f",SPRDModeControl.operation_search_sigma[1]);
	LOGMode_insertEvent(buf);
	sprintf(buf,"s3=%.1f",SPRDModeControl.operation_search_sigma[2]);
	LOGMode_insertEvent(buf);
	sprintf(buf,"s4=%.1f",SPRDModeControl.operation_search_sigma[3]);
	LOGMode_insertEvent(buf);
	sprintf(buf,"s5=%.1f",SPRDModeControl.operation_search_sigma[4]);
	LOGMode_insertEvent(buf);
	sprintf(buf,"s6=%.1f",SPRDModeControl.operation_search_sigma[5]);
	LOGMode_insertEvent(buf);*/
}

float SETUPMode_calcSigmaEx(UINT period, float dwell)
{
	float retVal = -1;
	UINT gth;
	double m_gamma_prob;	//probability
	double fap = 60.0*period;
	//for sleep and oper mode
	if(puasson_CalcthG(STANDART_BKG_CPS, fap, dwell, &gth, &m_gamma_prob))
	{//OK
		retVal = fabs((double)gth - STANDART_BKG_CPS)/sqrt(STANDART_BKG_CPS);
	}else
	{
		//error!!!!!!
		exception(__FILE__,__FUNCTION__,__LINE__,"puasson calculation error!");
	}
	return retVal;
}



BOOL SETUPMode_menu1_calcOperSigma(void)
{
	EditMode_EditInt("False alarm period\0""False alarm period\0""False alarm period\0""Период ложных тревог",
					 SPRDModeControl.false_alarm_period_oper,
					 1.0,
					 14400.0,
					 "minutes\0""minutes\0""minutes\0""минут",
					 SETUPMode_menu1_calcOperSigma_edit_done);
	return (BOOL)-1;
}

const char* SETUPMode_menu1_calcOperSigma_onUpdate(void)
{
	return "False alarm period\0""False alarm period\0""False alarm period\0""Период ложн.тревог";
}


void SETUPMode_menu1_calcOperSigma_edit_done(BOOL bOK)
{
	if(bOK)
	{
		SPRDModeControl.false_alarm_period_oper = atoi(EditModeControl.edit_buf);
		//compute
		SETUPMode_calcSigma(SPRDModeControl.false_alarm_period_oper);
//			ini_write_system_ini_float("SPRDModeControl", "operation_search_sigma", SPRDModeControl.operation_search_sigma);
		InterProc_setSigmaSearchThresholds();
		if(!ini_write_system_ini_int("SPRDModeControl", "false_alarm_period_oper", SPRDModeControl.false_alarm_period_oper))
		{
			;//!!!!!!!!!error
		}
	}		
	SETUPMode_setModeOnSelf();
}








////////////////////////////////////////////////////////////////////////////////////////////////////////

//test sound
/*
void SETUPMode_display_soundData(void)
{
	char buf[100];
	sprintf(buf, "\rf=%x; pos=%x", (UINT)soundControl.flg, (UINT)soundControl.addr);
	Display_outputText(buf);
}

BOOL SETUPMode_menu1_recordSound(void)
{
	Display_clearScreen();
	Display_setCurrentFont(fnt16x16);
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextColor(BROWN);	//set text color
	Display_outputText("Recording...");
	sound_ISD4004_Record(0);

	SETUPMode_display_soundData();
	
	PowerControl_sleep(3000);
	return 1;
}

const char* SETUPMode_menu1_recordSound_onUpdate(void)
{
	return "Sound record\0""Sound record\0""Sound record\0""Запись звука";
}


BOOL SETUPMode_menu1_playSound(void)
{
	int i;
	Display_clearScreen();
	Display_setCurrentFont(fnt16x16);
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextColor(BROWN);	//set text color
	Display_outputText("Playing...");

	sound_ISD4004_PowerUp();
	for(i=0;i<SOUND_NUM;i++)
	{
		sound_playSample(i);
		SETUPMode_display_soundData();
		PowerControl_sleep(3000);
	}
	SETUPMode_display_soundData();
	PowerControl_sleep(2000);
	return 1;
}

const char* SETUPMode_menu1_playSound_onUpdate(void)
{
	return "Sound play\0""Sound play\0""Sound play\0""Играть звук";
}


BOOL SETUPMode_menu1_stopRecord(void)
{
	Display_clearScreen();
	Display_setCurrentFont(fnt16x16);
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextColor(BROWN);	//set text color
	Display_outputText("Stopping...");
	sound_ISD4004_StopPwdn();
	SETUPMode_display_soundData();
	PowerControl_sleep(3000);
	return 1;
}

const char* SETUPMode_menu1_stopRecord_onUpdate(void)
{
	return "Stop record\0""Stop record\0""Stop record\0""Стоп записи";
}









//place markers
BOOL SETUPMode_menu1_placeMarkers(void)
{
	Display_clearScreen();
	Display_setCurrentFont(fnt16x16);
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextColor(BROWN);	//set text color
	Display_outputText("Placing markers...");
	sound_placeMarkers();
	SETUPMode_display_soundData();
	PowerControl_sleep(3000);
	return 1;
}

const char* SETUPMode_menu1_placeMarkers_onUpdate(void)
{
	return "Place markers\0""Place markers\0""Place markers\0""Расставить маркеры";
}

*/












//запрос на очистку памяти
BOOL SETUPMode_clear_memory(void)
{
	if(!powerControl.bBatteryAlarm)
	{//only of battery is not discharged
		YESNOMode_DoNotModal(RED, "REQUEST\0""REQUEST\0""REQUEST\0""ЗАПРОС",
			"No memory! Previous file save operation is not performed. Please confirm deletion of autospectra, mcs and log.\0"
			"No memory! Previous file save operation is not performed. Please confirm deletion of autospectra, mcs and log.\0"
			"No memory! Previous file save operation is not performed. Please confirm deletion of autospectra, mcs and log.\0"
		"Нет памяти! Предыдущая операция записи файла не выполнена. Пожалуйста подтвердите удаление автоспектров, мкд и журнала.", SETUPMode_clear_memory_confirm);
	}
	return TRUE;
}
void SETUPMode_clear_memory_confirm(BOOL bYes)
{
	if(bYes)
	{
		int progress = 0;
		//delete all autospz files
		HFILE hfile=NULL;
		do
		{
			hfile = filesystem_open_firstEx("спек_", "spz");
			if(hfile!=NULL)
			{
				if(filesystem_delete_file(hfile)==E_FAIL)
				{
					if(powerControl.bBatteryAlarm)//when battery discharged no file operation is allowed
						break;
				}
				SETUPMode_pleaseWait(++progress);
			}
			PowerControl_kickWatchDog();
		}while(hfile!=NULL);
		do
		{
			hfile = filesystem_open_firstEx("spec_", "spz");
			if(hfile!=NULL)
			{
				if(filesystem_delete_file(hfile)==E_FAIL)
				{
					if(powerControl.bBatteryAlarm)//when battery discharged no file operation is allowed
						break;
				}
				SETUPMode_pleaseWait(++progress);
			}
			PowerControl_kickWatchDog();
		}while(hfile!=NULL);
		do
		{
			hfile = filesystem_open_first("log");
			if(hfile!=NULL)
			{
				if(filesystem_delete_file(hfile)==E_FAIL)
				{
					if(powerControl.bBatteryAlarm)//when battery discharged no file operation is allowed
						break;
				}
				SETUPMode_pleaseWait(++progress);
			}
			PowerControl_kickWatchDog();
		}while(hfile!=NULL);
		do
		{
			hfile = filesystem_open_first("mcs");
			if(hfile!=NULL)
			{
				if(filesystem_delete_file(hfile)==E_FAIL)
				{
					if(powerControl.bBatteryAlarm)//when battery discharged no file operation is allowed
						break;
				}
				SETUPMode_pleaseWait(++progress);

			}
			PowerControl_kickWatchDog();
		}while(hfile!=NULL);
		//recreate log file
		LOGMode_createLog();	
//		LOGMode_insertEventByLang("Power ON\0""Power ON\0""Power ON\0""Включение");
	}
	Modes_clearModeArea();	//clear screen area for mode
	Modes_updateMode();
}










BOOL SETUPMode_menu1_editDTCOEF(void)
{
	EditMode_EditInt("Deadtime coef\0""Deadtime coef\0""Deadtime coef\0""Коэф.мертв.времени",
					 interProcControl.rsModbus.fDTCOEF,
					 0.000001,
					 2.0,
					 "\0""\0""\0""",
					 SETUPMode_menu1_editDTCOEF_edit_done);
	return (BOOL)-1;
}

const char* SETUPMode_menu1_editDTCOEF_onUpdate(void)
{
	return "False alarm period\0""False alarm period\0""False alarm period\0""Период ложн.тревог";
}


void SETUPMode_menu1_editDTCOEF_edit_done(BOOL bOK)
{
	if(bOK)
	{
		interProcControl.rsModbus.fDTCOEF = atoi(EditModeControl.edit_buf);
		InterProc_setDTCEOF(interProcControl.rsModbus.fDTCOEF);
	}		
	SETUPMode_setModeOnSelf();
}
