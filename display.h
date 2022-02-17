#ifndef _DISPLAY_H
#define _DISPLAY_H


#include <iolpc2388.h>
#include "types.h"



#define X_SCREEN_SIZE 240
#define Y_SCREEN_SIZE 320
#define X_SCREEN_MAX X_SCREEN_SIZE-1
#define Y_SCREEN_MAX Y_SCREEN_SIZE-1



//!!!!!!!!! ������ bmp ������ ���� ������� 4

extern const BYTE bmp_bat0[];
extern const BYTE bmp_bat1[];
extern const BYTE bmp_bat2[];
extern const BYTE bmp_bat3[];
extern const BYTE bmp_bat4[];

extern const BYTE bmp_bth[];
extern const BYTE bmp_gps[];
extern const BYTE bmp_memlow[];
extern const BYTE bmp_sound[];
extern const BYTE bmp_vibro[];
extern const BYTE bmp_silent[];

//colors////////////////////////
#define WHITE RGB(63,63,63)
#define BLACK RGB(0,0,0)
#define GREEN RGB(0,63,0)
#define GREEN_N RGB(0,63,0)
#define GREEN_DARK RGB(0,32,0)
#define BLUE RGB(0,0,63)
#define RED RGB(63,0,0)
#define RED_N RGB(63,00,4)
#define RED_DARK RGB(32,0,0)

#define ORANGE RGB(63,32,0)
#define ORANGE_DARK RGB(32,16,0)
#define BROWN RGB(32,63,0)
#define BROWN_DARK RGB(16,32,0)
#define BROWN_DARK_TST RGB(16,32,0)
#define BROWN_DARK_TST1 RGB(16,32,4)


#define YELLOW RGB(63,63,0)
#define YELLOW_DARK RGB(32,32,0)

#define LIME RGB(0,63,63)
#define PURPLE RGB(63,0,63)

////////////////////////////////

#pragma pack(1)

/*
bfType � ��� �����, ������� 'BM'.
bfSize � ������ ����� � ������.
bfReserved1 �
bfReserved2 � ���������������, �������� ����.
bfOffBits � �������� �������� � ������ �� ������ ��������� BITMAPFILEHEADER �� ��������������� ����� �����������.
*/

typedef struct tagBITMAPFILEHEADER
{
  WORD    bfType;
  DWORD   bfSize;
  WORD    bfReserved1;
  WORD    bfReserved2;
  DWORD   bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;


/*
biSize � ������ ��������� � ������
biWidth � ������ ����������� � ��������. ��� Win98/Me � Win2000/XP: ���� ���� biCompression �������� BI_JPEG ��� BI_PNG, ����� ������� ������ �������������� �����������.
biHeight � ������ ����������� � ��������. ���� �������� ������������� �������� � ����������� �������� � ������� ����� �����, ������� ������� � ������ ����� ����. ���� �������� ������������� � ����������� �������� ������ ����, ������� ������� � ������� ����� ���� �����������. ���� biCompression ������ ��������� �������� BI_RGB ��� BI_BITFIELDS. ����� ����������� �� ����� ���� �����.
biPlanes � �������� �������.
biBitCount � ��������� ���������� ��� �� �������. ����� ��������� ��������� ��������:
0 � ����� ����� ��� Win98/Me/2000/XP/. ����� ��� �� ������� ���������� ������ JPEG ��� PNG.
1 � ����������� �����������. ���� bmiColors ��������� BITMAPINFO �������� ��� ��������. ������ ��� ����������� ������������ ���� �������; ���� ��� ����� ���� � ������� ����� ���� ������� �������� ������� bmiColors, ����� � ���� �������.
4 � ������������������ �����������. ������� ������������ 4-� ������� ���������, ������ ���� ����������� �������� ���������� � ���� �������� � ������� 4 ���� ��� �������, ���������� � ��� �������.
8 � � ������� ���������� �� 256 ������, ������ ���� ����������� ������ ������ � ������� ��� ������ �������.
16 � ���� ���� biCompression �������� �������� BI_RGB, ���� �� �������� �������. ������ ��� ����� ����������� ������ ������������� �������, ������ � ����� ���������� ������ �������. ��� ���� ������� ��� �� ������������, �� ������ ���������� �������� 5 ���: 0RRRRRGGGGGBBBBB.
���� ���� biCompression �������� �������� BI_BITFIELDS, ������� ������ ��� ����������� ��������, ������������ ����� ��� ������ �� ��� ��������� �����. ������ ������� ����������� ����������� ����������� ���������, �� �������� � ������� ����� ����������� �������� ����������. ��� WinNT/2000/XP � ������������������ ��� ������ ���������� ������ ��������� ����������, �� ������������ � �� ����������� � �������������������� ������ ���������. ��� Win95/98/Me � �������������� ������ ��������� �����: 5-5-5, ��� ����� ����� ���������� 0�001F, ������ 0x03E0, ������� 0x7C00; � 5-6-5, ��� ����� ����� ���������� 0x001F, ������ 0x07E0, ������� 0xF800.
24 � ������� �� ������������, ������ ������ ���� ����������� ������������ ���� �������, �� ����� ��� ������������� ������, ������� � �������� ������ ��������������.
32 � ���� ���� biCompression �������� �������� BI_RGB, ����������� �� �������� �������. ������ ������ ����� ����������� ������������ ���� �������, �� ����� ��� ������������� ������, ������� � �������� ������ ��������������. ������� ���� ������ ������� �� ������������.
���� ���� biCompression �������� �������� BI_BITFIELDS, � ������� �������� ��� ������������� �������� ����� � ��� �������, ������ � ����� ����������. ������ ������� ����������� ����������� �������� �������. WinNT/2000: ����� ��������� �� ������ ������������� ��� ������������. Windows 95/98/Me: ������� ������������ ������ ���� ����� ������, ��������� ����������� ������ ��� ���������� BI_RGB � ������� ���� ������ ������� �� ������������, ��������� ��� �������� ��� ������, ������� � �������� ������ ��������������.
biCompression � ��������� ��� ������ ��� ������ �����������:
��������	�������������	����������
0	BI_RGB	�������� �����������
1	BI_RLE8	������ RLE ��� ������������ �����������
2	BI_RLE4	������ RLE ��� ������������ �����������
3	BI_BITFIELDS	����������� �� �����, ������� �������� ��� ������������� ����� ��� �������, ������ � ����� ��������� �����. ������������ ��� 16 � 32-������ �����������
4	BI_JPEG	Win98/Me/2000/XP: JPEG-������
5	BI_PNG	Win98/Me/2000/XP: PNG-������
biSizeImage � ��������� ������ ����������� � ������. ����� ��������� ���� ��� BI_RGB-�����������. Win98/Me/2000/XP: ���� biCompression �������� BI_JPEG ��� BI_PNG, biSizeImage ��������� ������ BI_JPEG ��� BI_PNG ������ �����������.
biXPelsPerMeter � ��������� �������������� ���������� � �������� �� ���� ��� �������� ����������. ���������� ����� ������������ ��� �������� ��� ������ �� ������ �������� �����������, �������� ����������� ��� �������� ����������.
biYPelsPerMeter � ��������� ������������ ���������� � �������� �� ���� ��� �������� ����������.
biClrUsed � ��������� ���������� ������������ �������� �������� � �������. ���� �������� ����� ���� � ����������� ���������� ����������� ��������� ���������� ��������, � ������������ �� ��������� biBitCount � ������� ������, ��������� � biCompression.
���� �������� ��������� �������� � biBitCount ������ 16, biClrUsed ��������� ���������� ������, � ������� ����� ���������� ������� ���������� ��� ����������. ���� biBitCount ������ ��� ����� 16, biClrUsed ��������� ������ �������, ������������ ��� ����������� ������ ��������� ������. ���� biBitCount ����� 16 ��� 32, ����������� ������� ������� ����� ����� ��� ������������� ����. � ����������� ����������� ������ �������� ������� ����� ����� ��������� BITMAPINFO, biClrUsed ������ ��������� ����, ���� �������� ������ �������.
biClrImportant � ��������� ���������� ��������, ����������� ��� ����������� �����������. ���� �������� ���� � ��� ������� ��������� �����.
����������: ��������� BITMAPINFO ���������� BITMAPINFOHEADER � �������, ������������ ������ �������� �������� � ������ �����������.
����� ����� ������� � ��������� BITMAPINFO, ���������� ������ ������������ ����������, �������� � biSize, ��������� �������:

������������������ ��������, ���������� � ��� ��� ���� ����. ������� �������� ���������, ����� �����.
������ ������ ����������� ����������� ������ �� �����, ������� ������ ������.

*/

typedef struct tagBITMAPINFOHEADER{
  DWORD  biSize;
  LONG   biWidth;
  LONG   biHeight;
  WORD   biPlanes;
  WORD   biBitCount;
  DWORD  biCompression;
  DWORD  biSizeImage;
  LONG   biXPelsPerMeter;
  LONG   biYPelsPerMeter;
  DWORD  biClrUsed;
  DWORD  biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;






#pragma pack()



enum ENUM_TEXT_JUSTIFY
{
	NONE=0x10000,
	LEFT,
	CENTER,
	RIGHT
};


//display data structures///////////////////////////////////
struct tagTextData
{
	int gstrX;//text position in window coordinates
	int gstrY;
	int stepY;//steps for Y and X position changing (these are addons for symbol size)
	int stepX;
	int winX;//text output window
	int winY;
	int winSX;//window size
	int winSY;
	BOOL bDoubleHeight;	//if 1 then symbols in twice larger
	BOOL bWrap;	//if 1 then symbol wrapping on next line esle no
	BOOL bTextLineClear;	//if 1 then every new line of text will be cleared before displaying
	enum ENUM_TEXT_JUSTIFY justify;	//text justify on screen
	COLORREF color;
	const BYTE* pFont;
};

struct tagDisplayData
{
  BOOL bLCDON;
  struct tagTextData text;
  COLORREF arReadOut[Y_SCREEN_SIZE];
  unsigned int test_var;
  unsigned int adr;
  unsigned char data;
  
  unsigned int adr_clr_b;
  BYTE data_clr_b;
  
  unsigned int adr_clr_g;
  BYTE data_clr_g;
  
  unsigned int adr_clr_r;
  BYTE data_clr_r;
  
  BYTE tst_rd;
  COLORREF tst_clr;
  COLORREF volatile * pBuf;
  unsigned int test_var1;
  BOOL bTstON;
  BOOL bTst1;
  
};

extern struct tagDisplayData display;

//////////////////////////////////////////////






extern const BYTE fnt6x16[];
extern const BYTE fnt8x16[];
extern const BYTE fnt16x16[];
extern const BYTE fnt32x32[];


void Display_startup_display_start(void);
//void Display_startup_display_end(void);
void Display_LED_Init(void);
void Display_startup_LED(void);



void Display_setTextJustify(enum ENUM_TEXT_JUSTIFY justify);

int Display_getRightStartPos(const char* pText);
int Display_getLeftStartPos(const char* pText);

int Display_getWordLen(const char* pText);

void Display_setTextWrap(BOOL wrap);
void Display_setTextDoubleHeight(BOOL bDoubleHeight);
void Display_setTextSteps(int stepX, int stepY);
void Display_setTextXY(int x, int y);
void Display_setTextWin(int X,int Y,int SX,int SY);

//increment of gstrX of text functions
int Display_inc_textX(int x);

UINT Display_getSymWidth(char symbol);

const BYTE* Display_getSymFontOffset(char symbol);
UINT Display_getFontSizeY(void);
const BYTE* Display_getCurrentFont(void);
void Display_setCurrentFont(const BYTE* pFont);
int Display_showSymbol(char symbol);

void Display_setTextColor(COLORREF clr);

BOOL Display_gotoNextLine(void);

//����� ������
void Display_outputText(const char* pText);

char* Display_getClockDateTimeStr(char* pStr);

void Display_setTextLineClear(BOOL bClear);



//display driver
//31/10/2008 Bystrov

//===== ������� ==================
#ifdef FAST_PORT_ON
//resetb .Reste pin initializes the IC when low. Should be reset after power on
#define DIR_NRES FIO1DIR_bit.P1_19
#define SET_NRES MY_FIO1SET(B_19)
#define CLR_NRES MY_FIO1CLR(B_19)

//register select 0-Index/status, 1-instruction parameter/GRAM data
#define DIR_RS FIO1DIR_bit.P1_14
#define SET_RS FIO1SET_bit.P1_14=1
#define CLR_RS FIO1CLR_bit.P1_14=1
//#define SET_RS MY_FIO1SET(B_14)
//#define CLR_RS MY_FIO1CLR(B_14)


//Select the CPU interface mode. (0=paralel, 1-serial)
#define DIR_SPB FIO1DIR_bit.P1_1
#define SET_SPB MY_FIO1SET(B_1)
#define CLR_SPB MY_FIO1CLR(B_1)

//select the CPU type. 0-intel 80, 1-intel 68
#define DIR_ID_MIB FIO1DIR_bit.P1_4
#define SET_ID_MIB MY_FIO1SET(B_4)
#define CLR_ID_MIB MY_FIO1CLR(B_4)

// RGB pin
#define DIR_VSYNC FIO1DIR_bit.P1_15
#define SET_VSYNC FIO1SET_bit.P1_15
#define CLR_VSYNC FIO1CLR_bit.P1_15

#define DIR_HSYNC FIO1DIR_bit.P1_16
#define SET_HSYNC FIO1SET_bit.P1_16
#define CLR_HSYNC FIO1CLR_bit.P1_16

#define DIR_DOTCLK FIO1DIR_bit.P1_17
#define SET_DOTCLK FIO1SET_bit.P1_17
#define CLR_DOTCLK FIO1CLR_bit.P1_17

// SPI
#define DIR_SDOUT FIO0DIR_bit.P0_12
#define SET_SDOUT FIO0SET_bit.P0_12
#define CLR_SDOUT FIO0CLR_bit.P0_12

#define DIR_SDIN FIO0DIR_bit.P0_13
#define SET_SDIN FIO0SET_bit.P0_13
#define CLR_SDIN FIO0CLR_bit.P0_13


#else

//resetb .Reste pin initializes the IC when low. Should be reset after power on
#define DIR_NRES IO1DIR_bit.P1_19
#define SET_NRES MY_IO1SET(B_19)
#define CLR_NRES MY_IO1CLR(B_19)

//register select 0-Index/status, 1-instruction parameter/GRAM data
#define DIR_RS IO1DIR_bit.P1_14
#define SET_RS IO1SET_bit.P1_14=1
#define CLR_RS IO1CLR_bit.P1_14=1
//#define SET_RS MY_FIO1SET(B_14)
//#define CLR_RS MY_FIO1CLR(B_14)


//Select the CPU interface mode. (0=paralel, 1-serial)
#define DIR_SPB IO1DIR_bit.P1_1
#define SET_SPB MY_IO1SET(B_1)
#define CLR_SPB MY_IO1CLR(B_1)

//select the CPU type. 0-intel 80, 1-intel 68
#define DIR_ID_MIB IO1DIR_bit.P1_4
#define SET_ID_MIB MY_IO1SET(B_4)
#define CLR_ID_MIB MY_IO1CLR(B_4)

// RGB pin
#define DIR_VSYNC IO1DIR_bit.P1_15
#define SET_VSYNC IO1SET_bit.P1_15
#define CLR_VSYNC IO1CLR_bit.P1_15

#define DIR_HSYNC IO1DIR_bit.P1_16
#define SET_HSYNC IO1SET_bit.P1_16
#define CLR_HSYNC IO1CLR_bit.P1_16

#define DIR_DOTCLK IO1DIR_bit.P1_17
#define SET_DOTCLK IO1SET_bit.P1_17
#define CLR_DOTCLK IO1CLR_bit.P1_17

// SPI
#define DIR_SDOUT IO0DIR_bit.P0_12
#define SET_SDOUT IO0SET_bit.P0_12
#define CLR_SDOUT IO0CLR_bit.P0_12

#define DIR_SDIN IO0DIR_bit.P0_13
#define SET_SDIN IO0SET_bit.P0_13
#define CLR_SDIN IO0CLR_bit.P0_13

#endif

//select indicator driver. 0 - drvier IC is selected and can be accessed, 1 -
#define DIR_NCS FIO4DIR_bit.P4_31
#define SET_NCS MY_FIO4SET(B_31)
#define CLR_NCS MY_FIO4CLR(B_31)
//==================================


#define DIR_D8 FIO4DIR_bit.P4_0
#define SET_D8 MY_FIO4SET(B_0)
#define CLR_D8 MY_FIO4CLR(B_0)
#define DIR_D9 FIO4DIR_bit.P4_1
#define SET_D9 MY_FIO4SET(B_1)
#define CLR_D9 MY_FIO4CLR(B_1)
#define DIR_D10 FIO4DIR_bit.P4_2
#define SET_D10 MY_FIO4SET(B_2)
#define CLR_D10 MY_FIO4CLR(B_2)
#define DIR_D11 FIO4DIR_bit.P4_3
#define SET_D11 MY_FIO4SET(B_3)
#define CLR_D11 MY_FIO4CLR(B_3)
#define DIR_D12 FIO4DIR_bit.P4_4
#define SET_D12 MY_FIO4SET(B_4)
#define CLR_D12 MY_FIO4CLR(B_4)
#define DIR_D13 FIO4DIR_bit.P4_5
#define SET_D13 MY_FIO4SET(B_5)
#define CLR_D13 MY_FIO4CLR(B_5)
#define DIR_D14 FIO4DIR_bit.P4_6
#define SET_D14 MY_FIO4SET(B_6)
#define CLR_D14 MY_FIO4CLR(B_6)
#define DIR_D15 FIO4DIR_bit.P4_7
#define SET_D15 MY_FIO4SET(B_7)
#define CLR_D15 MY_FIO4CLR(B_7)
#define DIR_D16 FIO4DIR_bit.P4_8
#define SET_D16 MY_FIO4SET(B_8)
#define CLR_D16 MY_FIO4CLR(B_8)
#define DIR_D17 FIO4DIR_bit.P4_9
#define SET_D17 MY_FIO4SET(B_9)
#define CLR_D17 MY_FIO4CLR(B_9)


















//////LED////////////////////////

#ifdef FAST_PORT_ON

#define DIR_LED_G FIO1DIR_bit.P1_9
#define SET_LED_G MY_FIO1SET(B_9)
#define CLR_LED_G MY_FIO1CLR(B_9)
#define PIN_LED_G FIO1PIN_bit.P1_9

#define DIR_LED_O FIO1DIR_bit.P1_10
#define SET_LED_O MY_FIO1SET(B_10)
#define CLR_LED_O MY_FIO1CLR(B_10)
#define PIN_LED_O FIO1PIN_bit.P1_10

#define DIR_LED_R FIO1DIR_bit.P1_8
#define SET_LED_R MY_FIO1SET(B_8)
#define CLR_LED_R MY_FIO1CLR(B_8)
#define PIN_LED_R FIO1PIN_bit.P1_8

#else

#define DIR_LED_G IO1DIR_bit.P1_9
#define SET_LED_G MY_IO1SET(B_9)
#define CLR_LED_G MY_IO1CLR(B_9)
#define PIN_LED_G IO1PIN_bit.P1_9

#define DIR_LED_O IO1DIR_bit.P1_10
#define SET_LED_O MY_IO1SET(B_10)
#define CLR_LED_O MY_IO1CLR(B_10)
#define PIN_LED_O IO1PIN_bit.P1_10

#define DIR_LED_R IO1DIR_bit.P1_8
#define SET_LED_R MY_IO1SET(B_8)
#define CLR_LED_R MY_IO1CLR(B_8)
#define PIN_LED_R IO1PIN_bit.P1_8

#endif

//////////////////////////////////


void Display_init(void);

void Display_turnON(void);
void Display_turnOFF(void);

void Display_Init_8bit_262k( void);
void Display_Init_18bit_262k( void);

void Display_EMC_Init(void);


__IO_REG8(     DisplayData,              0x81000000,__READ_WRITE);
__IO_REG8(     DisplayData_R,              0x8100FC00,__READ_WRITE);
__IO_REG8(     DisplayData_G,              0x810000FC,__READ_WRITE);

//extern char ON_OFF_display;



void Display_flashGreenLED(void);
void Display_flashRedLED(void);
void Display_flashOrangeLED(void);
void Display_turnOFF_LEDs(void);
void Display_turnONGreenLED(void);
void Display_turnONRedLED(void);
void Display_turnONOrangeLED(void);
void Display_turnOFFGreenLED(void);
void Display_turnOFFRedLED(void);
void Display_turnOFFOrangeLED(void);
void Display_BlinkREDLED(DWORD ms);

void Display_EMC18_BUS_Init(BOOL b18bit);

void Display_Init_18bit_262k_updownleftright( void);


void Display_set_clip_region(WORD x1, WORD y1, WORD x2, WORD y2);
void Display_dot(WORD x, WORD y, COLORREF clr);
void Display_dot_xor(WORD x, WORD y, COLORREF clr);
void Display_output_bmp(WORD x, WORD y, const BITMAPFILEHEADER* pBmp);
void Display_set_screen_memory_adr(WORD x, WORD y);
void Display_fillRect(RECT rect, COLORREF clr);

void Display_clearScreen(void);
void Display_clearRect(RECT rect, int vsync);

void Display_catchVSYNC(int line);

void Display_emc_gotoIdleMode(void);
void Display_emc_outOfIdleMode(void);

void Display_startup_display2(int i);

void Display_drawLine(int x1, int y1, int x2, int y2, COLORREF clr);
void Display_drawLine_xor(int x1, int y1, int x2, int y2, COLORREF clr);

void Display_set_display_entrymode(BOOL bRGBmode, BOOL bUpTodown);

void Display_showStatusLine(void);

void Display_clearTextWin(int vsync);
__arm void Display_printR(DWORD r14);
__arm void Display_printHEX(BYTE value);
__arm void Display_prepareEmergencyTextControl(void);

COLORREF Display_read_dot(WORD x, WORD y);

void Display_outputTextByLang(const char* pText);
void Display_drawRect(int x1, int y1, int x2, int y2, COLORREF clr);
void Display_drawRect_xor(int x1, int y1, int x2, int y2, COLORREF clr);

void Display_drawVLine_xor(int x, int y1, int y2, COLORREF clr);
void Display_drawVLine(int x, int y1, int y2, COLORREF clr);
void Display_drawHLine(int x1, int y, int x2, COLORREF clr);
void Display_drawHLine_xor(int x1, int y, int x2, COLORREF clr);
void Display_left_scroll(int x1, int y1, int x2, int y2, int step);
void Display_warmup_display_start(void);
const char* Display_getTextByLang(const char* pText);

void Display_checkForClearLine(void);
void Display_clearUserPart(void);

void Display_outputTextByLang_log(const char* pText);

void Display_Init_18bit_262k_leftrightdownup( void);

void Display_outputTextFromLine(const char* pText, int lineIndex);
int Display_calcTextLines(const char* pText);
void Display_getScreen(void);

void Display_ClearSteps(int stepX, int winX);

#define FON_COLOR 0x81000055
#define FON_COLOR_BYTE 0x55

void Display_Symbol_clr(int x,int vsync);
void Display_outputText_withclean(const char* pText, int clr_pos);
void Display_outputTextByLang_withclean(const char* pText,  int clr_pos);
__arm void Display_left_scroll_new(int x1, int y1, int x2, int y2, int step);
void Display_Init_18bit_262k_tst( unsigned int adr, unsigned char data);


#endif	//#ifndef _DISPLAY_H
