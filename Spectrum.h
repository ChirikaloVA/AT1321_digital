#ifndef _SPECTRUM_H
#define _SPECTRUM_H

#include "types.h"
#include "modes.h"

//spectrum window parameters
#define SPECTRUM_WIN_HEIGHT 164
#define SPECTRUM_WIN_WIDTH X_SCREEN_SIZE
#define SPECTRUM_WIN_BOTTOM Y_SCREEN_MAX-50
#define SPECTRUM_WIN_TOP SPECTRUM_WIN_BOTTOM-SPECTRUM_WIN_HEIGHT+1
#define SPECTRUM_WIN_LEFT 0




#define OPER_SPECTRUM_NUMBER (int)250
#define CALIBR_SPECTRUM_NUMBER (int)10
#define CALIBR_SPECTRUM_FROM (int)2090
#define CALIBR_SPECTRUM_TO (int)2099
#define OPER_SPECTRUM_FROM (int)0
#define OPER_SPECTRUM_TO (OPER_SPECTRUM_NUMBER-1)
#define SPEC_ITEM_CNT OPER_SPECTRUM_NUMBER

//len of spectrum data + extra data
//количество каналов + доп данные
#define SLEN 3328

void Spectrum_showSpectrum(void);



#define MAX_ACQ_TIME 64800






enum ENU_SPEC_CTRL_MODE
{
	enum_scm_marker_movment,
	enum_scm_spec_acquire,
	enum_scm_spec_wider,
	enum_scm_spec_higher,
	enum_scm_spec_movment,
	enum_scm_spec_window,
	enum_scm_spec_view,
	MAX_SPEC_CTRL_MODES
};

struct tagSpectrum
{
	WORD wAcqTime;	//spectrum acqruring time
	DWORD dwarSpectrum[CHANNELS];
	DWORD dwCount;	//common count in spectrum
	struct tagDateTime dateTime;
	struct tagCommonGPS commonGPS;
	float fTemperature;
	char report[MAX_REPORT_SYMS];
	WORD wRealTime;	//spectrum acqruring real time
	float fDoserate;	//doserate
	float fCps;	//cps
};

//количество окон для расчета спектр дозы
#define SD_WIN_SIZE 17


struct tagSpectrumControl
{	
	//таблица канал энергия
	struct tagIndexMeanTable warChEnTable[MAX_TABLE_RECORDS];
	//таблица канал сигма
	struct tagIndexMeanTable warChSiTable[MAX_TABLE_RECORDS];
	
	
	BOOL bSpectrumInDots;	//=1 spectrum in dots, 0-in lines
	int iMarkerChannel;	//channel of marker
	int iMarkerChannel2;	//channel of marker, if -1 then only one marker
	BOOL bHasEnergy;
	WORD warEnergy[CHANNELS];
	BOOL bHasSigma;
	WORD warSigma[CHANNELS];	//здесь хранится сигма в 1,5 уже чем калибровочная, это нужно для поиска слитых пиков. во всех остальных местах кроме идентификации нажуно множить ее на 1,5

	int iAcquiringTime;
	
	struct tagSpectrum acqSpectrum;
	struct tagSpectrum opnSpectrum;
	
	struct tagSpectrum* pShowSpectrum;	//pointer to spectrum to show
	
//	int specTablePage;	//страница отображения таблицы спектров
//	BOOL specTableChanged;
//	BYTE barOperData[256];
//	BOOL hasSpecTable;	//if 1 then spec table we have

//	const struct tagMode* pCurMode;//used to return from edit functions
	
	int iSpectrumControlMode;	//mode of spectrum control by keyboard
	
	//channels of spectrum border
	int iViewChannelFrom;
	int iViewChannelTo;
	
	int iHighMul;	//coef of make spectrum higher
	
	char peakProcRes[100];	//report of peak processing
	
	WORD wins1[SD_WIN_SIZE+1];
	BOOL bLeftRightClicked;	
	
	int iStopAcq;	//counter to stop acquiring after status shows that acquiring stoped
	BOOL bStopAcq;
	
	BOOL bLogView;	//true for log view
	
	BOOL bSpectrumEmergSaved;	//true if spectrum already saved in emergency mode (battery low)
};

extern struct tagSpectrumControl spectrumControl;

extern const struct tagMode modes_SpectrumMode;
extern const struct tagMode modes_RID_PSW_Mode;

extern const struct tagMenu spectrum_menu;





extern const WORD spectrumDoserateEnergyWin[SD_WIN_SIZE];









void Spectrum_Init(void);


BOOL Spectrum_OnActivate(void);
BOOL Spectrum_OnLeft(void);
BOOL Spectrum_OnRight(void);
BOOL Spectrum_OnUp(void);
BOOL Spectrum_OnDown(void);
BOOL Spectrum_OnIdle(void);
BOOL Spectrum_OnShow(void);
BOOL Spectrum_OnExit(void);
BOOL Spectrum_OnPowerDown(void);
BOOL Spectrum_OnWakeUp(void);
BOOL Spectrum_OnTimer(void);
void Spectrum_showAcqData(void);
void Spectrum_showModeScreen(void);
void Spectrum_showMarker(void);

BOOL Spectrum_menu1_acqtime(void);
BOOL Spectrum_menu1_savespectrum(void);
BOOL Spectrum_menu1_openspectrum(void);
BOOL Spectrum_menu1_gaincode(void);
void Spectrum_peakProc(void);
BOOL Spectrum_menu1_secondmarker(void);
BOOL Spectrum_menu1_6(void);
BOOL Spectrum_menu1_7(void);

const char* Spectrum_menu1_acqtime_onUpdate(void);
const char* Spectrum_menu1_savespectrum_onUpdate(void);
const char* Spectrum_menu1_openspectrum_onUpdate(void);
const char* Spectrum_menu1_gaincode_onUpdate(void);
const char* Spectrum_menu1_secondmarker_onUpdate(void);
const char* Spectrum_menu1_6_onUpdate(void);
const char* Spectrum_menu1_7_onUpdate(void);

void Spectrum_showMarkerData(void);


BOOL Spectrum_menu1_easyMode(void);
const char* Spectrum_menu1_easyMode_onUpdate(void);


const char* Spectrum_NameOnUpdate(void);
const char* Spectrum_LeftOnUpdate(void);
const char* Spectrum_RightOnUpdate(void);
const char* Spectrum_UpOnUpdate(void);
const char* Spectrum_DownOnUpdate(void);

int Spectrum_save(char* pFileName, BOOL bOverwrite);
int Spectrum_open(char* pFileName);
void Spectrum_calcCount(void);
void Spectrum_startAcq(void);
void Spectrum_startAcq_ex(int acqTime);

int Spectrum_read_sigma_cal(void);
int Spectrum_read_energy_cal(void);
int Spectrum_open_ex(HFILE hfile);

void Spectrum_menu1_acqtime_edit_done(BOOL bOK);
void Spectrum_menu1_gaincode_edit_done(BOOL bOK);

int Spectrum_retrieveTable(HFILE hfile, struct tagIndexMeanTable*  pTable);
void Spectrum_showID(void);
void Spectrum_menu1_openspectrum_done(BOOL bOK);
void Spectrum_menu1_savespectrum_done(BOOL bOK);


void Spectrum_showPeakProcResult(void);

BOOL Spectrum_QuickPeakCalculation(long left, long right, float *position, float *sigma);
BOOL Spectrum_peakProc_ex(float *position, float* sigma);
void Spectrum_setupDoseWindowTable(void);
void Spectrum_makeEnergyWins(void);

void Spectrum_clear(void);


BOOL Spectrum_menu1_openspectrum_onNextPage(void);
BOOL Spectrum_menu1_openspectrum_onPrevPage(void);
void Spectrum_menu1_openspectrum_done(BOOL bOK);







const char* RID_PSW_LeftOnUpdate(void);//"marker\rleft\0""маркер\rвлево",
const char* RID_PSW_RightOnUpdate(void);//"marker\rright\0""маркер\rвправо",


//режимы работы со спектром
//отображаемое слово и есть текущий режим
const char* RID_PSW_UpOnUpdate(void);

const char* RID_PSW_DownOnUpdate(void);//"menu\0""меню",


BOOL RID_PSW_OnActivate(void);
BOOL RID_PSW_OnLeft(void);
BOOL RID_PSW_OnRight(void);
BOOL RID_PSW_OnUp(void);
BOOL RID_PSW_OnDown(void);

BOOL RID_PSW_OnShow(void);
BOOL RID_PSW_OnPowerDown(void);


//задать пороги
BOOL Spectrum_menu1_lowlimit(void);
void Spectrum_menu1_lowlimit_edit_done(BOOL bOK);
BOOL Spectrum_menu1_highlimit(void);
void Spectrum_menu1_highlimit_edit_done(BOOL bOK);
const char* Spectrum_menu1_lowlimit_onUpdate(void);
const char* Spectrum_menu1_highlimit_onUpdate(void);


BOOL Spectrum_menu1_deletespec(void);
const char* Spectrum_menu1_deletespec_onUpdate(void);

void Spectrum_showMessageOnFileNamechange(char* pFileName);

void Spectrum_showMessageOnFileNamechange_onDraw(const void* pParam);
void Spectrum_showMessageOnFileNamechange_onExit(void);


#endif	//ifndef _SPECTRUM_H
