//puasson.h


#ifndef PUASSON_H
#define PUASSON_H

#include "types.h"



struct tagPuasson
{
	double summ;
};



extern struct tagPuasson puasson;


BOOL puasson_CalcthG(float bkgval, double period, double dwell, UINT* t1, double* p2);
double puasson_puasson(UINT x, double mid);
double puasson_pow_fact(double mid, ULONG k);
double puasson_puasson_next(UINT x, double mid);

UINT puasson_findMaxThreshold(
	double mid, /*middle value*/
	double dtmi, /*dwel time*/
	double ntmi, /*need time*/
	double thr,	/*sigma threshold*/
	double* p /*probability*/
	);



#endif	//PUASSON_H
