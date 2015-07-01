//keyboard control
//31/10/2008 Bystrov

//#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>

#include "keyboard.h"
#include "types.h"
#include "syncObj.h"
#include "sound.h"
#include "powerControl.h"
#include "modes.h"
#include "interproc.h"
#include "display.h"




struct tagKeyboardData volatile keyboard;
struct tagKeyboardData volatile keyboard_safe;




void KeyboardControl_copyKeyboardDataToSafePlace(void)
{
	SyncObj_memcpy_safe((BYTE*)&keyboard_safe, (const BYTE*)&keyboard, sizeof(struct tagKeyboardData));
}


BOOL KeyboardControl_testKeyDown(void)
{
  return (PIN_KEY1 == 0);
}

BOOL KeyboardControl_testKeyLeft(void)
{
  return (PIN_KEY4 == 0);
}

BOOL KeyboardControl_testKeyRight(void)
{
  return (PIN_KEY2 == 0);
}

BOOL KeyboardControl_testKeyUp(void)
{
  return (PIN_KEY3 == 0);
}



//interrupt from timer0
__arm void _INT_Keyboard_Timer0(void)
{
	__ir_bits stru = {0};
	if(T0IR_bit.MR0INT)
	{//long term holding after first click
	
		//check if button still pressed then enable int for holding
		//if no then disable timer0 and change polarity for INT3.
		if(!KeyboardControl_anyKeyPressed_hardware_intcall())
		{//no key is pressed
			KeyboardControl_turnOFF();
			//		IO1INTCLR_bit.P1_12 = 1;  //clear p2_12 interrupt status
			EXTINT_bit.EINT2 = 1; //clear INT
			VICINTENABLE_bit.EINT2 = 1;	//enable INT3
		}else
		{
			KeyboardControl_refresh_keyboard_status_intcall(1/*reset processed if key is holding*/, KEYBOARD_FIRST_PUSH_TIME);
			T0MCR_bit.MR0I = 0; //disable interrupt for first clicking
			T0MCR_bit.MR1I = 1; //enable interrupt for holding
			T0MCR_bit.MR1R = 1; //reset counter after reach comparator
			T0TCR_bit.CR = 1;
			T0TCR_bit.CR = 0;
		}
		stru.MR0INT = 1;
		
	}else
	if(T0IR_bit.MR1INT)
	{//short term for next holding button
	
		if(!KeyboardControl_anyKeyPressed_hardware_intcall())
		{//no key is pressed
			KeyboardControl_turnOFF();
			//		IO1INTCLR_bit.P1_12 = 1;  //clear p2_12 interrupt status
			EXTINT_bit.EINT2 = 1; //clear INT
			VICINTENABLE_bit.EINT2 = 1;	//enable INT3
		}else
		{
			KeyboardControl_refresh_keyboard_status_intcall(1/*reset processed if key is holding*/, KEYBOARD_OTHER_PUSH_TIME);
		}
	
		stru.MR1INT = 1;
	}else
	if(T0IR_bit.MR2INT)
	{//catch first click
	
		if(!KeyboardControl_anyKeyPressed_hardware_intcall())
		{//no key is pressed
			KeyboardControl_turnOFF();
			//		IO1INTCLR_bit.P1_12 = 1;  //clear p2_12 interrupt status
			EXTINT_bit.EINT2 = 1; //clear INT
			VICINTENABLE_bit.EINT2 = 1;	//enable INT3
		}else
		{
			KeyboardControl_refresh_keyboard_status_intcall(1/*reset processed if key is holding*/, 0);
			T0MCR_bit.MR0I = 1; //enable interrupt for first clicking
			T0MCR_bit.MR0R = 0; //reset counter after reach comparator
			T0MCR_bit.MR0S = 0; //reset counter after reach comparator
			T0MCR_bit.MR1I = 0; //disable interrupt for holding
			T0MCR_bit.MR1R = 0; //do not reset counter after reach comparator
			T0MCR_bit.MR1S = 0; //do not reset counter after reach comparator
			T0MCR_bit.MR2I = 0; //disable interrupt for holding
			T0MCR_bit.MR2R = 0; //do not reset counter after reach comparator
			T0MCR_bit.MR2S = 0; //do not reset counter after reach comparator
			T0TCR_bit.CR = 1;
			T0TCR_bit.CR = 0;
			
			//sample playing
			//!!!!!!!!		playSample((int)SI_ADJECT, 0);
			soundControl.bPlayClick = TRUE;
		
		}
		stru.MR2INT = 1;
	}else
	if(T0IR_bit.MR3INT)
	{
		//clear TIMER0 interrupt status
		T0MR3 = T0TC+1;  //50 ms for process catch first click
		if(!KeyboardControl_anyKeyPressed_hardware_intcall())
		{//no key is pressed
			KeyboardControl_turnOFF();
			//		IO1INTCLR_bit.P1_12 = 1;  //clear p2_12 interrupt status
			EXTINT_bit.EINT2 = 1; //clear INT
			VICINTENABLE_bit.EINT2 = 1;	//enable INT3
		}
		stru.MR3INT = 1;
	}
	//clear TIMER0 interrupt status
	T0IR = *(ULONG*)&stru;
}




void KeyboardControl_turnOFF(void)
{
	T0TCR_bit.CE = 0;
//	PCONP_bit.PCTIM0 = 1;	//give power
}


//restore default state, as no key is pressed
void KeyboardControl_restoreKeyboard(void)
{
	//снимем обработку нажатой кнопки, чтобы пропустить первое нажатие после выход из повердауна
	KeyboardControl_turnOFF();
	PowerControl_sleep(100);
	SAFE_DECLARE; DISABLE_VIC; 
	keyboard.keyDown.bProcessed=1;//set key as processed
	keyboard.keyUp.bProcessed=1;//set key as processed
	keyboard.keyLeft.bProcessed=1;//set key as processed
	keyboard.keyRight.bProcessed=1;//set key as processed
	ENABLE_VIC;
	PowerControl_sleep(100);
	VICINTENABLE_bit.EINT2 = 1;	//enable INT2
}





//keyboard interrupts, called from supervisor mode!!!
//run under high level! so we need disable int2 after processing
__arm void _INT_Keyboard(void)
{//resuming after idle and powerdown mode.

	if(KeyboardControl_anyKeyPressed_hardware_intcall())
	{
		KeyboardControl_turnON();
		T0TCR_bit.CE = 1;
		VICINTENCLEAR = 0x10000;//disable INT2
		powerControl.bAwakedByKeyboard = 1;	//awaked by key pushing
	}
	//clear INT3 interrupt status
//	IO1INTCLR_bit.P1_12 = 1;  //clear p2_12 interrupt status
	EXTINT_bit.EINT2 = 1;  //clear INT
}

//test for any key pressed by hardware GPIO
BOOL KeyboardControl_anyKeyPressed_hardware_intcall(void)
{
  BOOL retVal=0;
  retVal |= KeyboardControl_testKeyDown()|KeyboardControl_testKeyLeft()|KeyboardControl_testKeyRight()|KeyboardControl_testKeyUp();
  return retVal;
}

//test for any key pressed (only by keyboard data not by hardware GPIO)
BOOL KeyboardControl_anyKeyPressed_safe(void)
{
  BOOL retVal=0;
  retVal = (keyboard_safe.keyUp.bPressed & !keyboard_safe.keyUp.bProcessed)|
    (keyboard_safe.keyDown.bPressed & !keyboard_safe.keyDown.bProcessed)|
    (keyboard_safe.keyLeft.bPressed & !keyboard_safe.keyLeft.bProcessed)|
    (keyboard_safe.keyRight.bPressed & !keyboard_safe.keyRight.bProcessed);
  return retVal;
}



//init keyboard data
void KeyboardControl_keyboardData_Init(void)
{
  keyboard.keyUp.bPressed = 0;
  keyboard.keyUp.pressedTime = 0;
  keyboard.keyUp.bProcessed = 0;
  keyboard.keyDown.bPressed = 0;
  keyboard.keyDown.pressedTime = 0;
  keyboard.keyDown.bProcessed = 0;
  keyboard.keyLeft.bPressed = 0;
  keyboard.keyLeft.pressedTime = 0;
  keyboard.keyLeft.bProcessed = 0;
  keyboard.keyRight.bPressed = 0;
  keyboard.keyRight.pressedTime = 0;
  keyboard.keyRight.bProcessed = 0;
	//keyboard.bStatusUpdated = FALSE;
}




//first init keyboard control
void KeyboardControl_turnON(void)
{
	PCONP_bit.PCTIM0 = 1;	//give power on
	
	//disable timer0
	T0TCR_bit.CE = 0;
	//adjust predivider
	T0PR = HW_FREQ/1000*KEYBOARD_TIMER_VAL-1;  //при частоте шины железа 9216000 √ц это значение предделител€ даст нам 50мс циклы инкремента таймера
	T0PC = 0;
	//adjust comparators 0 and 1
	T0MR0 = KEYBOARD_FIRST_PUSH_TIME/KEYBOARD_TIMER_VAL-1;  //1000 ms for process first click
	T0MR1 = KEYBOARD_OTHER_PUSH_TIME/KEYBOARD_TIMER_VAL-1;  //100 ms for process holding button
	T0MR2 = KEYBOARD_OTHER_PUSH_TIME/KEYBOARD_TIMER_VAL-1;  //100 ms for process catch first click


  //adjust action on comparators 0 and 1
	__mcr_bits t0mcr;
	t0mcr.MR0I = 0; //disable interrupt for first clicking
	t0mcr.MR0R = 0; //reset counter after reach comparator
	t0mcr.MR0S = 0; //reset counter after reach comparator
	t0mcr.MR1I = 0; //disable interrupt for holding
	t0mcr.MR1R = 0; //do not reset counter after reach comparator
	t0mcr.MR1S = 0; //do not reset counter after reach comparator
	t0mcr.MR2I = 1; //enable interrupt for catch first click
	t0mcr.MR2R = 0; //do not reset counter after reach comparator
	t0mcr.MR2S = 0; //do not reset counter after reach comparator
	t0mcr.MR3I = 1; //enable interrupt for catch first click
	t0mcr.MR3R = 0; //do not reset counter after reach comparator
	t0mcr.MR3S = 0; //do not reset counter after reach comparator
	T0MCR_bit = t0mcr;
	T0TCR_bit.CR = 1;
	T0TCR_bit.CR = 0;
	T0MR3 = T0TC+1;  //100 ms for process catch first click
}


//refresh all keys status
//if milisec_addon==0 then reset timers to 0 esle it is addon
void KeyboardControl_refresh_keyboard_status_intcall(BOOL resetProcessedAnyWay, DWORD milisec_addon)
{
  BOOL val;

  val = KeyboardControl_testKeyUp();
  if(val && (!keyboard.keyUp.bPressed || resetProcessedAnyWay))
    keyboard.keyUp.bProcessed = 0;  //reset processed flag if it is first click
  keyboard.keyUp.bPressed = val;
  if(val && milisec_addon)
     keyboard.keyUp.pressedTime+=milisec_addon;
  else
     keyboard.keyUp.pressedTime=milisec_addon;

  val = KeyboardControl_testKeyRight();
  if(val && (!keyboard.keyRight.bPressed || resetProcessedAnyWay))
    keyboard.keyRight.bProcessed = 0;  //reset processed flag if it is first click
  keyboard.keyRight.bPressed = val;
  if(val && milisec_addon)
     keyboard.keyRight.pressedTime+=milisec_addon;
  else
     keyboard.keyRight.pressedTime=milisec_addon;

  val = KeyboardControl_testKeyLeft();
  if(val && (!keyboard.keyLeft.bPressed || resetProcessedAnyWay))
    keyboard.keyLeft.bProcessed = 0;  //reset processed flag if it is first click
  keyboard.keyLeft.bPressed = val;
  if(val && milisec_addon)
     keyboard.keyLeft.pressedTime+=milisec_addon;
  else
     keyboard.keyLeft.pressedTime=milisec_addon;

  val = KeyboardControl_testKeyDown();
  if(val && (!keyboard.keyDown.bPressed || resetProcessedAnyWay))
    keyboard.keyDown.bProcessed = 0;  //reset processed flag if it is first click
  keyboard.keyDown.bPressed = val;
  if(val && milisec_addon)
     keyboard.keyDown.pressedTime+=milisec_addon;
  else
     keyboard.keyDown.pressedTime=milisec_addon;

 // keyboard.bStatusUpdated = TRUE;

}


//test key pressed and not processed
BOOL KeyboardControl_keyUpPressed_safe(void)
{
  return (SAFE_GET_DWORD(&keyboard_safe.keyUp.bPressed) & !SAFE_GET_DWORD(&keyboard_safe.keyUp.bProcessed));
}
BOOL KeyboardControl_keyDownPressed_safe(void)
{
  return (SAFE_GET_DWORD(&keyboard_safe.keyDown.bPressed) & !SAFE_GET_DWORD(&keyboard_safe.keyDown.bProcessed));
}
BOOL KeyboardControl_keyLeftPressed_safe(void)
{
  return (SAFE_GET_DWORD(&keyboard_safe.keyLeft.bPressed) & !SAFE_GET_DWORD(&keyboard_safe.keyLeft.bProcessed));
}
BOOL KeyboardControl_keyRightPressed_safe(void)
{
  return (SAFE_GET_DWORD(&keyboard_safe.keyRight.bPressed) & !SAFE_GET_DWORD(&keyboard_safe.keyRight.bProcessed));
}
BOOL KeyboardControl_keyUpPressedOnly_safe(void)
{
	return (KeyboardControl_keyUpPressed_safe() && !KeyboardControl_keyDownPressed_safe() && !KeyboardControl_keyLeftPressed_safe() && !KeyboardControl_keyRightPressed_safe());
}
BOOL KeyboardControl_keyDownPressedOnly_safe(void)
{
	return (!KeyboardControl_keyUpPressed_safe() && KeyboardControl_keyDownPressed_safe() && !KeyboardControl_keyLeftPressed_safe() && !KeyboardControl_keyRightPressed_safe());
}
BOOL KeyboardControl_keyLeftPressedOnly_safe(void)
{
	return (!KeyboardControl_keyUpPressed_safe() && !KeyboardControl_keyDownPressed_safe() && KeyboardControl_keyLeftPressed_safe() && !KeyboardControl_keyRightPressed_safe());
}
BOOL KeyboardControl_keyRightPressedOnly_safe(void)
{
	return (!KeyboardControl_keyUpPressed_safe() && !KeyboardControl_keyDownPressed_safe() && !KeyboardControl_keyLeftPressed_safe() && KeyboardControl_keyRightPressed_safe());
}


//ret 1 if some key was processed else 0
BOOL KeyboardControl_keyboardControl(void)
{
/*	
	//added 10/02/08 for better control keyboard
	if(!keyboard.bStatusUpdated)
	{//status is not changed
		PowerControl_gotoIdleMode();//we can go idle if no keys pressed
		return FALSE;
	}
	keyboard.bStatusUpdated = FALSE;
	//
	*/
	
	KeyboardControl_copyKeyboardDataToSafePlace();

	BOOL ret = 0;	//no key was processed by default

	if(!KeyboardControl_anyKeyPressed_safe())
	{//nothing pressed, no timer is reached and we can go to idle mode
	//!!!!!!!!!!! подводный камень!!!!!!!!!!!!
	//возможна ситуаци€ когда в основном цикле до перехода в idle но после проверки нажати€ кнопки происходит прерывание нажати€ кнопки,
	//и мы его не обрабатываем или обрабатываем но все равно переходим в idle при этом не обработав само нажатие
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		
		//!!!!!!!!!!!!
		//before idle mode we must check do we need send some cmd to proc2? if yes then NO IDLE
		if(!modeControl.dwTimerReach && //no softtimers reached
		   !interProcControl.bTimeOutReached)	//have to answer on request
			PowerControl_gotoIdleMode();//we can go idle if no keys pressed
		
	}else
	{//pressed any key, must be handled

		if(!display.bLCDON)
		{//first click when LCD OFF, pass it
			Display_turnON();
			//added from 10/02/2011
			modeControl.bMenuON = 0;	//turn off menu by default when exit LCD off mode, else void menu will be showed
			//
			Modes_updateMode();
			KeyboardControl_restoreKeyboard();
			return TRUE;
		}
		
		if(soundControl.bPlayClick)
		{
			soundControl.bPlayClick = FALSE;
			if(soundControl.bSound==SNDST_SOUND)
				SoundControl_Beep(40, SOUND_FREQ_1_SI*8);
//			sound_playSample(SND_CLICK);
		}
		
		//process power off and power down, also once click on button
		if(KeyboardControl_keyDownPressed_safe())
		{
			DWORD ptm = SAFE_GET_DWORD(&keyboard.keyDown.pressedTime);
			if(ptm>TIME_TO_ENTER_POWERDOWN_BY_KEY)
			{
#ifdef BNC	
				//turn off mode
					PowerControl_turnOFF_device("RELEASE THE KEY\0""RELEASE THE KEY\0""RELEASE THE KEY\0""ќ“ѕ”—“»“≈  Ќќѕ ”");
#else //#ifdef BNC	
				//!!!!!!!!!				playSample(SI_TURNOFF, 1);
				if(!SAFE_GET_DWORD(&keyboard.keyLeft.bPressed))
				{//power down mode
					PowerControl_gotoPowerDownMode();
				}else
				{//turn off mode
					PowerControl_turnOFF_device("RELEASE THE KEY\0""RELEASE THE KEY\0""RELEASE THE KEY\0""ќ“ѕ”—“»“≈  Ќќѕ ”");
				}
#endif	//#ifdef BNC	
			}else if(ptm==0)
			{//process once click here
				SAFE_SET(keyboard.keyDown.bProcessed, 1);//set key as processed
				Modes_OnDown();

				//!!!!!!!!!!!!!!!!
				//switch music
/*				if(++mus>=SAMPLES_NUMBER)mus = 0;
				Display_setTextColor(BLUE);	//set text color
				Display_setTextWin(0,280,X_SCREEN_SIZE,20);	//set text window
				Display_setTextXY(0,0);	//set start coords in window
				Display_setTextWrap(0);
				Display_setTextDoubleHeight(0);
				Display_setTextJustify(CENTER);
				Display_setCurrentFont(fnt16x16);	//set current font
				Display_clearTextWin(10);
				Display_outputText((char*)samples_names[mus]);*/
			}
		}else
		{//next keys will be processed only when down key is unpressed
		
			if(KeyboardControl_keyUpPressedOnly_safe())
			{
				SAFE_SET(keyboard.keyUp.bProcessed, 1); //set key as processed
				Modes_OnUp();
			}
			if(KeyboardControl_keyLeftPressedOnly_safe())
			{
				SAFE_SET(keyboard.keyLeft.bProcessed, 1);//set key as processed
				Modes_OnLeft();
			}
			if(KeyboardControl_keyRightPressedOnly_safe())
			{
				SAFE_SET(keyboard.keyRight.bProcessed, 1);//set key as processed
				Modes_OnRight();
			}
		}
		ret = 1;	//key was processed
	}
	return ret;
}
