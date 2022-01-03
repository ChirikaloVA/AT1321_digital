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


//------------------------------------------------------
// 23.12.2021 Чирикало Таймер 3 для счетчика гейгера
// 
//-------------------------------------------------------
void Geiger_Timer3_cnt_Init(void)
{
  PINMODE1_bit.P0_23 = 3;
  PCONP_bit.PCTIM3 = 1;
  PINSEL1_bit.P0_23 = 3;
  T3TCR_bit.CR = 1;
  T3TCR_bit.CE = 0;
  T3CTCR_bit.CTM = 1;
  T3CTCR_bit.CIS = 0;
}

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
//  T3TCR_bit.CR = 1;
//  T3TCR_bit.CR = 0;
  T3TCR_bit.CE = 1;
	geigerControl.esentVals.fCps = 0;
	geigerControl.esentVals.fDoserate = 0;
	geigerControl.bReset = 0;
	geigerControl.esentVals.fCpsErr = 0;
	geigerControl.esentVals.fSKO = 0;
	geigerControl.dwCountBkg = 0;
	geigerControl.dwCount = 0;
        geigerControl.dwMomCount = 0;
	for(int i=0;i<MAX_COUNT_ITEMS;i++)
		geigerControl.dwCount += geigerControl.countBuf[i];
	geigerControl.dwTime = MAX_COUNT_ITEMS;
//        geigerControl.dwMomCount = geigerControl.dwCount/geigerControl.dwTime;
        geigerControl.dwMomCount = T3TC;
	geigerControl.dwMomCountCopy = geigerControl.countBuf[MAX_COUNT_ITEMS-1];
}



//adjust counter
void Geiger_INT_Init(void)
{
	//////to wake up from geiger
        DIR_G_CNT = 0; 
	PINMODE1_bit.P0_23 = 3;
	IO0INTENR = 0x0;
	IO0INTENR_bit.P0_23 = 1;  //enable interrupt from GPIO0_23 (P0_23)
	IO0INTCLR = 0xffffffff;
	///////////
}


//called once a second to make first process of geiger data
void Geiger_GetCount_intcall(void)
{
        SET_ISD_INT;
	//calc geiger count per seconds
	//take in account deadtime, coef, selfcps
	if(geigerControl.bReset)
	{//reset if flag is set
		Geiger_values_Init();
	}
        else
        {
          geigerControl.dwMomCount = T3TC;
        }
        
	geigerControl.dwCount += geigerControl.dwMomCount;
	geigerControl.dwTotalCount += geigerControl.dwMomCount;
	geigerControl.dwMomCountCopy = geigerControl.dwMomCount;
//        geigerControl.dwCount = geigerControl.dwTime;
//	geigerControl.dwTotalCount = geigerControl.dwTime;
//	geigerControl.dwMomCountCopy = geigerControl.dwTime;
	geigerControl.dwMomCount = 0;
        T3TCR_bit.CR = 1;
        T3TCR_bit.CR = 0;
	geigerControl.dwTime++;
	geigerControl.dwTotalTime++;
//        geigerControl.dwTime = 1;
//	geigerControl.dwTotalTime = 1;
	//we must process data in interrupts of clock, that is because of in power down mode we would not done it.
	Geiger_processData();	//this routine needs much processor time
        CLR_ISD_INT;
}

__arm void _INT_Geiger(void)
{
  if(IO0INTSTATR_bit.P0_23)
  {
    if(PIN_G_CNT)
    {
      geigerControl.dwMomCount++;	//count geiger pulses
    }
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
//        geigerControl.esentVals.fCps = (float)geigerControl.dwCount/(float)geigerControl.dwTime;
	if(geigerControl.esentVals.fCps<0)//consider -cps to 0
		geigerControl.esentVals.fCps=0;
	val = 1.0 - (float)geigerControl.fDrDeadTime * geigerControl.esentVals.fCps;
	if(val<=0)
        {
		geigerControl.esentVals.fDoserate = MAX_DR;
//                geigerControl.esentVals.fDoserate = (float)geigerControl.dwTime;
        }
	else
        {
		geigerControl.esentVals.fDoserate = geigerControl.esentVals.fCps / val * geigerControl.fDrCoef;
//                geigerControl.esentVals.fDoserate = (float)geigerControl.dwTime;
        }



	if(geigerControl.dwCount)//calculate statistical error
		geigerControl.esentVals.fCpsErr = 200.0/sqrt((float)geigerControl.dwCount);
	else
		geigerControl.esentVals.fCpsErr = 200.0;
	geigerControl.dwCountBkg+=geigerControl.countBuf[0];
	memmove((void*)&geigerControl.countBuf[0], (void const *)&geigerControl.countBuf[1], sizeof(DWORD)*(MAX_COUNT_ITEMS-1));
	geigerControl.countBuf[MAX_COUNT_ITEMS-1] = geigerControl.dwMomCountCopy;
	//теперь в countBuf содержится история за последние MAX_COUNT_ITEMS измерений, а в dwCountBkg содержится сумма имп за все пред измерения минус история за MAX_COUNT_ITEMS
	if(geigerControl.dwCountBkg>=9 && geigerControl.dwTime>MAX_COUNT_ITEMS)
	{//статистика достаточна
		//определяем необходимость сброса усреднения
		DWORD sum=0;
		for(int i=0;i<MAX_COUNT_ITEMS;i++)
			sum+=geigerControl.countBuf[i];
		float bkg = (float)geigerControl.dwCountBkg/((float)(geigerControl.dwTime-MAX_COUNT_ITEMS)/(float)MAX_COUNT_ITEMS);//привели фона ко времени измеренного в буфере
		geigerControl.esentVals.fSKO = (((float)sum-bkg))/sqrt(bkg);


		if(fabs(geigerControl.esentVals.fSKO)>=5 || (geigerControl.esentVals.fSKO<-2.5 && sum==0))
		{//reset averaging
			geigerControl.bReset = 1;
		}
	}
	if(geigerControl.dwTotalTime>300)
	{//geiger is not operating
		if(geigerControl.dwTotalCount==0 && !modeControl.bSysDefault)//показать исключение только если не настройки по умолчанию это нужно для наладки
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
			LOGMode_insertEventByLang("Radiation safety alarm!\0""Strahlungssicherheit Alarm!\0""Radiation safety alarm!\0""Тревога радиационной безопасности!");
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
			LOGMode_insertEventByLang("OVERLOAD!\0""UBERLADUNG!\0""OVERLOAD!\0""ПЕРЕГРУЗКА!");
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
