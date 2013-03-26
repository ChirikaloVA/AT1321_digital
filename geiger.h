//geiger.h

#ifndef _GEIGER_H
#define _GEIGER_H

#include "types.h"


struct tagGeigerEsentValues
{
	float fCps;
	float fDoserate;
	float fCpsErr;
	float fSKO;
	BOOL bOverload;
	BOOL bSafetyAlarm;	//1-safety alarm, 0-no alarm
};


#define MAX_COUNT_ITEMS 9

struct tagGeigerControl
{
	volatile DWORD dwCount;
	volatile DWORD dwTotalCount;	//total count form the spirid start time
	volatile DWORD dwMomCount;
	volatile DWORD dwMomCountCopy;
	volatile DWORD dwTime;	//number of averaged values
	volatile DWORD dwTotalTime;	//number of measured values from the spirid start time
	volatile struct tagGeigerEsentValues esentVals;
	volatile struct tagGeigerEsentValues esentVals_safe;
	volatile DWORD countBuf[MAX_COUNT_ITEMS];
	volatile DWORD dwCountBkg;
	//need to save three values in INI
	float fDrCoef;
	float fDrSelfCps;
	float fDrDeadTime;
	volatile BOOL bReset;

	//следующие значения нужны для однократного сообщения в журнале, если они false при аларме, то идет запись
	BOOL bSafetyAlarm_log;	//1-safety alarm, 0-no alarm
	BOOL bOverload_log;

//	DWORD dwReadStatus;	//counter of read data from structure
//	DWORD dwWriteStatus;	//counter of write data to structure
};

extern struct tagGeigerControl geigerControl;


__arm void _INT_Geiger(void);
void Geiger_Init(void);
void Geiger_GetCount_intcall(void);
void Geiger_values_Init(void);
void Geiger_INT_Init(void);

void Geiger_processData(void);

void Geiger_copyDataToSafePlace(void);
void Geiger_control(void);

#endif	//#ifndef _GEIGER_H
