//puasson.c

#include <math.h>
#include "puasson.h"
#include <iolpc2388.h>
#include "powerControl.h"

struct tagPuasson puasson;

/*
double puasson_pow_fact(double mid, ULONG k)
{
	if(mid<=0)return 0;
	double e = -mid+log(mid)*k;
	double lnn = 0;
	for(ULONG i=1;i<=k;i++)
		lnn+=log((double)i);
	e = e-lnn;
	e = exp(e);
	return e;
}*/

double puasson_pow_fact(double mid, ULONG k)
{
	double ret = 1;
	if(k==0)return ret;
	for(ULONG i=1;i<=k;i++)
		ret*=mid/(double)i;
	return ret;
}

//ret FALSE if overrange
BOOL puasson_CalcthG(float bkgval, double period, double dwell, UINT *t1, double *p2)
{
#define GTHRESH (double)(0.05)

	*p2 = 0;
	double dt1 = dwell;
	double mid = bkgval;	//middle background cps
	double div2 = 1.0/dt1;
	UINT fmt = puasson_findMaxThreshold(mid, dt1, period, GTHRESH, p2);
	if(fmt==0)return FALSE;	//error
	*t1 = div2*fmt;
	return TRUE;
}


// find and ret max threshold by calculation puasson, p returns probability for cur threshold
//ret 0 if overrange
UINT puasson_findMaxThreshold(
	double mid, /*middle value*/
	double dtmi, /*dwel time*/
	double ntmi, /*need time*/
	double thr,	/*sigma threshold*/
	double* p /*probability*/
	)
{
	double dm = mid*dtmi;
	double nd = ntmi/dtmi;
	UINT t=(UINT)dm;
	UINT maxt = (t+1)*30;
	double pu = puasson_puasson(t,dm);
	if(pu==-1)return 0;	//error
	*p = (1.0-pu)*nd;
	if(*p<thr)return t;
	t++;
	for(;t<maxt;t++)//from middle to 30 times more threshold value
	{
		PowerControl_kickWatchDog();
		pu = puasson_puasson_next(t, dm);
		if(pu==-1)return 0;	//error
		*p = (1.0-pu)*nd;
		if(*p<thr)break;
	}
	return t;
}

//ret -1 if overrange
double puasson_puasson_next(UINT x, double mid)
{
	double ep1 = 1.0;
	ep1 = exp(-mid);
	UINT k = x;
	puasson.summ += puasson_pow_fact(mid, k);
	if(puasson.summ>1.0e290)
	{
		return -1;	//error
	}
	double ret;
	ret = ep1*puasson.summ;
	return ret;
}


//ret -1 if overrange
double puasson_puasson(UINT x, double mid)
{
	double ep1 = 1.0;
	ep1 = exp(-mid);
	puasson.summ = 0;
	for(UINT k=0;k<=x;k++)
	{
		puasson.summ += puasson_pow_fact(mid, k);
		if(puasson.summ>1.0e290)
		{
			return -1;	//error
		}
	}
	double ret = 0;
	ret = ep1*puasson.summ;
	return ret;
}
