//ini_control.c


#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <string.h>
#include <stdlib.h>


#include "eeprom.h"
#include "powerControl.h"
#include "display.h"
#include "interrupts.h"
#include "filesystem.h"
#include "YESNO_Mode.h"
#include "clock.h"
#include "identify.h"
#include "ini_control.h"
#include "bluetooth.h"
#include "gps.h"
#include "geiger.h"
#include "Spectrum.h"
#include "interProc.h"
#include "SPRD_Mode.h"
#include "sound.h"
#include "LOG_mode.h"
#include "SETUP_mode.h"





BOOL filesystem_restore_identify_ini(void)
{
	HFILE hfile = filesystem_create_file("identify", /*name of the file, will be found and rewrite*/
						"ini", /*ext of the file*/
						TRUE	/*1 then exists file will be overwritten*/
					  );
	if(hfile==NULL)return FALSE;
//		exception(__FILE__,__FUNCTION__,__LINE__,"No free memory");
	int file_pos = 0;

	int len = filesystem_file_put(hfile, /*file descriptor = number of file in file record table*/
							&file_pos, /*it is counter or written bytes, input: start from 0, as output ret pointer for the next non written byte*/
							  (BYTE*)identify_ini, /*send buffer*/
							  sizeof(identify_ini));/*buffer len in bytes*/
	
	return (BOOL)(len!=E_FAIL);
//		exception(__FILE__,__FUNCTION__,__LINE__,"filesystem error");
}




//restore system.ini from backup
BOOL filesystem_restore_system_ini_from_backup(void)
{
	HFILE hSrcFile = filesystem_open_file("system","bak");
	if(hSrcFile==NULL)return FALSE;
	//we have system.bak
	HFILE hfile = filesystem_create_file("system", /*name of the file, will be found and rewrite*/
						"ini", /*ext of the file*/
						TRUE	/*1 then exists file will be overwritten*/
					  );
	//we created system.ini
	if(hfile==NULL)return FALSE;
	int len=0;
	len = filesystem_get_length(hSrcFile);
	if(len==0)return FALSE;	//invalid len of bak file
	int ret = filesystem_copy_file_data(hfile, hSrcFile, 0, 0, len);
	return (ret!=E_FAIL);
}



//by default
BOOL filesystem_restore_system_ini(void)
{
	HFILE hfile = filesystem_create_file("system", /*name of the file, will be found and rewrite*/
						"ini", /*ext of the file*/
						TRUE	/*1 then exists file will be overwritten*/
					  );
	if(hfile==NULL)return FALSE;
		//exception(__FILE__,__FUNCTION__,__LINE__,"No free memory");
	int file_pos = 0;


	int len = filesystem_file_put(hfile, /*file descriptor = number of file in file record table*/
							&file_pos, /*it is counter or written bytes, input: start from 0, as output ret pointer for the next non written byte*/
							  (BYTE*)system_ini, /*send buffer*/
							  sizeof(system_ini));/*buffer len in bytes*/

	
	/////////////////
	//19/03/2010
	if(len!=E_FAIL)
	{
	//восстановим старые параметры гейгера мюллера, т.к. это важные параметры
		ini_write_system_ini_float("geigerControl", "fDrCoef", geigerControl.fDrCoef);
		ini_write_system_ini_float("geigerControl", "fDrSelfCps", geigerControl.fDrSelfCps);
		ini_write_system_ini_float("geigerControl", "fDrDeadTime", geigerControl.fDrDeadTime);
	}
	/////////////////
	
	
	return (BOOL)(len!=E_FAIL);
//		exception(__FILE__,__FUNCTION__,__LINE__,"filesystem error");
}


BOOL filesystem_restore_main_lib(void)
{
	HFILE hfile = filesystem_create_file("main", /*name of the file, will be found and rewrite*/
						"lib", /*ext of the file*/
						TRUE	/*1 then exists file will be overwritten*/
					  );
	if(hfile==NULL)return FALSE;
	//	exception(__FILE__,__FUNCTION__,__LINE__,"No free memory");
	int file_pos = 0;

	int len = filesystem_file_put(hfile, /*file descriptor = number of file in file record table*/
							&file_pos, /*it is counter or written bytes, input: start from 0, as output ret pointer for the next non written byte*/
							  (BYTE*)main_lib, /*send buffer*/
							  sizeof(main_lib));/*buffer len in bytes*/
	
	return (BOOL)(len!=E_FAIL);
	//	exception(__FILE__,__FUNCTION__,__LINE__,"filesystem error");
}



//create esential files
//create system.ini
//all section must be created here.
//absend section will cause exception
//all values must be created here.
//absend values will cause exception
void filesystem_check_ini_files(void)
{
	Display_setTextWrap(0);
	Display_setTextXY(0,0);	//set start coords in window
	Display_setCurrentFont(fnt8x16);	//set current font
	Display_setTextDoubleHeight(0);
	Display_setTextSteps(1,1);//set steps
	Display_setTextJustify(LEFT);

	Display_setTextColor(RED);	//set text color
	
	Display_setTextWin(0,Y_SCREEN_MAX-49,X_SCREEN_SIZE,49);	//set text window
	Display_clearTextWin(10);
	Display_outputTextByLang("Initialization...\r\0""Initialization...\r\0""Initialization...\r\0""Инициализация...\r");
	Display_setTextWin(0,Y_SCREEN_MAX-33,X_SCREEN_SIZE,33);	//set text window
	Display_outputTextByLang("Loading system files...\r\0""Loading system files...\r\0""Loading system files...\r\0""Загрузка системных файлов...\r");



	int ret = ini_read_system_ini("ini");
	if(ret==E_FAIL)
	{//no ini file, create it //!!!!!!!! here we must mark that file was recreated
		PowerControl_sleep(1000);
		Display_clearTextWin(10);
		Display_setTextXY(0,0);	//set start coords in window
		const char pMsg1[] = "Invalid system.ini\r\0""Fehlerhaft system.ini\r\0""Invalid system.ini\r\0""Испорчен system.ini\r";
		Display_outputTextByLang_log(pMsg1);
		
		if(!powerControl.bBatteryAlarm)
		{
			const char pMsg2[] = "Restoring from backup...\0""Erneuerung...\0""Restoring from backup...\0""Восстановление из bak...";
			Display_outputTextByLang_log(pMsg2);
			BOOL bNeedRest2 = FALSE;
			bNeedRest2 = !filesystem_restore_system_ini_from_backup();
			if(!bNeedRest2)
			{//restored from bak file
				ret = ini_read_system_ini("ini");
				if(ret==E_FAIL)//bak file is invalid
				{
					Display_clearTextWin(10);
					Display_setTextXY(0,0);	//set start coords in window
					const char pMsg1[] = "Invalid system.bak\r\0""Fehlerhaft system.bak\r\0""Invalid system.bak\r\0""Испорчен system.bak\r";
					Display_outputTextByLang_log(pMsg1);
//					PowerControl_sleep(1000);
//					Display_setTextXY(0,0);	//set start coords in window
					const char pMsg2[] = "GM and Neutron incorrect\0""GM and Neutron incorrect\0""GM and Neutron incorrect\0""ГМ и нейтр.некорректны ";
					Display_outputTextByLang_log(pMsg2);
					PowerControl_sleep(3000);
					bNeedRest2 = TRUE;
				}
			}else
			{
				Display_clearTextWin(10);
				Display_setTextXY(0,0);	//set start coords in window
				const char pMsg1[] = "Invalid system.ini\r\0""Fehlerhaft system.ini\r\0""Invalid system.ini\r\0""Испорчен system.ini\r";
				Display_outputTextByLang_log(pMsg1);
				const char pMsg2[] = "No backup file...\0""No backup file...\0""No backup file...\0""Нет bak файла...";
				Display_outputTextByLang_log(pMsg2);
				modeControl.bNoSystemBak = TRUE;
				PowerControl_sleep(3000);
			}
			if(bNeedRest2)
			{
				Display_clearTextWin(10);
				Display_setTextXY(0,0);	//set start coords in window
				const char pMsg1[] = "Invalid system.ini\r\0""Fehlerhaft system.ini\r\0""Invalid system.ini\r\0""Испорчен system.ini\r";
				Display_outputTextByLang_log(pMsg1);
				const char pMsg2[] = "System default...\0""System default...\0""System default...\0""Системные умолчания...";
				Display_outputTextByLang_log(pMsg2);
				if(filesystem_restore_system_ini())
				{
					ret = ini_read_system_ini("ini");
					if(ret==E_FAIL)
						exception(__FILE__,__FUNCTION__,__LINE__,"invalid default system.ini");
				}else
				{
					exception(__FILE__,__FUNCTION__,__LINE__,"failed to restore default system.ini");
				}
			}
		}else
		{
			;//!!!!!!!error
		}
	}
	
	//check system.bak
	HFILE hFile = filesystem_open_file("system","bak");
	if(hFile==NULL)
	{//mark that no system.bak
		modeControl.bNoSystemBak = TRUE;
	}
	//!!!!!!!! крайне не эффективно, т.к. один большой кусок повторяется как и предыдущий!!!!
	//update screen, we have read language settings
	Display_warmup_display_start();

	Display_setTextColor(RED);	//set text color
	Display_setCurrentFont(fnt8x16);	//set current font
	Display_setTextWin(0,Y_SCREEN_MAX-49,X_SCREEN_SIZE,49);	//set text window
	Display_clearTextWin(10);
	Display_outputTextByLang("Initialization...\r\0""Initialisierung...\r\0""Initialization...\r\0""Инициализация...\r");
	Display_setTextWin(0,Y_SCREEN_MAX-33,X_SCREEN_SIZE,33);	//set text window
	Display_outputTextByLang("Loading system files...\r\0""System files laden...\r\0""Loading system files...\r\0""Загрузка системных файлов...\r");
	////////////////////////////////////////////////////////////////////////////
	
	
	
	
	///////////////////////////////identify.ini//////////////////////////////////
	
	//read library from file name from identify_ini
	ret = identify_read_identify_ini();
	if(ret==E_FAIL)
	{
		modeControl.bIdentDefault = TRUE;
		//!!!!!!!!! need to show MSG about error
		PowerControl_sleep(1000);
		Display_clearTextWin(10);
		Display_setTextXY(0,0);	//set start coords in window
		const char pMsg1[] = "Invalid identify.ini\r\0""Fehlerhaft identify.ini\r\0""Invalid identify.ini\r\0""Испорчен identify.ini\r";
		Display_outputTextByLang_log(pMsg1);

		if(!powerControl.bBatteryAlarm)
		{

			const char pMsg2[]="Restoring...\0""Erneuerung...\0""Restoring...\0""Восстановление...";
			Display_outputTextByLang_log(pMsg2);
	
			if(filesystem_restore_identify_ini())
			{
				ret = identify_read_identify_ini();
				if(ret==E_FAIL)
					exception(__FILE__,__FUNCTION__,__LINE__,"invalid default identify.ini");
			}else
			{
				exception(__FILE__,__FUNCTION__,__LINE__,"failed to restore default identify.ini");
			}
		}else
		{
			;//!!!!!!!!error
		}
	}
	
	
	
	
	PowerControl_sleep(1000);
	Display_clearTextWin(10);
	Display_setTextXY(0,0);	//set start coords in window
	Display_outputTextByLang("Loading calibrations...\r\0""Eichendaten laden...\r\0""Loading calibrations...\r\0""Загрузка калибровок...\r");
	
	
	////////////////////////////energy.cal//////////////////////////////////////
	ret = Spectrum_read_energy_cal();
	if(ret==E_FAIL)
	{//
		modeControl.bNoEnergyCal = TRUE;
		PowerControl_sleep(1000);
		Display_clearTextWin(10);
		Display_setTextXY(0,0);	//set start coords in window
		const char pMsg1[] = "No energy.cal\r\0""Fehlen energy.cal\r\0""No energy.cal\r\0""Нет energy.cal\r";
		Display_outputTextByLang_log(pMsg1);
		const char pMsg2[]="Identification disabled\0""Identifizierung geblockt\0""Identification disabled\0""Идентификация запрещена";
		Display_outputTextByLang_log(pMsg2);
		
		HFILE hfile  = filesystem_open_file("energy","spz");
		if(hfile!=NULL)
			filesystem_delete_file(hfile);
	}
	if(ret!=E_FAIL)
	{
		//save energy cal as energy.spz
		for(int i=0;i<CHANNELS;i++)
			spectrumControl.acqSpectrum.dwarSpectrum[i] = spectrumControl.warEnergy[i];
		spectrumControl.acqSpectrum.wAcqTime = 1;
		int iret=Spectrum_save("energy", TRUE);
		//тут если не сапишется файл energy изза нехватки памяти то пипец!
		if(!iret)
		{//
			modeControl.bNoEnergySigmaSpz = TRUE;
	
			PowerControl_sleep(1000);
			Display_clearTextWin(10);
			Display_setTextXY(0,0);	//set start coords in window
			const char pMsg1[] = "No enough memory for system files\r\0""No enough memory for system files\r\0""No enough memory for system files\r\0""Нет памяти под системные файлы\r";
			Display_outputTextByLang_log(pMsg1);
			const char pMsg2[]="Computer Software can failed\0""Computer Software can failed\0""Computer Software can failed\0""Компьютерная программа может не работать";
			Display_outputTextByLang_log(pMsg2);
		}
	}
	
	/////////////////////////////sigma.cal//////////////////////////////////////////
	ret = Spectrum_read_sigma_cal();
	if(ret==E_FAIL)
	{//
		modeControl.bNoSigmaCal = TRUE;
		PowerControl_sleep(1000);
		Display_clearTextWin(10);
		Display_setTextXY(0,0);	//set start coords in window
		const char pMsg1[]="No sigma.cal\r\0""Fehlen sigma.cal\r\0""No sigma.cal\r\0""Нет sigma.cal\r";
		Display_outputTextByLang_log(pMsg1);
		const char pMsg2[] ="Identification disabled\0""Identifizierung geblockt\0""Identification disabled\0""Идентификация запрещена";
		Display_outputTextByLang_log(pMsg2);

		HFILE hfile  = filesystem_open_file("sigma","spz");
		if(hfile!=NULL)
			filesystem_delete_file(hfile);
	}
	if(ret!=E_FAIL)
	{
		//save sigma cal as sigma.spz
		for(int i=0;i<CHANNELS;i++)
			spectrumControl.acqSpectrum.dwarSpectrum[i] = spectrumControl.warSigma[i]
#ifdef _THIN_SIGMA
				*SIGMA_THIN_FACTOR_M/SIGMA_THIN_FACTOR_D
#endif	//#ifdef _THIN_SIGMA
		;
		spectrumControl.acqSpectrum.wAcqTime = 1;
		int iret=Spectrum_save("sigma", TRUE);
		//тут если не сапишется файл sigma изза нехватки памяти то пипец!
		if(!iret)
		{//
			modeControl.bNoEnergySigmaSpz = TRUE;
	
			PowerControl_sleep(1000);
			Display_clearTextWin(10);
			Display_setTextXY(0,0);	//set start coords in window
			const char pMsg1[] = "No enough memory for system files\r\0""No enough memory for system files\r\0""No enough memory for system files\r\0""Нет памяти под системные файлы\r";
			Display_outputTextByLang_log(pMsg1);
			const char pMsg2[]="Computer Software can failed\0""Computer Software can failed\0""Computer Software can failed\0""Компьютерная программа может не работать";
			Display_outputTextByLang_log(pMsg2);
//			Display_refresh();
		}
	}
	
	Spectrum_clear();
	
	
	
	
	
	/////////////////////////////////main.lib//////////////////////////////////////////
	
	PowerControl_sleep(1000);
	Display_clearTextWin(10);
	Display_setTextXY(0,0);	//set start coords in window
	Display_outputTextByLang("Loading nuclide library...\r\0""Nuklidbibliothek laden...\r\0""Loading nuclide library...\r\0""Загрузка библиотеки нуклидов...\r");

	

	ret = identify_open_library();
	
	//read library from file name from identify_ini
	if(ret==E_FAIL)
	{
		modeControl.bNoLibrary = TRUE;
		PowerControl_sleep(1000);
		Display_clearTextWin(10);
		Display_setTextXY(0,0);	//set start coords in window
		const char pMsg1[]="Invalid library file\r\0""Fehlerhaft bibliothek file\r\0""Invalid library file\r\0""Испорчен файл библиотеки\r";
		Display_outputTextByLang_log(pMsg1);
		const char pMsg2[]="Loading default...\0""Laden default...\0""Loading default...\0""Загрузка по умолчанию...";
		Display_outputTextByLang_log(pMsg2);

		PowerControl_sleep(1000);
		Display_clearTextWin(10);
		Display_setTextXY(0,0);	//set start coords in window
		Display_outputTextByLang_log("Loading main.lib...\r\0""main.lib laden...\r\0""Loading main.lib...\r\0""Загрузка main.lib...\r");
		
		strcpy(identifyControl.libraryFileName, "main");
		ret = identify_open_library();
		//read library from file name from identify_ini
		if(ret==E_FAIL)
		{
			PowerControl_sleep(1000);
			identifyControl.NUCLNUM = 0;	//reset to 0 if error
	
			const char pMsg1[]="Invalid main.lib\r\0""Fehlerhaft main.lib\r\0""Invalid main.lib\r\0""Испорчен main.lib\r";
			Display_clearTextWin(10);
			Display_setTextXY(0,0);	//set start coords in window
			Display_outputTextByLang_log(pMsg1);
			
			if(!powerControl.bBatteryAlarm)
			{
				const char pMsg2[]="Restoring...\0""Erneuerung...\0""Restoring...\0""Восстановление...";
				Display_outputTextByLang_log(pMsg2);
				//!!!!!!!!! need to show MSG about error
				if(filesystem_restore_main_lib())
				{
					ret = identify_open_library();
					if(ret==E_FAIL)
						exception(__FILE__,__FUNCTION__,__LINE__,"invalid default main.lib");
				}else
				{
				exception(__FILE__,__FUNCTION__,__LINE__,"failed to restore default main.lib");
					;//!!!!!!!!error
				}
			}else
			{
				;//!!!!!!!!error
			}
		}
	}
	

	////////////////////////////////////////////////////////////////////////////
}













//get string from the file from the end of string to the beginning
//ret string len
//string_len must be <MAX_INI_STRING_LEN
//if E_FAIL then error or file empty
//ret string without '\r' and '\0'
//ret 0 if empty string
int filesystem_get_stringReverse(HFILE hFile, int* pFile_pos, char* pString, int string_len)
{
	if(string_len>MAX_INI_STRING_LEN)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Requested string is too long");
	}
	char buf[MAX_INI_STRING_LEN];
	memset(buf,0,MAX_INI_STRING_LEN);
	int pos = *pFile_pos - MAX_INI_STRING_LEN+2;
	if(pos<0)pos=0;
	int pos2 = pos;
	int ln = *pFile_pos-pos+1;
	if(ln<1)return E_FAIL;
	if(ln>=MAX_INI_STRING_LEN)ln=MAX_INI_STRING_LEN-1;
	int rlen = filesystem_file_get(hFile, &pos, buf, ln);
	if(rlen==E_FAIL)return E_FAIL;
	if(rlen==0)
	{
		pString[0]='\0';
		return rlen;
	}
	char* pAdr = strrchr(buf, '\r');
	if(!pAdr)
	{//may be it is last string or string is too long
		char* pAdr0 = strchr(buf, '\0');
		pAdr = pAdr0;
		if(!pAdr)return E_FAIL;	//string is too long
//		if(*pAdr!='\0')return E_FAIL;	//last 0
		//it is last string
	}
	
	char* pAdr2 = pAdr;
	while(--pAdr2>=buf && *pAdr2!='\r');
	pAdr2++;
	//copy string
	int retlen = pAdr-pAdr2;
	if(retlen>string_len)return E_FAIL;	//empty string
	if(retlen==0)
	{//empty string
		pString[0]=pAdr2[0];
		pString[1]='\0';
	}else
	{
		memcpy(pString,pAdr2,retlen);
		if(retlen<string_len)
			pString[retlen]='\0';
	}
//	pos = *pFile_pos;
//	pos -= retlen+1;
	pos2 += pAdr2-buf-1;
	if(pos2<0)pos2=0;
	*pFile_pos = pos2;
	return retlen;
}















//get string from the file
//ret string len
//string_len must be <MAX_INI_STRING_LEN
//if E_FAIL then error or file empty
//ret string without '\r' and '\0'
//ret 0 if empty string
int filesystem_get_string(HFILE hFile, int* pFile_pos, char* pString, int string_len)
{
	if(string_len>MAX_INI_STRING_LEN)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Requested string is too long");
	}
	char buf[MAX_INI_STRING_LEN];
	memset(buf,0,MAX_INI_STRING_LEN);
	int pos = *pFile_pos;
	int rlen = filesystem_file_get(hFile, &pos, buf, MAX_INI_STRING_LEN);
	if(rlen==E_FAIL)return E_FAIL;
	if(rlen==0)
	{
		pString[0]='\0';
		return rlen;
	}
	char* pAdr = strchr(buf, '\r');
	if(!pAdr)
	{//may be it is last string or string is too long
		pAdr = strchr(buf, '\0');
		if(!pAdr)return E_FAIL;	//string is too long
//		if(*pAdr!='\0')return E_FAIL;	//last 0
		//it is last string
	}
	//copy string
	int retlen = pAdr-buf;
	if(retlen>string_len)return E_FAIL;	//empty string
	if(retlen==0)
	{//empty string
		pString[0]=buf[0];
		pString[1]='\0';
	}else
	{
		memcpy(pString,buf,retlen);
		if(retlen<string_len)
			pString[retlen]='\0';
	}
	*pFile_pos += retlen+1;
	return retlen;
}

/*
//put string in a file
//ret new len of a file
int filesystem_put_string(HFILE hFile, char* pString)
{
	int pos = filesystem_get_length(hFile);
	int rlen = filesystem_file_put(hFile, &pos, (BYTE*)pString, strlen(pString));
	if(rlen==E_FAIL)return E_FAIL;
	return rlen+pos;
}
*/


//get value in int format from the txt file, depends on section and value name
//ret S_OK
//if ret E_FAIL then empty file, no section, no value
int filesystem_ini_get_int(HFILE hFile, const char* pSection, const char* pName, int* pValue)
{
	char buf[MAX_INI_STRING_LEN];
	memset(buf, 0, sizeof(buf));
	int rlen = filesystem_ini_get_string(hFile, pSection, pName, buf, sizeof(buf));
	if(rlen==E_FAIL)return E_FAIL;
	*pValue = atoi(buf);
	return S_OK;
}



//put value in int format in the txt file, depends on section and value name
//ret S_OK
//if ret E_FAIL then empty file, no section, no value
int filesystem_ini_put_int(HFILE hFile, const char* pSection, const char* pName, int value)
{
	char buf[MAX_INI_STRING_LEN];
	sprintf(buf, "%d", value);
	int rlen = filesystem_ini_put_string(hFile, pSection, pName, buf);
	if(rlen==E_FAIL)return E_FAIL;
	return S_OK;
}

//get value in float format from the txt file, depends on section and value name
//ret S_OK
//if ret E_FAIL then empty file, no section, no value
int filesystem_ini_get_float(HFILE hFile, const char* pSection, const char* pName, float* pValue)
{
	char buf[MAX_INI_STRING_LEN];
	memset(buf, 0, sizeof(buf));
	int rlen = filesystem_ini_get_string(hFile, pSection, pName, buf, sizeof(buf));
	if(rlen==E_FAIL)return E_FAIL;
	*pValue = atof(buf);
	return S_OK;
}



//put value in float format in the txt file, depends on section and value name
//ret S_OK
//if ret E_FAIL then empty file, no section, no value
int filesystem_ini_put_float(HFILE hFile, const char* pSection, const char* pName, float value)
{
	char buf[MAX_INI_STRING_LEN];
	sprintf(buf, "%f", value);
	int rlen = filesystem_ini_put_string(hFile, pSection, pName, buf);
	if(rlen==E_FAIL)return E_FAIL;
	return S_OK;
}


//get value in txt from the txt file, depends on section and value name
//ret value len
//if ret E_FAIL then empty file, no section, no value
int filesystem_ini_get_string(HFILE hFile, const char* pSection, const char* pName, char* pValue, int value_len)
{
	int file_pos = 0;
	
	file_pos = file_system_find_section(hFile, pSection);
	if(file_pos==E_FAIL)return E_FAIL;
	
	file_pos = filesystem_find_value(hFile, file_pos, pName);
	if(file_pos==E_FAIL)return E_FAIL;

	memset(pValue, 0, value_len);
	int rlen = filesystem_get_string(hFile, &file_pos, pValue, value_len);
	if(rlen==E_FAIL)return E_FAIL;
	return strlen(pValue);
}


//find value in txt file by name in one section
//ret pos of value mean
//ret E_FAIL if error or no found
int filesystem_find_value(HFILE hFile, int start_file_pos, const char* pName)
{
	char buf[MAX_INI_STRING_LEN];
	int buflen = sizeof(buf);
	int name_len = strlen(pName);
	int file_pos;
	int stop_file_pos = 0;
	file_pos = start_file_pos;
	if(filesystem.file_pos_last_value>=0)//will try to search from the last value
		file_pos = filesystem.file_pos_last_value;
	do
	{
		PowerControl_kickWatchDog();
		
		int rlen = filesystem_get_string(hFile, &file_pos, buf, buflen);
		if(rlen==E_FAIL ||
		   (stop_file_pos>0 && file_pos>stop_file_pos) ||
			  buf[0]=='[')
		{
			//выход на втором цикле поиска
			if(filesystem.file_pos_last_value>=0)
			{
				stop_file_pos = filesystem.file_pos_last_value;	//для раннего выхода из второго круга
				filesystem.file_pos_last_value = -1;
				file_pos = start_file_pos;
				if(filesystem.file_pos_last_value>=0)//will try to search from the last value
					file_pos = filesystem.file_pos_last_value;
				continue;
			}else
				return E_FAIL;
		}
		if(buf[0]==';')continue;	//remark
		//look for value
		char* pAdr = strstr(buf, pName);
		if(!pAdr)continue;
		if(pAdr[name_len]!='=' && pAdr[name_len]!=' ')continue;
		pAdr = strchr(pAdr, '=');
		if(!pAdr)continue;
		//value found
		//pass spaces
		do
		{
			pAdr++;
		}while(*pAdr==' ');
		file_pos = file_pos-rlen+(pAdr-buf-1);
		filesystem.file_pos_last_value = file_pos;
		return file_pos;
	}while(TRUE);
	return E_FAIL;
}



//find section in txt file
//ret pos of next line after section
//ret E_FAIL if error or no found
//реализован алгоритм поиска секции последовательно от последней использованной.
//но если ничего не будет найдено, то тогда поиск начнется с начала, что займет монго больше времени
int file_system_find_section(HFILE hFile, const char* pSection)
{
	//check if we are already in this section
	if(strcmp(filesystem.lastSectionName, pSection)==0 && filesystem.file_pos_last_section>=0)
		return filesystem.file_pos_last_section;
	
	filesystem.file_pos_last_value = -1;
	
	char buf[MAX_INI_STRING_LEN];
	int buflen = sizeof(buf);
	int file_pos;
	int stop_file_pos = 0;
	
	file_pos = 0;
	if(filesystem.file_pos_last_section>=0)//will try to search from the last section
		file_pos = filesystem.file_pos_last_section;
	int section_len = strlen(pSection);
	do
	{
		
		PowerControl_kickWatchDog();
		
		int rlen = filesystem_get_string(hFile, &file_pos, buf, buflen);
		if(rlen==E_FAIL || (stop_file_pos>0 && file_pos>stop_file_pos))
		{
			//выход на втором цикле поиска
			if(filesystem.file_pos_last_section>=0)
			{
				stop_file_pos = filesystem.file_pos_last_section;	//для раннего выхода из второго круга
				filesystem.file_pos_last_section = -1;
				filesystem.file_pos_last_value = -1;
				file_pos = 0;
				if(filesystem.file_pos_last_section>=0)//will try to search from the last section
					file_pos = filesystem.file_pos_last_section;
				continue;
			}else
				return E_FAIL;
		}
		if(buf[0]==';')continue;	//remark
		if(buf[0]!='[')continue;	//not a section start
		char* pAdr = strstr(buf, pSection);
		if(!pAdr)continue;
		if(pAdr[section_len]!=']')continue;
		//found section
		strcpy(filesystem.lastSectionName, pSection);
		filesystem.file_pos_last_section = file_pos;
		return file_pos;
	}while(TRUE);
	return E_FAIL;
}


//put value in txt to the txt file, depends on section and value name
//if OK then ret S_OK
//if ret E_FAIL then no section, no value, empty file, several faults
int filesystem_ini_put_string(HFILE hFile, const char* pSection, const char* pName, const char* pValue)
{
	int file_pos = 0;
	
	file_pos = file_system_find_section(hFile, pSection);
	if(file_pos==E_FAIL)return E_FAIL;
	
	file_pos = filesystem_find_value(hFile, file_pos, pName);
	if(file_pos==E_FAIL)return E_FAIL;

	int fpos = file_pos;
	char buf[MAX_INI_STRING_LEN];
	int buflen = sizeof(buf);
	int rlen = filesystem_get_string(hFile, &fpos, buf, buflen);
	if(rlen==E_FAIL)return E_FAIL;
	int value_len = strlen(pValue);
	if(value_len>=buflen)return E_FAIL;
	int ret = filesystem_cut_file(hFile, file_pos, rlen-value_len);
	if(ret!=S_OK)return E_FAIL;
	int wlen = filesystem_file_put(hFile, &file_pos, (const unsigned char*)pValue, value_len);
	if(wlen==E_FAIL)return E_FAIL;//failed to write file or no free memory
	return S_OK;
}






//read settings from ini
//ret E_FAIL if error
int ini_read_system_ini(char* pExt)
{
	//read library file from identify.ini
	HFILE hfile = filesystem_open_file("system", /*name of the file, will be found*/
		   pExt /*ext of the file*/
			   );
	if(hfile==NULL)
		return E_FAIL;

	int rVal;
	int ret = filesystem_ini_get_int(hfile, "modeControl", "bLang", &rVal);
	if(ret==E_FAIL)
		return E_FAIL;
	modeControl.bLang = rVal;
	ret = filesystem_ini_get_int(hfile, "modeControl", "bSysDefault", &rVal);
	if(ret==E_FAIL)
		return E_FAIL;
	modeControl.bSysDefault = rVal;
	ret = filesystem_ini_get_int(hfile, "powerControl", "dwPowerDownDeadTime", &rVal);
	if(ret==E_FAIL)
		return E_FAIL;
	powerControl.dwPowerDownDeadTime = rVal;
	
	ret = filesystem_ini_get_float(hfile, "powerControl", "fBatCoef", &powerControl.fBatCoef);	
	if(ret==E_FAIL)
		return E_FAIL;
	if(powerControl.fBatCoef<MIN_BAT_COEF)
		powerControl.fBatCoef = MIN_BAT_COEF;
	if(powerControl.fBatCoef>MAX_BAT_COEF)
		powerControl.fBatCoef = MAX_BAT_COEF;

#ifndef GPS_BT_FREE	
	
	ret = filesystem_ini_get_int(hfile, "bluetoothControl", "comm_state", &rVal);	
	if(ret==E_FAIL)
		return E_FAIL;
	bluetoothControl.comm_state = rVal;
	ret = filesystem_ini_get_int(hfile, "bluetoothControl", "comm_power_state", &rVal);	
	if(ret==E_FAIL)
		return E_FAIL;
	bluetoothControl.comm_power_state = rVal;

	ret = filesystem_ini_get_int(hfile, "GPSControl", "gps_state", &rVal);	
	if(ret==E_FAIL)
		return E_FAIL;
	GPSControl.gps_state = rVal;
#endif	//#ifndef GPS_BT_FREE	

	ret = filesystem_ini_get_float(hfile, "geigerControl", "fDrCoef", &geigerControl.fDrCoef);	
	if(ret==E_FAIL)
		return E_FAIL;
	ret = filesystem_ini_get_float(hfile, "geigerControl", "fDrSelfCps", &geigerControl.fDrSelfCps);	
	if(ret==E_FAIL)
		return E_FAIL;
	ret = filesystem_ini_get_float(hfile, "geigerControl", "fDrDeadTime", &geigerControl.fDrDeadTime);	
	if(ret==E_FAIL)
		return E_FAIL;

	ret = filesystem_ini_get_int(hfile, "interProcControl", "wdHighLimit", &rVal);	
	if(ret==E_FAIL)
		return E_FAIL;
	interProcControl.rsModbus.wdHighLimit = rVal;
	ret = filesystem_ini_get_int(hfile, "interProcControl", "wdLowLimit", &rVal);	
	if(ret==E_FAIL)
		return E_FAIL;
	interProcControl.rsModbus.wdLowLimit = rVal;
	
/*	ret = filesystem_ini_get_float(hfile, "interProcControl", "fStabGain", &interProcControl.rsModbus.fStabGain);	
	if(ret==E_FAIL)
		return E_FAIL;*/

	ret = filesystem_ini_get_float(hfile, "SPRDModeControl", "fDRThreshold", &SPRDModeControl.fDRThreshold);	
	if(ret==E_FAIL)
		return E_FAIL;

/*	ret = filesystem_ini_get_int(hfile, "SPRDModeControl", "iDimension", &rVal);	
	if(ret==E_FAIL)
		return E_FAIL;
	SPRDModeControl.iDimension = rVal;*/
	
/*	ret = filesystem_ini_get_float(hfile, "SPRDModeControl", "operation_search_sigma", &SPRDModeControl.operation_search_sigma);	
	if(ret==E_FAIL)
		return E_FAIL;

	ret = filesystem_ini_get_float(hfile, "SPRDModeControl", "sleepmode_search_sigma", &SPRDModeControl.sleepmode_search_sigma);	
	if(ret==E_FAIL)
		return E_FAIL;*/

	ret = filesystem_ini_get_int(hfile, "SPRDModeControl", "false_alarm_period_oper", &rVal);	
	if(ret==E_FAIL)
		return E_FAIL;
	SPRDModeControl.false_alarm_period_oper = rVal;

/*	ret = filesystem_ini_get_int(hfile, "SPRDModeControl", "false_alarm_period_sleep", &rVal);	
	if(ret==E_FAIL)
		return E_FAIL;
	SPRDModeControl.false_alarm_period_sleep = rVal;*/

	ret = filesystem_ini_get_int(hfile, "soundControl", "bSound", &rVal);
	if(ret==E_FAIL)
		return E_FAIL;
	soundControl.bSound = rVal;
	
	InterProc_setLowLimit(interProcControl.rsModbus.wdLowLimit);
	InterProc_setHighLimit(interProcControl.rsModbus.wdHighLimit);
	//!!!!!!!!!!! must be switch to selected dimension

//	SPRDModeControl.sleepmode_search_sigma = SETUPMode_calcSigma(SPRDModeControl.false_alarm_period_sleep);
	SETUPMode_calcSigma(SPRDModeControl.false_alarm_period_oper);
	
	//must be setup sigma thresholds
	InterProc_setSigmaSearchThresholds();
//	InterProc_setSigmaSleepThresholds(SPRDModeControl.sleepmode_search_sigma);
	
	
	return S_OK;
}


//write int value to system  ini
//if failed then exception
BOOL ini_write_system_ini_int(const char * pSection, const char* pValueName, int value)
{
	//read library file from identify.ini
	HFILE hfile = filesystem_open_file("system", /*name of the file, will be found*/
		   "ini" /*ext of the file*/
			   );
	if(hfile==NULL)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid system.ini");
	}

	int ret = filesystem_ini_put_int(hfile, pSection, pValueName, value);
	return (BOOL)(ret!=E_FAIL);
}

//write int value to system  ini
//if failed then exception
BOOL ini_write_system_ini_float(const char * pSection, const char* pValueName, float value)
{
	//read library file from identify.ini
	HFILE hfile = filesystem_open_file("system", /*name of the file, will be found*/
		   "ini" /*ext of the file*/
			   );
	if(hfile==NULL)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid system.ini");
	}

	int ret = filesystem_ini_put_float(hfile, pSection, pValueName, value);
	return (BOOL)(ret!=E_FAIL);
}


//default system.ini
const char system_ini[394]=
";system.ini\r"
"[modeControl]\r"
"bLang=0\r"
"bSysDefault=1\r"
"[powerControl]\r"
"dwPowerDownDeadTime=120\r"
"fBatCoef=1.0\r"
"[bluetoothControl]\r"
"comm_state=65537\r"	//be careful of default value = 0x10000 (start value) + 1 (item number)
"comm_power_state=65536\r"	//be careful of default value = 0x10000 (start value) + 1 (item number)
"[GPSControl]\r"
"gps_state=65538\r"	//be careful of default value = 0x10000 (start value) + 2 (item number)
"[geigerControl]\r"
#ifdef BNC
"fDrCoef=0.45\r"
"fDrSelfCps=0.02\r"
"fDrDeadTime=53.97000e-6\r"
"[interProcControl]\r"
"wdHighLimit=4060\r"
"wdLowLimit=40\r"
"[SPRDModeControl]\r"
"fDRThreshold=10\r"
#else
"fDrCoef=4.5\r"
"fDrSelfCps=0.02\r"
"fDrDeadTime=53.97000e-6\r"
"[interProcControl]\r"
"wdHighLimit=4060\r"
"wdLowLimit=40\r"
"[SPRDModeControl]\r"
"fDRThreshold=100\r"
#endif
"false_alarm_period_oper=600\r"
"[soundControl]\r"
"bSound=1\r"
;
/*
"operation_search_sigma=5.03\r"
"sleepmode_search_sigma=5.03\r"
*/



//ret filled items num or 0 if error
//there is used index-value order
int ini_retrieveTable(HFILE hfile, struct tagIndexMeanTable*  pTable)
{
#define TABLE_STRING_LEN 50	
	char nums[]=".0123456789";
	int itemsnum = 0;
	char buf[TABLE_STRING_LEN+1];
	char strbuf[10];
	int filepos = 0;
	int len;
	while((len = filesystem_get_string(hfile, &filepos, buf, TABLE_STRING_LEN))!=E_FAIL)
	{
		if(!len)continue;
		//have a string
		memset(strbuf,0,10);
		buf[len] = '\0';
		int next = strcspn(buf, nums);//found digits
		if(next==len)continue;	//just empty string
		char* pbuf = &buf[next];
		next = strspn(pbuf, nums);	//found not digits
		if(next==strlen(pbuf))return 0;	//error!!! thrash in the file
		strncpy(strbuf, pbuf ,next);	//cut digits
		pTable[itemsnum].index = atof(strbuf);
		pbuf = &pbuf[next];
		next = strcspn(pbuf, nums);	//found next digits
		if(next==strlen(pbuf))return 0;	//error!!! thrash in the file
		pbuf = &pbuf[next];
		strncpy(strbuf, pbuf, 10);
		pTable[itemsnum].mean = atof(strbuf);
		itemsnum++;
	};
	return itemsnum;
}
