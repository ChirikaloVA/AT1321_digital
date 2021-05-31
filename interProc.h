#ifndef _INTER_PROC_H
#define _INTER_PROC_H


#include "types.h"

#define INTERPROC_TIMER_VAL 10

#define INTERPROC_WAIT_TIME 500

//address in command to second proc from first proc
#define INTERPROC_ADDRESS 0x01

//transmite buffer len
//#define TRM_BUF_LEN 45
//max number of commands in order
#define MAX_CMD_IN_ORDER 30

//значение запросов на посылки для рестарта обмена по конкретной команде
//#define MAX_READYTOTRM_REQ_REP 10
//значение запросов на посылки для вывода исключения
//#define MAX_READYTOTRM_REQ_EXC 13

//кол попыток отправить пакет
#define MAX_TRM_TRIES 15

/*
регистров управления (03h)
Регистр	Назначение
0	Время накопления спектра в секундах - 2-х байтное(16-ное) число (1б ст. часть)
1	резерв
2	Управлением усиления. 0…4095
3	Значение нижнего порога. Диапазон допустимых значений: 0…65535. (2байта)
4	Значение верхнего порога. Диапазон допустимых значений: 0…65535. (2байта)
5	Коэффициент на кол-во сигм для определения порогов по скорости счета (условное количество сигм) плавающее число, ст  2 байта
6	--"-- мл байт
7	Кол-во сигм для мин порога по фону для поискового режима -целое 00 хх
8	Номер канала разбиения на окна для поискового режима - 2-х байтное(16-ное) число (1б ст. часть)

регистров данных (04h)
Регистр	Назначение
0	"Живое" время в секундах - - 2-х байтное(16-ное) число (1б ст. часть)
1	резерв
2	Текущая температура в градусах   с точностью   до десятых   ( ХХ.Х ). Ст. байт целая часть, мл.  - дробная . Отрицательная температура передается: в дополнительном коде , дробная часть-мл. байт - всегда положительная.
0Eh (14)	Отклонение скорости счета от фонового значения в поисковом режиме (2 байта) (количество сигм со знаком, отрицательное знач в доп коде) - 2-х байтное(16-ное) число (1б ст. часть)

управляющий сигнал (05h)
Код 16-е	Назначение
0000	Обнулить память накопления спектра и таймер живого времени
0001	Разрешить/остановить накопление спектра
0002	Резерв
0003	Резерв
0103	Установить режим 512 каналов - только в режиме настройки
0203	Установить режим 1024 канала - только в режиме настройки
0303	Установить режим 2048 каналов - только в режиме настройки
0005	Включит/выключить температурную коррекцию - только в режиме настройки
0007	Включить/выключить режим настройки
0024	Включить/выключить поисковый режим
0050	Удалить содержимое записной книжки
0052	Выполнить стабилизацию по источнику  (Коэффициент сохраняется в EEPROM)

регистра управления (06h)

слово состояния (07h)
Бит	Значение
0	0	Питающее напряжение в норме
	1	Питающее напряжение ниже нормы
1	0	Накопление спектра запрещено
	1	Идет накопление спектра
2	=1	Идет стабилизация по источнику
3	=1	Температурная коррекция включена
4	0	-215кан	1	Режим
512 кан	0	Режим
1024 кан	1	Режим
2048 кан
5	0		0		1		1
6	=1 	Поисковый режим включен
7	= 1 	Режим настройки включен

накапливаемого спектра (0Bh)

идентификационных значений блока детектирования (10h)
Регистр	Назначение
0	Идентификационный код прибора - 2байта данных ( 00 0С)
1	Номер версии ROM - 2 байта данных (00 01)
2	Заводской номер -2х байтное 16-е число
3	Дата изготовления прибора ст.часть - год (например 04 - 2004), мл часть номер недели в году (например 17)
4	Интерфейсный адрес прибора - 2 байта (00 01)

идентификационный код блока детектирования (11h)

регистра диагностики (08h, подкоманда 02)
Бит	Значение
0	Питающее напряжение ниже нормы
1	Нет прерываний от детектора
2	Ошибка EEPROM
4	Переполнение 3-х байт измерения
7	Разрушены коэфф усиления и пороги в EEPROM
8	Разрушены калибровочные спектры в EEPROM
9	Ошибка чтения внешней памяти
10	Температура вне допустимого диапазона (-10…+50)
11	Авария датчика температуры
12	Авария клавиатуры - кнопка 1
13	Авария клавиатуры - кнопка 2
14	Авария часов (время неопределенно)
15	Сброс питания часов реального времени

1.5.1	Коды исключений
Коды исключений приведены в следующей таблице.
Код исключения	Описание исключения
01	Недопустимая команда
02	Недопустимый адрес данных
03	Недопустимое значение данных
04	 Детальную информацию можно получить с помощью команды чтения регистра диагностики
05	Ошибка ввода вывода - переполнение и/или нет стоп бита
06	Блок детектирования занят выполнением предыдущей команды

*/

extern const BYTE arCmd_GetStatus[];
//extern const BYTE arCmd_GetControlRegister_simgaThresholds[];
extern const BYTE arCmd_GetControlRegister_presetAcqTime[];
extern const BYTE arCmd_GetControlRegister_gain[];
extern const BYTE arCmd_GetControlRegister_lowLimit[];
extern const BYTE arCmd_GetControlRegister_highLimit[];

//extern const BYTE arCmd_GetControlRegister_bkgDR[];
extern const BYTE arCmd_GetControlRegister_bkgCPS[];

extern const BYTE arCmd_GetDataRegister_acqTime[];
extern const BYTE arCmd_GetDataRegister_MeasurementRegs[];
extern const BYTE arCmd_GetDataRegister_temperature[];
extern const BYTE arCmd_GetDiagnostic[];
extern const BYTE arCmd_Command_ResetSpectrum[];
extern const BYTE arCmd_Command_StartAcq[];
extern const BYTE arCmd_Command_StopAcq[];
extern const BYTE arCmd_GetSpectrumPart1[];
extern const BYTE arCmd_GetSpectrumPart2[];
extern const BYTE arCmd_GetSpectrumPart3[];
extern const BYTE arCmd_GetSpectrumPart4[];
extern const BYTE arCmd_GetSpectrumPart5[];
extern const BYTE arCmd_GetSpectrumPart6[];
extern const BYTE arCmd_GetSpectrumPart7[];
extern const BYTE arCmd_GetSpectrumPart8[];
extern const BYTE arCmd_GetSpectrumPart9[];
extern const BYTE arCmd_GetSpectrumPart10[];
extern const BYTE arCmd_GetSpectrumPart11[];
extern const BYTE arCmd_GetSpectrumPart12[];
extern const BYTE arCmd_GetSpectrumPart13[];
extern const BYTE arCmd_Command_setMeasurementMode[];
extern const BYTE arCmd_Command_setSearchMode[];
extern const BYTE arCmd_Command_resetAveraging[];
extern const BYTE arCmd_Command_resetDose[];
extern const BYTE arCmd_Command_makeCpsBaseForSearch[];
extern const BYTE arCmd_Command_makeCpsAsBkg[];
extern const BYTE arCmd_Command_turnonSubtrackBkg[];
extern const BYTE arCmd_Command_turnoffSubtrackBkg[];
extern const BYTE arCmd_Command_turnoffTuning[];
extern const BYTE arCmd_Command_turnonTuning[];












//структура для синхронизации потока даных
/*
нормальная последовательность:
1,1,0 - начальное состояние как готовность отправить
1,1,1 - отправлено, готовы принять ответ
2,1,1 - принято, готово к обработке
2,2,1 - обработано и снова начальное состояние

ненормальная последовательность:
1,1,0 - начальное состояние как готовность отправить
1,1,1 - отправлено, готовы принять ответ
1,1,1 - НЕ принято
*/
struct tagInteProcRSModbusSync
{
	//индексы
	DWORD dw_inc;	//текущий отосланный
	DWORD dw_incLastRcv;	//индекс ответа последней
	DWORD dw_incLastTrm;	//последней отправленный
};


struct tagInterProcRSModbus
{
	//control registers
	WORD wdPresetAcqTime;
	struct tagInteProcRSModbusSync swdPresetAcqTime;

/*	WORD wdGain;
	struct tagInteProcRSModbusSync swdGain;
*/

	WORD wdLowLimit;
	struct tagInteProcRSModbusSync swdLowLimit;

	WORD wdHighLimit;
	struct tagInteProcRSModbusSync swdHighLimit;

/*	WORD wdSigmaOper;
	WORD wdSigmaSleep;
	struct tagInteProcRSModbusSync swdSigmaThresholds;*/

	//data resgisters
	WORD wdAcqTime;
	struct tagInteProcRSModbusSync swdAcqTime;

	float fMomCps;
	float fCps;	//
	float fCpsErr;	//
	float fDoserate;	//
	float fDoserateErr;	//
	float fDose;
	short sSigma;
	struct tagInteProcRSModbusSync swdMeasurementRegs;	//swdMomCps;

	WORD wdTemperature;
	struct tagInteProcRSModbusSync swdTemperature;

	//status register
	BYTE btStatus;
	struct tagInteProcRSModbusSync sbtStatus;

	//diagnostic register
	WORD wdDiag;
	struct tagInteProcRSModbusSync swdDiag;

	float fStabGain;
	struct tagInteProcRSModbusSync swdStabGain;

//	float fBkgDR;
//	struct tagInteProcRSModbusSync swdBkgDR;
	float fBkgCPS;
	struct tagInteProcRSModbusSync swdBkgCPS;

	float fDTCOEF;
	struct tagInteProcRSModbusSync swdDTCOEF;

	//spectrum
	BYTE arSpectrum[CHANNELS*3];	//by 3 bytes on channel
//	struct tagInteProcRSModbusSync sarSpectrum;

	struct tagInteProcRSModbusSync sarSpectrumZip;
};




////////////////////////////////
//order of transmitted commands
//#define INT_RCV_BUF_LEN 260
#define INT_RCV_BUF_LEN 4096
#define INT_TRM_BUF_LEN 48

struct tagCmd
{
	BYTE arBuff[INT_TRM_BUF_LEN];
	int len;	//buf len
	//if cmdIndex = -1 then buffer is empty
	int cmdIndex;	//current command index, growth from 0 to 0x7fffffff
};


#define MAX_TEMPER 50.0
#define MIN_TEMPER -20.0


struct tagInterProcControl
{
//#pragma pack(1)
	volatile BYTE rcvBuff[INT_RCV_BUF_LEN];
	volatile BYTE trmBuff[INT_TRM_BUF_LEN];
	BYTE rcvBuff_safe[INT_RCV_BUF_LEN];
//#pragma pack()

	struct tagUART uart;
	struct tagInterProcRSModbus rsModbus;
	//temperature mean
	float fTemperature;
	struct tagCmd arCmd[MAX_CMD_IN_ORDER];
	BOOL bTimeOutReached;	//if true then system will no go to idle mode before answer processing
	BYTE btStatus;	//копия статуса
};



extern struct tagInterProcControl interProcControl;








void InterProc_iterateDataReady(struct tagInteProcRSModbusSync volatile* pIPRSMS);


BOOL InterProc_isDataReady(struct tagInteProcRSModbusSync volatile* pIPRSMS);


void InterProc_UART1_Init(void);

__arm void _INT_UART1_InterProc(void);

void InterProc_Init(void);
void InterProc_sendSequence(/*const unsigned char* sequence, */int len);
void InterProc_InterProcControl(void);
//BYTE InterProc_isReadyToSend(void);
void InterProc_checkDataReceived_intcall(void);

void InterProc_readTemperature(void);

__arm void _INT_Timer2_InterProc(void);
void InterProc_Timer_turnON_intcall(void);
void InterProc_Timer_turnOFF(void);

void InterProc_rcvData_first_Dispatcher(void);

void InterProc_readDiagnostic(void);
void InterProc_showTemperature(void);
int InterProc_getNextCmd(BYTE volatile * cmd);
void InterProc_fillNewCmd(const BYTE volatile * cmd, int len);
void InterProc_initCmdOrder(void);
void InterProc_setHighLimit(WORD val);
void InterProc_setLowLimit(WORD val);
void InterProc_setGain(void);

void InterProc_stopSpectrumAcq(void);
void InterProc_startSpectrumAcq(void);
void InterProc_resetSpectrum(void);
void InterProc_setAcqTime(WORD val);
void InterProc_readStatus(void);

void InterProc_readAcqTime(void);
void InterProc_readSpectrum(void);

void InterProc_second_Dispatcher(void);
void InterProc_resetSyncData(struct tagInteProcRSModbusSync volatile* pIPRSMS);
BOOL InterProc_isReadyToTransmit(struct tagInteProcRSModbusSync volatile* pIPRSMS);
void InterProc_Timer_init(void);

void InterProc_readMeasurementRegs(void);
void InterProc_resendSequence(void);


void InterProc_goSleep(void);
void InterProc_goAwake(void);

void InterProc_setSearchMode(void);
void InterProc_setMeasurementMode(void);
void InterProc_resetAveraging(void);
void InterProc_makeCpsAsBkg(void);
void InterProc_makeCpsBaseForSearch(void);
void InterProc_turnonSubtrackBkg(void);
void InterProc_turnoffSubtrackBkg(void);
void InterProc_turnoffTuning(void);
void InterProc_turnonTuning(void);

void InterProc_setWinTable(void);


void InterProc_findFreeSlot(int * pIndexIndex, int * pMaxIndex);

BOOL InterProc_isDataFinalReady(struct tagInteProcRSModbusSync volatile* pIPRSMS);
void InterProc_resetDose(void);
void InterProc_getGain(void);

void InterProc_setSigmaSearchThresholds(void);
///void InterProc_setSigmaSleepThresholds(float value);

void InterProc_setStabPeak(float value);
void InterProc_setCurPeak(float value);
void InterProc_stabilize(void);


//void InterProc_getBkgDR(void);
void InterProc_getBkgCPS(void);

int InterProc_countFreeSlots(void);

void InterProc_setDTCEOF(float value);
void InterProc_readDTCOEF(void);

void InterProc_readSpectrumZip(void);

#endif	//#ifndef _INTER_PROC_H

