//interrupts.c
#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <string.h>
#include "interrupts.h"
#include "keyboard.h"
#include "types.h"
#include "syncObj.h"
#include "main.h"
#include "display.h"
#include "clock.h"
#include "sound.h"
#include "powerControl.h"
#include "interProc.h"
#include "gps.h"
#include "bluetooth.h"
#include "USBRS.h"
#include "modes.h"
#include "geiger.h"
#include "eeprom.h"

///////////registers////////////////
DWORD _stackVal1, _stackVal2, _stackVal3, _stackVal4;
DWORD _reg_R13, _reg_R14, _reg_CPSR, _reg_SPSR;
DWORD 	_reg_R0, _reg_R1, _reg_R2, _reg_R3,_reg_R4, _reg_R5, _reg_R6, _reg_R7,_reg_R8, _reg_R9, _reg_R10, _reg_R11, _reg_R12;
///////////registers////////////////


//removed __nested
__irq  __nested __arm void irq_handler(void)
{
  void (*interrupt_function)();
  unsigned int vector;

  __disable_interrupt();

  vector = VICADDRESS; // Get interrupt vector.
  if(!vector)return;

  interrupt_function = (void(*)())vector;

 __enable_interrupt(); // Allow other IRQ interrupts
                        //to be serviced from this
                        //point.
  (*interrupt_function)();  // Execute the task associated
                        //with this interrupt.
   __disable_interrupt();
    VICADDRESS = 0; // Clear interrupt in VIC.
}


__arm void Interrupts_printAllRegs(void)
{
	Display_outputText("\rSPSR=0x");
	Display_printR(_reg_SPSR);
	Display_outputText("\rR14=0x");
	Display_printR(_reg_R14);
	Display_outputText("\rR13=0x");
	Display_printR(_reg_R13);
	Display_outputText("\rR12=0x");
	Display_printR(_reg_R12);
	Display_outputText("\rR11=0x");
	Display_printR(_reg_R11);
	Display_outputText("\rR10=0x");
	Display_printR(_reg_R10);
	Display_outputText("\rR9=0x");
	Display_printR(_reg_R9);
	Display_outputText("\rR8=0x");
	Display_printR(_reg_R8);
	Display_outputText("\rR7=0x");
	Display_printR(_reg_R7);
	Display_outputText("\rR6=0x");
	Display_printR(_reg_R6);
	Display_outputText("\rR5=0x");
	Display_printR(_reg_R5);
	Display_outputText("\rR4=0x");
	Display_printR(_reg_R4);
	Display_outputText("\rR3=0x");
	Display_printR(_reg_R3);
	Display_outputText("\rR2=0x");
	Display_printR(_reg_R2);
	Display_outputText("\rR1=0x");
	Display_printR(_reg_R1);
	Display_outputText("\rR0=0x");
	Display_printR(_reg_R0);
}

//wait until user read message and click any button but not power button
__arm void Interrupts_waitForRead(void)
{

	//turn on watch dog
#ifdef DEBUG
	WDMOD_bit.WDRESET = 1;	//must be =1 for RELEASE MODE
#endif	//#ifdef DEBUG

	Display_turnOFF_LEDs();
	
	for(long i=0x1;i<0x40;i++)
	{
		Display_startup_LED();
		for(long j=0;j<i*0x300;j++)
		{
			SET_DG_ON;
		}
		PowerControl_kickWatchDog();
	}

	Display_setCurrentFont(fnt16x16);	//set current font
	Display_setTextColor(RED);	//set text color
	Display_setTextSteps(1,1);//set steps
	Display_setTextWin(0,Y_SCREEN_MAX-16,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(LEFT);

	CLR_AN_ON;	//turn off second proc
//	char buf[40];
//	int timer = 30;
	do
	{
		Display_startup_LED();
		for(long j=0;j<0x60000;j++)
		{
			SET_DG_ON;
		}
		PowerControl_kickWatchDog();
		
	//	sprintf(buf,"Time to reset: %d",timer);
//		Display_clearTextWin(100);
	//	Display_setTextXY(0,0);	//set start coords in window
		//Display_outputText(buf);
		
	}while(!(KeyboardControl_testKeyLeft()|KeyboardControl_testKeyRight()|KeyboardControl_testKeyUp())
		   /*&& --timer*/);

/*	if(timer)
	{*/
		//по кнопке выкл
		//только выключение
		CLR_DG_ON; //Выключение питания
/*	}else
	{// автомат на перезапуск
		//perform reset
		while(1);
	}*/
}


__irq __arm void undef_handler(void)
{
	__disable_interrupt();
	Interrupts_get_Registers();
	SoundControl_StopVibro();
	SoundControl_StopBeep();
	Display_prepareEmergencyTextControl();
	Display_outputText("UNDEFINE INSTRUCTION");
	Interrupts_printAllRegs();
	Interrupts_waitForRead();
}

__swi __arm void swi_handler(void)
{
	__disable_interrupt();
	Interrupts_get_Registers();
	SoundControl_StopVibro();
	SoundControl_StopBeep();
	Display_prepareEmergencyTextControl();
	Display_outputText("PROGRAM EXCEPTION");
/*	int len = strlen((char*)_reg_R0);
	int i=len-1;
	for(;i>=0;i--)
	{
		if(*((char*)_reg_R0+i)=='\\')break;
	}
	Display_outputText((char*)_reg_R0+i+1);*/
	Display_outputText("\rFUNCTION:\r");
	Display_outputText((char*)_reg_R1);
	char buf[20];
	sprintf(buf, "\rLINE:\r%d", _reg_R2);
	Display_outputText((char*)buf);
	Display_outputText("\rDESCRIPTION:\r");
	Display_outputText((char*)_reg_R3);
	Display_outputText("\rSTACK:\r");
	sprintf(buf, "%x, ", _stackVal1);
	Display_outputText((char*)buf);
	sprintf(buf, "%x, ", _stackVal2);
	Display_outputText((char*)buf);
	sprintf(buf, "%x, ", _stackVal3);
	Display_outputText((char*)buf);
	sprintf(buf, "%x", _stackVal4);
	Display_outputText((char*)buf);
	Display_outputText("\rTRMBUF:\r");
	sprintf(buf, "%x, ", interProcControl.uart.trmBuff[0]);
	Display_outputText((char*)buf);
	sprintf(buf, "%x, ", interProcControl.uart.trmBuff[1]);
	Display_outputText((char*)buf);
	sprintf(buf, "%x, ", interProcControl.uart.trmBuff[2]);
	Display_outputText((char*)buf);
	sprintf(buf, "%x, ", interProcControl.uart.trmBuff[3]);
	Display_outputText((char*)buf);
	sprintf(buf, "%x, ", interProcControl.uart.trmBuff[4]);
	Display_outputText((char*)buf);
	sprintf(buf, "%x", interProcControl.uart.trmBuff[5]);
	Display_outputText((char*)buf);
	Display_outputText("\rRCVBUF:\r");
	sprintf(buf, "%x, ", interProcControl.uart.rcvBuff[0]);
	Display_outputText((char*)buf);
	sprintf(buf, "%x, ", interProcControl.uart.rcvBuff[1]);
	Display_outputText((char*)buf);
	sprintf(buf, "%x, ", interProcControl.uart.rcvBuff[2]);
	Display_outputText((char*)buf);
	sprintf(buf, "%x, ", interProcControl.uart.rcvBuff[3]);
	Display_outputText((char*)buf);
	sprintf(buf, "%x, ", interProcControl.uart.rcvBuff[4]);
	Display_outputText((char*)buf);
	sprintf(buf, "%x", interProcControl.uart.rcvBuff[5]);
	Display_outputText((char*)buf);
	Interrupts_waitForRead();
}

__irq __nested __arm void fiq_handler(void)
{
	__disable_interrupt();
	Interrupts_get_Registers();
	SoundControl_StopVibro();
	SoundControl_StopBeep();
	Display_prepareEmergencyTextControl();
	Display_outputText("FIQ INTERRUPT");
	Interrupts_printAllRegs();
	Interrupts_waitForRead();
}


__irq __arm void prefetch_handler(void)
{
	__disable_interrupt();
	Interrupts_get_Registers();
	SoundControl_StopVibro();
	SoundControl_StopBeep();
	Display_prepareEmergencyTextControl();
	Display_outputText("PREFETCH ABORT");
	Interrupts_printAllRegs();
	Interrupts_waitForRead();
}

__irq __arm void data_handler(void)
{
	__disable_interrupt();
	Interrupts_get_Registers();
	SoundControl_StopVibro();
	SoundControl_StopBeep();
	Display_prepareEmergencyTextControl();
	Display_outputText("DATA ABORT");
	Interrupts_printAllRegs();
	Interrupts_waitForRead();
}


void Interrupts_Init(void)
{
  //====================================================
  //настройка прерываний VIC

  //common adjusting
  VICINTSELECT = 0;
  VICINTENCLEAR = 0xFFFFFFFF;     // reset bits of all interrupts
  VICPROTECTION = 0;              // Accesss VIC in USR | PROTECT
  VICADDRESS = 0;                // Clear interrupt

  //vector 17 is INT3
  PINSEL4_bit.P2_12 = 0x1;	//adjust INT2 keyboard
  EXTMODE_bit.EXTMODE2 = 0; //by level
  EXTPOLAR_bit.EXTPOLAR2 = 1; //on high
  EXTINT_bit.EINT2 = 1;


  PINSEL4_bit.P2_11 = 0x1;	//adjust INT1 wakeup from second proc
  EXTMODE_bit.EXTMODE1 = 1; //by front
  EXTPOLAR_bit.EXTPOLAR1 = 1; //on rising edge
  EXTINT_bit.EINT1 = 1;


  //INT2 keyboard, awaking from power down
  VICVECTADDR16 = (unsigned int)&_INT_Keyboard;
  VICVECTPRIORITY16_bit.PRIORITY = 5;  //lowest must be the same as for timer0 to avoid interrupt each other
  VICINTENABLE_bit.EINT2 = 1; //enable INT2

  //INT3 count geiger and awaking from power down
  VICVECTADDR17 = (unsigned int)&_INT_Geiger;
  VICVECTPRIORITY17_bit.PRIORITY = 10;  //priority must be lower then clock
  VICINTENABLE_bit.EINT3 = 1; //enable INT3

  //vector 4 timer0
  VICVECTADDR4 = (unsigned int)&_INT_Keyboard_Timer0;
  VICVECTPRIORITY4_bit.PRIORITY = 5;  //lowest must be the same as for INT2 to avoid interrupt each other
  VICINTENABLE_bit.TIMER0 = 1; //enable TIMER0 interrupts



  //vector 5 timer1
  VICVECTADDR5 = (unsigned int)&_INT_SoundTimer1;
  VICVECTPRIORITY5_bit.PRIORITY = 0;  //
  VICINTENABLE_bit.TIMER1 = 1; //enable TIMER1 interrupts

  //vector 26 timer2
  VICVECTADDR26 = (unsigned int)&_INT_Timer2_InterProc;
  VICVECTPRIORITY26_bit.PRIORITY = 3;  //must be eq or higher then _INT_WAKEUP_PowerControl
  VICINTENABLE_bit.TIMER2 = 1; //enable TIMER2 interrupts

  //vector 13 clock RTC, awaking from power down
  VICVECTADDR13 = (unsigned int)&_INT_Clock;
  VICVECTPRIORITY13_bit.PRIORITY = 2;
  VICINTENABLE_bit.RTC = 1; //enable clock interrupts

  //vector 7 UART1
  VICVECTADDR7 = (unsigned int)&_INT_UART1_InterProc;
  VICVECTPRIORITY7_bit.PRIORITY = 1;
  VICINTENABLE_bit.UART1 = 1; //enable UART1 interrupts

  //vector 28 UART2
#ifndef GPS_BT_FREE	
  VICVECTADDR28 = (unsigned int)&_INT_UART2_GPS;
  VICVECTPRIORITY28_bit.PRIORITY = 7;
  VICINTENABLE_bit.UART2 = 1;

  //vector 29 UART3
  VICVECTADDR29 = (unsigned int)&_INT_UART3_Bluetooth;
  VICVECTPRIORITY29_bit.PRIORITY = 9;
  VICINTENABLE_bit.UART3 = 1;
#endif	//#ifndef GPS_BT_FREE	

  //vector 6 UART0
  VICVECTADDR6 = (unsigned int)&_INT_UART0_USBRS;
  VICVECTPRIORITY6_bit.PRIORITY = 8;
  VICINTENABLE_bit.UART0 = 1;

  //vector 18 ADC
  VICVECTADDR18 = (unsigned int)&_INT_ADC_PowerControl;
  VICVECTPRIORITY18_bit.PRIORITY = 4;
  VICINTENABLE_bit.AD0 = 1;

  //vector 15 INT1 wakeup from second proc INTA
  VICVECTADDR15 = (unsigned int)&_INT_WAKEUP_PowerControl;
  VICVECTPRIORITY15_bit.PRIORITY = 6;  //priority must be lower then clock and eq or lower then sound
  VICINTENABLE_bit.EINT1 = 1; //enable INT3


  //====================================================

}

