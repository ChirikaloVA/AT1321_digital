//file_list.c


#include <string.h>



#include "display.h"
#include "file_list.h"
#include "modes.h"
#include "interrupts.h"




struct tagFileListModeControl FileListModeControl;


const struct tagMode modes_FileListMode=
{
	FileListMode_NameOnUpdate,//"FileList\0,	//mode name
	RGB(43,63,0),	//mode color
	Modes_menu_LeftOnUpdate,//,	//right
	Modes_menu_RightOnUpdate,//,	//left
	Modes_menu_UpOnUpdate,////up
	Modes_menu_DownOnUpdate,//,	//down
	FileListMode_OnActivate,
	FileListMode_OnLeft,
	FileListMode_OnRight,
	FileListMode_OnUp,
	FileListMode_OnDown,
	FileListMode_OnIdle,
	FileListMode_OnShow,
	FileListMode_OnExit,
	FileListMode_OnPowerDown,
	FileListMode_OnWakeUp,
	FileListMode_OnTimer
};

const char* FileListMode_NameOnUpdate(void)//"FileList\0",	//mode name
{
	return "LIST\0""LIST\0""LIST\0""СПИСОК";
}





const struct tagMenu FileList_menu=
{
	"Select item\0""Select item\0""Select item\0""Выберите пункт",	//menu name
	9,	//number of items
	{
		FileListMode_menu1_punkt1,
		FileListMode_menu1_punkt2,
		FileListMode_menu1_punkt_any,
		FileListMode_menu1_punkt_any,
		FileListMode_menu1_punkt_any,
		FileListMode_menu1_punkt_any,
		FileListMode_menu1_punkt_any,
		FileListMode_menu1_punkt_any,
		FileListMode_menu1_punkt_any,
	},
	{
		FileListMode_menu1_punkt1_onUpdate,
		FileListMode_menu1_punkt2_onUpdate,
		FileListMode_menu1_punkt3_onUpdate,
		FileListMode_menu1_punkt4_onUpdate,
		FileListMode_menu1_punkt5_onUpdate,
		FileListMode_menu1_punkt6_onUpdate,
		FileListMode_menu1_punkt7_onUpdate,
		FileListMode_menu1_punkt8_onUpdate,
		FileListMode_menu1_punkt9_onUpdate,
	}
};











void FileListMode_Init(void)
{
	FileListModeControl.iMarkerPos = 0;
	FileListModeControl.iPage = 0;
	for(int i=0;i<MAX_ITEMS;i++)
		memset(FileListModeControl.listItems[i].name, 0, FILE_NAME_SZ);
	FileListModeControl.pOnNextPage = NULL;
	FileListModeControl.pOnPrevPage = NULL;
	FileListModeControl.pRetFunction = NULL;
	FileListModeControl.bSelected = FALSE;
	FileListModeControl.iItemsNum = 0;
}



BOOL FileListMode_OnActivate(void)
{
	FileListModeControl.iMarkerPos = 0;
	FileListModeControl.iPage = 0;
	FileListModeControl.bSelected = FALSE;	
	Modes_activateMenu(&FileList_menu);
	return 1;
}

BOOL FileListMode_OnTimer(void)
{
	return 1;
}





BOOL FileListMode_OnRight(void)
{
	return 1;
}

BOOL FileListMode_OnLeft(void)
{
	return 1;
}

BOOL FileListMode_OnUp(void)
{
	return 1;
}
BOOL FileListMode_OnDown(void)
{
	Modes_activateMenu(&FileList_menu);
	return 1;
}
BOOL FileListMode_OnIdle(void)
{
	if(!modeControl.bMenuON)
	{//emergency exit when user has selected item
		if(!FileListModeControl.pRetFunction)
		{
			exception(__FILE__,__FUNCTION__,__LINE__, "pRetFunction must not be NULL");
		}
		(*FileListModeControl.pRetFunction)(FileListModeControl.bSelected);	//ret function after edit, it must switch to neccessary mode
	}
	return 1;
}
BOOL FileListMode_OnShow(void)
{
	Display_setTextWin(0,Y_SCREEN_MAX-200,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(0);
	Display_setTextSteps(1,3);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(NONE);
	Display_setCurrentFont(fnt16x16);
	Display_outputTextByLang("Please wait...\0""Bitte warten...\0""Please wait...\0""Пожалуйста ждите...");
	return 1;
}
BOOL FileListMode_OnExit(void)
{
	FileListModeControl.pRetFunction = NULL;
	return 1;
}
BOOL FileListMode_OnWakeUp(void)
{
	return 1;
}
BOOL FileListMode_OnPowerDown(void)
{
	return 1;	//allow enter power down
}






BOOL FileListMode_menu1_punkt1(void)
{
	if(!FileListModeControl.pOnNextPage)
	{
		exception(__FILE__,__FUNCTION__,__LINE__, "pOnNextPage must not be NULL");
	}
	(*FileListModeControl.pOnNextPage)();
	return 0;
}
const char* FileListMode_menu1_punkt1_onUpdate(void)
{
	return ">>Next page\0"">>Next page\0"">>Next page\0"">>Следующая стр.";
}
BOOL FileListMode_menu1_punkt2(void)
{
	if(!FileListModeControl.pOnPrevPage)
	{
		exception(__FILE__,__FUNCTION__,__LINE__, "pOnPrevPage must not be NULL");
	}
	(*FileListModeControl.pOnPrevPage)();
	return 0;
}
const char* FileListMode_menu1_punkt2_onUpdate(void)
{
	return "<<Previous page\0""<<Previous page\0""<<Previous page\0""<<Предыдущая стр.";
}


//prepare item name for four lang
//index - of listItems
//pbuf - recepient buffer
void FileListMode_prepareItemText(int index, char* pbuf, int len)
{
	char buf2[FILE_NAME_SZ+1];
	memset(pbuf,0,len);
	memset(buf2,0,sizeof(buf2));
	strncpy(buf2,FileListModeControl.listItems[index].name, FILE_NAME_SZ);
	strcat(pbuf, buf2);
	pbuf+=strlen(buf2);
	pbuf[0]=0;
	pbuf++;
	strcat(pbuf, buf2);
	pbuf+=strlen(buf2);
	pbuf[0]=0;
	pbuf++;
	strcat(pbuf, buf2);
	pbuf+=strlen(buf2);
	pbuf[0]=0;
	pbuf++;
	strcat(pbuf, buf2);
	pbuf+=strlen(buf2);
	pbuf[0]=0;
}


BOOL FileListMode_menu1_punkt_any(void)
{
	FileListModeControl.iMarkerPos = modeControl.dwCurMenuItem-2;
	FileListModeControl.bSelected = TRUE;
	return 1;	//select and close menu
}
const char* FileListMode_menu1_punkt3_onUpdate(void)
{
	if(FileListModeControl.iItemsNum<1)return NULL;
	FileListMode_prepareItemText(0, FileListModeControl.buf, sizeof(FileListModeControl.buf));
	return FileListModeControl.buf;
}
const char* FileListMode_menu1_punkt4_onUpdate(void)
{
	if(FileListModeControl.iItemsNum<2)return NULL;
	FileListMode_prepareItemText(1, FileListModeControl.buf, sizeof(FileListModeControl.buf));
	return FileListModeControl.buf;
}
const char* FileListMode_menu1_punkt5_onUpdate(void)
{
	if(FileListModeControl.iItemsNum<3)return NULL;
	FileListMode_prepareItemText(2, FileListModeControl.buf, sizeof(FileListModeControl.buf));
	return FileListModeControl.buf;
}
const char* FileListMode_menu1_punkt6_onUpdate(void)
{
	if(FileListModeControl.iItemsNum<4)return NULL;
	FileListMode_prepareItemText(3, FileListModeControl.buf, sizeof(FileListModeControl.buf));
	return FileListModeControl.buf;
}
const char* FileListMode_menu1_punkt7_onUpdate(void)
{
	if(FileListModeControl.iItemsNum<5)return NULL;
	FileListMode_prepareItemText(4, FileListModeControl.buf, sizeof(FileListModeControl.buf));
	return FileListModeControl.buf;
}
const char* FileListMode_menu1_punkt8_onUpdate(void)
{
	if(FileListModeControl.iItemsNum<6)return NULL;
	FileListMode_prepareItemText(5, FileListModeControl.buf, sizeof(FileListModeControl.buf));
	return FileListModeControl.buf;
}
const char* FileListMode_menu1_punkt9_onUpdate(void)
{
	if(FileListModeControl.iItemsNum<7)return NULL;
	FileListMode_prepareItemText(6, FileListModeControl.buf, sizeof(FileListModeControl.buf));
	return FileListModeControl.buf;
}
