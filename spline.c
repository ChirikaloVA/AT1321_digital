//spline.c


#include <math.h>
#include "spline.h"
#include <iolpc2388.h>








//calc spline
//table: index-channels, mean - energies or sigmas
void spline_calcSpline(
	struct tagIndexMeanTable*  pTable, /*table of index and values*/
	int itemsNum, /*items number in table*/
	WORD* pRetArray,	/*return array of values, must be propertly sized*/
	float mult	/*multiplyer for values*/
		)
{
	int j;
	float min, mean;
	//сортировка массива энергий, каналов и сигм по возрастанию каналов
	for(int k=0;k<itemsNum;k++)
	{
		min=pTable[k].index;
		j = k;
		for(int i=k+1;i<itemsNum;i++)
		{
			mean = pTable[i].index;
			if(mean<min)
			{
				min = mean;
				j = i;
			}
		}
		pTable[j].index= pTable[k].index;
		pTable[k].index = min;
		mean = pTable[j].mean;
		pTable[j].mean = pTable[k].mean;
		pTable[k].mean = mean;
	}

	float xx[MAX_TABLE_RECORDS];
	float yy[MAX_TABLE_RECORDS];
	for(int k=0;k<itemsNum;k++)
	{
		xx[k] = pTable[k].index;
		yy[k] = pTable[k].mean;
	}
	float buf[MAX_TABLE_RECORDS];
	
	//получаем массив сплайн коэффициентов
	spline_spline(xx, yy, itemsNum, 2E30, 2E30, buf);
	//получаем данные сплайна
	for(int ii=0;ii<CHANNELS;ii++)
	{
		spline_splint(xx, yy, buf, itemsNum, ii, &min);
		pRetArray[ii]=(WORD)(min*mult+0.5);
	}
}













/*Given arrays x[1..n] and y[1..n] containing a tabulated function, i.e., yi = f(xi), with
x1 <x2 < :: : < xN, and given values yp1 and ypn for the rst derivative of the interpolating
function at points 1 and n, respectively, this routine returns an array y2[1..n] that contains
the second derivatives of the interpolating function at the tabulated points xi. If yp1 and/or
ypn are equal to 1*10^30 or larger, the routine is signaled to set the corresponding boundary
condition for a natural spline, with zero second derivative on that boundary.*/
void spline_spline(float * x, float * y, int n, float yp1, float ypn, float * y2)
{
	int i,k;
	float p,qn,sig,un,u[MAX_TABLE_RECORDS];

	if (yp1 > 0.99e30) //The lower boundary condition is set either to be natural
		y2[0]=u[0]=0.0;
	else
	{// or else to have a specified first derivative.
		y2[0] = -0.5;
		u[0]=(3.0/(x[1]-x[0]))*((y[1]-y[0])/(x[1]-x[0])-yp1);
	}
	for (i=1;i<n-1;i++)
	{
		/*This is the decomposition loop of the tridiagonal algorithm.
		y2 and u are used for temporary
		storage of the decomposed
		factors.*/
		sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
		p=sig*y2[i-1]+2.0;
		y2[i]=(sig-1.0)/p;
		u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
		u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
	}
	if (ypn > 0.99e30) //The upper boundary condition is set either to be natural
		qn=un=0.0;
	else
	{// or else to have a specified first derivative.
		qn=0.5;
		un=(3.0/(x[n-1]-x[n-2]))*(ypn-(y[n-1]-y[n-2])/(x[n-1]-x[n-2]));
	}
	y2[n-1]=(un-qn*u[n-2])/(qn*y2[n-2]+1.0);
	for (k=n-2;k>=0;k--) //This is the backsubstitution loop of the tridiagonal algorithm.
		y2[k]=y2[k]*y2[k+1]+u[k];
}


/*
Given the arrays xa[1..n] and ya[1..n], which tabulate a function (with the xai's in order),
and given the array y2a[1..n], which is the output from spline above, and given a value of
x, this routine returns a cubic-spline interpolated value y.
*/
char spline_splint(float * xa, float * ya, float * y2a, int n, float x, float *y)
{
	int klo,khi,k;
	float  h,b,a;
	klo=0;
	/*We will find the right place in the table by means of
	bisection. This is optimal if sequential calls to this
	routine are at random values of x. If sequential calls
	are in order, and closely spaced, one would do better
	to store previous values of klo and khi and test if
	they remain appropriate on the next call.*/
	khi=n-1;
	while (khi-klo > 1)
	{
		k=(khi+klo) >> 1;
		if (xa[k] > x)
			khi=k;
		else
			klo=k;
	} //klo and khi now bracket the input value of x.
	h=xa[khi]-xa[klo];
	if (h == 0.0) return 0;//nrerror("Bad xa input to routine splint"); //The xa's must be distinct.
	a=(xa[khi]-x)/h;
	b=(x-xa[klo])/h; //Cubic spline polynomial is now evaluated.
	*y=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])*(h*h)/6.0;
	return 1;
}



