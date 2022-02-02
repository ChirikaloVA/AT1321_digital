//display2.c

//large procedures
#include "display.h"
#include "interrupts.h"
#include "sound.h"


#pragma optimize=speed

void Display_fillRect(RECT rect, COLORREF clr)
{
	volatile BYTE* pData = (BYTE*)0x81000000;
	
	DWORD len = (rect.right-rect.left+1)*(rect.bottom-rect.top+1);
	if(rect.left>=X_SCREEN_SIZE)rect.left=X_SCREEN_SIZE-1;
	if(rect.top>=Y_SCREEN_SIZE)rect.top=Y_SCREEN_SIZE-1;
	if(rect.right>=X_SCREEN_SIZE)rect.right=X_SCREEN_SIZE-1;
	if(rect.bottom>=Y_SCREEN_SIZE)rect.bottom=Y_SCREEN_SIZE-1;
	if(len==0)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"RECT height*width must be >0");
	}
	
	Display_Init_8bit_262k();
	
	Display_set_clip_region(rect.left,rect.top,rect.right,rect.bottom);
	Display_set_screen_memory_adr(rect.left,rect.top);

	Display_Init_18bit_262k();

	CLR_RS;
	DisplayData = 0x22;
	SET_RS;
        BYTE c1=LOBYTE(clr),c2=LO2BYTE(clr),c3=LO3BYTE(clr);

        DWORD  hlen = HIBYTE_W(len);
        DWORD  llen = LOBYTE(len);

		WORD wrd;
		c1 = c1>>1;
		c1 |= (c2<<5)&0xe0;
		wrd = c3;
		pData+=(WORD)((wrd<<3)&0x1f8)|(WORD)(c2>>3);

        if(hlen>0)
        {
          do
          {
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
			  *pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;*pData = c1;
          }while(--hlen);
        }

        if(llen>0)
        {
          do
          {
			  *pData=c1;
          }while(--llen);
        }

	Display_Init_8bit_262k();
}



















#pragma optimize=speed

//if vsync = 0xffff then no wait of sync
void Display_clearRect(RECT rect, int vsync)
{
	volatile BYTE* pData = (BYTE*)0x81000000;

	DWORD len = (rect.right-rect.left+1)*(rect.bottom-rect.top+1);
	if(rect.left>=X_SCREEN_SIZE)rect.left=X_SCREEN_SIZE-1;
	if(rect.top>=Y_SCREEN_SIZE)rect.top=Y_SCREEN_SIZE-1;
	if(rect.right>=X_SCREEN_SIZE)rect.right=X_SCREEN_SIZE-1;
	if(rect.bottom>=Y_SCREEN_SIZE)rect.bottom=Y_SCREEN_SIZE-1;
	if(len==0)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"RECT height*width must be >0");
	}
		
	//catch VSYNC
	Display_Init_8bit_262k();

	Display_set_clip_region(rect.left,rect.top,rect.right,rect.bottom);
	Display_set_screen_memory_adr(rect.left,rect.top);
	
	
	
	DWORD  hlen = HIBYTE_W(len);
	DWORD  llen = LOBYTE(len);
	
	
	
	
	//init to 18 bit to clean screen three time faster
	Display_Init_18bit_262k();
	Display_catchVSYNC(vsync);
	
	
	CLR_RS;
	DisplayData = 0x22;
	SET_RS;
	
	if(hlen>0)
	{
		do
		{
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;*pData = 0;
			
		}while(--hlen);
	}
	
	
	if(llen>0)
	{
		do
		{
			*pData = 0;
		}while(--llen);
	}
	
	
	Display_Init_8bit_262k();
}









#pragma optimize=speed


//prepare for interrupt emergeny text output
__arm void Display_prepareEmergencyTextControl(void)
{
	Display_turnON();
	Display_setCurrentFont(fnt16x16);	//set current font
	Display_setTextColor(RED);	//set text color
	Display_setTextSteps(1,1);//set steps
	Display_setTextWin(0,0,X_SCREEN_SIZE,Y_SCREEN_SIZE);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextLineClear(0);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_clearTextWin(100);
}
#pragma optimize=speed
__arm void Display_printHEX(BYTE value)
{
	BYTE v;
	char buff[3];
	v = ((value>>4)&0x0f)+'0';
	if(v>'9')v += 7;
	buff[0] = v;
	v = (value&0x0f)+'0';
	if(v>'9')v += 7;
	buff[1] = v;
	buff[2]='\0';
	Display_outputText(buff);
}
#pragma optimize=speed
__arm void Display_printR(DWORD r14)
{
	Display_printHEX(HIBYTE(r14));
	Display_printHEX(LO3BYTE(r14));
	Display_printHEX(LO2BYTE(r14));
	Display_printHEX(LOBYTE(r14));
}


#pragma optimize=speed
//draw line
void Display_drawLine(int x1, int y1, int x2, int y2, COLORREF clr)
{
	int e,denom,yinc = 1;
	if(x2<x1)
	{//�������������� ��������� ����� �� �����
		e=x2;x2=x1;x1=e;
		e=y2;y2=y1;y1=e;
	}
	x2 = x2-x1; y2 = y2-y1;
	if(y2<0){yinc = -1; y2 = -y2;}
	if(y2>x2)
	{
		e=x2; x2=y2; y2=e;
		denom = x2<<1;
		y2 = y2<<1;
		e = -x2;
		while(x2-->=0)
		{
			Display_dot(x1, y1, clr);
			if((e+=y2)>0)
			{
				x1++;
				e-=denom;
			}
			y1+=yinc;
		};
	}else
	{
		denom = x2<<1;
		y2 = y2<<1;
		e = -x2;
		while(x2-->=0)
		{
			Display_dot(x1, y1, clr);
			if((e+=y2)>0)
			{
				y1+=yinc;
				e-=denom;
			}
			x1++;
		};
	}
}
#pragma optimize=speed
void Display_drawLine_xor(int x1, int y1, int x2, int y2, COLORREF clr)
{
	int e,denom,yinc = 1;
	if(x2<x1)
	{//�������������� ��������� ����� �� �����
		e=x2;x2=x1;x1=e;
		e=y2;y2=y1;y1=e;
	}
	x2 = x2-x1; y2 = y2-y1;
	if(y2<0){yinc = -1; y2 = -y2;}
	if(y2>x2)
	{
		e=x2; x2=y2; y2=e;
		denom = x2<<1;
		y2 = y2<<1;
		e = -x2;
		while(x2-->=0)
		{
			Display_dot_xor(x1, y1, clr);
			if((e+=y2)>0)
			{
				x1++;
				e-=denom;
			}
			y1+=yinc;
		};
	}else
	{
		denom = x2<<1;
		y2 = y2<<1;
		e = -x2;
		while(x2-->=0)
		{
			Display_dot_xor(x1, y1, clr);
			if((e+=y2)>0)
			{
				y1+=yinc;
				e-=denom;
			}
			x1++;
		};
	}
}

//#pragma optimize=speed
//draw vert line
void Display_drawVLine(int x, int y1, int y2, COLORREF clr)
{
	volatile BYTE* pData = (BYTE*)0x81000000;
	int y;
	if(y1>y2)
	{
		y=y1;
		y1=y2;
		y2=y;
	}

	
	Display_Init_8bit_262k();
	
	Display_set_clip_region(x,y1,x,y2);
	Display_set_screen_memory_adr(x,y1);

	DWORD len = y2-y1+1;
	
	Display_Init_18bit_262k();
	BYTE c1=LOBYTE(clr),c2=LO2BYTE(clr),c3=LO3BYTE(clr);

	DWORD hlen = len>>4;
	DWORD llen = len&0xf;

	WORD wrd;
	c1 = c1>>1;
	c1 |= (c2<<5)&0xe0;
	wrd = c3;
	pData+=(WORD)((wrd<<3)&0x1f8)|(WORD)(c2>>3);

	
	CLR_RS;
	DisplayData = 0x22;
	SET_RS;
	
	if(hlen>0)
	{
		do
		{
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
		}while(--hlen);
	}
	
	if(llen>0)
	{
		do
		{
			*pData = c1;
		}while(--llen);
	}
	
	Display_Init_8bit_262k();
}


#pragma optimize=speed
//draw horz line
void Display_drawHLine(int x1, int y, int x2, COLORREF clr)
{
	volatile BYTE* pData = (BYTE*)0x81000000;
	int x;
	if(x1>x2)
	{
		x=x1;
		x1=x2;
		x2=x;
	}

	Display_Init_8bit_262k();
	
	
	Display_set_clip_region(x1,y,x2,y);
	Display_set_screen_memory_adr(x1,y);

	DWORD len = x2-x1+1;
	
	Display_Init_18bit_262k();
	
	BYTE c1=LOBYTE(clr),c2=LO2BYTE(clr),c3=LO3BYTE(clr);

	WORD wrd;
	c1 = c1>>1;
	c1 |= (c2<<5)&0xe0;
	wrd = c3;
	pData+=(WORD)((wrd<<3)&0x1f8)|(WORD)(c2>>3);
	
	
	DWORD hlen = len>>4;
	DWORD llen = len&0xf;

	CLR_RS;
	DisplayData = 0x22;
	SET_RS;

	if(hlen>0)
	{
		do
		{
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
			*pData = c1;*pData = c1;
		}while(--hlen);
	}
	
	if(llen>0)
	{
		do
		{
			*pData = c1;
		}while(--llen);
	}
	Display_Init_8bit_262k();
}


#pragma optimize=speed
//draw vert line by xor
void Display_drawVLine_xor(int x, int y1, int y2, COLORREF clr)
{
	int y;
	if(y1>y2)
	{
		y=y1;
		y1=y2;
		y2=y;
	}
	
	
	Display_Init_8bit_262k();
	
	Display_set_clip_region(x,y1,x,y2);
	Display_set_screen_memory_adr(x,y1);
	
	DWORD len = y2-y1+1;
	DWORD hlen = len>>4;
	DWORD llen = len&0xf;

	COLORREF volatile * pBuf = &display.arReadOut[0];

	
	CLR_RS;
	DisplayData = 0x22;
	SET_RS;
	
	BYTE b1 = DisplayData;
	b1 = DisplayData;
	b1 = DisplayData;

	if(hlen>0)
	{
		do
		{
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
		}while(--hlen);
	}
	
	if(llen>0)
	{
		do
		{
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
		}while(--llen);
	}



	Display_set_screen_memory_adr(x,y1);

	pBuf = &display.arReadOut[0];
	
	
	CLR_RS;
	DisplayData = 0x22;
	SET_RS;
	BYTE c1=LOBYTE(clr),c2=LO2BYTE(clr),c3=LO3BYTE(clr);

	hlen = len>>4;
	llen = len&0xf;


	if(hlen>0)
	{
		do
		{
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
		}while(--hlen);
	}
	
	if(llen>0)
	{
		do
		{
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
		}while(--llen);
	}

	Display_Init_18bit_262k();

}






#pragma optimize=speed

//draw horz line by xor
void Display_drawHLine_xor(int x1, int y, int x2, COLORREF clr)
{
	int x;
	if(x1>x2)
	{
		x=x1;
		x1=x2;
		x2=x;
	}

	Display_Init_8bit_262k();

	Display_set_clip_region(x1,y,x2,y);
	Display_set_screen_memory_adr(x1,y);
	
	DWORD len = x2-x1+1;
	DWORD hlen = len>>4;
	DWORD llen = len&0xf;

	COLORREF volatile * pBuf = &display.arReadOut[0];
	
	
	CLR_RS;
	DisplayData = 0x22;
	SET_RS;
	
	BYTE b1 = DisplayData;
	b1 = DisplayData;
	b1 = DisplayData;

	if(hlen>0)
	{
		do
		{
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
		}while(--hlen);
	}
	
	if(llen>0)
	{
		do
		{
			*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
		}while(--llen);
	}



	Display_set_screen_memory_adr(x1,y);

	pBuf = &display.arReadOut[0];


	
	CLR_RS;
	DisplayData = 0x22;
	SET_RS;
	BYTE c1=LOBYTE(clr),c2=LO2BYTE(clr),c3=LO3BYTE(clr);

	hlen = len>>4;
	llen = len&0xf;


	if(hlen>0)
	{
		do
		{
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
		}while(--hlen);
	}
	
	if(llen>0)
	{
		do
		{
			DisplayData = c3 ^ *((BYTE*)pBuf+2);DisplayData = c2 ^ *((BYTE*)pBuf+1);DisplayData = c1 ^ *((BYTE*)pBuf+0); pBuf++;
		}while(--llen);
	}
	Display_Init_18bit_262k();

}


















//#pragma optimize=speed

//scrolling screen
__arm void Display_left_scroll(int x1, int y1, int x2, int y2, int step)
{
	int y, x;
        SET_ISD_INT;
	if(y1>y2)
	{
		y=y1;
		y1=y2;
		y2=y;
	}
	if(x1>x2)
	{
		x=x1;
		x1=x2;
		x2=x;
	}
	
	Display_Init_8bit_262k();
	
	while(x1<=x2)
	{
	
		x = x1;
		
		Display_set_clip_region(x,y1,x,y2);
		Display_set_screen_memory_adr(x,y1);
		
		DWORD len = y2-y1+1;
		DWORD hlen = len>>4;
		DWORD llen = len&0xf;
	
		COLORREF volatile * pBuf = &display.arReadOut[0];
		
		
		CLR_RS;
		DisplayData = 0x22;
		SET_RS;
		
		BYTE b1 = DisplayData;
		b1 = DisplayData;
		b1 = DisplayData;
	
		if(hlen>0)
		{
                  
			do
			{
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			}while(--hlen);
                        
		}
		
		if(llen>0)
		{
			do
			{
				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			}while(--llen);
		}
	
	
	
		x -= step;
		Display_set_clip_region(x,y1,x,y2);
		Display_set_screen_memory_adr(x,y1);
	
		pBuf = &display.arReadOut[0];

		
		CLR_RS;
		DisplayData = 0x22;
		SET_RS;
	
		hlen = len>>4;
		llen = len&0xf;
	
	
		if(hlen>0)
		{
                  
			do
			{
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
			}while(--hlen);
                        
		}
		
		if(llen>0)
		{
			do
			{
				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
			}while(--llen);
		}

		x1++;
		
	};	//while

	Display_Init_18bit_262k();
        CLR_ISD_INT;
}

__arm void Display_left_scroll_new(int x1, int y1, int x2, int y2, int step)
{
	int y, x;
        BYTE* pData_b;
        BYTE* pData_g;
        BYTE* pData_r;
        
        SET_ISD_INT;
        pData_b = (BYTE*)display.adr_clr_b;
        pData_g = (BYTE*)display.adr_clr_g;
        pData_r = (BYTE*)display.adr_clr_r;
	if(y1>y2)
	{
		y=y1;
		y1=y2;
		y2=y;
	}
	if(x1>x2)
	{
		x=x1;
		x1=x2;
		x2=x;
	}
	
	
	
	while(x1<=x2)
	{
                Display_Init_8bit_262k();
		x = x1;
		
		Display_set_clip_region(x,y1,x,y2);
		Display_set_screen_memory_adr(x,y1);
		
		DWORD len = y2-y1+1;
		DWORD hlen = len>>4;
		DWORD llen = len&0xf;
	
		COLORREF volatile * pBuf = &display.arReadOut[0];
		
//		Display_Init_18bit_262k_updownleftright();
                Display_Init_18bit_262k_tst( display.adr, display.data);
                CLR_RS;
                
//                SET_RS;
//		CLR_RS;
		DisplayData = 0x22;
//                DisplayData = 0x23;
		SET_RS;
		
		BYTE b1 = DisplayData;
		b1 = DisplayData;
		b1 = DisplayData;
	
		if(hlen>0)
		{
                  
			do
			{
                          b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                           b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
                          
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			}while(--hlen);
                        
		}
		
		if(llen>0)
		{
			do
			{
                          b1 = DisplayData;
                          if(b1 == 0x00)
                          {
                            *(pBuf) = 0;
                          }
                          else if(b1 == 0x0c)
                          {
                            *(pBuf) = 0x00FC000C;
                          }
                          else if(b1 == 0x10)
                          {
                            *(pBuf) = 0x0000FC10;
                          }
                          pBuf++;
//				*((BYTE*)pBuf+2) = DisplayData;	*((BYTE*)pBuf+1) = DisplayData;	*((BYTE*)pBuf+0) = DisplayData; pBuf++;
			}while(--llen);
		}
	
	
	
		x -= step;
                Display_Init_8bit_262k();
		Display_set_clip_region(x,y1,x,y2);
		Display_set_screen_memory_adr(x,y1);
//                Display_Init_18bit_262k_updownleftright();
                Display_Init_18bit_262k_tst( display.adr, display.data);
                
		pBuf = &display.arReadOut[0];

		
		CLR_RS;
                
		DisplayData = 0x22;
//                DisplayData = 0x23;
		SET_RS;
	
		hlen = len>>4;
		llen = len&0xf;
	
	
		if(hlen>0)
		{
                  
			do
			{
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          if(*(pBuf) == 0)                      //Bla
                          {
                            *(pData_b) = display.data_clr_b;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            *(pData_r) = display.data_clr_r;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            *(pData_g) = display.data_clr_g;
                          }
                          pBuf++;
                          
                          
                          
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
			}while(--hlen);
                        
		}
		
		if(llen>0)
		{
			do
			{
                          if(*(pBuf) == 0)                      //Bla
                          {
                            DisplayData = 0;
//                            DisplayData_G = 0x10;
                          }
                          else if(*(pBuf) == 0x00FC000C)        //R
                          {
                            DisplayData_R = 0x0c;
                          }
                          else if(*(pBuf) == 0x0000FC10)        //G
                          {
                            DisplayData_G = 0x10;
                          }
                          pBuf++;
//				DisplayData = *((BYTE*)pBuf+2);DisplayData = *((BYTE*)pBuf+1);DisplayData = *((BYTE*)pBuf+0); pBuf++;
			}while(--llen);
		}

		x1++;
		
	};	//while

	Display_Init_18bit_262k();
        CLR_ISD_INT;
}

