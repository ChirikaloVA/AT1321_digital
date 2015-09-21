//second_proc_h


#include <iolpc2388.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "types.h"
#include "interProc.h"
#include "syncObj.h"
#include "display.h"
#include "interrupts.h"
#include "clock.h"
#include "spectrum.h"
#include "modes.h"
#include "sound.h"
#include "powerControl.h"
#include "SPRD_mode.h"
#include "TC_mode.h"
#include "USBRS.h"
#include "bluetooth.h"
#include "NMEA_Parser.h"


//some of usb crc procedures
int USBRS_calcCRC(BYTE volatile * pData, int len);
BOOL USBRS_checkCRC(const BYTE volatile * pData, int len);


struct tagInterProcControl interProcControl;

















//прерывание приема передачи
__arm void _INT_UART1_InterProc(void)
{
	BYTE byt;
	while((U1LSR&0x1) || ((U1IIR&0x0c)==0x0c))
	{//DR
		byt = U1RBR;
		if(interProcControl.uart.rcvBuffLen < interProcControl.uart.constRcvBuffLen)
		{
			InterProc_Timer_turnON_intcall();	//заводим таймер заново
			interProcControl.uart.rcvBuff[interProcControl.uart.rcvBuffLen++] = byt;
			if(interProcControl.uart.rcvBuffLen>=5)
			{
				InterProc_checkDataReceived_intcall();
			}
		}else
		{//error receiver buffer len
			interProcControl.uart.bRcvError = RCV_ERR_BUF_OVERFLOW;
		}
	};
	if(U1LSR&0x20)
	{//THRE
		if(interProcControl.uart.trmBuffLen <  interProcControl.uart.trmBuffLenConst)
		{//передача байта
			U1THR = interProcControl.uart.trmBuff[interProcControl.uart.trmBuffLen++];
		}else
		{//все передали, просто ничего не делаем
			;//прерывания не будут генериться
		}
	}
	if(U1LSR&0x0e)
	{
		interProcControl.uart.bRcvError = RCV_ERR_BYTE;
	}
}


//was received all data
//only fo use in interrupts
void InterProc_checkDataReceived_intcall(void)
{
	//анализируем количество ответных байт
	DWORD rlen = interProcControl.uart.rcvBuffLen;
	DWORD bytes=5;
	switch(interProcControl.uart.rcvBuff[1])
	{
	case 3:
	case 2:
	case 4:
	case 9:
	case 10:
	case 11:
	case 17:
	case 18:
	case 19:
		bytes += interProcControl.uart.rcvBuff[2];
		break;
	case 6:
	case 5:
	case 8:
	case 16:
		bytes+=3;
		break;
	case 7:
		bytes+=0;
		break;
	case 12:
	case 13:
		bytes+=1;
		break;
	case 0x40:
	case 0x41:
		bytes+=interProcControl.uart.rcvBuff[2]*256;
		bytes+=interProcControl.uart.rcvBuff[3];
		bytes+=1;
		break;
	default:
		bytes = rlen;
	}
	if(rlen>=bytes)
	{
		interProcControl.uart.rcvBuffLen = bytes;
		InterProc_Timer_turnOFF();	//выключим таймер ожидания первых байт
		//check CRC
		if(!USBRS_checkCRC(interProcControl.uart.rcvBuff,bytes))
		{//error CRC
			interProcControl.uart.bRcvError=RCV_ERR_CRC;
		}else
		{
			interProcControl.uart.bDataReceived=1;
		}
		interProcControl.bTimeOutReached = TRUE;
	}
}



//send any sequence of bytes
void InterProc_sendSequence(int len)
{
	if(len>interProcControl.uart.constTrmBuffLen)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Send sequence is too long");
		return;
	}
	
	//assume that transmiter is ready
	SAFE_DECLARE;
	DISABLE_VIC;
	
	memset((void*)interProcControl.uart.rcvBuff, 0xff, interProcControl.uart.constRcvBuffLen);
	interProcControl.uart.bTrmReady = 0;
	interProcControl.uart.trmBuffLenConst = USBRS_calcCRC(interProcControl.uart.trmBuff, len);
	interProcControl.uart.trmBuffLen = 1;
	interProcControl.uart.rcvBuffLen = 0;
	interProcControl.uart.bRcvError = RCV_OK;
	interProcControl.bTimeOutReached = FALSE;
	interProcControl.uart.bDataReceived = 0;
	//start transmition data from first byte in buffer
	U1THR = interProcControl.uart.trmBuff[0];
	InterProc_Timer_turnON_intcall();
	ENABLE_VIC;
}

//resend any sequence of bytes again
void InterProc_resendSequence(void)
{
	//assume that transmiter is ready
	SAFE_DECLARE;
	DISABLE_VIC;

	memset((void*)interProcControl.uart.rcvBuff, 0xff, interProcControl.uart.constRcvBuffLen);
	
	interProcControl.uart.bTrmReady = 0;
	interProcControl.uart.trmBuffLen = 1;
	interProcControl.uart.rcvBuffLen = 0;
	interProcControl.uart.bRcvError = RCV_OK;
	interProcControl.bTimeOutReached = FALSE;
	interProcControl.uart.bDataReceived = 0;
	//start transmition data from first byte in buffer
	U1THR = interProcControl.uart.trmBuff[0];
	InterProc_Timer_turnON_intcall();
	ENABLE_VIC;
}













/****************************************************************************/
void InterProc_UART1_Init(void)
{
	
	int SpeedRS232;
	
	SpeedRS232 = 288000;   //скорость обмена
	
	
	PCONP_bit.PCUART1 = 1;	//give power to UART2

	
	PINSEL0_bit.P0_15 = 0x01;	//select pins for UART2
	PINSEL1_bit.P0_16 = 0x01;
	
	// enable access to divisor latch regs
	U1IER = 0;	//disable int

	
	
	U1LCR = 0x83;        //LCR_ENABLE_LATCH_ACCESS;
	
	// set divisor for desired baud
	//divider = HW_FREQ/UART_baudrate/16
	DWORD div = HW_FREQ/SpeedRS232/16;
	U1DLM = 0x00;
	U1DLL = div;//((HW_FREQ*1000) / (SpeedRS232*16)); //0x2;  //115200 ->8/4 //надо 0x0A/4->18432000/(16*115200)
	// disable access to divisor latch regs (enable access to xmit/rcv fifos
	// and int enable regs)
	U1LCR =0x3; //LCR_DISABLE_LATCH_ACCESS;
	// setup fifo control reg - trigger level 0 (1 byte fifos), no dma
	// disable fifos (450 mode) прерывание по приему 1-го байта
	
	//U0FCR =0x07; //1byte, clear FIFO
	__uartfcriir_bits u1fcr;
	u1fcr.FCRFE=1;
	u1fcr.RFR=1;
	u1fcr.TFR=1;
	u1fcr.RTLS=3;//interrupt after each 14 bytes
	U1FCR_bit = u1fcr;
	
	__uartmcr_bits u1mcr;
	u1mcr.DTR = 0;
	u1mcr.RTS = 0;
	u1mcr.LMS = 0;
	u1mcr.RTSEN = 0;	
	u1mcr.CTSEN = 0;
	U1MCR_bit = u1mcr;
	
	// enable UART0 interrupts
	U1IER = 0x7;	//enable RBR = 1, THRE = 1, status int =1
	
	// setup line control reg - disable break transmittion, even parity,
	// 1 stop bit, 8 bit chars
	

}






//controlling inerproc communication process
//called from main cycle
void InterProc_InterProcControl(void)
{
	//copy data to safeplace
	SyncObj_copyUARTtoSafePlace(&interProcControl.uart);
	
	//call first dispatcher
	if(interProcControl.uart.bDataReceived_safe)
	{//must process received data
		//first process, the results must be processed in the procedures of initiate transmition command
		//no cmd should be prepared in trm buffer if rcv data is not processed here!!!!!!!!!
		if(interProcControl.uart.rcvBuff_safe[0]==INTERPROC_ADDRESS)
			InterProc_rcvData_first_Dispatcher();
		else if(interProcControl.uart.rcvBuff_safe[0]==USBRS_ADDRESS)
			USBRS_answer_first_Dispatcher();
		else if(interProcControl.uart.rcvBuff_safe[0]==BLUETOOTH_ADDRESS)
			Bluetooth_answer_first_Dispatcher();
		interProcControl.uart.bTrmReady = 1;
		interProcControl.uart.iTries = 0;	//reset tries counter
	}
	//process error
	if(interProcControl.uart.bRcvError_safe!=RCV_OK)
	{
		if(++interProcControl.uart.iTries>MAX_TRM_TRIES)	//inc tries counter
		{
			//решено убрать отсюда исключение для реализаии механизма нескольких повторов в процедуре проверки готовности к отправке
			//InterProc_isReadyToTransmit
			//exception because of receive error. must be removed in release version I think
			const char* const errDesc[]={"OK","RCV_ERR_TIMEOUT","RCV_ERR_BYTE","RCV_ERR_BUF_OVERFLOW","RCV_ERR_CRC"};
			exception(__FILE__,__FUNCTION__,__LINE__, errDesc[interProcControl.uart.bRcvError_safe]);
		}

		if(interProcControl.uart.trmBuff[0]==INTERPROC_ADDRESS)
			InterProc_resendSequence();//commands of interproc must be repeated
		else
		{
			interProcControl.uart.bTrmReady = 1;//commands from PC must not be repeated
			interProcControl.uart.iTries = 0;	//reset tries counter
		}
		
//it was disabled becasue of useage	InterProc_resendSequence
//		interProcControl.uart.bTrmReady = 1;	//to be sure that data will be transmitted next time
		
	}
	
	//here data must be send from the order
	if(interProcControl.uart.bTrmReady)
	{
		interProcControl.bTimeOutReached = FALSE;
		int len = InterProc_getNextCmd(&interProcControl.uart.trmBuff[0]);
		if(len>0)
		{
			InterProc_sendSequence(len);
		}
	}
	//second dispatcher
	//it analyzes received data that processed by first dispatcher
	InterProc_second_Dispatcher();
}





//timer of waiting of InterProc UART answer
//used to create system timer for OnTimer
//used to wait of answer of interproc communication
//used to stop vibra signal
__arm void _INT_Timer2_InterProc(void)
{
	__ir_bits stru = {0};
	//next int disable is used to avoid timer adjusting fauls in another interrupts
	if(T2IR_bit.MR1INT)
	{//timer for system reasons for OnTimer function
		//clear TIMER1 interrupt status
		DWORD ms = modeControl.dwTimerPeriod/INTERPROC_TIMER_VAL;
		T2MR1 = T2TC+ms;
		modeControl.dwTimerReach=1;	//timer is reached
		stru.MR1INT = 1;
	}
	if(T2IR_bit.MR0INT)
	{//timer for interproc comm
		if(!interProcControl.uart.bDataReceived)
		{//exception if no received
			interProcControl.uart.bRcvError = RCV_ERR_TIMEOUT;	//Receiving ERROR! no bytes we have
		}
		InterProc_Timer_turnOFF();
		interProcControl.bTimeOutReached = TRUE;
		stru.MR0INT = 1;
	}
	if(T2IR_bit.MR2INT)
	{//timer for vibra signals
		//clear TIMER1 interrupt status
		T2MCR_bit.MR2I = 0; //disable interrupt for vibra
		SoundControl_StopVibro();
		SouncControl_PlaySequence();
		stru.MR2INT = 1;
	}
	if(T2IR_bit.MR3INT)
	{
		//clear TIMER1 interrupt status
		T2MCR_bit.MR3I = 0; //disable interrupt for LED
		Display_turnOFFRedLED();
		stru.MR3INT = 1;
	}
	T2IR = *(ULONG*)&stru;
}




//init timer for interproc and system timer and vibra
void InterProc_Timer_init(void)
{
	PCONP_bit.PCTIM2 = 1;
	
	T2TCR_bit.CE = 0;
	__mcr_bits t2mcr;
	t2mcr.MR0I = 0; //disable interrupt
	t2mcr.MR0R = 0; //reset counter after reach comparator
	t2mcr.MR0S = 0; //dont stop timer after reach value
	t2mcr.MR1I = 0; //disable interrupt
	t2mcr.MR1R = 0; //reset counter after reach comparator
	t2mcr.MR1S = 0; //dont stop timer after reach value
	t2mcr.MR2I = 0; //disable interrupt
	t2mcr.MR2R = 0; //reset counter after reach comparator
	t2mcr.MR2S = 0; //dont stop timer after reach value
	t2mcr.MR3I = 0; //disable interrupt for LED
	t2mcr.MR3R = 0; //reset counter after reach comparator
	t2mcr.MR3S = 0; //dont stop timer after reach value
	T2MCR_bit = t2mcr;
	T2TCR_bit.CR = 1;	//reset timer
	T2TCR_bit.CR = 0;
	T2PR = HW_FREQ/1000*INTERPROC_TIMER_VAL-1;
	T2PC = 0;

	//adjust comparators 0
	T2MR0 = INTERPROC_WAIT_TIME/INTERPROC_TIMER_VAL-1;
	T2TCR_bit.CE = 1;//start right now
}




//включить таймер для обмена меж процами
void InterProc_Timer_turnON_intcall(void)
{
	InterProc_Timer_turnOFF();
	T2TCR_bit.CE = 0;
	T2MR0 = T2TC+INTERPROC_WAIT_TIME/INTERPROC_TIMER_VAL;
	T2MCR_bit.MR0I = 1; //enable interrupt for interproc
	T2TCR_bit.CE = 1;
}



//выключить таймер для обмена, он нам уже не нужен
void InterProc_Timer_turnOFF(void)
{
	T2MCR_bit.MR0I = 0; //disable interrupt for interproc
}





//first init vars of interproc
void InterProc_Init(void)
{
	memset(&interProcControl, 0, sizeof(interProcControl));
	
	interProcControl.uart.rcvBuff = interProcControl.rcvBuff;
	interProcControl.uart.rcvBuff_safe = interProcControl.rcvBuff_safe;
	interProcControl.uart.trmBuff = interProcControl.trmBuff;
	interProcControl.uart.constRcvBuffLen = sizeof(interProcControl.rcvBuff);
	interProcControl.uart.constTrmBuffLen = sizeof(interProcControl.trmBuff);


	interProcControl.rsModbus.wdPresetAcqTime=0;
	InterProc_resetSyncData(&interProcControl.rsModbus.swdPresetAcqTime);

	interProcControl.rsModbus.fStabGain=0;
	InterProc_resetSyncData(&interProcControl.rsModbus.swdStabGain);

	interProcControl.rsModbus.wdLowLimit=0;
	InterProc_resetSyncData(&interProcControl.rsModbus.swdLowLimit);

	interProcControl.rsModbus.wdHighLimit=0;
	InterProc_resetSyncData(&interProcControl.rsModbus.swdHighLimit);

/*	interProcControl.rsModbus.wdSigmaOper=0;
	interProcControl.rsModbus.wdSigmaSleep=0;
	InterProc_resetSyncData(&interProcControl.rsModbus.swdSigmaThresholds);*/

	//data resgisters
	interProcControl.rsModbus.wdAcqTime=0;
	InterProc_resetSyncData(&interProcControl.rsModbus.swdAcqTime);
	
	interProcControl.rsModbus.fMomCps=0;
	interProcControl.rsModbus.fCps=0;
	interProcControl.rsModbus.fCpsErr=200;
	interProcControl.rsModbus.fDoserate=0;
	interProcControl.rsModbus.fDoserateErr=0;
	interProcControl.rsModbus.fDose=0;
	interProcControl.rsModbus.sSigma=0;
	InterProc_resetSyncData(&interProcControl.rsModbus.swdMeasurementRegs);
	
	interProcControl.rsModbus.wdTemperature=0;
	InterProc_resetSyncData(&interProcControl.rsModbus.swdTemperature);
	
	//status register
	interProcControl.rsModbus.btStatus=0;
	InterProc_resetSyncData(&interProcControl.rsModbus.sbtStatus);
	
	//diagnostic register
	interProcControl.rsModbus.wdDiag=0;
	InterProc_resetSyncData(&interProcControl.rsModbus.swdDiag);
	
	//spectrum
	for(int i=0;i<CHANNELS;i++)
	{
		interProcControl.rsModbus.arSpectrum[i*3+0]=0;	//by 3 bytes on channel
		interProcControl.rsModbus.arSpectrum[i*3+1]=0;	//by 3 bytes on channel
		interProcControl.rsModbus.arSpectrum[i*3+2]=0;	//by 3 bytes on channel
	}
	InterProc_resetSyncData(&interProcControl.rsModbus.sarSpectrum);
	

	interProcControl.rsModbus.fBkgCPS = 0;
	InterProc_resetSyncData(&interProcControl.rsModbus.swdBkgCPS);
//	interProcControl.rsModbus.fBkgDR = 0;
//	InterProc_resetSyncData(&interProcControl.rsModbus.swdBkgDR);
	
	
	//init buffer and values
	interProcControl.uart.bTrmReady = 1;
	for(int i=0;i<interProcControl.uart.constTrmBuffLen;i++)
	{
		interProcControl.uart.trmBuff[i] = 0;
	}
	for(int i=0;i<interProcControl.uart.constRcvBuffLen;i++)
	{
		interProcControl.uart.rcvBuff[i] = 0;
		interProcControl.uart.rcvBuff_safe[i] = 0;
	}	
	interProcControl.uart.rcvBuffLen = 0;	//must be reset to 0 when start transmition
	interProcControl.uart.bRcvError = RCV_OK;	//must be reset when start transmition
	interProcControl.uart.bDataReceived = 0;
	interProcControl.uart.trmBuffLen = 0;	
	interProcControl.uart.trmBuffLenConst = 0;
	interProcControl.uart.iTries = 0;	//reset tries counter
	
	interProcControl.fTemperature =0;
	interProcControl.bTimeOutReached = FALSE;
	interProcControl.btStatus = 0;
}




//first dispatcher of received data
void InterProc_rcvData_first_Dispatcher(void)
{
	WORD adr;
	BYTE len;
	BYTE bt1, bt2;
	
	if(interProcControl.uart.rcvBuff_safe[1]&0x80)
	{//exception!!!!! process it
		char buf [40];
		sprintf(buf, "Exception %u in reply on %u",(UINT)interProcControl.uart.rcvBuff_safe[2],(UINT)(interProcControl.uart.rcvBuff_safe[1]&0x7f));
		exception(__FILE__,__FUNCTION__,__LINE__,buf);
	}else
	{
		//common processor of received data.
		//it sort data to their places in rsModbus and inc refreshing counters
		switch(interProcControl.uart.rcvBuff_safe[1])
		{
		case 0x07:	//status
			interProcControl.rsModbus.btStatus = interProcControl.uart.rcvBuff_safe[2];
			InterProc_iterateDataReady(&interProcControl.rsModbus.sbtStatus);
			break;
		case 0x04:	//data regs		//!!!!!! warning! by one register processing
			switch(interProcControl.uart.trmBuff[3])
			{
			case 0x00://live time of spectrum acquiring
				interProcControl.rsModbus.wdAcqTime = (interProcControl.uart.rcvBuff_safe[5]<<8) | interProcControl.uart.rcvBuff_safe[6];
				InterProc_iterateDataReady(&interProcControl.rsModbus.swdAcqTime);
				break;
			case 0x02://temperature
				interProcControl.rsModbus.wdTemperature = (interProcControl.uart.rcvBuff_safe[3]<<8) | interProcControl.uart.rcvBuff_safe[4];
				InterProc_iterateDataReady(&interProcControl.rsModbus.swdTemperature);
				break;
			case 0x04://momcps, here we know that we read many registers at once
				MAKEFLOAT(interProcControl.rsModbus.fMomCps,interProcControl.uart.rcvBuff_safe[6],interProcControl.uart.rcvBuff_safe[5],interProcControl.uart.rcvBuff_safe[4],interProcControl.uart.rcvBuff_safe[3]);
				MAKEFLOAT(interProcControl.rsModbus.fCps,interProcControl.uart.rcvBuff_safe[10],interProcControl.uart.rcvBuff_safe[9],interProcControl.uart.rcvBuff_safe[8],interProcControl.uart.rcvBuff_safe[7]);
				MAKEFLOAT(interProcControl.rsModbus.fCpsErr,interProcControl.uart.rcvBuff_safe[14],interProcControl.uart.rcvBuff_safe[13],interProcControl.uart.rcvBuff_safe[12],interProcControl.uart.rcvBuff_safe[11]);
				MAKEFLOAT(interProcControl.rsModbus.fDoserate,interProcControl.uart.rcvBuff_safe[18],interProcControl.uart.rcvBuff_safe[17],interProcControl.uart.rcvBuff_safe[16],interProcControl.uart.rcvBuff_safe[15]);
				MAKEFLOAT(interProcControl.rsModbus.fDoserateErr,interProcControl.uart.rcvBuff_safe[22],interProcControl.uart.rcvBuff_safe[21],interProcControl.uart.rcvBuff_safe[20],interProcControl.uart.rcvBuff_safe[19]);
				MAKEFLOAT(interProcControl.rsModbus.fDose,interProcControl.uart.rcvBuff_safe[26],interProcControl.uart.rcvBuff_safe[25],interProcControl.uart.rcvBuff_safe[24],interProcControl.uart.rcvBuff_safe[23]);
				interProcControl.rsModbus.sSigma = MAKESHORT(interProcControl.uart.rcvBuff_safe[28],interProcControl.uart.rcvBuff_safe[27]);
				InterProc_iterateDataReady(&interProcControl.rsModbus.swdMeasurementRegs);
				break;
			default://not supported register in answer!!!!!!!!
				exception(__FILE__,__FUNCTION__,__LINE__,"Unsupported data registers");
			}
			break;
		case 0x03:	//control regs		//!!!!!! warning! by one register processing only
			switch(interProcControl.uart.trmBuff[3])
			{
			case 0x00://preset time of spectrum acquiring
				interProcControl.rsModbus.wdPresetAcqTime = (interProcControl.uart.rcvBuff_safe[3]<<8) | interProcControl.uart.rcvBuff_safe[4];
				InterProc_iterateDataReady(&interProcControl.rsModbus.swdPresetAcqTime);
				break;
			case 22://gain
				MAKEFLOAT(interProcControl.rsModbus.fStabGain,interProcControl.uart.rcvBuff_safe[6],interProcControl.uart.rcvBuff_safe[5],interProcControl.uart.rcvBuff_safe[4],interProcControl.uart.rcvBuff_safe[3]);
//				interProcControl.rsModbus.wdGain = (interProcControl.uart.rcvBuff_safe[3]<<8) | interProcControl.uart.rcvBuff_safe[4];
				InterProc_iterateDataReady(&interProcControl.rsModbus.swdStabGain);
				break;
			case 0x0b://bkg CPS
				MAKEFLOAT(interProcControl.rsModbus.fBkgCPS,interProcControl.uart.rcvBuff_safe[6],interProcControl.uart.rcvBuff_safe[5],interProcControl.uart.rcvBuff_safe[4],interProcControl.uart.rcvBuff_safe[3]);
				InterProc_iterateDataReady(&interProcControl.rsModbus.swdBkgCPS);
				break;
/*			case 0x11://bkg DR
				MAKEFLOAT(interProcControl.rsModbus.fBkgDR,interProcControl.uart.rcvBuff_safe[6],interProcControl.uart.rcvBuff_safe[5],interProcControl.uart.rcvBuff_safe[4],interProcControl.uart.rcvBuff_safe[3]);
				InterProc_iterateDataReady(&interProcControl.rsModbus.swdBkgDR);
				break;*/
			case 0x03://lowLimit
				interProcControl.rsModbus.wdLowLimit = (interProcControl.uart.rcvBuff_safe[3]<<8) | interProcControl.uart.rcvBuff_safe[4];
				InterProc_iterateDataReady(&interProcControl.rsModbus.swdLowLimit);
				break;
			case 0x04://highLimit
				interProcControl.rsModbus.wdHighLimit = (interProcControl.uart.rcvBuff_safe[3]<<8) | interProcControl.uart.rcvBuff_safe[4];
				InterProc_iterateDataReady(&interProcControl.rsModbus.swdHighLimit);
				break;
/*			case 0x07://sigma thresholds (2 words)
				interProcControl.rsModbus.wdSigmaOper = (interProcControl.uart.rcvBuff_safe[3]<<8) | interProcControl.uart.rcvBuff_safe[4];
				interProcControl.rsModbus.wdSigmaSleep = (interProcControl.uart.rcvBuff_safe[5]<<8) | interProcControl.uart.rcvBuff_safe[6];
				InterProc_iterateDataReady(&interProcControl.rsModbus.swdSigmaThresholds);
				break;*/
			default://not supported register in answer!!!!!!!!
				exception(__FILE__,__FUNCTION__,__LINE__,"Unsupported control registers");
			}
			break;
		case 0x06:	//changing control regs
		case 0x05:	//set control signal
		case 0x13:
		case 0x10:
			break;
		case 0x08:	//diagnostic
			if(interProcControl.uart.rcvBuff_safe[2]==0 && interProcControl.uart.rcvBuff_safe[3]==2)
			{
				interProcControl.rsModbus.wdDiag = (interProcControl.uart.rcvBuff_safe[4]<<8) | interProcControl.uart.rcvBuff_safe[5];
				InterProc_iterateDataReady(&interProcControl.rsModbus.swdDiag);
			}else
			{//not supported subcommand in answer!!!!!!!!
				exception(__FILE__,__FUNCTION__,__LINE__,"Unsupported diagnostic command");
			}
			break;
		case 0x0b:	//spectrum
			bt1 = interProcControl.uart.trmBuff[3];
			bt2 = interProcControl.uart.trmBuff[4];
			adr = (bt1<<8) | bt2;
			len = interProcControl.uart.rcvBuff_safe[2];
			if((adr+len)<=(CHANNELS*3))
			{
				for(int i=0;i<len;i++)
				{
					interProcControl.rsModbus.arSpectrum[adr+i] = interProcControl.uart.rcvBuff_safe[3+i];
				}
				if((adr+len)==(CHANNELS*3))
				{//end of transmition of whole spectrum
					InterProc_iterateDataReady(&interProcControl.rsModbus.sarSpectrum);
				}
			}else
			{	//not supported prm in answer!!!!!!!!
				exception(__FILE__,__FUNCTION__,__LINE__,"Unsupported spectrum len");
			}
			break;
		default:	//not supported command in answer!!!!!!!!
			exception(__FILE__,__FUNCTION__,__LINE__,"Unsupported command in reply");
		}
	}
}
















//init order of command to interproc
void InterProc_initCmdOrder(void)
{
	for(int i=0;i<MAX_CMD_IN_ORDER;i++)
	{
		interProcControl.arCmd[i].cmdIndex = -1;
	}
}




//if no space then exception
//fill new command sequence in order
void InterProc_fillNewCmd(const BYTE volatile * cmd, int len)
{
	if(len>interProcControl.uart.constTrmBuffLen || len==0)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid send sequence len");
	}
	//first find buf with max index to get next index
	//here store max cmdIndex
	int maxIndex = -1;
	//here store index of first empty buff
	int indexIndex = -1;
	InterProc_findFreeSlot(&indexIndex, &maxIndex);
	if(indexIndex==-1)
	{
		exception(__FILE__,__FUNCTION__,__LINE__, "No free space in command order");
	}
	//we have free place in buf
	interProcControl.arCmd[indexIndex].len = len;
	for(int i=0;i<len;i++)
	{
		interProcControl.arCmd[indexIndex].arBuff[i]=cmd[i];
	}
	interProcControl.arCmd[indexIndex].cmdIndex = ++maxIndex;
}

//find first free slot in command order
//ret index of
void InterProc_findFreeSlot(int * pIndexIndex, int * pMaxIndex)
{
	int indexIndex = -1;
	int maxIndex = -1;
	for(int i=0;i<MAX_CMD_IN_ORDER;i++)
	{
		if(interProcControl.arCmd[i].cmdIndex==-1 && indexIndex==-1)
		{
			indexIndex = i;	//find first empty buff
		}else
		if(interProcControl.arCmd[i].cmdIndex>maxIndex)
		{//not empty, find max cmdIndex
			maxIndex = interProcControl.arCmd[i].cmdIndex;
		}
	}
	*pIndexIndex = indexIndex;
	*pMaxIndex = maxIndex;
}


//ADDED 29/07/2013 to resolve exeption in SPRD mode in time of frequent pressing of BGND button
//count free slots in command order
//ret count
int InterProc_countFreeSlots(void)
{
	int count = 0;
	for(int i=0;i<MAX_CMD_IN_ORDER;i++)
	{
		if(interProcControl.arCmd[i].cmdIndex==-1)
		{
			count++;
		}
	}
	return count;
}

//make exception if len in buff > interProcControl.uart.constTrmBuffLen
//ret Len, if =0 then no cmd
//proc fills cmd with buffer from order
int InterProc_getNextCmd(BYTE volatile * cmd)
{
	//first find buf with min index
	//here store min cmdIndex
	int minIndex = 0x7fffffff;
	//here store index of first buff
	int indexIndex = -1;
	for(int i=0;i<MAX_CMD_IN_ORDER;i++)
	{
		if(interProcControl.arCmd[i].cmdIndex!=-1 && interProcControl.arCmd[i].cmdIndex<minIndex)
		{//not empty, find max cmdIndex
			minIndex = interProcControl.arCmd[i].cmdIndex;
			indexIndex = i;	//find first empty buff
		}
	}
	if(indexIndex==-1)return 0;	//no cmd in order
	//we have buf
	int len = interProcControl.arCmd[indexIndex].len;
	if(len>interProcControl.uart.constTrmBuffLen || len==0)
	{
		//error in buf len
		exception(__FILE__,__FUNCTION__,__LINE__,"Send sequence is too long");
		return 0;
	}
	for(int i=0;i<len;i++)
	{
		cmd[i] = interProcControl.arCmd[indexIndex].arBuff[i];
	}
	interProcControl.arCmd[indexIndex].cmdIndex = -1;//make buff free
	return len;
}


//проверка на то что данные приняты и требуют обработки вторичным диспетчером
//устанавливаем признак того  что данные обработаны вторичным диспетчером
BOOL InterProc_isDataReady(struct tagInteProcRSModbusSync volatile* pIPRSMS)
{
	DWORD dw1, dw2;
	dw1 = pIPRSMS->dw_incLastRcv;
	dw2 = pIPRSMS->dw_inc;
	BOOL ret = (dw1 != dw2);
	if(ret)
		pIPRSMS->dw_incLastRcv = pIPRSMS->dw_inc;
	return ret;
}

//is data final ready after second dispatcher to processed in some procedure
BOOL InterProc_isDataFinalReady(struct tagInteProcRSModbusSync volatile* pIPRSMS)
{
	DWORD dw1, dw2;
	dw1 = pIPRSMS->dw_incLastRcv;
	dw2 = pIPRSMS->dw_inc;
	BOOL ret = (dw1 == dw2 && dw2 != pIPRSMS->dw_incLastTrm);
	return ret;
}

//ready to transmit data? if yes then we can fill new cmd from order
//also check for timeout of reply and show exception
//!!!!!!!!! здесь есть проблема для непериодических команд, т.к. если изза них есть исключение или не ответ то они не повторятся
BOOL InterProc_isReadyToTransmit(struct tagInteProcRSModbusSync volatile* pIPRSMS)
{
	BOOL ret = InterProc_isDataFinalReady(pIPRSMS);
	if(ret)
	{
		pIPRSMS->dw_incLastTrm = pIPRSMS->dw_inc;
	}
	return ret;
}

//mark data is ready, to be processed
void InterProc_iterateDataReady(struct tagInteProcRSModbusSync volatile* pIPRSMS)
{
	pIPRSMS->dw_inc++;
}


void InterProc_resetSyncData(struct tagInteProcRSModbusSync volatile* pIPRSMS)
{
	pIPRSMS->dw_inc=1;
	pIPRSMS->dw_incLastRcv=1;
	pIPRSMS->dw_incLastTrm=0;
}




//second dispatcher
//it analyzes received data that processed by first dispatcher
void InterProc_second_Dispatcher(void)
{
	
	//analize temperature
	if(InterProc_isDataReady(&interProcControl.rsModbus.swdTemperature))
	{
		interProcControl.fTemperature = (float)((float)((short)interProcControl.rsModbus.wdTemperature/256.0))+(float)(interProcControl.rsModbus.wdTemperature&0xff)/(float)10.0;
//		interProcControl.fTemperature = (float)(char)(interProcControl.rsModbus.wdTemperature>>8)+(float)(char)(interProcControl.rsModbus.wdTemperature&0xff)/10;
	}
	if(InterProc_isDataReady(&interProcControl.rsModbus.sbtStatus))
	{
		interProcControl.btStatus = interProcControl.rsModbus.btStatus;
		//confirm background or not background mode reading status value
		if(SPRDModeControl.bBkgMode_confirmed && !SPRDModeControl.bBkgMode_assumed
		   && !(interProcControl.btStatus&0x40))
		{//sure that it is BKG mode
			SPRDModeControl.bBkgMode_assumed = SPRDModeControl.bBkgMode_confirmed;
		}else if(!SPRDModeControl.bBkgMode_confirmed && SPRDModeControl.bBkgMode_assumed
				 && (interProcControl.btStatus&0x40))
		{//sure that it is not BKG mode
			SPRDModeControl.bBkgMode_assumed = SPRDModeControl.bBkgMode_confirmed;
		}
	}
	if(InterProc_isDataReady(&interProcControl.rsModbus.swdDiag))
	{
		;
	}
	if(InterProc_isDataReady(&interProcControl.rsModbus.swdStabGain))
	{
		;
	}
	if(InterProc_isDataReady(&interProcControl.rsModbus.swdLowLimit))
	{
		;
	}
	if(InterProc_isDataReady(&interProcControl.rsModbus.swdHighLimit))
	{
		;
	}
/*	if(InterProc_isDataReady(&interProcControl.rsModbus.swdSigmaThresholds))
	{
		SPRDModeControl.operation_search_sigma = interProcControl.rsModbus.wdSigmaOper;
		SPRDModeControl.sleepmode_search_sigma = interProcControl.rsModbus.wdSigmaSleep;
	}*/
	if(InterProc_isDataReady(&interProcControl.rsModbus.swdPresetAcqTime))
	{
		;
	}
	if(InterProc_isDataReady(&interProcControl.rsModbus.swdAcqTime))
	{//data is fresh
		spectrumControl.acqSpectrum.wAcqTime = interProcControl.rsModbus.wdAcqTime;
	}
	if(InterProc_isDataReady(&interProcControl.rsModbus.swdMeasurementRegs))
	{//data is fresh
		
		//for TC mode
		if(TCModeControl.bRun)
		{
			TCModeControl.dwCount += (DWORD)interProcControl.rsModbus.fMomCps/3;
			TCModeControl.dwTimer++;
		}
		
		//for SPRD mode
		SPRDModeControl.fMomCps = interProcControl.rsModbus.fMomCps;
		SPRDModeControl.fCps = interProcControl.rsModbus.fCps;
		SPRDModeControl.fCpsErr = interProcControl.rsModbus.fCpsErr;
#ifdef BNC
		//from rem to mrem
		SPRDModeControl.fDoserate = (float)1E3*interProcControl.rsModbus.fDoserate;
		SPRDModeControl.fDose = (float)1E3*interProcControl.rsModbus.fDose;
#else
		//from sv to mksv
		SPRDModeControl.fDoserate = (float)1E6*interProcControl.rsModbus.fDoserate;
		SPRDModeControl.fDose = (float)1E6*interProcControl.rsModbus.fDose;
#endif
		SPRDModeControl.fDoserateErr = interProcControl.rsModbus.fDoserateErr;
		SPRDModeControl.sSigma = interProcControl.rsModbus.sSigma;
	
		//move mcs to put new value in not bkg mode
		if(!SPRDModeControl.bBkgMode_assumed && !SPRDModeControl.bBkgMode_confirmed)
		{
			memmove((void*)&SPRDModeControl.arMCS[0],(void*)&SPRDModeControl.arMCS[1],(MCS_WIN_WIDTH-1)*sizeof(int));
			SPRDModeControl.arMCS[MCS_WIN_WIDTH-1] = (int)SPRDModeControl.sSigma;
		}
	}
	if(InterProc_isDataReady(&interProcControl.rsModbus.sarSpectrum))
	{//received new spectrum
		//copy spectrum and calc total counts
		DWORD val;
		DWORD sum=0;
		for(int i=0;i<CHANNELS;i++)
		{
			val = ((DWORD)interProcControl.rsModbus.arSpectrum[i*3+2]<<16)|((DWORD)interProcControl.rsModbus.arSpectrum[i*3+1]<<8)|((DWORD)interProcControl.rsModbus.arSpectrum[i*3+0]);
			spectrumControl.acqSpectrum.dwarSpectrum[i] = val;
			if(i<700)//суммируем до пика светодиода
				sum+=val;
		}
		spectrumControl.acqSpectrum.dwCount = sum;
		
		//!!!!!!!!!!!!!
		//фильтр динелинейности
#ifdef _DIFF_FILTER
		Spectrum_DiffFilter();
#endif	//#ifdef _DIFF_FILTER
		//!!!!!!!!!!!!!!1


		//copy gps and temperature data	and doserate
		memcpy((void*)&spectrumControl.acqSpectrum.commonGPS, (const void*)&NMEAParserControl.commonGPS, sizeof(NMEAParserControl.commonGPS));
		memcpy((void*)&spectrumControl.acqSpectrum.fTemperature, (const void*)&interProcControl.fTemperature, sizeof(interProcControl.fTemperature));
#ifdef BNC
		spectrumControl.acqSpectrum.fDoserate = (float)SPRDModeControl.fDoserate;	//keep it in mrem/h
#else
		spectrumControl.acqSpectrum.fDoserate = (float)SPRDModeControl.fDoserate*1000.0;	//convert to nanosivert
#endif		
		spectrumControl.acqSpectrum.fCps = SPRDModeControl.fCps;

	}
}




//must be run after Clock_showDateTime
void InterProc_showTemperature(void)
{
	if((interProcControl.fTemperature<MIN_TEMPER || interProcControl.fTemperature>MAX_TEMPER)&&
	   (clockData.dateTime.second&0x1))
		Display_setTextColor(RED);	//set text color
	else
		Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(80,0,50,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	char buf[10];
	if(interProcControl.fTemperature!=0)
		sprintf(buf, "%+0.0f°C", interProcControl.fTemperature);
	else
		sprintf(buf, "%0.0f°C", interProcControl.fTemperature);
	Display_clearTextWin(10);
	Display_outputText(buf);
}
