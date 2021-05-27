//ini_control.h


#ifndef _INI_CONTROL_H
#define _INI_CONTROL_H


#include <iolpc2388.h>

#include "types.h"
#include "eeprom.h"



extern const char system_ini[413];

//max ini string len
#define MAX_INI_STRING_LEN 100


//INI function
int filesystem_ini_get_string(HFILE hFile, const char* pSection, const char* pName, char* pValue, int value_len);
int filesystem_ini_put_string(HFILE hFile, const char* pSection, const char* pName, const char* pValue);
int filesystem_ini_get_int(HFILE hFile, const char* pSection, const char* pName, int* pValue);
int filesystem_ini_put_int(HFILE hFile, const char* pSection, const char* pName, int value);
int filesystem_ini_put_float(HFILE hFile, const char* pSection, const char* pName, float value);
int filesystem_ini_get_float(HFILE hFile, const char* pSection, const char* pName, float* pValue);


void filesystem_check_ini_files(void);
BOOL filesystem_restore_identify_ini(void);
BOOL filesystem_restore_system_ini(void);
BOOL filesystem_restore_main_lib(void);


int filesystem_get_string(HFILE hFile, int* pFile_pos, char* pString, int string_len);


int filesystem_find_value(HFILE hFile, int start_file_pos, const char* pName);

int file_system_find_section(HFILE hFile, const char* pSection);

int ini_read_system_ini(char* pExt);

int ini_retrieveTable(HFILE hfile, struct tagIndexMeanTable*  pTable);

BOOL ini_write_system_ini_int(const char * pSection, const char* pValueName, int value);
BOOL ini_write_system_ini_float(const char * pSection, const char* pValueName, float value);


int filesystem_get_stringReverse(HFILE hFile, int* pFile_pos, char* pString, int string_len);

BOOL filesystem_restore_system_ini_from_backup(void);

#endif	//#ifndef _INI_CONTROL_H
