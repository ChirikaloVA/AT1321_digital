//powerControl.c

#ifndef _POWERCONTROL_H
#define _POWERCONTROL_H

#include <iolpc2388.h>
#include "types.h"

//количество значений емкости для усреднения и получения итоговой емкости батарей
#define BATCAP_NUM_MAX 30

//seconds to turn off device if bBatteryAlarm==1
#define BAT_LOW_TIME_TO_OFF 30


//ms to enter power down mode by key press
#define TIME_TO_ENTER_POWERDOWN_BY_KEY 2000


//минимальное и максимальное значения коэф батарей
#define MIN_BAT_COEF 0.5
#define MAX_BAT_COEF 2.0
#define DEF_BAT_COEF 1.0


struct tagPOWERCONTROL
{
	//=1 тогда 73 МГц, иначе 18 МГц
	BOOL bTurboMode;

	////////////////////
	//set in interrupt
	volatile WORD ADC_REG;
	volatile float batV;	//voltage in V
	float batV_aver;	//voltage in V
	BOOL batStatus;	//>0-charged, 0-discharged (by hardware)
	BOOL bShow_Bat_sym;	//used to show bat symbol: 1=show,0=hide
	int batCapacity;	//in percent
//	volatile int batCapacityMom;	//in percent
	float batV_Ar[BATCAP_NUM_MAX];	//in percent averaged
	int batV_Number;	//number of averaged
	int batV_Index;	//index of record
	float fBatCoef;	//calibratino coefficient of battery


	///////////////////////
	//set in interrupts
	//these values must be cleared before enter power down
	volatile BOOL bAwakedByKeyboard;	//mean that processor is awaked by key pressing
	volatile BOOL bAwakedByClock;	//mean that processor is awaked by clock (1 in a second)
	volatile BOOL bAwakedByInterProc;	//mean that processor is awaked by second processor
	volatile BOOL bAwakedByGeiger;	//mean that processor is awaked by geiger
	volatile BOOL bBatteryAlarm;	//0-charged, 1-discharged, if 1 then no power down mode
	
	
	volatile DWORD dwBatteryAlarmCnt;	//counter of low battery alarm if > BAT_LOW_TIME_TO_OFF then turnOFF
	
	volatile BOOL bControlBat;	//if 1 then ADC was calculate battery voltage and we can process battery status
	//////////////////////
	
	//исп для разрешения ситуации когда мы должны войти в повердаун и тут приходит прерывание от второго проца
	//оно включает вибру, звук и красный лед и это все не выключится
	BOOL bInPowerDownMode;	//1-if we are in power down
	
	//seconds, how many time processor 1 is in Idle mode
	//used to enter power down mode
	//must be set to 0 before start work and after process any keys
	//must be reset after powerdown
	volatile DWORD dwIdleTime;	
	DWORD dwPowerDownDeadTime;	//s, time to enter power down must be saved in ini	
};

extern struct tagPOWERCONTROL powerControl;


#define PLLFEED_DATA1 0xaa
#define PLLFEED_DATA2 0x55

//first proc power control: 1-turn OFF
#define DIR_DG_ON FIO1DIR_bit.P1_21
#define SET_DG_ON MY_FIO1SET(B_21)
#define CLR_DG_ON MY_FIO1CLR(B_21)
//second proc power conrtol: 1-turn OFF
#define DIR_AN_ON FIO1DIR_bit.P1_20
#define SET_AN_ON MY_FIO1SET(B_20)
#define CLR_AN_ON MY_FIO1CLR(B_20)
//second proc reset
#define DIR_AN_RES FIO1DIR_bit.P1_22
#define SET_AN_RES MY_FIO1SET(B_22)
#define CLR_AN_RES MY_FIO1CLR(B_22)

//unused signal
#define DIR_AN_X FIO1DIR_bit.P1_23
#define SET_AN_X MY_FIO1SET(B_23)
#define CLR_AN_X MY_FIO1CLR(B_23)

//second proc power error status
#define DIR_AN_ERR FIO4DIR_bit.P4_30
#define SET_AN_ERR MY_FIO4SET(B_30)
#define CLR_AN_ERR MY_FIO4CLR(B_30)
#define PIN_AN_ERR FIO4PIN_bit.P4_30

//second proc updater
#define DIR_AN_PGM FIO4DIR_bit.P4_15
#define SET_AN_PGM MY_FIO4SET(B_15)
#define CLR_AN_PGM MY_FIO4CLR(B_15)
#define PIN_AN_PGM FIO4PIN_bit.P4_15


#define DIR_LBO FIO2DIR_bit.P2_13
#define GET_LBO FIO2PIN_bit.P2_13




#define OSC_FREQ  (ULONG)18432000
#define HW_FREQ  (ULONG)OSC_FREQ/2



//flash re-program enable
#define DIR_PRG_EN FIO2DIR_bit.P2_10
#define SET_PRG_EN MY_FIO2SET(B_10)
#define CLR_PRG_EN MY_FIO2CLR(B_10)
#define PIN_PRG_EN FIO2PIN_bit.P2_10


//////////////////////////////////

#define DIR_DPWON FIO1DIR_bit.P1_0
#define SET_DPWON MY_FIO1SET(B_0)
#define CLR_DPWON MY_FIO1CLR(B_0)




#define DIR_PON FIO0DIR_bit.P0_25
#define SET_PON MY_FIO0SET(B_25)
#define CLR_PON MY_FIO0CLR(B_25)

#define DIR_MON FIO0DIR_bit.P0_24
#define SET_MON MY_FIO0SET(B_24)
#define CLR_MON MY_FIO0CLR(B_24)


//=========================

#define DIR_P4_28 FIO4DIR_bit.P4_28
#define SET_P4_28 MY_FIO4SET(B_28)
#define CLR_P4_28 MY_FIO4CLR(B_28)

#define DIR_P4_29 FIO4DIR_bit.P4_29
#define SET_P4_29 MY_FIO4SET(B_29)
#define CLR_P4_29 MY_FIO4CLR(B_29)







//опорное напряжение на АЦП измерителя напряжения
#define VREF 3.3
//меньше этого то разряд и выключение
#define VREF_BAT_MIN_CRITICAL 2.05
#define VREF_BAT_MIN VREF_BAT_MIN_CRITICAL
#define VREF_BAT_MAX 2.4


BOOL PowerControl_getBatStatus(void);

void PowerControl_show_bat_status(void);

void PowerControl_Bat2ADC_Init(void);


void PowerControl_turnOFF_device_show_screen(const char* pTextReason);
void PowerControl_powerDown_show_screen(void);


//float PowerControl_getBatVoltage(void);

void PowerControl_enterIdleMode(void);

void PowerControl_Init(void);

void PowerControl_setPeripherialClockDivider(void);

void PowerControl_gotoIdleMode(void);
void PowerControl_enterPowerDownMode(void);
void PowerControl_gotoPowerDownMode(void);

void PowerControl_turnOFF_device(const char* pTextReason);

void PowerControl_turnOFF_MAM(void);
void PowerControl_turnON_MAM(void);

void PowerControl_turboModeOFF(void);
void PowerControl_turboModeON(void);
void PowerControl_EMC_OFF(void);
void PowerControl_EMC_ON(void);

void PowerControl_showBatStatus(int x);

//control averaging and store
void PowerControl_controlBatStatus(void);

void PowerControl_watchDog_Init(void);


void PowerControl_sleep(DWORD ms);

void PowerControl_kickWatchDog(void);

__arm void _INT_ADC_PowerControl(void);
__arm void _INT_WAKEUP_PowerControl(void);

void PowerControl_startADC_intcall(void);

void PowerControl_startBootLoader(void);


void PowerControl_emergencyCheckBattery(void);

void pause(int timeout);


void PowerControl_sendAllCommands(void);




#endif  //#ifndef _POWERCONTROL_H
