//sprd_mode.c
/*
режим предназначен для поиска источников гамма-излучения путем анализа скорости счета гамма-излучения над фоном, звуковой и световой сигнализации
и вывода диаграммы среднеквадратичных отклонений скорости счета гамма-излучения над фоном.
при обнаружении не происходит автоматического переключения в режим идентификации.
при накоплении от 30 и более данных скоросчетй счета гамма-излучения создается временный файл скоростей счета который будет накапливаться
до выхода из режима MCS.
по кнопке ВНИЗ происходит принудительное сохранение спектра гамма-излучения который накапливался обновременно с данными скоростей счета, а также
временный файл будет переименован в постоянный файл скоростей счета.
*/

#include <string.h>
#include <stdio.h>
#include <iolpc2388.h>

#include "types.h"
#include "display.h"
#include "interProc.h"
#include "gps_mode.h"
#include "mcs_mode.h"
#include "sprd_mode.h"
#include "clock.h"
#include "sound.h"
#include "NMEA_Parser.h"
#include "LOG_mode.h"
#include "SETUP_Mode.h"
#include "spectrum.h"

struct tagMCSModeControl MCSModeControl;


const struct tagMode modes_MCSMode=
{
	MCSMode_NameOnUpdate,//"MCS\0""ДИАГРАММА",	//mode name
	RGB(0,63,0),	//mode color
	MCSMode_LeftOnUpdate,//"back\0""назад",	//left
	MCSMode_RightOnUpdate,//"setup\0""опции",	//right
	SPRDMode_UpOnUpdate,//"save\0""сохран",//up
	MCSMode_DownOnUpdate,//"menu\0""меню",	//down
	MCSMode_OnActivate,
	MCSMode_OnLeft,
	MCSMode_OnRight,
	MCSMode_OnUp,
	MCSMode_OnDown,
	SPRDMode_OnIdle,
	SPRDMode_OnShow,
	MCSMode_OnExit,
	MCSMode_OnPowerDown,
	SPRDMode_OnWakeUp,
	MCSMode_OnTimer
};




const char* MCSMode_NameOnUpdate(void)//"MCS\0""ДИАГРАММА",	//mode name
{
	if(SPRDModeControl.bBkgMode_confirmed)
#ifdef _IAEA
		return "Bgnd\0""Bgnd\0""Bgnd\0""Фон";
	else
		return "Measure\0""Measure\0""Measure\0""Измер";
#else
		return "MCS BG\0""MCS BG\0""MCS BG\0""МКД фон";
	else
		return "MCS\0""MCS\0""MCS\0""МКД";
#endif
}

const char* MCSMode_LeftOnUpdate(void)//"back\0""назад",	//left
{
	return "back\0""wieder\0""back\0""назад";
}
const char* MCSMode_RightOnUpdate(void)//"setup\0""опции",	//right
{
#ifndef GPS_BT_FREE
	return "gps\0""gps\0""gps\0""gps";
#else //#ifndef GPS_BT_FREE
	return "setup\0""setup\0""setup\0""опции";
#endif	//#ifndef GPS_BT_FREE
}
const char* MCSMode_DownOnUpdate(void)//"menu\0""меню",	//down
{
	return "menu\0""menu\0""menu\0""меню";
}
const struct tagMenu MCS_menu=
{
	"MENU: MCS\0""MENU: MCS\0""MENU: MCS\0""МЕНЮ: МКД",	//menu name
	2,
	{MCSMode_menu1_SaveValue,
	MCSMode_menu1_SaveRowValues,
//	MCSMode_menu1_SUBmode,
	},
	{SPRDMode_menu1_SaveValue_onUpdate,
	MCSMode_menu1_SaveRowValues_onUpdate,
//	MCSMode_menu1_SUBmode_onUpdate,
	}
};
//save MCS and SPECTRUM
void MCSMode_saveValue(void)
{
	Spectrum_silent_startAcq_ex_start(10);
	if(!MCSModeControl.hfile_tmp)
		MCSMode_createTempFile();
	if(MCSMode_saveArSpec())
		MCSMode_saveMCS();//save mcs with manual saved spectrum
	//count realtime
	Spectrum_silent_startAcq_ex_end();
	MCSMode_prepare();
}
//save MCS and SPECTRUM once from menu
BOOL MCSMode_menu1_SaveValue(void)
{
	MCSMode_saveValue();
	SPRDMode_menu1_SaveValue();
	return 1;
}
////switch to sprd mdoe
//BOOL MCSMode_menu1_SUBmode(void)
//{
//	if(!SPRDModeControl.bGMMode && !SPRDModeControl.bNaIMode)
//	{
//		SPRDModeControl.bGMMode = FALSE;
//		SPRDModeControl.bNaIMode = TRUE;
////		SPRDModeControl.bBkgMode = TRUE;
//		//COMMRS_setMeasurementMode();
//		Modes_updateMode();
//	}else if(SPRDModeControl.bNaIMode)
//	{
//		SPRDModeControl.bGMMode = TRUE;
//		SPRDModeControl.bNaIMode = FALSE;
////		SPRDModeControl.bBkgMode = TRUE;
//		//COMMRS_setMeasurementMode();
//		Modes_updateMode();
//	}else if(SPRDModeControl.bGMMode)
//	{
//		SPRDModeControl.bGMMode = FALSE;
//		SPRDModeControl.bNaIMode = FALSE;
//		Modes_updateMode();
////		Modes_setActiveMode(&modes_SPRDMode);
//	}
//	return TRUE;
//}
//const char* MCSMode_menu1_SUBmode_onUpdate(void)
//{
//	if(!SPRDModeControl.bGMMode && !SPRDModeControl.bNaIMode)
//	   return "NaI mode\0""NaI mode\0""NaI mode\0""Режим NaI";
//	else if(SPRDModeControl.bNaIMode)
//		return "GM mode\0""GM mode\0""GM mode\0""Режим ГМ";
//	else
//		return "MEASURE mode\0""MEASURE mode\0""MEASURE mode\0""Режим ИЗМЕР";
//}
//

BOOL MCSMode_OnPowerDown(void)
{
	return 0;	//not allowed
}



BOOL MCSMode_OnExit(void)
{
	if(MCSModeControl.bSavingON)
	{
		MCSMode_saveValue();

	}
	return SPRDMode_OnExit();
}

void MCSMode_Init(void)
{
	memset(&MCSModeControl,0,sizeof(MCSModeControl));
	SPRDMode_Init();
	MCSMode_prepare();
	Spectrum_startAcq_ex(10);
	MCSModeControl.bSavingON = FALSE;
	MCSModeControl.hfile_tmp = NULL;
}


BOOL MCSMode_OnActivate(void)
{
	SPRDMode_OnActivate();
	MCSModeControl.bSavingON = FALSE;
	MCSModeControl.hfile_tmp = NULL;
	MCSMode_prepare();
	Spectrum_startAcq_ex(10);
	InterProc_resetSyncData(&interProcControl.rsModbus.sarSpectrumZip);
	return 1;
}


void MCSMode_prepare(void)
{
	memset(&MCSModeControl.mcs[0], 0, sizeof(MCSModeControl.mcs));


	MCSModeControl.iNumberOfCps = 0;
//	MCSModeControl.iNumberOfCpsTotal = 0;
	//copy date time gps for next data
	memcpy((void*)&MCSModeControl.dateTime, (const void*)&clockData.dateTime, sizeof(clockData.dateTime));
	memcpy((void*)&MCSModeControl.commonGPS, (const void*)&NMEAParserControl.commonGPS, sizeof(NMEAParserControl.commonGPS));
	memset(&SPRDModeControl.spec_name[0], 0, sizeof(SPRDModeControl.spec_name));
}

/*
structure of mcs file
DWORD 0xeeeeeeee - marker of file begin
DWORD 0xffffffff - marker of header structure
struct tagDateTime
{
  WORD year;
  BYTE month;
  BYTE dayOfMonth;
  BYTE hour;
  BYTE minute;
  BYTE second;
};
struct tagCommonGPS
{
	double Lat;
	double Lon;
	float Alt;
	float Spd;
	float Dir;
	unsigned char Vld;
	unsigned char Sat;
};
char spec_name[FILE_NAME_SZ];	//name of auto saved spectrum
DWORD cps;	!=0xffffffff, !=0xeeeeeeee
*/

/*
generate MCS file name and spectrum array file name
*/
void MCSMode_createTempFile(void)
{
unsigned char temp[FILE_NAME_SZ];
unsigned char autoname[FILE_NAME_SZ];
	sprintf((char*)autoname, "mcs_%s", Clock_getClockDateTimeStrEx(temp));
	MCSModeControl.hfile_tmp = filesystem_create_file(autoname,"mc2", 1);
	if(MCSModeControl.hfile_tmp)
	{
		int pos = 0;
		DWORD val = 0xeeeeeeee;
		int rlen = filesystem_file_put(MCSModeControl.hfile_tmp, &pos,
									   (BYTE*)&val,
									   sizeof(val));
		if(rlen==E_FAIL)
		{//failed to create/save data in a file
			//delete it
			filesystem_delete_file(MCSModeControl.hfile_tmp);
			MCSModeControl.hfile_tmp = NULL;
		}
	}
	if(modeControl.bLang==enu_lang_russian)
		sprintf((char*)MCSModeControl.specar_name, "спар_%s", temp);
	else
		sprintf((char*)MCSModeControl.specar_name, "spar_%s", temp);
}


BOOL MCSMode_OnTimer(void)
{
	InterProc_readMeasurementRegs();

	if(clockData.dateTime.second!=MCSModeControl.savedSeconds)
	{
		MCSModeControl.savedSeconds = clockData.dateTime.second;
		InterProc_readSpectrumZip();	//every 2 seconds
	}

	//goto search mode if fCpsErr < CPS_ERR_THRESHOLD
	if((SPRDModeControl.bBkgMode_assumed && SPRDModeControl.bBkgMode_confirmed) &&
	   SPRDModeControl.fCpsErr<=CPS_ERR_THRESHOLD)
	{
		//switch to search mode
		int slotscnt = InterProc_countFreeSlots();
		if(slotscnt>=9)
		{
			InterProc_makeCpsAsBkg();
			InterProc_makeCpsBaseForSearch();
			SPRDMode_OnUp();
		}
	}else if(!SPRDModeControl.bBkgMode_assumed && !SPRDModeControl.bBkgMode_confirmed)
	{//save cps in a file

			//collect before overload array
			MCSModeControl.mcs[MCSModeControl.iNumberOfCps].dwMomCPS =
				(UINT)SPRDModeControl.fMomCps | (SPRDModeControl.bRadFound?0x80000000:0x0);
			MCSModeControl.mcs[MCSModeControl.iNumberOfCps].fDR = SPRD_GetCurrentDoserate();
			MCSModeControl.mcs[MCSModeControl.iNumberOfCps].btError = (BYTE)(SPRDModeControl.fDoserateErr>200?200:SPRDModeControl.fDoserateErr);
			MCSModeControl.mcs[MCSModeControl.iNumberOfCps].fNCPS = 0;
			MCSModeControl.iNumberOfCps++;
			if(MCSModeControl.iNumberOfCps==MAX_MCS)
				MCSMode_saveMCS();//must save in a temp file
//			MCSModeControl.iNumberOfCpsTotal++;
	}

/*	if(MCSModeControl.bSaveSpectrum &&
	   InterProc_isDataFinalReady(&interProcControl.rsModbus.sarSpectrum) && MCSModeControl.iNumberOfCps>0)
	{
		MCSModeControl.bSaveSpectrum = FALSE;
		if(SPRDMode_saveAutoSpec())
		{
			MCSMode_saveMCS();//save mcs with manual saved spectrum
		}
	}*/

	if(MCSModeControl.bSavingON &&
	   spectrumControl.acqSpectrum.wAcqTime >= 5)
	{
		MCSMode_saveValue();
		//!!!!!!!!!!!!!!!!
		Display_BlinkREDLED(500);
		//!!!!!!!!!!!!!!!
	}
	SPRDModeControl.bRadFound = FALSE;	//reset alarm flag

	Modes_OnShow();
	return 1;
}


//save spectrum to array
BOOL MCSMode_saveArSpec(void)
{
	spectrumControl.acqSpectrum.wRealTime = clockData.dwTotalSecondsFromStart-spectrumControl.acqSpectrum.wRealTime;
	int iret = Spectrum_saveSpecAr(MCSModeControl.specar_name);
	if(iret==0)
	{//nomemory
		MCSModeControl.bSavingON = FALSE;
		SETUPMode_clear_memory();
		return FALSE;
	}
	return TRUE;
}
//save doserate value with gps coords
BOOL MCSMode_menu1_SaveRowValues(void)
{
	//	MCSModeControl.bSaveSpectrum = TRUE;
	MCSModeControl.bSavingON = !MCSModeControl.bSavingON;
	MCSMode_prepare();
	Spectrum_startAcq_ex(10);
	return FALSE;	//just redraw menu
}
const char* MCSMode_menu1_SaveRowValues_onUpdate(void)
{
	if(MCSModeControl.bSavingON)
		return "Autosaving: ON\0""Autosaving: ON\0""Autosaving: ON\0""Автосохран.: ВКЛ";
	else
		return "Autosaving: OFF\0""Autosaving: OFF\0""Autosaving: OFF\0""Автосохран.: ВЫКЛ";
}


//auto save mcs in temp file
void MCSMode_saveMCS(void)
{
	if(MCSModeControl.iNumberOfCps<=0)
	{
		MCSMode_prepare();
		return;	//failed
	}
	if(!MCSModeControl.hfile_tmp)
		MCSMode_createTempFile();

	if(!MCSModeControl.hfile_tmp)
	{//error no file, or no memory
		MCSModeControl.iNumberOfCps = 0;
		SETUPMode_clear_memory();
		return;
	}
	//pre-first DWORD =0xffffffff is a marker of datetime and other structure
	//first structure is a date time
	//second structure is gps data
	//third array of 16 chars is a string of file name
	//fourth array of unknown DWORDs count is a cps array in DWORD
	//summary is a 61 bytes of header
	int pos = filesystem_get_length(MCSModeControl.hfile_tmp);
	BYTE buf[64];
	int len = 0;
	BYTE* pBuf = buf;
	DWORD dw = 0xffffffff;
	memcpy(pBuf, &dw, sizeof(dw));
	pBuf+=sizeof(dw);
	len+=sizeof(dw);
	memcpy(pBuf, (void*)&clockData.dateTime, sizeof(clockData.dateTime));
	pBuf+=sizeof(clockData.dateTime);
	len+=sizeof(clockData.dateTime);
	memcpy(pBuf, &NMEAParserControl.commonGPS, sizeof(NMEAParserControl.commonGPS));
	pBuf+=sizeof(NMEAParserControl.commonGPS);
	len+=sizeof(NMEAParserControl.commonGPS);
	memcpy(pBuf, &SPRDModeControl.spec_name, sizeof(SPRDModeControl.spec_name));
	pBuf+=sizeof(SPRDModeControl.spec_name);
	len+=sizeof(SPRDModeControl.spec_name);
	int rlen = filesystem_file_put(MCSModeControl.hfile_tmp, &pos,
								   (BYTE*)&buf,
								   len);
	if(rlen>0)
	{
		rlen = filesystem_file_put(MCSModeControl.hfile_tmp, &pos,
								   (BYTE*)&MCSModeControl.mcs[0],
								   MCSModeControl.iNumberOfCps*sizeof(MCSModeControl.mcs[0]));
	}
	//clear spectrum name to be sure of wrong info
	MCSMode_prepare();

	if(rlen==E_FAIL)
	{//nomemory
		SETUPMode_clear_memory();
	}


}


BOOL MCSMode_OnLeft(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return 1;
}
BOOL MCSMode_OnRight(void)
{
#ifndef GPS_BT_FREE
	GPSMode_setActiveMode();
#else	//#ifndef GPS_BT_FREE
	Modes_setActiveMode(&modes_SETUPMode);
#endif	//#ifndef GPS_BT_FREE
	return 1;
}

//сохран диаграммы и спектра
BOOL MCSMode_OnDown(void)
{
	Modes_activateMenu(&MCS_menu);
	SPRDModeControl.bUpdateMCS = TRUE;
	SPRDModeControl.bCanUpdateIdentResult = TRUE;	//чтобы данные обновились на экране

	return 1;
}


BOOL MCSMode_OnUp(void)
{
	MCSMode_prepare();
	Spectrum_startAcq_ex(10);
	return SPRDMode_OnUp();
}




