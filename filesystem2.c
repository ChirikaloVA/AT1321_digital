//filesystem2.c

#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <string.h>


#include "eeprom.h"
#include "powerControl.h"
#include "display.h"
#include "interrupts.h"
#include "filesystem.h"
#include "YESNO_Mode.h"
#include "clock.h"
#include "identify.h"
#include "ini_control.h"
#include "gps.h"


















//can be run once per start or rarely
//restoring claster table, find lost clasters and mark them empty
//losten clasters are from files that was saved but not closed
/*
заложен следующий механизм:
1.проверка всех секторов на формат
1.1. если сектор дает сбой, то переформатирование всей фс
2.проверка файлов по таблице файлов
2.1. если таблица файлов дает сбой, то переформатирование всей фс
2.2. если последовательность для файла ошибочна, то удаляется файл
3. поиск потерянных кластеров и воостановление их статуса пустых
*/
void filesystem_check_and_restore(void)
{
	Display_setTextWin(0,Y_SCREEN_MAX-49,X_SCREEN_SIZE,49);	//set text window
	Display_setTextWrap(0);
	Display_setTextXY(0,0);	//set start coords in window
	Display_setCurrentFont(fnt8x16);	//set current font
	Display_setTextDoubleHeight(0);
	Display_setTextSteps(1,1);//set steps
	Display_setTextJustify(LEFT);
	COLORREF clr = RED;
	Display_setTextColor(clr);	//set text color
	Display_clearTextWin(10);
	Display_outputTextByLang("Initialization...\r\0""Initialization...\r\0""Initialization...\r\0""Инициализация...\r");
	Display_setTextWin(0,Y_SCREEN_MAX-33,X_SCREEN_SIZE,33);	//set text window
	
	//add 24/09/2010
	if(EEPROMControl.wdEepromWritesCounter>9500)
	{//regenerate eeprom
		for(int i=0;i<MAX_SECTORS;i++)
		{
			if((i%50)==0)
			{
				Display_clearTextWin(10);
				Display_setTextXY(0,0);	//set start coords in window
				Display_outputTextByLang("Regenerating memory...\r\0""Regenerating memory...\r\0""Regenerating memory...\r\0""Регенерация памяти...\r");
				Display_outputTextByLang("progress \0""progress \0""progress \0""выполнение ");
				char str[10];
				sprintf(str, "%d%%", (int)(i*100/MAX_SECTORS));
				Display_setTextJustify(NONE);
				Display_outputText(str);
				Display_setTextJustify(LEFT);
			}
			PowerControl_kickWatchDog();
			EEPROM_RegenerateFlash(i);
		}
		EEPROMControl.wdEepromWritesCounter = 0;
		//save essential data in eeprom (it is a eeprom writes counter )
		EEPROM_UpdateEssentialDataInEeprom();
	}
	
	Display_clearTextWin(10);
	Display_setTextXY(0,0);	//set start coords in window
	//end add 24/09/2010
	
	Display_outputTextByLang("Checking memory...\r\0""Checking memory...\r\0""Checking memory...\r\0""Проверка памяти...\r");
	
	BOOL bShowOnce = FALSE;
	BOOL bError = FALSE;
	
	//потому как мы испортили содержимое clasterBuffer
	filesystem.iCurrentFileClaster=-1;
	filesystem.iCurrentFileClasterSector=-1;
	
	//check claster table and try to restore it before check sectors
	for(int i=FILES_CLASTER_START_NO;i<=FILES_CLASTER_END_NO;i++)
	{
		PowerControl_kickWatchDog();
		filesystem_get_clasterIndex(i);
	}

	//потому как мы испортили содержимое clasterBuffer
	filesystem.iCurrentFileClaster=-1;
	filesystem.iCurrentFileClasterSector=-1;

	//check files table and try to restore it before check sectors
	for(int i=FILERECORD_START_NO;i<=FILERECORD_END_NO;i++)
	{
		PowerControl_kickWatchDog();
		filesystem_get_filerecordIndex(i);
	}

	//потому как мы испортили содержимое clasterBuffer
	filesystem.iCurrentFileClaster=-1;
	filesystem.iCurrentFileClasterSector=-1;
	

	Display_clearTextWin(10);
	Display_setTextXY(0,0);	//set start coords in window
	Display_outputTextByLang("Checking files...\r\0""Checking files...\r\0""Checking files...\r\0""Проверка файлов...\r");
	
#pragma pack(1)
	BYTE claster_buffer[MAX_CLASTERS];	//0-not used, 1-used
#pragma pack()
	memset(&claster_buffer[0], 0, MAX_CLASTERS);

	bShowOnce = FALSE;
	
	for(int k=FILERECORD_START_NO;k<=FILERECORD_END_NO;k++)
	{

		if(bError)break;
		
		PowerControl_kickWatchDog();

		int index = filesystem_get_filerecordIndex(k);
		if(index==E_FAIL)
		{//error in filerecord, bad record in eeprom, all files of this records must be deleted
			bError = 1;	//restore everything
			break;
		}else
		{//found
			int startclaster = filesystem.fileRecordsOnSector[index].wdStartClaster;
			if(startclaster==CLASTERTABLE_EMPTY_RECORD)
			{//empty, no file, go to the next one
				continue;
			}else if(startclaster<FILES_CLASTER_START_NO || startclaster>FILES_CLASTER_END_NO)
			{//error in file record
				//здесь можно просто пометить кластер файла как пустой и удалится только файл и не надо переформатировать всю фс
				//при этом образуются потерянные кластеры
				filesystem.fileRecordsOnSector[index].wdStartClaster = CLASTERTABLE_EMPTY_RECORD;
				filesystem_write_filerecordstable();
				//bError = 1;	//mark that whole file system needs reformat
				Display_clearTextWin(10);
				Display_outputTextByLang("Delete bad files...\0""Abspeichern schlecht files...\0""Delete bad files...\0""Удаление плохих файлов...");
//				Display_refresh();
				bShowOnce = TRUE;
//				break;
			}else
			{//have a file
				BOOL itsend =FALSE;
				do
				{
					if(startclaster==CLASTERTABLE_EMPTY_RECORD ||	//bad file
					   (startclaster!=CLASTERTABLE_EMPTY_RECORD &&
					   claster_buffer[startclaster]))//error, this claster is already used
					{//bad file, have to delete it
						if(!bShowOnce)
						{
							Display_outputTextByLang("Deleting bad files...\0""Abspeichern schlecht files...\0""Deleting bad files...\0""Удаление плохих файлов...");
							bShowOnce = TRUE;
						}
						filesystem_delete_file(index+1);
						break;
					}
					//ok, go ahead, mark as used
					claster_buffer[startclaster]=1;
					int cindex = filesystem_get_clasterIndex(startclaster);
					if(cindex==E_FAIL)
						exception(__FILE__,__FUNCTION__,__LINE__,"invalid claster record");
						
					int newstartclaster = filesystem.clasterTableOnSector[cindex];

					if(newstartclaster<FILES_CLASTER_START_NO || newstartclaster>FILES_CLASTER_END_NO)
					{//error in file record
//						filesystem.clasterTableOnSector[index] = CLASTERTABLE_EMPTY_RECORD;
	//					filesystem_write_clastertable();
//						bError = 1;	//mark that whole file system needs reformat
						//break;
						startclaster = CLASTERTABLE_EMPTY_RECORD;
						itsend = FALSE;
						continue;	//go up to remove file
					}
					itsend = newstartclaster==startclaster;
					startclaster = newstartclaster;
				}while(!itsend);
			}
		}
	}
	
	
	Display_clearTextWin(10);
	Display_setTextXY(0,0);	//set start coords in window
	Display_outputTextByLang("Checking memory...\r\0""Checking memory...\r\0""Checking memory...\r\0""Проверка памяти...\r");
	
	//check sector's format by CRC
	for(int i=CLASTERTABLE_START_SECTOR_NO;i<MAX_SECTORS;i++)
	{
		
		PowerControl_kickWatchDog();
		if(!EEPROM_ReadSector(i, (BYTE*)&filesystem.clasterBuffer[0]))
		{//bad sector, format it
			if(!bShowOnce)
			{
				Display_outputTextByLang("Memory formatting...\0""Datenspeicher formatieren...\0""Memory formatting...\0""Форматирование памяти...");
				bShowOnce = TRUE;
			}
//we dont need to ask to reformat the whole file system that is maybe empy space only
//			bError = 1;	//mark that whole file system needs reformat
			if(!powerControl.bBatteryAlarm)	//when battery discharged no file operation is allowed
			{
				for(int j=0;j<SECTOR_DATA_LEN;j++)
					filesystem.clasterBuffer[j] = 0xff;
				EEPROM_WriteSector(i, (BYTE*)&filesystem.clasterBuffer[0]);
				if(!EEPROM_ReadSector(i, (BYTE*)&filesystem.clasterBuffer[0]))	//check after write
				{
					exception(__FILE__,__FUNCTION__,__LINE__,"EEPROM failed!");
				}
			}
		}
	}
	//потому как мы испортили содержимое clasterBuffer
	filesystem.iCurrentFileClaster=-1;
	filesystem.iCurrentFileClasterSector=-1;

	
	PowerControl_sleep(500);
	
	
	if(bError)
	{//request for restore everything
		YESNOMode_DoModal(RED,"ERROR!\0""FEHLER!\0""ERROR!\0""ОШИБКА!",
		"File system FAILED! It is recommended to restore factory defaults!\rAll files will be lost!\r\rConfirm...\0"
		"File system FAILED! It is recommended to restore factory defaults!\rAll files will be lost!\r\rConfirm...\0"
		"File system FAILED! It is recommended to restore factory defaults!\rAll files will be lost!\r\rConfirm...\0"
		"СБОЙ Файловой системы! Рекомендовано восстановить заводские установки!\rВсе данные будут удалены!\r\rПодтвердите...",
		&filesystem_format_filetable);
	}else
	{//look for not used clasters and clean them

		if(!powerControl.bBatteryAlarm)//when battery discharged no file operation is allowed
		{
			Display_clearTextWin(10);
			Display_setTextXY(0,0);	//set start coords in window
			Display_outputTextByLang("Cleaning memory...\r\0""Datenspeicher reinigung...\r\0""Cleaning memory...\r\0""Очистка памяти...\r");

			BOOL bNeedSave = 0;
			for(int i=FILES_CLASTER_START_NO;i<=FILES_CLASTER_END_NO;i++)
			{
				if(!claster_buffer[i])
				{//not used
					int sector = CLASTERTABLE_START_SECTOR_NO+i/CLASTERTABLE_RECORDS_ON_SECTOR;
					if(sector!=filesystem.iCurrentClasterTableSector && bNeedSave)
					{
						filesystem_write_clastertable();
						bNeedSave = 0;
					}
	
					int index = filesystem_get_clasterIndex(i);
					if(index==E_FAIL)
						exception(__FILE__,__FUNCTION__,__LINE__,"invalid claster record");
					
					if(filesystem.clasterTableOnSector[index]!=CLASTERTABLE_EMPTY_RECORD)
					{//not empty claster but no file associated
						filesystem.clasterTableOnSector[index] = CLASTERTABLE_EMPTY_RECORD;
						bNeedSave = 1;
					}
				}
			}
			if(bNeedSave)
			{
				filesystem_write_clastertable();
				bNeedSave = 0;
			}
		}
		PowerControl_sleep(500);
	}
}


//erase whole file system
void filesystem_format_filetable(BOOL bYes)
{
	if(!bYes)return;
	
	if(powerControl.bBatteryAlarm)return;	//when battery discharged no file operation is allowed
	
	//!!!!!!!!!!! be careful everything will be lost
	//restore claster table
	for(int j=0;j<CLASTERTABLE_RECORDS_ON_SECTOR;j++)
		filesystem.clasterTableOnSector[j] = CLASTERTABLE_EMPTY_RECORD;
	for(int i=0;i<CLASTERTABLE_SECTOR_AREA_LEN;i++)
	{//rewrite all clasters
		filesystem.iCurrentClasterTableSector = i+CLASTERTABLE_START_SECTOR_NO;
		filesystem_write_clastertable();
		PowerControl_kickWatchDog();
	}
	//restore filerecord table
	for(int j=0;j<FILERECORD_RECORDS_ON_SECTOR;j++)
		filesystem.fileRecordsOnSector[j].wdStartClaster = CLASTERTABLE_EMPTY_RECORD;
	for(int i=0;i<FILERECORD_SECTOR_AREA_LEN;i++)
	{//rewrite all clasters
		filesystem.iCurrentFileRecordSector = i+FILERECORD_START_SECTOR_NO;
		filesystem_write_filerecordstable();
		PowerControl_kickWatchDog();
	}
	filesystem.iCurrentClasterTableSector = -1;
	filesystem.iCurrentFileRecordSector = -1;
}






//copy bytes from one file to another from one position to another
//if ret E_FAIL then no free memory
int filesystem_copy_file_data(HFILE hFileDst, HFILE hFileSrc, int file_pos_dst, int file_pos_src, int len)
{
	BYTE buf[CLASTER_DATA_LEN];
	int pass_clasters = len/CLASTER_DATA_LEN;
	int pass_bytes = len%CLASTER_DATA_LEN;
	for(int i=0;i<pass_clasters;i++)
	{
		int rlen = filesystem_file_get(hFileSrc, &file_pos_src, buf, CLASTER_DATA_LEN);
		if(rlen==E_FAIL)return E_FAIL;//failed to read file or file empty
		if(rlen!=CLASTER_DATA_LEN)
		{
			exception(__FILE__,__FUNCTION__,__LINE__,"Internal error!");
		}
		int wlen = filesystem_file_put(hFileDst, &file_pos_dst, buf, rlen);
		if(wlen==E_FAIL)return E_FAIL;//failed to write file or no free memory
		PowerControl_kickWatchDog();
	}
	int rlen = filesystem_file_get(hFileSrc, &file_pos_src, buf, pass_bytes);
	if(rlen==E_FAIL)return E_FAIL;//failed to read file or file empty
	if(rlen!=pass_bytes)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Internal error!");
	}
	int wlen = filesystem_file_put(hFileDst, &file_pos_dst, buf, rlen);
	if(wlen==E_FAIL)return E_FAIL;//failed to write file or no free memory
	return S_OK;
}












//cut file on bytes_cut if >0, grows file if <0
//procedure is going next: create temp file, save all data to temp file ith cut, delete original, rename temp to original
//ret cur len of file
//file_pos<file_len and bytes_cut<file_len
//ret E_FAIL if no free memory
int filesystem_cut_file(HFILE hFile, /*file that will be cut*/
						 int file_pos, /*file pos from cut*/
						 int bytes_cut /*bytes to cut >0, if <0 then grows*/
							 )
{
	if(bytes_cut==0)return S_OK;
	
	int flen = filesystem_get_length(hFile);
	if(file_pos>=flen)
	{
		//invalid file pos
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid parameters");
	}
	if(bytes_cut>=flen)
	{
		//invalid bytes_cut
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid parameters");
	}
	char newFileName[FILE_NAME_SZ];
	char temp[FILE_NAME_SZ];
	sprintf(newFileName, "tmp_%s", Clock_getClockDateTimeStrEx(temp));
	HFILE hNewFile = filesystem_create_file(newFileName,"tmp", 1);
	if(hNewFile==NULL)return E_FAIL;
	//read from old file and save to the new
	//copy unchanged data byte to byte
	int ret = filesystem_copy_file_data(hNewFile, hFile, 0, 0, file_pos);
	if(ret==E_FAIL)return E_FAIL;
	//we saved all data upto cut position
	//now we save data depends on cutting bytes number
	int newpos = file_pos+bytes_cut;
	ret = filesystem_copy_file_data(hNewFile, hFile, file_pos, newpos, flen-newpos);
	if(ret==E_FAIL)return E_FAIL;
	/////////////
	filesystem_delete_file(hFile);
	//reside newHFILE to the prev HFILE
	int idx = filesystem_get_first_filerecordIndex(hNewFile);
	int cls_idx = filesystem.fileRecordsOnSector[idx].wdStartClaster;
	DWORD src_len = filesystem.fileRecordsOnSector[idx].dwLength;
	int idx_dst = filesystem_get_first_filerecordIndex(hFile);
	//reside data from temp file to the deleted file, and now it is restored
	filesystem.fileRecordsOnSector[idx_dst].wdStartClaster = cls_idx;
	filesystem.fileRecordsOnSector[idx_dst].dwLength = src_len;
	memcpy(&filesystem.fileRecordsOnSector[idx_dst].dateTime,
		   &filesystem.fileRecordsOnSector[idx].dateTime,
		   sizeof(struct tagDateTime));
	filesystem_write_filerecordstable();
	//delete temp file
	idx = filesystem_get_first_filerecordIndex(hNewFile);
	filesystem.fileRecordsOnSector[idx].wdStartClaster = CLASTERTABLE_EMPTY_RECORD;
	filesystem_write_filerecordstable();
	return S_OK;
}





//подсичтать количество специальных файлов
void filesystem_calc_special_files_number(UINT* pAllFilesNum, 
					UINT* pSPZFilesNum,
					UINT* pLIBFilesNum,
					UINT* pMCSFilesNum,
					UINT* pLOGFilesNum)
{
	UINT allfiles=0;
	UINT spzfiles=0;
	UINT mcsfiles=0;
	UINT logfiles=0;
	UINT libfiles=0;
	for(int i=0;i<FILERECORD_SECTOR_AREA_LEN;i++)
	{
		if(!filesystem_read_filerecordstable(i+FILERECORD_START_SECTOR_NO))
		{
			exception(__FILE__,__FUNCTION__,__LINE__,"Filesystem failed");
		}
		for(int j=0;j<FILERECORD_RECORDS_ON_SECTOR;j++)
		{
			if(filesystem.fileRecordsOnSector[j].wdStartClaster!=CLASTERTABLE_EMPTY_RECORD)
			{
				allfiles++;
				if(!strncmp(filesystem.fileRecordsOnSector[j].ext, "spz", FILE_EXT_SZ))
					spzfiles++;
				else
				if(!strncmp(filesystem.fileRecordsOnSector[j].ext, "lib", FILE_EXT_SZ))
					libfiles++;
				else
				if(!strncmp(filesystem.fileRecordsOnSector[j].ext, "mcs", FILE_EXT_SZ))
					mcsfiles++;
				else
				if(!strncmp(filesystem.fileRecordsOnSector[j].ext, "log", FILE_EXT_SZ))
					logfiles++;
			}
		}
	}
	*pAllFilesNum=allfiles;
	*pSPZFilesNum=spzfiles;
	*pLIBFilesNum=libfiles;
	*pMCSFilesNum=mcsfiles;
	*pLOGFilesNum=logfiles;
}


//calc number of files
//pExt - filter by extension if not NULL then calc only those files
UINT filesystem_calc_files_number(char* pExt)
{
	UINT files=0;
	for(int i=0;i<FILERECORD_SECTOR_AREA_LEN;i++)
	{
		if(!filesystem_read_filerecordstable(i+FILERECORD_START_SECTOR_NO))
		{
			exception(__FILE__,__FUNCTION__,__LINE__,"Filesystem failed");
		}
		for(int j=0;j<FILERECORD_RECORDS_ON_SECTOR;j++)
		{
			if(filesystem.fileRecordsOnSector[j].wdStartClaster!=CLASTERTABLE_EMPTY_RECORD)
			{
				if(!pExt)
					files++;
				else if(!strncmp(filesystem.fileRecordsOnSector[j].ext, pExt, FILE_EXT_SZ))
					files++;
			}
		}
	}
	return files;
}



//ret free memory in bytes
UINT filesystem_calc_free_memory(void)
{
	UINT membusy=0;
	for(int i=0;i<CLASTERTABLE_SECTOR_AREA_LEN;i++)
	{
		if(!filesystem_read_clastertable(i+CLASTERTABLE_START_SECTOR_NO))
		{
			exception(__FILE__,__FUNCTION__,__LINE__,"Filesystem failed");
		}
		for(int j=0;j<CLASTERTABLE_RECORDS_ON_SECTOR;j++)
		{
			if(filesystem.clasterTableOnSector[j]!=CLASTERTABLE_EMPTY_RECORD)membusy++;
		}
	}
	return ((FILES_CLASTER_AREA_LEN-membusy)*CLASTER_DATA_LEN);
}





