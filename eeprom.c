//eeprom.c

#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <string.h>


#include "eeprom.h"
//#include "powerControl.h"
//#include "display.h"
#include "interrupts.h"
//#include "bluetooth.h"
#include "setup_mode.h"
#include "USBRS.h"



struct tagEEPROMControl EEPROMControl;






void EEPROM_Init(void)
{
	DIR_WP = 1;
	DIR_RES = 1;
	CLR_RES;
	SET_RES;
	DIR_ECS = 1;
	
}

void EEPROM_SSP0_Init(void)
{
	EEPROM_powerONSSP();
	
	
	//PINSEL0_bit.P0_6 = 2;	//EECS -> SSEL1
	PINSEL0_bit.P0_7 = 2;	//EESCK -> SCK1
	PINSEL0_bit.P0_8 = 2;	//EESO -> MISO1
	PINSEL0_bit.P0_9 = 2;	//EESI -> MOSI1
	
	//int turn them off
	__sspimsc_bits sspimcs;
	sspimcs.RORIM = 0;	//Receive Overrun occurs
	sspimcs.RTIM = 0;	//Receive Timeout condition occur
	sspimcs.RXIM = 0;	//Rx FIFO is at least half full
	sspimcs.TXIM = 0;	//Tx FIFO is at least half empty
	SSP1IMSC_bit = sspimcs;
	
	SSP1ICR = 3;	//clear all INT
	
	__sspcr0_bits ssp1cr0;
	ssp1cr0.DSS = 7;	//8 bit transfer
	ssp1cr0.FRF = 0;	//Frame format SPI
	ssp1cr0.SPO = 0;	//SSP controller maintains the bus clock low between frames
/*SSP controller captures serial data on the second clock
transition of the frame, that is, the transition back to the
inter-frame state of the clock line.*/
	ssp1cr0.SPH = 0;
	ssp1cr0.SCR = 0;
	SSP1CR0_bit = ssp1cr0;
	
/*The SSP controller acts as a master on the bus, driving the
SCLK, MOSI, and SSEL lines and receiving the MISO line.*/
	__sspcr1_bits ssp1cr1;
	ssp1cr1.MS = 0;
	ssp1cr1.LBM = 0;	//normal operation
	ssp1cr1.SOD = 0;	//normal operation
	SSP1CR1_bit = ssp1cr1;
	
	SSP1CPSR = 2;
	
	EEPROM_clearFIFO();
	
	SSP1CR1_bit.SSE = 1;	//enable
}

//clear fifo buf in eeprom
void EEPROM_clearFIFO(void)
{
	BYTE Dummy;
/*	for (int i = 0; i < FIFOSIZE; i++ )
	{
		if(!SSP1SR_bit.RNE)break;
		Dummy = SSP1DR;		
	}*/
	while(SSP1SR_bit.RNE)
	{
		Dummy = SSP1DR;		/* clear the RxFIFO */
	};
}







//read one sector from eeprom with chech crc
BOOL EEPROM_ReadSector(int SectorNo, unsigned char* SectorBuf)
{
	//26/12/2011
	//добавлено перечитывание при сбое чтения
	BOOL ret;
	for(int i=0;i<3;i++)
	{
		EEPROM_ReadFlash(SectorNo, EEPROMControl.flashBuffer);
		if(!SectorBuf)return 1;
		memcpy(SectorBuf, EEPROMControl.flashBuffer, SECTOR_DATA_LEN);
		ret = USBRS_checkCRC(EEPROMControl.flashBuffer, 258);
		if(ret)break;
	}
	return ret;
}
//write sector to eeprom with chech crc
void EEPROM_WriteSector(int SectorNo, const unsigned char* SectorBuf)
{
	memcpy(EEPROMControl.flashBuffer, SectorBuf, SECTOR_DATA_LEN);
	USBRS_calcCRC(EEPROMControl.flashBuffer, 256);
	EEPROM_WriteFlash(SectorNo, EEPROMControl.flashBuffer);
}


//write sector to eeprom with no chech crc
void EEPROM_WriteFlash(int SectorNo, const unsigned char * SectorBuffer)
{
	EEPROM_WriteBlok(SECTOR_LEN, SectorBuffer,
		(ULONG)EEPROM_ADDR+(ULONG)SectorNo*(ULONG)SECTOR_LEN_TRUE);
}

//read one sector from eeprom with no chech crc
void EEPROM_ReadFlash(int SectorNo, unsigned char * SectorBuffer)
{
	EEPROM_ReadBlok (SECTOR_LEN,   // Длина
      SectorBuffer,     // Адрес в RAM
      (ULONG)EEPROM_ADDR+(ULONG)SectorNo*(ULONG)SECTOR_LEN_TRUE);
}

//write system sector to eeprom with no chech crc
void EEPROM_WriteSystemSector(const unsigned char * SectorBuffer)
{
	EEPROM_WriteBlok(SECTOR_LEN, SectorBuffer,
		(ULONG)0);
}

//read system sector from eeprom with no chech crc
void EEPROM_ReadSystemSector(unsigned char * SectorBuffer)
{
	EEPROM_ReadBlok (SECTOR_LEN,   // Длина
      SectorBuffer,     // Адрес в RAM
      (ULONG)0);
}


//regeneration of sector
void EEPROM_RegenerateFlash(int SectorNo)
{
	DWORD EEPROM = (ULONG)EEPROM_ADDR+(ULONG)SectorNo*(ULONG)SECTOR_LEN_TRUE;
	EEPROMControl.arTrmBuffer[0]=0x58;
	EEPROMControl.arTrmBuffer[1]=*((unsigned char *)&EEPROM+2);
	EEPROMControl.arTrmBuffer[2]=*((unsigned char *)&EEPROM+1);
	EEPROMControl.arTrmBuffer[3]=*((unsigned char *)&EEPROM+0);
	EEPROMControl.dwTrmLen = 4;
	EEPROM_StartTrm(0);
}



//read a block of memory in one sector
void EEPROM_ReadBlok (unsigned int         count,   // Длина
              unsigned char  * RAM,     // Адрес в RAM
              DWORD EEPROM)  // Адрес в EEPROM
{
	EEPROMControl.arTrmBuffer[0]=0xD2;
	EEPROMControl.arTrmBuffer[1]=*((unsigned char *)&EEPROM+2);
	EEPROMControl.arTrmBuffer[2]=*((unsigned char *)&EEPROM+1);
	EEPROMControl.arTrmBuffer[3]=*((unsigned char *)&EEPROM+0);
	EEPROMControl.arTrmBuffer[4]=0xff;
	EEPROMControl.arTrmBuffer[5]=0xff;
	EEPROMControl.arTrmBuffer[6]=0xff;
	EEPROMControl.arTrmBuffer[7]=0xff;
	EEPROMControl.dwTrmLen = 8;
	EEPROMControl.dwRcvLen = count;
	EEPROM_StartTrm(1);

	if(RAM)
		for(UINT i=0;i<count;i++)
			RAM[i] = EEPROMControl.arRcvBuffer[i];
}



void EEPROM_WriteBlok (unsigned int count,   // Длина
                const unsigned char  * RAM,     // Адрес в RAM
                DWORD EEPROM  // Адрес в EEPROM
                )    // Разрешение записи CRC

{
	EEPROMControl.arTrmBuffer[0] = 0x53;
	EEPROMControl.arTrmBuffer[1] = *((unsigned char *)&EEPROM+2);
	EEPROMControl.arTrmBuffer[2] = *((unsigned char *)&EEPROM+1);
	EEPROMControl.arTrmBuffer[3] = *((unsigned char *)&EEPROM+0);
	EEPROMControl.dwTrmLen = 4;
	EEPROM_StartTrm(0);
		
	EEPROMControl.arTrmBuffer[0] = 0x82;
	EEPROMControl.arTrmBuffer[1] = *((unsigned char *)&EEPROM+2);
	EEPROMControl.arTrmBuffer[2] = *((unsigned char *)&EEPROM+1);
	EEPROMControl.arTrmBuffer[3] = *((unsigned char *)&EEPROM+0);
	
	for(UINT i=0;i<count;i++)
		EEPROMControl.arTrmBuffer[4+i]=RAM[i];
	
	EEPROMControl.dwTrmLen = count+4;
	EEPROM_StartTrm(0);
	
	//add 24/09/2010
	EEPROMControl.wdEepromWritesCounter++;

}



//turn on SSP interface
void EEPROM_powerONSSP(void)
{
	PCONP_bit.PCSSP1 = 1;
}
//turn OFF SSP
void EEPROM_powerOFFSSP(void)
{
	PCONP_bit.PCSSP1 = 0;
}


//read eeprom status
BYTE EEPROM_ReadStatus(void)
{
	EEPROM_clearFIFO();
	CLR_ECS;
	SSP1DR = 0xd7;
	SSP1DR = 0xff;
	while(!SSP1SR_bit.RNE);
	BYTE status1 = SSP1DR;
	while(!SSP1SR_bit.RNE);
	status1 = SSP1DR;
	SET_ECS;
	return status1;
}


//ret 1 if SSP is busy
//ret 0 if SSP idle
/*BOOL EEPROM_SSP_Busy(void)
{
	return (SSP1SR_bit.BSY);
}*/

//ret TRUE is eeprom is busy
BOOL EEPROM_Busy(void)
{
	return !(EEPROM_ReadStatus()&0x80);
}


//start transmition data to eeprom
//and receiving data if bReceive is set to 1
void EEPROM_StartTrm(BOOL bReceive)
{
	//wait while busy
	while(EEPROM_Busy());
	
	
	CLR_ECS;
	

	BYTE* pBuf = EEPROMControl.arTrmBuffer;
	for(int i=0;i<EEPROMControl.dwTrmLen;i++)
	{
		while(!SSP1SR_bit.TNF);
		SSP1DR = *pBuf++;
	};
	

	while(!SSP1SR_bit.TFE);
	
	if(bReceive)
	{
		EEPROM_clearFIFO();
		BYTE* pBuf = EEPROMControl.arRcvBuffer;
		for(int i=0;i<EEPROMControl.dwRcvLen;i++)
		{
			SSP1DR = 0xff;
			while(!SSP1SR_bit.RNE);
			*pBuf++ = SSP1DR;
		};
		
	}
	
	SET_ECS;
}




//update essential data in eeprom
void EEPROM_UpdateEssentialDataInEeprom(void)
{
	BYTE buf[SECTOR_LEN];
	memset(buf, 0, sizeof(buf));
	
	//19/03/2010
	EEPROM_ReadSector(0, &buf[0]);
	
	EEPROMControl.wdEepromWritesCounter++;
	////////////
	//первые 7 байт это серийный номер месяц и год выпуска
	buf[0] = (BYTE)(SETUPModeControl.Serial&0xff);
	buf[1] = (BYTE)((SETUPModeControl.Serial>>8)&0xff);
	buf[2] = (BYTE)((SETUPModeControl.Serial>>16)&0xff);
	buf[3] = (BYTE)((SETUPModeControl.Serial>>24)&0xff);
	buf[4] = (BYTE)(SETUPModeControl.ManufacturedMonth);
	buf[5] = (BYTE)(SETUPModeControl.ManufacturedYear&0xff);
	buf[6] = (BYTE)((SETUPModeControl.ManufacturedYear>>8)&0xff);
	buf[7] = (BYTE)(EEPROMControl.wdEepromWritesCounter&0xff);
	buf[8] = (BYTE)((EEPROMControl.wdEepromWritesCounter>>8)&0xff);
	buf[9] = (BYTE)modeControl.bLang;
	EEPROM_WriteSector(0, &buf[0]);
}

