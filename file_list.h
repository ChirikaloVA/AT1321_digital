//file_list.h

#ifndef _FILE_LIST_H
#define _FILE_LIST_H


#include <iolpc2388.h>

#include "types.h"
#include "filesystem.h"


//max items in the list
#define MAX_ITEMS 7

struct tagFileListModeControl
{
//	char pFileExtFilter[FILE_EXT_SZ];
	int iMarkerPos;
	int iPage;
	int iItemsNum;	//items in this page
	struct tagDirEntry listItems[MAX_ITEMS];
	BOOL(*pOnNextPage)(void);
	BOOL(*pOnPrevPage)(void);
	char buf[(FILE_NAME_SZ+1)*4];	//!!!!!!!! be careful when changing languge number it must be changed too
	void(*pRetFunction)(BOOL bOK);	//ret function after edit, it must switch to neccessary mode
	BOOL bSelected;	//for return, if true then item selected
};

extern struct tagFileListModeControl FileListModeControl;

extern const struct tagMode modes_FileListMode;


extern const struct tagMenu FileList_menu;


void FileListMode_Init(void);


BOOL FileListMode_OnActivate(void);
BOOL FileListMode_OnLeft(void);
BOOL FileListMode_OnRight(void);
BOOL FileListMode_OnUp(void);
BOOL FileListMode_OnDown(void);
BOOL FileListMode_OnIdle(void);
BOOL FileListMode_OnShow(void);
BOOL FileListMode_OnExit(void);
BOOL FileListMode_OnPowerDown(void);
BOOL FileListMode_OnWakeUp(void);
BOOL FileListMode_OnTimer(void);
void FileListMode_showModeScreen(void);



const char* FileListMode_NameOnUpdate(void);//"FileList\0""ÑÏÐÄ",	//mode name


BOOL FileListMode_menu1_punkt1(void);
const char* FileListMode_menu1_punkt1_onUpdate(void);
BOOL FileListMode_menu1_punkt2(void);
const char* FileListMode_menu1_punkt2_onUpdate(void);
BOOL FileListMode_menu1_punkt_any(void);
const char* FileListMode_menu1_punkt3_onUpdate(void);
const char* FileListMode_menu1_punkt4_onUpdate(void);
const char* FileListMode_menu1_punkt5_onUpdate(void);
const char* FileListMode_menu1_punkt6_onUpdate(void);
const char* FileListMode_menu1_punkt7_onUpdate(void);
const char* FileListMode_menu1_punkt8_onUpdate(void);
const char* FileListMode_menu1_punkt9_onUpdate(void);

#endif	//#ifndef _FILE_LIST_H
