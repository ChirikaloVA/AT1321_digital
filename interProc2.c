//interProc2.c


#include <iolpc2388.h>

#include "types.h"
#include "interProc.h"
#include "spectrum.h"
#include "SPRD_Mode.h"
#include "powerControl.h"


//rsmodbus commands
const BYTE arCmd_GetStatus[]={1,7};
//const BYTE arCmd_GetControlRegister_simgaThresholds[]={1,3,0,7,0,2};
const BYTE arCmd_GetControlRegister_presetAcqTime[]={1,3,0,0,0,1};
const BYTE arCmd_GetControlRegister_gain[]={1,3,0,22,0,2};
const BYTE arCmd_GetControlRegister_lowLimit[]={1,3,0,3,0,1};
const BYTE arCmd_GetControlRegister_highLimit[]={1,3,0,4,0,1};

//const BYTE arCmd_GetControlRegister_bkgDR[]={1,3,0,0x11,0,2};
const BYTE arCmd_GetControlRegister_bkgCPS[]={1,3,0,0x0b,0,2};

const BYTE arCmd_GetControlRegister_curPeak[]={1,3,0,24,0,2};
const BYTE arCmd_GetControlRegister_stabPeak[]={1,3,0,26,0,2};
const BYTE arCmd_GetDataRegister_acqTime[]={1,4,0,0,0,2};
const BYTE arCmd_GetDataRegister_MeasurementRegs[]={1,4,0,4,0,13};
const BYTE arCmd_GetDataRegister_temperature[]={1,4,0,2,0,1};
const BYTE arCmd_GetDiagnostic[]={1,8,0,2,0,0};
const BYTE arCmd_Command_ResetSpectrum[]={1,5,0,0,0xff,0};
const BYTE arCmd_Command_StartAcq[]={1,5,0,1,0xff,0};
const BYTE arCmd_Command_StopAcq[]={1,5,0,1,0x00,0};
const BYTE arCmd_GetSpectrumPart1[]={1,0xb,4,0,0,0,0xf0};
const BYTE arCmd_GetSpectrumPart2[]={1,0xb,4,0,0xf0,0,0xf0};
const BYTE arCmd_GetSpectrumPart3[]={1,0xb,4,1,0xe0,0,0xf0};
const BYTE arCmd_GetSpectrumPart4[]={1,0xb,4,2,0xd0,0,0xf0};
const BYTE arCmd_GetSpectrumPart5[]={1,0xb,4,3,0xc0,0,0xf0};
const BYTE arCmd_GetSpectrumPart6[]={1,0xb,4,4,0xb0,0,0xf0};
const BYTE arCmd_GetSpectrumPart7[]={1,0xb,4,5,0xa0,0,0xf0};
const BYTE arCmd_GetSpectrumPart8[]={1,0xb,4,6,0x90,0,0xf0};
const BYTE arCmd_GetSpectrumPart9[]={1,0xb,4,7,0x80,0,0xf0};
const BYTE arCmd_GetSpectrumPart10[]={1,0xb,4,8,0x70,0,0xf0};
const BYTE arCmd_GetSpectrumPart11[]={1,0xb,4,9,0x60,0,0xf0};
const BYTE arCmd_GetSpectrumPart12[]={1,0xb,4,0xa,0x50,0,0xf0};
const BYTE arCmd_GetSpectrumPart13[]={1,0xb,4,0xb,0x40,0,0xc0};
const BYTE arCmd_Command_setMeasurementMode[]={1,5,0,0x24,0,0};
const BYTE arCmd_Command_setSearchMode[]={1,5,0,0x24,0xff,0};
const BYTE arCmd_Command_resetAveraging[]={1,5,0,0x22,0xff,0};
const BYTE arCmd_Command_resetDose[]={1,5,0,0x23,0xff,0};
const BYTE arCmd_Command_makeCpsBaseForSearch[]={1,5,0,0x25,0xff,0};
const BYTE arCmd_Command_makeCpsAsBkg[]={1,5,0,0x27,0xff,0};
const BYTE arCmd_Command_turnonSubtrackBkg[]={1,5,0,0x26,0xff,0};
const BYTE arCmd_Command_turnoffSubtrackBkg[]={1,5,0,0x26,0,0};
const BYTE arCmd_Command_turnoffTuning[]={1,5,0,7,0,0};
const BYTE arCmd_Command_turnonTuning[]={1,5,0,7,0xff,0};
const BYTE arCmd_Command_stabilize[]={1,5,0,0x52,0xff,0};
const BYTE arCmd_Command_goSleep[]={1,5,0,3,0xff,0};
const BYTE arCmd_Command_goAwake[]={1,5,0,3,0,0};











void InterProc_readTemperature(void)
{
	if(InterProc_isReadyToTransmit(&interProcControl.rsModbus.swdTemperature))
	{
		InterProc_fillNewCmd(arCmd_GetDataRegister_temperature, sizeof(arCmd_GetDataRegister_temperature));
	}
}


void InterProc_readStatus(void)
{
	if(InterProc_isReadyToTransmit(&interProcControl.rsModbus.sbtStatus))
	{
		InterProc_fillNewCmd(arCmd_GetStatus, sizeof(arCmd_GetStatus));
	}
}


void InterProc_readDiagnostic(void)
{
	if(InterProc_isReadyToTransmit(&interProcControl.rsModbus.swdDiag))
	{
		InterProc_fillNewCmd(arCmd_GetDiagnostic, sizeof(arCmd_GetDiagnostic));
	}
}


//always set and check interProcControl.rsModbus.fStabGain before call it
void InterProc_setGain(void)
{
	float value = interProcControl.rsModbus.fStabGain;
	BYTE ar[]={1,0x10,8,0,22,0,2,*((unsigned char*)&value+3),*((unsigned char*)&value+2),*((unsigned char*)&value+1),*((unsigned char*)&value+0)};
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
}



void InterProc_setCurPeak(float value)
{
	BYTE ar[]={1,0x10,8,0,24,0,2,*((unsigned char*)&value+3),*((unsigned char*)&value+2),*((unsigned char*)&value+1),*((unsigned char*)&value+0)};
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
}
void InterProc_setStabPeak(float value)
{
	BYTE ar[]={1,0x10,8,0,26,0,2,*((unsigned char*)&value+3),*((unsigned char*)&value+2),*((unsigned char*)&value+1),*((unsigned char*)&value+0)};
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
}



void InterProc_setSigmaSearchThresholds(void)
{
	float value;
	BYTE ar[]={1,6,0,0,0,0};
	value = SPRDModeControl.operation_search_sigma[0];
	ar[3]=0x28;
	ar[4]=*((unsigned char*)&value+3);
	ar[5]=*((unsigned char*)&value+2);
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
	ar[3]=0x29;
	ar[4]=*((unsigned char*)&value+1);
	ar[5]=*((unsigned char*)&value+0);
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
	
	value = SPRDModeControl.operation_search_sigma[1];
	ar[3]=0x2a;
	ar[4]=*((unsigned char*)&value+3);
	ar[5]=*((unsigned char*)&value+2);
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
	ar[3]=0x2b;
	ar[4]=*((unsigned char*)&value+1);
	ar[5]=*((unsigned char*)&value+0);
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
	
	value = SPRDModeControl.operation_search_sigma[2];
	ar[3]=0x2c;
	ar[4]=*((unsigned char*)&value+3);
	ar[5]=*((unsigned char*)&value+2);
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
	ar[3]=0x2d;
	ar[4]=*((unsigned char*)&value+1);
	ar[5]=*((unsigned char*)&value+0);
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
	
	value = SPRDModeControl.operation_search_sigma[3];
	ar[3]=0x2e;
	ar[4]=*((unsigned char*)&value+3);
	ar[5]=*((unsigned char*)&value+2);
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
	ar[3]=0x2f;
	ar[4]=*((unsigned char*)&value+1);
	ar[5]=*((unsigned char*)&value+0);
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
	
	value = SPRDModeControl.operation_search_sigma[4];
	ar[3]=0x30;
	ar[4]=*((unsigned char*)&value+3);
	ar[5]=*((unsigned char*)&value+2);
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
	ar[3]=0x31;
	ar[4]=*((unsigned char*)&value+1);
	ar[5]=*((unsigned char*)&value+0);
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
	
	value = SPRDModeControl.operation_search_sigma[5];
	ar[3]=0x32;
	ar[4]=*((unsigned char*)&value+3);
	ar[5]=*((unsigned char*)&value+2);
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
	ar[3]=0x33;
	ar[4]=*((unsigned char*)&value+1);
	ar[5]=*((unsigned char*)&value+0);
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));

	PowerControl_sendAllCommands();

}

/*void InterProc_setSigmaSleepThresholds(float value)
{
	BYTE ar[]={1,6,0,9,*((unsigned char*)&value+3),*((unsigned char*)&value+2)};
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
	BYTE ar2[]={1,6,0,10,*((unsigned char*)&value+1),*((unsigned char*)&value+0)};
	InterProc_fillNewCmd((const BYTE*)ar2, sizeof(ar2));
}*/

void InterProc_setLowLimit(WORD val)
{
	BYTE ar[]={1,6,0,3,LO2BYTE(val),LOBYTE(val)};
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
}


void InterProc_setHighLimit(WORD val)
{
	BYTE ar[]={1,6,0,4,LO2BYTE(val),LOBYTE(val)};
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
}

void InterProc_startSpectrumAcq(void)
{
	InterProc_fillNewCmd(arCmd_Command_StartAcq, sizeof(arCmd_Command_StartAcq));
}

void InterProc_stopSpectrumAcq(void)
{
	InterProc_fillNewCmd(arCmd_Command_StopAcq, sizeof(arCmd_Command_StopAcq));
}

void InterProc_resetSpectrum(void)
{
	InterProc_fillNewCmd(arCmd_Command_ResetSpectrum, sizeof(arCmd_Command_ResetSpectrum));
}

void InterProc_setAcqTime(WORD val)
{
	if(val==0)
		val = MAX_ACQ_TIME;
	BYTE ar[]={1,6,0,1,LO2BYTE(val),LOBYTE(val)};
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
}

void InterProc_readAcqTime(void)
{
	if(InterProc_isReadyToTransmit(&interProcControl.rsModbus.swdAcqTime))
	{
		InterProc_fillNewCmd(arCmd_GetDataRegister_acqTime, sizeof(arCmd_GetDataRegister_acqTime));
	}
}

void InterProc_readMeasurementRegs(void)
{
	if(InterProc_isReadyToTransmit(&interProcControl.rsModbus.swdMeasurementRegs))
	{
		InterProc_fillNewCmd(arCmd_GetDataRegister_MeasurementRegs, sizeof(arCmd_GetDataRegister_MeasurementRegs));	//arCmd_GetDataRegister_momCps
	}
}

void InterProc_readSpectrum(void)
{
	if(InterProc_isReadyToTransmit(&interProcControl.rsModbus.sarSpectrum))
	{
		InterProc_fillNewCmd(arCmd_GetSpectrumPart1, sizeof(arCmd_GetSpectrumPart1));
		InterProc_fillNewCmd(arCmd_GetSpectrumPart2, sizeof(arCmd_GetSpectrumPart2));
		InterProc_fillNewCmd(arCmd_GetSpectrumPart3, sizeof(arCmd_GetSpectrumPart3));
		InterProc_fillNewCmd(arCmd_GetSpectrumPart4, sizeof(arCmd_GetSpectrumPart4));
		InterProc_fillNewCmd(arCmd_GetSpectrumPart5, sizeof(arCmd_GetSpectrumPart5));
		InterProc_fillNewCmd(arCmd_GetSpectrumPart6, sizeof(arCmd_GetSpectrumPart6));
		InterProc_fillNewCmd(arCmd_GetSpectrumPart7, sizeof(arCmd_GetSpectrumPart7));
		InterProc_fillNewCmd(arCmd_GetSpectrumPart8, sizeof(arCmd_GetSpectrumPart8));
		InterProc_fillNewCmd(arCmd_GetSpectrumPart9, sizeof(arCmd_GetSpectrumPart9));
		InterProc_fillNewCmd(arCmd_GetSpectrumPart10, sizeof(arCmd_GetSpectrumPart10));
		InterProc_fillNewCmd(arCmd_GetSpectrumPart11, sizeof(arCmd_GetSpectrumPart11));
		InterProc_fillNewCmd(arCmd_GetSpectrumPart12, sizeof(arCmd_GetSpectrumPart12));
		InterProc_fillNewCmd(arCmd_GetSpectrumPart13, sizeof(arCmd_GetSpectrumPart13));
	}
}


void InterProc_setMeasurementMode(void)
{
	InterProc_fillNewCmd(arCmd_Command_setMeasurementMode, sizeof(arCmd_Command_setMeasurementMode));
}


void InterProc_setSearchMode(void)
{
	InterProc_fillNewCmd(arCmd_Command_setSearchMode, sizeof(arCmd_Command_setSearchMode));
}

void InterProc_resetAveraging(void)
{
	InterProc_fillNewCmd(arCmd_Command_resetAveraging, sizeof(arCmd_Command_resetAveraging));
}

void InterProc_resetDose(void)
{
	InterProc_fillNewCmd(arCmd_Command_resetDose, sizeof(arCmd_Command_resetDose));
}

void InterProc_makeCpsBaseForSearch(void)
{
	InterProc_fillNewCmd(arCmd_Command_makeCpsBaseForSearch, sizeof(arCmd_Command_makeCpsBaseForSearch));
}

void InterProc_makeCpsAsBkg(void)
{
	InterProc_fillNewCmd(arCmd_Command_makeCpsAsBkg, sizeof(arCmd_Command_makeCpsAsBkg));
}

void InterProc_turnonSubtrackBkg(void)
{
	InterProc_fillNewCmd(arCmd_Command_turnonSubtrackBkg, sizeof(arCmd_Command_turnonSubtrackBkg));
}

void InterProc_turnoffSubtrackBkg(void)
{
	InterProc_fillNewCmd(arCmd_Command_turnoffSubtrackBkg, sizeof(arCmd_Command_turnoffSubtrackBkg));
}

void InterProc_turnoffTuning(void)
{
	InterProc_fillNewCmd(arCmd_Command_turnoffTuning, sizeof(arCmd_Command_turnoffTuning));
}

void InterProc_turnonTuning(void)
{
	InterProc_fillNewCmd(arCmd_Command_turnonTuning, sizeof(arCmd_Command_turnonTuning));
}

//enter second proc in sleep mode
void InterProc_goSleep(void)
{
	InterProc_fillNewCmd(arCmd_Command_goSleep, sizeof(arCmd_Command_goSleep));
}
//awake from sleep
void InterProc_goAwake(void)
{
	InterProc_fillNewCmd(arCmd_Command_goAwake, sizeof(arCmd_Command_goAwake));
}



//send doserate window table in second proc
//prior it must be calculated
void InterProc_setWinTable(void)
{
	BYTE ar[43]={1,0x13,40,0xff,0x80,0x00,36};
	for(int i=0;i<SD_WIN_SIZE+1;i++)
	{
		ar[7+i*2]=HIBYTE_W(spectrumControl.wins1[i]);
		ar[7+i*2+1]=LOBYTE(spectrumControl.wins1[i]);
	}
	InterProc_fillNewCmd((const BYTE*)ar, sizeof(ar));
}

void InterProc_getGain(void)
{
	if(InterProc_isReadyToTransmit(&interProcControl.rsModbus.swdStabGain))
	{
		InterProc_fillNewCmd(arCmd_GetControlRegister_gain, sizeof(arCmd_GetControlRegister_gain));
	}
}


void InterProc_stabilize(void)
{
	InterProc_fillNewCmd(arCmd_Command_stabilize, sizeof(arCmd_Command_stabilize));
}


/*
void InterProc_getBkgDR(void)
{
	if(InterProc_isReadyToTransmit(&interProcControl.rsModbus.swdBkgDR))
	{
		InterProc_fillNewCmd(arCmd_GetControlRegister_bkgDR, sizeof(arCmd_GetControlRegister_bkgDR));
	}
}
*/

void InterProc_getBkgCPS(void)
{
//	if(InterProc_isReadyToTransmit(&interProcControl.rsModbus.swdBkgCPS))
	//{
		InterProc_fillNewCmd(arCmd_GetControlRegister_bkgCPS, sizeof(arCmd_GetControlRegister_bkgCPS));
//	}
}
