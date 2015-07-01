//info_mode.c

#include <string.h>
#include "info_mode.h"
#include "stab_mode.h"
#include "display.h"



struct tagINFOModeControl INFOModeControl;


const struct tagMode modes_INFOMode=
{
	INFOMode_NameOnUpdate,//
	RGB(40,60,0),	//mode color
	INFOMode_LeftOnUpdate,//
	INFOMode_RightOnUpdate,//
	INFOMode_UpOnUpdate,//
	INFOMode_DownOnUpdate,//
	INFOMode_OnActivate,
	INFOMode_OnLeft,
	INFOMode_OnRight,
	INFOMode_OnUp,
	INFOMode_OnDown,
	INFOMode_OnIdle,
	INFOMode_OnShow,
	INFOMode_OnExit,
	INFOMode_OnPowerDown,
	INFOMode_OnWakeUp,
	INFOMode_OnTimer
};




const char* INFOMode_NameOnUpdate(void)//
{
	return "INFO\0""INFO\0""INFO\0""ИНФО";
}
const char* INFOMode_LeftOnUpdate(void)//
{
	return "stabil\0""stabil\0""stabil\0""стабил";
}
const char* INFOMode_RightOnUpdate(void)//
{
	return "next\0""next\0""next\0""след";
}
const char* INFOMode_UpOnUpdate(void)//
{
	return "prev\0""prev\0""prev\0""пред";
}
const char* INFOMode_DownOnUpdate(void)//
{
	return "\0""\0""\0""";
}









void INFOMode_Init(void)
{
	memset(&INFOModeControl,0,sizeof(INFOModeControl));
}



BOOL INFOMode_OnActivate(void)
{
	INFOMode_Init();
//	Modes_createTimer(1000);
	Modes_updateMode();	
	return 1;
}

BOOL INFOMode_OnTimer(void)
{
/*	Modes_OnShow();
	int numlines = Display_calcTextLines(INFOModeControl.text);
	if(++INFOModeControl.iLine+5>numlines)
		INFOModeControl.iLine = 0;
	*/
	return 1;
}


BOOL INFOMode_OnLeft(void)
{
	Modes_setActiveMode(&modes_STABMode);
	return 1;
}
BOOL INFOMode_OnRight(void)
{
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,1);
	Display_setTextJustify(LEFT);
	Display_setCurrentFont(fnt6x16);
	int numlines = Display_calcTextLines(INFOModeControl.text);
	if(INFOModeControl.iLine+13<numlines)
		INFOModeControl.iLine++;
	Modes_OnShow();
	return 1;
}
BOOL INFOMode_OnUp(void)
{
	if(INFOModeControl.iLine)
		--INFOModeControl.iLine;
	Modes_OnShow();
	return 1;
}
BOOL INFOMode_OnDown(void)
{
	return 1;
}
BOOL INFOMode_OnIdle(void)
{
	return 1;
}
BOOL INFOMode_OnShow(void)
{
	INFOMode_showModeScreen();
	return 1;
}
BOOL INFOMode_OnExit(void)
{
	return 1;
}
BOOL INFOMode_OnWakeUp(void)
{
	return 1;
}
BOOL INFOMode_OnPowerDown(void)
{
	return 1;	//allow enter power down
}







void INFOMode_showModeScreen(void)
{
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,MODE_USER_HEIGHT);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,1);
	Display_setTextJustify(LEFT);
	Display_setCurrentFont(fnt6x16);
	Display_setTextLineClear(1);
	Display_checkForClearLine();
	
	memset(INFOModeControl.text, 0, sizeof(INFOModeControl.text));
	
	const char* pTxt = Display_getTextByLang("Important information!!!\r\0""Important information!!!\r\0""Important information!!!\r\0""Важная информация!!!\r");
	int len = sizeof(INFOModeControl.text) - strlen(INFOModeControl.text) - 1;
	if(len>0)
		strncat(INFOModeControl.text, pTxt, len);
	
	
	if(modeControl.bNoSysSettings)
	{
		pTxt = Display_getTextByLang("Invalid MCA Settings. Please contact manufacturer for repair.\r\r\0""Invalid MCA Settings. Please contact manufacturer for repair.\r\r\0""Invalid MCA Settings. Please contact manufacturer for repair.\r\r\0""Разрушены настройки многоканального анализатора. Пожалуйста, свяжитесь с производителем для ремонта.\r\r");
		len = sizeof(INFOModeControl.text) - strlen(INFOModeControl.text) - 1;
		if(len>0)
			strncat(INFOModeControl.text, pTxt, len);
	}
	if(modeControl.bSysDefault)
	{
		pTxt = Display_getTextByLang("Applied default system settings. Values from GM counter are incorrect.\r\r\0""Applied default system settings. Values from GM counter are incorrect.\r\r\0""Applied default system settings. Values from GM counter are incorrect.\r\r\0""Установлены системные настройки по умолчанию. Данные ГМ счетчика будут некорректными.\r\r");
		len = sizeof(INFOModeControl.text) - strlen(INFOModeControl.text) - 1;
		if(len>0)
			strncat(INFOModeControl.text, pTxt, len);
	}
	if(modeControl.bNoLibrary)
	{
		pTxt = Display_getTextByLang("No selected nuclide library file. Default main.lib used.\r\r\0""No selected nuclide library file. Default main.lib used.\r\r\0""No selected nuclide library file. Default main.lib used.\r\r\0""Нет выбранного файла библиотеки нуклидов. Используется main.lib по умолчанию.\r\r");
		len = sizeof(INFOModeControl.text) - strlen(INFOModeControl.text) - 1;
		if(len>0)
			strncat(INFOModeControl.text, pTxt, len);
	}
	if(modeControl.bIdentDefault)
	{
		pTxt = Display_getTextByLang("Applied default identification settings.\r\r\0""Applied default identification settings.\r\r\0""Applied default identification settings.\r\r\0""Установлены настройки идентификации нуклидов по умолчанию.\r\r");
		len = sizeof(INFOModeControl.text) - strlen(INFOModeControl.text) - 1;
		if(len>0)
			strncat(INFOModeControl.text, pTxt, len);
	}
	if(modeControl.bNoEnergyCal)
	{
		pTxt = Display_getTextByLang("No energy.cal file. Doserate incorrect. Stabilization and nuclide identification impossible.\r\r\0""No energy.cal file. Doserate incorrect. Stabilization and nuclide identification impossible.\r\r\0""No energy.cal file. Doserate incorrect. Stabilization and nuclide identification impossible.\r\r\0""Нет файла energy.cal. Мощность дозы некорректная. Стабилизация и идентификация нуклидов невозможны.\r\r");
		len = sizeof(INFOModeControl.text) - strlen(INFOModeControl.text) - 1;
		if(len>0)
			strncat(INFOModeControl.text, pTxt, len);
	}
	if(modeControl.bNoSigmaCal)
	{
		pTxt = Display_getTextByLang("No sigma.cal file. Stabilization and nuclide identification impossible.\r\r\0""No sigma.cal file. Stabilization and nuclide identification impossible.\r\r\0""No sigma.cal file. Stabilization and nuclide identification impossible.\r\r\0""Нет файла sigma.cal. Стабилизация и идентификация нуклидов невозможны.\r\r");
		len = sizeof(INFOModeControl.text) - strlen(INFOModeControl.text) - 1;
		if(len>0)
			strncat(INFOModeControl.text, pTxt, len);
	}
	if(modeControl.bNoSystemBak)
	{
		pTxt = Display_getTextByLang("No system.bak file. Restoring system.ini impossible.\r\r\0""No system.bak file. Restoring system.ini impossible.\r\r\0""No system.bak file. Restoring system.ini impossible.\r\r\0""Нет файла system.bak. Восстановление system.ini невозможно.\r\r");
		len = sizeof(INFOModeControl.text) - strlen(INFOModeControl.text) - 1;
		if(len>0)
			strncat(INFOModeControl.text, pTxt, len);
	}
	if(modeControl.bNoEnergySigmaSpz)
	{
		pTxt = Display_getTextByLang("Not enough memory for system files. Computer Software can failed.\r\r\0""Not enough memory for system files. Computer Software can failed.\r\r\0""Not enough memory for system files. Computer Software can failed.\r\r\0""Нет памяти под системные файлы. Компьютерная программа может не работать.\r\r");
		len = sizeof(INFOModeControl.text) - strlen(INFOModeControl.text) - 1;
		if(len>0)
			strncat(INFOModeControl.text, pTxt, len);
	}
	
	
	Display_outputTextFromLine(INFOModeControl.text, INFOModeControl.iLine);
	
	Display_clearUserPart();
	
	Display_setTextLineClear(0);
}


