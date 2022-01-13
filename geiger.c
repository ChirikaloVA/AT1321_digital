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
  geigerControl.dwTime = 0;
  for(int i=0;i<MAX_COUNT_ITEMS;i++)
     geigerControl.countBuf[i] = 0;
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
//        SET_ISD_INT;
	//calc geiger count per seconds
	//take in account deadtime, coef, selfcps
	if(geigerControl.bReset)
	{//reset if flag is set
		Geiger_values_Init();
	}   
        geigerControl.dwMomCount = T3TC;
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
//        CLR_ISD_INT;
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

        if(oloAMAR_update(geigerControl.dwMomCountCopy,1.0))
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
  0.000000,
  10.845172,
  12.865839,
  14.809671,
  16.540866,
  18.289689,
  20.021175,
  21.697415,
  23.278856,
  24.725807,
  26.262696,
  27.616163,
  29.332708,
  30.535946,
  32.109638,
  33.426787,
  34.797966,
  36.225391,
  37.711369,
  39.258303,
  40.460005,
  41.698492,
  42.974890,
  44.737735,
  45.646092,
  47.043326,
  48.483329,
  49.967411,
  50.981952,
  52.542517,
  53.609343,
  55.250334,
  56.372139,
  57.516722,
  58.684544,
  59.876078,
  61.708894,
  62.961835,
  64.240215,
  65.544551,
  66.206618,
  67.550880,
  68.922437,
  70.321841,
  71.749660,
  72.474404,
  73.945928,
  75.447329,
  76.209424,
  77.756784,
  78.542206,
  80.136931,
  80.946395,
  82.589934,
  83.424176,
  85.118025,
  85.977803,
  87.723500,
  88.609596,
  89.504643,
  91.321950,
  92.244393,
  93.176155,
  95.068008,
  96.028291,
  96.998274,
  97.978055,
  99.967406,
  100.977178,
  101.997149,
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
  118.593289,
  119.791201,
  121.001213,
  122.223448,
  123.458028,
  124.705079,
  125.964726,
  127.237097,
  127.237097,
  128.522320,
  129.820526,
  131.131844,
  132.456408,
  133.794352,
  135.145810,
  136.510919,
  136.510919
};
//const float reset_threshold_min[100] = {
//	0.0,
//	16.375458,
//	18.849539,
//	21.052980,
//	23.046068,
//	24.975562,
//	26.795935,
//	28.748987,
//	30.535946,
//	32.109638,
//	33.764431,
//	35.504505,
//	37.334255,
//	38.865720,
//	40.460005,
//	42.119689,
//	43.408979,
//	45.189631,
//	46.572893,
//	47.998496,
//	49.467737,
//	50.981952,
//	52.542517,
//	54.150852,
//	55.250334,
//	56.941555,
//	58.097699,
//	59.876078,
//	61.091805,
//	62.332216,
//	64.240215,
//	65.544551,
//	66.875371,
//	68.233212,
//	69.618623,
//	71.032163,
//	72.474404,
//	73.945928,
//	74.692856,
//	76.209424,
//	77.756784,
//	79.335561,
//	80.136931,
//	81.764035,
//	83.424176,
//	84.266844,
//	85.977803,
//	86.846265,
//	88.609596,
//	89.504643,
//	91.321950,
//	92.244393,
//	93.176155,
//	95.068008,
//	96.028291,
//	97.978055,
//	98.967732,
//	99.967406,
//	100.977178,
//	103.027423,
//	104.068105,
//	105.119298,
//	106.181109,
//	108.337015,
//	109.431328,
//	110.536695,
//	111.653228,
//	112.781038,
//	113.920240,
//	115.070950,
//	116.233283,
//	117.407356,
//	119.791201,
//	121.001213,
//	122.223448,
//	123.458028,
//	124.705079,
//	125.964726,
//	127.237097,
//	128.522320,
//	129.820526,
//	129.820526,
//	131.131844,
//	132.456408,
//	133.794352,
//	135.145810,
//	136.510919,
//	137.889817,
//	139.282644,
//	140.689539,
//	142.110645,
//	142.110645,
//	143.546107,
//	144.996067,
//	146.460674,
//	147.940075,
//	149.434419,
//	149.434419,
//	150.943857,
//	152.468543
//};
const float reset_threshold_max[100] = {
  0.000000,
  0.020288,
  0.108681,
  0.276747,
  0.516060,
  0.803067,
  1.141616,
  1.527916,
  1.925262,
  2.377666,
  2.849163,
  3.312756,
  3.813262,
  4.345494,
  4.902491,
  5.420818,
  5.993947,
  6.561395,
  7.182562,
  7.783911,
  8.351250,
  8.959941,
  9.612996,
  10.210514,
  10.845172,
  11.519278,
  12.112932,
  12.865839,
  13.528889,
  14.083849,
  14.809671,
  15.572897,
  16.211703,
  16.876713,
  17.569002,
  18.289689,
  19.039938,
  19.622753,
  20.427686,
  21.052980,
  21.697415,
  22.587451,
  23.278856,
  23.991426,
  24.725807,
  25.482668,
  26.262696,
  26.795935,
  27.616163,
  28.461498,
  29.039381,
  29.928281,
  30.535946,
  31.470656,
  32.109638,
  32.761594,
  33.426787,
  34.449986,
  35.149460,
  35.863137,
  36.591304,
  37.334255,
  38.092292,
  38.865720,
  39.654851,
  40.460005,
  41.281507,
  42.119689,
  42.974890,
  43.408979,
  44.290357,
  45.189631,
  46.107164,
  46.572893,
  47.518511,
  48.483329,
  48.973060,
  49.967411,
  50.472133,
  51.496921,
  52.017092,
  53.073250,
  53.609343,
  54.697830,
  55.250334,
  56.372139,
  56.941555,
  57.516722,
  58.684544,
  59.277317,
  59.876078,
  61.091805,
  61.708894,
  62.332216,
  63.597813,
  64.240215,
  64.889106,
  65.544551,
  66.875371,
  67.550880	
    
};
//const float reset_threshold_max[100] = {
//	0.0,
//	0.001391,
//	0.018051,
//	0.070104,
//	0.166386,
//	0.307165,
//	0.492626,
//	0.714521,
//	0.965960,
//	1.254423,
//	1.564839,
//	1.913224,
//	2.269695,
//	2.665657,
//	3.068397,
//	3.461698,
//	3.905412,
//	4.361941,
//	4.823117,
//	5.279722,
//	5.779554,
//	6.263438,
//	6.787834,
//	7.282573,
//	7.813371,
//	8.382858,
//	8.903914,
//	9.457357,
//	9.944748,
//	10.562887,
//	11.107253,
//	11.679673,
//	12.281593,
//	12.914534,
//	13.444293,
//	13.995782,
//	14.717065,
//	15.320764,
//	15.949228,
//	16.437436,
//	17.111705,
//	17.813634,
//	18.358912,
//	19.112001,
//	19.697022,
//	20.299951,
//	20.921335,
//	21.561741,
//	22.221749,
//	22.901960,
//	23.602992,
//	24.325484,
//	24.819389,
//	25.579115,
//	26.098474,
//	26.897353,
//	27.443478,
//	28.283527,
//	28.857797,
//	29.741138,
//	30.345004,
//	30.961131,
//	31.589767,
//	32.231167,
//	33.217768,
//	33.892223,
//	34.580373,
//	35.282494,
//	35.998872,
//	36.729795,
//	37.475558,
//	37.854099,
//	38.622691,
//	39.406888,
//	40.207008,
//	41.023373,
//	41.437750,
//	42.279104,
//	43.137541,
//	43.573274,
//	44.457988,
//	45.360665,
//	45.818854,
//	46.749162,
//	47.221376,
//	48.180161,
//	48.666829,
//	49.654963,
//	50.156529,
//	51.174909,
//	51.691828,
//	52.741381,
//	53.274122,
//	53.812245,
//	54.904851,
//	55.459446,
//	56.019642,
//	57.157068,
//	57.734412,
//	58.317588
//};
unsigned long oloAMAR_quantile_poisson(double _probability, double _mid)
{
	if (_mid <= 0.0 || _mid > 4.0E9)
		return 0;

	double sigma_n = 10.0;
	double mid_sigma_val = sigma_n * ((_mid > 2.25) ? sqrt(_mid) : 1.5);
	double left = _mid - mid_sigma_val;
	double right = _mid + mid_sigma_val;
	if (left < 0.0)		left = 0.0;
	unsigned long nleft = (unsigned long)left;
	unsigned long nright = 1 + (unsigned long)right;


	double sum = 0.0;
	double v_sum = 0.0;
	for (unsigned long j = 1; j < nleft; j++)
		v_sum += log((double)j);
	for (unsigned long i = nleft; i < nright; i++)
	{
		double dNum = (double)i;
		if (i > 0)
			v_sum += log((double)i);
		double result_i = exp(dNum * log(_mid) - _mid - v_sum);
		sum += result_i;
		if (sum >= _probability)
			return i;
	}
	return nright;
}



double oloAMAR_quantile_gauss_in_sigm(double _probability)
{
	if (_probability < 0.0)
		_probability = 0.0;
	else if (_probability > 1.0)
		_probability = 1.0;
	double a = 6.3830764864229228470E-4;
	double b = 0.00000128;// 1.0/(2.0*sigma*sigma)	
	double lim = (_probability <= 0.5) ? (1.0 - 2.0 * _probability) : (1.0 - 2.0 * (1.0 - _probability));
	double result = a;
	for (long i = 1; i < 5000; i++)
	{
		double di = (double)i;
		double gauss_value_i = a * exp(-di * di * b);
		result += 2.0 * gauss_value_i;
		if (result >= lim)
		{
			result = di * 0.0016;
			if (_probability <= 0.5)
				result *= -1.0;
			return result;
		}
	}
	return (_probability <= 0.5) ? -8.0 : 8.0;
}
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
  prm_sigma = 3.529600; //_fap = 600
  geigerControl._device_time = 1.0;
  geigerControl._fap = 600.0;
  for (i = 0; i < 100; i++)
  {
    prm_limit_min[i] = reset_threshold_min[i];
    prm_limit_max[i] = reset_threshold_max[i];
  }
  oloAMAR_reset();
}




//инициализация
//_fap - период ложных тревог
//_device_time - время обновления мгновенной скорости счёта
BOOL oloAMAR_init(double _fap, double _device_time)
{
  if (_fap <= 0.0 || _device_time <= 0.0)
    return FALSE;
  double probability = 0.5 * _device_time / (_fap * oldAMAR_N * 0.5);
  if (probability <= 0.0 || probability > 1.0)
    return FALSE;
  
  prm_limit_max[0] = 0.0;
#ifdef DBG_PRINTF_EN
  printf("const float reset_threshold_max[100] = {\r\n");
  printf("%f,\r\n",prm_limit_max[0]);
#endif
  for (unsigned int i = 1; i < 100; i++)
  {
    double m = (double)i;
    double mid = 200.0;
    double v = oloAMAR_quantile_poisson(1.0 - probability, mid);
    while (v > m)
    {
      v = oloAMAR_quantile_poisson(1.0 - probability, mid);
      mid *= 0.99;
    }
    prm_limit_max[i] = mid;
#ifdef DBG_PRINTF_EN
    printf("%f,\r\n",mid);
#endif
  }
#ifdef DBG_PRINTF_EN
  printf("}\r\n");
#endif
  prm_limit_min[0] = 0.0;
#ifdef DBG_PRINTF_EN
  printf("const float prm_limit_min[100] = {\r\n");
  printf("%f,\r\n",prm_limit_min[0]);
#endif
  for (unsigned int i = 1; i < 100; i++)
  {
    double m = (double)i;
    double mid = 200.0;
    double v = oloAMAR_quantile_poisson(probability, mid);
    while (v > m)
    {
      v = oloAMAR_quantile_poisson(probability, mid);
      mid *= 0.99;
    }
    prm_limit_min[i] = mid;
#ifdef DBG_PRINTF_EN
    printf("%f,\r\n",mid);
#endif
  }
#ifdef DBG_PRINTF_EN
  printf("}\r\n");
#endif
  prm_sigma = oloAMAR_quantile_gauss_in_sigm(1.0 - probability);
#ifdef DBG_PRINTF_EN
    printf("prm_sigma=%f,\r\n",prm_sigma);
#endif
  
  return TRUE;
}


//Обновление
//_count - кол-во импульсов за интервал времени
//_time - длительность интервала времени, с
//возвращает необходимость сброса усреднения
BOOL oloAMAR_update(unsigned int _count, float _time)
{
  float N_mid;
  unsigned int  N_int;
  unsigned int  N_mid_l;
  unsigned int  N_mid_r;
  float T_int;
  BOOL result = FALSE;
	if (_time <= 0.0)
		return result;

	count_all += _count;
	time_all += _time;
//#ifdef DBG_PRINTF_EN
//        printf("_count = %i,\r\n",_count);
//        printf("_time = %f,\r\n",_time);
//        printf("count_all = %i,\r\n",count_all);
//        printf("time_all = %f,\r\n",time_all);
//#endif
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
		T_int = 0.0;
		N_int = 0;
		N_mid = 0.0;
//		level = 0.0;
		for (unsigned int i = 0; i < array_n; i++)
		{
                  T_int += array_time[i];
                  N_int += array_count[i];
                  N_mid = T_int * cps;
                  
                  N_mid_l = 0;
                  N_mid_r = 0;
                  if (N_mid < prm_limit_max[99])
                  {
                    N_mid_l = 0;				
                    N_mid_r = 99;
                    for (unsigned int j = 1; j < 100; j++)
                    {
                      if (N_mid <= prm_limit_min[j])
                      {
                        N_mid_l = j - 1;
                        break;
                      }
                    }
                    for (unsigned int j = 2; j < 100; j++)
                    {
                      if (N_mid <= prm_limit_max[j])
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
//#ifdef DBG_PRINTF_EN
//                    SET_ISD_INT;
//                    //        printf("cps=%f,\r\n",cps);
//                    printf("----------------\r\n");
//                    printf("N_mid=%f,\r\n",N_mid);
//                    printf("N_int=%i,\r\n",N_int);
//                    printf("N_mid_l=%i,\r\n",N_mid_l);
//                    printf("N_mid_r=%i,\r\n",N_mid_r);
//                    printf("----------------\r\n");
//                    CLR_ISD_INT;
//#endif
                    count_all = 0;
                    time_all = 0.0;
                    array_n = 0;
                    //				level = 0.0;
                    break;
                  }
		}
	}
//#ifdef DBG_PRINTF_EN
//
//        //        printf("cps=%f,\r\n",cps);
//        printf("N_mid=%f,\r\n",N_mid);
//        printf("N_int=%i,\r\n",N_int);
//        printf("N_mid_l=%i,\r\n",N_mid_l);
//        printf("N_mid_r=%i,\r\n",N_mid_r);
//        printf("result=%i,\r\n",result);
//
//#endif
	return result;
}
