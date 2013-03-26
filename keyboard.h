#ifndef _KEYBOARD_H
#define _KEYBOARD_H

//keyboard control
//31/10/2008 Bystrov

#include "types.h"

//in ms
#define KEYBOARD_TIMER_VAL 50

#define KEYBOARD_FIRST_PUSH_TIME  1000
#define KEYBOARD_OTHER_PUSH_TIME  100


//======== Кнопки ================
#define DIR_KEY1 FIO2DIR_bit.P2_0
#define SET_KEY1 MY_FIO2SET(B_0)
#define CLR_KEY1 MY_FIO2CLR(B_0)
#define PIN_KEY1 FIO2PIN_bit.P2_0

#define DIR_KEY2 FIO2DIR_bit.P2_1
#define SET_KEY2 MY_FIO2SET(B_1)
#define CLR_KEY2 MY_FIO2CLR(B_1)
#define PIN_KEY2 FIO2PIN_bit.P2_1

#define DIR_KEY3 FIO2DIR_bit.P2_2
#define SET_KEY3 MY_FIO2SET(B_2)
#define CLR_KEY3 MY_FIO2CLR(B_2)
#define PIN_KEY3 FIO2PIN_bit.P2_2

#define DIR_KEY4 FIO2DIR_bit.P2_3
#define SET_KEY4 MY_FIO2SET(B_3)
#define CLR_KEY4 MY_FIO2CLR(B_3)
#define PIN_KEY4 FIO2PIN_bit.P2_3

struct tagKeyboardData
{

  struct
  {
    BOOL bPressed; //=1 then key1 is pressed, handler must clear it
    DWORD pressedTime; //in timer cycles
    BOOL bProcessed; //=1 if click is processed, =0 if not
  } keyUp;
  struct
  {
    BOOL bPressed; //=1 then key1 is pressed, handler must clear it
    DWORD pressedTime; //in timer cycles
    BOOL bProcessed; //=1 if click is processed, =0 if not
  } keyRight;
  struct
  {
    BOOL bPressed; //=1 then key1 is pressed, handler must clear it
    DWORD pressedTime; //in timer cycles
    BOOL bProcessed; //=1 if click is processed, =0 if not
  } keyLeft;
  struct
  {
    BOOL bPressed; //=1 then key1 is pressed, handler must clear it
    DWORD pressedTime; //in timer cycles
    BOOL bProcessed; //=1 if click is processed, =0 if not
  } keyDown;

  //BOOL bStatusUpdated;

};

extern struct tagKeyboardData volatile keyboard;
extern struct tagKeyboardData volatile keyboard_safe;


void KeyboardControl_copyKeyboardDataToSafePlace(void);

void KeyboardControl_turnON(void);
void KeyboardControl_turnOFF(void);


BOOL KeyboardControl_anyKeyPressed_hardware_intcall(void);

BOOL KeyboardControl_testKeyUp(void);
BOOL KeyboardControl_testKeyRight(void);
BOOL KeyboardControl_testKeyLeft(void);
BOOL KeyboardControl_testKeyDown(void);


BOOL KeyboardControl_anyKeyPressed_safe(void);

BOOL KeyboardControl_keyUpPressed_safe(void);
BOOL KeyboardControl_keyDownPressed_safe(void);
BOOL KeyboardControl_keyLeftPressed_safe(void);
BOOL KeyboardControl_keyRightPressed_safe(void);

void KeyboardControl_refresh_keyboard_status_intcall(BOOL resetProcessedAnyWay, DWORD milisec_addon);
__arm void _INT_Keyboard_Timer0(void);



void KeyboardControl_keyboardData_Init(void);

__arm void _INT_Keyboard(void);


BOOL KeyboardControl_keyUpPressedOnly_safe(void);
BOOL KeyboardControl_keyDownPressedOnly_safe(void);
BOOL KeyboardControl_keyLeftPressedOnly_safe(void);
BOOL KeyboardControl_keyRightPressedOnly_safe(void);

BOOL KeyboardControl_keyboardControl(void);

void KeyboardControl_restoreKeyboard(void);


#endif	//#ifndef _KEYBOARD_H
