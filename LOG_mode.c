//LOG_mode.c



#include <string.h>
#include <stdio.h>
//#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
//#include <math.h>

#include "types.h"
#include "GPS.h"
#include "display.h"
#include "NMEA_Parser.h"
//#include "powerControl.h"
#include "clock.h"
#include "filesystem.h"
#include "interrupts.h"
#include "ini_control.h"
#include "LOG_mode.h"
#include "SPRD_mode.h"









struct tagLOGModeControl LOGModeControl;


const struct tagMode modes_LOGMode=
{
	LOGMode_NameOnUpdate,//"LOG\0""ОПЦИИ",	//mode name
	RGB(10,63,20),	//mode color
	LOGMode_LeftOnUpdate,//"back\0""назад",	//left
	LOGMode_RightOnUpdate,//"gps\0""gps",	//right
	LOGMode_UpOnUpdate,//"acquir\0""набор",//up
	LOGMode_DownOnUpdate,//"menu\0""меню",	//down
	LOGMode_OnActivate,
	LOGMode_OnLeft,
	LOGMode_OnRight,
	LOGMode_OnUp,
	LOGMode_OnDown,
	LOGMode_OnIdle,
	LOGMode_OnShow,
	LOGMode_OnExit,
	LOGMode_OnPowerDown,
	LOGMode_OnWakeUp,
	LOGMode_OnTimer
};



const char* LOGMode_NameOnUpdate(void)//"LOG\0""ОПЦИИ",	//mode name
{
	return "LOG\0""LOG\0""LOG\0""ЖУРНАЛ";
}
const char* LOGMode_LeftOnUpdate(void)//"back\0""назад",	//left
{
	return "back\0""back\0""back\0""назад";
}
const char* LOGMode_RightOnUpdate(void)//"gps\0""gps",	//right
{
	return "back\0""back\0""back\0""назад";
}
const char* LOGMode_UpOnUpdate(void)//"acquir\0""набор",//up
{
	return "next\0""next\0""next\0""след";
}
const char* LOGMode_DownOnUpdate(void)//"menu\0""меню",	//down
{
	return "begin\0""begin\0""begin\0""начало";
}


BOOL LOGMode_OnTimer(void)
{
	return TRUE;
}





///////////////////////////////////////////////////////////////////////////////





void LOGMode_Init(void)
{
	LOGModeControl.iPage = 0;
	LOGModeControl.iPos = -1;
	LOGModeControl.iPosCur = -1;
	memset(LOGModeControl.txtPage, 0, sizeof(LOGModeControl.txtPage));
}

BOOL LOGMode_OnActivate(void)
{
	LOGMode_Init();
	Modes_updateMode();
	LOGModeControl.iPos = LOGModeControl.iPosCur;
	return 1;
}


BOOL LOGMode_OnLeft(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return 1;
}
BOOL LOGMode_OnRight(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return 1;
}
BOOL LOGMode_OnUp(void)
{
	LOGModeControl.iPage++;
	Modes_clearModeArea();	//clear screen area for mode	
	Modes_OnShow();
	LOGModeControl.iPos = LOGModeControl.iPosCur;
	return 1;
}
BOOL LOGMode_OnDown(void)
{
	LOGModeControl.iPage=0;
	LOGModeControl.iPos = -1;
	Modes_clearModeArea();	//clear screen area for mode	
	Modes_OnShow();
	LOGModeControl.iPos = LOGModeControl.iPosCur;
	return 1;
}
BOOL LOGMode_OnIdle(void)
{
	return 1;
}
BOOL LOGMode_OnShow(void)
{
	LOGMode_showModeScreen();
	return 1;
}
BOOL LOGMode_OnExit(void)
{
	return 1;
}
BOOL LOGMode_OnWakeUp(void)
{
	return 1;
}
BOOL LOGMode_OnPowerDown(void)
{
	return 1;	//allow enter power down
}


void LOGMode_showModeScreen(void)
{
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,3);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt8x16);
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_BOTTOM-MODE_USER_TOP);	//set text window
	Display_setTextColor(YELLOW);	//set text color
	
	if(!LOGModeControl.hfile_log)return;
	
	char str_dt[12];
	memset(str_dt,0,sizeof(str_dt));
	//all datetime messages start from digits
	//no other messages start from dogots
	int pos = LOGModeControl.iPos;
	if(pos==-1)
	{
		pos = filesystem_get_length(LOGModeControl.hfile_log)-1;
		if(pos<0)pos=0;
	}
	int ret;
	do
	{
		if(pos==0)break;

		ret = filesystem_get_stringReverse(LOGModeControl.hfile_log,
									&pos,LOGModeControl.txtPage,sizeof(LOGModeControl.txtPage));
		if(ret==E_FAIL)return;
		if(ret==0)continue;
		//analyze for datetime
		char* pStr = LOGModeControl.txtPage;
		if(LOGModeControl.txtPage[0]>='0' && LOGModeControl.txtPage[0]<='9')
		{//datetime
			pStr = strchr(LOGModeControl.txtPage, ' ');
			if(pStr!=NULL)
			{//found time
				int len = pStr - LOGModeControl.txtPage;
				if(strncmp(str_dt, LOGModeControl.txtPage, len))
				{//another date
					memset(str_dt,0,sizeof(str_dt));
					strncpy(str_dt, LOGModeControl.txtPage, len);
					Display_setTextColor(ORANGE);	//set text color
					Display_outputText(str_dt);
					Display_outputText("\r");
					Display_setTextColor(YELLOW);	//set text color
				}
				pStr++;
			}else
				pStr = LOGModeControl.txtPage;
		}
		
		
		Display_outputText(pStr);
		Display_outputText("\r");
	}while(display.text.gstrY<190);
	LOGModeControl.iPosCur = pos;
}

BOOL LOGMode_insertEventByLang(const char* pEvent)
{
	return LOGMode_insertEvent(Display_getTextByLang(pEvent));
}

//insert text event in log, len of event message must be no more than 230 symbols
BOOL LOGMode_insertEvent(const char* pEvent)
{
	if(!LOGModeControl.hfile_log)return FALSE;
	int pos = filesystem_get_length(LOGModeControl.hfile_log);
	char buf[256];
	Clock_getClockDateTimeStr(buf);
	strcat(buf, "\t");
	strncat(buf, pEvent, MAX_INI_STRING_LEN-26);
	strcat(buf, "\r");
	
	if(strlen(buf)>MAX_INI_STRING_LEN)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Event description is too long");
	}
	
	int rlen = filesystem_file_put(LOGModeControl.hfile_log, &pos,
								   (BYTE*)&buf,
								   strlen(buf));
	return (rlen!=E_FAIL);
}


//create new log or open exists
BOOL LOGMode_createLog(void)
{
	if((LOGModeControl.hfile_log = filesystem_find_file("log","log"))==NULL)
		LOGModeControl.hfile_log = filesystem_create_file("log","log", 1);
	return (LOGModeControl.hfile_log!=NULL);
}


BOOL LOGMode_insertDoserate(float val)
{
	char buf[50];
	strcpy(buf,Display_getTextByLang("Dose rate\0""Dose rate\0""Dose rate\0""Мощность дозы"));
	char buf2[20];
	sprintf(buf2," %.2f ",val);
	strcat(buf,buf2);
	strcat(buf,SPRDMode_getDimension());
	return LOGMode_insertEvent(buf);
}


#ifndef GPS_BT_FREE	

//insert GPS coords in log
BOOL LOGMode_insertGPS(void)
{
	if(!GPSControl.bGPS_ON || !GPSControl.bGPS_Fix)return FALSE;	//no gps or coords
	char buf[MAX_INI_STRING_LEN-26];
	strcpy(buf,"GPS: ");
	char buf2[20];
	sprintf(buf2,"Lat=%f ",(float)NMEAParserControl.commonGPS.Lat);
	strcat(buf,buf2);
	sprintf(buf2,"Lon=%f ",(float)NMEAParserControl.commonGPS.Lon);
	strcat(buf,buf2);
	sprintf(buf2,"Alt=%f ",(float)NMEAParserControl.commonGPS.Alt);
	strcat(buf,buf2);
	LOGMode_insertEvent(buf);
//	sprintf(buf2,"Dir=%.0f ",(float)NMEAParserControl.commonGPS.Dir);
//	strcat(buf,buf2);
//	sprintf(buf2,"Speed=%.0f ",(float)NMEAParserControl.commonGPS.Spd);
//	strcat(buf,buf2);
	strcpy(buf,"GPS: ");
	sprintf(buf2,"HDOP=%.1f ",NMEAParserControl.m_dGSAHDOP);
	strcat(buf,buf2);
	sprintf(buf2,"Sats=%u",(UINT)NMEAParserControl.m_btGGANumOfSatsInUse);
	strcat(buf,buf2);
	LOGMode_insertEvent(buf);
	return TRUE;
}
#endif	//#ifndef GPS_BT_FREE	
