#ifndef _SPRDMODE_H
#define _SPRDMODE_H

#include "types.h"
#include "modes.h"
#include "filesystem.h"
#include "display.h"

#define MCS_WIN_BOTTOM 286
#define MCS_WIN_HEIGHT 128
#define MCS_WIN_HEIGHT2 112
#define MCS_WIN_TOP MCS_WIN_BOTTOM-MCS_WIN_HEIGHT+1
#define MCS_WIN_MIDLLE_D MCS_WIN_HEIGHT2/4
#define MCS_WIN_MIDLLE_U MCS_WIN_HEIGHT2-MCS_WIN_MIDLLE_D
#define MCS_WIN_WIDTH X_SCREEN_SIZE


//предел набора фона
#define CPS_ERR_THRESHOLD (float)7.0


#ifdef BNC
//max doserate by NaI in mrem/h
#define MAX_NAI_DR 30
//max doserate of whole SPRD in mrem/h
#define MAX_DR 10000
#else
//max doserate by NaI in mkSv/h
#define MAX_NAI_DR 300
//max doserate of whole SPRD in mkSv/h
#define MAX_DR 100000
#endif



//max cps for NAI
#define MAX_NAI_CPS 50000
//!!!!!!!!!!!!!!!! for test
//#define MAX_NAI_CPS 1000

//standart background cps
#define STANDART_BKG_CPS 50.0

/*
#define MAX_DIMS 3
enum ENU_DIMENSION
{
	enu_dim_sv,
	enu_dim_gy,
	enu_dim_r,
};
*/

extern const char strIdentified[];

struct tagSPRDModeControl
{
	int arMCS[MCS_WIN_WIDTH];
	float fMomCps;	//momentary cps
	float fCps;	//
	float fCpsErr;	//
	float fDoserate;	//
	float fDoserateErr;	//
	float fDose;
	short sSigma;
	BOOL bBkgMode_confirmed;	//mode of bkg acq
	BOOL bBkgMode_assumed;	//used to alarm on search, it is setup when status read
	float fDRThreshold;


	volatile BOOL bRadFound;	//true if radiation found, used to wakeup from sleep mode, to store event msg in log
	BOOL bIdentMode;	//true if radiation found and spec started
	BOOL bCanUpdateIdentResult;	//if true then ident result on screen will be updated
	char buf[100];
	BOOL bIdentStoped;	//TRUE if Ident stoped
	volatile BOOL bUpdateMCS;	//used to update MCS at one step after no geiger alarm detected
//	volatile BOOL bSearchAlarm;	//1-then search alarm, 0-no alarm
	volatile int iAlarmTimer;	//
//	int iDimension;	//dimension
	//alarm sigmas for diff modes
	float operation_search_sigma[6];
//	float sleepmode_search_sigma[6];
	//false alarmperiod for diff modes
//	UINT false_alarm_period_sleep;	//in minutes
	UINT false_alarm_period_oper;	//in minutes

	char spec_name[FILE_NAME_SZ];	//name of auto saved spectrum
	BOOL bNaIOverload;	//NaI oveload
	volatile BOOL bMustSwitchToSPRD;	//if true then system will enable SPRD mode as active when alarm is detected

	BOOL bGMMode;
	BOOL bNaIMode;

	int year;
	int month;
	int daym;
	int dayw;
	int dayy;
	int hour;
	int minute;

	BOOL bAutoSaveSpectra;	//true spectra will save after identification complete
};

extern struct tagSPRDModeControl SPRDModeControl;

extern const struct tagMode modes_SPRDMode;


extern const struct tagMenu SPRD_menu;

void SPRDMode_Init(void);


BOOL SPRDMode_OnActivate(void);
BOOL SPRDMode_OnLeft(void);
BOOL SPRDMode_OnRight(void);
BOOL SPRDMode_OnUp(void);
BOOL SPRDMode_OnDown(void);
BOOL SPRDMode_OnIdle(void);
BOOL SPRDMode_OnShow(void);
BOOL SPRDMode_OnExit(void);
BOOL SPRDMode_OnPowerDown(void);
BOOL SPRDMode_OnWakeUp(void);
BOOL SPRDMode_OnTimer(void);
void SPRDMode_showModeScreen(void);
void SPRDMode_showModeHeaders(void);
void SPRDMode_showDR(void);
void SPRDMode_showCps(void);


void SPRDMode_showMCS(void);

const char* SPRDMode_NameOnUpdate(void);//"SPRD\0""СПРД",	//mode name
const char* SPRDMode_LeftOnUpdate(void);//"setup\0""опции",	//right
const char* SPRDMode_RightOnUpdate(void);//"rid\0""рид",	//left
const char* SPRDMode_UpOnUpdate(void);//"acquir\0""набор",//up
const char* SPRDMode_DownOnUpdate(void);//"menu\0""меню",	//down

void SPRDMode_showWholeMCS_once(void);

void SPRDMode_clearMCS(void);




void SPRDMode_PSW_done(BOOL bOK);
void SPRDMode_showIdent(void);


void SPRDMode_Init_for_ident(void);
BOOL SPRDMode_checkForIdentMode(void);
void SPRDMode_showAlarm(void);
BOOL SPRDMode_saveAutoSpec(void);



char* SPRDMode_getDimension(void);
char* SPRDMode_getDimensionDose(void);
//void SPRDMode_showTime(void);
void SPRDMode_showNaIAlarm(void);

BOOL SPRDMode_menu1_Stabilize(void);
const char* SPRDMode_menu1_Stabilize_onUpdate(void);






//NaI measurement mode
BOOL SPRDMode_menu1_NaIMeasure(void);
const char* SPRDMode_menu1_NaIMeasure_onUpdate(void);
//GM measurement mode
BOOL SPRDMode_menu1_GMMeasure(void);
const char* SPRDMode_menu1_GMMeasure_onUpdate(void);


//switch to sprd mdoe
BOOL SPRDMode_menu1_SPRDmode(void);
const char* SPRDMode_menu1_SPRDmode_onUpdate(void);



BOOL SPRDMode_menu1_lang(void);
const char* SPRDMode_menu1_lang_onUpdate(void);



const char* SPRDMode_menu1_datetime_onUpdate(void);
BOOL SPRDMode_menu1_datetime(void);
void SPRDMode_menu1_datetime_year_done(BOOL bOK);
void SPRDMode_menu1_datetime_month_done(BOOL bOK);
void SPRDMode_menu1_datetime_daym_done(BOOL bOK);
void SPRDMode_menu1_datetime_dayw_done(BOOL bOK);
//void SPRDMode_menu1_datetime_dayy_done(BOOL bOK);
void SPRDMode_menu1_datetime_hour_done(BOOL bOK);
void SPRDMode_menu1_datetime_minute_done(BOOL bOK);


BOOL SPRDMode_menu1_TurnOFF(void);
const char* SPRDMode_menu1_TurnOFF_onUpdate(void);


BOOL SPRDMode_menu1_SaveValue(void);
const char* SPRDMode_menu1_SaveValue_onUpdate(void);

float SPRD_GetCurrentDoserate(void);
float SPRD_GetCurrentDoserateErr(void);


const char* SPRDMode_menu1_BkgMode_onUpdate(void);
BOOL SPRDMode_menu1_BkgMode(void);


void SPRDMode_showGMOverload(void);


#endif	//ifndef _SPRDMODE_H
