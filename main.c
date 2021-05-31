//main.c

//#include <stdio.h>
//#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>

#include "main.h"
#include "display.h"
#include "keyboard.h"
#include "types.h"
#include "interrupts.h"
#include "syncObj.h"
#include "powerControl.h"
#include "clock.h"
#include "sound.h"
#include "gps.h"
#include "bluetooth.h"
#include "interProc.h"
#include "string.h"
#include "NMEA_Parser.h"
#include "USBRS.h"
#include "spectrum.h"
#include "modes.h"
#include "geiger.h"
#include "eeprom.h"
#include "SPRD_mode.h"
#include "filesystem.h"
#include "ini_control.h"
#include "identify.h"
#include "USB_mode.h"
#include "STAB_mode.h"
#include "Setup_mode.h"
#include "LOG_mode.h"
#include "crc32.h"
#include "info_mode.h"
#include "packspectrum.h"




/*
кодирование версий: главный номер, точка, второстепенный номер, буква исполнения
буква исполнения:
A - ядерные исполнения
B - безядерные исполнения
*/


#ifndef SUBVER
#error "SUBVER not set!!!"
#endif


//#define VERT(maj, min) "FirmWare ver. " #maj "." #min " "SUBVER"\0""FirmWare ver. " #maj "." #min "\0""FirmWare ver. " #maj "." #min "\0""Версия ПО " #maj "." #min "\0";

#define MAJOR "4"
#define MINOR "12"


const char txtVersion[]="FirmWare ver. " MAJOR "." MINOR SUBVER"\0""FirmWare ver. " MAJOR "." MINOR SUBVER"\0""FirmWare ver. " MAJOR "." MINOR SUBVER"\0""Версия ПО " MAJOR "." MINOR SUBVER"\0";
//#ifdef _SNM
//#ifdef _IAEA
//I
//#else
//A
//#endif
//#else	//#ifdef _SNM
//#ifdef BNC
//C
//#else
//B
//#endif
//#endif	//#ifdef _SNM
//);


//const char txtVersion[]="FirmWare ver. 4.00\0""FirmWare ver. 4.00\0""FirmWare ver. 4.00\0""Версия ПО 4.00";

//#define OUR__DATE__ "26.02.2014"
#define OUR__DATE__ "07.05.2020"

const char txtCompileDate[]="Date: "OUR__DATE__"\0""Date: "OUR__DATE__"\0""Date: "OUR__DATE__"\0""Дата: "OUR__DATE__;


//unsigned int D_idx,D_idx_col;
__noreturn void main(void)
{

	//== Выключаем ETM =========
	PINSEL10_bit.GPIO_TRACE = 0;
  //==========================

  //==== ДЛя питания дисплея ===============
  	DIR_DPWON = 1;//Питание индикатора
  	SET_DPWON;//Выключен
  	DIR_PON = 1;
	DIR_MON = 1;
	CLR_PON;
	CLR_MON;
  //==========================================
//bluetooth off
	DIR_BT_ON = 1;
	SET_BT_ON;
	DIR_BT_RES = 1;
	CLR_BT_RES;


	DIR_FR = 0;
	PINMODE3_bit.P1_26 = 2;

//
//gps off
	DIR_GPS_O = 1;
	DIR_GPS = 1;
	SET_GPS_O;	//take power from GPS
//
//motor off
	DIR_M_ON = 1;
	SET_M_ON;
//


	//======== Звук усилитель ==============
	DIR_SND_AU =1;
	CLR_SND_AU;

	__disable_interrupt();


	//stage 1: clock of CPU
	//======== Установки системы тактирования ===============
	SCS_bit.GPIOM = 1;	//fast PORT 0 and 1
	SCS_bit.OSCRANGE =0;      //Выбор диапозона кварца
	SCS_bit.OSCEN =1;         //Включение Кв. генератора
	CLKSRCSEL_bit.CLKSRC=1;  //Main OSC
	PCLKSEL0_bit.PCLK_SSP1=1;
	PCLKSEL0_bit.PCLK_PWM1=1;
	//==================================




	//stage 2: power control
	//must be called FIRST
	PowerControl_Init();

	PowerControl_watchDog_Init();	//init watch dog








	PowerControl_Bat2ADC_Init();	//init ADC
        //==============POWER==============


	//try to safe power
	USBCLKCFG_bit.USBSEL = 0xf; //we dont use USB so we can divide it by maximum value, try to safe power
	IRCTRIM_bit.IRCTRIM = 0xff; //we divide here IRC by maximum value

	//turn off turbo by default
	PowerControl_turboModeOFF();


	//stage 3: EMC
	//============== Конфигурирование EMC =============
	Display_EMC_Init();

	//stage 3/1: Clock control init
	//=================================================
	Clock_control_Init();

	//stage 4: turn on MAM
	//=============включение МАМ==============
	PowerControl_turnON_MAM();

	//stage 5: display init
	//================turn on display===========
	Display_init();
	Display_clearScreen();


	//=============LED================
	Display_LED_Init();



	//====================EEPROM==================
	EEPROM_Init();
	EEPROM_SSP0_Init();


	//stage 6: keyboard init
	//=================keyboard init==========
	KeyboardControl_turnON();
	KeyboardControl_keyboardData_Init();


	//stage 6.1 sound init
	//=================sound init==========
	SoundControl_Init();





	//==============InterProc Init===========
	InterProc_Init();
	//this timer is used for OS purpose too
	InterProc_Timer_init();	//!!!!must be inited before Modes_Init
	InterProc_UART1_Init();	//288000 bod
	InterProc_initCmdOrder();


	//================geiger=================
	Geiger_Init();
	Geiger_INT_Init();


	//==================Bluetooth Init====================
	Bluetooth_Init();
	Bluetooth_turnOFF();


	//==================USBRS Init====================
	USBRS_Init();


	//==================GPS Init====================
#ifndef GPS_BT_FREE
	GPS_Init();
	NMEAParser_Init();
#endif	//#ifndef GPS_BT_FREE

	//stage 7: interrupts init
	//================init interrupts=============
	Interrupts_Init();



	//stage 8: enalbe interrupts and after this must be main cycle
	//=================enable int
	__enable_interrupt();





//здесь и не ранее! выключаем подсистему звука
	SoundControl_turnOFF();
	SoundControl_turnON();



	//=====================================
	//once init all modes
	Modes_Init();

	//подготовка к сжатию спектра
	packspectrum_init();

	//подсчет MD5
	CRC32ROM(SETUPModeControl.lpzCalcCRC32);


	//====================== check for discharged


	//ident init
	identify_InitIdent();



	//===================FILESYSTEM============
	//!!!!!!!!!!! до этих функций чтение данных FS невозможно
	filesystem_Init();




	//================================================
	//check for hardreset mode
	if(KeyboardControl_testKeyDown() && KeyboardControl_testKeyUp())
	{
		SETUPMode_menu1_hardreset();
	}

	//============start flasher if pressed down right==============
	if(KeyboardControl_testKeyDown() && KeyboardControl_testKeyLeft())
	{//flashing second proc
		USBRS_turnON();
		PowerControl_startBootLoader();
	}





	//show startup screen at the beginning
	Display_startup_display_start();

	PowerControl_emergencyCheckBattery();

	Display_clearScreen();
	Display_setTextColor(BROWN);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-300,X_SCREEN_SIZE,34);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt32x32);	//set current font
	Display_outputText("INFO");
	Display_drawHLine(0,MODE_USER_TOP-2,X_SCREEN_MAX,BROWN);

	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,3);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	SETUPMode_showVersion(255);


	Display_turnOFF_LEDs();


	//move sound down to start later
	SoundControl_BeepSeq(beepSeq_ON);

	PowerControl_kickWatchDog();
	filesystem_check_and_restore();


	//отобразить экран прогрева, версии
	Display_warmup_display_start();
	//!!!!!!!!!!! до этих функций чтение данных FS невозможно
	PowerControl_kickWatchDog();

	//detect memory low
	filesystem_detect_memorylow();


	SoundControl_PlayVibro(100);





	//=================create log file================

	LOGMode_createLog();
	LOGMode_insertEventByLang("Power ON\0""Power ON\0""Power ON\0""Включение");
	//===================================

//	PowerControl_emergencyCheckBattery();
	filesystem_check_ini_files();

	//must be run after read calibration from files
	identify_InitSigma1();


	//========start interfaces and devices by state	in ini
	//make some starts of gps
#ifndef GPS_BT_FREE
	GPS_startGPSbyState();
	Bluetooth_startbyState();
#endif	//#ifndef GPS_BT_FREE

	//================adjust doserate window table=============
	Spectrum_setupDoseWindowTable();



	//============= startup adjusting of second proc==============
	Spectrum_startAcq();
	//============================================================


	//====================================
	//first read these

	InterProc_readTemperature();
	InterProc_readDiagnostic();
	InterProc_setMeasurementMode();

	//read bkg values
	InterProc_getBkgCPS();

	InterProc_resetAveraging();
	InterProc_readMeasurementRegs();
	InterProc_readStatus();

	InterProc_readDTCOEF();


	//set mode by default
	if(modeControl.bSysDefault
	   || modeControl.bNoSysSettings
		|| modeControl.bNoSystemBak
		|| modeControl.bNoEnergyCal
		|| modeControl.bNoSigmaCal
		|| modeControl.bNoLibrary
		|| modeControl.bIdentDefault
		|| modeControl.bNoEnergySigmaSpz
			)
		Modes_setActiveMode(&modes_INFOMode);
	else
		Modes_setActiveMode(&modes_STABMode);


	//здесь надо очистить буфер клавы чтобы случ нажатия не обработать
	KeyboardControl_restoreKeyboard();


	///////////////////////////////////////////////
	//allow USB communications here
	USBRS_turnON();



	//============================
	if(filesystem.bMemoryLow)
	{//memory low, propose to delete files
		SETUPMode_clear_memory();
	}


	while(1)
	{


		//test second proc fault
		if(!PIN_AN_ERR)
		{//second proc exception !!!!!!!!!!!! a seriously fault
			exception(__FILE__,__FUNCTION__,__LINE__,"Processor 2 failed!");
		}

		if(powerControl.bControlBat)
		{//battery control
			PowerControl_controlBatStatus();
			powerControl.bControlBat = FALSE;
		}



		if(clockData.mayUpdateDateTimeView)
		{//once a second

			PowerControl_kickWatchDog();
			///////////////


			if(powerControl.dwBatteryAlarmCnt>BAT_LOW_TIME_TO_OFF)
			{//turn off device by bat low status
				PowerControl_turnOFF_device("Battery low!\0""Battery low!\0""Battery low!\0""Батареи разряжены!");
			}




			//!!!!!!!!must be realized check of bluetooth, usb, GPS data transmition, not only key pushing
			//goto power down mode after 120 seconds no key pushing
			++powerControl.dwIdleTime;
			if(powerControl.dwIdleTime>=powerControl.dwPowerDownDeadTime)	//add second to idle counter, it will be reset if some key is pressed
			{//now only if no key is pressed
				if(Modes_OnPowerDown())
				{
					PowerControl_gotoPowerDownMode();
					powerControl.dwIdleTime = 0;	//reset idle time counter after power down
				}else
				{
					if(!SPRDModeControl.bGMMode && !SPRDModeControl.bNaIMode && display.bLCDON  && modeControl.pMode != &modes_STABMode)
					{
						Display_turnOFF();
						powerControl.dwIdleTime = 0;	//reset idle time counter after power down
					}
				}
			}



			//control status of GPS
#ifndef GPS_BT_FREE
			GPS_control();

			//control status symbol of GPS
			GPS_sym_control();
			//test GPS connection and change the speed of GPS module if somthing wrong
			if(GPSControl.bGPS_ON)
				GPS_testGPSConnection();

			//control status symbol of bluetoth
			Bluetooth_sym_control();
#endif	//#ifndef GPS_BT_FREE

			//control status symbol of USB
			USBRS_sym_control();


			//copy geiger data to use in min cycle
			Geiger_copyDataToSafePlace();
			//control geiger safety alarm to switch to SPRD mode automatically
			Geiger_control();

			//periodically retrieve status, temperature, diagnostic info from second proc
			if((clockData.dateTime.second%5)==0)
			{//every 20 seconds read temperature status and diag

				InterProc_readStatus();
				InterProc_readTemperature();
				InterProc_readDiagnostic();
				filesystem_detect_memorylow();

				if(powerControl.bBatteryAlarm)
					SoundControl_BeepSeq(beepSeq_LOWBAT);
			}


			//show status line and reset clockData.mayUpdateDateTimeView to 0
			Display_showStatusLine();
		}




		Modes_OnIdle();

		//Sys Timer processing
		if(modeControl.bTimerON &&
		   modeControl.dwTimerReach)
		{
			modeControl.dwTimerReach=0;
			Modes_OnTimer();
		}


#ifndef GPS_BT_FREE
		//control bluetooth data exchange
		Bluetooth_control();
#endif	//#ifndef GPS_BT_FREE

		//control USBRS data exchange
		USBRS_control();

		//execute execute.sys
		//this is used to execute some commands from PC in SPRD
		//time adjusting, or other
		if(filesystem.hFileExecuteSys!=0)
		{
			main_execute_sys(filesystem.hFileExecuteSys);
			filesystem_delete_file(filesystem.hFileExecuteSys);
			filesystem.hFileExecuteSys = 0;
			powerControl.dwIdleTime = 0;	//reset idle time counter after execution
		}

		////////////////////////////////////////////////
		//here we must process the order of cmd to proc2
		InterProc_InterProcControl();
		//!!!!!!! InterProc_InterProcControl must be before KeyboardControl_keyboardControl to make send data before idle mode
		////////////////
		//возможна потенциальная проблема если войти в idle после приема данных по рс но не обработав их
		//и тогда не будет прерываний чтобы выйти из idle кроме часов
		//keys processing and go to idle if no work yet
		if(KeyboardControl_keyboardControl())
		{//some key was processed
			//after key processing
			powerControl.dwIdleTime = 0;	//reset idle time counter after key processing
		}
	}
}


//execute script file
void main_execute_sys(HFILE hfile)
{
	//check for datetime adjusting
	main_execute__Clock_setDateTime(hfile);
	//click key Modes_OnDown
	main_execute__Modes_OnDown( hfile);
	//click key Modes_OnUp
	main_execute__Modes_OnUp( hfile);
	//click key Modes_OnLeft
	main_execute__Modes_OnLeft( hfile);
	//click key Modes_OnRight
	main_execute__Modes_OnRight( hfile);
	//check for serial number update
	main_execute__System_serialAndDate(hfile);
	//get screen
	main_execute__Display_getScreen(hfile);
	//switch to bridge mode
	main_execute__System_setBridgeMode(hfile);
	//start burn sound

	//turn off device
	main_execute__Modes_OnTurnOFF( hfile);

}



//get screen in file
void main_execute__Display_getScreen(HFILE hfile)
{
	int retlen;
	int order;
	retlen = filesystem_ini_get_int(hfile, "Display_getScreen", "order", &order);
	if(retlen!=S_OK)return;
	Display_getScreen();
}

//setup serial number and date of manufacturing
void main_execute__System_serialAndDate(HFILE hfile)
{
	int retlen;
	//16/01/2014 убрано обнуление серийного и даты, почему оно тут было?:???????????
	int serial=0; int month=0; int year=0;
	retlen = filesystem_ini_get_int(hfile, "System_setSerialAndDate", "serial", &serial);
	if(retlen!=S_OK)return;
	retlen = filesystem_ini_get_int(hfile, "System_setSerialAndDate", "month", &month);
	if(retlen!=S_OK)return;
	retlen = filesystem_ini_get_int(hfile, "System_setSerialAndDate", "year", &year);
	if(retlen!=S_OK)return;

	SETUPModeControl.Serial = serial;
	SETUPModeControl.ManufacturedMonth = month;
	SETUPModeControl.ManufacturedYear = year;

	EEPROM_UpdateEssentialDataInEeprom();
}



//check for datetime adjusting
void main_execute__Clock_setDateTime(HFILE hfile)
{
	int retlen;
	//void Clock_setDateTime(int sec, int min, int hour, int dom, int dow, int doy, int month, int year);
	int sec; int min; int hour; int dom; int dow; int doy; int month; int year;
	retlen = filesystem_ini_get_int(hfile, "Clock_setDateTime", "sec", &sec);
	if(retlen!=S_OK)return;
	retlen = filesystem_ini_get_int(hfile, "Clock_setDateTime", "min", &min);
	if(retlen!=S_OK)return;
	retlen = filesystem_ini_get_int(hfile, "Clock_setDateTime", "hour", &hour);
	if(retlen!=S_OK)return;
	retlen = filesystem_ini_get_int(hfile, "Clock_setDateTime", "dom", &dom);
	if(retlen!=S_OK)return;
	retlen = filesystem_ini_get_int(hfile, "Clock_setDateTime", "dow", &dow);
	if(retlen!=S_OK)return;
	retlen = filesystem_ini_get_int(hfile, "Clock_setDateTime", "doy", &doy);
	if(retlen!=S_OK)return;
	retlen = filesystem_ini_get_int(hfile, "Clock_setDateTime", "month", &month);
	if(retlen!=S_OK)return;
	retlen = filesystem_ini_get_int(hfile, "Clock_setDateTime", "year", &year);
	if(retlen!=S_OK)return;
	Clock_setDateTime(sec, min, hour, dom, dow, doy, month, year);
}

//turn off the device
void main_execute__Modes_OnTurnOFF(HFILE hfile)
{
	int retlen;
	int order;
	retlen = filesystem_ini_get_int(hfile, "Modes_OnTurnOFF", "order", &order);
	if(retlen!=S_OK)return;
	PowerControl_turnOFF_device("\0""\0""\0""");
}



//click key Modes_OnDown
void main_execute__Modes_OnDown(HFILE hfile)
{
	int retlen;
	int order;
	retlen = filesystem_ini_get_int(hfile, "Modes_OnDown", "order", &order);
	if(retlen!=S_OK)return;
	Modes_OnDown();
}

//click key Modes_OnUp
void main_execute__Modes_OnUp(HFILE hfile)
{
	int retlen;
	int order;
	retlen = filesystem_ini_get_int(hfile, "Modes_OnUp", "order", &order);
	if(retlen!=S_OK)return;
	Modes_OnUp();
}

//click key Modes_OnLeft
void main_execute__Modes_OnLeft(HFILE hfile)
{
	int retlen;
	int order;
	retlen = filesystem_ini_get_int(hfile, "Modes_OnLeft", "order", &order);
	if(retlen!=S_OK)return;
	Modes_OnLeft();
}

//click key Modes_OnRight
void main_execute__Modes_OnRight(HFILE hfile)
{
	int retlen;
	int order;
	retlen = filesystem_ini_get_int(hfile, "Modes_OnRight", "order", &order);
	if(retlen!=S_OK)return;
	Modes_OnRight();
}

//bridge mode usb to bluetooth
void main_execute__System_setBridgeMode(HFILE hfile)
{
	int retlen;
	int order;
	retlen = filesystem_ini_get_int(hfile, "System_setBridgeMode", "order", &order);
	if(retlen!=S_OK)return;

	int i = 100;
#ifndef GPS_BT_FREE
	Bluetooth_turnON();
#endif	//#ifndef GPS_BT_FREE
	while(!USBRSControl.uart.bTrmReady && --i)
	{
		PowerControl_sleep(10);
	}
	SAFE_DECLARE;
	DISABLE_VIC;
	USBRS_Init();
	USBRS_InitInBridgeMode();
	ENABLE_VIC;
}
