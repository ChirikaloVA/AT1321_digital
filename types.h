#ifndef _TYPES_H
#define _TYPES_H

//user types
//31/10/2008 Bystrov



#define CHANNELS 1024

#ifndef VOID
#define VOID void
typedef signed char CHAR;
typedef short SHORT;
typedef long LONG;
#endif




#ifndef BASETYPES
#define BASETYPES
typedef unsigned long ULONG;
typedef ULONG *PULONG;
typedef unsigned short USHORT;
typedef USHORT *PUSHORT;
typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
typedef char *PSZ;
#endif  /* !BASETYPES */

#define MAX_PATH          260

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif



#undef FAR
#undef  NEAR
#define FAR
#define NEAR
#ifndef CONST
#define CONST               const
#endif

typedef unsigned long       DWORD;
typedef unsigned long       BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef BOOL            *PBOOL;
typedef BOOL             *LPBOOL;
typedef BYTE            *PBYTE;
typedef BYTE             *LPBYTE;
typedef int             *PINT;
typedef int              *LPINT;
typedef WORD            *PWORD;
typedef WORD             *LPWORD;
typedef long             *LPLONG;
typedef DWORD           *PDWORD;
typedef DWORD            *LPDWORD;
typedef void             *LPVOID;
typedef CONST void       *LPCVOID;


typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int        *PUINT;


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

typedef long LONG_PTR, *PLONG_PTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;

//
// Add Windows flavor DWORD_PTR types
//

typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;


#define MAKEFLOAT(val, a, b, c, d)      *((char*)&val)=a;*((char*)&val+1)=b;*((char*)&val+2)=c;*((char*)&val+3)=d;
#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#define MAKESHORT(a, b)      ((SHORT)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((SHORT)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#define HIBYTE(w)           ((BYTE)((DWORD_PTR)(w) >> 24))
#define HIBYTE_W(w)           ((DWORD)((DWORD_PTR)(w) >> 8))
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define LO2BYTE(w)           ((BYTE)(((DWORD_PTR)(w) >> 8) & 0xff))
#define LO3BYTE(w)           ((BYTE)(((DWORD_PTR)(w) >> 16) & 0xff))


typedef DWORD   COLORREF;
typedef DWORD   *LPCOLORREF;


typedef struct tagRECT
{
    WORD    left;
    WORD    top;
    WORD    right;
    WORD    bottom;
} RECT, *PRECT, NEAR *NPRECT, FAR *LPRECT;

typedef const RECT FAR* LPCRECT;

typedef struct _RECTL       /* rcl */
{
    WORD    left;
    WORD    top;
    WORD    right;
    WORD    bottom;
} RECTL, *PRECTL, *LPRECTL;

typedef const RECTL FAR* LPCRECTL;

typedef struct tagPOINT
{
    WORD  x;
    WORD  y;
} POINT, *PPOINT, NEAR *NPPOINT, FAR *LPPOINT;

typedef struct _POINTL      /* ptl  */
{
    WORD  x;
    WORD  y;
} POINTL, *PPOINTL;

typedef struct tagSIZE
{
    WORD        cx;
    WORD        cy;
} SIZE, *PSIZE, *LPSIZE;

typedef SIZE               SIZEL;
typedef SIZE               *PSIZEL, *LPSIZEL;

typedef struct tagPOINTS
{
#ifndef _MAC
    SHORT   x;
    SHORT   y;
#else
    SHORT   y;
    SHORT   x;
#endif
} POINTS, *PPOINTS, *LPPOINTS;

#define RGB(r,g,b)          ((COLORREF)((((BYTE)(b&0x3f))|((WORD)((BYTE)(g&0x3f))<<8))|(((DWORD)(BYTE)(r&0x3f))<<16)))



//receiver buffer len in bytes
//#define RCV_BUF_LEN 300
//transmitter buffer len
//#define EXTRM_BUF_LEN 1040



//these values must be from 0-4
enum RCV_ERRORS
{
	RCV_OK,
	RCV_ERR_TIMEOUT,
	RCV_ERR_BYTE,
	RCV_ERR_BUF_OVERFLOW,
	RCV_ERR_CRC,
};


struct tagUART
{
	volatile BYTE* rcvBuff;//[RCV_BUF_LEN];
	volatile DWORD rcvBuffLen;	//must be reset to 0 before transmition
	volatile enum RCV_ERRORS bRcvError;	//must be reset before transmition
	volatile BOOL bDataReceived;	//must be reset before transmition
	
	volatile BYTE* trmBuff;//[EXTRM_BUF_LEN];
	volatile DWORD trmBuffLen;	//must be reset to 0 before transmition
	volatile DWORD trmBuffLenConst;	//постоянное значение длинны передающего буфера
	volatile BOOL bTrmReady;	//must be set to 1 before main cycle. if 0 then NOONE allows to put data to trm buffer, it will be set to 1 if rcv data is process by first dispatcher

	volatile int iTries;	//number of tries of sending
	
	//длинны буферов обмена
	DWORD constRcvBuffLen;
	DWORD constTrmBuffLen;
	
	BYTE *rcvBuff_safe;//[RCV_BUF_LEN];
	DWORD rcvBuffLen_safe;	
	enum RCV_ERRORS bRcvError_safe;
	BOOL bDataReceived_safe;
};




#pragma pack(1)
struct tagDateTime
{
  WORD year;
  BYTE month;
  BYTE dayOfMonth;
  BYTE hour;
  BYTE minute;
  BYTE second;
};
#pragma pack()

#pragma pack(1)
struct tagCommonGPS
{
	double Lat;
	double Lon;
	float Alt;
	float Spd;
	float Dir;
	unsigned char Vld;
	unsigned char Sat;
};
#pragma pack()


typedef DWORD HFILE;


typedef long HRESULT;


#define _HRESULT_TYPEDEF_(_sc) ((HRESULT)_sc)


#ifndef SUCCEEDED
#define SUCCEEDED(hr)  ((HRESULT)(hr) >= 0)
#endif

#ifndef FAILED
#define FAILED(hr)  ((HRESULT)(hr) < 0)
#endif

#ifndef S_OK
#define S_OK  ((HRESULT)0x00000000L)
#endif



//
// Codes 0x4000-0x40ff are reserved for OLE
//
//
// Error codes
//
//
// MessageId: E_UNEXPECTED
//
// MessageText:
//
//  Catastrophic failure
//
#define E_UNEXPECTED                     _HRESULT_TYPEDEF_(0x8000FFFFL)

//
// MessageId: E_NOTIMPL
//
// MessageText:
//
//  Not implemented
//
#define E_NOTIMPL                        _HRESULT_TYPEDEF_(0x80004001L)

//
// MessageId: E_OUTOFMEMORY
//
// MessageText:
//
//  Ran out of memory
//
#define E_OUTOFMEMORY                    _HRESULT_TYPEDEF_(0x8007000EL)

//
// MessageId: E_INVALIDARG
//
// MessageText:
//
//  One or more arguments are invalid
//
#define E_INVALIDARG                     _HRESULT_TYPEDEF_(0x80070057L)

//
// MessageId: E_NOINTERFACE
//
// MessageText:
//
//  No such interface supported
//
#define E_NOINTERFACE                    _HRESULT_TYPEDEF_(0x80004002L)

//
// MessageId: E_POINTER
//
// MessageText:
//
//  Invalid pointer
//
#define E_POINTER                        _HRESULT_TYPEDEF_(0x80004003L)

//
// MessageId: E_HANDLE
//
// MessageText:
//
//  Invalid handle
//
#define E_HANDLE                         _HRESULT_TYPEDEF_(0x80070006L)

//
// MessageId: E_ABORT
//
// MessageText:
//
//  Operation aborted
//
#define E_ABORT                          _HRESULT_TYPEDEF_(0x80004004L)

//
// MessageId: E_FAIL
//
// MessageText:
//
//  Unspecified error
//
#define E_FAIL                           _HRESULT_TYPEDEF_(0x80004005L)

//
// MessageId: E_ACCESSDENIED
//
// MessageText:
//
//  General access denied error
//
#define E_ACCESSDENIED                   _HRESULT_TYPEDEF_(0x80070005L)



//number of symbols in report area of spectrum
#define MAX_REPORT_SYMS 150


//макс кол элементов в табл
#define MAX_TABLE_RECORDS 30

//таблица пар индекса и значения
//исп для таблиц канал энергия и канал сигма
struct tagIndexMeanTable
{
	float index;
	float mean;
};




#define B_0	1
#define B_1	2
#define B_2	4
#define B_3	8
#define B_4	16
#define B_5	32
#define B_6	64
#define B_7	128
#define B_8	256
#define B_9	512
#define B_10	1024
#define B_11	2048
#define B_12	4096
#define B_13	8192
#define B_14	16384
#define B_15	32768
#define B_16	65536
#define B_17	131072
#define B_18	262144
#define B_19	524288
#define B_20	1048576
#define B_21	2097152
#define B_22	4194304
#define B_23	8388608
#define B_24	16777216
#define B_25	33554432
#define B_26	67108864
#define B_27	134217728
#define B_28	268435456
#define B_29	536870912
#define B_30	1073741824
#define B_31	2147483648


#define MY_FIO0SET(value)	*((unsigned long*)0x3FFFC018) = value
#define MY_FIO0CLR(value)	*((unsigned long*)0x3FFFC01C) = value
#define MY_FIO1SET(value)	*((unsigned long*)0x3FFFC038) = value
#define MY_FIO1CLR(value)	*((unsigned long*)0x3FFFC03C) = value
#define MY_FIO2SET(value)	*((unsigned long*)0x3FFFC058) = value
#define MY_FIO2CLR(value)	*((unsigned long*)0x3FFFC05C) = value
#define MY_FIO3SET(value)	*((unsigned long*)0x3FFFC078) = value
#define MY_FIO3CLR(value)	*((unsigned long*)0x3FFFC07C) = value
#define MY_FIO4SET(value)	*((unsigned long*)0x3FFFC098) = value
#define MY_FIO4CLR(value)	*((unsigned long*)0x3FFFC09C) = value

	





#endif	//#ifndef _TYPES_H
