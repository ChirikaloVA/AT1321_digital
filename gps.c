//gps.c


#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <string.h>

#include "types.h"
#include "syncObj.h"
#include "powerControl.h"
#include "display.h"
#include "clock.h"
#include "gps.h"
#include "NMEA_Parser.h"


struct tagGPSControl GPSControl;







#ifndef GPS_BT_FREE	



//прерывание приема передачи
__arm void _INT_UART2_GPS(void)
{
	DWORD dw1,dw2;
	BYTE byt;
	while((U2LSR&0x1) || ((U2IIR&0x0c)==0x0c))
	{//DR
		byt = U2RBR;
		if(GPSControl.uart.rcvBuffLen>0 || byt=='$')
		{
			if(GPSControl.uart.rcvBuffLen < GPSControl.uart.constRcvBuffLen)
			{
				GPSControl.uart.rcvBuff[GPSControl.uart.rcvBuffLen++] = byt;
				if(GPSControl.uart.rcvBuffLen < GPSControl.uart.constRcvBuffLen)
					GPSControl.uart.rcvBuff[GPSControl.uart.rcvBuffLen] = 0;
			}else
			{//error receiver buffer len
				//just bypass this byte, dont fire error
				;
			}
		}//no start symbol
	};
	if(U2LSR&0x20)
	{//THRE
		dw1 = GPSControl.uart.trmBuffLen ;
		dw2 = GPSControl.uart.trmBuffLenConst;
		if(dw1< dw2)
		{//передача байта
			byt = GPSControl.uart.trmBuff[GPSControl.uart.trmBuffLen++];
			U2THR = byt;
		}else
		{//все передали, просто ничего не делаем
			;//прерывания не будут генериться
		}
	}
	if(U2LSR&0x0e)
	{
		GPSControl.uart.bRcvError = RCV_ERR_TIMEOUT;
	}
}














//show GPS symbol in status lnie
void GPS_show_symbol(int x)
{
	if(GPSControl.bShow_GPS_sym)
		Display_output_bmp(x,0, (BITMAPFILEHEADER*)bmp_gps);
	else
	{
		RECT rect = {x,0,x+23,16};
		Display_clearRect(rect, 100);
	}
}

//control GPS status and data
void GPS_control(void)
{
	int i;
	GPSControl.bGPS_Fix = FALSE;

	if(GPSControl.bGPS_ON)
	{//смотрим на полученные данные
		SAFE_DECLARE;
		DISABLE_VIC;

		GPSControl.uart.bDataReceived = 0;	//reset it now	//17/01/08	because of we would proceed all time the same buffer


		
		if(GPSControl.uart.bRcvError)
		{
			GPSControl.uart.rcvBuffLen = 0;
			ENABLE_VIC;
		}else
		{
			for(i=GPSControl.uart.rcvBuffLen-1;i>=0;i--)
			{
				if(GPSControl.uart.rcvBuff[i]=='$')
				{
					break;
				}
			}
			
			if(i<=0)
			{
				if(GPSControl.uart.rcvBuffLen==GPSControl.uart.constRcvBuffLen)
				{
					GPSControl.uart.rcvBuffLen = 0;
				}
				ENABLE_VIC;
			}else
			{
				memcpy((void*)GPSControl.uart.rcvBuff_safe, (void*)GPSControl.uart.rcvBuff, i);
				int movlen = GPSControl.uart.rcvBuffLen-i;
				if(movlen>0)
					memmove((void*)GPSControl.uart.rcvBuff, (void*)&GPSControl.uart.rcvBuff[i], movlen);
				GPSControl.uart.rcvBuffLen=movlen;
				if(GPSControl.uart.rcvBuffLen<GPSControl.uart.constRcvBuffLen)
					GPSControl.uart.rcvBuff[GPSControl.uart.rcvBuffLen] = '\0';
				GPSControl.uart.rcvBuffLen_safe = i;
				GPSControl.uart.bRcvError_safe = GPSControl.uart.bRcvError;
				GPSControl.uart.bDataReceived_safe = GPSControl.uart.bDataReceived;
				ENABLE_VIC;
			
				if(GPSControl.uart.rcvBuffLen_safe<GPSControl.uart.constRcvBuffLen)
					GPSControl.uart.rcvBuff_safe[GPSControl.uart.rcvBuffLen_safe] = '\0';
				
			
				
				NMEAParser_ParseBuffer((BYTE*)GPSControl.uart.rcvBuff_safe, GPSControl.uart.rcvBuffLen_safe);
	
				
				GPSControl.bGPS_Fix = (NMEAParserControl.m_btGSAFixMode>=2);
			}
		}
	}
}

//control gps symbol
void GPS_sym_control(void)
{
	//управление отображением символа ГПС, выкл, вкл, мерцание
	//мерцание когда вкл and fix
	if(GPSControl.bGPS_ON)
	{
		if(GPSControl.bGPS_Fix)
			GPSControl.bShow_GPS_sym=!GPSControl.bShow_GPS_sym;
		else
			GPSControl.bShow_GPS_sym=1;
	}else
	{
		GPSControl.bShow_GPS_sym=0;
	}
}


//init GPS
void GPS_Init(void)
{
	GPSControl.uart.rcvBuff = GPSControl.rcvBuff;
	GPSControl.uart.rcvBuff_safe = GPSControl.rcvBuff_safe;
	GPSControl.uart.trmBuff = GPSControl.trmBuff;
	GPSControl.uart.constRcvBuffLen = sizeof(GPSControl.rcvBuff);
	GPSControl.uart.constTrmBuffLen = sizeof(GPSControl.trmBuff);


	//adjust on output
	DIR_GPS_O = 1;
	DIR_GPS = 1;
	
	DIR_1PPS= 0;  // Запись
	DIR_RFPW = 0;
	
	GPSControl.bGPS_ON = 0;
	GPSControl.bGPS_Fix = 0;
	GPSControl.bShow_GPS_sym = 0;
	
	GPSControl.uartSpeed = 1200;   //скорость обмена стартовая
	//read eeprom values
	GPSControl.gps_state = GPS_STATE_ALWAYS_OFF;	

}


void GPS_UART2_Init(void)
{
	PCONP_bit.PCUART2 = 1;	//give power to UART2

	
	PINSEL0_bit.P0_10 = 0x01;	//select pins for UART2
	PINSEL0_bit.P0_11 = 0x01;
	PINMODE0_bit.P0_10 = 0x0;
	PINMODE0_bit.P0_11 = 0x0;
	
	GPS_UART2_InitEx(GPSControl.uartSpeed);
}
void GPS_UART2_InitEx(DWORD speed)
{
	// enable access to divisor latch regs
	U2IER = 0;	//disable int
	
	
	//init buffer and values
	for(int i=0;i<GPSControl.uart.constTrmBuffLen;i++)
	{
		GPSControl.uart.trmBuff[i] = 0;
	}
	for(int i=0;i<GPSControl.uart.constRcvBuffLen;i++)
	{
		GPSControl.uart.rcvBuff[i] = 0;
		GPSControl.uart.rcvBuff_safe[i] = 0;
	}

	GPSControl.uart.rcvBuffLen = 0;	//must be reset to 0 when start transmition
	GPSControl.uart.bRcvError = RCV_OK;	//must be reset when start transmition
	GPSControl.uart.trmBuffLen = 0;	
	GPSControl.uart.trmBuffLenConst = 0;
	

	U2LCR = 0x83;        //LCR_ENABLE_LATCH_ACCESS;
	
	// set divisor for desired baud
	//divider = HW_FREQ/UART_baudrate/16
	DWORD div = HW_FREQ/speed/16;
	U2DLM = 0x00;
	U2DLL = div;//((HW_FREQ*1000) / (SpeedRS232*16)); //0x2;  //115200 ->8/4 //надо 0x0A/4->18432000/(16*115200)
	//U0FCR =0x07; //1byte, clear FIFO
	// disable access to divisor latch regs (enable access to xmit/rcv fifos
	// and int enable regs)
	U2LCR =0x3; //LCR_DISABLE_LATCH_ACCESS;
	// setup fifo control reg - trigger level 0 (1 byte fifos), no dma
	// disable fifos (450 mode) прерывание по приему 1-го байта
	
	__uartfcriir_bits u2fcr;
	u2fcr.FCRFE=1;
	u2fcr.RFR=1;
	u2fcr.TFR=1;
	u2fcr.RTLS=3;//пачками по 14 байт
	U2FCR_bit = u2fcr;
	
	
	// enable UART0 interrupts
	U2IER = 0x7;	//enable RBR = 1, THRE = 1, Status Rx
	
	GPSControl.dwSecondsOfStartGPS = clockData.dwTotalSecondsFromStart;
}




void GPS_testGPSConnection(void)
{
	if(NMEAParserControl.m_dwCommandCount>0)
	{//received some commands, so speed is correct
		GPSControl.bGPSFailed = FALSE;
		return;	//we have connection on this speed
	}else if((clockData.dwTotalSecondsFromStart - GPSControl.dwSecondsOfStartGPS)>2)
	{
		//change UART speed
		if(GPSControl.uartSpeed==38400)
		{
			GPSControl.uartSpeed = 1200;
			GPSControl.bGPSFailed = TRUE;
		}else
			GPSControl.uartSpeed*=2;
		GPS_UART2_InitEx(GPSControl.uartSpeed);
	}
}



//ret 1 if GPS is ON
BOOL GPS_isTurnedON(void)
{
	return (PIN_GPS_O==0);
}

//take power off and turn off UART
void GPS_turnOFF(void)
{
	PCONP_bit.PCUART2 = 0;	//take power from UART2
	SET_GPS_O;	//take power from GPS
	GPSControl.bGPS_ON = 0;
}

//give power and adjust UART
void GPS_turnON(void)
{
	CLR_GPS_O;	//give power to GPS
	GPS_wakeup();
	GPS_UART2_Init();
	GPSControl.bGPS_ON = 1;
}

//sleep GPS
void GPS_sleep(void)
{
	if(PIN_RFPW)
	{//module in operating mode, sleep it
		CLR_GPS;	//sleep
	}
}

void GPS_wakeup(void)
{
	if(!PIN_RFPW)
	{//module in sleep mode, wake it up
		SET_GPS;	//wake up
	}
}


//send init seq to start GPS work
void GPS_sendInitSequence(void)
{
	const char query[]="$PSRF103,00,00,02,01";
	GPS_sendSequence(query);
	GPS_waitTrmEnd();
}

void GPS_waitTrmEnd(void)
{
	DWORD dw1, dw2;
	int i=10000000;
	do
	{
		dw1 = GPSControl.uart.trmBuffLen ;
		dw2 = GPSControl.uart.trmBuffLenConst ;
	}while(dw1!=dw2 && --i);
}

//send any sequence of text in GPS
void GPS_sendSequence(const char* sequence)
{
	int len = strlen(sequence);
	SAFE_DECLARE;
	DISABLE_VIC;
	for(int i=0;i<len;i++)
	{
		GPSControl.uart.trmBuff[i] = sequence[i];
	}
	GPSControl.uart.trmBuffLenConst = GPS_calcCRC(GPSControl.uart.trmBuff, len);
	GPS_StartTrm();
	ENABLE_VIC;
}

//start transmition data from first byte in buffer
void GPS_StartTrm(void)
{
	GPSControl.uart.trmBuffLen = 1;
	GPSControl.uart.rcvBuffLen = 0;
	GPSControl.uart.bRcvError = RCV_OK;
	U2THR = GPSControl.uart.trmBuff[0];
}

//ret len in bytes
int GPS_calcCRC(BYTE volatile * pData, int len)
{
	BYTE xor=0;
	int i;
	for(i=1;i<len;i++)
	{
		xor ^= pData[i];
	}
	pData[i++] = '*';
	char xx;
	xx = ((xor>>4)&0x0f)+'0';
	if(xx>'9')xx += 7;
	pData[i++] = xx;
	xx = (xor&0x0f)+'0';
	if(xx>'9')xx += 7;
	pData[i++] = xx;
	pData[i++] = '\r';
	pData[i++] = '\n';
	return i;
}


void GPS_ColdStart(void)
{
	const char query[]="$PSRF101,-2686700,-4304200,3851624,96000,497260,921,12,4";
	GPS_sendSequence(query);
	GPS_waitTrmEnd();
}

void GPS_WarmStart2(void)
{
	const char query[]="$PSRF101,-2686700,-4304200,3851624,96000,497260,921,12,3";
	GPS_sendSequence(query);
	GPS_waitTrmEnd();
}

void GPS_WarmStart(void)
{
	const char query[]="$PSRF101,-2686700,-4304200,3851624,96000,497260,921,12,2";
	GPS_sendSequence(query);
	GPS_waitTrmEnd();
}

void GPS_HotStart(void)
{
	const char query[]="$PSRF101,-2686700,-4304200,3851624,96000,497260,921,12,1";
	GPS_sendSequence(query);
	GPS_waitTrmEnd();
}

//it starts gps depends on value gpsstate
void GPS_startGPSbyState(void)
{
	if(GPSControl.gps_state!=GPS_STATE_ALWAYS_OFF)
		GPS_turnON();
	else
		GPS_turnOFF();
}

#endif	//#ifndef GPS_BT_FREE	
