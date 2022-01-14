#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H


#include <iolpc2388.h>
#include "types.h"

//power on/off BT module

#ifdef FAST_PORT_ON

#define DIR_BT_LS FIO0DIR_bit.P0_22
#define SET_BT_LS MY_FIO0SET(B_22)//IO0SET_bit.P0_14
#define CLR_BT_LS MY_FIO0CLR(B_22)//IO0CLR_bit.P0_14
#define PIN_BT_LS FIO0PIN_bit.P0_22

#define DIR_BT_ON FIO0DIR_bit.P0_14
#define SET_BT_ON MY_FIO0SET(B_14)
#define CLR_BT_ON MY_FIO0CLR(B_14)
#define PIN_BT_ON FIO0PIN_bit.P0_14

//reset bluetooth
#define DIR_BT_RES FIO0DIR_bit.P0_30
#define SET_BT_RES MY_FIO0SET(B_30)
#define CLR_BT_RES MY_FIO0CLR(B_30)
#define PIN_BT_RES FIO0PIN_bit.P0_30

//
#define DIR_BT_RF FIO0DIR_bit.P0_29
#define SET_BT_RF MY_FIO0SET(B_29)
#define CLR_BT_RF MY_FIO0CLR(B_29)
#define PIN_BT_RF FIO0PIN_bit.P0_29

#else

#define DIR_BT_LS IO0DIR_bit.P0_22
#define SET_BT_LS MY_IO0SET(B_22)//IO0SET_bit.P0_14
#define CLR_BT_LS MY_IO0CLR(B_22)//IO0CLR_bit.P0_14
#define PIN_BT_LS IO0PIN_bit.P0_22

#define DIR_BT_ON IO0DIR_bit.P0_14
#define SET_BT_ON MY_IO0SET(B_14)
#define CLR_BT_ON MY_IO0CLR(B_14)
#define PIN_BT_ON IO0PIN_bit.P0_14

//reset bluetooth
#define DIR_BT_RES IO0DIR_bit.P0_30
#define SET_BT_RES MY_IO0SET(B_30)
#define CLR_BT_RES MY_IO0CLR(B_30)
#define PIN_BT_RES IO0PIN_bit.P0_30

//
#define DIR_BT_RF IO0DIR_bit.P0_29
#define SET_BT_RF MY_IO0SET(B_29)
#define CLR_BT_RF MY_IO0CLR(B_29)
#define PIN_BT_RF IO0PIN_bit.P0_29

#endif


#define BLUETOOTH_ADDRESS 0x03





enum ENU_COMM_STATE
{
	enu_comm_state_modem=0x10000,
	enu_comm_state_pc,
	enu_comm_state_du,
};

enum ENU_COMM_POWER_STATE
{
	enu_comm_power_state_manually=0x10000,
	enu_comm_power_state_on,
};


struct tagBluetoothControl
{
	BOOL bBluetooth_ON;	//bluetooth is ON or OFF
	BOOL bBluetooth_Connected;	//have any connection?
	BOOL bShow_Bluetooth_sym;	//show status of symbol

#define BTH_RCV_BUF_LEN 1024
#define BTH_TRM_BUF_LEN (1024+16)
//#pragma pack(1)
	volatile BYTE rcvBuff[BTH_RCV_BUF_LEN+16];
	volatile BYTE trmBuff[BTH_TRM_BUF_LEN];
	BYTE rcvBuff_safe[BTH_RCV_BUF_LEN+16];
//#pragma pack()

	struct tagUART uart;
	//this value must saved in eeprom
	enum ENU_COMM_STATE comm_state;
	//this value must saved in eeprom
	enum ENU_COMM_POWER_STATE comm_power_state;
	char moduleName[40];
	char moduleNameTemp[40];
	int iModuleComm;	//if >0 then module communication protocol is used
};

extern struct tagBluetoothControl bluetoothControl;

#ifndef GPS_BT_FREE	


void Bluetooth_show_symbol(int x);
void Bluetooth_control(void);
void Bluetooth_UART3_Init(void);
__arm void _INT_UART3_Bluetooth(void);
void Bluetooth_turnON(void);
void Bluetooth_sym_control(void);
void Bluetooth_checkDataReceived(void);
void Bluetooth_sendSequence(int len);

void Bluetooth_rcvData_first_Dispatcher(void);
void Bluetooth_answer_first_Dispatcher(void);
void Bluetooth_startbyState(void);

void Bluetooth_readModuleName(void);
void Bluetooth_sendSequenceEx(int len);
void Bluetooth_rcvData_module_Dispatcher(void);

void Bluetooth_writeModuleName(void);
void Bluetooth_prepareModuleName(void);

#endif	//#ifndef GPS_BT_FREE	

void Bluetooth_Init(void);
void Bluetooth_turnOFF(void);

#endif //#ifndef _BLUETOOTH_H
