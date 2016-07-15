#ifndef _USBRS_H
#define _USBRS_H


#include <iolpc2388.h>
#include "types.h"


//address in command to second proc from usb
#define USBRS_ADDRESS 0x02


#define USB_RCV_BUF_LEN 1024
#define USB_TRM_BUF_LEN (1024+16)

#define RS_BUFF_SIZE (USB_TRM_BUF_LEN-16)



struct tagUSBRSControl
{
	BOOL bUSBRS_used;	//have any transmittion
	BOOL bShow_USBRS_sym;	//symbol status

//#pragma pack(1)
	volatile BYTE rcvBuff[USB_RCV_BUF_LEN+16];
	volatile BYTE trmBuff[USB_TRM_BUF_LEN];
	BYTE rcvBuff_safe[USB_RCV_BUF_LEN+16];
//#pragma pack()


	struct tagUART uart;
	
	BOOL bSysExecution;	//if TRUE then there is a sys execution command and we dont need to switch to USBRS_Mode
	
	BOOL bBridgeMode;	//true if bridge mode from USB to bluetooth
};

extern struct tagUSBRSControl USBRSControl;

extern const unsigned char auchCRCHi[];
extern const unsigned char auchCRCLo[];



//BOOL USBRS_isTurnedON(void);


//void USBRS_show_symbol(int x);

void USBRS_Init(void);

void USBRS_control(void);
void USBRS_sym_control(void);

void USBRS_UART0_Init(void);

__arm void _INT_UART0_USBRS(void);

void USBRS_turnON(void);
void USBRS_turnOFF(void);
void USBRS_rcvData_first_Dispatcher(void);



void USBRS_waitTrmEnd(void);


void USBRS_StartTrm(void);
int USBRS_calcCRC(BYTE volatile * pData, int len);

BOOL USBRS_checkCRC(const BYTE volatile * pData, int len);


void USBRS_show_data(void);
WORD USBRS_CRC (BYTE Sbyte, WORD CRCbuf);
void USBRS_checkDataReceived_intcall(struct tagUART * pUart);
void USBRS_sendSequence(int len);

void USBRS_answer_first_Dispatcher(void);

void USBRS_rcvData_second_Dispatcher(struct tagUART * pUart);

void USBRS_readDir(struct tagUART * pUart);
void USBRS_readFile(struct tagUART * pUart);
void USBRS_writeFile(struct tagUART * pUart);
void USBRS_createFile(struct tagUART * pUart);
void USBRS_deleteFile(struct tagUART * pUart);

void USBRS_except(BYTE exceptCode, struct tagUART * pUart);

void USBRS_getFileNum(struct tagUART * pUart);
void USBRS_executeFile(struct tagUART * pUart);

void USBRS_readRefSpec(struct tagUART * pUart);
void USBRS_readIDData(struct tagUART * pUart);

int USBRS_calcCRCEx(BYTE volatile * pData, int len);

void USBRS_InitInBridgeMode(void);

void USBRS_readDataReg(struct tagUART * pUart);



void putFLOAT(void* pBuffer, float val);

void putUSHORT(void* pBuffer, unsigned short val);

void putULONG(void* pBuffer, unsigned long val);

void USBRS_readBinSig(struct tagUART * pUart);

#endif //#ifndef _USBRS_H
