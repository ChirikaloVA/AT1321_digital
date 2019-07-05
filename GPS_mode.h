

#ifndef _GPSMODE_H
#define _GPSMODE_H

#include "types.h"
#include "modes.h"

#define MAX_GPS_SCREENS 5
enum enu_GPS_SCREEN
{
	GPS_SCREEN_COMMON,
	GPS_SCREEN_GPGGA,
	GPS_SCREEN_GPGSA,
	GPS_SCREEN_GPGSV,
	GPS_SCREEN_GPRMC
};


struct tagGPSModeControl
{
	int iScreenMode;
};


extern struct tagGPSModeControl GPSModeControl;

extern const struct tagMode modes_GPSMode;

extern const struct tagMenu gps_menu;


void GPSMode_Init(void);


BOOL GPSMode_OnActivate(void);
BOOL GPSMode_OnLeft(void);
BOOL GPSMode_OnRight(void);
BOOL GPSMode_OnUp(void);
BOOL GPSMode_OnDown(void);
BOOL GPSMode_OnIdle(void);
BOOL GPSMode_OnShow(void);
BOOL GPSMode_OnExit(void);
BOOL GPSMode_OnPowerDown(void);
BOOL GPSMode_OnWakeUp(void);
BOOL GPSMode_OnTimer(void);
void GPSMode_showModeScreen(void);

void GPSMode_show_GPGGA(void);
void GPSMode_show_GPGSA(void);
void GPSMode_show_GPGSV(void);
void GPSMode_show_GPRMC(void);


void GPSMode_show_common(void);

void GPSMode_setActiveMode(void);



BOOL GPSMode_menu1_HotStart(void);
BOOL GPSMode_menu1_WarmStart(void);
BOOL GPSMode_menu1_WarmStart2(void);
BOOL GPSMode_menu1_ColdStart(void);

BOOL GPSMode_menu1_gps_change_state(void);


const char* GPSMode_menu1_gps_change_state_onUpdate(void);
const char* GPSMode_menu1_ColdStart_onUpdate(void);
const char* GPSMode_menu1_WarmStart2_onUpdate(void);
const char* GPSMode_menu1_WarmStart_onUpdate(void);
const char* GPSMode_menu1_HotStart_onUpdate(void);


const char* GPSMode_NameOnUpdate(void);//"GPS OFF\0""GPS ВЫКЛ",	//mode name
const char* GPSMode_LeftOnUpdate(void);//"back\0""назад",	//left
const char* GPSMode_RightOnUpdate(void);//"comm\0""связь",	//right
const char* GPSMode_UpOnUpdate(void);//"on\0""вкл",//up
const char* GPSMode_DownOnUpdate(void);//"\0""",	//down

char* GPSMode_coord2txt(double dval, char* pBuf, char worldpart1, char worldpart2);

BOOL GPSMode_menu1_TurnOFF(void);
const char* GPSMode_menu1_TurnOFF_onUpdate(void);




#endif	//ifndef _GPSMODE_H


