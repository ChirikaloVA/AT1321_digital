//display driver
//31/10/2008 Bystrov
//coordinates: by Y: from 0 to 320 goes from down to up!!!!

#include <string.h>
#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include "display.h"
#include "types.h"
#include "syncObj.h"
#include "powerControl.h"
#include "gps.h"
#include "bluetooth.h"
#include "clock.h"
#include "USBRS.h"
#include "interrupts.h"
#include "InterProc.h"
#include "sound.h"
#include "modes.h"
#include "main.h"
#include "SPRD_Mode.h"
#include "LOG_Mode.h"
#include "filesystem.h"
#include <stdlib.h>















//!!!!!!!!! ширина bmp должна быть кратной 4

const BYTE bmp_bat0[]={
#include ".\gfx\bat0.bmp.txt"
};
const BYTE bmp_bat1[]={
#include ".\gfx\bat1.bmp.txt"
};
const BYTE bmp_bat2[]={
#include ".\gfx\bat2.bmp.txt"
};
const BYTE bmp_bat3[]={
#include ".\gfx\bat3.bmp.txt"
};
const BYTE bmp_bat4[]={
#include ".\gfx\bat4.bmp.txt"
};

const BYTE bmp_bth[]={
#include ".\gfx\bth.bmp.txt"
};
const BYTE bmp_gps[]={
#include ".\gfx\gps.bmp.txt"
};

const BYTE bmp_memlow[]={
#include ".\gfx\memlow.bmp.txt"
};


const BYTE bmp_sound[]={
#include ".\gfx\sound.bmp.txt"
};

const BYTE bmp_vibro[]={
#include ".\gfx\vibro.bmp.txt"
};

const BYTE bmp_silent[]={
#include ".\gfx\silent.bmp.txt"
};





struct tagDisplayData display;



//char ON_OFF_display;


//точки уложены битами в байтах с лева на право (байты), т.е. байт одна верт лини€ или ее половина в зависимости от размера шрифта
const BYTE fnt6x16[] = {
#include "font6x16.h"
};

const BYTE fnt8x16[] = {
#include "font8x16.h"
};

const BYTE fnt16x16[] = {
#include "font16x16 v3.h"
};

const BYTE fnt32x32[] = {
#include "font32x32.h"
};



//return text line width
int Display_getTextLineWidth(const char* pText)
{
	int x = -display.text.stepX;
	char sym;
	while((sym = *pText++))
	{
		if(sym=='\r')
		{//перевод строки
			break;
		}else
		{
			x += Display_getSymWidth(sym)+display.text.stepX;
		}
	};
	return x;
}

//calculation of center start position for the line
int Display_getCenterStartPos(const char* pText)
{
	int startpos = 0;
	int pixlen = Display_getTextLineWidth(pText);
	int winlen = display.text.winSX;
	if(winlen>pixlen)
	{
		startpos = (winlen-pixlen)/2;
	}
	return startpos;
}


//calculation of center start position for the line
int Display_getLeftStartPos(const char* pText)
{
	int startpos = 0;
	return startpos;
}

//calculation of center start position for the line
int Display_getRightStartPos(const char* pText)
{
	int startpos = 0;
	int pixlen = Display_getTextLineWidth(pText);
	int winlen = display.text.winSX;
	startpos = winlen-pixlen;
	return startpos;
}

//justify text
void Display_justifyText(const char* pText)
{
	if(display.text.justify==CENTER)
		display.text.gstrX = Display_getCenterStartPos(pText);
	else if(display.text.justify==LEFT)
		display.text.gstrX = Display_getLeftStartPos(pText);
	else if(display.text.justify==RIGHT)
		display.text.gstrX = Display_getRightStartPos(pText);
}


const char* Display_getTextByLang(const char* pText)
{
	for(int i=0;i<modeControl.bLang;i++)
	{
		pText+=strlen(pText)+1;
	};
	return pText;
}

void Display_outputTextByLang(const char* pText)
{
	Display_outputText(Display_getTextByLang(pText));
}

void Display_outputTextByLang_withclean(const char* pText,  int clr_pos)
{
	Display_outputText_withclean(Display_getTextByLang(pText),  clr_pos);
}


//display msg and log it
void Display_outputTextByLang_log(const char* pText)
{
	Display_outputTextByLang(pText);
	LOGMode_insertEventByLang(pText);
}

//ret word len in pix
int Display_getWordLen(const char* pText)
{
	int len = 0;
	char sym;
	while((sym = *pText++))
	{
		if(sym==' ' || sym=='\r')break;
		len+=Display_getSymWidth(sym)+display.text.stepX;
	};
	len-=display.text.stepX;
	return len;
}


void Display_outputText(const char* pText)
{
	int x;
	char sym;
	BOOL bWordTested = FALSE;
	Display_justifyText(pText);
	while((sym = *pText++))
	{
		if(sym=='\r')
		{//перевод строки
			if(!Display_gotoNextLine())return;//out of bounds
			Display_checkForClearLine();
			Display_justifyText(pText);
			bWordTested = FALSE;
		}else
		{
			if(display.text.bWrap && !bWordTested && sym!=' ')
			{
				bWordTested = TRUE;
				int len = Display_getWordLen(pText-1);
				if((display.text.gstrX+len)>display.text.winSX)
				{
					if(!Display_gotoNextLine())return;	//out of bounds
					Display_checkForClearLine();
				}
			}
			if(sym==' ')bWordTested = FALSE; //to check next word
			x = Display_showSymbol(sym);
                        Display_ClearSteps(display.text.stepX, x + display.text.winX);
			Display_inc_textX(x+display.text.stepX);
		}
	};
}

void Display_outputText_withclean(const char* pText, int clr_pos)
{
	int x;
	char sym;
	BOOL bWordTested = FALSE;
	Display_justifyText(pText);
	while((sym = *pText++))
	{
		if(sym=='\r')
		{//перевод строки
			if(!Display_gotoNextLine())return;//out of bounds
			Display_checkForClearLine();
			Display_justifyText(pText);
			bWordTested = FALSE;
		}else
		{
			if(display.text.bWrap && !bWordTested && sym!=' ')
			{
				bWordTested = TRUE;
				int len = Display_getWordLen(pText-1);
				if((display.text.gstrX+len)>display.text.winSX)
				{
					if(!Display_gotoNextLine())return;	//out of bounds
					Display_checkForClearLine();
				}
			}
			if(sym==' ')bWordTested = FALSE; //to check next word
			x = Display_showSymbol(sym);
                        Display_ClearSteps(display.text.stepX, x + display.text.winX);
			Display_inc_textX(x+display.text.stepX);
		}
	};
        if(x >= display.text.stepX)
        {
          x = x - display.text.stepX;
        }
        if(x < clr_pos)
        {
          
          Display_Symbol_clr(x,clr_pos);
          
        }
}


/*
calculate amount of text lines in the text
before use it is important to call the next:
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,1);
	Display_setTextJustify(LEFT);
	Display_setCurrentFont(fnt6x16);
*/
int Display_calcTextLines(const char* pText)
{
	int iTextLines = 1;
	int x;
	char sym;
	//to restore them further
	int xx = display.text.gstrX;
	int yy = display.text.gstrY;

	int fy = Display_getFontSizeY();
	BOOL bWordTested = FALSE;
	Display_justifyText(pText);
	while((sym = *pText++))
	{
		if(sym=='\r')
		{//перевод строки
			iTextLines++;
			Display_gotoNextLine();
			//пересчет координаты х
			Display_justifyText(pText);
			bWordTested = FALSE;
		}else
		{
			if(display.text.bWrap && !bWordTested && sym!=' ')
			{
				bWordTested = TRUE;
				int len = Display_getWordLen(pText-1);
				if((display.text.gstrX+len)>display.text.winSX)
				{
					iTextLines++;
					Display_gotoNextLine();
					--pText;
					continue;
				}
			}
			if(sym==' ')bWordTested = FALSE; //to check next word
			x = Display_getSymWidth(sym);
			Display_inc_textX(x+display.text.stepX);
		}
	};
	display.text.gstrX = xx;
	display.text.gstrY = yy;
	return iTextLines;
}


/*
output text that not depends on language from pointed text line
*/
void Display_outputTextFromLine(const char* pText, int lineIndex)
{
	int x;
	char sym;
	int fy = Display_getFontSizeY();
	BOOL bWordTested = FALSE;
	Display_justifyText(pText);
	while((sym = *pText++))
	{
		if(sym=='\r')
		{//перевод строки
			if(lineIndex==0)
			{//переводим строку если уже нужно выводить текст
				Display_gotoNextLine();
				if((display.text.gstrY+fy-1)>display.text.winSY)break;
				Display_checkForClearLine();
			}else//иначе ждем следующей строки текста
			{
				--lineIndex;
				display.text.gstrX = 0;	//это нужно дл€ перевода строки, хот€ y не мен€етс€
			}
			//пересчет координаты х
			Display_justifyText(pText);
			bWordTested = FALSE;
		}else
		{
			BOOL bAllowOutput = (lineIndex==0);
			if(display.text.bWrap && !bWordTested && sym!=' ')
			{
				bWordTested = TRUE;
				int len = Display_getWordLen(pText-1);
				if((display.text.gstrX+len)>display.text.winSX)
				{
					if(bAllowOutput)
					{//нужно выводить текст
						Display_gotoNextLine();
						if((display.text.gstrY+fy-1)>display.text.winSY)break;
						Display_checkForClearLine();
					}else
					{
						--lineIndex;
						display.text.gstrX = 0;
						--pText;	//вернемс€ на символ назад чтобы начать вывод этого слова
						continue;
					}
				}
			}
			if(sym==' ')bWordTested = FALSE; //to check next word
			if(bAllowOutput)
				x = Display_showSymbol(sym);
			else
				x = Display_getSymWidth(sym);
			Display_inc_textX(x+display.text.stepX);
		}
	};
}

//set text justify, must be values from enum ENUM_TEXT_JUSTIFY
void Display_setTextJustify(enum ENUM_TEXT_JUSTIFY justify)
{
	display.text.justify = justify;
}

//set text window
void Display_setTextWin(int X,int Y,int SX,int SY)
{
	if(X+SX>X_SCREEN_SIZE || Y+SY>Y_SCREEN_SIZE)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Text window is out of bounds");
	}
	display.text.winX = X;
	display.text.winY = Y;
	display.text.winSX = SX;
	display.text.winSY = SY;
	Display_setTextXY(0,0);	//set start coords in window from 0,0
}

//clear RECT that is match to text window
void Display_clearTextWin(int vsync)
{
	RECT rect={display.text.winX,display.text.winY,display.text.winX+display.text.winSX-1,display.text.winY+display.text.winSY-1};
	Display_clearRect(rect, vsync);
}


void Display_checkForClearLine(void)
{
	if(display.text.bTextLineClear)
	{//must clear the line
		int sz = Display_getFontSizeY()*(display.text.bDoubleHeight?2:1)+display.text.stepY;
		int gstrY;
		gstrY = display.text.winY+display.text.gstrY;
		if(gstrY>Y_SCREEN_SIZE-sz)return;
		RECT rect={display.text.winX,gstrY,display.text.winX+display.text.winSX-1,gstrY+sz};
		Display_clearRect(rect, gstrY+16);
	}
}

BOOL Display_gotoNextLine(void)
{
	display.text.gstrX = 0;
	int sz = Display_getFontSizeY()*(display.text.bDoubleHeight?2:1)+display.text.stepY;
	display.text.gstrY += sz;
	return (BOOL)(display.text.gstrY<(display.text.winY+display.text.winSY));
}



void Display_setTextDoubleHeight(BOOL bDoubleHeight)
{
	display.text.bDoubleHeight = bDoubleHeight;
}


//set text wrap
void Display_setTextWrap(BOOL wrap)
{
	display.text.bWrap = wrap;
}

//set incrementing step values
void Display_setTextSteps(int stepX, int stepY)
{
	display.text.stepX = stepX;
	display.text.stepY = stepY;
}


//increment of gstrX of text functions
int Display_inc_textX(int x)
{
	display.text.gstrX+=x;
	return display.text.gstrX;
}

void Display_setTextXY(int x, int y)
{
	display.text.gstrX = x;
	display.text.gstrY = y;
}

void Display_setCurrentFont(const BYTE* pFont)
{
	display.text.pFont = pFont;
}

const BYTE* Display_getCurrentFont(void)
{
	return display.text.pFont;
}

const BYTE* Display_getSymFontOffset(char symbol)
{
	const BYTE* pFont = Display_getCurrentFont();
	int si = symbol;
	for(int i=32;i<si;i++)
	{
		pFont+=*pFont;
	}
	return pFont;
}

//get current font x size
//должно быть кратно 8
UINT Display_getFontSizeY(void)
{
	int fsy = 16;
	if(display.text.pFont==fnt8x16)
	{
		fsy = 16;
	}else
	if(display.text.pFont==fnt16x16)
	{
		fsy = 16;
	}else
	if(display.text.pFont==fnt32x32)
	{
		fsy = 32;
	}
	return fsy;
}


//get symbol width in pixels
UINT Display_getSymWidth(char symbol)
{
	const BYTE* pFont = Display_getSymFontOffset(symbol);
	UINT len = (*pFont++)-1;	//symbol data len in bytes
	return (len*8/Display_getFontSizeY());	//symbol width
}

//set text ciolor
void Display_setTextColor(COLORREF clr)
{
	display.text.color = clr;
}

void Display_ClearSteps(int stepX, int winX)
{
  volatile BYTE* pData_b = (BYTE*)0x81000000;
  int gstrX, gstrY;
  UINT len;
  gstrX = display.text.gstrX+winX;
  gstrY = display.text.winY+display.text.gstrY;

	UINT fsy = Display_getFontSizeY();
	//get symbol address
	
	UINT fsx = stepX;	//symbol width

	fsy*=(display.text.bDoubleHeight?2:1);
        
        len = fsy * stepX;

	Display_Init_8bit_262k();

	//set clip region
	Display_set_clip_region(gstrX,gstrY,gstrX+fsx-1,gstrY+fsy-1);
	Display_set_screen_memory_adr(gstrX,gstrY);

//    BYTE c1=LOBYTE(display.text.color),c2=LO2BYTE(display.text.color),c3=LO3BYTE(display.text.color);
//	BYTE read;
//
//	WORD wrd;
//	c1 = c1>>1;
//	c1 |= (c2<<5)&0xe0;
//	wrd = c3;
//	pData+=(WORD)((wrd<<3)&0x1f8)|(WORD)(c2>>3);

	Display_Init_18bit_262k_updownleftright();



	CLR_RS;
	DisplayData = 0x22;
	SET_RS;


	BYTE byt;
	if(display.text.bDoubleHeight)
	{
		do
		{
                        {*pData_b=0 ;*pData_b = 0;}
			{*pData_b=0 ;*pData_b = 0;}
                        {*pData_b=0 ;*pData_b = 0;}
                        {*pData_b=0 ;*pData_b = 0;}
			{*pData_b=0 ;*pData_b = 0;}
			{*pData_b=0 ;*pData_b = 0;}
			{*pData_b=0 ;*pData_b = 0;}
			{*pData_b=0 ;*pData_b = 0;}
                        
                        
		}while(--len);
	}else
	{
		do
		{                       
			{*pData_b = 0;}
			{*pData_b = 0;}
			{*pData_b = 0;}
			{*pData_b = 0;}
                        {*pData_b = 0;}
			{*pData_b = 0;}
			{*pData_b = 0;}
			{*pData_b = 0;}
                        
		}while(--len);
	}

	Display_Init_8bit_262k();
}



//ret symbol width
//show symbol
int Display_showSymbol(char symbol)
{
  volatile BYTE* pData_b = (BYTE*)0x81000000;
	volatile BYTE* pData = (BYTE*)0x81000000;
	int gstrX, gstrY;
	gstrX = display.text.gstrX+display.text.winX;
	gstrY = display.text.winY+display.text.gstrY;
	if(gstrX>=X_SCREEN_SIZE || gstrY>=Y_SCREEN_SIZE)return 0;

	UINT fsy = Display_getFontSizeY();
	//get symbol address
	const BYTE* pFont = Display_getSymFontOffset(symbol);
	UINT len = (*pFont++)-1;	//symbol data len in bytes
	if(len==0)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Symbol len in font must be >0");
	}
	UINT fsx = len*8/fsy;	//symbol width

	fsy*=(display.text.bDoubleHeight?2:1);

	Display_Init_8bit_262k();

	//set clip region
	Display_set_clip_region(gstrX,gstrY,gstrX+fsx-1,gstrY+fsy-1);
	Display_set_screen_memory_adr(gstrX,gstrY);

    BYTE c1=LOBYTE(display.text.color),c2=LO2BYTE(display.text.color),c3=LO3BYTE(display.text.color);
	BYTE read;

	WORD wrd;
	c1 = c1>>1;
	c1 |= (c2<<5)&0xe0;
	wrd = c3;
	pData+=(WORD)((wrd<<3)&0x1f8)|(WORD)(c2>>3);

	Display_Init_18bit_262k_updownleftright();



	CLR_RS;
	DisplayData = 0x22;
	SET_RS;


	BYTE byt;
	if(display.text.bDoubleHeight)
	{
		do
		{
//			byt = *pFont++;
//			if(byt&0x01){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x02){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x04){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x08){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x10){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x20){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x40){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x80){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
                        
                        byt = *pFont++;
			if(byt&0x01){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x02){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x04){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x08){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x10){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x20){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x40){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x80){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
                        
                        
		}while(--len);
	}else
	{
		do
		{
//			byt = *pFont++;
//			if(byt&0x01){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x02){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x04){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x08){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x10){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x20){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x40){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x80){*pData=c1;}
//			else {read = *pData;}
                        
                  byt = *pFont++;
			if(byt&0x01){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x02){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x04){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x08){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x10){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x20){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x40){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x80){*pData=c1;}
			else {*pData_b = 0;}
                        
		}while(--len);
	}

	Display_Init_8bit_262k();

	return fsx;
}




int Display_showSymbol_clr(char symbol, int vsync)
{
        volatile BYTE* pData_b = (BYTE*)0x81000000;
	volatile BYTE* pData = (BYTE*)0x81000000;
	int gstrX, gstrY;
        int len_clr;
	gstrX = display.text.gstrX+display.text.winX;
	gstrY = display.text.winY+display.text.gstrY;
	if(gstrX>=X_SCREEN_SIZE || gstrY>=Y_SCREEN_SIZE)return 0;

	UINT fsy = Display_getFontSizeY();
	//get symbol address
	const BYTE* pFont = Display_getSymFontOffset(symbol);
	UINT len = (*pFont++)-1;	//symbol data len in bytes
	if(len==0)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Symbol len in font must be >0");
	}
	UINT fsx = len*8/fsy;	//symbol width

	fsy*=(display.text.bDoubleHeight?2:1);

	Display_Init_8bit_262k();

	//set clip region
//	Display_set_clip_region(gstrX,gstrY,gstrX+fsx-1,gstrY+fsy-1);
//        if(display.text.bDoubleHeight)
//        {
//          len_clr = 32;
//        }
//        else
//        {
//          len_clr = 16;
//        }
        Display_set_clip_region(gstrX,gstrY,gstrX+fsy-1,gstrY+fsy-1);
	Display_set_screen_memory_adr(gstrX,gstrY);

    BYTE c1=LOBYTE(display.text.color),c2=LO2BYTE(display.text.color),c3=LO3BYTE(display.text.color);
	BYTE read;

	WORD wrd;
	c1 = c1>>1;
	c1 |= (c2<<5)&0xe0;
	wrd = c3;
	pData+=(WORD)((wrd<<3)&0x1f8)|(WORD)(c2>>3);

	Display_Init_18bit_262k_updownleftright();



	CLR_RS;
	DisplayData = 0x22;
	SET_RS;


	BYTE byt;
	if(display.text.bDoubleHeight)
	{
          len_clr = 128 - len;
		do
		{
//			byt = *pFont++;
//			if(byt&0x01){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x02){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x04){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x08){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x10){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x20){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x40){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
//			if(byt&0x80){*pData=c1;*pData=c1;}
//			else {read = *pData ;read = *pData;}
                        byt = *pFont++;
			if(byt&0x01){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x02){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x04){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x08){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x10){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x20){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x40){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
			if(byt&0x80){*pData=c1;*pData=c1;}
			else {*pData_b=0 ;*pData_b = 0;}
		}while(--len);
                if(len_clr > 0)
                {
                do
		{
                        byt = *pFont++;
			*pData_b=0 ;*pData_b = 0;
			*pData_b=0 ;*pData_b = 0;
			*pData_b=0 ;*pData_b = 0;
			*pData_b=0 ;*pData_b = 0;
			*pData_b=0 ;*pData_b = 0;
			*pData_b=0 ;*pData_b = 0;
			*pData_b=0 ;*pData_b = 0;
			*pData_b=0 ;*pData_b = 0;
			
		}while(--len_clr);
                }
                else
                {
                  len_clr = 9;
                }
	}else
	{
          len_clr = fsy * 2 - len;
		do
		{
//			byt = *pFont++;
//			if(byt&0x01){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x02){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x04){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x08){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x10){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x20){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x40){*pData=c1;}
//			else {read = *pData;}
//			if(byt&0x80){*pData=c1;}
//			else {read = *pData;}
                        byt = *pFont++;
			if(byt&0x01){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x02){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x04){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x08){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x10){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x20){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x40){*pData=c1;}
			else {*pData_b = 0;}
			if(byt&0x80){*pData=c1;}
			else {*pData_b = 0;}
		}while(--len);
                if(len_clr > 0)
                {
                do
		{
                        byt = *pFont++;
			*pData_b=0 ;
			*pData_b=0 ;
			*pData_b=0 ;
			*pData_b=0 ;
			*pData_b=0 ;
			*pData_b=0 ;
			*pData_b=0 ;
			*pData_b=0 ;
			
		}while(--len_clr);
                }
                else
                {
                  len_clr = 8;
                }
	}

	Display_Init_8bit_262k();

	return fsx;
}


void Display_Symbol_clr(int x,int vsync)
{
  volatile BYTE* pData_b = (BYTE*)FON_COLOR;
//  volatile BYTE* pData = (BYTE*)0x81000000;
  int gstrX, gstrY;
  int len_clr;
  gstrX = display.text.gstrX+display.text.winX;
  gstrY = display.text.winY+display.text.gstrY;
  if(gstrX>=X_SCREEN_SIZE || gstrY>=Y_SCREEN_SIZE)
  {
    return;
  }
  
  UINT fsy = Display_getFontSizeY();
  len_clr = (((fsy - 1) * (fsy - 1))/8);
  Display_Init_8bit_262k();
  Display_set_clip_region(gstrX,gstrY,gstrX+fsy-1,gstrY+fsy-1);
  Display_set_screen_memory_adr(gstrX,gstrY);
  Display_Init_18bit_262k_updownleftright();
  CLR_RS;
  DisplayData = 0x22;
  SET_RS;
  if(display.text.bDoubleHeight)
  {
    if(len_clr > 0)
    {
      do
      {
        *pData_b=FON_COLOR_BYTE ;*pData_b = FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE ;*pData_b = FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE ;*pData_b = FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE ;*pData_b = FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE ;*pData_b = FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE ;*pData_b = FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE ;*pData_b = FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE ;*pData_b = FON_COLOR_BYTE;
        
      }while(--len_clr);
    }
  }
  else
  {
    if(len_clr > 0)
    {
      do
      {
        *pData_b=FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE;
        *pData_b=FON_COLOR_BYTE;
        
      }while(--len_clr);
    }
  }
  
  Display_Init_8bit_262k();
}








//output bmp on screen
//!!!!!!!!! ширина bmp должны быть кратной 4
void Display_output_bmp(WORD x, WORD y, const BITMAPFILEHEADER* pBmp)
{
  volatile BYTE* pVData = (BYTE*)0x81000000;
  const BYTE* pData = (const BYTE*)pBmp+pBmp->bfOffBits;
  const BITMAPINFOHEADER* pInfo = (const BITMAPINFOHEADER*)((const BYTE*)pBmp+sizeof(struct tagBITMAPFILEHEADER));
  LONG dx = pInfo->biWidth;
  LONG dy = pInfo->biHeight;
  if(dy<0)dy=-dy;
  DWORD len = dx*dy;
  
  if(x>=X_SCREEN_SIZE)x=X_SCREEN_SIZE-1;
  if(y>=Y_SCREEN_SIZE)y=Y_SCREEN_SIZE-1;
  if(dx>=X_SCREEN_SIZE)dx=X_SCREEN_SIZE-1;
  if(dy>=Y_SCREEN_SIZE)dy=Y_SCREEN_SIZE-1;
  if(len==0)
  {
    exception(__FILE__,__FUNCTION__,__LINE__,"Bitmap len must be >0");
  }
  
  Display_Init_8bit_262k();
  
  Display_set_clip_region(x,y,x+dx-1,y+dy-1);
  
  
  if(pInfo->biHeight>0)
  {
    Display_set_screen_memory_adr(x,y+dy-1);
    Display_Init_18bit_262k_leftrightdownup();
  }else
  {
    Display_set_screen_memory_adr(x,y);
    Display_Init_18bit_262k();
  }
  
  WORD wrd;
  BYTE c1,c2,c3;
  
  CLR_RS;
  DisplayData = 0x22;
  SET_RS;
  do
  {
    c1=(*pData++)>>3;
    c2=*pData++;
    c3=*pData++;
    //здесь преобразование с учетом перевода из 8-бит в 6-бит формат
    c1 |= (c2<<3)&0xe0;
    wrd = c3;
    pVData= (BYTE*)(0x81000000+(WORD)((wrd<<1)&0x1f8)|(WORD)(c2>>5));
    *pVData=c1;
  }while(--len);
  
}










void Display_EMC18_BUS_Init(BOOL b18bit)
{
	PINSEL8_bit.P4_0 = b18bit; //EMC A0
	PINSEL8_bit.P4_1 = b18bit; //EMC A1
	PINSEL8_bit.P4_2 = b18bit; //EMC A2
	PINSEL8_bit.P4_3 = b18bit; //EMC A3
	PINSEL8_bit.P4_4 = b18bit; //EMC A4
	PINSEL8_bit.P4_5 = b18bit; //EMC A5
	PINSEL8_bit.P4_6 = b18bit; //EMC A6
	PINSEL8_bit.P4_7 = b18bit; //EMC A7
	PINSEL8_bit.P4_8 = b18bit; //EMC A8
}



//init EMC to USE with display
//low power consuption
void Display_EMC_Init(void)
{
	PowerControl_EMC_ON();
	PINSEL6_bit.P3_0 = 1; //EMC D0
	PINSEL6_bit.P3_1 = 1; //EMC D1
	PINSEL6_bit.P3_2 = 1; //EMC D2
	PINSEL6_bit.P3_3 = 1; //EMC D3
	PINSEL6_bit.P3_4 = 1; //EMC D4
	PINSEL6_bit.P3_5 = 1; //EMC D5
	PINSEL6_bit.P3_6 = 1; //EMC D6
	PINSEL6_bit.P3_7 = 1; //EMC D7
	PINSEL9_bit.P4_24 = 1; //EMC RD
	PINSEL9_bit.P4_25 = 2; //EMC WR
	PINSEL9_bit.P4_30 = 1; //EMC CSA
	PINSEL9_bit.P4_31 = 1; //EMC NCS

	Display_EMC18_BUS_Init(1);

	//configure EMC
	EMCCONTROL_bit.E = 1; //enable EMC
	EMCSTATICEXTENDEDWAIT = 0;  //fastest extended wait
	EMCSTATICCNFG1_bit.EW = 0; //disable extended wait
	EMCSTATICCNFG1_bit.B = 0; //disable buffer
	EMCSTATICWAITWR1 = 0; //1 CCKL Static Memory Write Delay
	EMCSTATICWAITTURN1 = 0; //1 CCKL Static Memory Turn Round Delay
	EMCSTATICWAITPG1 = 0;//1 CCKL Static Memory Page Mode Read Delay
	EMCSTATICWAITRD1 = 4;//5 CCKL Static Memory Read Delay
	EMCSTATICWAITOEN1 = 0;	//no Static Memory Output Enable delay
	EMCSTATICWAITWEN1 = 0;	//1 CCKL Static Memory Write Enable Delay
}






void Display_turnOFF(void)
{

	display.bLCDON = FALSE;



	PowerControl_EMC_OFF();



	SET_DPWON;//выключим питание с экрана
	pause(1000);
	CLR_PON;
	pause(1000);
	CLR_MON;
}



void Display_turnON(void)
{
  display.bLCDON = TRUE;


  PowerControl_EMC_ON();


  Display_Init_8bit_262k();

  CLR_RS;
  DisplayData = 0x10;
  SET_RS;
  DisplayData = 0x00;
  DisplayData = 0x00;

  CLR_RS;
  DisplayData = 0x18;
  SET_RS;
  DisplayData = 0x00;
  DisplayData = 0x1f;
  
  CLR_RS;
  DisplayData = 0x06;
  SET_RS;
  DisplayData = 0x00;
  DisplayData = 0x00;
  
  CLR_RS;
  DisplayData = 0xF8;
  SET_RS;
  DisplayData = 0x00;
  DisplayData = 0x09;
  
  CLR_RS;
  DisplayData = 0xF9;
  SET_RS;
  DisplayData = 0x00;
  DisplayData = 0x08;

  

//  CLR_RS;
//  DisplayData = 0x43;
//  SET_RS;
//  DisplayData = 0;
//  DisplayData = 0x1;//gamma set
  
//Gamma Register Setting:

  CLR_RS;
  DisplayData = 0x70;
  SET_RS;
  DisplayData = 0x2B;
  DisplayData = 0x80;
  CLR_RS;
  DisplayData = 0x71;
  SET_RS;
  DisplayData = 0x36;
  DisplayData = 0x00;  
  CLR_RS;
  DisplayData = 0x72;
  SET_RS;
  DisplayData = 0x3E;
  DisplayData = 0x00;
  CLR_RS;
  DisplayData = 0x73;
  SET_RS;
  DisplayData = 0x1F;
  DisplayData = 0x19;
  CLR_RS;
  DisplayData = 0x74;
  SET_RS;
  DisplayData = 0x22;
  DisplayData = 0x14; 
  CLR_RS;
  DisplayData = 0x75;
  SET_RS;
  DisplayData = 0x22;
  DisplayData = 0x1B;
  CLR_RS;
  DisplayData = 0x76;
  SET_RS;
  DisplayData = 0x1E;
  DisplayData = 0x16;
  CLR_RS;
  DisplayData = 0x77;
  SET_RS;
  DisplayData = 0x24;
  DisplayData = 0x1E;
  CLR_RS;
  DisplayData = 0x78;
  SET_RS;
  DisplayData = 0x26;
  DisplayData = 0x17;    
  
  Display_clearScreen();

  

  pause(50000);
//  pause(50000);
//  pause(50000);
//  pause(50000);
//  pause(50000);
//  pause(10000);

  SET_PON;

  pause(50000);
//  pause(50000);
//  pause(50000);
//  pause(50000);

  SET_MON;
  
  CLR_DPWON;
  
   pause(5000);
  
  CLR_RS;
  DisplayData = 0x05;
  SET_RS;
  DisplayData = 0x00;
  DisplayData = 0x01;

}


void Display_clearScreen(void)
{
	RECT rect = {0,0,X_SCREEN_SIZE-1,Y_SCREEN_SIZE-1};
	Display_clearRect(rect, 250);
}


//поймать лучь
//если задано 0xffff то поиск не ведетс€
void Display_catchVSYNC(int line)
{
	int vsync;
	int counter = 50000;
	BYTE b1,b2;
	while(line>=Y_SCREEN_SIZE)line -= Y_SCREEN_SIZE;
	while(line<0)line += Y_SCREEN_SIZE;
	do
	{
		CLR_RS;
		b1 = DisplayData;
		b2 = DisplayData;
		SET_RS;
		vsync = ((b1 & 0x01)<<8) | b2;
	}while((abs(vsync-line)>=3) && --counter);
}





void Display_set_clip_region(WORD x1, WORD y1, WORD x2, WORD y2)
{
	if(x1>=X_SCREEN_SIZE)x1=X_SCREEN_SIZE-1;
	if(x2>=X_SCREEN_SIZE)x2=X_SCREEN_SIZE-1;
	if(y1>=Y_SCREEN_SIZE)y1=Y_SCREEN_SIZE-1;
	if(y2>=Y_SCREEN_SIZE)y2=Y_SCREEN_SIZE-1;


	CLR_RS;
	DisplayData = 0x35;
	SET_RS;
	DisplayData = (BYTE)LO2BYTE(y1);
	DisplayData = (BYTE)LOBYTE(y1);
	CLR_RS;
	DisplayData = 0x36;
	SET_RS;
	DisplayData = (BYTE)LO2BYTE(y2);
	DisplayData = (BYTE)LOBYTE(y2);
	CLR_RS;
	DisplayData = 0x37;
	SET_RS;
	DisplayData = (BYTE)LOBYTE(x1);
	DisplayData = (BYTE)LOBYTE(x2);
}


void Display_set_screen_memory_adr(WORD x, WORD y)
{
	if(x>=X_SCREEN_SIZE)x=X_SCREEN_SIZE-1;
	if(y>=Y_SCREEN_SIZE)y=Y_SCREEN_SIZE-1;

	CLR_RS;
	DisplayData = 0x20;
	SET_RS;
	DisplayData = 0x00;
	DisplayData = (BYTE)LOBYTE(x);
	CLR_RS;
	DisplayData = 0x21;
	SET_RS;
	DisplayData = (BYTE)HIBYTE_W(y);
	DisplayData = (BYTE)LOBYTE(y);
}


//place dot on screen
//x,y coords x [0-239], y[0-319]
//clr is a RGB
void Display_dot(WORD x, WORD y, COLORREF clr)
{
	Display_set_screen_memory_adr(x,y);
	CLR_RS;
	DisplayData = 0x22;
	SET_RS;
	DisplayData = LO3BYTE(clr);
	DisplayData = LO2BYTE(clr);
	DisplayData = LOBYTE(clr);
}


COLORREF Display_read_dot(WORD x, WORD y)
{
	COLORREF clr;
	Display_set_screen_memory_adr(x,y);
	CLR_RS;
	DisplayData = 0x22;
	SET_RS;
	BYTE b1 = DisplayData;
	b1 = DisplayData;
	b1 = DisplayData;
	*((BYTE*)&clr+2) = DisplayData;
	*((BYTE*)&clr+1) = DisplayData;
	*((BYTE*)&clr+0) = DisplayData;
	return clr;
}

void Display_dot_xor(WORD x, WORD y, COLORREF clr)
{
	COLORREF clr1 = Display_read_dot(x,y);
	Display_dot(x,y,clr^clr1);
}




void Display_init(void)
{
	//====== ƒисплей ================================
	DIR_NCS = 1;
	DIR_NRES = 1;
	DIR_RS = 1;
	DIR_ID_MIB = 1;
	DIR_SPB = 1;
	SET_NRES;
	SET_RS;
	CLR_ID_MIB;
	CLR_SPB;

	//===============================================



	DIR_D8 = 1;
	DIR_D9 = 1;
	DIR_D10 = 1;
	DIR_D11 = 1;
	DIR_D12 = 1;
	DIR_D13 = 1;
	DIR_D14 = 1;
	DIR_D15 = 1;
	DIR_D16 = 1;
	DIR_D17 = 1;
	CLR_D8;
	CLR_D9;
	CLR_D10;
	CLR_D11;
	CLR_D12;
	CLR_D13;
	CLR_D14;
	CLR_D15;
	CLR_D16;
	CLR_D17;


//===========RGB======================================
        DIR_VSYNC = 1;
        CLR_VSYNC;
        DIR_HSYNC = 1;
        CLR_HSYNC;
        DIR_DOTCLK = 1;
        CLR_DOTCLK;
 //==========SPI================================
        DIR_SDOUT = 0;
        DIR_SDIN = 1;
        CLR_SDIN;


	DIR_P4_28 = 1;
	CLR_P4_28;
	DIR_P4_29 = 1;
	CLR_P4_29;

	//===== »ниацилизаци€ и включение диспле€ ============================
	CLR_NRES;
	SET_NRES;


	Display_turnON();


	//=======================================================


	//==adjust texts output values===========
	Display_setCurrentFont(fnt32x32);	//set current font
	Display_setTextColor(WHITE);	//set text color
	Display_setTextSteps(1,1);//set steps
	Display_setTextWin(0,0,X_SCREEN_SIZE,Y_SCREEN_SIZE);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setTextLineClear(0);
        
        display.adr = 0x81000102;
        display.data = 0x31;
        
        display.data_clr_b = 0x00;
        display.adr_clr_b = 0x81000000;
        
         display.data_clr_g = 0xE2;
//        display.adr_clr_g = 0x81000007;
        display.adr_clr_g = 0x81000003;
        
         display.data_clr_r = 0x04;
//        display.adr_clr_r = 0x810001f8;
        display.adr_clr_r = 0x810000f8;
        display.bTstON = TRUE;

}






//init screen to use 18 bit and 262k of colors
void Display_Init_18bit_262k_updownleftright( void)
{
	BYTE zero = 0;

	CLR_RS;
	DisplayData = 0x23;
	SET_RS;
	CLR_RS;
	DisplayData = 0x02;
	SET_RS;
	DisplayData = zero;
	DisplayData = zero;

	volatile BYTE* pData = (BYTE*)0x81000102;
	CLR_RS;
	DisplayData = 0x03;
	SET_RS;
	*pData = 0x31;

}



void Display_Init_18bit_262k_tst( unsigned int adr, unsigned char data)
{
	BYTE zero = 0;

	CLR_RS;
	DisplayData = 0x23;
	SET_RS;
	CLR_RS;
	DisplayData = 0x02;
	SET_RS;
	DisplayData = zero;
	DisplayData = zero;

	volatile BYTE* pData = (BYTE*)adr;
	CLR_RS;
	DisplayData = 0x03;
	SET_RS;
	*pData = data;

}

//init screen to use 18 bit and 262k of colors
void Display_Init_18bit_262k( void)
{
	BYTE zero = 0;

	CLR_RS;
	DisplayData = 0x23;
	SET_RS;
	CLR_RS;
	DisplayData = 0x02;
	SET_RS;
	DisplayData = zero;
//	DisplayData = zero;

	volatile BYTE* pData = (BYTE*)0x81000102;
//        volatile BYTE* pData = (BYTE*)0x81000080;
	CLR_RS;
	DisplayData = 0x03;
	SET_RS;
	*pData = 0x30;



}

//init screen to use 18 bit and 262k of colors
void Display_Init_18bit_262k_leftrightdownup( void)
{
	BYTE zero = 0;

	CLR_RS;
	DisplayData = 0x23;
	SET_RS;
	CLR_RS;
	DisplayData = 0x02;
	SET_RS;
	DisplayData = zero;
//	DisplayData = zero;

	volatile BYTE* pData = (BYTE*)0x81000102;
	CLR_RS;
	DisplayData = 0x03;
	SET_RS;
	*pData = 0x10;
}


//******************************
//
// »ниациализаци€ диспле€




//******************************
void Display_Init_8bit_262k( void)
{
	BYTE zero = 0;


	CLR_RS;
	DisplayData = 0x24;
	SET_RS;
	CLR_RS;
	DisplayData = 0x02;
	SET_RS;
	DisplayData = zero;
	DisplayData = zero;
	Display_set_display_entrymode(1 /*bRGBmode*/, 1/*bUpTodown*/);


}

//задать режим экрана
//BYTE bRGBmode =1 RGB режим (по байту на цвет), =0 B режим (только синий байт)
//BYTE bUpToDown =1 режим с верху вниз, =0 режим снизу вверх
void Display_set_display_entrymode(BOOL bRGBmode, BOOL bUpToDown)
{
	CLR_RS;
	DisplayData = 0x03;
	SET_RS;
	DisplayData = bRGBmode?0x41:0x81;
	DisplayData = bUpToDown?0x30:0x31;
}



//инициализаци€ дл€ LED
void Display_LED_Init(void)
{
	DIR_LED_G = 1;
	DIR_LED_R = 1;
	DIR_LED_O = 1;
	Display_turnOFF_LEDs();
}




void Display_turnOFF_LEDs(void)
{
	//turn off leds
	Display_turnOFFGreenLED();
	Display_turnOFFRedLED();
	Display_turnOFFOrangeLED();
}




//blink RED LED only for ms
//used for search mode when second proc is awaking first one
void Display_BlinkREDLED(DWORD ms)
{
	if(T2MCR_bit.MR3I)return;//already flashed
	if(ms<INTERPROC_TIMER_VAL)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"LED blink delay is out of range");
	}
	ms/=INTERPROC_TIMER_VAL;
	Display_turnONRedLED();
	T2MR3 = T2TC+ms;
	T2MCR_bit.MR3I = 1; //enable interrupt for LED
}



void Display_startup_LED(void)
{
	//маргнет LEDами
	Display_flashOrangeLED();
	Display_flashGreenLED();
	Display_flashRedLED();
}




void Display_flashGreenLED(void)
{
	if(PIN_LED_G)
		CLR_LED_G;
	else
		SET_LED_G;
}




void Display_flashRedLED(void)
{
	if(PIN_LED_R)
		CLR_LED_R;
	else
		SET_LED_R;
}


void Display_flashOrangeLED(void)
{
	if(PIN_LED_O)
		CLR_LED_O;
	else
		SET_LED_O;
}


void Display_turnONGreenLED(void)
{
	CLR_LED_G;
}


void Display_turnONRedLED(void)
{
	CLR_LED_R;
}


void Display_turnONOrangeLED(void)
{
	CLR_LED_O;
}


void Display_turnOFFGreenLED(void)
{
	SET_LED_G;
}


void Display_turnOFFRedLED(void)
{
	SET_LED_R;
}


void Display_turnOFFOrangeLED(void)
{
	SET_LED_O;
}








//just show color lines on the screen
void Display_warmup_display_start(void)
{
	Display_setTextSteps(1,1);//set steps
	Display_setTextWin(0,Y_SCREEN_MAX-216,X_SCREEN_SIZE,100);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextJustify(CENTER);
	Display_setTextDoubleHeight(0);

	Display_clearScreen();

	Display_setCurrentFont(fnt32x32);	//set current font
	Display_setTextColor(YELLOW);	//set text color
	Display_outputTextByLang("WARMUP\r\0""ANWARM\r\0""WARMUP\r\0""ѕ–ќ√–≈¬\r");	//"¬џ Ћ"
	Display_setCurrentFont(fnt16x16);	//set current font
	Display_outputTextByLang("IN PROGRESS\r\r\0""ERFULLUNG\r\r\0""IN PROGRESS\r\r\0""¬џѕќЋЌя≈“—я\r\r");	//"¬џ Ћ"
	Display_setTextColor(GREEN);	//set text color
	Display_setTextJustify(LEFT);
	Display_outputTextByLang("Please wait...\0""Bitte warten...\0""Please wait...\0""ѕожалуйста ждите...");	//"ќ“ѕ”—“»“≈  Ќќѕ ”"
	Display_drawHLine(0,Y_SCREEN_MAX-184,X_SCREEN_MAX, RED);
}











//just show color lines on the screen
void Display_startup_display_start(void)
{
	Display_clearScreen();
	Display_setTextWin(0,0,X_SCREEN_SIZE,Y_SCREEN_SIZE);	//set text window
	Display_setTextWrap(0);
		Display_startup_LED();
		Display_startup_display2(63);
		PowerControl_sleep(5);
		PowerControl_kickWatchDog();

	Display_setTextColor(LIME);	//set text color
	Display_setCurrentFont(fnt16x16);	//set current font
	Display_setTextXY(0,Y_SCREEN_SIZE-16);	//set start coords in window
	Display_outputTextByLang(txtVersion);	//"ќ“ѕ”—“»“≈  Ќќѕ ”"

}


//отображение текста в нужных позици€х
void Display_startup_display2(int i)
{
	Display_setTextXY(20,10);	//set start coords in window
	Display_setCurrentFont(fnt16x16);	//set current font
	Display_setTextDoubleHeight(0);
	Display_setTextSteps(2,3);//set steps
	Display_setTextJustify(LEFT);
	COLORREF clr = RGB(0,0,i);
	Display_setTextColor(clr);	//set text color
	Display_outputText("S");
	clr = RGB(0,0,i>32?32:i);
	Display_setTextColor(clr);	//set text color
	Display_setTextJustify(NONE);
	Display_outputText("pectrometric\r");
	clr = RGB(0,0,i);
	Display_setTextColor(clr);	//set text color
	Display_setTextJustify(LEFT);
	Display_outputText("P");
	clr = RGB(0,0,i>32?32:i);
	Display_setTextColor(clr);	//set text color
	Display_setTextJustify(NONE);
	Display_outputText("ersonal\r");
	clr = RGB(0,0,i);
	Display_setTextColor(clr);	//set text color
	Display_setTextJustify(LEFT);
	Display_outputText("R");
	clr = RGB(0,0,i>32?32:i);
	Display_setTextColor(clr);	//set text color
	Display_setTextJustify(NONE);
	Display_outputText("adiation\r");
	clr = RGB(0,0,i);
	Display_setTextColor(clr);	//set text color
	Display_setTextJustify(LEFT);
	Display_outputText("D");
	clr = RGB(0,0,i>32?32:i);
	Display_setTextColor(clr);	//set text color
	Display_setTextJustify(NONE);
	Display_outputText("etector");

	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt32x32);	//set current font
	Display_setTextDoubleHeight(0);
	Display_setTextSteps(1,1);//set steps
	clr = RGB(i,i,i);
	Display_setTextColor(clr);	//set text color
	Display_setTextXY(1,131);	//set start coords in window
#ifdef BNC
	Display_outputText("palmRAD");
#else
	Display_outputText("Rad");
#endif

	Display_drawHLine(0,163,X_SCREEN_MAX, clr);

	Display_setTextDoubleHeight(0);
	Display_setCurrentFont(fnt32x32);	//set current font
	Display_setTextSteps(1,1);//set steps
	clr = RGB(0,0,i);
	Display_setTextColor(clr);	//set text color
	Display_setTextXY(0,163);	//set start coords in window

#ifdef BNC
	Display_outputText("\r\r");
	Display_setCurrentFont(fnt16x16);	//set current font
	Display_outputText("Model 920\r\r");

	Display_outputText("BNC\r");
	Display_outputText("©2021\r");
#else
	Display_outputText("Searcher\r\r");
	Display_setCurrentFont(fnt16x16);	//set current font
	Display_outputText("AT1321\r\r");

	Display_outputText("ATOMTEX\r");
	Display_outputText("©2011-2021\r");
#endif
}








//show upper status line
void Display_showStatusLine(void)
{
  
	//must be run together
	Clock_showDateTime();
	InterProc_showTemperature();
	//
	BYTE min = clockData.dateTime.minute;
	int x1,x2,x3,x4,x5;
	switch(min&0x03)
	{
	case 0:
		x5=136;//L=16
		x1=152;//L=24
		x2=176;//L=16
		x3=192;//L=24
		x4=216;//L=24
		break;
	case 1:
		x4=136;//L=24
		x5=160;//L=16
		x1=176;//L=24
		x2=200;//L=16
		x3=216;//L=24
		break;
	case 2:
		x3=136;//L=24
		x4=160;//L=24
		x5=184;//L=16
		x1=200;//L=24
		x2=224;//L=16
		break;
	case 3:
		x2=136;//L=16
		x3=152;//L=24
		x4=176;//L=24
		x5=200;//L=16
		x1=216;//L=24
		break;
	}

	SoundControl_showSoundVibro(x5);
	filesystem_show_symbol(x1);
#ifndef GPS_BT_FREE
	Bluetooth_show_symbol(x2);	//L=16
	GPS_show_symbol(x3);		//L=24
#else
	{
		RECT rect = {x2,0,x2+15,16};
//		Display_clearRect(rect, 100);
	}
	{
		RECT rect = {x3,0,x3+23,16};
//		Display_clearRect(rect, 100);
	}
#endif	//#ifndef GPS_BT_FREE
	PowerControl_showBatStatus(x4);			//L=24
        
}


void Display_drawRect(int x1, int y1, int x2, int y2, COLORREF clr)
{
	Display_drawHLine(x1, y1, x2, clr);
	Display_drawHLine(x1, y2, x2, clr);
	Display_drawVLine(x1, y1, y2, clr);
	Display_drawVLine(x2, y1, y2, clr);
}


void Display_drawRect_xor(int x1, int y1, int x2, int y2, COLORREF clr)
{
	Display_drawHLine_xor(x1, y1, x2, clr);
	Display_drawHLine_xor(x1, y2, x2, clr);
	Display_drawVLine_xor(x1, y1, y2, clr);
	Display_drawVLine_xor(x2, y1, y2, clr);
}


//set text wrap
void Display_setTextLineClear(BOOL bClear)
{
	display.text.bTextLineClear = bClear;
}


//clear user part of screen (text area where MCS output)
void Display_clearUserPart(void)
{
	int sz = Display_getFontSizeY()*(display.text.bDoubleHeight?2:1)+display.text.stepY;
	while(display.text.winY+display.text.gstrY+sz*2 <= MODE_BUTTONS_TOP)
	{
		Display_outputText("\r");
	};
	if(display.text.winY+display.text.gstrY+sz <= MODE_BUTTONS_TOP)
	{//have some space to clear
		display.text.gstrY -= sz-(MODE_BUTTONS_TOP-(display.text.winY+display.text.gstrY+sz));
		Display_outputText("\r");
	}
}


//get screen in file
void Display_getScreen(void)
{
	Display_Init_8bit_262k();


	BYTE buf[3*256];
	COLORREF clr;
	HFILE hfile = filesystem_create_file("screen","bmp",TRUE);
	if(hfile==NULL)return;	//error
	unsigned int i=0;	//buf index
	int pos = 0;	//position in file

	//prepare headers and save them to file
	BITMAPFILEHEADER bmpfilehdr;
	memset(&bmpfilehdr, 0, sizeof(BITMAPFILEHEADER));
	bmpfilehdr.bfType = ((WORD)('M')<<8)|(WORD)('B');
	bmpfilehdr.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	if(filesystem_file_put(hfile, &pos, (const BYTE*)&bmpfilehdr, sizeof(BITMAPFILEHEADER))==E_FAIL)return;	//error

	BITMAPINFOHEADER bmpinfohdr;
	memset(&bmpinfohdr, 0, sizeof(BITMAPINFOHEADER));
	bmpinfohdr.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfohdr.biWidth = X_SCREEN_SIZE;
	bmpinfohdr.biHeight = -Y_SCREEN_SIZE;
	bmpinfohdr.biPlanes = 1;
	bmpinfohdr.biBitCount = 24;
	bmpinfohdr.biCompression = 0;
	bmpinfohdr.biSizeImage = X_SCREEN_SIZE*Y_SCREEN_SIZE*3;
	bmpinfohdr.biXPelsPerMeter = 0;
	bmpinfohdr.biYPelsPerMeter = 0;
	bmpinfohdr.biClrUsed = 0;
	bmpinfohdr.biClrImportant = 0;
	if(filesystem_file_put(hfile, &pos, (const BYTE*)&bmpinfohdr, sizeof(BITMAPINFOHEADER))==E_FAIL)return;	//error

	PowerControl_turboModeON();

	for(int y=0;y<Y_SCREEN_SIZE;y++)
	{
		for(int x=0;x<X_SCREEN_SIZE;x++)
		{
			clr = Display_read_dot(x,y);
			buf[i++]=LOBYTE(clr);
			buf[i++]=LO2BYTE(clr);
			buf[i++]=LO3BYTE(clr);
			if(i==sizeof(buf))
			{//save in a file
				if(filesystem_file_put(hfile, &pos, buf, i)==E_FAIL)
				{
					i=0;
					break;	//error
				}
				i = 0;	//start fill buf from beginning
			}
		}
	}

	PowerControl_turboModeOFF();

	if(i!=0)
	{//save last part
		if(filesystem_file_put(hfile, &pos, buf, i)==E_FAIL)return;	//error
	}

}
