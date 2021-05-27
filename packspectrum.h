//packspectrum.h
//pack spectrum algorithm

#ifndef _PACKSPECTRUM_H
#define _PACKSPECTRUM_H

#include "types.h"
#include "modes.h"

BOOL UnpackSuperCompressSpectrum(/*BYTE* pbindata, */DWORD szbin, /*BYTE* pretbindata, */DWORD* pszretbin);
BOOL Restore0x80(/*BYTE* pbindata, */DWORD* pszbin);
void remove0x80(/*BYTE* pbindata, */DWORD* pszbin);
int  countSeq0x80(BYTE* pbindata, DWORD sz, int index);
BOOL superCompressSpectrum(/*BYTE* pbindata, */DWORD* pszbin);
int  countSeq1(DWORD sz, int index);
int  countSeq2(DWORD sz, int index);
int packSeq2(DWORD sz, int indexBD, int cnt, int indexNBD);
int  countSeq4(DWORD sz, int index);
int packSeq4(DWORD sz, int indexBD, int cnt, int indexNBD);

void packspectrum_init(void);


struct tagPackSpectrum
{
	BYTE* packed;
	BYTE* unpacked;
	WORD maskdata[4096];
};

extern struct tagPackSpectrum packspec;









#endif