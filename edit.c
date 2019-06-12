//edit.c


#include <string.h>
#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <math.h>
#include <stdlib.h>

#include "edit.h"

#include "types.h"
//#include "syncObj.h"
#include "display.h"
//#include "keyboard.h"
#include "powerControl.h"
#include "sound.h"
#include "interrupts.h"
#include "modes.h"









struct tagEditMode EditModeControl;


const struct tagMode modes_EditMode=
{
	EditMode_NameOnUpdate,//"SPRD\0""СПРД",	//mode name
	RGB(20,63,20),	//mode color
	EditMode_LeftOnUpdate,//"setup\0""опции",	//right
	EditMode_RightOnUpdate,//"rid\0""рид",	//left
	EditMode_UpOnUpdate,//"bkg\0""фон",//up
	EditMode_DownOnUpdate,//"menu\0""меню",	//down
	EditMode_OnActivate,
	EditMode_OnLeft,
	EditMode_OnRight,
	EditMode_OnUp,
	EditMode_OnDown,
	EditMode_OnIdle,
	EditMode_OnShow,
	EditMode_OnExit,
	EditMode_OnPowerDown,
	EditMode_OnWakeUp,
	EditMode_OnTimer
};

const char* EditMode_NameOnUpdate(void)//"SPRD\0""СПРД",	//mode name
{
	return "EDIT\0""EDIT\0""EDIT\0""ВВОД";
}
const char* EditMode_LeftOnUpdate(void)//"setup\0""опции",	//right
{
	if(EditModeControl.bText)
		return "symbol\0""symbol\0""symbol\0""символ";
	else
		return "digit\0""digit\0""digit\0""цифра";
}
const char* EditMode_RightOnUpdate(void)//"rid\0""рид",	//left
{
	return "next\0""next\0""next\0""дальше";
}
const char* EditMode_UpOnUpdate(void)//"acquir\0""набор",//up
{
	if(EditModeControl.bFloat && /*allow enter float value*/
	   !EditModeControl.bDotPlaced && /*dot is not placed yet*/
		   EditModeControl.iCursor<EditModeControl.iSymbolNumber-1 /*there is a place for dot and one number at least*/
			   )
		return "point\0""point\0""point\0""точка";
	else
		return "enter\0""enter\0""enter\0""ввод";
}
const char* EditMode_DownOnUpdate(void)//"menu\0""меню",	//down
{
	return "cancel\0""cancel\0""cancel\0""отмена";
}





void EditMode_Init(void)
{
	EditModeControl.iCursor = 0;
	EditModeControl.pHeaderText = NULL;	//name of the value
	EditModeControl.pDimensionText = NULL;	//name of the dimension
	EditModeControl.pValueMask = NULL;	//mask to show value
	EditModeControl.fMinValue = 0;
	EditModeControl.fMaxValue = 0;
	EditModeControl.iSymbolNumber = 0;	//set allowed number of symbol
	EditModeControl.bFloat = FALSE;	//if true then allows to enter DOT
	EditModeControl.pRetFunction = NULL;
	EditModeControl.bText = FALSE;
	EditModeControl.bDotPlaced = FALSE;
	EditModeControl.bUsePrevValue = FALSE;
}



void EditMode_EditFloat(const char* pHeader,
						 float fValue,
						 float fMin,
						 float fMax,
						 const char* pDimension,
						 const char* pMask,
						 void(*pRetFunction)(BOOL bOK))
{
	char sym;
	int i;
	EditModeControl.bText = FALSE;
	EditModeControl.fValue = fValue;
	EditModeControl.fMinValue = fMin;
	EditModeControl.fMaxValue = fMax;
	EditModeControl.bDotPlaced = FALSE;
	EditModeControl.bFloat = TRUE;
	EditModeControl.pHeaderText = pHeader;
	EditModeControl.pDimensionText = pDimension;
	EditModeControl.pValueMask = pMask;
	EditModeControl.pRetFunction = pRetFunction;
	if(fMax<10)
		EditModeControl.iSymbolNumber = 3;
	else if(fMax<100)
		EditModeControl.iSymbolNumber = 4;
	else if(fMax<1000)
		EditModeControl.iSymbolNumber = 5;
	else if(fMax<10000)
		EditModeControl.iSymbolNumber = 6;
	else if(fMax<100000)
		EditModeControl.iSymbolNumber = 7;
	else
		EditModeControl.iSymbolNumber = 8;
	
	//check mask to increase iSymbolNumber
	i=0;
	while((sym=pMask[i++]))
	{
		if(sym=='f')
		{
			if(i>3)
			{
				if((sym=pMask[i-2]))
				{
					if(sym=='.')
					{
						if((sym=pMask[i-1]))
						{
							if(sym>='2' && sym<='9')
								EditModeControl.iSymbolNumber += sym-'0'-1;
						}
					}
				}
			}else
				EditModeControl.iSymbolNumber = 8;
			break;
		}
	}
	
	EditModeControl.bUsePrevValue = FALSE;
	Modes_setActiveMode(&modes_EditMode);
}



void EditMode_EditInt(const char* pHeader,
						 int iValue,
						 int iMin,
						 int iMax,
						 const char* pDimension,
						 void(*pRetFunction)(BOOL bOK))
{
	EditMode_EditInt_ex(pHeader,
						iValue,
						iMin,
						iMax,
						pDimension,
						*pRetFunction,
						FALSE);
}

void EditMode_EditInt_ex(const char* pHeader,
						 int iValue,
						 int iMin,
						 int iMax,
						 const char* pDimension,
						 void(*pRetFunction)(BOOL bOK),
						 BOOL bUsePrevValue)
{
	EditModeControl.bText = FALSE;
	EditModeControl.fValue = (float)iValue;
	EditModeControl.fMinValue = (float)iMin;
	EditModeControl.fMaxValue = (float)iMax;
	EditModeControl.bDotPlaced = FALSE;
	EditModeControl.bFloat = FALSE;
	EditModeControl.pHeaderText = pHeader;
	EditModeControl.pDimensionText = pDimension;
	EditModeControl.pValueMask = "%.0f";
	EditModeControl.pRetFunction = pRetFunction;
	EditModeControl.bUsePrevValue = bUsePrevValue;
	if(iMax<10)
		EditModeControl.iSymbolNumber = 1;
	else if(iMax<100)
		EditModeControl.iSymbolNumber = 2;
	else if(iMax<1000)
		EditModeControl.iSymbolNumber = 3;
	else if(iMax<10000)
		EditModeControl.iSymbolNumber = 4;
	else if(iMax<100000)
		EditModeControl.iSymbolNumber = 5;
	else
		EditModeControl.iSymbolNumber = 6;
	Modes_setActiveMode(&modes_EditMode);
}


void EditMode_EditText(const char* pHeader,
//						 const char* pText,
						 int iTextLen,
						 void(*pRetFunction)(BOOL bOK))
{
	EditModeControl.bText = TRUE;
	EditModeControl.bDotPlaced = FALSE;
	EditModeControl.pHeaderText = pHeader;
	EditModeControl.pRetFunction = pRetFunction;
	EditModeControl.iSymbolNumber = iTextLen;
	EditModeControl.bUsePrevValue = FALSE;
//	strncpy(EditModeControl.prevText,pText,EDIT_BUF_LEN);
	Modes_setActiveMode(&modes_EditMode);
}



BOOL EditMode_OnActivate(void)
{
	memset(EditModeControl.edit_buf,0,EDIT_BUF_LEN);
	EditModeControl.iCursor = 0;
	if(EditModeControl.bText)
	{
		if(modeControl.bLang==enu_lang_russian)
		{//russian
			EditModeControl.edit_buf[0] = 'а';
		}else
		{//english etc
			EditModeControl.edit_buf[0] = 'a';
		}
	}else
	{
		sprintf(EditModeControl.prevText, EditModeControl.pValueMask, EditModeControl.fValue);
		if(EditModeControl.bUsePrevValue)
		{
			strncpy(EditModeControl.edit_buf, EditModeControl.prevText, EDIT_BUF_LEN);
			EditModeControl.iCursor = strlen(EditModeControl.edit_buf)-1;
		}else
			EditModeControl.edit_buf[0] = '1';
	}
	Modes_createTimer(500);
	Modes_updateMode();
	return 1;
}

BOOL EditMode_OnTimer(void)
{
	//blink cursor
//	EditMode_showValue();
//	EditMode_showCursor();
	return 1;
}

//next digit or symbol
BOOL EditMode_OnLeft(void)
{
	char sym = EditModeControl.edit_buf[EditModeControl.iCursor];
	if(EditModeControl.bText)
	{
		if(modeControl.bLang==enu_lang_russian)
		{//russ
			if(sym=='я')sym='0';
			else if(sym=='9')sym='_';
			else if(sym=='_')sym='а';
			else ++sym;
		}else
		{//engl
			if(sym=='z')sym='0';
			else if(sym=='9')sym='_';
			else if(sym=='_')sym='a';
			else ++sym;
		}
	}else
	{
		if(++sym>'9')sym='0';
	}
	EditModeControl.edit_buf[EditModeControl.iCursor] = sym;
	EditMode_showValue();
	EditMode_showCursor();
	EditMode_showRemain();
	return 1;
}
//next position
BOOL EditMode_OnRight(void)
{
	if(EditModeControl.iSymbolNumber>=EDIT_BUF_LEN)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid parameters");	
	}
	
	if(++EditModeControl.iCursor>=EditModeControl.iSymbolNumber)
	{//edit first symbol, all other symbols are deleted
		EditModeControl.iCursor = 0;
		EditModeControl.bDotPlaced = FALSE;
		Modes_showButtons();
	}else
	{
		if(EditModeControl.bText)
		{
			if(modeControl.bLang==enu_lang_russian)
			{//russian
				EditModeControl.edit_buf[EditModeControl.iCursor] = 'а';
			}else
			{//english etc
				EditModeControl.edit_buf[EditModeControl.iCursor] = 'a';
			}
		}else
		{
			EditModeControl.edit_buf[EditModeControl.iCursor] = '0';
		}
	}
	//int len = strlen(EditModeControl.edit_buf);
	EditModeControl.edit_buf[EditModeControl.iCursor+1] = '\0';
	EditMode_showValue();
	EditMode_showCursor();
	EditMode_showRemain();
	return 1;
}
//enter
BOOL EditMode_OnUp(void)
{
	if(!EditModeControl.bText && EditModeControl.bFloat &&
	   !EditModeControl.bDotPlaced &&
		EditModeControl.iCursor<EditModeControl.iSymbolNumber-2)
	{//place dot if we have to do it
		EditModeControl.bDotPlaced = TRUE;
		EditModeControl.edit_buf[++EditModeControl.iCursor] = '.';
		EditModeControl.edit_buf[++EditModeControl.iCursor] = '0';
		EditModeControl.edit_buf[EditModeControl.iCursor+1] = '\0';
		EditMode_showValue();
		EditMode_showCursor();
		EditMode_showRemain();
		Modes_showButtons();	//buttons changed, have to update view
	}else
	{//enter
		if(!EditModeControl.bText)
		{
			float fval = atof(EditModeControl.edit_buf);
			//test on limits
			if(fval<EditModeControl.fMinValue ||
			   fval>EditModeControl.fMaxValue)
			{
				return 1;	//out of limits
			}
		}
			
		if(!EditModeControl.pRetFunction)
			exception(__FILE__,__FUNCTION__,__LINE__,"Invalid parameters");	
		
		SoundControl_BeepSeq(beepSeq_OK);
//		sound_playSample(SND_OK);
		
		(*EditModeControl.pRetFunction)(TRUE);
	}
	return 1;
}
//cancel
BOOL EditMode_OnDown(void)
{
	if(!EditModeControl.pRetFunction)
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid parameters");	
	(*EditModeControl.pRetFunction)(FALSE);
	return 1;
}
BOOL EditMode_OnIdle(void)
{
	return 1;
}
BOOL EditMode_OnShow(void)
{
	EditMode_showModeScreen();
	return 1;
}
BOOL EditMode_OnExit(void)
{
	return 1;
}
BOOL EditMode_OnWakeUp(void)
{
	return 1;
}
BOOL EditMode_OnPowerDown(void)
{
	return 1;	//allow enter power down
}


void EditMode_showModeScreen(void)
{
	EditMode_showModeHeader();
	if(!EditModeControl.bText)//in text edit dont show range
	{
		EditMode_showValueRange();
		EditMode_showValuePrev();
	}
	EditMode_showValue();
	EditMode_showCursor();
	if(!EditModeControl.bText)//in test edit dont show dimension
		EditMode_showDimension();
	EditMode_showRemain();
}

//show header of edit mode
void EditMode_showModeHeader(void)
{
	if(!EditModeControl.pHeaderText)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid parameters");
	}
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,100);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(10);
	Display_outputTextByLang(EditModeControl.pHeaderText);
//	Display_drawHLine(0,248,X_SCREEN_MAX,RGB(0,32,0));
}


void EditMode_showValueRange(void)
{
	sprintf(EditModeControl.rangeText, "[%.0f - %.0f]", EditModeControl.fMinValue,EditModeControl.fMaxValue);
	
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,MODE_USER_TOP+100,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(10);
	Display_outputText(EditModeControl.rangeText);
}

//show prev value mean
void EditMode_showValuePrev(void)
{
	if(!EditModeControl.pValueMask)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid parameters");
	}
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,MODE_USER_TOP+120,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(10);
	Display_outputText(EditModeControl.prevText);
}

void EditMode_showValue(void)
{
	Display_setTextColor(ORANGE);	//set text color
	Display_setTextWin(0,MODE_USER_TOP+140,X_SCREEN_SIZE,19);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(10);
	Display_outputText(EditModeControl.edit_buf);
}


void EditMode_showDimension(void)
{
	if(!EditModeControl.pHeaderText)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid parameters");
	}
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,MODE_USER_TOP+160,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(10);
	Display_outputTextByLang(EditModeControl.pDimensionText);
}

void EditMode_showRemain(void)
{
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,MODE_USER_TOP+180,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(10);
	char buf[40];
	int sumremain = EditModeControl.iSymbolNumber-EditModeControl.iCursor-1;
	if(modeControl.bLang==enu_lang_russian)
		sprintf(buf, "Осталось симв.: %d", sumremain);
	else if(modeControl.bLang==enu_lang_french)
		sprintf(buf, "Remaining symbols: %d", sumremain);
	else if(modeControl.bLang==enu_lang_german)
		sprintf(buf, "Remaining symbols: %d", sumremain);
	else
		sprintf(buf, "Remaining symbols: %d", sumremain);
	Display_outputText(buf);
}


//show cursor by xor mask
//it means that next call will remove it
//!!!!!! must be called after EditMode_showValue only
void EditMode_showCursor(void)
{
	//display.text.gstrX contains last x of last symbol
	int gstrX = display.text.gstrX+display.text.winX;
	int gstrY = display.text.winY+display.text.gstrY+Display_getFontSizeY()+1;
	int sx = Display_getSymWidth('0');
	Display_drawLine(gstrX-sx,gstrY,gstrX,gstrY, YELLOW);
	Display_drawLine(gstrX-sx,gstrY-1,gstrX,gstrY-1, YELLOW);
}
