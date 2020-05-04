//bluetooth.c


#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <string.h>

#include "types.h"
#include "syncObj.h"
#include "powerControl.h"
#include "display.h"
#include "keyboard.h"
#include "bluetooth.h"
#include "COMM_mode.h"
#include "interProc.h"
#include "USBRS.h"
#include "interrupts.h"
#include "SETUP_Mode.h"
#include "USB_mode.h"
#include "geiger.h"



struct tagBluetoothControl bluetoothControl;

#ifndef GPS_BT_FREE	


//show bluetooth symbol, hide it or blink it
void Bluetooth_show_symbol(int x)
{
	if(bluetoothControl.bShow_Bluetooth_sym)
		Display_output_bmp(x,0, (BITMAPFILEHEADER*)bmp_bth);
	else
	{
		RECT rect = {x,0,x+15,16};
		Display_clearRect(rect, 100);
	}
}





//управление отображением символа bluetooth, выкл, вкл, мерцание
//мерцание когда вкл но не fix
void Bluetooth_sym_control(void)
{
	if(bluetoothControl.bBluetooth_ON)
	{//turned on
		if(!bluetoothControl.bBluetooth_Connected)
			bluetoothControl.bShow_Bluetooth_sym=!bluetoothControl.bShow_Bluetooth_sym;
		else
			bluetoothControl.bShow_Bluetooth_sym=1;
	}else
	{//turned off
		bluetoothControl.bShow_Bluetooth_sym=0;
	}
}





//control bluetooth status and data
void Bluetooth_control(void)
{
	if(bluetoothControl.bBluetooth_ON)
	{
	
		SyncObj_copyUARTtoSafePlace(&bluetoothControl.uart);

		if(bluetoothControl.uart.bDataReceived_safe)
		{
			if(bluetoothControl.iModuleComm)
			{
				Bluetooth_rcvData_module_Dispatcher();
			}else
			{
				//first process, the results must be processed in the procedures of initiate transmition command
				//no cmd should be prepared in trm buffer if rcv data is not processed here!!!!!!!!!
				Bluetooth_rcvData_first_Dispatcher();
			}
			powerControl.dwIdleTime = 0;	//reset idle time counter after key processing
		}
		
	}
	
	if(bluetoothControl.iModuleComm)//если идет обмен с модулем, то декремент счетчика выключения такого обмена
		bluetoothControl.iModuleComm--;

}


//first inition
void Bluetooth_Init(void)
{
	bluetoothControl.uart.rcvBuff = bluetoothControl.rcvBuff;
	bluetoothControl.uart.rcvBuff_safe = bluetoothControl.rcvBuff_safe;
	bluetoothControl.uart.trmBuff = bluetoothControl.trmBuff;
	bluetoothControl.uart.constRcvBuffLen = sizeof(bluetoothControl.rcvBuff);
	bluetoothControl.uart.constTrmBuffLen = sizeof(bluetoothControl.trmBuff);
	
	
	for(int i=0;i<bluetoothControl.uart.constTrmBuffLen;i++)
	{
		bluetoothControl.uart.trmBuff[i] = 0;
	}
	for(int i=0;i<bluetoothControl.uart.constRcvBuffLen;i++)
	{
		bluetoothControl.uart.rcvBuff[i] = 0;
		bluetoothControl.uart.rcvBuff_safe[i] = 0;
	}

	DIR_BT_ON = 1;
	SET_BT_ON;	
	DIR_BT_RES = 1;
	CLR_BT_RES;
        DIR_BT_RF = 1;
        SET_BT_RF;
	bluetoothControl.bBluetooth_ON = 0;
	bluetoothControl.bBluetooth_Connected = 0;
	bluetoothControl.bShow_Bluetooth_sym = 0;
	
	bluetoothControl.iModuleComm = 0;	//comm with PC
	
	memset(bluetoothControl.moduleName, 0, sizeof(bluetoothControl.moduleName));
}


void Bluetooth_UART3_Init(void)
{
	int SpeedRS232;
	
	SpeedRS232 = 115200;   //скорость обмена
	
	
	PCONP_bit.PCUART3 = 1;	//give power to UART2

	//===== Chirikalo
        PINSEL0_bit.P0_0 = 0x02;	//select pins for GPIO
	PINSEL0_bit.P0_1 = 0x02;
        //===== Chirikalo
	PINMODE0_bit.P0_0 = 0x00;
	PINMODE0_bit.P0_1 = 0x00;
	
	// enable access to divisor latch regs
	U3IER = 0;	//disable int
	
	
	
	//init buffer and values
	for(int i=0;i<bluetoothControl.uart.constTrmBuffLen;i++)
	{
		bluetoothControl.uart.trmBuff[i] = 0;
	}
	for(int i=0;i<bluetoothControl.uart.constRcvBuffLen;i++)
	{
		bluetoothControl.uart.rcvBuff[i] = 0;
		bluetoothControl.uart.rcvBuff_safe[i] = 0;
	}
	bluetoothControl.uart.rcvBuffLen = 0;	//must be reset to 0 when start transmition
	bluetoothControl.uart.bRcvError = RCV_OK;	//must be reset when start transmition
	bluetoothControl.uart.trmBuffLen = 0;	
	bluetoothControl.uart.trmBuffLenConst = 0;

	
	
	U3LCR = 0x83;        //LCR_ENABLE_LATCH_ACCESS;
	
	// set divisor for desired baud
	//divider = HW_FREQ/UART_baudrate/16
	DWORD div = HW_FREQ/SpeedRS232/16;
	U3DLM = 0x00;
	U3DLL = div;//((HW_FREQ*1000) / (SpeedRS232*16)); //0x2;  //115200 ->8/4 //надо 0x0A/4->18432000/(16*115200)
	//U3FCR =0x07; //1byte, clear FIFO
	// disable access to divisor latch regs (enable access to xmit/rcv fifos
	// and int enable regs)
	U3LCR =0x3; //LCR_DISABLE_LATCH_ACCESS;
	// setup fifo control reg - trigger level 0 (1 byte fifos), no dma
	// disable fifos (450 mode) прерывание по приему 1-го байта
	
	__uartfcriir_bits u3fcr;
	u3fcr.FCRFE=1;
	u3fcr.RFR=1;
	u3fcr.TFR=1;
	u3fcr.RTLS=2;//пачками по 8 байт
	U3FCR_bit = u3fcr;
	
	
	// enable UART0 interrupts
	U3IER = 0x7;	//enable RBR = 1, THRE = 1, Status Rx
}




//прерывание приема передачи
__arm void _INT_UART3_Bluetooth(void)
{
	DWORD dw1,dw2;
	BYTE byt;
	if(bluetoothControl.uart.bRcvError != RCV_OK)
	{//ошибка обмена
		//сброс ошибки и буфера приема
		bluetoothControl.uart.rcvBuffLen = 0;
		bluetoothControl.uart.bRcvError = RCV_OK;
	}
	while((U3LSR&0x1) || ((U3IIR&0x0c)==0x0c))
	{//DR

		byt = U3RBR;
		if(USBRSControl.bBridgeMode)
		{
			USBRSControl.uart.trmBuffLen = USBRSControl.uart.trmBuffLenConst = 0;
			U0THR = byt;
			COMMModeControl.dwReceived++;
			USBModeControl.dwTransmitted++;
			Display_flashRedLED();
		}else		
		if(bluetoothControl.iModuleComm)
		{//module communication
			if(bluetoothControl.uart.rcvBuffLen < bluetoothControl.uart.constRcvBuffLen)
			{
				COMMModeControl.dwReceived++;
				bluetoothControl.uart.rcvBuff[bluetoothControl.uart.rcvBuffLen++] = byt;
				if(bluetoothControl.uart.rcvBuffLen>4)
				{
					dw1 = (bluetoothControl.uart.rcvBuff[4]<<8);
					dw1 |= bluetoothControl.uart.rcvBuff[3];
					if(bluetoothControl.uart.rcvBuffLen-7>=dw1)
						bluetoothControl.uart.bDataReceived=1;
				}
			}
		}else
		{//else PC communication
		
			if(bluetoothControl.uart.rcvBuffLen < bluetoothControl.uart.constRcvBuffLen)
			{
				if((bluetoothControl.uart.rcvBuffLen>=1) ||
				   (bluetoothControl.uart.rcvBuffLen==0 && (byt==INTERPROC_ADDRESS || byt==USBRS_ADDRESS)))
				{
					COMMModeControl.dwReceived++;
					bluetoothControl.uart.rcvBuff[bluetoothControl.uart.rcvBuffLen++] = byt;
					if(bluetoothControl.uart.rcvBuffLen>=4)
					{
						USBRS_checkDataReceived_intcall(&bluetoothControl.uart);
					}
				}
			}else
			{//error receiver buffer len
				bluetoothControl.uart.bRcvError = RCV_ERR_BUF_OVERFLOW;
			}
	
/*
//предположительно изза этого очередь команд заступоривается		
			if(bluetoothControl.uart.bRcvError == RCV_ERR_CRC)
			{
				bluetoothControl.uart.rcvBuffLen = 0;
				bluetoothControl.uart.bRcvError = RCV_OK;
			}
*/
                        
			//find pocket start
			if(bluetoothControl.uart.rcvBuffLen>=2 && bluetoothControl.uart.bRcvError != RCV_OK)
			{//move pocket to find start byte: address then check for command if amount of bytes enough
				do
				{
					bluetoothControl.uart.rcvBuffLen--;
					memmove((void*)&bluetoothControl.uart.rcvBuff[0],(void const*)&bluetoothControl.uart.rcvBuff[1],bluetoothControl.uart.rcvBuffLen);
					if((bluetoothControl.uart.rcvBuff[0]==INTERPROC_ADDRESS ||
					   bluetoothControl.uart.rcvBuff[0]==USBRS_ADDRESS))
					{
						bluetoothControl.uart.bRcvError = RCV_OK;
						if(bluetoothControl.uart.rcvBuffLen>=4)
						{
							USBRS_checkDataReceived_intcall(&bluetoothControl.uart);
						}
						break;
					}
				}while(bluetoothControl.uart.rcvBuffLen>=2);
			}
		}
	};
	if(U3LSR&0x20)
	{//THRE
		dw1 = bluetoothControl.uart.trmBuffLen;
		dw2 = bluetoothControl.uart.trmBuffLenConst;
		if(dw1 < dw2)
		{//передача байта
			COMMModeControl.dwTransmitted++;
			byt = bluetoothControl.uart.trmBuff[bluetoothControl.uart.trmBuffLen++];
			U3THR = byt;
		}else
		{//все передали, просто ничего не делаем
			;//прерывания не будут генериться
			bluetoothControl.uart.bTrmReady = 1;
		}
	}
	if(U3LSR&0x0e)
	{
		bluetoothControl.uart.bRcvError = RCV_ERR_BYTE;
	}
}





void Bluetooth_turnON(void)
{

	CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;	//give power to GPS
        SET_BT_ON;
        CLR_BT_ON;
//===== Chirikalo
//	PINMODE0_bit.P0_0 = 0x02;	//select pins for UART2
//	PINMODE0_bit.P0_1 = 0x02;
        //===== Chirikalo
	Bluetooth_UART3_Init();
	bluetoothControl.bBluetooth_ON = 1;

	PowerControl_sleep(10);
	
	CLR_BT_RES;
	PowerControl_sleep(50);
	SET_BT_RES;
        DIR_BT_RF = 0;    //Чтобы не было выход на выход
	
	PowerControl_sleep(500);

	Bluetooth_readModuleName();
}

//send any sequence of bytes
void Bluetooth_sendSequence(int len)
{
	if(len<1 || len>bluetoothControl.uart.constRcvBuffLen)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Send sequence is too long");
		return;
	}
	
	//assume that transmiter is ready
	SAFE_DECLARE;
	DISABLE_VIC;
	bluetoothControl.uart.bTrmReady = 0;
	
	if(USBRSControl.bBridgeMode)//in bridge mode do not calc CRC
		bluetoothControl.uart.trmBuffLenConst = len;
	else
		bluetoothControl.uart.trmBuffLenConst = USBRS_calcCRC(bluetoothControl.uart.trmBuff, len);
	
	bluetoothControl.uart.trmBuffLen = 1;
	bluetoothControl.uart.rcvBuffLen = 0;
	bluetoothControl.uart.bRcvError = RCV_OK;
	bluetoothControl.uart.bDataReceived = 0;
	//start transmition data from first byte in buffer
	U3THR = bluetoothControl.uart.trmBuff[0];
	COMMModeControl.dwTransmitted++;
	ENABLE_VIC;
}


//insert crc one byte
//insert end delimeter
//send sequence 
void Bluetooth_sendSequenceEx(int len)
{
	if(len<1 || len>bluetoothControl.uart.constRcvBuffLen)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Send sequence is too long");
		return;
	}
	
	//assume that transmiter is ready
	SAFE_DECLARE;
	DISABLE_VIC;
	bluetoothControl.uart.bTrmReady = 0;
	
	bluetoothControl.uart.trmBuffLenConst = USBRS_calcCRCEx(bluetoothControl.uart.trmBuff, len);
	
	bluetoothControl.uart.trmBuffLen = 1;
	bluetoothControl.uart.rcvBuffLen = 0;
	bluetoothControl.uart.bRcvError = RCV_OK;
	bluetoothControl.uart.bDataReceived = 0;
	//start transmition data from first byte in buffer
	U3THR = bluetoothControl.uart.trmBuff[0];
	COMMModeControl.dwTransmitted++;
	ENABLE_VIC;
}


//first dispatcher of received data
void Bluetooth_rcvData_first_Dispatcher(void)
{
	{
		if(bluetoothControl.uart.rcvBuff_safe[0]==INTERPROC_ADDRESS
			   //21/01/2010
			   && bluetoothControl.uart.rcvBuff_safe[1]!=0x09  /*it is not a command of read of eeprom spectrum*/
			   && bluetoothControl.uart.rcvBuff_safe[1]!=0x11  /*it is not a command of read of ID data*/
				//14.04.2016
			   && bluetoothControl.uart.rcvBuff_safe[1]!=0x02  /*it is not binary signals*/
				&& !(bluetoothControl.uart.rcvBuff_safe[1]==0x04 && bluetoothControl.uart.rcvBuff_safe[2]==0x00 && 
					 (bluetoothControl.uart.rcvBuff_safe[3]>=0x1e && bluetoothControl.uart.rcvBuff_safe[3]<=0x27)))
			  ///////////
		{//command to the second processor
			//translate it to second proc
			
			//have to check it before send else will receive exception
			int maxIndex = -1;
			//here store index of first empty buff
			int indexIndex = -1;
			InterProc_findFreeSlot(&indexIndex, &maxIndex);
			if(indexIndex==-1 || bluetoothControl.uart.rcvBuffLen_safe>interProcControl.uart.constTrmBuffLen ||
			   bluetoothControl.uart.rcvBuffLen_safe==0)
			{
				return;	//error in cmd, ignore it
			}
	
			//change address to resolve answers from first proc and usb
			bluetoothControl.uart.rcvBuff_safe[0]=BLUETOOTH_ADDRESS;
			InterProc_fillNewCmd(bluetoothControl.uart.rcvBuff_safe, bluetoothControl.uart.rcvBuffLen_safe-2);
		}else if(bluetoothControl.uart.rcvBuff_safe[0]==USBRS_ADDRESS
					 //21/01/2010
					|| bluetoothControl.uart.rcvBuff_safe[1]==0x09  /*it is a command of read of eeprom spectrum*/
					|| bluetoothControl.uart.rcvBuff_safe[1]==0x11  /*it is a command of read of ID data*/
					//14.04.2016
					|| bluetoothControl.uart.rcvBuff_safe[1]==0x02  /*it is a command of read binary signals*/
				|| (bluetoothControl.uart.rcvBuff_safe[1]==0x04 && bluetoothControl.uart.rcvBuff_safe[2]==0x00 && 
					 (bluetoothControl.uart.rcvBuff_safe[3]>=0x1e && bluetoothControl.uart.rcvBuff_safe[3]<=0x27)))
				///////////
		{//command to this processor
			USBRS_rcvData_second_Dispatcher(&bluetoothControl.uart);
			if(bluetoothControl.uart.trmBuffLenConst>=2 && bluetoothControl.uart.trmBuffLenConst<bluetoothControl.uart.constTrmBuffLen)
				Bluetooth_sendSequence(bluetoothControl.uart.trmBuffLenConst);
		}
	}
}


//process answer from second proc and resent it to bluetooth
void Bluetooth_answer_first_Dispatcher(void)
{
	if(interProcControl.uart.rcvBuffLen_safe<2 || interProcControl.uart.rcvBuffLen_safe>bluetoothControl.uart.constTrmBuffLen)
		return;//error in answer from second proc
	//change address of answer from second proc to be resolved by PC
	interProcControl.uart.rcvBuff_safe[0]=INTERPROC_ADDRESS;
	memcpy((void*)bluetoothControl.uart.trmBuff,(void*)interProcControl.uart.rcvBuff_safe,interProcControl.uart.rcvBuffLen_safe-2);
	//check for signal control 0x22 to reset GM
	if(bluetoothControl.uart.trmBuff[1]==0x5 && bluetoothControl.uart.trmBuff[3]==0x22 && bluetoothControl.uart.trmBuff[4]==0xff)
	{//reset GM
		geigerControl.bReset = 1;
	}
	//
	Bluetooth_sendSequence(interProcControl.uart.rcvBuffLen_safe-2);
}



//it starts Bluetooth depends on value Bluetoothstate
void Bluetooth_startbyState(void)
{
	if(bluetoothControl.comm_power_state!=enu_comm_power_state_manually)
		Bluetooth_turnON();
	else
		Bluetooth_turnOFF();
}


//get module name
void Bluetooth_readModuleName(void)
{
	const BYTE cmd[]={0x02,0x52,0x03,0x00,0x00,0x00};
	memcpy((void*)&bluetoothControl.uart.trmBuff[0], cmd, sizeof(cmd));

	bluetoothControl.iModuleComm = 200;//set as a counter to shutdown module comm

	Bluetooth_sendSequenceEx(sizeof(cmd));
}

//get module name
void Bluetooth_writeModuleName(void)
{
	int len = 0;
#ifdef BNC	
	const BYTE cmd[]={0x02,0x52,0x04,0x15,0x00,0x00,0x14};
#else
	const BYTE cmd[]={0x02,0x52,0x04,0x14,0x00,0x00,0x13};
#endif
	Bluetooth_prepareModuleName();
	memcpy((void*)&bluetoothControl.uart.trmBuff[0], cmd, sizeof(cmd));
	memcpy((void*)&bluetoothControl.uart.trmBuff[sizeof(cmd)], bluetoothControl.moduleNameTemp, strlen(bluetoothControl.moduleNameTemp)+1);
	len=sizeof(cmd)+strlen(bluetoothControl.moduleNameTemp)+1;

	bluetoothControl.iModuleComm = 200;//set as a counter to shutdown module comm

	Bluetooth_sendSequenceEx(len);
}


void Bluetooth_prepareModuleName(void)
{
	memset(bluetoothControl.moduleNameTemp,0,sizeof(bluetoothControl.moduleNameTemp));
#ifdef BNC	
	sprintf(bluetoothControl.moduleNameTemp,"palmRad m920 #%05u",(UINT)SETUPModeControl.Serial);
#else
	sprintf(bluetoothControl.moduleNameTemp,"RadSearcher #%05u",(UINT)SETUPModeControl.Serial);
#endif
}


//расшифровка ответа от модуля
void Bluetooth_rcvData_module_Dispatcher(void)
{
	if(!bluetoothControl.uart.rcvBuff_safe[0]==0x02 || !bluetoothControl.uart.rcvBuff_safe[1]==0x43)return;	//error
	switch(bluetoothControl.uart.rcvBuff_safe[2])
	{
	case 3://module name
		Bluetooth_prepareModuleName();
		memset(bluetoothControl.moduleName, 0, sizeof(bluetoothControl.moduleName));
		if(bluetoothControl.uart.rcvBuff_safe[7]>0 && bluetoothControl.uart.rcvBuff_safe[7]<sizeof(bluetoothControl.moduleName))
		{
			strncpy(bluetoothControl.moduleName, (const char*)&bluetoothControl.uart.rcvBuff_safe[8], bluetoothControl.uart.rcvBuff_safe[7]);
			if(strcmp(bluetoothControl.moduleName, bluetoothControl.moduleNameTemp))
			{
				Bluetooth_writeModuleName();
			}else
				bluetoothControl.iModuleComm = 0;	//end of module communication
		}else
			bluetoothControl.iModuleComm = 0;	//end of module communication
		break;
	case 4://module name
		Bluetooth_readModuleName();
		break;
	default:
		;
	}
}

#else


//first inition
void Bluetooth_Init(void)
{
	bluetoothControl.uart.rcvBuff = bluetoothControl.rcvBuff;
	bluetoothControl.uart.rcvBuff_safe = bluetoothControl.rcvBuff_safe;
	bluetoothControl.uart.trmBuff = bluetoothControl.trmBuff;
	bluetoothControl.uart.constRcvBuffLen = sizeof(bluetoothControl.rcvBuff);
	bluetoothControl.uart.constTrmBuffLen = sizeof(bluetoothControl.trmBuff);
	
	
	for(int i=0;i<bluetoothControl.uart.constTrmBuffLen;i++)
	{
		bluetoothControl.uart.trmBuff[i] = 0;
	}
	for(int i=0;i<bluetoothControl.uart.constRcvBuffLen;i++)
	{
		bluetoothControl.uart.rcvBuff[i] = 0;
		bluetoothControl.uart.rcvBuff_safe[i] = 0;
	}

	bluetoothControl.bBluetooth_ON = 0;
	bluetoothControl.bBluetooth_Connected = 0;
	bluetoothControl.bShow_Bluetooth_sym = 0;
	
	bluetoothControl.iModuleComm = 0;	//comm with PC
	
	memset(bluetoothControl.moduleName, 0, sizeof(bluetoothControl.moduleName));
}

#endif

void Bluetooth_turnOFF(void)
{
	PCONP_bit.PCUART3 = 0;	//take power from UART3
        //===== Chirikalo
	PINMODE0_bit.P0_0 = 0x00;	//select pins for GPIO
	PINMODE0_bit.P0_1 = 0x00;
	PINSEL0_bit.P0_0 = 0x00;	//select pins for GPIO
	PINSEL0_bit.P0_1 = 0x00;
        //===== Chirikalo
        DIR_BT_RF = 1;
	SET_BT_ON;	//takeoff power
	CLR_BT_RES;
	bluetoothControl.bBluetooth_ON = 0;
}

