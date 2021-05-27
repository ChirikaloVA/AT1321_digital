//filesystem.c

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
#include "SETUP_Mode.h"
#include "clock.h"


struct tagFileSystem filesystem;







void filesystem_Init(void)
{
	//mean current sector of claster table if 0xff then no claster table was loaded
	//if -1 then no file record was loaded
	filesystem.iCurrentClasterTableSector = -1;	//nothing loaded by default
	//mean current sector of file record
	//if -1 then no file record was loaded
	filesystem.iCurrentFileRecordSector = -1;	//nothing loaded by default
	filesystem.iCurrentFileClaster = -1;
	filesystem.iCurrentFileClasterSector = -1;

	filesystem.bMemoryLow = FALSE;

	filesystem.file_pos_last_value = -1;
	filesystem.file_pos_last_section = -1;
	memset(filesystem.lastSectionName,0 , sizeof(filesystem.lastSectionName));
	filesystem.hFileExecuteSys = NULL;


	//here we read internal non erasable data with serial number and so on.
	//get serial number
	/*
	7 байт используютс€ дл€ серийного номера мес€ца и года выпуска
	2 байта дл€ счетчика перезаписей еепром
	1 байт дл€ призначка €зыка интерфейса пользовател€
	*/
	BYTE buf[SECTOR_LEN];
	if(EEPROM_ReadSector(0, &buf[0]))
	{//yes we have it
		SETUPModeControl.Serial = ((DWORD)buf[3]<<24)|((DWORD)buf[2]<<16)|((DWORD)buf[1]<<8)|((DWORD)buf[0]);
		SETUPModeControl.ManufacturedMonth = buf[4];
		SETUPModeControl.ManufacturedYear = ((WORD)buf[6]<<8)|((WORD)buf[5]);
		//add 24/09/2010
		EEPROMControl.wdEepromWritesCounter = ((WORD)buf[8]<<8)|((WORD)buf[7]);
		modeControl.bLang = (int)buf[9];
	}else
	{//set settings by default in RAM
		if(!modeControl.bNoSysSettings)
		{
			modeControl.bNeedSetTrue_NoSysSettings = TRUE;
		}
//		MCA.usREGs[0xA7-0x80] = 600;	//default contrast
	}
}




















/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//filerecord work

//ret index of record in fileRecordsOnSector
//if E_FAIL then error
//unsafe function for read claster, if it is bad then no exception
int filesystem_get_filerecordIndex(int filerecord)
{
	if(filerecord<FILERECORD_START_NO || filerecord>FILERECORD_END_NO)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"filerecord is out of range");
		return E_FAIL;
	}
	int sector_num = FILERECORD_START_SECTOR_NO+filerecord/FILERECORD_RECORDS_ON_SECTOR;
	BOOL ret = filesystem_read_filerecordstable(sector_num);
	int index;
	if(ret)
		index = filerecord%FILERECORD_RECORDS_ON_SECTOR;
	else//error
		index = E_FAIL;
	return index;
}




//read a part of file records table by sector number
//ret 0 if no file records at this sector
//else 1
BOOL filesystem_read_filerecordstable(int sector_num)
{
	if(sector_num<FILERECORD_START_SECTOR_NO || sector_num>=FILERECORD_START_SECTOR_NO+FILERECORD_SECTOR_AREA_LEN)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"sector number is out of range");
		return 0;
	}
	BOOL ret = 1;
	if(filesystem.iCurrentFileRecordSector != sector_num)
	{
		ret = EEPROM_ReadSector(sector_num, (BYTE*)&filesystem.fileRecordsOnSector[0]);
		if(!ret)
		{//сбой основной копии таблицы файлов. восстановление из резерва
			ret = EEPROM_ReadSector(sector_num+FILERECORD_SECTOR_AREA_LEN, (BYTE*)&filesystem.fileRecordsOnSector[0]);
			if(ret)
				EEPROM_WriteSector(sector_num,(BYTE*)&filesystem.fileRecordsOnSector[0]);
			else
				;//сбой резерва таблицы файлов
		}
		if(ret)
			filesystem.iCurrentFileRecordSector = sector_num;
	}
	return ret;
}

//write a part of file records table by sector number in glob variables
BOOL filesystem_write_filerecordstable(void)
{
	if(filesystem.iCurrentFileRecordSector<FILERECORD_START_SECTOR_NO || filesystem.iCurrentFileRecordSector>=FILERECORD_START_SECTOR_NO+FILERECORD_SECTOR_AREA_LEN)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"sector number is out of range");
		return 0;
	}
	EEPROM_WriteSector(filesystem.iCurrentFileRecordSector,(BYTE*)&filesystem.fileRecordsOnSector[0]);
	//запсиь в резерв
	EEPROM_WriteSector(filesystem.iCurrentFileRecordSector+FILERECORD_SECTOR_AREA_LEN,(BYTE*)&filesystem.fileRecordsOnSector[0]);
	return 1;
}

//get file record table index with exception processing
//safe function, it is guarantee that index will be OK
int filesystem_get_first_filerecordIndex(HFILE hFile)
{
	int findex = hFile-1;
	int index = filesystem_get_filerecordIndex(findex);
	if(index==E_FAIL)
	{//error in filerecord, bad record in eeprom, all files of this records must be deleted
		exception(__FILE__,__FUNCTION__,__LINE__,"invalid file record");
	}
	return index;
}

//find free record in filerecord table
//ret E_FAIL if no filerecords is free
//else filerecord index
int filesystem_find_free_filerecord(void)
{
	int k;
	for(k=FILERECORD_START_NO;k<=FILERECORD_END_NO;k++)
	{
		PowerControl_kickWatchDog();
		int index = filesystem_get_filerecordIndex(k);
		if(index==E_FAIL)
		{//error in filerecord, bad record in eeprom, all files of this records must be deleted
			exception(__FILE__,__FUNCTION__,__LINE__,"invalid file record");
		}else
		{//found
			if(filesystem.fileRecordsOnSector[index].wdStartClaster==CLASTERTABLE_EMPTY_RECORD)break;
		}
	}
	if(k>FILERECORD_END_NO)
		return E_FAIL;
	else
		return k;
}

//filerecord work
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


























/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//claster work



//read a part of claster table
BOOL filesystem_read_clastertable(int sector)
{
	if((sector<(CLASTERTABLE_START_SECTOR_NO)) || (sector>=(CLASTERTABLE_START_SECTOR_NO+CLASTERTABLE_SECTOR_AREA_LEN)))
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"sector number is out of range");
		return 0;
	}
	BOOL ret = 1;
	if(filesystem.iCurrentClasterTableSector != sector)
	{
		ret = EEPROM_ReadSector(sector,(BYTE*)&filesystem.clasterTableOnSector[0]);
		if(!ret)
		{
			//сбой в главной копии, восстановление из резерва
			ret = EEPROM_ReadSector(sector+CLASTERTABLE_SECTOR_AREA_LEN,(BYTE*)&filesystem.clasterTableOnSector[0]);
			if(ret)
				EEPROM_WriteSector(sector,(BYTE*)&filesystem.clasterTableOnSector[0]);
			else
				;//сбой резервной копии
		}
		if(ret)
		{
			filesystem.iCurrentClasterTableSector = sector;
		}
	}
	return ret;
}

//write a part of claster table in sector that is in glob variables
BOOL filesystem_write_clastertable(void)
{
	if(filesystem.iCurrentClasterTableSector<CLASTERTABLE_START_SECTOR_NO || filesystem.iCurrentClasterTableSector>=CLASTERTABLE_START_SECTOR_NO+CLASTERTABLE_SECTOR_AREA_LEN)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"sector number is out of range");
		return 0;
	}
	EEPROM_WriteSector(filesystem.iCurrentClasterTableSector,(BYTE*)&filesystem.clasterTableOnSector[0]);
	//дублировать запись сектора кластерной таблицы
	EEPROM_WriteSector(filesystem.iCurrentClasterTableSector+CLASTERTABLE_SECTOR_AREA_LEN,(BYTE*)&filesystem.clasterTableOnSector[0]);
	return 1;
}

//read one claster of some file
BOOL filesystem_read_claster(int claster_number)
{
	if(claster_number<FILES_CLASTER_START_NO || claster_number>FILES_CLASTER_END_NO)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"claster number is out of range");
		return FALSE;
	}
	BOOL ret  = TRUE;
	if(claster_number!=filesystem.iCurrentFileClaster)
	{//!!!!!!! здесь есть опасность что если надо перечитать один и тот же кластер, то это не будет сделано
		int sector = claster_number*CLASTER_SZ_IN_SECTORS;
		for(int i=0;i<CLASTER_SZ_IN_SECTORS;i++)
			ret &= EEPROM_ReadSector(sector+i, &filesystem.clasterBuffer[i*SECTOR_DATA_LEN]);
		if(ret)
		{
			filesystem.iCurrentFileClaster = claster_number;
			filesystem.iCurrentFileClasterSector = sector;
		}
	}
	return ret;
}

//write one claster of some file
BOOL filesystem_write_claster(int claster_number)
{
	if(claster_number<FILES_CLASTER_START_NO || claster_number>FILES_CLASTER_END_NO)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"claster number is out of range");
		return 0;
	}
	int sector = claster_number*CLASTER_SZ_IN_SECTORS;
	for(int i=0;i<CLASTER_SZ_IN_SECTORS;i++)
		EEPROM_WriteSector(sector+i, &filesystem.clasterBuffer[i*SECTOR_DATA_LEN]);
	filesystem.iCurrentFileClaster = claster_number;
	filesystem.iCurrentFileClasterSector = sector;
	return 1;
}


//ret index of claster in clasterTableOnSector
//if E_FAIL then error
int filesystem_get_clasterIndex(int claster_number)
{
	if(claster_number<FILES_CLASTER_START_NO || claster_number>FILES_CLASTER_END_NO)
		exception(__FILE__,__FUNCTION__,__LINE__,"claster_number is out of range");
	int sector = CLASTERTABLE_START_SECTOR_NO+claster_number/CLASTERTABLE_RECORDS_ON_SECTOR;
	BOOL ret = filesystem_read_clastertable(sector);
	int index;
	if(ret)
		index = claster_number%CLASTERTABLE_RECORDS_ON_SECTOR;
	else//error
		index = E_FAIL;
	return index;
}

//find free claster in claster table for FILES ONLY
//ret E_FAIL if no claster is free
//else claster index
//notThisClaster used to pass this claster
//if notThisClaster==CLASTERTABLE_EMPTY_RECORD then procedure will take first free
int filesystem_find_free_claster(int notThisClaster)
{
	int k;
	for(k=FILES_CLASTER_START_NO;k<=FILES_CLASTER_END_NO;k++)
	{
		PowerControl_kickWatchDog();
		int index = filesystem_get_clasterIndex(k);
		if(index==E_FAIL)
		{//error in filerecord, bad record in eeprom, all files of this records must be deleted
			exception(__FILE__,__FUNCTION__,__LINE__,"invalid file record");
		}else
		{//found
			if(k!=notThisClaster && filesystem.clasterTableOnSector[index]==CLASTERTABLE_EMPTY_RECORD)break;
		}
	}
	if(k>FILES_CLASTER_END_NO)k=E_FAIL;	//to make result NULL
	return k;
}

//claster work
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






















/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//file work





//find a file by name and extention
HFILE filesystem_find_file(const char* pFileName, /*name of the file, will be found and rewrite*/
							const char* pFileExt /*ext of the file*/
						  )
{
	int k;
	for(k=FILERECORD_START_NO;k<=FILERECORD_END_NO;k++)
	{
		PowerControl_kickWatchDog();
		int index = filesystem_get_filerecordIndex(k);
		if(index==E_FAIL)
		{//error in filerecord, bad record in eeprom, all files of this records must be deleted
			exception(__FILE__,__FUNCTION__,__LINE__,"invalid file record");
		}else
		{//found
			if(filesystem.fileRecordsOnSector[index].wdStartClaster==CLASTERTABLE_EMPTY_RECORD)continue;
			int cmp = strncmp(filesystem.fileRecordsOnSector[index].name, pFileName, FILE_NAME_SZ);
			int cmp2 = strncmp(filesystem.fileRecordsOnSector[index].ext, pFileExt, FILE_EXT_SZ);
			if(!(cmp|cmp2))break;
		}
	}
	if(k>FILERECORD_END_NO)
		return NULL;
	else
		return (HFILE)(k+1);
}





//remove file
//ret S_OK if OK
//ret E_FAIL if file is already deleted or battery discharged
HRESULT filesystem_delete_file(HFILE hfile)
{
	if(powerControl.bBatteryAlarm)return E_FAIL;	//when battery discharged no file operation is allowed

	int index = filesystem_get_first_filerecordIndex(hfile);
	WORD startclaster = filesystem.fileRecordsOnSector[index].wdStartClaster;
	if(startclaster==CLASTERTABLE_EMPTY_RECORD)
	{//no such file
		return E_FAIL;
	}
	BOOL bItsEnd = FALSE;
	do
	{
		PowerControl_kickWatchDog();
		int clasterindex = filesystem_get_clasterIndex(startclaster);
		if(clasterindex==E_FAIL)
		{
			exception(__FILE__,__FUNCTION__,__LINE__,"invalid claster record");
			return E_FAIL;
		}else
		{
			int newstartclaster = filesystem.clasterTableOnSector[clasterindex];
			bItsEnd = newstartclaster==startclaster;
			startclaster = newstartclaster;
			if(startclaster!=CLASTERTABLE_EMPTY_RECORD)
			{
				filesystem.clasterTableOnSector[clasterindex] = CLASTERTABLE_EMPTY_RECORD;	//change and write
				filesystem_write_clastertable();
			}
		}
	}while(!bItsEnd && startclaster!=CLASTERTABLE_EMPTY_RECORD);
	filesystem.fileRecordsOnSector[index].wdStartClaster = CLASTERTABLE_EMPTY_RECORD;
	filesystem_write_filerecordstable();
	return (HRESULT)S_OK;
}




//create new file in directory
//ret NULL if file is already exists and bOverwrite = 0 or no free space or battery discharged
//HFILE is a index of file in filerecord table +1
HFILE filesystem_create_file(const char* pFileName, /*name of the file, will be found and rewrite*/
							const char* pFileExt, /*ext of the file*/
							BOOL bOverwrite	/*1 then exists file will be overwritten*/
						  )
{
	if(powerControl.bBatteryAlarm)return NULL;	//when battery discharged no file operation is allowed

	HFILE hfile = filesystem_find_file(pFileName, pFileExt);
	if(hfile!=NULL)
	{
		if(!bOverwrite)return NULL;//already exists and no overwrite allow
		//remove this file
		filesystem_delete_file(hfile);
	}
	int freefilerecord = filesystem_find_free_filerecord();
	if(freefilerecord==E_FAIL)return NULL;//no free space in filerecord
	int index = filesystem_get_filerecordIndex(freefilerecord);
	if(index==E_FAIL)
	{//error in filerecord, bad record in eeprom
		exception(__FILE__,__FUNCTION__,__LINE__,"invalid file record");
		return NULL;
	}
	memset(filesystem.fileRecordsOnSector[index].name, 0, FILE_NAME_SZ);
	strncpy(filesystem.fileRecordsOnSector[index].name,pFileName,FILE_NAME_SZ);
	memset(filesystem.fileRecordsOnSector[index].ext, 0, FILE_EXT_SZ);
	strncpy(filesystem.fileRecordsOnSector[index].ext,pFileExt,FILE_EXT_SZ);
	memcpy(&filesystem.fileRecordsOnSector[index].dateTime, (void const*)&clockData.dateTime, sizeof(struct tagDateTime));
	filesystem.fileRecordsOnSector[index].dwLength = 0;
	return (HFILE)(freefilerecord+1);
}

//rename file
int filesystem_rename_file(const char* pFileName, /*name of the file*/
							const char* pFileExt, /*ext of the file*/
						   const char* pNewFileName, /*new name of the file*/
							const char* pNewFileExt /*new ext of the file*/
								)
{
	if(!pFileName || !pFileExt || !pNewFileName || !pNewFileExt)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid parameters");
	}

	if(powerControl.bBatteryAlarm)return E_FAIL;	//when battery discharged no file operation is allowed

	HFILE hfile = filesystem_find_file(pFileName, pFileExt);
	if(hfile==NULL)return E_FAIL;	//no file
	HFILE hfile2 = filesystem_find_file(pNewFileName, pNewFileExt);
	if(hfile2!=NULL)return E_FAIL;	//new file name is exists
	int index = filesystem_get_first_filerecordIndex(hfile);
	memset(filesystem.fileRecordsOnSector[index].name, 0,FILE_NAME_SZ);
	strncpy(filesystem.fileRecordsOnSector[index].name, pNewFileName,FILE_NAME_SZ);
	memset(filesystem.fileRecordsOnSector[index].ext, 0,FILE_EXT_SZ);
	strncpy(filesystem.fileRecordsOnSector[index].ext, pNewFileExt,FILE_EXT_SZ);
	filesystem_write_filerecordstable();
	return S_OK;
}


//get file name
int filesystem_get_filename(HFILE hFile, /*hfile of file we are looking for name and ext*/
							char* pFileName, /*name of the file*/
							char* pFileExt /*ext of the file*/
								)
{
	if(!pFileName || !pFileExt)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid parameters");
	}
	int index = filesystem_get_first_filerecordIndex(hFile);
	memset(pFileName, 0, FILE_NAME_SZ);
	strncpy(pFileName, filesystem.fileRecordsOnSector[index].name, FILE_NAME_SZ);
	memset(pFileExt, 0, FILE_EXT_SZ);
	strncpy(pFileExt, filesystem.fileRecordsOnSector[index].ext, FILE_EXT_SZ);
	return S_OK;
}


//open file from directory
//ret NULL if no file
//HFILE is a index of file in filerecord table +1
HFILE filesystem_open_file(const char* pFileName, /*name of the file, will be found*/
						   const char* pFileExt /*ext of the file*/
						  )
{
	//reset INI default state
	filesystem.file_pos_last_value = -1;
	filesystem.file_pos_last_section = -1;
	memset(filesystem.lastSectionName,0 , sizeof(filesystem.lastSectionName));
	return filesystem_find_file(pFileName, pFileExt);
}



//find poisition in file and open nessesary claster
//ret E_FAIL if file empty or out of file range
//ret data index of clasterBuffer
int filesystem_read_from_position(HFILE hFile, /*file descriptor = number of file in file record table*/
						int file_pos /*it is counter or read bytes, input: start from 0, as output ret pointer for the next non read byte*/
							)
{
	int index = filesystem_get_first_filerecordIndex(hFile);
	int startclaster = (int)filesystem.fileRecordsOnSector[index].wdStartClaster;
	if(startclaster==CLASTERTABLE_EMPTY_RECORD)return E_FAIL;	//empty file

	int flen = filesystem.fileRecordsOnSector[index].dwLength;
	if(file_pos >= flen)return E_FAIL;	//out of file

	int spendclasters = file_pos/CLASTER_DATA_LEN;
	for(int i=0;i<spendclasters;i++)
	{
		PowerControl_kickWatchDog();
		int clasterindex = filesystem_get_clasterIndex(startclaster);
		if(clasterindex==E_FAIL)
		{
			exception(__FILE__,__FUNCTION__,__LINE__,"invalid claster record");
			return E_FAIL;
		}else
		{
			int newstartclaster = filesystem.clasterTableOnSector[clasterindex];
			BOOL bItsEnd = startclaster == newstartclaster;
			startclaster = newstartclaster;
			if(bItsEnd || startclaster==CLASTERTABLE_EMPTY_RECORD)return E_FAIL;	//out of file
		}
	}
	//startclaster now contain a start claster of position
	if(!filesystem_read_claster(startclaster))
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"invalid file claster");
		return E_FAIL;
	}
	return (file_pos%CLASTER_DATA_LEN);
}



//ret file length in bytes
//ret 0 if file is empty
DWORD filesystem_get_length(HFILE hFile /*file descriptor = number of file in file record table*/
							)
{
	int index = filesystem_get_first_filerecordIndex(hFile);
	if(filesystem.fileRecordsOnSector[index].wdStartClaster==CLASTERTABLE_EMPTY_RECORD)
		return 0;
	else
		return filesystem.fileRecordsOnSector[index].dwLength;
}





//read file data from filesystem
//ret number of bytes read
//ret E_FAIL - if file_pos out of file ot file empty
int filesystem_file_get(HFILE hFile, /*file descriptor = number of file in file record table*/
						int* pFile_pos, /*it is counter of read bytes, input: start from 0, as output ret pointer for the next non read byte*/
						  BYTE* pBuffer, /*recepient buffer*/
						  int len)/*input: buffer len in bytes, output: read len in bytes*/
{
	int rlen = 0;
	int flen = filesystem_get_length(hFile);
	flen -= *pFile_pos;
	if(flen<=0)return E_FAIL;
	do
	{
		int buf_index = filesystem_read_from_position(hFile, *pFile_pos);
		if(buf_index==E_FAIL)return E_FAIL;	//some errors: empty file, out of file
		int calc_len = CLASTER_DATA_LEN-buf_index;
		if(len<calc_len)
		{
			calc_len = len;
		}
		if(flen<calc_len)
		{
			calc_len = flen;
		}
		memcpy(&pBuffer[rlen], &filesystem.clasterBuffer[buf_index], calc_len);
		rlen+=calc_len;
		*pFile_pos+=calc_len;
		len -= calc_len;
		flen -= calc_len;
	}while(flen>0 && len>0);
	return rlen;
}



//change file length
//len must be >0 else exception
//if ret E_FAIL then file is already 0, no free space,
//ret S_OK if OK
HRESULT filesystem_file_change_len(HFILE hFile, /*file descriptor = number of file in file record table*/
							   int len /*new len of the file*/
								   )
{
	if(powerControl.bBatteryAlarm)return E_FAIL;	//when battery discharged no file operation is allowed

	int newclasterlen = len/CLASTER_DATA_LEN + ((len%CLASTER_DATA_LEN)?1:0);
	if(newclasterlen==0)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"len parameter must not be 0");
		return E_FAIL;
	}
	DWORD curlen = filesystem_get_length(hFile);
	int curclasterlen = curlen/CLASTER_DATA_LEN + ((curlen%CLASTER_DATA_LEN)?1:0);
	int clasterlen = newclasterlen-curclasterlen;
	int index = filesystem_get_first_filerecordIndex(hFile);
	int startclaster = filesystem.fileRecordsOnSector[index].wdStartClaster;
	if(curlen>len && len>0)
	{//make smaller
		int clbufidx = filesystem_read_from_position(hFile, len-1);
		if(clbufidx==E_FAIL)
		{
			exception(__FILE__,__FUNCTION__,__LINE__,"invalid file");
			return E_FAIL;
		}
		//delete all next clasters
		BOOL bFirstOne = TRUE;
		BOOL bItsEnd = FALSE;
		int delstartclaster = filesystem.iCurrentFileClaster;
		do
		{
			int clasterindex = filesystem_get_clasterIndex(delstartclaster);
			if(clasterindex==E_FAIL)
			{
				exception(__FILE__,__FUNCTION__,__LINE__,"invalid claster record");
				return E_FAIL;
			}
			int newstartclaster = filesystem.clasterTableOnSector[clasterindex];
			bItsEnd = delstartclaster == newstartclaster;
			if(!bItsEnd)
			{
				filesystem.clasterTableOnSector[clasterindex] = bFirstOne?delstartclaster:CLASTERTABLE_EMPTY_RECORD;
				filesystem_write_clastertable();
				bFirstOne = FALSE;
			}
			delstartclaster = newstartclaster;
		}while(!bItsEnd);
	}else if(curlen<len)
	{//make bigger
		if(clasterlen>0)
		{
			int clbufidx = filesystem_read_from_position(hFile, curlen-1);
			int newstartclaster = filesystem.iCurrentFileClaster;
			if(clbufidx==E_FAIL)
			{//no clasters yet for file
				startclaster = filesystem_find_free_claster(CLASTERTABLE_EMPTY_RECORD);
				if(startclaster==E_FAIL)return E_FAIL;//no free space
				//here must do mark claster as busy
				newstartclaster = startclaster;
				int clasterindex = filesystem_get_clasterIndex(newstartclaster);
				if(clasterindex==E_FAIL)
					exception(__FILE__,__FUNCTION__,__LINE__,"invalid claster record");
				filesystem.clasterTableOnSector[clasterindex] = newstartclaster;
				filesystem_write_clastertable();
				clasterlen--;
			}
			//make all next clasters
			for(int i=0;i<clasterlen;i++)
			{
				int new2startclaster = filesystem_find_free_claster(startclaster);
				if(new2startclaster==E_FAIL)return E_FAIL;//no free space
				int clasterindex = filesystem_get_clasterIndex(newstartclaster);
				if(clasterindex==E_FAIL)
					exception(__FILE__,__FUNCTION__,__LINE__,"invalid claster record");
				newstartclaster = new2startclaster;
				filesystem.clasterTableOnSector[clasterindex] = newstartclaster;
				filesystem_write_clastertable();
				clasterindex = filesystem_get_clasterIndex(newstartclaster);
				if(clasterindex==E_FAIL)
					exception(__FILE__,__FUNCTION__,__LINE__,"invalid claster record");
				filesystem.clasterTableOnSector[clasterindex] = newstartclaster;
				filesystem_write_clastertable();
			}
		}
	}
	filesystem.fileRecordsOnSector[index].wdStartClaster = startclaster;
	filesystem.fileRecordsOnSector[index].dwLength = len;
	filesystem_write_filerecordstable();
	return S_OK;
}




//save file to filesystem
//ret number of bytes written
//ret E_FAIL - if error
int filesystem_file_put(HFILE hFile, /*file descriptor = number of file in file record table*/
						int* pFile_pos, /*it is counter or written bytes, input: start from 0, as output ret pointer for the next non written byte*/
						  const BYTE* pBuffer, /*send buffer*/
						  int len)/*buffer len in bytes*/
{
	if(powerControl.bBatteryAlarm)return E_FAIL;	//when battery discharged no file operation is allowed

	int filelen = *pFile_pos + len;
	if(filelen==0)return E_FAIL;	//no free space
	int wlen = 0;
	do
	{
		int buf_index = filesystem_read_from_position(hFile, *pFile_pos);
		if(buf_index==E_FAIL)
		{
			if(filesystem_file_change_len(hFile, filelen)!=S_OK)
			{
				return E_FAIL;
			}
			buf_index = filesystem_read_from_position(hFile, *pFile_pos);
			if(buf_index==E_FAIL)
			{
				return E_FAIL;	//invalid file
			}
		}
		int calc_len = CLASTER_DATA_LEN-buf_index;
		if(len<calc_len)
		{
			calc_len = len;
			len = 0;
		}else
			len -= calc_len;
		memcpy(&filesystem.clasterBuffer[buf_index], &pBuffer[wlen], calc_len);
		filesystem_write_claster(filesystem.iCurrentFileClaster);
		wlen+=calc_len;
		*pFile_pos+=calc_len;
	}while(len>0);
	return wlen;
}



//create file dir list
//ret items number retrieved
//does not get system files
int filesystem_get_dir(const char* pFileExt, /*ext of file, if NULL then all files */
						struct tagDirEntry* pDir, /*output directory entries*/
						int itemsNum, /*items number of pDir*/
						int itemsPass	/*items number must be passed*/
							)
{
	if(itemsNum<1)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"itemsNum < 1");
	}
	int retnum = 0;
	for(int i=FILERECORD_START_NO;i<=FILERECORD_END_NO;i++)
	{
		PowerControl_kickWatchDog();
		int index = filesystem_get_filerecordIndex(i);
		if(index==E_FAIL)
		{//error in filerecord, bad record in eeprom, all files of this records must be deleted
			exception(__FILE__,__FUNCTION__,__LINE__,"invalid file record");
		}else
		{
			if(filesystem.fileRecordsOnSector[index].wdStartClaster==CLASTERTABLE_EMPTY_RECORD)continue;	//deleted
			if(!strncmp(filesystem.fileRecordsOnSector[index].ext,pFileExt,FILE_EXT_SZ) &&
				!(!strncmp(filesystem.fileRecordsOnSector[index].ext,"sz2",FILE_EXT_SZ)
			   && (!strncmp(filesystem.fileRecordsOnSector[index].name,"energy",FILE_NAME_SZ)
			   || (!strncmp(filesystem.fileRecordsOnSector[index].name,"sigma",FILE_NAME_SZ)))))
			{//not deleted, not filtered, not system
				if(itemsPass)
				{
					--itemsPass;
				}else
				{
					//yes we found one
					memset(pDir->name, 0, FILE_NAME_SZ);
					strncpy(pDir->name, filesystem.fileRecordsOnSector[index].name, FILE_NAME_SZ);
					retnum++;
					pDir++;
					if(--itemsNum==0)break;
				}
			}
		}
	}
	return retnum;
}


//find first file with filter by extension
HFILE filesystem_open_first(char* pExt)
{
	HFILE hfile = NULL;
	for(int i=0;i<FILERECORD_SECTOR_AREA_LEN;i++)
	{
		PowerControl_kickWatchDog();
		if(!filesystem_read_filerecordstable(i+FILERECORD_START_SECTOR_NO))
		{
			exception(__FILE__,__FUNCTION__,__LINE__,"Filesystem failed");
		}
		for(int j=0;j<FILERECORD_RECORDS_ON_SECTOR;j++)
		{
			if(filesystem.fileRecordsOnSector[j].wdStartClaster!=CLASTERTABLE_EMPTY_RECORD)
			{
				hfile = i*FILERECORD_RECORDS_ON_SECTOR+j+1;
				if(!pExt)
					return hfile;
				else if(!strncmp(filesystem.fileRecordsOnSector[j].ext, pExt, FILE_EXT_SZ))
					return hfile;
			}
		}
	}
	return NULL;
}

//find first file with filter by extension and name that begins from
HFILE filesystem_open_firstEx(char* pName, char* pExt)
{
	HFILE hfile = NULL;
	for(int i=0;i<FILERECORD_SECTOR_AREA_LEN;i++)
	{
		PowerControl_kickWatchDog();
		if(!filesystem_read_filerecordstable(i+FILERECORD_START_SECTOR_NO))
		{
			exception(__FILE__,__FUNCTION__,__LINE__,"Filesystem failed");
		}
		for(int j=0;j<FILERECORD_RECORDS_ON_SECTOR;j++)
		{
			if(filesystem.fileRecordsOnSector[j].wdStartClaster!=CLASTERTABLE_EMPTY_RECORD)
			{
				hfile = i*FILERECORD_RECORDS_ON_SECTOR+j+1;
				if(!pExt)
					return hfile;
				else if(!strncmp(filesystem.fileRecordsOnSector[j].ext, pExt, FILE_EXT_SZ) &&
						strstr(filesystem.fileRecordsOnSector[j].name, pName)==filesystem.fileRecordsOnSector[j].name)
					return hfile;
			}
		}
	}
	return NULL;
}

//check that file is exists
//ret TRUE if exists
//ret FALSE if error or no file
BOOL filesystem_check_file_exists(HFILE hFile)
{
	int filerecord = hFile-1;
	if(filerecord<FILERECORD_START_NO || filerecord>FILERECORD_END_NO)
		return FALSE;
	int idx = filesystem_get_filerecordIndex(filerecord);
	if(idx==E_FAIL)return FALSE;
	return (filesystem.fileRecordsOnSector[idx].wdStartClaster!=CLASTERTABLE_EMPTY_RECORD);
}



//file work
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//show memlow symbol in status lnie
void filesystem_show_symbol(int x)
{
	if(filesystem.bMemoryLow && (clockData.dateTime.second&1))
		Display_output_bmp(x,0, (BITMAPFILEHEADER*)bmp_memlow);
	else
	{
		RECT rect = {x,0,x+23,16};
		Display_clearRect(rect, 100);
	}
}


void filesystem_detect_memorylow(void)
{
	SETUPModeControl.uiCalcFreeMem = filesystem_calc_free_memory()/1024;
	filesystem.bMemoryLow = ((UINT)(SETUPModeControl.uiCalcFreeMem*100/TOTAL_MEMORY) < 3);
}

