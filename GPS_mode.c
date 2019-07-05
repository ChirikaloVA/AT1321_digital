//gps_mode.c



#include <string.h>
#include <stdio.h>
//#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <math.h>

#include "types.h"
//#include "syncObj.h"
//#include "spectrum.h"
#include "display.h"
//#include "interProc.h"
#include "keyboard.h"
#include "powerControl.h"
#include "gps_mode.h"
#include "gps.h"
#include "NMEA_Parser.h"
#include "sprd_mode.h"
#include "comm_mode.h"
#include "ini_control.h"
/*
#include "mcs_mode.h"
#include "tc_mode.h"
*/






struct tagGPSModeControl GPSModeControl;



#ifndef GPS_BT_FREE	


//menu if gps is on
const struct tagMenu gps_menu=
{
	"MENU: GPS\0""MENU: GPS\0""MENU: GPS\0""МЕНЮ: GPS",	//menu name
	6,	//number of items
	{GPSMode_menu1_TurnOFF,GPSMode_menu1_HotStart,GPSMode_menu1_WarmStart,GPSMode_menu1_WarmStart2,GPSMode_menu1_ColdStart,GPSMode_menu1_gps_change_state,
	},
	{GPSMode_menu1_TurnOFF_onUpdate, GPSMode_menu1_HotStart_onUpdate,GPSMode_menu1_WarmStart_onUpdate,GPSMode_menu1_WarmStart2_onUpdate,GPSMode_menu1_ColdStart_onUpdate,GPSMode_menu1_gps_change_state_onUpdate,
	}
};







const struct tagMode modes_GPSMode=
{
	GPSMode_NameOnUpdate,//"GPS OFF\0""GPS ВЫКЛ",	//mode name
	RGB(0,63,30),	//mode color
	GPSMode_LeftOnUpdate,//"back\0""назад",	//left
	GPSMode_RightOnUpdate,//"comm\0""связь",	//right
	GPSMode_UpOnUpdate,//"on\0""вкл",//up
	GPSMode_DownOnUpdate,//"\0""",	//down
	GPSMode_OnActivate,
	GPSMode_OnLeft,
	GPSMode_OnRight,
	GPSMode_OnUp,
	GPSMode_OnDown,
	GPSMode_OnIdle,
	GPSMode_OnShow,
	GPSMode_OnExit,
	GPSMode_OnPowerDown,
	GPSMode_OnWakeUp,
	GPSMode_OnTimer
};



const char* GPSMode_NameOnUpdate(void)//"GPS OFF\0""GPS ВЫКЛ",	//mode name
{
	if(GPSControl.bGPS_ON)
		return "GPS ON\0""GPS ON\0""GPS ON\0""GPS ВКЛ";
	else
		return "GPS OFF\0""GPS OFF\0""GPS OFF\0""GPS ВЫКЛ";
}
const char* GPSMode_LeftOnUpdate(void)//"back\0""назад",	//left
{
	return "back\0""back\0""back\0""назад";
}
const char* GPSMode_RightOnUpdate(void)//"comm\0""связь",	//right
{
	return "link\0""link\0""link\0""связь";
}
const char* GPSMode_UpOnUpdate(void)//"on\0""вкл",//up
{
	const char* pRet=NULL;
	if(!GPSControl.bGPS_ON)
	{
		pRet = "on\0""on\0""on\0""вкл";
	}else
	{
		switch(GPSModeControl.iScreenMode)
		{
		case GPS_SCREEN_COMMON:
			pRet = "gpgga\0""gpgga\0""gpgga\0""gpgga";
			break;
		case GPS_SCREEN_GPGGA:
			pRet = "gpgsa\0""gpgsa\0""gpgsa\0""gpgsa";
			break;
		case GPS_SCREEN_GPGSA:
			pRet = "gpgsv\0""gpgsv\0""gpgsv\0""gpgsv";
			break;
		case GPS_SCREEN_GPGSV:
			pRet = "gprmc\0""gprmc\0""gprmc\0""gprmc";
			break;
		case GPS_SCREEN_GPRMC:
			pRet = "common\0""common\0""common\0""общие";
			break;
		}
	}
	return pRet;
}
const char* GPSMode_DownOnUpdate(void)//"\0""",	//down
{
	return "menu\0""menu\0""menu\0""меню";
}



//init by default
void GPSMode_Init(void)
{
	GPSModeControl.iScreenMode = 0;
}

BOOL GPSMode_OnActivate(void)
{
	Modes_createTimer(1000);
	Modes_updateMode();
	return 1;
}

BOOL GPSMode_OnTimer(void)
{
	Modes_OnShow();
	return 1;
}


BOOL GPSMode_OnLeft(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return 1;
}
BOOL GPSMode_OnRight(void)
{
	COMMMode_setActiveMode();
	return 1;
}
BOOL GPSMode_OnUp(void)
{
	if(!GPSControl.bGPS_ON)
	{
		GPS_turnON();
	}else
	{
		if(++GPSModeControl.iScreenMode>=MAX_GPS_SCREENS)
			GPSModeControl.iScreenMode = 0;
	}
	Modes_updateMode();
	return 1;
}
BOOL GPSMode_OnDown(void)
{
	Modes_activateMenu(&gps_menu);
	return 1;
}
BOOL GPSMode_OnIdle(void)
{
	return 1;
}
BOOL GPSMode_OnShow(void)
{
	GPSMode_showModeScreen();
	return 1;
}
BOOL GPSMode_OnExit(void)
{
	return 1;
}
BOOL GPSMode_OnWakeUp(void)
{
//	GPSMode_OnShow();
	return 1;
}
BOOL GPSMode_OnPowerDown(void)
{
	return 0;	//disable enter power down
}


void GPSMode_showModeScreen(void)
{
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,110);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextSteps(1,2);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(LEFT);
	Display_setCurrentFont(fnt16x16);	//set current font
	Display_setTextLineClear(1);
	Display_checkForClearLine();
	if(GPSControl.bGPSFailed)
	{
		Display_outputTextByLang("No GPS receiver\0""No GPS receiver\0""No GPS receiver\0""Нет GPS приемника");		
	}else
	{
	if(GPSControl.bGPS_ON)
	{
		switch(GPSModeControl.iScreenMode)
		{
		case GPS_SCREEN_COMMON:
			GPSMode_show_common();
			break;
		case GPS_SCREEN_GPGGA:
			GPSMode_show_GPGGA();
			break;
		case GPS_SCREEN_GPGSA:
			GPSMode_show_GPGSA();
			break;
		case GPS_SCREEN_GPGSV:
			GPSMode_show_GPGSV();
			break;
		case GPS_SCREEN_GPRMC:
			GPSMode_show_GPRMC();
			break;
		}
	}else
	{
//		Display_clearTextWin(10);
		Display_outputTextByLang("GPS OFF\0""GPS OFF\0""GPS OFF\0""GPS ВЫКЛЮЧЕН");		
	}
	}
	Display_setTextLineClear(0);
}


//convert coord in double to text
char* GPSMode_coord2txt(double dval, char* pBuf, char worldpart1, char worldpart2)
{
	char sym = worldpart1;
	if(dval<0)
	{
		sym = worldpart2;
		dval=-dval;
	}
	int deg = (int)dval;
	float sec = (dval-deg)*60.0;
	int min = (int)(sec);
	sec = (float)((sec-min)*60.0);
	char buf[2];
	buf[0] = sym;
	buf[1] = '\0';
	sprintf(pBuf, "%d°%d'%.2f'' %s", deg, min, sec, buf);
	return pBuf;
}



//just show debug data
void GPSMode_show_common(void)
{
	char gps_text[40];
	char buf[20];

//	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputTextByLang("Common data:\r\0""Common data:\r\0""Common data:\r\0""Общие данные\r");

	sprintf(gps_text, "Lat=%s\r", GPSMode_coord2txt(NMEAParserControl.commonGPS.Lat,buf,'N','S'));
//	Display_setTextWin(0,MODE_USER_TOP-16,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);
	sprintf(gps_text, "Lon=%s\r", GPSMode_coord2txt(NMEAParserControl.commonGPS.Lon,buf,'E','W'));
//	Display_setTextWin(0,MODE_USER_TOP-32,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);
	sprintf(gps_text, "Alt=%.2f\r", NMEAParserControl.commonGPS.Alt);
//	Display_setTextWin(0,MODE_USER_TOP-48,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);
	sprintf(gps_text, "Spd=%.2f\r", NMEAParserControl.commonGPS.Spd);
//	Display_setTextWin(0,MODE_USER_TOP-64,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);
	sprintf(gps_text, "Dir=%.2f\r", NMEAParserControl.commonGPS.Dir);
//	Display_setTextWin(0,MODE_USER_TOP-80,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);
	sprintf(gps_text, "Sats=%d\r", (int)NMEAParserControl.m_btGGANumOfSatsInUse);
//	Display_setTextWin(0,MODE_USER_TOP-96,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);
	
//	Display_setTextWin(0,MODE_USER_TOP-112,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	if(GPSControl.uart.bRcvError!=RCV_OK)
	{
		Display_outputText("Receive Error");
		GPSControl.uart.bRcvError = RCV_OK;
	}else
		Display_outputText("\r");
}



void GPSMode_show_GPGGA(void)
{
/*
	BYTE m_btGGAHour;					//
	BYTE m_btGGAMinute;					//
	BYTE m_btGGASecond;					//
	double m_dGGALatitude;				// < 0 = South, > 0 = North
	double m_dGGALongitude;				// < 0 = West, > 0 = East
	BYTE m_btGGAGPSQuality;				// 0 = fix not available, 1 = GPS sps mode, 2 = Differential GPS, SPS mode, fix valid, 3 = GPS PPS mode, fix valid
	BYTE m_btGGANumOfSatsInUse;			//
	double m_dGGAHDOP;					//
	double m_dGGAAltitude;				// Altitude: mean-sea-level (geoid) meters
	double m_dGGAVertSpeed;				//	
	*/
	
	
	char gps_text[40];
	char buf[20];

//	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText("GPGGA:\r");

	sprintf(gps_text, "Clock=%02u:%02u:%02u\r", (UINT)NMEAParserControl.m_btGGAHour,
			(UINT)NMEAParserControl.m_btGGAMinute,
			(UINT)NMEAParserControl.m_btGGASecond);
//	Display_setTextWin(0,MODE_USER_TOP-16,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "Lat=%s\r", GPSMode_coord2txt(NMEAParserControl.m_dGGALatitude,buf,'N','S'));
//	Display_setTextWin(0,MODE_USER_TOP-32,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "Lon=%s\r", GPSMode_coord2txt(NMEAParserControl.m_dGGALongitude,buf,'E','W'));
//	Display_setTextWin(0,MODE_USER_TOP-48,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "GPSQuality=%u\r", (UINT)NMEAParserControl.m_btGGAGPSQuality);
//	Display_setTextWin(0,MODE_USER_TOP-64,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "NumOfSatsInUse=%u\r", (UINT)NMEAParserControl.m_btGGANumOfSatsInUse);
//	Display_setTextWin(0,MODE_USER_TOP-80,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "HDOP=%.3f\r", (float)NMEAParserControl.m_dGGAHDOP);
//	Display_setTextWin(0,MODE_USER_TOP-96,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "Alt=%.2f\r", NMEAParserControl.m_dGGAAltitude);
//	Display_setTextWin(0,MODE_USER_TOP-112,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);
	
	sprintf(gps_text, "Vert speed=%.2f\r", NMEAParserControl.m_dGGAVertSpeed);
//	Display_setTextWin(0,MODE_USER_TOP-128,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);
}

void GPSMode_show_GPGSA(void)
{
	/*
	BYTE m_btGSAMode;					// M = manual, A = automatic 2D/3D
	BYTE m_btGSAFixMode;				// 1 = fix not available, 2 = 2D, 3 = 3D
	WORD m_wGSASatsInSolution[NP_MAX_CHAN]; // ID of sats in solution
	double m_dGSAPDOP;					//
	double m_dGSAHDOP;					//
	double m_dGSAVDOP;					//
	*/


	char gps_text[40];
//	char buf[20];
	char buf2[2];


//	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText("GPGSA:\r");

	buf2[0]=NMEAParserControl.m_btGGAHour;
	buf2[1]=0;
	sprintf(gps_text, "Mode=%s\r", buf2);
//	Display_setTextWin(0,MODE_USER_TOP-16,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "FixMode=%u\r", (UINT)NMEAParserControl.m_btGSAFixMode);
//	Display_setTextWin(0,MODE_USER_TOP-32,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "PDOP=%.3f\r", (float)NMEAParserControl.m_dGSAPDOP);
//	Display_setTextWin(0,MODE_USER_TOP-48,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "HDOP=%.3f\r", (float)NMEAParserControl.m_dGSAHDOP);
//	Display_setTextWin(0,MODE_USER_TOP-64,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "VDOP=%.3f\r", (float)NMEAParserControl.m_dGSAVDOP);
//	Display_setTextWin(0,MODE_USER_TOP-80,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

//	Display_setTextWin(0,MODE_USER_TOP-96,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText("SatsInSolution=\r");	
	for(int i=0;i<12;i+=3)
	{
		sprintf(gps_text, "%u, %u, %u\r",
				(UINT)NMEAParserControl.m_wGSASatsInSolution[i],
				(UINT)NMEAParserControl.m_wGSASatsInSolution[i+1],
				(UINT)NMEAParserControl.m_wGSASatsInSolution[i+2]);
//		Display_setTextWin(0,MODE_USER_TOP-112-i*6,X_SCREEN_SIZE,16);	//set text window
//		Display_clearTextWin(10);
		Display_outputText((char*)gps_text);
	}	
}

void GPSMode_show_GPGSV(void)
{
	/*
	BYTE m_btGSVTotalNumOfMsg;			//
	WORD m_wGSVTotalNumSatsInView;		//
	struct tagNPSatInfo m_GSVSatInfo[NP_MAX_CHAN];	//
	DWORD m_dwGSVCount;					//

struct tagNPSatInfo
{
	WORD	m_wPRN;						//
	WORD	m_wSignalQuality;			//
	BOOL	m_bUsedInSolution;			//
	WORD	m_wAzimuth;					//
	WORD	m_wElevation;				//
};

	*/
	char gps_text[40];
//	char buf[20];
//	char buf2[2];
	
	Display_setTextSteps(1,1);	//set start coords in window

//	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	sprintf(gps_text, "GPGSV: Sats=%u", (UINT)NMEAParserControl.m_wGSVTotalNumSatsInView);
//	Display_setTextWin(0,MODE_USER_TOP-16,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	int i;	
	for(i=0;i<NMEAParserControl.m_wGSVTotalNumSatsInView;i++)
	{
		sprintf(gps_text, "\rid=%u, used=%u, sq=%u", (UINT)NMEAParserControl.m_GSVSatInfo[i].m_wPRN,
				(UINT)NMEAParserControl.m_GSVSatInfo[i].m_bUsedInSolution,
				(UINT)NMEAParserControl.m_GSVSatInfo[i].m_wSignalQuality);
//		Display_setTextWin(0,MODE_USER_TOP-32-i*16,X_SCREEN_SIZE,16);	//set text window
//		Display_clearTextWin(10);
		Display_outputText((char*)gps_text);
	}
	//added 03/12/2012 to delete old data about satelites
	strcpy(gps_text, "\rid=-, used=-, sq=-");
	for(;i<12;i++)
	{
		Display_outputText((char*)gps_text);
	}
}
void GPSMode_show_GPRMC(void)
{
	/*
	BYTE m_btRMCHour;					//
	BYTE m_btRMCMinute;					//
	BYTE m_btRMCSecond;					//
	BYTE m_btRMCDataValid;				// A = Data valid, V = navigation rx warning
	double m_dRMCLatitude;				// current latitude
	double m_dRMCLongitude;				// current longitude
	double m_dRMCGroundSpeed;			// speed over ground, knots
	double m_dRMCCourse;				// course over ground, degrees TRUE
	BYTE m_btRMCDay;					//
	BYTE m_btRMCMonth;					//
	WORD m_wRMCYear;					//
	double m_dRMCMagVar;				// magnitic variation, degrees East(+)/West(-)
	DWORD m_dwRMCCount;					//
	*/
	char gps_text[40];
	char buf[20];
	char buf2[2];
//	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText("GPRMC:\r");
	
	sprintf(gps_text, "Clock=%02u:%02u:%02u\r", (UINT)NMEAParserControl.m_btRMCHour,
			(UINT)NMEAParserControl.m_btRMCMinute,
			(UINT)NMEAParserControl.m_btRMCSecond);
//	Display_setTextWin(0,MODE_USER_TOP-16,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "Date=%u:%02u:%02u\r", (UINT)NMEAParserControl.m_wRMCYear,
			(UINT)NMEAParserControl.m_btRMCMonth,
			(UINT)NMEAParserControl.m_btRMCDay);
//	Display_setTextWin(0,MODE_USER_TOP-32,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "Lat=%s\r", GPSMode_coord2txt(NMEAParserControl.m_dRMCLatitude,buf,'N','S'));
//	Display_setTextWin(0,MODE_USER_TOP-48,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "Lon=%s\r", GPSMode_coord2txt(NMEAParserControl.m_dRMCLongitude,buf,'E','W'));
//	Display_setTextWin(0,MODE_USER_TOP-64,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);
	
	sprintf(gps_text, "Ground speed=%.2f\r", NMEAParserControl.m_dRMCGroundSpeed);
//	Display_setTextWin(0,MODE_USER_TOP-80,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);
	
	sprintf(gps_text, "Course=%.2f\r", NMEAParserControl.m_dRMCCourse);
//	Display_setTextWin(0,MODE_USER_TOP-96,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	sprintf(gps_text, "MagVar=%f\r", NMEAParserControl.m_dRMCMagVar);
//	Display_setTextWin(0,MODE_USER_TOP-112,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);

	buf2[0]=NMEAParserControl.m_btRMCDataValid;
	buf2[1]=0;
	sprintf(gps_text, "DataValid=%s\r", buf2);
//	Display_setTextWin(0,MODE_USER_TOP-128,X_SCREEN_SIZE,16);	//set text window
//	Display_clearTextWin(10);
	Display_outputText((char*)gps_text);
}
















//set self mode depends on gps state
void GPSMode_setActiveMode(void)
{
	Modes_setActiveMode(&modes_GPSMode);
}





BOOL GPSMode_menu1_HotStart(void)
{
	GPS_HotStart();
	return 1;
}

BOOL GPSMode_menu1_WarmStart(void)
{
	GPS_WarmStart();
	return 1;
}

BOOL GPSMode_menu1_WarmStart2(void)
{
	GPS_WarmStart2();
	return 1;
}

BOOL GPSMode_menu1_ColdStart(void)
{
	GPS_ColdStart();
	return 1;
}



const char* GPSMode_menu1_gps_change_state_onUpdate(void)
{
	if(GPSControl.gps_state==GPS_STATE_ALWAYS_ON)
		return "ON: always\0""ON: always\0""ON: always\0""ВКЛ: всегда";
	else if(GPSControl.gps_state==GPS_STATE_ALWAYS_OFF)
		return "ON: manually\0""ON: manually\0""ON: manually\0""ВКЛ: вручную";
	else
		return "ON: active\0""ON: active\0""ON: active\0""ВКЛ: активно";
}


const char* GPSMode_menu1_ColdStart_onUpdate(void)
{
	if(!GPSControl.bGPS_ON)return NULL;
	return "Cold start\0""Cold start\0""Cold start\0""Холодный пуск";
}
const char* GPSMode_menu1_WarmStart2_onUpdate(void)
{
	if(!GPSControl.bGPS_ON)return NULL;
	return "Warm start\0""Warm start\0""Warm start\0""Тёплый пуск 2";
}
const char* GPSMode_menu1_WarmStart_onUpdate(void)
{
	if(!GPSControl.bGPS_ON)return NULL;
	return "Warm start\0""Warm start\0""Warm start\0""Тёплый пуск";
}
const char* GPSMode_menu1_HotStart_onUpdate(void)
{
	if(!GPSControl.bGPS_ON)return NULL;
	return "Hot start\0""Hot start\0""Hot start\0""Горячий пуск";
}



//change state of gps
BOOL GPSMode_menu1_gps_change_state(void)
{
	if(GPSControl.gps_state==GPS_STATE_ALWAYS_ON)
		GPSControl.gps_state=GPS_STATE_ALWAYS_OFF;
	else if(GPSControl.gps_state==GPS_STATE_ALWAYS_OFF)
		GPSControl.gps_state=GPS_STATE_OFF_IN_SLEEP;
	else
		GPSControl.gps_state=GPS_STATE_ALWAYS_ON;
	if(!ini_write_system_ini_int("GPSControl", "gps_state", GPSControl.gps_state))
	{
		;//!!!!!!!!!error
	}
	return 0;	//just redraw menu
}



const char* GPSMode_menu1_TurnOFF_onUpdate(void)
{
	if(!GPSControl.bGPS_ON)return NULL;
	return "Turn OFF\0""Turn OFF\0""Turn OFF\0""Выключить";
}

BOOL GPSMode_menu1_TurnOFF(void)
{
	GPS_turnOFF();
	return TRUE;
}


#endif	//#ifndef GPS_BT_FREE	

