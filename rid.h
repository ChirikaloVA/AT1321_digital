#ifndef _RID_H
#define _RID_H

#include "types.h"
#include "modes.h"





struct tagRIDControl
{	
	BOOL bExpertMode;
};

extern struct tagRIDControl RIDControl;

extern const struct tagMode modes_RID_Mode;

extern const struct tagMenu rid_menu;




void RID_Init(void);






const char* RID_LeftOnUpdate(void);//"marker\rleft\0""маркер\rвлево",
const char* RID_RightOnUpdate(void);//"marker\rright\0""маркер\rвправо",


//режимы работы со спектром
//отображаемое слово и есть текущий режим
const char* RID_UpOnUpdate(void);

const char* RID_DownOnUpdate(void);//"menu\0""меню",


BOOL RID_OnActivate(void);
BOOL RID_OnLeft(void);
BOOL RID_OnRight(void);
BOOL RID_OnUp(void);
BOOL RID_OnDown(void);

BOOL RID_OnShow(void);
BOOL RID_OnPowerDown(void);

void RID_done(BOOL bOK);

const char* RID_menu1_TC_onUpdate(void);
BOOL RID_menu1_TC(void);


const char* RID_NameOnUpdate(void);//"RID adv\0""РИД расш",


BOOL RID_OnIdle(void);
BOOL RID_OnWakeUp(void);
BOOL RID_OnExit(void);
BOOL RID_OnTimer(void);

BOOL RID_menu1_exitExpertMode(void);
const char* RID_menu1_exitExpertMode_onUpdate(void);

#endif	//ifndef _RID_H
