//main.h

#ifndef _MAIN_H
#define _MAIN_H



#include <iolpc2388.h>
#include "types.h"


void main_execute_sys(HFILE hfile);
void main_execute__Clock_setDateTime(HFILE hfile);

//click key Modes_OnDown
void main_execute__Modes_OnDown(HFILE hfile);
//click key Modes_OnUp
void main_execute__Modes_OnUp(HFILE hfile);
//click key Modes_OnLeft
void main_execute__Modes_OnLeft(HFILE hfile);
//click key Modes_OnRight
void main_execute__Modes_OnRight(HFILE hfile);

void main_execute__System_serialAndDate(HFILE hfile);
void main_execute__Display_getScreen(HFILE hfile);

extern const char txtVersion[];
extern const char txtCompileDate[];

//void main_execute__startBurn(HFILE hfile);
//stop burn sound
//void main_execute__stopBurn(HFILE hfile);

void main_execute__System_setBridgeMode(HFILE hfile);

void main_execute__Modes_OnTurnOFF(HFILE hfile);

#endif  //#ifndef _MAIN_H
