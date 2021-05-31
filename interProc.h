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

//�������� �������� �� ������� ��� �������� ������ �� ���������� �������
//#define MAX_READYTOTRM_REQ_REP 10
//�������� �������� �� ������� ��� ������ ����������
//#define MAX_READYTOTRM_REQ_EXC 13

//��� ������� ��������� �����
#define MAX_TRM_TRIES 15

/*
��������� ���������� (03h)
�������	����������
0	����� ���������� ������� � �������� - 2-� �������(16-���) ����� (1� ��. �����)
1	������
2	����������� ��������. 0�4095
3	�������� ������� ������. �������� ���������� ��������: 0�65535. (2�����)
4	�������� �������� ������. �������� ���������� ��������: 0�65535. (2�����)
5	����������� �� ���-�� ���� ��� ����������� ������� �� �������� ����� (�������� ���������� ����) ��������� �����, ��  2 �����
6	--"-- �� ����
7	���-�� ���� ��� ��� ������ �� ���� ��� ���������� ������ -����� 00 ��
8	����� ������ ��������� �� ���� ��� ���������� ������ - 2-� �������(16-���) ����� (1� ��. �����)

��������� ������ (04h)
�������	����������
0	"�����" ����� � �������� - - 2-� �������(16-���) ����� (1� ��. �����)
1	������
2	������� ����������� � ��������   � ���������   �� �������   ( ��.� ). ��. ���� ����� �����, ��.  - ������� . ������������� ����������� ����������: � �������������� ���� , ������� �����-��. ���� - ������ �������������.
0Eh (14)	���������� �������� ����� �� �������� �������� � ��������� ������ (2 �����) (���������� ���� �� ������, ������������� ���� � ��� ����) - 2-� �������(16-���) ����� (1� ��. �����)

����������� ������ (05h)
��� 16-�	����������
0000	�������� ������ ���������� ������� � ������ ������ �������
0001	���������/���������� ���������� �������
0002	������
0003	������
0103	���������� ����� 512 ������� - ������ � ������ ���������
0203	���������� ����� 1024 ������ - ������ � ������ ���������
0303	���������� ����� 2048 ������� - ������ � ������ ���������
0005	�������/��������� ������������� ��������� - ������ � ������ ���������
0007	��������/��������� ����� ���������
0024	��������/��������� ��������� �����
0050	������� ���������� �������� ������
0052	��������� ������������ �� ���������  (����������� ����������� � EEPROM)

�������� ���������� (06h)

����� ��������� (07h)
���	��������
0	0	�������� ���������� � �����
	1	�������� ���������� ���� �����
1	0	���������� ������� ���������
	1	���� ���������� �������
2	=1	���� ������������ �� ���������
3	=1	������������� ��������� ��������
4	0	-215���	1	�����
512 ���	0	�����
1024 ���	1	�����
2048 ���
5	0		0		1		1
6	=1 	��������� ����� �������
7	= 1 	����� ��������� �������

�������������� ������� (0Bh)

����������������� �������� ����� �������������� (10h)
�������	����������
0	����������������� ��� ������� - 2����� ������ ( 00 0�)
1	����� ������ ROM - 2 ����� ������ (00 01)
2	��������� ����� -2� ������� 16-� �����
3	���� ������������ ������� ��.����� - ��� (�������� 04 - 2004), �� ����� ����� ������ � ���� (�������� 17)
4	������������ ����� ������� - 2 ����� (00 01)

����������������� ��� ����� �������������� (11h)

�������� ����������� (08h, ���������� 02)
���	��������
0	�������� ���������� ���� �����
1	��� ���������� �� ���������
2	������ EEPROM
4	������������ 3-� ���� ���������
7	��������� ����� �������� � ������ � EEPROM
8	��������� ������������� ������� � EEPROM
9	������ ������ ������� ������
10	����������� ��� ����������� ��������� (-10�+50)
11	������ ������� �����������
12	������ ���������� - ������ 1
13	������ ���������� - ������ 2
14	������ ����� (����� �������������)
15	����� ������� ����� ��������� �������

1.5.1	���� ����������
���� ���������� ��������� � ��������� �������.
��� ����������	�������� ����������
01	������������ �������
02	������������ ����� ������
03	������������ �������� ������
04	 ��������� ���������� ����� �������� � ������� ������� ������ �������� �����������
05	������ ����� ������ - ������������ �/��� ��� ���� ����
06	���� �������������� ����� ����������� ���������� �������

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












//��������� ��� ������������� ������ �����
/*
���������� ������������������:
1,1,0 - ��������� ��������� ��� ���������� ���������
1,1,1 - ����������, ������ ������� �����
2,1,1 - �������, ������ � ���������
2,2,1 - ���������� � ����� ��������� ���������

������������ ������������������:
1,1,0 - ��������� ��������� ��� ���������� ���������
1,1,1 - ����������, ������ ������� �����
1,1,1 - �� �������
*/
struct tagInteProcRSModbusSync
{
	//�������
	DWORD dw_inc;	//������� ����������
	DWORD dw_incLastRcv;	//������ ������ ���������
	DWORD dw_incLastTrm;	//��������� ������������
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
	BYTE btStatus;	//����� �������
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

