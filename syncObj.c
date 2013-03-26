//syncObj.c
#include <string.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>

#include "types.h"
#include "syncObj.h"
//#include "clock.h"
//#include "powerControl.h"


void SyncObj_memcpy_safe(BYTE* pDestination, const BYTE* pSource, int size)
{
	SAFE_DECLARE;
	DISABLE_VIC;
	memcpy(pDestination, pSource, size);
	ENABLE_VIC;
}


DWORD SAFE_GET_DWORD(DWORD volatile *pVariable)
{
	SAFE_DECLARE;
	DWORD value;
	DISABLE_VIC;
	value = *pVariable;
	ENABLE_VIC;
	return value;
}


WORD SAFE_GET_WORD(WORD volatile *pVariable)
{
	SAFE_DECLARE;
	WORD value;
	DISABLE_VIC;
	value = *pVariable;
	ENABLE_VIC;
	return value;
}


BYTE SAFE_GET_BYTE(BYTE volatile *pVariable)
{
	SAFE_DECLARE;
	BYTE value;
	DISABLE_VIC;
	value = *pVariable;
	ENABLE_VIC;
	return value;
}




//copy uart data to safe place
void SyncObj_copyUARTtoSafePlace(struct tagUART * pUart)
{
	SAFE_DECLARE;
	DISABLE_VIC;
	memcpy((void*)pUart->rcvBuff_safe, (void*)pUart->rcvBuff, pUart->constRcvBuffLen);
	pUart->rcvBuffLen_safe = pUart->rcvBuffLen;
	pUart->bRcvError_safe = pUart->bRcvError;
	pUart->bDataReceived_safe = pUart->bDataReceived;
	pUart->bDataReceived = 0;	//reset it now	//17/01/08	because of we would proceed all time the same buffer
	ENABLE_VIC;
}
