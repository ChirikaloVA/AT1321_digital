//spline.h


#ifndef ENSICAL_H
#define ENSICAL_H

#include "types.h"


void spline_calcSpline(
	struct tagIndexMeanTable*  pTable, /*table of index and values*/
	int itemsNum, /*items number in table*/
	WORD* pRetArray,	/*return array of values, must be propertly sized*/
	float mult	/*multiplyer for values*/
		);


void spline_spline(float * x, float * y, int n, float yp1, float ypn, float * y2);
char spline_splint(float * xa, float * ya, float * y2a, int n, float x, float *y);


#endif	//ENSICAL_H
