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
//	if(geigerControl.dwCountBkg>=9 && geigerControl.dwTime>MAX_COUNT_ITEMS)
//	{//статистика достаточна
//		//определяем необходимость сброса усреднения
//		DWORD sum=0;
//		for(int i=0;i<MAX_COUNT_ITEMS;i++)
//			sum+=geigerControl.countBuf[i];
//		float bkg = (float)geigerControl.dwCountBkg/((float)(geigerControl.dwTime-MAX_COUNT_ITEMS)/(float)MAX_COUNT_ITEMS);//привели фона ко времени измеренного в буфере
//		geigerControl.esentVals.fSKO = (((float)sum-bkg))/sqrt(bkg);
//
//
//		if(fabs(geigerControl.esentVals.fSKO)>=5 || (geigerControl.esentVals.fSKO<-2.5 && sum==0))
//		{//reset averaging
//			geigerControl.bReset = 1;
//		}
//	}
        if(oloAMAR_update(geigerControl.dwMomCount,1.0))
        {
          geigerControl.bReset = 1;
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

//----------------------------------------------------
// 3.01.2021 алгоритм oloAMAR
//----------------------------------------------------

float prm_limit_min[100];
float prm_limit_max[100];
float prm_sigma;

unsigned int array_count[oldAMAR_N];
float array_time[oldAMAR_N];
unsigned int array_n;


//Общее кол-во импульсов
unsigned int count_all;
//Общее время накопления
float time_all;

const float reset_threshold_min[100] = {
	0.0,
	16.375458,
	18.849539,
	21.052980,
	23.046068,
	24.975562,
	26.795935,
	28.748987,
	30.535946,
	32.109638,
	33.764431,
	35.504505,
	37.334255,
	38.865720,
	40.460005,
	42.119689,
	43.408979,
	45.189631,
	46.572893,
	47.998496,
	49.467737,
	50.981952,
	52.542517,
	54.150852,
	55.250334,
	56.941555,
	58.097699,
	59.876078,
	61.091805,
	62.332216,
	64.240215,
	65.544551,
	66.875371,
	68.233212,
	69.618623,
	71.032163,
	72.474404,
	73.945928,
	74.692856,
	76.209424,
	77.756784,
	79.335561,
	80.136931,
	81.764035,
	83.424176,
	84.266844,
	85.977803,
	86.846265,
	88.609596,
	89.504643,
	91.321950,
	92.244393,
	93.176155,
	95.068008,
	96.028291,
	97.978055,
	98.967732,
	99.967406,
	100.977178,
	103.027423,
	104.068105,
	105.119298,
	106.181109,
	108.337015,
	109.431328,
	110.536695,
	111.653228,
	112.781038,
	113.920240,
	115.070950,
	116.233283,
	117.407356,
	119.791201,
	121.001213,
	122.223448,
	123.458028,
	124.705079,
	125.964726,
	127.237097,
	128.522320,
	129.820526,
	129.820526,
	131.131844,
	132.456408,
	133.794352,
	135.145810,
	136.510919,
	137.889817,
	139.282644,
	140.689539,
	142.110645,
	142.110645,
	143.546107,
	144.996067,
	146.460674,
	147.940075,
	149.434419,
	149.434419,
	150.943857,
	152.468543
};

const float reset_threshold_max[100] = {
	0.0,
	0.001391,
	0.018051,
	0.070104,
	0.166386,
	0.307165,
	0.492626,
	0.714521,
	0.965960,
	1.254423,
	1.564839,
	1.913224,
	2.269695,
	2.665657,
	3.068397,
	3.461698,
	3.905412,
	4.361941,
	4.823117,
	5.279722,
	5.779554,
	6.263438,
	6.787834,
	7.282573,
	7.813371,
	8.382858,
	8.903914,
	9.457357,
	9.944748,
	10.562887,
	11.107253,
	11.679673,
	12.281593,
	12.914534,
	13.444293,
	13.995782,
	14.717065,
	15.320764,
	15.949228,
	16.437436,
	17.111705,
	17.813634,
	18.358912,
	19.112001,
	19.697022,
	20.299951,
	20.921335,
	21.561741,
	22.221749,
	22.901960,
	23.602992,
	24.325484,
	24.819389,
	25.579115,
	26.098474,
	26.897353,
	27.443478,
	28.283527,
	28.857797,
	29.741138,
	30.345004,
	30.961131,
	31.589767,
	32.231167,
	33.217768,
	33.892223,
	34.580373,
	35.282494,
	35.998872,
	36.729795,
	37.475558,
	37.854099,
	38.622691,
	39.406888,
	40.207008,
	41.023373,
	41.437750,
	42.279104,
	43.137541,
	43.573274,
	44.457988,
	45.360665,
	45.818854,
	46.749162,
	47.221376,
	48.180161,
	48.666829,
	49.654963,
	50.156529,
	51.174909,
	51.691828,
	52.741381,
	53.274122,
	53.812245,
	54.904851,
	55.459446,
	56.019642,
	57.157068,
	57.734412,
	58.317588
};

//Сброс
void oloAMAR_reset(void)
{
  count_all = 0;
  time_all = 0.0;
  for (unsigned int i = 0; i < oldAMAR_N; i++)
  {
    array_count[i] = 0;
    array_time[i] = 0.0;
  }
  array_n = 0;
//  level = 0.0;
}

//инит при запуску прибора
void oloAMAR_start(void)
{
  unsigned int i; 
  prm_sigma = 4.2656;
//  for (i = 0; i < 100; i++)
//  {
//    prm_limit_min[i] = reset_threshold_min[i];
//    prm_limit_max[i] = reset_threshold_max[i];
//  }
  oloAMAR_reset();
}




//инициализация
//_fap - период ложных тревог
//_device_time - время обновления мгновенной скорости счёта
BOOL oloAMAR_init(float _fap, float _device_time)
{
//  if (_fap <= 0.0 || _device_time <= 0.0)
//    return FALSE;
//  float probability = 0.5 * _device_time / (_fap * oldAMAR_N * 0.5);
//  if (probability <= 0.0 || probability > 1.0)
//    return FALSE;
//  
//  prm_limit_max[0] = 0.0;
//  for (unsigned int i = 1; i < 100; i++)
//  {
//    float m = (float)i;
//    float mid = 200.0;
//    float v = oloAMAR_quantile_poisson(1.0 - probability, mid);
//    while (v > m)
//    {
//      v = oloAMAR_quantile_poisson(1.0 - probability, mid);
//      mid *= 0.99;
//    }
//    prm_limit_max[i] = mid;
//  }
//  prm_limit_min[0] = 0.0;
//  for (unsigned int i = 1; i < 100; i++)
//  {
//    float m = (float)i;
//    float mid = 200.0;
//    float v = oloAMAR_quantile_poisson(probability, mid);
//    while (v > m)
//    {
//      v = oloAMAR_quantile_poisson(probability, mid);
//      mid *= 0.99;
//    }
//    prm_limit_min[i] = mid;
//  }
//  
//  prm_sigma = oloAMAR_quantile_gauss_in_sigm(1.0 - probability);
  
  return TRUE;
}


//Обновление
//_count - кол-во импульсов за интервал времени
//_time - длительность интервала времени, с
//возвращает необходимость сброса усреднения
BOOL oloAMAR_update(unsigned int _count, float _time)
{
	BOOL result = FALSE;
	if (_time <= 0.0)
		return result;
	count_all += _count;
	time_all += _time;
	if (array_n > 0 && array_count[0] == 0)
	{
		array_count[0] += _count;
		array_time[0] += _time;
	}
	else
	{
		unsigned int n = array_n;
		if (n > (oldAMAR_N - 1))
			n = oldAMAR_N - 1;
		for (unsigned int i = n; i > 0; i--)
		{
			array_count[i] = array_count[i - 1];
			array_time[i] = array_time[i - 1];
		}
		array_count[0] = _count;
		array_time[0] = _time;
		if (array_n < oldAMAR_N)
			array_n++;
	}
	float cps = (time_all > 0.0) ? (((float)count_all) / time_all) : 0.0;
	if (cps > 0.0)
	{
		float T_int = 0.0;
		unsigned int  N_int = 0;
		float N_mid = 0.0;
//		level = 0.0;
		for (unsigned int i = 0; i < array_n; i++)
		{
			T_int += array_time[i];
			N_int += array_count[i];
			N_mid = T_int * cps;

			unsigned int  N_mid_l = 0;
			unsigned int  N_mid_r = 0;
			if (N_mid < prm_limit_max[99])
			{
				N_mid_l = 0;				
				N_mid_r = 99;
				for (unsigned int j = 1; j < 100; j++)
				{
					if (N_mid <= reset_threshold_min[j])
					{
						N_mid_l = j - 1;
						break;
					}
				}
				for (unsigned int j = 2; j < 100; j++)
				{
					if (N_mid <= reset_threshold_max[j])
					{
						N_mid_r = j - 1;
						break;
					}
				}
			}
			else
			{
				float sgm = prm_sigma * sqrt(N_mid);
				N_mid_l = 1 + (unsigned int )(N_mid - sgm);
				N_mid_r = (unsigned int )(N_mid + sgm);
			}

			result = (N_int < N_mid_l || N_int > N_mid_r);
			if (result)
			{
				count_all = 0;
				time_all = 0.0;
				array_n = 0;
//				level = 0.0;
				break;
			}
		}
	}
	return result;
}
