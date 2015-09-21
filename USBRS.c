//USBRS.c


#include <stdio.h>
//#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <string.h>

#include "types.h"
#include "syncObj.h"
#include "powerControl.h"
#include "display.h"
#include "spectrum.h"
#include "USBRS.h"
#include "interrupts.h"
#include "interProc.h"
#include "USB_mode.h"
#include "filesystem.h"
#include "Setup_Mode.h"
#include "bluetooth.h"
#include "COMM_mode.h"
#include "identify.h"

struct tagUSBRSControl USBRSControl;


   /* Table of CRC values for high-order byte */
   const unsigned char auchCRCHi[] = {
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
   0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
   0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
   0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
   0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
   0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
   0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
   0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
   0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
   0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
   } ;

   /* Table of CRC values for low-order byte */
   const unsigned char auchCRCLo[] = {
   0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
   0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
   0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
   0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
   0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
   0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
   0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
   0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
   0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
   0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
   0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
   0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
   0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
   0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
   0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
   0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
   0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
   0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
   0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
   0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
   0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
   0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
   0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
   0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
   0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
   0x43, 0x83, 0x41, 0x81, 0x80, 0x40
   } ;






//прерывание приема передачи
__arm void _INT_UART0_USBRS(void)
{
	DWORD dw1,dw2;
	BYTE byt;
	if(USBRSControl.uart.bRcvError != RCV_OK)
	{//ошибка обмена
		//сброс ошибки и буфера приема
		USBRSControl.uart.rcvBuffLen = 0;
		USBRSControl.uart.bRcvError = RCV_OK;
	}
	while((U0LSR&0x1) || ((U0IIR&0x0c)==0x0c))
	{//DR
		byt = U0RBR;

		if(USBRSControl.bBridgeMode)
		{//bridge mode
			bluetoothControl.uart.trmBuffLen = bluetoothControl.uart.trmBuffLenConst = 0;
			U3THR = byt;
			COMMModeControl.dwTransmitted++;
			USBModeControl.dwReceived++;
			Display_flashOrangeLED();
			
/*			if(USBRSControl.uart.rcvBuffLen < USBRSControl.uart.constRcvBuffLen)
			{
				USBModeControl.dwReceived++;
				USBRSControl.uart.rcvBuff[USBRSControl.uart.rcvBuffLen++] = byt;
				if(USBRSControl.uart.rcvBuffLen>4)
				{
					dw1 = (USBRSControl.uart.rcvBuff[4]<<8);
					dw1 |= USBRSControl.uart.rcvBuff[3];
					if(USBRSControl.uart.rcvBuffLen-7>=dw1)
					{
						USBRSControl.uart.bDataReceived=1;
						Display_flashOrangeLED();
					}
				}
			}*/
		}else
		{
			if(USBRSControl.uart.rcvBuffLen < USBRSControl.uart.constRcvBuffLen)
			{
				if((USBRSControl.uart.rcvBuffLen>=1) ||
				   (USBRSControl.uart.rcvBuffLen==0 && (byt==INTERPROC_ADDRESS || byt==USBRS_ADDRESS)))
				{
					USBModeControl.dwReceived++;
					USBRSControl.uart.rcvBuff[USBRSControl.uart.rcvBuffLen++] = byt;
					if(USBRSControl.uart.rcvBuffLen>=4)
					{
						USBRS_checkDataReceived_intcall(&USBRSControl.uart);
					}
				}
			}else
			{//error receiver buffer len
				USBRSControl.uart.bRcvError = RCV_ERR_BUF_OVERFLOW;
			}
			
			if(USBRSControl.uart.bRcvError == RCV_ERR_CRC)
			{
				USBRSControl.uart.rcvBuffLen = 0;
				USBRSControl.uart.bRcvError = RCV_OK;
			}

			//find pocket start
			if(USBRSControl.uart.rcvBuffLen>=2 && USBRSControl.uart.bRcvError != RCV_OK)
			{//move pocket to find start byte: address then check for command if amount of bytes enough
				do
				{
					USBRSControl.uart.rcvBuffLen--;
					memmove((void*)&USBRSControl.uart.rcvBuff[0],
							(void const*)&USBRSControl.uart.rcvBuff[1],USBRSControl.uart.rcvBuffLen);
					if((USBRSControl.uart.rcvBuff[0]==INTERPROC_ADDRESS ||
					   USBRSControl.uart.rcvBuff[0]==USBRS_ADDRESS))
					{
						USBRSControl.uart.bRcvError = RCV_OK;
						if(USBRSControl.uart.rcvBuffLen>=4)
						{
							USBRS_checkDataReceived_intcall(&USBRSControl.uart);
						}
						break;
					}
				}while(USBRSControl.uart.rcvBuffLen>=2);
			}
		}
	};
	if(U0LSR&0x20)
	{//THRE
		dw1 = USBRSControl.uart.trmBuffLen;
		dw2 = USBRSControl.uart.trmBuffLenConst;
		if(dw1 < dw2)
		{//передача байта
			USBModeControl.dwTransmitted++;
			byt = USBRSControl.uart.trmBuff[USBRSControl.uart.trmBuffLen++];
			U0THR = byt;
		}else
		{//все передали, просто ничего не делаем
			;//прерывания не будут генериться
			USBRSControl.uart.bTrmReady = 1;
		}
	}
	if(U0LSR&0x0e)
	{
		USBRSControl.uart.bRcvError = RCV_ERR_BYTE;
	}
}







//was received all data
//only fo use in interrupts
void USBRS_checkDataReceived_intcall(struct tagUART * pUart)
{
	BYTE bt1, bt2;
	DWORD bytes=4;
	DWORD rlen = pUart->rcvBuffLen;
	if(pUart->rcvBuff[0]==USBRS_ADDRESS)
	{//commands to SPRD
		//анализируем количество байт
		switch(pUart->rcvBuff[1])
		{
		case 0x51:	//Считать список файлов
			bytes+=4;
			break;
		case 0x52:	//Считать файл
			bytes+=8;
			break;
		case 0x53:	//Записать файл
			bt1 = pUart->rcvBuff[8];
			bt2 = pUart->rcvBuff[9];
			if(rlen>=10)
				bytes+=8+bt1*256+bt2;
			else
				bytes = 255;
			break;
		case 0x54://Создать файл
		case 0x56://get file num 
			bytes+=19;
			break;
		case 0x55://Удалить файл
			bytes+=2;
			break;
		case 0x57://execute file "execute.sys"
			bytes+=0;
			break;
		default:
			bytes = 0;
			pUart->bRcvError=RCV_ERR_BYTE;
		}
	}else if(pUart->rcvBuff[0]==INTERPROC_ADDRESS)
	{//commands to BDKG
		//анализируем количество байт
		switch(pUart->rcvBuff[1])
		{
		case 3:
		case 2:
		case 4:
		case 5:
		case 6:
		case 8:
			bytes+=4;
			break;
		case 9:
		case 10:
		case 11:
		case 18:
		case 19:
		case 16:	//это должно быть строго здесь!
			bytes += pUart->rcvBuff[2]+1;
			break;
//			bytes+=4;
	//		break;
		case 17:
		case 7:
		case 0x40:
		case 0x41:
			bytes+=0;
			break;
		case 12:
		case 13:
			bytes+=2;
			break;
		default:
			bytes = 0;
			pUart->bRcvError=RCV_ERR_BYTE;
		}
	}else
		pUart->bRcvError=RCV_ERR_BYTE;
	
	if(bytes==rlen)
	{
		//check CRC
		if(!USBRS_checkCRC(pUart->rcvBuff,rlen))
		{//error CRC
			pUart->bRcvError=RCV_ERR_CRC;
		}else
		{
			pUart->bDataReceived=1;
		}
	}else if(rlen>bytes)
	{
		pUart->bRcvError=RCV_ERR_BYTE;
	}
}






//send any sequence of bytes
void USBRS_sendSequence(int len)
{
	if(len<1 || len>USBRSControl.uart.constTrmBuffLen)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Send sequence is too long");
		return;
	}
	
	//assume that transmiter is ready
	SAFE_DECLARE;
	DISABLE_VIC;
	USBRSControl.uart.bTrmReady = 0;
	
	if(USBRSControl.bBridgeMode)//in bridge mode do not calc CRC
		USBRSControl.uart.trmBuffLenConst = len;
	else
		USBRSControl.uart.trmBuffLenConst = USBRS_calcCRC(USBRSControl.uart.trmBuff, len);
	
	USBRSControl.uart.trmBuffLen = 1;
	USBRSControl.uart.rcvBuffLen = 0;
	USBRSControl.uart.bRcvError = RCV_OK;
	USBRSControl.uart.bDataReceived = 0;
	//start transmition data from first byte in buffer
	U0THR = USBRSControl.uart.trmBuff[0];
	USBModeControl.dwTransmitted++;
	ENABLE_VIC;
}


//show USBRS symbol in status lnie
/*
void USBRS_show_symbol(int x)
{
	if(USBRSControl.bShow_USBRS_sym)
		Display_output_bmp(x,304, (BITMAPFILEHEADER*)bmp_usbrs);
	else
	{
		RECT rect = {x,304,x+23,Y_SCREEN_MAX};
		Display_clearRect(rect, 100);
	}
}*/

//control USBRS status and data
void USBRS_control(void)
{
	//смотрим на полученные данные
	SyncObj_copyUARTtoSafePlace(&USBRSControl.uart);
	//call first dispatcher
	if(USBRSControl.uart.bDataReceived_safe)
	{//must process received data
//		if(modeControl.pMode != &modes_USBMode && !USBRSControl.bSysExecution && USBModeControl.bEnterThisMode)
	//	{//switch to USB mode when first command received and we are no in this mode
		//	Modes_setActiveMode(&modes_USBMode);
//		}

		//first process, the results must be processed in the procedures of initiate transmition command
		//no cmd should be prepared in trm buffer if rcv data is not processed here!!!!!!!!!
		USBRS_rcvData_first_Dispatcher();
		USBRSControl.bUSBRS_used = 1;	//mark USB as used
		powerControl.dwIdleTime = 0;	//reset idle time counter after key processing
	}
}









//first dispatcher of received data
void USBRS_rcvData_first_Dispatcher(void)
{
/*	if(USBRSControl.bBridgeMode)
	{//bridge mode
		if(USBRSControl.uart.rcvBuffLen_safe>0 && USBRSControl.uart.rcvBuffLen_safe<BTH_TRM_BUF_LEN)
		{//len if valid
			memcpy((void*)bluetoothControl.uart.trmBuff,USBRSControl.uart.rcvBuff_safe,USBRSControl.uart.rcvBuffLen_safe);
			Bluetooth_sendSequence(USBRSControl.uart.rcvBuffLen_safe);
			Display_flashRedLED();
			SAFE_DECLARE;
			DISABLE_VIC;
			USBRSControl.uart.trmBuffLen = 0;
			USBRSControl.uart.rcvBuffLen = 0;
			USBRSControl.uart.bRcvError = RCV_OK;
			USBRSControl.uart.bDataReceived = 0;
			ENABLE_VIC;
		}
	}else*/
	{
		if(USBRSControl.uart.rcvBuff_safe[0]==INTERPROC_ADDRESS
			   //21/01/2010
			   && USBRSControl.uart.rcvBuff_safe[1]!=0x09  /*it is not a command of read of eeprom spectrum*/
			   && USBRSControl.uart.rcvBuff_safe[1]!=0x11  /*it is not a command of read of ID data*/)
			  ///////////
		{//command to the second processor
			//translate it to second proc
			
			//have to check it before send else will receive exception
			int maxIndex = -1;
			//here store index of first empty buff
			int indexIndex = -1;
			InterProc_findFreeSlot(&indexIndex, &maxIndex);
			if(indexIndex==-1 || USBRSControl.uart.rcvBuffLen_safe>interProcControl.uart.constTrmBuffLen ||
			   USBRSControl.uart.rcvBuffLen_safe==0)
			{
				return;	//error in cmd, ignore it
			}
	
			//change address to resolve answers from first proc and usb
			USBRSControl.uart.rcvBuff_safe[0]=USBRS_ADDRESS;
			InterProc_fillNewCmd(USBRSControl.uart.rcvBuff_safe, USBRSControl.uart.rcvBuffLen_safe-2);
		}else if(USBRSControl.uart.rcvBuff_safe[0]==USBRS_ADDRESS
					 //21/01/2010
					|| USBRSControl.uart.rcvBuff_safe[1]==0x09  /*it is a command of read of eeprom spectrum*/
					|| USBRSControl.uart.rcvBuff_safe[1]==0x11  /*it is a command of read of ID data*/)
					///////////
		{//command to this processor
			USBRS_rcvData_second_Dispatcher(&USBRSControl.uart);
			if(USBRSControl.uart.trmBuffLenConst>=2 && USBRSControl.uart.trmBuffLenConst<USBRSControl.uart.constTrmBuffLen)
				USBRS_sendSequence(USBRSControl.uart.trmBuffLenConst);
		}
	}
}


//process answer from second proc and resent it to usb
void USBRS_answer_first_Dispatcher(void)
{
	if(interProcControl.uart.rcvBuffLen_safe<2 || interProcControl.uart.rcvBuffLen_safe>USBRSControl.uart.constTrmBuffLen)
		return;//error in answer from second proc
	//change address of answer from second proc to be resolved by PC
	interProcControl.uart.rcvBuff_safe[0]=INTERPROC_ADDRESS;
	memcpy((void*)USBRSControl.uart.trmBuff,(void*)interProcControl.uart.rcvBuff_safe,interProcControl.uart.rcvBuffLen_safe-2);
	USBRS_sendSequence(interProcControl.uart.rcvBuffLen_safe-2);
}
















//control USB symbol
void USBRS_sym_control(void)
{
	//управление отображением символа ГПС, выкл, вкл, мерцание
	//мерцание когда вкл но не fix
	USBRSControl.bShow_USBRS_sym=USBRSControl.bUSBRS_used;
	USBRSControl.bUSBRS_used = 0;	//mark as unused USB
}


//init USB in bridge mode 115200 baud
void USBRS_InitInBridgeMode(void)
{
	USBRSControl.bBridgeMode = TRUE;
	U0LCR = 0x83;        //LCR_ENABLE_LATCH_ACCESS;
	DWORD div = HW_FREQ/115200/16;
	U0DLM = 0x00;
	U0DLL = div;//((HW_FREQ*1000) / (SpeedRS232*16)); //0x2;  //115200 ->8/4 //надо 0x0A/4->18432000/(16*115200)
	U0LCR =0x3; //LCR_DISABLE_LATCH_ACCESS;
}

//init USBRS
void USBRS_Init(void)
{
	USBRSControl.uart.rcvBuff = USBRSControl.rcvBuff;
	USBRSControl.uart.rcvBuff_safe = USBRSControl.rcvBuff_safe;
	USBRSControl.uart.trmBuff = USBRSControl.trmBuff;
	USBRSControl.uart.constRcvBuffLen = sizeof(USBRSControl.rcvBuff);
	USBRSControl.uart.constTrmBuffLen = sizeof(USBRSControl.trmBuff);
	
	USBRSControl.bBridgeMode = FALSE;
	//adjust on output
	USBRSControl.bUSBRS_used = 0;
	USBRSControl.bShow_USBRS_sym = 0;
	for(int i=0;i<USBRSControl.uart.constTrmBuffLen;i++)
	{
		USBRSControl.uart.trmBuff[i] = 0;
	}
	for(int i=0;i<USBRSControl.uart.constRcvBuffLen;i++)
	{
		USBRSControl.uart.rcvBuff[i] = 0;
		USBRSControl.uart.rcvBuff_safe[i] = 0;
	}
	USBRSControl.uart.rcvBuffLen = 0;	//must be reset to 0 when start transmition
	USBRSControl.uart.bRcvError = RCV_OK;	//must be reset when start transmition
	USBRSControl.uart.bDataReceived = 0;
	USBRSControl.uart.trmBuffLen = 0;	
	USBRSControl.uart.trmBuffLenConst = 0;
	USBRSControl.bSysExecution = FALSE;
}


void USBRS_UART0_Init(void)
{

	int SpeedRS232;
	
	SpeedRS232 = 288000;   //скорость обмена
	
	
	PCONP_bit.PCUART0 = 1;	//give power to UART0

	PINSEL0_bit.P0_2 = 0x01;	//select pins for UART0
	PINSEL0_bit.P0_3 = 0x01;
	PINMODE0_bit.P0_2 = 0x0;
	PINMODE0_bit.P0_3 = 0x0;
	
	// enable access to divisor latch regs
	U0IER = 0;	//disable int
	
	

	
	U0LCR = 0x83;        //LCR_ENABLE_LATCH_ACCESS;
	
	// set divisor for desired baud
	//divider = HW_FREQ/UART_baudrate/16
	DWORD div = HW_FREQ/SpeedRS232/16;
	U0DLM = 0x00;
	U0DLL = div;//((HW_FREQ*1000) / (SpeedRS232*16)); //0x2;  //115200 ->8/4 //надо 0x0A/4->18432000/(16*115200)
	//U0FCR =0x07; //1byte, clear FIFO
	// disable access to divisor latch regs (enable access to xmit/rcv fifos
	// and int enable regs)
	U0LCR =0x3; //LCR_DISABLE_LATCH_ACCESS;
	// setup fifo control reg - trigger level 0 (1 byte fifos), no dma
	// disable fifos (450 mode) прерывание по приему 1-го байта
	
	__uartfcriir_bits u0fcr;
	u0fcr.FCRFE=1;
	u0fcr.RFR=1;
	u0fcr.TFR=1;
	u0fcr.RTLS=3;//пачками по 14 байт
	U0FCR_bit =u0fcr;
	
	// enable UART0 interrupts
	U0IER = 0x7;	//enable RBR = 1, THRE = 1, Status Rx
	
}

//take power off and turn off UART
void USBRS_turnOFF(void)
{
	PCONP_bit.PCUART0 = 0;	//take power from UART0
}

//give power and adjust UART
void USBRS_turnON(void)
{
	USBRS_UART0_Init();
}





//ret len in bytes
int USBRS_calcCRC(BYTE volatile * pData, int len)
{
	WORD CRCbuf = 0xffff;
	int i = 0;
	for(; i<len; i++)
	{
		CRCbuf = USBRS_CRC(pData[i], CRCbuf);
	}
	pData[i++] = LOBYTE(CRCbuf);
	pData[i++] = LO2BYTE(CRCbuf);
	return i;
}


//ret len in bytes
int USBRS_calcCRCEx(BYTE volatile * pData, int len)
{
	BYTE CRCbuf = 0;
	int i = 1;
	for(; i<=4; i++)
	{
		CRCbuf += pData[i];
	}
	pData[5] = CRCbuf;
	pData[len] = 0x03;
	return len+1;
}


//check CRC
//ret !=0 if OK
BOOL USBRS_checkCRC(const BYTE volatile * pData, int len)
{
	BYTE bt1,bt2;
	WORD CRCbuf = 0xffff;
	int i = 0;
	for(; i<len-2; i++)
	{
		CRCbuf = USBRS_CRC(pData[i], CRCbuf);
	}
	bt1 = pData[len-1];
	bt2 = pData[len-2];
	return (CRCbuf==((bt1<<8)|(bt2)));
}



// Процедура вычисления кода циклического контроля.
//         Образующий полином  x^16 + x^14 + x + 1.
//     Входной параметр - очередной передаваемый/принятый байт.
//     CRC16 накапливается в CRCbuf.
WORD USBRS_CRC (BYTE Sbyte, WORD CRCbuf)
{
	unsigned char uIndex ;			
	uIndex = (LO2BYTE(CRCbuf)) ^ Sbyte ;	
	*((unsigned char *)&CRCbuf+1) = (unsigned char)(LOBYTE(CRCbuf)) ^ auchCRCHi[uIndex] ;
	*((unsigned char *)&CRCbuf) = auchCRCLo[uIndex] ;
	return CRCbuf;
}

//second dispatcher is used to understand commands to SPIRID
void USBRS_rcvData_second_Dispatcher(struct tagUART * pUart)
{
	pUart->trmBuff[0] = pUart->rcvBuff_safe[0];
	pUart->trmBuff[1] = pUart->rcvBuff_safe[1];
	switch(pUart->rcvBuff_safe[1])
	{
	  //21/01/2010
		case 0x09:
			USBRS_readRefSpec(pUart);
			break;
		case 0x11:
			USBRS_readIDData(pUart);
			break;
	///////////
		case 0x51:	//read directory
			USBRS_readDir(pUart);
			break;
		case 0x52:	//read file
			USBRS_readFile(pUart);
			break;
		case 0x53:	//write file
			USBRS_writeFile(pUart);
			break;
		case 0x54:	//create file
			USBRS_createFile(pUart);
			break;
		case 0x55:	//delete file
			USBRS_deleteFile(pUart);
			break;
		case 0x56:	//get file num
			USBRS_getFileNum(pUart);
			break;
		case 0x57:	//execute file "execute.sys"
			USBRS_executeFile(pUart);
			break;
	}
	if(pUart->rcvBuff_safe[1]==0x51)
	   USBRSControl.bSysExecution = FALSE;//cmd read dir make turn off execution mode
	else if(pUart->rcvBuff_safe[1]==0x57)
	   USBRSControl.bSysExecution = TRUE;	//if execution cmd then dont switch to USBRS_Mode
}

//read dir
void USBRS_readDir(struct tagUART * pUart)
{
	WORD adr = ((WORD)pUart->rcvBuff_safe[2]<<8)|pUart->rcvBuff_safe[3];
	if(adr>=MAX_FILES)
	{
		USBRS_except(2, pUart);
		return;
	}
	WORD recs = ((WORD)pUart->rcvBuff_safe[4]<<8)|pUart->rcvBuff_safe[5];
	if(recs>(RS_BUFF_SIZE/sizeof(struct tagFileRecord)) || recs<1)
	{
		USBRS_except(3, pUart);
		return;
	}
	for(int i=0;i<recs;i++)
	{
		int index = filesystem_get_filerecordIndex(adr+i);
		if(index==E_FAIL)
		{
			USBRS_except(4, pUart);
			return;
		}
		memcpy((void*)&pUart->trmBuff[4+i*sizeof(struct tagFileRecord)],
			   (void*)&filesystem.fileRecordsOnSector[index],
			   sizeof(struct tagFileRecord));
	}
	int len = recs*sizeof(struct tagFileRecord);
	pUart->trmBuff[2] = HIBYTE_W(len);
	pUart->trmBuff[3] = LOBYTE(len);
	pUart->trmBuffLenConst = 4+len;
}


//21/01/2010
//read reference spectrum as a spectrum from DU
void USBRS_readRefSpec(struct tagUART * pUart)
{
	if(pUart->rcvBuff_safe[2]!=6)
	{//number of bytes in the request is invalid
		USBRS_except(2, pUart);
		return;
	}

	WORD fnum = ((WORD)pUart->rcvBuff_safe[3]<<8)|pUart->rcvBuff_safe[4];
	
	DWORD flen = (CHANNELS)*3+256;
	DWORD adr = ((DWORD)pUart->rcvBuff_safe[5]<<8)|((DWORD)pUart->rcvBuff_safe[6]);
	if(adr>=flen)
	{//out of spectrum
		USBRS_except(2, pUart);
		return;
	}
	
	WORD bytes = ((WORD)pUart->rcvBuff_safe[7]<<8)|pUart->rcvBuff_safe[8];
	if(bytes>RS_BUFF_SIZE || bytes<1 || (adr+bytes)>flen)
	{//out of spectrum
		USBRS_except(3, pUart);
		return;
	}


	int j, k;
	WORD wrd;
	for(int i=0;i<bytes;i++)
	{
	  	j=(adr+i)/3;
		k=(adr+i)%3;
		
		if(fnum==2099 && j<CHANNELS)//energy
			wrd = spectrumControl.warEnergy[j];
		else if(fnum==2097 && j<CHANNELS)//sigma
			wrd = spectrumControl.warSigma[j]
#ifdef _THIN_SIGMA
				*SIGMA_THIN_FACTOR_M/SIGMA_THIN_FACTOR_D
#endif	//#ifdef _THIN_SIGMA
					;
		else
			wrd = 0;
		
		if(k==0)
			pUart->trmBuff[3+i]=0;	//VERYHIGH
		else if(k==1)
			pUart->trmBuff[3+i]=LO2BYTE(wrd);	//HIGH
		else if(k==2)
			pUart->trmBuff[3+i]=LOBYTE(wrd);	//LOW
	}	
	
	pUart->trmBuff[2] = bytes;
	pUart->trmBuffLenConst = 3+bytes;
}
////////////////

//08/02/2010
//read ID data
void USBRS_readIDData(struct tagUART * pUart)
{
	pUart->trmBuff[2] = 0x12;
	pUart->trmBuff[3] = 0xBC;
	pUart->trmBuff[4] = 0xFF;
	pUart->trmBuff[5] = HIBYTE(SETUPModeControl.Serial);
	pUart->trmBuff[6] = LO3BYTE(SETUPModeControl.Serial);
	pUart->trmBuff[7] = LO2BYTE(SETUPModeControl.Serial);
	pUart->trmBuff[8] = LOBYTE(SETUPModeControl.Serial);
	pUart->trmBuff[9] = 0x04;
	pUart->trmBuff[10] = 0x00;
	pUart->trmBuff[11] = SETUPModeControl.ManufacturedMonth;
	pUart->trmBuff[12] = LO2BYTE(SETUPModeControl.ManufacturedYear);
	pUart->trmBuff[13] = LOBYTE(SETUPModeControl.ManufacturedYear);
	pUart->trmBuff[14] = 0x00;
	pUart->trmBuff[15] = 0x00;
	pUart->trmBuff[16] = 0x00;
	pUart->trmBuff[17] = 0x00;
	pUart->trmBuff[18] = 0x00;
	pUart->trmBuff[19] = 0x00;
	pUart->trmBuff[20] = 0x00;
	pUart->trmBuffLenConst = 21;
}
////////////////



//read data from file
void USBRS_readFile(struct tagUART * pUart)
{
	WORD fnum = ((WORD)pUart->rcvBuff_safe[2]<<8)|pUart->rcvBuff_safe[3];
	if(fnum>=MAX_FILES)
	{//file number invalid
		USBRS_except(2, pUart);
		return;
	}
	HFILE hfile = (HFILE)(fnum+1);
	if(!filesystem_check_file_exists(hfile))
	{//no such file
		USBRS_except(2, pUart);
		return;
	}
	DWORD flen = filesystem_get_length(hfile);
	DWORD adr = ((DWORD)pUart->rcvBuff_safe[4]<<24)|((DWORD)pUart->rcvBuff_safe[5]<<16)|
		((DWORD)pUart->rcvBuff_safe[6]<<8)|pUart->rcvBuff_safe[7];
	if(adr>=flen)
	{//out of file
		USBRS_except(2, pUart);
		return;
	}
	WORD bytes = ((WORD)pUart->rcvBuff_safe[8]<<8)|pUart->rcvBuff_safe[9];
	if(bytes>RS_BUFF_SIZE || bytes<1 || (adr+bytes)>flen)
	{//out of file
		USBRS_except(3, pUart);
		return;
	}
	int ret = filesystem_file_get(hfile, /*file descriptor = number of file in file record table*/
							(int*)&adr, /*it is counter of read bytes, input: start from 0, as output ret pointer for the next non read byte*/
							  (BYTE*)&pUart->trmBuff[4], /*recepient buffer*/
							  bytes);/*input: buffer len in bytes, output: read len in bytes*/
	if(ret==E_FAIL)
	{
		USBRS_except(4, pUart);
		return;
	}
	pUart->trmBuff[2] = HIBYTE_W(ret);
	pUart->trmBuff[3] = LOBYTE(ret);
	pUart->trmBuffLenConst = 4+ret;
}


//write data to file
void USBRS_writeFile(struct tagUART * pUart)
{
	WORD fnum = ((WORD)pUart->rcvBuff_safe[2]<<8)|pUart->rcvBuff_safe[3];
	if(fnum>=MAX_FILES)
	{//file number invalid
		USBRS_except(2, pUart);
		return;
	}
	HFILE hfile = (HFILE)(fnum+1);
	
	DWORD adr = ((DWORD)pUart->rcvBuff_safe[4]<<24)|((DWORD)pUart->rcvBuff_safe[5]<<16)|
		((DWORD)pUart->rcvBuff_safe[6]<<8)|pUart->rcvBuff_safe[7];
	WORD bytes = ((WORD)pUart->rcvBuff_safe[8]<<8)|pUart->rcvBuff_safe[9];
	if(bytes>USB_RCV_BUF_LEN || bytes<1)
	{//out of file
		USBRS_except(3, pUart);
		return;
	}
	int ret = filesystem_file_put(hfile, /*file descriptor = number of file in file record table*/
							(int*)&adr, /*it is counter of read bytes, input: start from 0, as output ret pointer for the next non read byte*/
							  (BYTE*)&pUart->rcvBuff_safe[10], /*recepient buffer*/
							  bytes);/*input: buffer len in bytes, output: read len in bytes*/
	if(ret==E_FAIL)
	{
		USBRS_except(4, pUart);
		return;
	}
	pUart->trmBuffLenConst = 2;
}

//create file in filesystem
void USBRS_createFile(struct tagUART * pUart)
{
	char fname[FILE_NAME_SZ];
	memset(fname, 0, FILE_NAME_SZ);
	strncpy(fname, (char*)&pUart->rcvBuff_safe[2], FILE_NAME_SZ);
	char fext[FILE_EXT_SZ];
	memset(fext, 0, FILE_EXT_SZ);
	strncpy(fext, (char*)&pUart->rcvBuff_safe[2+FILE_NAME_SZ], FILE_EXT_SZ);
	HFILE hfile = filesystem_create_file(fname, fext, FALSE);
	if(hfile==NULL)
	{
		USBRS_except(4, pUart);
		return;
	}
	int idx = hfile-1;
	pUart->trmBuff[2] = HIBYTE_W(idx);
	pUart->trmBuff[3] = LOBYTE(idx);
	pUart->trmBuffLenConst = 4;
}


//delete file from filesystem
void USBRS_deleteFile(struct tagUART * pUart)
{
	WORD fnum = ((WORD)pUart->rcvBuff_safe[2]<<8)|pUart->rcvBuff_safe[3];
	if(fnum>=MAX_FILES)
	{//file number invalid
		USBRS_except(2, pUart);
		return;		
	}
	HFILE hfile = (HFILE)(fnum+1);
	filesystem_delete_file(hfile);
	pUart->trmBuffLenConst = 2;
}


//prepare exception reply
void USBRS_except(BYTE exceptCode, struct tagUART * pUart)
{
	pUart->trmBuff[1]|=0x80;
	pUart->trmBuff[2] = exceptCode;
	pUart->trmBuffLenConst = 3;
}




//get file number by name and extension for specified uart
void USBRS_getFileNum(struct tagUART * pUart)
{
	char fname[FILE_NAME_SZ];
	memset(fname, 0, FILE_NAME_SZ);
	strncpy(fname, (char*)&pUart->rcvBuff_safe[2], FILE_NAME_SZ);
	char fext[FILE_EXT_SZ];
	memset(fext, 0, FILE_EXT_SZ);
	strncpy(fext, (char*)&pUart->rcvBuff_safe[2+FILE_NAME_SZ], FILE_EXT_SZ);
	HFILE hfile = filesystem_find_file(fname, fext);
	int idx = hfile-1;
	pUart->trmBuff[2] = HIBYTE_W(idx);
	pUart->trmBuff[3] = LOBYTE(idx);
	pUart->trmBuffLenConst = 4;
}


//process of command of execution of execute.sys
//prepare answer and setup exec hfile
//after execution this file will be deleted
void USBRS_executeFile(struct tagUART * pUart)
{
	const char fname[]="execute";
	const char fext[]="sys";
	HFILE hfile = filesystem_find_file(fname, fext);
	if(hfile==NULL)
	{
		USBRS_except(3, pUart);
		return;		
	}
	filesystem.hFileExecuteSys = hfile;
	pUart->trmBuffLenConst = 2;
}


