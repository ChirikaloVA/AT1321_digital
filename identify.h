//identify.h

#ifndef _IDENTIFY_H
#define _IDENTIFY_H


#include "types.h"
#include "filesystem.h"


//вести расчет обогащения
#define _URANIUM_CALC


#define BADINDEX 255
#define MAXLINENUM 255


#define GSHPNUM 512
#define SRCHGSHPNUM 800
#define GSHPSIGM 150
#define GSHPSIGMx256 38400
#define GSHPSIGMx256x256 9830400




#define _NUMCHAN CHANNELS
//#define SPECNUMCHAN (4*_NUMCHAN/3)
#define ARRCHAN _NUMCHAN
#define SPECARRCHAN ARRCHAN
//#define SPECARRCHAN SPECNUMCHAN
#define MAXLINES 16	
#define NUMNUCLIDES 40	
#define NUCLNAMELENGTH 5

#define ENERGYTHRESHOLD 250
#define ENERGYTHRESHOLD2 30
#define MINWEIGHT 50


//разбили фактор на две части для елочисленных вычислений
//для 1,5
#define SIGMA_THIN_FACTOR_M 3
#define SIGMA_THIN_FACTOR_D 2


#define pEnergy spectrumControl.warEnergy
#define SigmaArray spectrumControl.warSigma



extern const char identify_ini[247];
extern const char main_lib
#ifdef _SNM
[6061]
#else
[4667]
#endif	//_SNM
;





//#pragma pack(1)

typedef struct{
	
	char Name[NUCLNAMELENGTH];
	char NumStr[NUCLNAMELENGTH];
	char category;
	CHAR selected;
	UCHAR num;
	short weight,weightM;
	short	energies[MAXLINES];
	USHORT  factors_noshield[MAXLINES],
			factors_shield[MAXLINES];
	
/*
#ifdef _SPECTRUM_ACTIVITY
	USHORT	quantumYield[MAXLINES]; //квантовый выход в долях * 32000
	BYTE	correction[MAXLINES]; //квантовый выход в долях * 128, =0 использование в качестве опорной для расчета толщины защиты
	float activity[MAXLINES];
	BYTE activityError[MAXLINES];
#endif	//_SPECTRUM_ACTIVITY
*/
	
	short closeness[MAXLINES];
	UCHAR indexes[MAXLINES];	//indeces of found lines for current nuclide, it can be tooked in acount only if ex_energy[index]=1
	
	BYTE confidence;	//confidence level of the nuclide
	
} TNucl,*PNucl;

//#pragma pack()




struct tagIdentify
{

	BOOL bHasLibrary;
	
	long threshold;	//порог идентификации
	
	char libraryFileName[FILE_NAME_SZ];	//file name of library
	
	int  NUCLNUM;
	
	int  LEFTBORDER;
		
	int  NUMCHAN;
	
	int  _nuclsDiffs;
	
	int  MAXENERGY;
	
	int  MINENERGY;
	int  nsigma_searchpeaks;
	
	UINT  SCALEINSTABILITY;
	float fSCALEINSTABILITY;
	
	
	float uranium;//степень обогащения урана
	
	long nsigma_peakcheck_left;
	long nsigma_peakcheck_right;
	long msigma_peakcheck;
		
	BOOL testNullPeak;	//признак необходимости проверки пиков с нулями в области
		
	long*  ISpectrum;
	long  BufSpec[ARRCHAN];
	long  BufSpec1[ARRCHAN];
	long  SDs[ARRCHAN];
//	char  DetBuf[ARRCHAN];
	
	int dwLibVer;	//version of library
	TNucl  Nucls[NUMNUCLIDES];
	
	
	long CHANB, CHANK;
	
	short energies[MAXLINENUM];//array of energy of found lines
	short deltas[MAXLINENUM];//array of ROI width of found lines
	float ni_channels[MAXLINENUM];	//array of channels of found lines
	
//	WORD  SigmaArray[ARRCHAN];
		
	
	char  ex_energy[MAXLINENUM];	//array of detected status of lines, 1 - line is found
	
	BYTE nLine;
	
//	long  *varspec;
	ULONG collectionTime;
	
	
	char report[MAX_REPORT_SYMS];
	
	int iNuclsIdentified;
	BOOL bHaveAlreadyResult;	//TRUE if we already found some nuclides in one stage of ID (from start to end if ID mode)
	int identifyDeadTime;	//current time to stop identify if no nuclides
	int identifyStartDeadTime;	//start time to stop identify if no nuclides
	BOOL bHaveUnknownResult;	//true if some unknown nuclides
	
};





extern struct tagIdentify identifyControl;







extern const unsigned char identify_GaussShape[GSHPNUM];
extern const CHAR identify_SearchShape[SRCHGSHPNUM];










void identify_InitIdent(void);
int identify_ChannelFromEnergyNear(int energy);
void identify_InitSigma1();
//void identify_InitSigma();
UINT identify_GetSigmaFromEnergy(int energy);
//short ChannelFromEnergy(short energy);
void identify_DetectLines(void);
void identify_ApplyFilter();
void identify_Smoothing();

//void identify_DetectLinesEx(int threshold,long  *variance);
void identify_AnalyzePeaks(void);

int identify_MakeNuclideIdentification(void);
int identify_testForLostPeak(TNucl* Nuc);
void identify_testNoStrongLines(TNucl* Nuc);
void identify_getRightEnergy();
void identify_nucInit(TNucl* Nuc);

//UCHAR identify_GShape(short j, WORD *sigma);
void identify_ApplyFilterEx(long  *Variance); //The same as ApplyFilter, but spectrum variance is in separate array

BOOL identify_findOtherStrong(int en);

void identify_excludeLines(TNucl* Nuc);

int identify_checkWeight(TNucl* Nuc);


void identify_SmoothingEx(long * pBuf, int sigmas);


//char identify_SShape(short j, WORD *sigma);


//void MultLongByte(unsigned char d, long *ps, unsigned long *res);

//void ZeroLONGLONG(LONGLONGG *v);

//void AddLongLong(const LONGLONGG  *v, LONGLONGG  *vl);

//void InvertLONGLONG(LONGLONGG  *v);

//float LongLongToFloat(LONGLONGG  *v);

//char IsGZeroLONGLONG(LONGLONGG *v);

//void Shift16BitRight(LONGLONGG *v);

//void MultLongLongByte(const char d, LONGLONGG  *vl, LONGLONGG  *res);

int identify_CheckPeak(int jm, int j, int i);

UINT identify_NuclsDiffer(TNucl *Nuc, TNucl *NucMain); //returns non zero, if all lines of nuclides differ, or NucMain->weightM/Nuc->weightM > 2
UINT identify_NuclsDifferOnly(TNucl *Nuc, TNucl *NucMain); //returns non zero, if all lines of nuclides differ, or NucMain->weightM/Nuc->weightM > 2

int identify_CalcDeltaEnergyFromChannel(int l);

int identify_CheckChannel(int E); //searches position of maximum near given channel (l+-SCALEINSTABILITY*l)

//ULONG MultFIXED4(ULONG *n1, ULONG *n2);

UINT identify_GetSigmaFromEnergy(int energy);

//void identify_calcNucActivity(TNucl * Nuc, unsigned char btInd, float chan);


float identify_EnergyFromChannel(float chan);

void identify_identify(BOOL bAddCategory);
int identify_open_library(void);
int identify_read_identify_ini(void);


void identify_getnuclidesinreport(char sym, const char* pHeader, BOOL bSimplify);
void identify_prepareReport(BOOL bAddCategory);
void identify_clearReport(void);
void identify_clearReportEx(void);
BOOL identify_write_identify_ini_int(const char * pSection, const char* pValueName, int value);

BOOL identify_write_identify_ini_string(const char * pSection, const char* pValueName, const char* value);

void identify_calcConfidence(void);

BOOL identify_convolute(void);

void identify_testForPuBa(TNucl* thisnuc);

void identify_checkForUnknown(void);
int identify_getnuclidetxt(int i, char* pstrNuc);

#ifdef _URANIUM_CALC
void identify_calcUranium(void);
#endif	//#ifdef _URANIUM_CALC


#endif	//#ifndef _IDENTIFY_H
