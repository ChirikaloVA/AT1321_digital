#ifndef _SETUPMODE_H
#define _SETUPMODE_H

#include "types.h"
#include "modes.h"

//три экрана сетуп инфы
#define MAX_SETUP_MODE_SCREENS 6

enum enu_setup_mode_SCR
{
	ENU_SETUP_MODE_SCR_VER,
	ENU_SETUP_MODE_SCR_MEM,
	ENU_SETUP_MODE_SCR_SPEC,
	ENU_SETUP_MODE_SCR_LIB,
	ENU_SETUP_MODE_SCR_GEIGER,
	ENU_SETUP_MODE_SCR_BAT,
};


struct tagSETUPModeControl
{
	int iModeScreen;
	UINT uiCalcFreeMem; //free memory in KB
	UINT uiAllFiles;
	UINT uiSPZFiles;
	UINT uiLIBFiles;
	UINT uiMCSFiles;
	UINT uiLOGFiles;
	char buf1[20];
	char buf2[20];
	char buf3[20];
	char buf4[20];
	char buf5[20];
	char buf6[20];
	char buf7[20];
	char buf8[20];
	char buf9[20];

	DWORD Serial;
	BYTE ManufacturedMonth;
	WORD ManufacturedYear;

//	BOOL bExpertMode;
//	int isplash;	//исп для ротации заставок, to get splash screens

//	int iJustLeaveScreen;	//to get splash screens


	char lpzCalcMD5[34];
	char lpzCalcCRC32[10];


};

extern struct tagSETUPModeControl SETUPModeControl;

extern const struct tagMode modes_SETUPMode;

extern const struct tagMenu setup_menu_version;
extern const struct tagMenu setup_menu_library;
extern const struct tagMenu setup_menu_memory;
extern const struct tagMenu setup_menu_geiger;
extern const struct tagMenu setup_menu_spec;
extern const struct tagMenu setup_menu_bat;



void SETUPMode_Init(void);


BOOL SETUPMode_OnActivate(void);
BOOL SETUPMode_OnLeft(void);
BOOL SETUPMode_OnRight(void);
BOOL SETUPMode_OnUp(void);
BOOL SETUPMode_OnDown(void);
BOOL SETUPMode_OnIdle(void);
BOOL SETUPMode_OnShow(void);
BOOL SETUPMode_OnExit(void);
BOOL SETUPMode_OnPowerDown(void);
BOOL SETUPMode_OnWakeUp(void);
BOOL SETUPMode_OnTimer(void);
void SETUPMode_showModeScreen(void);
void SETUPMode_showDateTime(void);

const char* SETUPMode_NameOnUpdate(void);//"SETUP\0""ОПЦИИ",	//mode name
const char* SETUPMode_LeftOnUpdate(void);//"back\0""назад",	//left
const char* SETUPMode_RightOnUpdate(void);//"gps\0""gps",	//right
const char* SETUPMode_UpOnUpdate(void);//"acquir\0""набор",//up
const char* SETUPMode_DownOnUpdate(void);//"menu\0""меню",	//down


void SETUPMode_showVersion(int y);
void SETUPMode_showLibrary(void);
void SETUPMode_showMemory(void);
void SETUPMode_showSpecification(void);
void SETUPMode_showBatteryInfo(void);
void SETUPMode_showGeigerInfo(void);





//edit date time
BOOL SETUPMode_menu1_select_lib(void);
BOOL SETUPMode_menu1_IDthreshold(void);
BOOL SETUPMode_menu1_instability(void);
const char* SETUPMode_menu1_select_lib_onUpdate(void);
const char* SETUPMode_menu1_IDthreshold_onUpdate(void);
const char* SETUPMode_menu1_instability_onUpdate(void);
BOOL SETUPMode_menu1_del_spz(void);
BOOL SETUPMode_menu1_del_log(void);
BOOL SETUPMode_menu1_del_mcs(void);
BOOL SETUPMode_menu1_del_lib(void);
BOOL SETUPMode_menu1_hardreset(void);
const char* SETUPMode_menu1_del_spz_onUpdate(void);
const char* SETUPMode_menu1_del_log_onUpdate(void);
const char* SETUPMode_menu1_del_mcs_onUpdate(void);
const char* SETUPMode_menu1_del_lib_onUpdate(void);
//const char* SETUPMode_menu1_hardreset_onUpdate(void);
BOOL SETUPMode_menu1_defaults(void);
const char* SETUPMode_menu1_defaults_onUpdate(void);
void SETUPMode_menu1_defaults_confirm(BOOL bYes);
void SETUPMode_menu1_hardreset_confirm(BOOL bYes);
void SETUPMode_menu1_del_lib_confirm(BOOL bYes);
void SETUPMode_menu1_del_mcs_confirm(BOOL bYes);
void SETUPMode_menu1_del_log_confirm(BOOL bYes);
void SETUPMode_menu1_del_spz_confirm(BOOL bYes);


/*const char* SETUPMode_menu1_loader_onUpdate(void);
BOOL SETUPMode_menu1_loader(void);
void SETUPMode_menu1_loader_confirm(BOOL bYes);*/


BOOL SETUPMode_menu1_testscreen(void);
const char* SETUPMode_menu1_testscreen_onUpdate(void);

BOOL SETUPMode_menu1_enableAutoSave(void);
const char* SETUPMode_menu1_enableAutoSave_onUpdate(void);



void SETUPMode_menu1_IDthreshold_edit_done(BOOL bOK);
void SETUPMode_menu1_instability_edit_done(BOOL bOK);


BOOL SETUPMode_menu1_GMcalibrate(void);
BOOL SETUPMode_menu1_setCoef(void);
BOOL SETUPMode_menu1_setDT(void);
BOOL SETUPMode_menu1_setSF(void);
const char* SETUPMode_menu1_GMcalibrate_onUpdate(void);
const char* SETUPMode_menu1_setCoef_onUpdate(void);
const char* SETUPMode_menu1_setDT_onUpdate(void);
const char* SETUPMode_menu1_setSF_onUpdate(void);
void SETUPMode_menu1_setCoef_edit_done(BOOL bOK);
void SETUPMode_menu1_setDT_edit_done(BOOL bOK);
void SETUPMode_menu1_setSF_edit_done(BOOL bOK);
BOOL SETUPMode_menu1_idleTime(void);
void SETUPMode_menu1_idleTime_edit_done(BOOL bOK);
const char* SETUPMode_menu1_idleTime_onUpdate(void);

void SETUPMode_menu1_hardreset_confirm_done(BOOL bOK);

const char* SETUPMode_menu1_setSigmaOperation_onUpdate(void);
BOOL SETUPMode_menu1_setSigmaOperation(void);
void SETUPMode_menu1_setSigmaOperation_edit_done(BOOL bOK);
//const char* SETUPMode_menu1_setSigmaSleep_onUpdate(void);
//BOOL SETUPMode_menu1_setSigmaSleep(void);
//void SETUPMode_menu1_setSigmaSleep_edit_done(BOOL bOK);


void SETUPMode_menu1_select_lib_done(BOOL bOK);
BOOL SETUPMode_menu1_select_lib_onNextPage(void);
BOOL SETUPMode_menu1_select_lib_onPrevPage(void);
void SETUPMode_menu1_select_lib_done_err(BOOL bOK);

void SETUPMode_setModeOnSelf(void);


//BOOL SETUPMode_menu1_dimension(void);
//const char* SETUPMode_menu1_dimension_onUpdate(void);


/*BOOL SETUPMode_menu1_calcSleepSigma(void);
const char* SETUPMode_menu1_calcSleepSigma_onUpdate(void);
void SETUPMode_menu1_calcSleepSigma_edit_done(BOOL bOK);
*/

BOOL SETUPMode_menu1_calcOperSigma(void);
const char* SETUPMode_menu1_calcOperSigma_onUpdate(void);
void SETUPMode_menu1_calcOperSigma_edit_done(BOOL bOK);
void SETUPMode_calcSigma(UINT period);
float SETUPMode_calcSigmaEx(UINT period, float dwell);



BOOL SETUPMode_menu1_editDTCOEF(void);
const char* SETUPMode_menu1_editDTCOEF_onUpdate(void);
void SETUPMode_menu1_editDTCOEF_edit_done(BOOL bOK);


///for test with sound
/*
BOOL SETUPMode_menu1_recordSound(void);
const char* SETUPMode_menu1_recordSound_onUpdate(void);
BOOL SETUPMode_menu1_playSound(void);
const char* SETUPMode_menu1_playSound_onUpdate(void);
BOOL SETUPMode_menu1_stopRecord(void);
const char* SETUPMode_menu1_stopRecord_onUpdate(void);
void SETUPMode_display_soundData(void);*/
/////////////////

void SETUPMode_PSW_done(BOOL bOK);


BOOL SETUPMode_menu1_identtime(void);
const char* SETUPMode_menu1_identtime_onUpdate(void);
void SETUPMode_menu1_identtime_done(BOOL bOK);

BOOL SETUPMode_menu1_SafetyThreshold(void);
const char* SETUPMode_menu1_SafetyThreshold_onUpdate(void);
void SETUPMode_menu1_SafetyThreshold_done(BOOL bOK);


const char* SETUPMode_menu1_del_autospz_onUpdate(void);
void SETUPMode_menu1_del_autospz_confirm(BOOL bYes);
BOOL SETUPMode_menu1_del_autospz(void);


void SETUPMode_pleaseWait(int progress);

/*
BOOL SETUPMode_menu1_placeMarkers(void);
const char* SETUPMode_menu1_placeMarkers_onUpdate(void);
*/


BOOL SETUPMode_clear_memory(void);
void SETUPMode_clear_memory_confirm(BOOL bYes);

void SETUPMode_clearBuffers(void);

BOOL SETUPMode_menu1_batcoef(void);
void SETUPMode_menu1_batcoef_edit_done(BOOL bOK);
const char* SETUPMode_menu1_batcoef_onUpdate(void);


BOOL SETUPMode_menu1_prirabotka(void);
const char* SETUPMode_menu1_prirabotka_onUpdate(void);
void SETUPMode_prirabotka(BOOL bYes);

/* //to get splash screens
BOOL SETUPMode_menu1_showSplash(void);
const char* SETUPMode_menu1_showSplash_onUpdate(void);
*/
void SETUPMode_accessGranted(void);
void SETUPMode_accessDenied(void);

const char* SETUPMode_menu1_enableDataOrder_onUpdate(void);
BOOL SETUPMode_menu1_enableDataOrder(void);


#endif	//ifndef _SETUPMODE_H
