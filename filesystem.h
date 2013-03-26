//filesystem.h

#ifndef FILESYSTEM_H
#define FILESYSTEM_H


#include <iolpc2388.h>

#include "types.h"
#include "eeprom.h"





#define CLASTERTABLE_EMPTY_RECORD 0xffff


#define FILE_NAME_SZ 			(int)16
#define FILE_EXT_SZ 			(int)3
#define CLASTER_SZ_IN_SECTORS 	(int)1
#define MAX_CLASTERS			(int)((int)MAX_SECTORS/(int)CLASTER_SZ_IN_SECTORS)
#define CLASTER_DATA_LEN 		(int)((int)SECTOR_DATA_LEN*(int)CLASTER_SZ_IN_SECTORS)
#define CLASTERTABLE_START_SECTOR_NO 	(int)4
#define CLASTERTABLE_SECTOR_AREA_LEN 		(int)((int)MAX_CLASTERS*(int)sizeof(WORD)/(int)SECTOR_DATA_LEN)
#define CLASTERTABLE_RECORDS_ON_SECTOR 	(int)((int)SECTOR_DATA_LEN/(int)sizeof(WORD))


#pragma pack(1)
struct tagDirEntry
{
	char name[FILE_NAME_SZ];
};
#pragma pack()


#pragma pack(1)
//size 32 bytes, it means 8 records on sector and 32 records on claster
//
struct tagFileRecord
{
	char name[FILE_NAME_SZ];
	char ext[FILE_EXT_SZ];
	struct tagDateTime dateTime;
	WORD wdStartClaster;//0xffff means file record is free
	DWORD dwLength;	//len of the file
};
#pragma pack()

#define FILERECORD_RECORDS_ON_SECTOR (int)((int)SECTOR_DATA_LEN/(int)sizeof(struct tagFileRecord))

//ВАЖНО!!!!!!!!! ПОРЯДОК работы с ФС
//при записи файла первым надо записать кластер данных, затем таблицу кластеров, затем таблицу файлов
//в таком случае сбой не приведет к форматированию

struct tagFileSystem
{
	//mean current sector of claster table if 0xff then no claster table was loaded
	//if -1 then no file record was loaded
	int iCurrentClasterTableSector;
	//0xffff claster is free
#pragma pack(1)
	//=CLASTERTABLE_EMPTY_RECORD mean free claster
	//=on self mean last claster in file
	WORD clasterTableOnSector[CLASTERTABLE_RECORDS_ON_SECTOR];
#pragma pack()
	//mean current sector of file record
	//if -1 then no file record was loaded
	int iCurrentFileRecordSector;
	//table of file records per one sector
#pragma pack(1)
	struct tagFileRecord fileRecordsOnSector[FILERECORD_RECORDS_ON_SECTOR];
#pragma pack()
	//mean current claster of file was loaded
	//if -1 then no file claster was loaded
	int iCurrentFileClaster;
	int iCurrentFileClasterSector;
	
#pragma pack(1)
	
	//must be higher or equal to SECTOR_LEN
	//если этот буфер будет испорчен не соответсвующим загруженному кластеру, то надо сделать следующее
	//filesystem.iCurrentFileClaster=-1;
	//filesystem.iCurrentFileClasterSector=-1;
	BYTE clasterBuffer[CLASTER_DATA_LEN+8];
	
#pragma pack()
	
	int file_pos_last_value;
	int file_pos_last_section;
	char lastSectionName[256];
	
	HFILE hFileExecuteSys;//HFILE of execute.sys, =NULL if no set
	
	BOOL bMemoryLow;
	
};

#define MAX_FILE_RECORDS			(int)1024
#define MAX_FILES 					(int)MAX_FILE_RECORDS

//сдвинули таблицу файлов изза появления резерва таблицы кластеров
#define FILERECORD_START_SECTOR_NO 	(int)((int)CLASTERTABLE_START_SECTOR_NO+(int)CLASTERTABLE_SECTOR_AREA_LEN+(int)CLASTERTABLE_SECTOR_AREA_LEN)
#define FILERECORD_SECTOR_AREA_LEN	(int)((int)MAX_FILE_RECORDS/(int)FILERECORD_RECORDS_ON_SECTOR)

//сдвинули файловую область изза появления резерва таблицы файлов
#define FILES_SECTOR_START_NO 		(int)((int)FILERECORD_START_SECTOR_NO+(int)FILERECORD_SECTOR_AREA_LEN+(int)FILERECORD_SECTOR_AREA_LEN)
#define FILES_SECTOR_AREA_LEN 		(int)((int)MAX_SECTORS-(int)FILES_SECTOR_START_NO)


#define FILES_CLASTER_START_NO 		(int)((int)FILES_SECTOR_START_NO/(int)CLASTER_SZ_IN_SECTORS)
#define FILES_CLASTER_AREA_LEN 		(int)((int)FILES_SECTOR_AREA_LEN/(int)CLASTER_SZ_IN_SECTORS)
#define FILES_CLASTER_END_NO 		(int)((int)FILES_CLASTER_START_NO+(int)FILES_CLASTER_AREA_LEN-1)

#define FILERECORD_START_NO		(int)0
#define FILERECORD_END_NO		(int)((int)MAX_FILES-1)

//in KB
#define TOTAL_MEMORY	(FILES_CLASTER_AREA_LEN*CLASTER_DATA_LEN/1024)

//#define CLASTER_START_NO		(int)0
//#define CLASTER_END_NO			(int)((int)(MAX_SECTORS-CLASTERTABLE_START_SECTOR_NO)/CLASTER_SZ_IN_SECTORS-1)






extern struct tagFileSystem filesystem;






//////////////////////////////////////////////////////////////////////////////////////////////////////
//system functions
void filesystem_Init(void);


int filesystem_get_clasterIndex(int claster_number);
int filesystem_get_filerecordIndex(int filerecord);
BOOL filesystem_read_filerecordstable(int sector_num);
BOOL filesystem_write_filerecordstable(void);
BOOL filesystem_read_clastertable(int sector);
BOOL filesystem_write_clastertable(void);
BOOL filesystem_read_claster(int claster_number);
BOOL filesystem_write_claster(int claster_number);

void filesystem_format_filetable(BOOL bYes);

HFILE filesystem_find_file(const char* pFileName, /*name of the file, will be found and rewrite*/
							const char* pFileExt /*ext of the file*/
						  );
int filesystem_find_free_filerecord(void);
int filesystem_read_from_position(HFILE hFile, /*file descriptor = number of file in file record table*/
						int file_pos /*it is counter or read bytes, input: start from 0, as output ret pointer for the next non read byte*/
							);

int filesystem_find_free_claster(int notThisClaster);

int filesystem_get_first_filerecordIndex(HFILE hFile);

HRESULT filesystem_file_change_len(HFILE hFile, /*file descriptor = number of file in file record table*/
							   int len /*new len of the file*/
								   );





int filesystem_get_filename(HFILE hFile, /*hfile of file we are looking for name and ext*/
							char* pFileName, /*name of the file*/
							char* pFileExt /*ext of the file*/
								);

int filesystem_copy_file_data(HFILE hFileDst, HFILE hFileSrc, int file_pos_dst, int file_pos_src, int len);

int filesystem_cut_file(HFILE hFile, /*file that will be cut*/
						 int file_pos, /*file pos from cut*/
						 int bytes_cut /*bytes to cut >0, if <0 then grows*/
							 );












///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//user function



/////////////////////////////////////////
//file function
int filesystem_rename_file(const char* pFileName, /*name of the file, will be found and rewrite*/
							const char* pFileExt, /*ext of the file*/
						   const char* pNewFileName, /*name of the file, will be found and rewrite*/
							const char* pNewFileExt /*ext of the file*/
								);
int filesystem_get_dir(const char* pFileExt, /*ext of file, if NULL then all files */
						struct tagDirEntry* pDir, /*output directory entries*/
						int itemsNum, /*items number of pDir*/
						int itemsPass	/*items number must be passed*/
							);
int filesystem_file_get(HFILE hFile, /*file descriptor = number of file in file record table*/
						int* pFile_pos, /*it is counter or read bytes, input: start from 0, as output ret pointer for the next non read byte*/
						  BYTE* pBuffer, /*recepient buffer*/
						  int len);/*input: buffer len in bytes, output: read len in bytes*/

int filesystem_file_put(HFILE hFile, /*file descriptor = number of file in file record table*/
						int* pFile_pos, /*it is counter or written bytes, input: start from 0, as output ret pointer for the next non written byte*/
						  const BYTE* pBuffer, /*send buffer*/
						  int len);/*buffer len in bytes*/
HRESULT filesystem_delete_file(HFILE hfile);
DWORD filesystem_get_length(HFILE hFile /*file descriptor = number of file in file record table*/
							);
HFILE filesystem_create_file(const char* pFileName, /*name of the file, will be found and rewrite*/
							const char* pFileExt, /*ext of the file*/
							BOOL bOverwrite	/*1 then exists file will be overwritten*/
						  );
HFILE filesystem_open_file(const char* pFileName, /*name of the file, will be found*/
						   const char* pFileExt /*ext of the file*/
						  );


/////////////////////////////////////////
//test function
void filesystem_test(void);
void filesystem_check_and_restore(void);

UINT filesystem_calc_files_number(char* pExt);
UINT filesystem_calc_free_memory(void);

HFILE filesystem_open_first(char* pExt);

BOOL filesystem_check_file_exists(HFILE hFile);

void filesystem_detect_memorylow(void);
void filesystem_show_symbol(int x);

//подсичтать количество специальных файлов
void filesystem_calc_special_files_number(UINT* pAllFilesNum, 
					UINT* pSPZFilesNum,
					UINT* pLIBFilesNum,
					UINT* pMCSFilesNum,
					UINT* pLOGFilesNum);


HFILE filesystem_open_firstEx(char* pName, char* pExt);

#endif	//#ifndef FILESYSTEM_H
