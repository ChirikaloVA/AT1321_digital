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
	return "gps\0""gps\0""gps\0""gps";
}
const char* MCSMode_DownOnUpdate(void)//"menu\0""меню",	//down
{
	if(!(SPRDModeControl.bBkgMode_assumed && SPRDModeControl.bBkgMode_confirmed))
		return "save\0""ablegn\0""save\0""сохран";
	else
		return NULL;
}


BOOL MCSMode_OnPowerDown(void)
{
	return 0;	//not allowed
}



BOOL MCSMode_OnExit(void)
{
	if(MCSModeControl.iNumberOfCpsTotal>=MAX_MCS)
	{//удалять вр файл 
		MCSMode_prepare();
		MCSMode_createTempFile();
	}
	return SPRDMode_OnExit();
}

void MCSMode_Init(void)
{
	SPRDMode_Init();
	MCSMode_prepare();
}


BOOL MCSMode_OnActivate(void)
{
	SPRDMode_OnActivate();
	MCSMode_prepare();
//	MCSMode_createTempFile();
	return 1;
}


void MCSMode_prepare(void)
{
	MCSModeControl.hfile_tmp = NULL;
	for(int i=0;i<MAX_MCS;i++)
		MCSModeControl.mcs[i]=0;
	MCSModeControl.iNumberOfCps = 0;
	MCSModeControl.iNumberOfCpsTotal = 0;
	MCSModeControl.bSaveSpectrum = FALSE;
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

void MCSMode_createTempFile(void)
{
	MCSModeControl.hfile_tmp = filesystem_create_file("mcs","tmp", 1);
	if(MCSModeControl.hfile_tmp)
	{
		int pos = 0;
		DWORD val = 0xeeeeeeee;
		int rlen = filesystem_file_put(MCSModeControl.hfile_tmp, &pos,
									   (BYTE*)&val,
									   sizeof(val));
		if(rlen==E_FAIL)
		{//failed to create/save data in a temp file
			//delete it
			filesystem_delete_file(MCSModeControl.hfile_tmp);
			MCSModeControl.hfile_tmp = NULL;
		}
	}
}


BOOL MCSMode_OnTimer(void)
{
	InterProc_readMeasurementRegs();
	
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
		MCSModeControl.mcs[MCSModeControl.iNumberOfCps++] =
			(UINT)SPRDModeControl.fMomCps | (SPRDModeControl.bRadFound?0x80000000:0x0);
		if(MCSModeControl.iNumberOfCps==MAX_MCS)
			MCSMode_saveMCS();//must save in a temp file
		MCSModeControl.iNumberOfCpsTotal++;
	}
	
	if(MCSModeControl.bSaveSpectrum &&
	   InterProc_isDataFinalReady(&interProcControl.rsModbus.sarSpectrum) && MCSModeControl.iNumberOfCps>0)
	{
		MCSModeControl.bSaveSpectrum = FALSE;
		if(SPRDMode_saveAutoSpec())
		{
			if(MCSMode_saveMCS())//save mcs with manual saved spectrum
			{
				char autoname[FILE_NAME_SZ];
				char temp[FILE_NAME_SZ];
				sprintf(autoname, "mcs_%s", Clock_getClockDateTimeStrEx(temp));
				filesystem_rename_file("mcs","tmp",autoname,"mcs");
				MCSMode_prepare();
		
				//save doserate and gps in log too
				if(LOGMode_insertGPS())
					LOGMode_insertDoserate(SPRD_GetCurrentDoserate());
				SoundControl_BeepSeq(beepSeq_OK);
			}
		
		}
	}
	SPRDModeControl.bRadFound = FALSE;	//reset alarm flag
	
	Modes_OnShow();
	return 1;
}



//auto save mcs in temp file
BOOL MCSMode_saveMCS(void)
{
	if(!MCSModeControl.hfile_tmp)
		MCSMode_createTempFile();
	
	if(!MCSModeControl.hfile_tmp)
	{//error no file, or no memory
		MCSModeControl.iNumberOfCps = 0;
		SETUPMode_clear_memory();
		return FALSE;
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
									   (BYTE*)&MCSModeControl.mcs,
									   MCSModeControl.iNumberOfCps*sizeof(DWORD));
	}
	//clear spectrum name to be sure of wrong info
	memset(SPRDModeControl.spec_name, 0, sizeof(SPRDModeControl.spec_name));
	MCSModeControl.iNumberOfCps = 0;
	if(rlen==E_FAIL)
	{//nomemory
		SETUPMode_clear_memory();
		return FALSE;
	}else
		return TRUE;
}


BOOL MCSMode_OnLeft(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return 1;
}
BOOL MCSMode_OnRight(void)
{
	GPSMode_setActiveMode();
	return 1;
}

//сохран диаграммы и спектра
BOOL MCSMode_OnDown(void)
{
	//read spectrum
	InterProc_readAcqTime();
	InterProc_readSpectrum();
	MCSModeControl.bSaveSpectrum = TRUE;
	return 1;
}


BOOL MCSMode_OnUp(void)
{
	MCSMode_prepare();
//	MCSMode_createTempFile();
	return SPRDMode_OnUp();
}
