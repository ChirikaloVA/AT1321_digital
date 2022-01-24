//sprd_mode.c



#include <string.h>
#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <math.h>
#include <stdlib.h>
#include "types.h"
#include "syncObj.h"
#include "spectrum.h"
#include "display.h"
#include "interProc.h"
#include "keyboard.h"
#include "powerControl.h"
#include "sprd_mode.h"
#include "mcs_mode.h"
#include "setup_mode.h"
#include "geiger.h"
#include "sound.h"
#include "eeprom.h"
#include "interrupts.h"
#include "edit.h"
#include "TC_mode.h"
#include "clock.h"
#include "identify.h"
#include "ini_control.h"
#include "LOG_mode.h"
#include "STAB_mode.h"
#include "rid.h"




const char strIdentified[]="IDENTIFICATION...\r\0""IDENTIFICIREN...\r\0""IDENTIFICATION...\r\0""ИДЕНТИФИКАЦИЯ...\r";








struct tagSPRDModeControl SPRDModeControl;


const struct tagMode modes_SPRDMode=
{
	SPRDMode_NameOnUpdate,//"SPRD\0""СПРД",	//mode name
	RGB(0,63,0),	//mode color
	SPRDMode_LeftOnUpdate,//"setup\0""опции",	//right
	SPRDMode_RightOnUpdate,//"rid\0""рид",	//left
	SPRDMode_UpOnUpdate,//"bkg\0""фон",//up
	SPRDMode_DownOnUpdate,//"menu\0""меню",	//down
	SPRDMode_OnActivate,
	SPRDMode_OnLeft,
	SPRDMode_OnRight,
	SPRDMode_OnUp,
	SPRDMode_OnDown,
	SPRDMode_OnIdle,
	SPRDMode_OnShow,
	SPRDMode_OnExit,
	SPRDMode_OnPowerDown,
	SPRDMode_OnWakeUp,
	SPRDMode_OnTimer
};

const char* SPRDMode_NameOnUpdate(void)//"SPRD\0""СПРД",	//mode name
{
	if(SPRDModeControl.bGMMode)
	{
		return "GM mode\0""GM mode\0""GM mode\0""Режим ГМ";
	}else
	if(SPRDModeControl.bNaIMode)
	{
		return "NaI mode\0""NaI mode\0""NaI mode\0""Режим NaI";
	}else
	if(SPRDModeControl.bBkgMode_confirmed)
#ifdef _IAEA
		return "Bgnd\0""Bgnd\0""Bgnd\0""Фон";
#else
		return "SPRD bkg\0""SPRD bkg\0""SPRD bkg\0""СПРД Фон";
#endif
	else
	{
		if(!SPRDModeControl.bIdentMode)
#ifdef _IAEA
			return "Search\0""Search\0""Search\0""Поиск";
#else
			return "SPRD\0""SPRD\0""SPRD\0""СПРД";
#endif
		else
		{
			int val = identifyControl.identifyDeadTime-(UINT)spectrumControl.acqSpectrum.wAcqTime;
			if(val<0)val=0;

			memset((void*)SPRDModeControl.buf, 0, sizeof(SPRDModeControl.buf));
			char buf[20];
			char volatile * pbuf = SPRDModeControl.buf;
			memset(buf, 0, sizeof(buf));
#ifdef _IAEA
			sprintf(buf, "Ident %d", val);
#else
			sprintf(buf, "SPRD %d", val);
#endif
			strcat((char*)pbuf, buf);
			pbuf+=strlen(buf);
			pbuf[0]=0;
			pbuf++;

			memset(buf, 0, sizeof(buf));
#ifdef _IAEA
			sprintf(buf, "Ident %d", val);
#else
			sprintf(buf, "SPRD %d", val);
#endif
			strcat((char*)pbuf, buf);
			pbuf+=strlen(buf);
			pbuf[0]=0;
			pbuf++;

			memset(buf, 0, sizeof(buf));
#ifdef _IAEA
			sprintf(buf, "Ident %d", val);
#else
			sprintf(buf, "SPRD %d", val);
#endif
			strcat((char*)pbuf, buf);
			pbuf+=strlen(buf);
			pbuf[0]=0;
			pbuf++;

			memset(buf, 0, sizeof(buf));
#ifdef _IAEA
			sprintf(buf, "Идент %d", val);
#else
			sprintf(buf, "СПРД %d", val);
#endif
			strcat((char*)pbuf, buf);
			pbuf+=strlen(buf);
			pbuf[0]=0;
			pbuf++;

			return (const char*)SPRDModeControl.buf;
		}
	}
}
const char* SPRDMode_LeftOnUpdate(void)//"setup\0""опции",	//right
{
	switch(soundControl.bSound)
	{
	case SNDST_SOUND:
		return "vibro\0""vibro\0""vibro\0""вибро";
	case SNDST_VIBRO:
		return "silent\0""silent\0""silent\0""тихо";
	default:
		return "sound\0""sound\0""sound\0""звук";
	}
}
const char* SPRDMode_RightOnUpdate(void)//"rid\0""рид",	//left
{
	if(SPRDModeControl.bIdentMode && !SPRDModeControl.bIdentStoped)
	{
		return "extend\rtime\0""extend\rtime\0""extend\rtime\0""расшир\rвремя";
	}else
		return "rid\0""rid\0""rid\0""рид";
}
const char* SPRDMode_UpOnUpdate(void)//"acquir\0""набор",//up
{
	if(SPRDModeControl.bGMMode || SPRDModeControl.bNaIMode)
	{
		return "reset\0""reset\0""reset\0""сброс";
	}

	if(SPRDModeControl.bBkgMode_confirmed)
		return "skip\0""skip\0""skip\0""отмена";
	else
	if(SPRDModeControl.bIdentMode)
	{
		if(SPRDModeControl.bIdentStoped)
			return "back\0""back\0""back\0""назад";
		else
			return "stop\0""stop\0""stop\0""стоп";
	}else
#ifdef _IAEA
		return "bgnd\0""bgnd\0""bgnd\0""фон";
#else
		return "bkg\0""bkg\0""bkg\0""фон";
#endif
}
const char* SPRDMode_DownOnUpdate(void)//"menu\0""меню",	//down
{
	return "menu\0""menu\0""menu\0""меню";
}





const struct tagMenu SPRD_menu=
{
#ifdef _IAEA
	"MENU: SEARCH\0""MENU: SEARCH\0""MENU: SEARCH\0""МЕНЮ: ПОИСК",	//menu name
#else
	"MENU: SPRD\0""MENU: SPRD\0""MENU: SPRD\0""МЕНЮ: СПРД",	//menu name
#endif
	9,	//number of items
	{SPRDMode_menu1_SPRDmode, SPRDMode_menu1_SaveValue, SPRDMode_menu1_BkgMode, SPRDMode_menu1_datetime,SPRDMode_menu1_lang,SPRDMode_menu1_Stabilize, SPRDMode_menu1_NaIMeasure, SPRDMode_menu1_GMMeasure, SPRDMode_menu1_TurnOFF},
	{SPRDMode_menu1_SPRDmode_onUpdate, SPRDMode_menu1_SaveValue_onUpdate, SPRDMode_menu1_BkgMode_onUpdate, SPRDMode_menu1_datetime_onUpdate,SPRDMode_menu1_lang_onUpdate,SPRDMode_menu1_Stabilize_onUpdate, SPRDMode_menu1_NaIMeasure_onUpdate, SPRDMode_menu1_GMMeasure_onUpdate, SPRDMode_menu1_TurnOFF_onUpdate}
};



//выключение прибора из меню
BOOL SPRDMode_menu1_TurnOFF(void)
{
	PowerControl_turnOFF_device("\0""\0""\0""");
	return TRUE;
}

const char* SPRDMode_menu1_TurnOFF_onUpdate(void)
{
	return "Turn OFF\0""Turn OFF\0""Turn OFF\0""Выключить";
}







//save doserate value with gps coords
BOOL SPRDMode_menu1_SaveValue(void)
{
#ifndef GPS_BT_FREE
	LOGMode_insertGPS();
#endif	//#ifndef GPS_BT_FREE
	LOGMode_insertDoserate(SPRD_GetCurrentDoserate());
	SoundControl_BeepSeq(beepSeq_OK);
	return TRUE;
}




const char* SPRDMode_menu1_SaveValue_onUpdate(void)
{
	return "Save values\0""Save values\0""Save values\0""Сохранить значения";
}















//switch to sprd mdoe
BOOL SPRDMode_menu1_SPRDmode(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return TRUE;
}


const char* SPRDMode_menu1_SPRDmode_onUpdate(void)
{
	if(SPRDModeControl.bGMMode || SPRDModeControl.bNaIMode)
#ifdef _IAEA
		return "Search mode\0""Search mode\0""Search mode\0""Режим Поиск";
#else
		return "SPRD mode\0""SPRD mode\0""SPRD mode\0""Режим СПРД";
#endif
	else
		return NULL;
}

//switch to bkg mode
BOOL SPRDMode_menu1_BkgMode(void)
{
	//!!!!!!!!!!!! must be some sound here
	if(SPRDModeControl.bBkgMode_confirmed)
	{//skip
		SPRDModeControl.bBkgMode_confirmed = FALSE;
		SPRDModeControl.bBkgMode_assumed = TRUE;
		InterProc_setSearchMode();
		//read bkg values
		InterProc_getBkgCPS();
	}else
	{//bkg
		SPRDModeControl.bBkgMode_confirmed = TRUE;
		SPRDModeControl.bBkgMode_assumed = FALSE;
		InterProc_setMeasurementMode();
	}
	SPRDMode_Init_for_ident();
	//start new spectrum
	Spectrum_startAcq_ex(MAX_ACQ_TIME);
	InterProc_resetAveraging();
	InterProc_readStatus();
	Modes_updateMode();
	SPRDMode_clearMCS();
	SPRDMode_showWholeMCS_once();
	return TRUE;
}


const char* SPRDMode_menu1_BkgMode_onUpdate(void)
{
	if(!SPRDModeControl.bGMMode && !SPRDModeControl.bNaIMode)
#ifdef _IAEA
		return "Bgnd mode\0""Bgnd mode\0""Bgnd mode\0""Режим Фон";
#else
		return "BKG mode\0""BKG mode\0""BKG mode\0""Режим Фон";
#endif
	else
		return NULL;
}



//switch to stabilization mode
BOOL SPRDMode_menu1_Stabilize(void)
{
	Modes_setActiveMode(&modes_STABMode);
	return TRUE;
}


const char* SPRDMode_menu1_Stabilize_onUpdate(void)
{
	return "Stabilization\0""Stabilization\0""Stabilization\0""Стабилизация";
}




//NaI measurement mode
BOOL SPRDMode_menu1_NaIMeasure(void)
{
	SPRDModeControl.bGMMode = FALSE;
	SPRDModeControl.bNaIMode = TRUE;
	SPRDModeControl.bBkgMode_confirmed = TRUE;
	SPRDModeControl.bBkgMode_assumed = FALSE;
	InterProc_setMeasurementMode();
	Modes_updateMode();
	return TRUE;
}


const char* SPRDMode_menu1_NaIMeasure_onUpdate(void)
{
	if(SPRDModeControl.bNaIMode)
		return NULL;
	else
		return "NaI detector\0""NaI detector\0""NaI detector\0""Детектор NaI";
}



//GM measurement mode
BOOL SPRDMode_menu1_GMMeasure(void)
{
	SPRDModeControl.bGMMode = TRUE;
	SPRDModeControl.bNaIMode = FALSE;
	SPRDModeControl.bBkgMode_confirmed = TRUE;
	SPRDModeControl.bBkgMode_assumed = FALSE;
	InterProc_setMeasurementMode();
	Modes_updateMode();
	return TRUE;
}


const char* SPRDMode_menu1_GMMeasure_onUpdate(void)
{
	if(SPRDModeControl.bGMMode)
		return NULL;
	else
		return "GM counter\0""GM counter\0""GM counter\0""Счетчик ГМ";
}















//clear MCS data only
void SPRDMode_clearMCS(void)
{
	for(int i=0;i<MCS_WIN_WIDTH;i++)
		SPRDModeControl.arMCS[i]=0;
}


void SPRDMode_Init(void)
{
	SPRDMode_clearMCS();
	SPRDModeControl.bBkgMode_confirmed = 1;
	SPRDModeControl.bBkgMode_assumed = 1;
	SPRDModeControl.fMomCps = 0;	//momentary cps
	SPRDModeControl.fCps = 0;	//
	SPRDModeControl.fCpsErr = 200;	//
	SPRDModeControl.fDoserate = 0;	//
	SPRDModeControl.fDoserateErr = 200;	//
	SPRDModeControl.fDose = 0;
	SPRDModeControl.sSigma = 0;
	SPRDModeControl.fDRThreshold = 1;
	SPRDModeControl.bUpdateMCS = FALSE;
	SPRDMode_Init_for_ident();

	SPRDModeControl.iAlarmTimer=0;
	SPRDModeControl.false_alarm_period_oper = 600;
	SPRDModeControl.bNaIOverload = FALSE;
	SPRDModeControl.bMustSwitchToSPRD = FALSE;

	SPRDModeControl.bGMMode = FALSE;
	SPRDModeControl.bNaIMode = FALSE;
}

void SPRDMode_Init_for_ident(void)
{
	memset(SPRDModeControl.spec_name, 0, sizeof(SPRDModeControl.spec_name));
	SPRDModeControl.bRadFound = FALSE;
	SPRDModeControl.bIdentMode = FALSE;
	SPRDModeControl.bCanUpdateIdentResult = FALSE;
	SPRDModeControl.bIdentStoped = FALSE;
	SPRDModeControl.bGMMode = FALSE;
	SPRDModeControl.bNaIMode = FALSE;
	identify_clearReport();
	identifyControl.identifyDeadTime = identifyControl.identifyStartDeadTime;	//current time to stop identify if no nuclides
}


BOOL SPRDMode_OnActivate(void)
{

	Modes_createTimer(333);
	SPRDMode_Init_for_ident();

	Modes_updateMode();
	SPRDMode_showWholeMCS_once();
	return 1;
}

BOOL SPRDMode_OnTimer(void)
{
	int slotscnt = InterProc_countFreeSlots();
	if(slotscnt<3)
	{
		return 1;
	}

	InterProc_readMeasurementRegs();

	if(!SPRDModeControl.bGMMode && !SPRDModeControl.bNaIMode)
	{//only for not gm and nai mode
		BOOL bal = (SPRDModeControl.fDoserate>MAX_NAI_DR || SPRDModeControl.fCps>MAX_NAI_CPS);
		if(SPRDModeControl.bNaIOverload != bal)
		{
			SPRDModeControl.bNaIOverload = bal;
			if(!bal)
			{//need update MCS on screen
				SPRDModeControl.bUpdateMCS = TRUE;
			}
		}

		//goto search mode if fCpsErr < CPS_ERR_THRESHOLD
		if((SPRDModeControl.bBkgMode_assumed && SPRDModeControl.bBkgMode_confirmed) &&
		   SPRDModeControl.fCpsErr<=CPS_ERR_THRESHOLD)
		{
			//switch to search mode
			int slotscnt = InterProc_countFreeSlots();
			if(slotscnt>=9)
			{
				//switch to search mode
				InterProc_makeCpsAsBkg();
				InterProc_makeCpsBaseForSearch();
				modeControl.bMenuON = 0;
				SPRDMode_OnUp();
			}
		}else if(!SPRDModeControl.bBkgMode_assumed && !SPRDModeControl.bBkgMode_confirmed &&
				 modeControl.pMode==&modes_SPRDMode &&
				!SPRDModeControl.bNaIOverload /*no NaI overload*/ &&
					!geigerControl.esentVals_safe.bOverload /*no danger*/)
		{//check for need ident mode
			if(!SPRDMode_checkForIdentMode())return 1;
		}
	}
	SPRDModeControl.bRadFound = FALSE;	//reset alarm flag must be placed after SPRDMode_checkForIdentMode
	Modes_OnShow();
	return 1;
}

//check if must switch to ident mode
//or if we are in ident mode check if we need stop it and save spectrum
//ret TRUE if continue
BOOL SPRDMode_checkForIdentMode(void)
{
	if(SPRDModeControl.bRadFound && !SPRDModeControl.bIdentMode)
	{//first switch to ident mode
#ifndef GPS_BT_FREE
		LOGMode_insertGPS();
#endif	//#ifndef GPS_BT_FREE
		LOGMode_insertEventByLang("Gamma radiation detected!\0""Gamma radiation detected!\0""Gamma radiation detected!\0""Обнаруженно гамма-излучение!");
		LOGMode_insertDoserate(SPRD_GetCurrentDoserate());
		SPRDModeControl.bIdentMode = TRUE;
		//here must start spectrum, but we have to do this earlier
		Spectrum_startAcq_ex(MAX_ACQ_TIME);
		Modes_showButtons();
		identify_clearReport();
		identifyControl.identifyDeadTime = identifyControl.identifyStartDeadTime;	//current time to stop identify if no nuclides
//		InterProc_readAcqTime();
		InterProc_readSpectrumZip();
	}else
	if(SPRDModeControl.bIdentMode)
	{//ident mode, request spectrum, and ident it
		if(InterProc_isDataFinalReady(&interProcControl.rsModbus.sarSpectrumZip))
		{
			identify_identify(TRUE);
			SPRDModeControl.bCanUpdateIdentResult = TRUE;
			if(spectrumControl.acqSpectrum.wAcqTime>=identifyControl.identifyDeadTime)
			{//stop and exit ident
				LOGMode_insertEventByLang(strIdentified);
				LOGMode_insertEvent(identifyControl.report);
				if(SPRDModeControl.bAutoSaveSpectra)
					SPRDMode_saveAutoSpec();
				SPRDModeControl.bIdentStoped = TRUE;	//have to be first to correct exit in OnLeft
				SPRDMode_OnUp();
				return FALSE;
			}
			Modes_showModeName();
		}
		if(!SPRDModeControl.bIdentStoped)
		{//in stop ident mode no need to acq spectrum
//			InterProc_readAcqTime();
			InterProc_readSpectrumZip();
		}
	}
	return TRUE;
}

//save spectrum with auto name
BOOL SPRDMode_saveAutoSpec(void)
{
	char temp[FILE_NAME_SZ];
	if(modeControl.bLang==enu_lang_russian)
		sprintf(SPRDModeControl.spec_name, "спек_%s", Clock_getClockDateTimeStrEx(temp));
	else
		sprintf(SPRDModeControl.spec_name, "spec_%s", Clock_getClockDateTimeStrEx(temp));
	//count realtime
	spectrumControl.acqSpectrum.wRealTime = clockData.dwTotalSecondsFromStart-spectrumControl.acqSpectrum.wRealTime;
	int iret = Spectrum_save(SPRDModeControl.spec_name, TRUE);
	if(iret==0)
	{//nomemory
		SETUPMode_clear_memory();
		return FALSE;
	}else
		return TRUE;
}



BOOL SPRDMode_OnRight(void)
{
	if(SPRDModeControl.bIdentMode && !SPRDModeControl.bIdentStoped)
	{//longer time to id
		identifyControl.identifyDeadTime+=identifyControl.identifyStartDeadTime;
		if(identifyControl.identifyDeadTime>900)
			identifyControl.identifyDeadTime=900;
	}else
	{//goto tc mode
		Modes_setActiveMode(&modes_RID_Mode);
	}
	return 1;
}

BOOL SPRDMode_OnLeft(void)
{
	//switch sound/vibro mode
	SAFE_DECLARE;
	DISABLE_VIC;
	switch(soundControl.bSound)
	{
	case SNDST_SOUND:
		soundControl.bSound = SNDST_VIBRO;
		break;
	case SNDST_VIBRO:
		soundControl.bSound = SNDST_SILENT;
		break;
	default:
		soundControl.bSound = SNDST_SOUND;
	}
	SoundControl_StopVibro();
	SoundControl_StopBeep();
	ENABLE_VIC;
	Modes_showButtons();
	return (BOOL)1;
}

void SPRDMode_PSW_done(BOOL bOK)
{
	if(bOK)
	{

//in debug mode dont need to enter psw
#ifdef DEBUG
		Modes_setActiveMode(&modes_SpectrumMode);
#else
		char pStr[5];
		memset(pStr,0,5);
		SAFE_DECLARE;
		DISABLE_VIC;
		sprintf((char*)pStr, "%02u%02u",
				(UINT)clockData.dateTime.hour, (UINT)clockData.dateTime.minute);
		ENABLE_VIC;

		if(!strcmp(pStr,EditModeControl.edit_buf))
			Modes_setActiveMode(&modes_SpectrumMode);
		else
			Modes_setActiveMode(&modes_SPRDMode);
#endif		//#ifdef DEBUG
	}else
		Modes_setActiveMode(&modes_SPRDMode);
}

BOOL SPRDMode_OnUp(void)
{
	int slotscnt = InterProc_countFreeSlots();
	if(slotscnt<1)
	{
		//can not make command!!!
		SoundControl_BeepSeq(beepSeq_NOK);
		SoundControl_PlayVibro(200);
		return 1;
	}

	if(SPRDModeControl.bGMMode || SPRDModeControl.bNaIMode)
	{//exit from GM or NaI mode
		InterProc_resetAveraging();
		geigerControl.bReset = 1;
		return 1;
	}

	if(slotscnt<7)
	{
		//can not make command!!!
		SoundControl_BeepSeq(beepSeq_NOK);
		SoundControl_PlayVibro(200);
		return 1;
	}


	//background control
	//!!!!!!!!!!!! must be some sound here
	if(SPRDModeControl.bBkgMode_confirmed)
	{//we have background
		SPRDModeControl.bBkgMode_confirmed = FALSE;
		SPRDModeControl.bBkgMode_assumed = TRUE;
		InterProc_setSearchMode();
		//read bkg values
		InterProc_getBkgCPS();
	}else
	{//bkg
		if(SPRDModeControl.bIdentMode)
		{
			if(SPRDModeControl.bIdentStoped)
			{//back
				Modes_setActiveMode(&modes_SPRDMode);
			}else
			{//stop
				LOGMode_insertEventByLang(strIdentified);
				LOGMode_insertEvent(identifyControl.report);
				if(SPRDModeControl.bAutoSaveSpectra)
					SPRDMode_saveAutoSpec();
				SPRDModeControl.bIdentStoped = TRUE;
				Modes_showButtons();
			}
			return 1;	//just return no need to go further
		}else
		{
			SPRDModeControl.bBkgMode_confirmed = TRUE;
			SPRDModeControl.bBkgMode_assumed = FALSE;
			InterProc_setMeasurementMode();
		}
	}
	SPRDMode_Init_for_ident();
	//start new spectrum
	Spectrum_startAcq_ex(MAX_ACQ_TIME);
	InterProc_resetAveraging();
	InterProc_readStatus();
	Modes_updateMode();
	SPRDMode_clearMCS();
	SPRDMode_showWholeMCS_once();
	return 1;
}
BOOL SPRDMode_OnDown(void)
{
	//menu
	Modes_activateMenu(&SPRD_menu);
	SPRDModeControl.bUpdateMCS = TRUE;
	return 1;
}
BOOL SPRDMode_OnIdle(void)
{
	return 1;
}
BOOL SPRDMode_OnShow(void)
{
	SPRDMode_showModeScreen();
	return 1;
}
BOOL SPRDMode_OnExit(void)
{
	if(SPRDModeControl.bIdentMode && !SPRDModeControl.bIdentStoped)
	{
		LOGMode_insertEventByLang(strIdentified);
		LOGMode_insertEvent(identifyControl.report);
	}
	SPRDMode_clearMCS();
	SPRDModeControl.bNaIMode = SPRDModeControl.bGMMode = FALSE;
	return 1;
}
BOOL SPRDMode_OnWakeUp(void)
{
	SPRDModeControl.bUpdateMCS = TRUE;
	return 1;
}
BOOL SPRDMode_OnPowerDown(void)
{
	return (!SPRDModeControl.bIdentMode &&
			!SPRDModeControl.bBkgMode_confirmed);	//allow enter power down if not Id mode and not BKg mode
}


void SPRDMode_showModeHeaders(void)
{
	Display_setTextColor(ORANGE);	//set text color
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(250);
	Display_outputTextByLang("Ѓ DOSE RATE\0""Ѓ DOSE RATE\0""Ѓ DOSE RATE\0""Ѓ МОЩНОСТЬ ДОЗЫ");
	Display_drawHLine(0,Y_SCREEN_MAX-216,X_SCREEN_MAX,RGB(0,32,0));

	Display_setTextColor(ORANGE);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-214,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_clearTextWin(250);
	Display_outputTextByLang("Ѓ COUNTS PER SECOND\0""Ѓ COUNTS PER SECOND\0""Ѓ COUNTS PER SECOND\0""Ѓ СКОРОСТЬ СЧЕТА");
	Display_drawHLine(0,Y_SCREEN_MAX-164,X_SCREEN_MAX,RGB(0,32,0));
}

void SPRDMode_showModeScreen(void)
{
	SPRDMode_showModeHeaders();

	SPRDMode_showDR();
	SPRDMode_showCps();
	if(geigerControl.esentVals_safe.bOverload)
	{//overload
		SPRDMode_showGMOverload();
	}else
	if(geigerControl.esentVals_safe.bSafetyAlarm)
	{//safety alarm
		SPRDMode_showAlarm();
	}else if(SPRDModeControl.bNaIOverload)
	{
		SPRDMode_showNaIAlarm();
	}else if(SPRDModeControl.bIdentMode && identifyControl.bHaveAlreadyResult)
	{//ident sub mode
		if(SPRDModeControl.bCanUpdateIdentResult)
		{
			SPRDMode_showIdent();
			SPRDModeControl.bCanUpdateIdentResult = FALSE;
		}
	}else
	{//normal mode
		if(SPRDModeControl.bUpdateMCS)
		{//однократное обновление диаграммы после отображения служеюный сообщений в ее зоне
			SPRDModeControl.bUpdateMCS = FALSE;
			SPRDMode_showWholeMCS_once();
		}else
		{
			SPRDMode_showMCS();
//			if(modeControl.pMode==&modes_MCSMode &&
//			   !SPRDModeControl.bBkgMode_assumed && !SPRDModeControl.bBkgMode_confirmed)
//			{//для режима МКД показываем истекшее время измерения
//				SPRDMode_showTime();
//			}
		}
	}
}



//
//void SPRDMode_showTime(void)
//{
//	Display_setTextWin(0,MCS_WIN_BOTTOM-MCS_WIN_HEIGHT,X_SCREEN_SIZE,16);	//set text window
//	Display_setTextXY(0,0);	//set start coords in window
//	Display_setTextWrap(0);
//	Display_setTextSteps(1,1);
//	Display_setTextDoubleHeight(0);
//	Display_setTextJustify(NONE);
//	Display_setCurrentFont(fnt16x16);
//	Display_setTextColor(BROWN);	//set text color
//	char buf[10];
//	sprintf(buf,"%u",(UINT)MCSModeControl.iNumberOfCpsTotal/3);
//	Display_clearTextWin(10);
//	Display_outputTextByLang("Elapsed time: \0""Elapsed time: \0""Elapsed time: \0""Истекш. время: ");
//	Display_outputText(buf);
//}



void SPRDMode_showAlarm(void)
{
	Display_setTextWin(0,MCS_WIN_BOTTOM-MCS_WIN_HEIGHT,X_SCREEN_SIZE,MCS_WIN_HEIGHT);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(2,2);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt16x16);
	COLORREF clr;
	clr = (clockData.dateTime.second&0x01)?RED:YELLOW;
	Display_setTextColor(clr);	//set text color
	Display_setTextLineClear(1);
	Display_checkForClearLine();
	Display_outputTextByLang("DANGER RADIATION LEVEL!\rGO BACK!\0""DANGER RADIATION LEVEL!\rGO BACK!\0""DANGER RADIATION LEVEL!\rGO BACK!\0""ОПАСНЫЙ УРОВЕНЬ РАДИАЦИИ!\rОТОЙДИТЕ НАЗАД!");
	int sz = Display_getFontSizeY()*(display.text.bDoubleHeight?2:1)+display.text.stepY;
	Display_clearUserPart();
	Display_setTextLineClear(0);
}

void SPRDMode_showGMOverload(void)
{
	Display_setTextWin(0,MCS_WIN_BOTTOM-MCS_WIN_HEIGHT,X_SCREEN_SIZE,MCS_WIN_HEIGHT);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(2,2);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt16x16);
	COLORREF clr;
	clr = (clockData.dateTime.second&0x01)?RED:YELLOW;
	Display_setTextColor(clr);	//set text color
	Display_setTextLineClear(1);
	Display_checkForClearLine();
	Display_outputTextByLang("OVERLOAD. DANGER!\rGO BACK!\0""OVERLOAD. DANGER!\rGO BACK!\0""OVERLOAD. DANGER!\rGO BACK!\0""ПЕРЕГРУЗКА. Опасно!\rОтойдите назад!");
	int sz = Display_getFontSizeY()*(display.text.bDoubleHeight?2:1)+display.text.stepY;
	Display_clearUserPart();
	Display_setTextLineClear(0);
}



void SPRDMode_showNaIAlarm(void)
{
	Display_setTextWin(0,MCS_WIN_BOTTOM-MCS_WIN_HEIGHT,X_SCREEN_SIZE,MCS_WIN_HEIGHT);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(2,2);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt16x16);
	COLORREF clr;
	clr = (clockData.dateTime.second&0x01)?RED:YELLOW;
	Display_setTextColor(clr);	//set text color
	Display_setTextLineClear(1);
	Display_checkForClearLine();
	Display_outputTextByLang("HIGH RADIATION.\rGO BACK.\0""HIGH RADIATION.\rGO BACK.\0""HIGH RADIATION.\rGO BACK.\0""ВЫСОКИЙ УРОВЕНЬ РАДИАЦИИ.\rОТОЙДИТЕ НАЗАД.");
	int sz = Display_getFontSizeY()*(display.text.bDoubleHeight?2:1)+display.text.stepY;
	Display_clearUserPart();
	Display_setTextLineClear(0);
}







void SPRDMode_showIdent(void)
{
	Display_setTextWin(0,MCS_WIN_BOTTOM-MCS_WIN_HEIGHT,X_SCREEN_SIZE,MCS_WIN_HEIGHT);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,2);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt16x16);
	Display_setTextColor(GREEN);	//set text color
	Display_setTextLineClear(1);
	Display_checkForClearLine();

	Display_outputTextByLang("Nuc.Lib: \0""Nuc.Lib: \0""Nuc.Lib: \0""Нук.биб: ");
	Display_outputText(identifyControl.libraryFileName);
	Display_outputText("\r");

	Display_checkForClearLine();
	Display_setTextColor(RED);	//set text color
	Display_outputTextByLang(strIdentified);
	Display_setTextColor(YELLOW);	//set text color
	Display_outputText(identifyControl.report);
	Display_clearUserPart();
	Display_setTextLineClear(0);
}


//отобразить диаграмму сканирования
void SPRDMode_showMCS(void)
{
	if(SPRDModeControl.bBkgMode_confirmed)return;

	Display_left_scroll(1,MCS_WIN_BOTTOM-MCS_WIN_HEIGHT2,MCS_WIN_WIDTH-1,MCS_WIN_BOTTOM, 1);

	COLORREF clr;

	int i = MCS_WIN_WIDTH-1;

	int y = (int)SPRDModeControl.arMCS[i];
	y=3*y;
	if(y>MCS_WIN_MIDLLE_U-1)
		y=MCS_WIN_MIDLLE_U-1;
	else if(y<-MCS_WIN_MIDLLE_D+1)
		y=-MCS_WIN_MIDLLE_D+1;
	int yy = (DWORD)MCS_WIN_BOTTOM-MCS_WIN_MIDLLE_D-y;
	if(y>=0)
		clr = RED;
	else if(y<0)
		clr = GREEN;
	Display_drawVLine(i,MCS_WIN_BOTTOM-MCS_WIN_HEIGHT2+1,MCS_WIN_BOTTOM, BLACK);
	Display_drawVLine(i,MCS_WIN_BOTTOM-MCS_WIN_MIDLLE_D,yy, clr);
}

void SPRDMode_showWholeMCS_once(void)
{

	RECT rect={0,MCS_WIN_TOP,MCS_WIN_WIDTH-1,MCS_WIN_BOTTOM};
	Display_clearRect(rect,50);

	if(SPRDModeControl.bBkgMode_confirmed)return;

	COLORREF clr;
	for(int i=0;i<MCS_WIN_WIDTH;i++)
	{
		int y = (int)SPRDModeControl.arMCS[i];
		y=3*y;
		if(y>MCS_WIN_MIDLLE_U-1)
			y=MCS_WIN_MIDLLE_U-1;
		else if(y<-MCS_WIN_MIDLLE_D+1)
			y=-MCS_WIN_MIDLLE_D+1;
		int yy = (DWORD)MCS_WIN_BOTTOM-MCS_WIN_MIDLLE_D-y;
		if(y>=0)
			clr = RED;
		else if(y<0)
			clr = GREEN;
		Display_drawVLine(i,MCS_WIN_BOTTOM-MCS_WIN_MIDLLE_D,yy, clr);
	}
}






void SPRDMode_showCps(void)
{
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-197,X_SCREEN_SIZE,32);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt32x32);

	char buf[100];
	float cps =SPRDModeControl.fCps;
	float cpserr =SPRDModeControl.fCpsErr;

	if(SPRDModeControl.bNaIOverload || SPRDModeControl.bGMMode)
	{//to show geiger doserate value if overrun or GM mode
		cps = geigerControl.esentVals_safe.fCps;
		cpserr = geigerControl.esentVals_safe.fCpsErr;
	}


	const char* pFltMsk;
	if(cps<1)
		pFltMsk = "%.3f";
	else if(cps<10)
		pFltMsk = "%.2f";
	else if(cps<100)
		pFltMsk = "%.1f";
	else
		pFltMsk = "%.0f";

	sprintf(buf,pFltMsk,cps);
//	Display_clearTextWin(200);
	Display_outputText(buf);
	Display_setCurrentFont(fnt16x16);
	//output %
	Display_setTextXY(160,0);
	UINT erval = (UINT)(cpserr+0.5);
	if(erval<1)
		erval = 1;
	sprintf(buf,"%u%%",erval);
	Display_outputText(buf);
	//output dimension
	Display_setTextXY(160,16);	//set start coords in window
	Display_outputText("cps");
}

//take current doserate value depends on NaI or GM now is ON
float SPRD_GetCurrentDoserate(void)
{
	//check for overload
	float val = SPRDModeControl.fDoserate;
	if(SPRDModeControl.bNaIOverload || SPRDModeControl.bGMMode)
	{//to show geiger doserate value if overrun or GM mode
		val = geigerControl.esentVals_safe.fDoserate;
	}
	return val;
}

float SPRD_GetCurrentDoserateErr(void)
{
	//check for overload
	float valerr = SPRDModeControl.fDoserateErr;
	if(SPRDModeControl.bNaIOverload || SPRDModeControl.bGMMode)
	{//to show geiger doserate value if overrun or GM mode
		valerr = geigerControl.esentVals_safe.fCpsErr;
	}
	return valerr;
}


void SPRDMode_showDR(void)
{
  SET_ISD_INT;
	COLORREF clr;
	if(!geigerControl.esentVals_safe.bSafetyAlarm &&
	   !geigerControl.esentVals_safe.bOverload)
		clr = YELLOW;
	else
		clr = (clockData.dateTime.second&0x01)?RED:YELLOW;
	Display_setTextColor(clr);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-249,X_SCREEN_SIZE,32);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt32x32);

	//check for overload
	float val = SPRD_GetCurrentDoserate();
	float valerr = SPRD_GetCurrentDoserateErr();

	if(geigerControl.esentVals_safe.bOverload)
	{//OVERLOAD MODE
		val = MAX_DR;
		valerr = 1;
		clr = (clockData.dateTime.second&0x01)?RED:YELLOW;
		Display_setTextColor(clr);	//set text color
	}

//	{//normal mode
	char buf[100];
	const char* pFltMsk;
	if(val<1)
		pFltMsk = "%.3f";
	else if(val<10)
		pFltMsk = "%.2f";
	else if(val<100)
		pFltMsk = "%.1f";
	else
		pFltMsk = "%.0f";

	sprintf(buf,pFltMsk,val);
	Display_clearTextWin(250);
	Display_outputText(buf);
	Display_setCurrentFont(fnt16x16);
	//output %
	Display_setTextXY(160,0);
	UINT erval = (UINT)(valerr+0.5);
	if(erval<1)
		erval = 1;
	sprintf(buf,"%u%%",erval);
	Display_outputText(buf);
	//output dimension
	Display_setTextXY(160,16);	//set start coords in window
	Display_outputTextByLang(SPRDMode_getDimension());
CLR_ISD_INT;
}


//get dimension
char* SPRDMode_getDimension(void)
{
#ifdef BNC
			return "mrem/h\0""mrem/h\0""mrem/h\0""mrem/h";
#else
			return "µSv/h\0""µSv/h\0""µSv/h\0""µSv/h";
#endif
}

char* SPRDMode_getDimensionDose(void)
{
#ifdef BNC
			return "mrem\0""mrem\0""mrem\0""mrem";
#else
			return "µSv\0""µSv\0""µSv\0""µSv";
#endif
}











//////////////////////////////////////////////////////////////////////////////////
//date time edit




//edit date time
BOOL SPRDMode_menu1_datetime(void)
{
	SPRDModeControl.year = clockData.dateTime.year;
	EditMode_EditInt_ex("Enter year\0""Enter year\0""Enter year\0""Введите год",
					 SPRDModeControl.year,
					 2000,
					 3000,
					 "year\0""year\0""year\0""год",
					 SPRDMode_menu1_datetime_year_done,
					 TRUE);
	return (BOOL)-1;
}

void SPRDMode_menu1_datetime_year_done(BOOL bOK)
{
	if(!bOK)
	{
		Modes_setActiveMode(&modes_SPRDMode);
		return;
	}

	SPRDModeControl.year = atoi(EditModeControl.edit_buf);
	SPRDModeControl.month = clockData.dateTime.month;
	EditMode_EditInt_ex("Enter month\0""Enter month\0""Enter month\0""Введите месяц",
					 SPRDModeControl.month,
					 1,
					 12,
					 "month\0""month\0""month\0""месяц",
					 SPRDMode_menu1_datetime_month_done,
					 TRUE);
}


void SPRDMode_menu1_datetime_month_done(BOOL bOK)
{
	if(!bOK)
	{
		Modes_setActiveMode(&modes_SPRDMode);
		return;
	}
	SPRDModeControl.month = atoi(EditModeControl.edit_buf);
	SPRDModeControl.daym = clockData.dateTime.dayOfMonth;
	EditMode_EditInt_ex("Enter day of month\0""Enter day of month\0""Enter day of month\0""Введите день месяца",
					 SPRDModeControl.daym,
					 1,
					 (SPRDModeControl.month!=2?((SPRDModeControl.month%2)^(SPRDModeControl.month>7))+30:(!(SPRDModeControl.year%400)||!(SPRDModeControl.year%4)&&(SPRDModeControl.year%25)?29:28)),
					 "day\0""day\0""day\0""день",
					 SPRDMode_menu1_datetime_daym_done,
					 TRUE);
}

void SPRDMode_menu1_datetime_daym_done(BOOL bOK)
{
	if(!bOK)
	{
		Modes_setActiveMode(&modes_SPRDMode);
		return;
	}
	SPRDModeControl.daym = atoi(EditModeControl.edit_buf);
	SPRDModeControl.dayw = clockData.dayOfWeek;
	EditMode_EditInt_ex("Enter day of week (0-sunday)\0""Enter day of week  (0-sunday)\0""Enter day of week  (0-sunday)\0""Введите день недели  (0-воскресение)",
					 SPRDModeControl.dayw,
					 0,
					 6,
					 "day\0""day\0""day\0""день",
					 SPRDMode_menu1_datetime_dayw_done,
					 TRUE);
}

void SPRDMode_menu1_datetime_dayw_done(BOOL bOK)
{
	if(!bOK)
	{
		Modes_setActiveMode(&modes_SPRDMode);
		return;
	}
	SPRDModeControl.dayw = atoi(EditModeControl.edit_buf);
	//compute day of year		я	ф	м	а	м	и	и	а	с	о	н	д
	const int daysInMonth[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	SPRDModeControl.dayy = 0;
	for(int i=1;i<SPRDModeControl.month;i++)
		SPRDModeControl.dayy += daysInMonth[i];
	SPRDModeControl.dayy += SPRDModeControl.daym;
	//add day if it is leap year
	if(SPRDModeControl.month>2 && !(SPRDModeControl.year&0x3))
	{
		if((SPRDModeControl.year%100)!=0 || (SPRDModeControl.year%400)==0)
			SPRDModeControl.dayy++;
	}
	SPRDModeControl.hour = clockData.dateTime.hour;
	EditMode_EditInt_ex("Enter hour\0""Enter hour\0""Enter hour\0""Введите час",
					 SPRDModeControl.hour,
					 0,
					 23,
					 "hour\0""hour\0""hour\0""час",
					 SPRDMode_menu1_datetime_hour_done,
					 TRUE);
}

void SPRDMode_menu1_datetime_hour_done(BOOL bOK)
{
	if(!bOK)
	{
		Modes_setActiveMode(&modes_SPRDMode);
		return;
	}
	SPRDModeControl.hour = atoi(EditModeControl.edit_buf);
	SPRDModeControl.minute = clockData.dateTime.minute;
	EditMode_EditInt_ex("Enter minute\0""Enter minute\0""Enter minute\0""Введите минуты",
					 SPRDModeControl.minute,
					 0,
					 59,
					 "minute\0""minute\0""minute\0""минута",
					 SPRDMode_menu1_datetime_minute_done,
					 TRUE);
}

void SPRDMode_menu1_datetime_minute_done(BOOL bOK)
{
	if(bOK)
	{
		SPRDModeControl.minute = atoi(EditModeControl.edit_buf);
		Clock_setDateTime(30,SPRDModeControl.minute,SPRDModeControl.hour,
		  SPRDModeControl.daym, SPRDModeControl.dayw,  SPRDModeControl.dayy,
		  SPRDModeControl.month,SPRDModeControl.year);
	}
	Modes_setActiveMode(&modes_SPRDMode);
}


const char* SPRDMode_menu1_datetime_onUpdate(void)
{
	return "Adjust clock\0""Adjust clock\0""Adjust clock\0""Настроить часы";
}





BOOL SPRDMode_menu1_lang(void)
{
	if(++modeControl.bLang>=MAX_ENU_LANG)
		modeControl.bLang = enu_lang_english;

	if(!ini_write_system_ini_int("modeControl", "bLang", modeControl.bLang))
	{
		;//!!!!!!!!!error
	}
	return 0;//update menu item
}

const char* SPRDMode_menu1_lang_onUpdate(void)
{
	char * pret;
	switch(modeControl.bLang)
	{
	case enu_lang_russian:
		pret = "Русский\0""Русский\0""Русский\0""Русский";
		break;
	case enu_lang_german:
		pret = "Deutsch\0""Deutsch\0""Deutsch\0""Deutsch";
		break;
	case enu_lang_french:
		pret = "French\0""French\0""French\0""French";
		break;
	default:
		pret = "English\0""English\0""English\0""English";
	};
	return pret;
}


