#ifndef _GPS_H
#define _GPS_H


#include <iolpc2388.h>
#include "types.h"

//power on/off GPS module
#define DIR_GPS_O FIO2DIR_bit.P2_7
#define SET_GPS_O MY_FIO2SET(B_7)
#define CLR_GPS_O MY_FIO2CLR(B_7)
#define PIN_GPS_O FIO2PIN_bit.P2_7

//================================

#define DIR_1PPS FIO1DIR_bit.P1_29
#define SET_1PPS MY_FIO1SET(B_29)//IO1SET_bit.P1_29
#define CLR_1PPS MY_FIO1CLR(B_29)//IO1CLR_bit.P1_29

//read power mode 0-sleep, 1-operate
#define DIR_RFPW FIO1DIR_bit.P1_30
#define PIN_RFPW FIO1PIN_bit.P1_30
//wake gps up
#define DIR_GPS FIO1DIR_bit.P1_31
#define SET_GPS MY_FIO1SET(B_31)
#define CLR_GPS MY_FIO1CLR(B_31)
#define PIN_GPS FIO1PIN_bit.P1_31

enum enuGPSState
{
	GPS_STATE_OFF_IN_SLEEP=0x10000, //it will be off in sleep and on after sleep
	GPS_STATE_ALWAYS_ON,	//it will be on even in sleep mode
	GPS_STATE_ALWAYS_OFF,	//it can be turn on manually but in sleep mode it will be turn off
};


struct tagGPSControl
{
	BOOL bGPS_ON;	//GPS is ON or OFF
	BOOL bGPS_Fix;	//have any FIX
	BOOL bShow_GPS_sym;	//symbol status
	
	//need to save it in eeprom
	enum enuGPSState gps_state;	//state by default (always on, or off)
	
#define GPS_RCV_BUF_LEN 512
#define GPS_TRM_BUF_LEN 70
//#pragma pack(1)
	volatile BYTE rcvBuff[GPS_RCV_BUF_LEN];
	volatile BYTE trmBuff[GPS_TRM_BUF_LEN];
	BYTE rcvBuff_safe[GPS_RCV_BUF_LEN];
//#pragma pack()

	struct tagUART uart;
	ULONG dwSecondsOfStartGPS;
	
	DWORD uartSpeed;
	BOOL bGPSFailed;//true if no connection to GPS
};

#ifndef GPS_BT_FREE	


BOOL GPS_isTurnedON(void);

extern struct tagGPSControl GPSControl;

void GPS_show_symbol(int x);

void GPS_Init(void);

void GPS_control(void);
void GPS_UART2_Init(void);

__arm void _INT_UART2_GPS(void);

void GPS_turnON(void);
void GPS_turnOFF(void);
void GPS_sendInitSequence(void);

void GPS_sendSequence(const char* initSeq);

void GPS_wakeup(void);
void GPS_sleep(void);
void GPS_waitTrmEnd(void);


void GPS_StartTrm(void);
int GPS_calcCRC(BYTE volatile * pData, int len);


void GPS_sym_control(void);


void GPS_HotStart(void);
void GPS_WarmStart(void);
void GPS_WarmStart2(void);
void GPS_ColdStart(void);

void GPS_startGPSbyState(void);

void GPS_testGPSConnection(void);
void GPS_UART2_InitEx(DWORD speed);

#endif	//#ifndef GPS_BT_FREE	

#endif //#ifndef _GPS_H
