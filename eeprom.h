//eeprom.h

#ifndef _EEPROM_H
#define _EEPROM_H


#include <iolpc2388.h>
#include "types.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

/*
Write Protect: When the WP pin is asserted, all sectors specified for protection by the Sector
Protection Register will be protected against program and erase operations regardless of whether
the Enable Sector Protection command has been issued or not. The WP pin functions
independently of the software controlled protection method. After the WP pin goes low, the
content of the Sector Protection Register cannot be modified.
If a program or erase command is issued to the device while the WP pin is asserted, the device
will simply ignore the command and perform no operation. The device will return to the idle state
once the CS pin has been deasserted. The Enable Sector Protection command and Sector
Lockdown command, however, will be recognized by the device when the WP pin is asserted.
The WP pin is internally pulled-high and may be left floating if hardware controlled protection will
not be used. However, it is recommended that the WP pin also be externally connected to VCC
whenever possible.
*/
#define DIR_WP FIO0DIR_bit.P0_3
#define SET_WP MY_FIO0SET(B_3)//IO0SET_bit.P0_3
#define CLR_WP MY_FIO0CLR(B_3)//IO0CLR_bit.P0_3
#define PIN_WP FIO0PIN_bit.P0_3

/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
Reset: A low state on the reset pin (RESET) will terminate the operation in progress and reset
the internal state machine to an idle state. The device will remain in the reset condition as long as
a low level is present on the RESET pin. Normal operation can resume once the RESET pin is
brought back to a high level.
The device incorporates an internal power-on reset circuit, so there are no restrictions on the
RESET pin during power-on sequences. If this pin and feature are not utilized it is recommended
that the RESET pin be driven high externally.
*/
#define DIR_RES FIO0DIR_bit.P0_4
#define SET_RES MY_FIO0SET(B_4)
#define CLR_RES MY_FIO0CLR(B_4)
#define PIN_RES FIO0PIN_bit.P0_4




#define DIR_ECS FIO0DIR_bit.P0_6
#define SET_ECS MY_FIO0SET(B_6)
#define CLR_ECS MY_FIO0CLR(B_6)
#define PIN_ECS	FIO0PIN_bit.P0_6




//len for sector addres for adressing
#define SECTOR_LEN_TRUE 512
//len of whole data including CRC
#define SECTOR_LEN 264
//len of data part of sector
#define SECTOR_DATA_LEN 256
//start adr, start from second sector, first one will be busy
#define EEPROM_ADDR 0x000000
//fifo buf size in eeprom
#define FIFOSIZE 8



//max number of sectors for this type of eeprom
#define MAX_SECTORS 4096





struct tagEEPROMControl
{
	BYTE flashBuffer[SECTOR_LEN];
	//these buffers must not be changed when eeprom is busy
	BYTE arTrmBuffer[SECTOR_LEN+8];
	BYTE arRcvBuffer[SECTOR_LEN+8];
	DWORD dwTrmLen;
	DWORD dwTrmIdx;
	DWORD dwRcvLen;
	DWORD dwRcvIdx;


//add 24/09/2010	
	WORD wdEepromWritesCounter;//counter of eeprom writes. it is saved in time of device power off only
	
};

extern struct tagEEPROMControl EEPROMControl;

























void EEPROM_Init(void);
void EEPROM_SSP0_Init(void);

BOOL EEPROM_Busy(void);

void EEPROM_WriteBlok (unsigned int count,   // Длина
                const unsigned char  * RAM,     // Адрес в RAM
                DWORD EEPROM  // Адрес в EEPROM
                );    // Разрешение записи CRC
void EEPROM_ReadBlok (unsigned int         count,   // Длина
              unsigned char  * RAM,     // Адрес в RAM
              DWORD EEPROM);  // Адрес в EEPROM
void EEPROM_ReadFlash(int SectorNo, unsigned char * SectorBuffer);
void EEPROM_WriteFlash(int SectorNo, const unsigned char * SectorBuffer);
void EEPROM_WriteSector(int SectorNo, const unsigned char* SectorBuf);
BOOL EEPROM_ReadSector(int SectorNo, unsigned char* SectorBuf);
void EEPROM_powerOFFSSP(void);
void EEPROM_powerONSSP(void);


void EEPROM_StartTrm(BOOL bReceive);
void EEPROM_clearFIFO(void);

BYTE EEPROM_ReadStatus(void);


void EEPROM_WriteSystemSector(const unsigned char * SectorBuffer);
void EEPROM_ReadSystemSector(unsigned char * SectorBuffer);
void EEPROM_RegenerateFlash(int SectorNo);

void EEPROM_UpdateEssentialDataInEeprom(void);

#endif	//#ifndef _EEPROM_H
