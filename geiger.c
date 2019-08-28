//geiger.c

#include <iolpc2388.h>
#include <string.h>
#include <stdio.h>

#include <math.h>
#include "types.h"
//#include "interProc.h"
#include "syncObj.h"
#include "display.h"
#include "geiger.h"
#include "powerControl.h"
#include "interrupts.h"
#include "SPRD_mode.h"
#include "LOG_mode.h"
#include "sound.h"

struct tagGeigerControl geigerControl;




void Geiger_Init(void)
{
	geigerControl.dwMomCount = 0;
	geigerControl.fDrCoef = 4.8;
	geigerControl.fDrSelfCps = 0.02;
	geigerControl.fDrDeadTime = 43e-6;
	geigerControl.dwTotalCount = 0;
	geigerControl.dwTotalTime = 0;
	memset((void*)&geigerControl.countBuf[0],0,MAX_COUNT_ITEMS*sizeof(DWORD));
	Geiger_values_Init();
	geigerControl.dwTime = 0;
	geigerControl.bSafetyAlarm_log =  FALSE;
	geigerControl.bOverload_log = FALSE;
	memset((void*)&geigerControl.esentVals, 0, sizeof(geigerControl.esentVals));
	memset((void*)&geigerControl.esentVals_safe, 0, sizeof(geigerControl.esentVals_safe));
}


//init values of
void Geiger_values_Init(void)
{
	geigerControl.esentVals.fCps = 0;
	geigerControl.esentVals.fDoserate = 0;
	geigerControl.bReset = 0;
	geigerControl.esentVals.fCpsErr = 0;
	geigerControl.esentVals.fSKO = 0;
	geigerControl.dwCountBkg = 0;
	geigerControl.dwCount = 0;
	for(int i=0;i<MAX_COUNT_ITEMS;i++)
		geigerControl.dwCount += geigerControl.countBuf[i];
	geigerControl.dwTime = MAX_COUNT_ITEMS;
	geigerControl.dwMomCountCopy = geigerControl.countBuf[MAX_COUNT_ITEMS-1];
}



//adjust counter
void Geiger_INT_Init(void)
{
	//////to wake up from geiger
	PINMODE1_bit.P0_23 = 2;
	IO0INTENR = 0x0;
	IO0INTENR_bit.P0_23 = 1;  //enable interrupt from GPIO0_23 (P0_23)
	IO0INTCLR = 0xffffffff;
	///////////
}


//called once a second to make first process of geiger data
void Geiger_GetCount_intcall(void)
{
	//calc geiger count per seconds
	//take in account deadtime, coef, selfcps
	if(geigerControl.bReset)
	{//reset if flag is set
		Geiger_values_Init();
	}
	geigerControl.dwCount += geigerControl.dwMomCount;
	geigerControl.dwTotalCount += geigerControl.dwMomCount;
	geigerControl.dwMomCountCopy = geigerControl.dwMomCount;
	geigerControl.dwMomCount = 0;
	geigerControl.dwTime++;
	geigerControl.dwTotalTime++;
	//we must process data in interrupts of clock, that is because of in power down mode we would not done it.
	Geiger_processData();	//this routine needs much processor time
}

__arm void _INT_Geiger(void)
{
	if(IO0INTSTATR_bit.P0_23)
	{
		geigerControl.dwMomCount++;	//count geiger pulses
		powerControl.bAwakedByGeiger = 1;	//awaked by geiger flag
		IO0INTCLR_bit.P0_23 = 1;
	}else
	if(IO2INTSTATR_bit.P2_4)
	{
		IO2INTCLR_bit.P2_4 = 1;
	}else
	{
		IO0INTCLR = 0xffffffff;
	}
}


//copy geiger data to safe place to be used by main routine
void Geiger_copyDataToSafePlace(void)
{
	SAFE_DECLARE;
	DISABLE_VIC;
	memcpy((void*)&geigerControl.esentVals_safe,(const void*)&geigerControl.esentVals,sizeof(geigerControl.esentVals));
	ENABLE_VIC;
}


//process data of geiger if INT is finished
void Geiger_processData(void)
{
	float val;
	//consider dead time
	geigerControl.esentVals.fCps = (float)geigerControl.dwCount/(float)geigerControl.dwTime - geigerControl.fDrSelfCps;
	if(geigerControl.esentVals.fCps<0)//consider -cps to 0
		geigerControl.esentVals.fCps=0;
	val = 1.0 - (float)geigerControl.fDrDeadTime * geigerControl.esentVals.fCps;
	if(val<=0)
		geigerControl.esentVals.fDoserate = MAX_DR;
	else
		geigerControl.esentVals.fDoserate = geigerControl.esentVals.fCps / val * geigerControl.fDrCoef;



	if(geigerControl.dwCount)//calculate statistical error
		geigerControl.esentVals.fCpsErr = 200.0/sqrt((float)geigerControl.dwCount);
	else
		geigerControl.esentVals.fCpsErr = 200.0;
	geigerControl.dwCountBkg+=geigerControl.countBuf[0];
	memmove((void*)&geigerControl.countBuf[0], (void const *)&geigerControl.countBuf[1], sizeof(DWORD)*(MAX_COUNT_ITEMS-1));
	geigerControl.countBuf[MAX_COUNT_ITEMS-1] = geigerControl.dwMomCountCopy;
	//������ � countBuf ���������� ������� �� ��������� MAX_COUNT_ITEMS ���������, � � dwCountBkg ���������� ����� ��� �� ��� ���� ��������� ����� ������� �� MAX_COUNT_ITEMS
	if(geigerControl.dwCountBkg>=9 && geigerControl.dwTime>MAX_COUNT_ITEMS)
	{//���������� ����������
		//���������� ������������� ������ ����������
		DWORD sum=0;
		for(int i=0;i<MAX_COUNT_ITEMS;i++)
			sum+=geigerControl.countBuf[i];
		float bkg = (float)geigerControl.dwCountBkg/((float)(geigerControl.dwTime-MAX_COUNT_ITEMS)/(float)MAX_COUNT_ITEMS);//������� ���� �� ������� ����������� � ������
		geigerControl.esentVals.fSKO = (((float)sum-bkg))/sqrt(bkg);


		if(fabs(geigerControl.esentVals.fSKO)>=5 || (geigerControl.esentVals.fSKO<-2.5 && sum==0))
		{//reset averaging
			geigerControl.bReset = 1;
		}
	}
	if(geigerControl.dwTotalTime>300)
	{//geiger is not operating
		if(geigerControl.dwTotalCount==0 && !modeControl.bSysDefault)//�������� ���������� ������ ���� �� ��������� �� ��������� ��� ����� ��� �������
			exception(__FILE__,__FUNCTION__,__LINE__,"GM counter failed");
	}
	
	//check for safety threshold and overloads
	BOOL bal = ((geigerControl.esentVals.fDoserate>SPRDModeControl.fDRThreshold ||
				 geigerControl.esentVals.fDoserate>MAX_DR) && geigerControl.esentVals.fCpsErr<50);
	if(geigerControl.esentVals.bSafetyAlarm != bal)
	{
		geigerControl.esentVals.bSafetyAlarm = bal;
		if(!bal)
		{//need update MCS on screen
			SPRDModeControl.bUpdateMCS = TRUE;
		}
	}
	//check for oveload
	geigerControl.esentVals.bOverload = (geigerControl.esentVals.fDoserate>MAX_DR &&
	   geigerControl.esentVals.fCpsErr<50);
}


//process safety threshold exceeding
//main cycle function
void Geiger_control(void)
{
	if(geigerControl.esentVals_safe.bSafetyAlarm)
	{//SAFETY THRESHOLD EXCEEDING ALARM

		SoundControl_Alarm_intcall(500, 500);

		if(!geigerControl.bSafetyAlarm_log)
		{//make event in log only once
			geigerControl.bSafetyAlarm_log = TRUE;
#ifndef GPS_BT_FREE	
			LOGMode_insertGPS();
#endif	//#ifndef GPS_BT_FREE	
			LOGMode_insertEventByLang("Radiation safety alarm!\0""Strahlungssicherheit Alarm!\0""Radiation safety alarm!\0""������� ������������ ������������!");
			LOGMode_insertDoserate(geigerControl.esentVals_safe.fDoserate);
			//activate SPRD mode
			if(modeControl.pMode != &modes_SPRDMode &&
			   !SPRDModeControl.bNaIMode &&
				!SPRDModeControl.bGMMode)
			{//switch to SPRD mode only when not NaI and GM mode
				Modes_setActiveMode(&modes_SPRDMode);
			}
		}


	}else
	{
		//no alarm and allow making log event again later if alarm occures
		geigerControl.bSafetyAlarm_log = FALSE;
	}


	//check for overload
	if(geigerControl.esentVals_safe.bOverload)
	{
		if(!geigerControl.bOverload_log)
		{//make overload event log once 
			geigerControl.bOverload_log = TRUE;
#ifndef GPS_BT_FREE	
			LOGMode_insertGPS();
#endif	//#ifndef GPS_BT_FREE	
			LOGMode_insertEventByLang("OVERLOAD!\0""UBERLADUNG!\0""OVERLOAD!\0""����������!");
			//activate SPRD mode
			if(modeControl.pMode != &modes_SPRDMode &&
			   !SPRDModeControl.bNaIMode &&
				!SPRDModeControl.bGMMode)
			{
				Modes_setActiveMode(&modes_SPRDMode);
			}
		}
	}else
	{
		geigerControl.bOverload_log = FALSE;
	}

}
