//powerControl.c

#include <iolpc2388.h>

#include <string.h>

#include "types.h"
#include "syncObj.h"
#include "powerControl.h"
#include "display.h"
#include "keyboard.h"
#include "gps.h"
#include "bluetooth.h"
#include "interrupts.h"
#include "clock.h"
#include "sound.h"
#include "interProc.h"
#include "SPRD_mode.h"
#include "MCS_mode.h"
#include "LOG_mode.h"
#include "geiger.h"
#include "modes.h"
#include "spectrum.h"


struct tagPOWERCONTROL powerControl;


//turn off device mode
//!!!!!!!!!! dont call it from CLOCK int
void PowerControl_turnOFF_device(const char* pTextReason)
{
				
//	sound_playSample(SND_TURNOFF);
				
	//do out of mode
	Modes_setActiveMode(NULL);
	//
	
	SoundControl_StopBeep();
	SoundControl_StopVibro();
	PowerControl_turboModeOFF();
#ifndef GPS_BT_FREE	
	Bluetooth_turnOFF();
	GPS_turnOFF();
#endif	//#ifndef GPS_BT_FREE	


	//27/08/2012
	if(!powerControl.bBatteryAlarm)//только если батареи не сели даем звук, иначе будет неожиданное выключение из-за просевшего питания
		SoundControl_BeepSeq(beepSeq_OFF);
	else
		SoundControl_turnOFF();

	//added since 27/09/2010
	//turn off battery low to allow eeprom operations	
	powerControl.bBatteryAlarm = 0;
	

	//save sound state
	ini_write_system_ini_int("soundControl", "bSound", soundControl.bSound);
	
	
	LOGMode_insertEventByLang("Power OFF\0""Power OFF\0""Power OFF\0""Выключение");
	//add 27/09/2010
	if(strlen(pTextReason)!=0)
		LOGMode_insertEventByLang(pTextReason);
	

	//save essential data in eeprom (it is a eeprom writes counter )
	EEPROM_UpdateEssentialDataInEeprom();


	
	
	CIIR_bit.IMSEC = 0;	//turn off clock interrupts
	Display_turnOFF_LEDs();
	PowerControl_turnOFF_device_show_screen(pTextReason);
	
	PowerControl_sendAllCommands();

	SoundControl_turnOFF();
	
	while(KeyboardControl_anyKeyPressed_hardware_intcall())PowerControl_kickWatchDog();
	Display_turnOFF();
	
	CLR_AN_ON;	//turn off second proc
	CLR_DG_ON; //Выключение питания of first proc
	while(1)PowerControl_kickWatchDog();
}

//показать экран при выключении прибора
void PowerControl_turnOFF_device_show_screen(const char* pTextReason)
{
	Display_setTextSteps(1,1);//set steps
	Display_setTextWin(0,Y_SCREEN_MAX-216,X_SCREEN_SIZE,100);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextJustify(CENTER);
	Display_setTextDoubleHeight(0);
	
	Display_clearScreen();
	
	Display_setCurrentFont(fnt32x32);	//set current font
	Display_setTextColor(RED);	//set text color
	Display_outputTextByLang("POWER\r\0""POWER\r\0""POWER\r\0""ПИТАНИЕ\r");	//"ВЫКЛ"
	Display_setCurrentFont(fnt16x16);	//set current font
	Display_outputTextByLang("OFF\r\r\0""OFF\r\r\0""OFF\r\r\0""ВЫКЛЮЧЕНО\r\r");	//"ВЫКЛ"
	Display_setTextColor(GREEN);	//set text color
	Display_outputTextByLang(pTextReason);	//"ОТПУСТИТЕ КНОПКУ"
	Display_drawHLine(0,Y_SCREEN_MAX-184,X_SCREEN_MAX, RED);
}

//показать экран при усыплении прибора
void PowerControl_powerDown_show_screen(void)
{
	Display_setTextSteps(1,1);//set steps
	Display_setTextWin(0,Y_SCREEN_SIZE-216,X_SCREEN_SIZE,100);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextJustify(CENTER);
	Display_setTextDoubleHeight(0);
	
	Display_clearScreen();
	
	Display_setCurrentFont(fnt32x32);	//set current font
	Display_setTextColor(YELLOW);	//set text color
	Display_outputTextByLang("SLEEP\r\0""SLEEP\r\0""SLEEP\r\0""РЕЖИМ\r");	//"Дежурный"
	Display_setCurrentFont(fnt16x16);	//set current font
	Display_outputTextByLang("MODE\r\r\0""MODE\r\r\0""MODE\r\r\0""ДЕЖУРНЫЙ\r\r");	//"РЕЖИМ"
	Display_setTextColor(GREEN);	//set text color
	Display_outputTextByLang("RELEASE THE KEY\0""RELEASE THE KEY\0""RELEASE THE KEY\0""ОТПУСТИТЕ КНОПКУ");	//"ОТПУСТИТЕ КНОПКУ"
	Display_drawHLine(0,Y_SCREEN_SIZE-184,X_SCREEN_MAX, RED);
}

//send all buffer commands to frre buffer
void PowerControl_sendAllCommands(void)
{
	//send all commands
	int i;
	for(i=0;i<MAX_CMD_IN_ORDER;i++)
	{
		InterProc_InterProcControl();
		PowerControl_sleep(2000/MAX_CMD_IN_ORDER);	//wait here until all sent to second proc
	}
}

//подготовка и переход в режим пониженного потребления
//выход из режима и повторный вход если вышли по причине хода часов
//!!!!!!!!!! dont call it from CLOCK int
void PowerControl_gotoPowerDownMode(void)
{
	SoundControl_BeepSeq(beepSeq_OFF);
//	sound_playSample(SND_POWERDOWN);


	//need to switch second proc to measurement mode to be less sensitive to radiation fluctuation
	//InterProc_setMeasurementMode();
	SPRDModeControl.bBkgMode_confirmed = FALSE;
	SPRDModeControl.bBkgMode_assumed = TRUE;
	InterProc_setSearchMode();

	InterProc_goSleep();

	InterProc_readStatus();
	


	PowerControl_turboModeOFF();
	////////////////
	//dont forget turn on all of this after exit power down if needed
	SoundControl_StopVibro();
	SoundControl_StopBeep();

#ifndef GPS_BT_FREE	
	Bluetooth_turnOFF();
	//turn off gps only if we are NOT in mode "alway on"
	if(GPSControl.gps_state!=GPS_STATE_ALWAYS_ON)
		GPS_turnOFF();
#endif	//#ifndef GPS_BT_FREE	
	
	/////////////////////
	PowerControl_powerDown_show_screen();

	PowerControl_sendAllCommands();


	SoundControl_turnOFF();
	
	while(KeyboardControl_anyKeyPressed_hardware_intcall())PowerControl_kickWatchDog();
	Display_turnOFF();
	
	//adjust wakeup interrupts
	INTWAKE_bit.EXTWAKE2 = 1; //key pushing
	INTWAKE_bit.EXTWAKE1 = 1; //waking up from second proc
	INTWAKE_bit.GPIO0WAKE = 1; //geiger
	INTWAKE_bit.RTCWAKE = 1; //clock
//        FIO0DIR = 0x037C51F0;
//        FIO1DIR = 0x137C4713;
//        FIO2DIR = 0x00000090;
//        FIO3DIR = 0x07000000;
//        FIO4DIR = 0x30003C00;
//        CLR_M_ON;
//        PINMODE0 = 0xAAAAAAAA;
//        PINMODE1 = 0xAAAAAAAA;
//        PINMODE2 = 0xAAAAAAAA;
//        PINMODE3 = 0xAAAAAAAA;
//        PINMODE4 = 0xAAAAAAAA;
//
//        PINMODE6 = 0xAAAAAAAA;
//        PINMODE7 = 0xAAAAAAAA;
//        PINMODE8 = 0xAAAAAAAA;
//        PINMODE9 = 0xAAAAAAAA;
//        SET_ECS;
	
        soundControl.pBeepSeq = NULL;
        SoundControl_StopBeep();
        CLKSRCSEL_bit.CLKSRC=0;  //Inter OSC
        CCLKCFG = 0;
	do
	{
		PowerControl_kickWatchDog();
		
		//reset all awaking statuses before entering power down
		powerControl.bAwakedByKeyboard = 0;	//mean that processor is awaked by key pressing
		powerControl.bAwakedByClock = 0;	//mean that processor is awaked by clock (1 in a second)
		powerControl.bAwakedByGeiger = 0;
		powerControl.bAwakedByInterProc = 0;	//mean that processor is awaked by second processor
		
		powerControl.bInPowerDownMode = 1;

		Display_turnOFFRedLED();
//		Display_turnOFFOrangeLED();
		
		PowerControl_enterPowerDownMode();
		//next is used to sleep while bat ADC is not ready
		//and for process int that awake from power down
		PowerControl_sleep(1);//for first int
		PowerControl_sleep(1);//for second int

		if(powerControl.bControlBat)
		{//process battery
			PowerControl_controlBatStatus();
			powerControl.bControlBat = FALSE;
		}
		
	}while(!powerControl.bAwakedByKeyboard && !powerControl.bAwakedByInterProc
		   && !powerControl.bBatteryAlarm && !geigerControl.esentVals.bSafetyAlarm
			   && !geigerControl.esentVals.bOverload);


	//выход из повер дауна
	
	CLKSRCSEL_bit.CLKSRC=1;  //Main OSC
        CCLKCFG = 0;
	//awake second proc
	InterProc_goAwake();
	
	InterProc_readStatus();
//	PowerControl_sendAllCommands();
	
	
	powerControl.bInPowerDownMode = 0;
	
	//ждем пока отпустят кнопку
	while(KeyboardControl_anyKeyPressed_hardware_intcall())PowerControl_kickWatchDog();
		

	//состояние клавы как если ничего не нажато
	KeyboardControl_restoreKeyboard();


	SoundControl_turnON();
//	sound_ISD4004_Stop();

	
	CLR_NRES;
	SET_NRES;
	
	Display_turnON();
	Display_clearScreen();
	
	//turn on gps only if mode "off in sleep" is used
	//in mode always on it will be on,
	//in mode always off we dont need to turn it on
#ifndef GPS_BT_FREE
	if(GPSControl.gps_state!=GPS_STATE_ALWAYS_OFF)
		GPS_turnON();
		
	if(bluetoothControl.comm_power_state==enu_comm_power_state_on)
		Bluetooth_turnON();
#endif	//#ifndef GPS_BT_FREE
	
/*	if(!SPRDModeControl.bBkgMode_confirmed)
	{//switch to search mode after exit power down
		InterProc_setSearchMode();
	}*/

	//просыпаем режим
	
	if(SPRDModeControl.bMustSwitchToSPRD)
	{
		Modes_setActiveMode(NULL);
		Modes_setActiveMode(&modes_SPRDMode);
		SPRDModeControl.bMustSwitchToSPRD = FALSE;
	}else
		Modes_OnWakeUp();	
}

//all preparation to enter sleep mode and exit from it
void PowerControl_gotoIdleMode(void)
{
	PowerControl_turnOFF_MAM();
	PowerControl_EMC_OFF();
	//////////////////
	PowerControl_enterIdleMode();
	//////////////////
	PowerControl_turnON_MAM();
	Display_EMC_Init();
//	EMC_ON();
}


//use only after powerControl.bTurboMode is set correctly
//set divider to get proper freq for hardware any time we change cclk
//divider = 2 -> divide on 2
//divider = 3 -> divide on 8
void PowerControl_setPeripherialClockDivider(void)
{
	DWORD divider;
	if(powerControl.bTurboMode)
		divider = 0xffffffff;//3;
	else
		divider = 0xaaaaaaaa;//2;
	
	PCLKSEL0 = divider;
	PCLKSEL1 = divider;
	
/*
__REG32 PCLK_WDT    : 2;
__REG32 PCLK_TIMER0 : 2;
__REG32 PCLK_TIMER1 : 2;
__REG32 PCLK_UART0  : 2;
__REG32 PCLK_UART1  : 2;
__REG32 PCLK_PWM0   : 2;
__REG32 PCLK_PWM1   : 2;
__REG32 PCLK_I2C0   : 2;
__REG32 PCLK_SPI    : 2;
__REG32 PCLK_RTC    : 2;
__REG32 PCLK_SSP1   : 2;
__REG32 PCLK_DAC    : 2;
__REG32 PCLK_ADC    : 2;
__REG32 PCLK_CAN1   : 2;
__REG32 PCLK_CAN2   : 2;
__REG32 PCLK_ACF    : 2;

*/
	
	/*
	PCLKSEL0_bit.PCLK_WDT = divider;
	PCLKSEL0_bit.PCLK_TIMER0 = divider;
	PCLKSEL0_bit.PCLK_TIMER1 = divider;
	PCLKSEL0_bit.PCLK_UART0 = divider;
	PCLKSEL0_bit.PCLK_UART1 = divider;
	PCLKSEL0_bit.PCLK_ADC = divider;
	PCLKSEL0_bit.PCLK_PWM1 = divider;
	PCLKSEL0_bit.PCLK_PWM0 = divider;
	PCLKSEL0_bit.PCLK_I2C0 = divider;
	PCLKSEL0_bit.PCLK_SPI = divider;
	PCLKSEL0_bit.PCLK_RTC = divider;
	PCLKSEL0_bit.PCLK_SSP1 = divider;
	PCLKSEL0_bit.PCLK_DAC = divider;
	PCLKSEL0_bit.PCLK_CAN1 = divider;
	PCLKSEL0_bit.PCLK_CAN2 = divider;
	PCLKSEL0_bit.PCLK_ACF = divider;
*/

/*
__REG32 PCLK_BAT_RAM: 2;
__REG32 PCLK_GPIO   : 2;
__REG32 PCLK_PCB    : 2;
__REG32 PCLK_I2C1   : 2;
__REG32             : 2;
__REG32 PCLK_SSP0   : 2;
__REG32 PCLK_TIMER2 : 2;
__REG32 PCLK_TIMER3 : 2;
__REG32 PCLK_UART2  : 2;
__REG32 PCLK_UART3  : 2;
__REG32 PCLK_I2C2   : 2;
__REG32 PCLK_I2S    : 2;
__REG32 PCLK_MCI    : 2;
__REG32             : 2;
__REG32 PCLK_SYSCON : 2;
*/	
	/*
	PCLKSEL1_bit.PCLK_GPIO = divider;
	PCLKSEL1_bit.PCLK_PCB = divider;
	PCLKSEL1_bit.PCLK_TIMER2 = divider;
	PCLKSEL1_bit.PCLK_TIMER3 = divider;
	PCLKSEL1_bit.PCLK_UART2 = divider;
	PCLKSEL1_bit.PCLK_UART3 = divider;
	PCLKSEL1_bit.PCLK_BAT_RAM = divider;
	PCLKSEL1_bit.PCLK_I2C1 = divider;
	PCLKSEL1_bit.PCLK_SSP0 = divider;
	PCLKSEL1_bit.PCLK_I2C2 = divider;
	PCLKSEL1_bit.PCLK_I2S = divider;
	PCLKSEL1_bit.PCLK_MCI = divider;
	PCLKSEL1_bit.PCLK_SYSCON = divider;
	*/
}








//initial power supply for hardware
//if gone use some we need to turn on any of them
//else we must turn off them
void PowerControl_Init(void)
{
//	CLR_DPWON;//
//	SET_PON;
//	SET_MON;
	
	DIR_AN_RES = 1;
	SET_AN_RES;
	
	DIR_AN_X = 0;
	
	DIR_DG_ON = 1; //Нога включения питания
	SET_DG_ON; //Включение питания

	//для нормальной работы второго проца, если в 0 то прошивка
	DIR_AN_PGM = 1;
	SET_AN_PGM;
	
	DIR_AN_ON = 1;	//pin to switch off second proc
	PINMODE3_bit.P1_20 = 0x2;	//neithrt pull up and down mode
	
	SET_AN_ON;	//by default it is ON
	
	DIR_AN_ERR = 0;	//работет как вход
	
	DIR_PRG_EN = 0;	//input

	
	//выключаем питание со всего что можно
	PCONP = 0;
	PCONP_bit.PCRTC = 1;
	PowerControl_EMC_OFF();
	
	powerControl.dwIdleTime = 0;	//must be reset before main cycle

	powerControl.bAwakedByKeyboard = 0;	//mean that processor is awaked by key pressing
	powerControl.bAwakedByClock = 0;	//mean that processor is awaked by clock (1 in a second)
	powerControl.bAwakedByGeiger = 0;
	powerControl.bAwakedByInterProc = 0;	//mean that processor is awaked by second processor
	powerControl.bBatteryAlarm = 0;
	powerControl.dwBatteryAlarmCnt = 0;
	powerControl.bInPowerDownMode = 0;
	
	powerControl.ADC_REG = 0;
	powerControl.batV = 3;	//voltage in V
	powerControl.batV_aver = 3;	//voltage in V
	powerControl.batStatus = 1;	//>0-charged, 0-discharged
	powerControl.batCapacity=100;	//in percent
//	powerControl.batCapacityMom=100;	//in percent
	
	powerControl.dwPowerDownDeadTime = 300;	//by default 300 s
	
	powerControl.bControlBat = FALSE;
	
	powerControl.fBatCoef = DEF_BAT_COEF;
	
}

//включить контроллер внешней памяти/шины
void PowerControl_EMC_ON(void)
{
  PCONP_bit.PCEMC = 1;  //EMC power ON
}
//выключить контроллер внешней памяти/шины
void PowerControl_EMC_OFF(void)
{
  PCONP_bit.PCEMC = 0;  //EMC power OFF
}

//switch processor to IDLE state
void PowerControl_enterIdleMode(void)
{
  PCON_bit.IDL = 1;
}

//switch processor to IDLE state
void PowerControl_enterPowerDownMode(void)
{
//  PCON_bit.PD = 1;
  PCON = 0x00000002;
}


//turn on memory acceleration controller
void PowerControl_turnON_MAM(void)
{
  PowerControl_turnOFF_MAM();
  MAMTIM_bit.CYCLES = 3;  //3 cycles for FSCO > 40 MHz
  MAMCR_bit.MODECTRL = 2; //full mode
}

//turn off memory acceleration controller
void PowerControl_turnOFF_MAM(void)
{
  MAMCR_bit.MODECTRL = 0;
}


//повышаем частоту до порядка 70 МГц
void PowerControl_turboModeON(void)
{
	
	SAFE_DECLARE;
	DISABLE_VIC;
	
    if(!powerControl.bTurboMode)
    {
		powerControl.bTurboMode=1;
		
		CCLKCFG = 3;  //divider (4) of PPL signal = 294,912 MHz
		PLLCFG_bit.MSEL = 7;      // multiplier = 8
		PLLCFG_bit.NSEL = 0;    // divider = 1, CCO = 294,912 MHz
		
		PLLCON_bit.PLLE = 1;     // Enable PLL
		PLLFEED = PLLFEED_DATA1;
		PLLFEED = PLLFEED_DATA2;
		
		while (!PLLSTAT_bit.PLOCK); // Wait PLL lock
		
		PLLCON_bit.PLLC = 1;	// Connect PLL

		PowerControl_setPeripherialClockDivider();

		PLLFEED = PLLFEED_DATA1;
		PLLFEED = PLLFEED_DATA2;
    }else
	    PowerControl_setPeripherialClockDivider();
	
	ENABLE_VIC;
}

//понижаем частоту до 18МГц
void PowerControl_turboModeOFF(void)
{
	SAFE_DECLARE;
	DISABLE_VIC;
	
	if(powerControl.bTurboMode)
	{
		powerControl.bTurboMode = 0;
		
		PLLCON_bit.PLLC = 0;	// disConnect PLL
		PLLFEED = PLLFEED_DATA1;
		PLLFEED = PLLFEED_DATA2;
		PLLCON_bit.PLLE = 0;     // disable PLL
		PLLFEED = PLLFEED_DATA1;
		PLLFEED = PLLFEED_DATA2;
		while (PLLSTAT_bit.PLOCK); // Wait PLL lock
		CCLKCFG = 0;  //divider (4) of PPL signal = 294,912 MHz
	}
	
	PowerControl_setPeripherialClockDivider();
		
	ENABLE_VIC;
}

//инициализация АЦП
void PowerControl_Bat2ADC_Init(void)
{
	PCONP_bit.PCAD = 1;	//give power on ADC
	PINSEL1_bit.P0_26 = 0x01;	//switch P0_26 to AD0.3
	PINMODE1_bit.P0_26 = 0x02;	//nor pullup nor puldown resistor
	AD0CR_bit.SEL = 0x08;	//AD7.0
	AD0CR_bit.BURST = 0;	//software mode
	AD0CR_bit.CLKDIV = 0xff;
	AD0CR_bit.CLKS = 0x00;	//10 bit ADC
	ADINTEN_bit.ADGINTEN = 1;//enable global int
	powerControl.batV_Number = 0;
	powerControl.batV_Index = 0;
	powerControl.bShow_Bat_sym = 0;
}




//ret !=0 if OK, 0 if discharged
BOOL PowerControl_getBatStatus(void)
{
	//return !(GET_LBO==0);
        return 1;  //Для тестирования 17.07.2015
}


//отобразить статус батарей на экране
void PowerControl_showBatStatus(int x)
{
	SAFE_DECLARE;
	DISABLE_VIC;
	BOOL bst = powerControl.bBatteryAlarm;
	int cap = powerControl.batCapacity;
	ENABLE_VIC;
	if(bst)
	{//bat alarm
		powerControl.bShow_Bat_sym = !powerControl.bShow_Bat_sym;
	}else
	{
		powerControl.bShow_Bat_sym = 1;
	}
	if(powerControl.bShow_Bat_sym)
	{
		if(bst)//bat alarm
			cap = 0;	//almost discharged
		
		if(cap>75)
		{
			Display_output_bmp(x,0, (BITMAPFILEHEADER*)bmp_bat4);
		}else if(cap>50)
		{
			Display_output_bmp(x,0, (BITMAPFILEHEADER*)bmp_bat3);
		}else if(cap>25)
		{
			Display_output_bmp(x,0, (BITMAPFILEHEADER*)bmp_bat2);
		}else if(cap>12)
		{
			Display_output_bmp(x,0, (BITMAPFILEHEADER*)bmp_bat1);
		}else
		{
			Display_output_bmp(x,0, (BITMAPFILEHEADER*)bmp_bat0);
		}
	}else
	{
		RECT rect = {x,0,x+23,16};
		Display_clearRect(rect, 100);
	}
}

/*
//тестовая по отображению состояния батарей
void PowerControl_display_bat_status(void)
{
	char buff[100];
	char buff2[25];
	sprintf((char*)&buff[0], "ADC_REG: %u\r",(UINT)powerControl.ADC_REG);
	sprintf((char*)&buff2[0], "volt: %f\r",(double)powerControl.batV);
	strcat(buff, buff2);
	sprintf((char*)&buff2[0], "cap: %d\r",(int)powerControl.batCapacity);
	strcat(buff, buff2);
	sprintf((char*)&buff2[0], "batStatus: %u",(UINT)powerControl.batStatus);
	strcat(buff, buff2);
	Display_setTextWin(0,150,X_SCREEN_SIZE,50);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);	//set current font
	Display_clearTextWin(100);
	Display_outputText(buff);
}
*/

void PowerControl_watchDog_Init(void)
{
	WDTC = 0x800000;	//reset for about of every 3.64 s
	WDCLKSEL_bit.WDSEL = 0x01;
	WDMOD_bit.WDTOF = 0;
	
#ifdef DEBUG
	WDMOD_bit.WDRESET = 0;	//must be =1 for RELEASE MODE
#else
	WDMOD_bit.WDRESET = 1;	//must be =1 for RELEASE MODE
#endif	//#ifdef DEBUG
	
	WDMOD_bit.WDEN = 1;
}



//sleep running for specified ms
//all interrupts are enabled
//!!!!!!!!!! dont call it from CLOCK int
void PowerControl_sleep(DWORD ms)
{
	if(ms==0)return;
	
	SAFE_SET(clockData.dwMiliseconds,0);
	
	CISS_bit.SUBSECSEL = 1; //every 977 mks
	CISS_bit.SUBSECENA = 1; //enable subseconds int
	
	while(SAFE_GET_DWORD(&clockData.dwMiliseconds)<ms)
	{
		PowerControl_kickWatchDog();
		PowerControl_gotoIdleMode();
	};
	
	CISS_bit.SUBSECENA = 0; //disable subseconds int
}


void PowerControl_kickWatchDog(void)
{
	//kick watchdog
	WDFEED = 0xaa;
	WDFEED = 0x55;
}

//выполнение преобразования для уровня напряжения от батарей
__arm void _INT_ADC_PowerControl(void)
{
	if(AD0GDR_bit.DONE && /*ADC ready*/
	   !powerControl.bControlBat /*prev bat status alread controled*/)
	{
		powerControl.ADC_REG = ADDR3_bit.RESULT;
		AD0CR_bit.START = 0;
		AD0CR_bit.PDN = 0;	//turn on power down mode of ADC
		powerControl.batV = (float)VREF*powerControl.fBatCoef*1.44*powerControl.ADC_REG/1023.0;
		powerControl.bControlBat = TRUE;
	}
}



//control averaging and store
//здесь измеряем и сохраняем значение напряжения, расч знач емкости и усредняем знач емкости за 10 секунд
//устанавливаем все флаги о разряде и т.д.
//выдача знач емкости идет после первого измерения
//обязательно должны быть инициализированы изначально:
//	powerControl.batCapNumber = 0;
//	powerControl.batCapIndex = BATCAP_NUM_MAX;
void PowerControl_controlBatStatus(void)
{
	if(powerControl.batV_Number<BATCAP_NUM_MAX)
	{
		powerControl.batV_Number++;
	}
	powerControl.batV_Ar[powerControl.batV_Index] = powerControl.batV;
	float aver = 0;
	int idx;
	for(int i=0;i<powerControl.batV_Number;i++)
	{
		idx = powerControl.batV_Index-i;
		if(idx<0)idx+=BATCAP_NUM_MAX;
		aver+=powerControl.batV_Ar[idx];
	}
	aver/=powerControl.batV_Number;
	powerControl.batV_aver = aver;

	if(++powerControl.batV_Index>=BATCAP_NUM_MAX)
		powerControl.batV_Index = 0;

	powerControl.batCapacity = (powerControl.batV_aver-VREF_BAT_MIN)*100/(VREF_BAT_MAX-VREF_BAT_MIN);
	if(powerControl.batCapacity>100)
		powerControl.batCapacity = 100;
	else if(powerControl.batCapacity<5)
		powerControl.batCapacity = 5;

/*
	powerControl.batCapacityMom = (powerControl.batV-VREF_BAT_MIN)*100/(VREF_BAT_MAX-VREF_BAT_MIN);
	if(powerControl.batCapacityMom>100)
		powerControl.batCapacityMom = 100;
	else if(powerControl.batCapacityMom<0)
		powerControl.batCapacityMom = 0;
	if(powerControl.batCapNumber<BATCAP_NUM_MAX)
	{
		powerControl.batCapNumber++;
		powerControl.batCapacityAr[powerControl.batCapIndex] = powerControl.batCapacityMom;
	}else
	{
		powerControl.batCapacityAr[powerControl.batCapIndex] = powerControl.batCapacityMom;
	}
	int aver = 0, idx;
	for(int i=0;i<powerControl.batCapNumber;i++)
	{
		idx = powerControl.batCapIndex-i;
		if(idx<0)idx+=BATCAP_NUM_MAX;
		aver+=powerControl.batCapacityAr[idx];
	}
	aver/=powerControl.batCapNumber;
	powerControl.batCapacity = aver;
	
	*/
	
	
	powerControl.batStatus = PowerControl_getBatStatus();
	if(!powerControl.batStatus || powerControl.batV_aver<VREF_BAT_MIN_CRITICAL)
	{//LOW BATTERY!!!!
/*
		if(!powerControl.bBatteryAlarm && //no low battery before
		   modeControl.pMode==&modes_SpectrumMode && //RID mode
		   !spectrumControl.bStopAcq && //spectrum is acquiring
			!powerControl.bInPowerDownMode //not in power down)
		{//save spectrum with auto name
			SPRDMode_saveAutoSpec();
			InterProc_stopSpectrumAcq();
		}
		*/
		if(!powerControl.bBatteryAlarm)
		{
			LOGMode_insertEventByLang("Battery low\0""Battery low""Battery low\0""Батареи разряжены");
			//выключить потребители
#ifndef GPS_BT_FREE	
			GPS_turnOFF();
			Bluetooth_turnOFF();
#endif	//#ifndef GPS_BT_FREE	
		}
		//disable switch to power down mode setting 1 to bat alarm status
		//disable write file operations
		powerControl.bBatteryAlarm = 1;
	}
	
	if(powerControl.bBatteryAlarm)
		powerControl.dwBatteryAlarmCnt++;	//time of alarm status
	else
	{
		powerControl.dwBatteryAlarmCnt = 0;
	}
}



//start ADC for battery voltage
//run from interrupt
//result in interrupt
void PowerControl_startADC_intcall(void)
{
	AD0CR_bit.PDN = 1;	//turn off power down mode of ADC
	AD0CR_bit.START = 0x01;	//start ADC
}



/*
//ret bat voltage in V
float PowerControl_getBatVoltage(void)
{
	DWORD r3=10;
	AD0CR_bit.PDN = 1;	//turn off power down mode of ADC
	PowerControl_sleep(10);	
	AD0CR_bit.START = 0x01;	//start ADC
	do
	{
		PowerControl_sleep(20);
	}while(!AD0GDR_bit.DONE && --r3);
	if(!r3)
	{
		exception(__FILE__,__FUNCTION__,__LINE__, "Failed to get battery voltage");
		return -1;
	}
	r3 = ADDR3_bit.RESULT;
	powerControl.ADC_REG = r3;
	AD0CR_bit.PDN = 0;	//turn on power down mode of ADC
	return (float)VREF*2*r3/1023.0;
}
*/


//awaking from second processor. ALARM
//processing search alarm
__arm void _INT_WAKEUP_PowerControl(void)
{
	powerControl.bAwakedByInterProc = 1;	//mean that processor is awaked by second processor

	if(powerControl.bInPowerDownMode //in a power down mode
	/*&& modeControl.pMode!=&modes_SPRDMode*/ //с этой строкой не переходит в идентификацию при пробуждении
	   )
	{//sleep mode and not a SPRD mode
		SPRDModeControl.bMustSwitchToSPRD = TRUE;
	}
	
	if(!SPRDModeControl.bBkgMode_assumed //not a background mode
	   && !geigerControl.esentVals.bSafetyAlarm) //not a safety alarm
	{
		if(modeControl.pMode==&modes_SPRDMode ||
		   modeControl.pMode==&modes_MCSMode)
		{
			SET_SND_AU;
			SoundControl_Alarm_intcall(50, 1000);
		}
		
		//radfound flag in SPRD mode only
		SPRDModeControl.bRadFound=TRUE;
		
		powerControl.dwIdleTime = 0;	//reset idle time counter
	}	
	EXTINT_bit.EINT1 = 1;	//clear INT
}


//start boot loader of first proc
void PowerControl_startBootLoader(void)
{

	Display_prepareEmergencyTextControl();
	
	Display_outputText("Second processor update procedure\r");
	Display_outputText("Run flasher at baud rate 57600\r\r");
//	Display_outputText("Note: to update first processor turn on device by press and hold left and power keys simultaneously and hold them until completion of flashing\r");
	
	CLR_AN_PGM;
	CLR_AN_RES;
	PowerControl_sleep(300);
	SET_AN_RES;
//	CLR_AN_ON;	//turn off second proc
//	PowerControl_sleep(2000);
//	SET_AN_ON;	//turn on second proc
	PowerControl_sleep(2000);
	__disable_interrupt();
	
	
	
	//ready to update second proc firmware
	
	////////////////////////////////////////////
	//adjust USB UART
	//enable access to divisor latch regs
	U0IER = 0;	//disable int
	U0LCR = 0x83;        //LCR_ENABLE_LATCH_ACCESS;
	// set divisor for desired baud
	//divider = HW_FREQ/UART_baudrate/16
	DWORD div = HW_FREQ/57600/16;
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
	// disable UART0 interrupts
	U0IER = 0x0;	


	////////////////////////////////////////////
	//adjust second proc UART
	// enable access to divisor latch regs
	U1IER = 0;	//disable int
	U1LCR = 0x83;        //LCR_ENABLE_LATCH_ACCESS;
	// set divisor for desired baud
	//divider = HW_FREQ/UART_baudrate/16
	div = HW_FREQ/57600/16;
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
	
	// disable UART0 interrupts
	U1IER = 0x0;	
	
	//turn on watch dog
#ifdef DEBUG
	WDMOD_bit.WDRESET = 1;	//must be =1 for RELEASE MODE
#endif	//#ifdef DEBUG

	
	
	//transmition from PC to second PROC
	//end operation by wathdog
	int keycnt = 0;
	do
	{
		if((U0LSR&0x1) && (U1LSR&0x20))
		{
			U1THR = U0RBR;
		};
		if((U1LSR&0x1) && (U0LSR&0x20))
		{
			U0THR = U1RBR;
		};
		if(KeyboardControl_testKeyDown())
			keycnt++;
		PowerControl_kickWatchDog();
	}while(keycnt<100);
	
	CLR_AN_ON;	//turn off second proc
	CLR_DG_ON; //Выключение питания of first proc

	__enable_interrupt();
	PowerControl_sleep(2000);
}













//emergency check battery status and exit if it is low
void PowerControl_emergencyCheckBattery(void)
{
	//result we will have in interrupt
	int i, batres = 0;
	for(i=0;i<9;i++)
	{
		PowerControl_startADC_intcall();
		PowerControl_sleep(300);
		batres += !PowerControl_getBatStatus();
	}
	if(batres==9)
	{//low bat, turn device off
		PowerControl_turnOFF_device("Battery low!\0""Battery low!\0""Battery low!\0""Батареи разряжены!");
	}
}


/*
void pause(int timeout)
{
	int i,j,k=0;
	for(i=0;i<timeout;i++)
	{
		for(j=0;j<10000;j++)k++;
	}
}
*/
void pause(int timeout)
{
	int i,j,k=0;
        DIR_P4_28 = 1;
        DIR_P4_29 = 1;
	for(i=0;i<timeout;i++)
	{
		//for(j=0;j<10000;j++)k++;
                CLR_P4_28;
                SET_P4_28;
                CLR_P4_28;
                SET_P4_28;

	}
}