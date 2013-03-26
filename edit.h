//edit.h

#ifndef _EDIT_H
#define _EDIT_H


#include <iolpc2388.h>
#include "types.h"

#define EDIT_BUF_LEN 30


struct tagEditMode
{
	int iCursor;	//cursor position in symbols
	char edit_buf[EDIT_BUF_LEN];	//editor buffer, will be auto reset anyway before edit
	char prevText[EDIT_BUF_LEN];	//prev text, for number edit it will atuo filled
	const char* pHeaderText;	//name of the edited value (3 lines)
	const char* pDimensionText;	//name of the dimension (1 line)
	char rangeText[50];	//range text, auto fill
	const char* pValueMask;	//mask to show value
	float fValue;
	float fMinValue;
	float fMaxValue;	//depend on it calc number of edited symbols for int
	int iSymbolNumber;	//set allowed number of symbol, for int it is auto calc, for float it is 9, must be set for text
	BOOL bFloat;	//if true then allows to enter DOT and float edit
	BOOL bDotPlaced;	//if true then dot is already placed
	void(*pRetFunction)(BOOL bOK);	//ret function after edit, it must switch to neccessary mode
	BOOL bText;	//if true then enter text
	BOOL bUsePrevValue;	//if true then fValue will be in edit buf
};

extern struct tagEditMode EditModeControl;

extern const struct tagMode modes_EditMode;








const char* EditMode_NameOnUpdate(void);
const char* EditMode_LeftOnUpdate(void);
const char* EditMode_RightOnUpdate(void);
const char* EditMode_UpOnUpdate(void);
const char* EditMode_DownOnUpdate(void);


void EditMode_Init(void);
BOOL EditMode_OnActivate(void);
BOOL EditMode_OnTimer(void);
BOOL EditMode_OnLeft(void);
BOOL EditMode_OnRight(void);
BOOL EditMode_OnUp(void);
BOOL EditMode_OnDown(void);
BOOL EditMode_OnIdle(void);
BOOL EditMode_OnShow(void);
BOOL EditMode_OnExit(void);
BOOL EditMode_OnWakeUp(void);
BOOL EditMode_OnPowerDown(void);

void EditMode_showModeScreen(void);
void EditMode_showModeHeader(void);
void EditMode_showValueRange(void);
void EditMode_showValue(void);
void EditMode_showDimension(void);
void EditMode_showCursor(void);
void EditMode_showValuePrev(void);
void EditMode_showRemain(void);

void EditMode_EditFloat(const char* pHeader,
						 float fValue,
						 float fMin,
						 float fMax,
						 const char* pDimension,
						 const char* pMask,
						 void(*pRetFunction)(BOOL bOK));

void EditMode_EditInt(const char* pHeader,
						 int iValue,
						 int iMin,
						 int iMax,
						 const char* pDimension,
						 void(*pRetFunction)(BOOL bOK));

void EditMode_EditInt_ex(const char* pHeader,
						 int iValue,
						 int iMin,
						 int iMax,
						 const char* pDimension,
						 void(*pRetFunction)(BOOL bOK),
						BOOL bUsePrevValue);

void EditMode_EditText(const char* pHeader,
//						 const char* pText,
						 int iTextLen,
						 void(*pRetFunction)(BOOL bOK));



#endif	//#ifndef _EDIT_H
