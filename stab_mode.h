#ifndef _STAB_H
#define _STAB_H

#include "types.h"
#include "modes.h"



//количество импульсов по спектру дл€ стабилизации
#define STABIL_CNT 30000.0	

#define BASE_STAB_ERROR	(float)0.006
#define BASE_STAB_ERROR_MIN	(float)0.003
#define BASE_PEAK_THRESHOLD	(float)3.5


//не мен€ть перечислений пор€док!!! есть зависимость от STB_END и далее
enum TSTABSTEPS{STB_WAITFORSETPEAK,/*STB_WAITFORSPECTRUM2, */STB_WAITFORSPECTRUM,STB_STABILIZATION,/*STB_CHECK,*/STB_BEGIN,/*STB_WAITFORSETPEAK2,*/STB_WAIT_STAB,STB_END,STB_ERROR, STB_DISABLED/*,STB_ENDFAIL,STB_WAITFORWARMING*/};

struct tagSTABModeControl
{
	enum TSTABSTEPS stabStep;	//этап стабилизации
	BOOL bGetGain;	//if true then gain is already saved
	BOOL bByCs137;	//if true then stab by Cs137
	DWORD cnt_max;	//max count to process spectrum
	float peakThreshold;	//порог определени€ пика
	float stab_error;
	float stab_error_cur;
	float currentPeakPos;
	float stabilPeakCenter;	//канал стабилизации
	float peakFactor;
	float fStabGainSaved;
};

extern struct tagSTABModeControl STABModeControl;

extern const struct tagMode modes_STABMode;

void STABMode_Init(void);

BOOL STABMode_OnActivate(void);
BOOL STABMode_OnLeft(void);
BOOL STABMode_OnRight(void);
BOOL STABMode_OnUp(void);
BOOL STABMode_OnDown(void);
BOOL STABMode_OnIdle(void);
BOOL STABMode_OnShow(void);
BOOL STABMode_OnExit(void);
BOOL STABMode_OnPowerDown(void);
BOOL STABMode_OnWakeUp(void);
BOOL STABMode_OnTimer(void);
void STABMode_showModeScreen(void);

const char* STABMode_NameOnUpdate(void);//"COMM\0""—¬я«№",	//mode name
const char* STABMode_LeftOnUpdate(void);//"back\0""назад",	//left
const char* STABMode_RightOnUpdate(void);//"comm\0""обмен",	//right
const char* STABMode_UpOnUpdate(void);//"on\0""вкл",//up
const char* STABMode_DownOnUpdate(void);//"menu\0""меню",	//down

BYTE STABMode_getPeakEx(void);
void STABMode_getPeakPrec(short cleft, short cright, float max);
void STABMode_proccessPeak(void);


void STABMode_updatePeakCenter(void);






#endif	//ifndef _STAB_H
