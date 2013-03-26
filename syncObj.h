//syncObj.h
#ifndef _SYNCOBJ_H
#define _SYNCOBJ_H

#include <iolpc2388.h>
#include "types.h"
#include <intrinsics.h>

#define SAFE_DECLARE

#define DISABLE_VIC __disable_interrupt();
#define ENABLE_VIC __enable_interrupt();

/*
#define SAFE_DECLARE DWORD VICIntEnable_copy;

#define DISABLE_VIC VICIntEnable_copy = VICIntEnable; VICIntEnClear = VICIntEnable_copy;
#define ENABLE_VIC VICIntEnable = VICIntEnable_copy;
*/
#define SAFE_SET(variable, value) SAFE_DECLARE; DISABLE_VIC; variable = value;  ENABLE_VIC;

BYTE SAFE_GET_BYTE(BYTE volatile *pVariable);
WORD SAFE_GET_WORD(WORD volatile *pVariable);
DWORD SAFE_GET_DWORD(DWORD volatile *pVariable);


void SyncObj_disableVIC(void);
void SyncObj_enableVIC(void);
void SyncObj_memcpy_safe(BYTE* pDestination, const BYTE* pSource, int size);

void SyncObj_copyUARTtoSafePlace(struct tagUART * pUart);

#endif  //#ifndef _SYNCOBJ_H


