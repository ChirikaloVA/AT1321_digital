//spectrum.c


#include <string.h>
#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <math.h>
#include <stdlib.h>
#include "types.h"
#include "spectrum.h"
#include "display.h"
#include "interProc.h"
#include "keyboard.h"
#include "powerControl.h"
#include "sprd_mode.h"
#include "tc_mode.h"
#include "eeprom.h"
#include "identify.h"
#include "clock.h"
#include "interrupts.h"
#include "NMEA_Parser.h"
#include "edit.h"
#include "ini_control.h"
#include "spline.h"
#include "file_list.h"
#include "sound.h"
#include "setup_mode.h"
#include "Message_mode.h"








struct tagSpectrumControl spectrumControl;



const struct tagMenu spectrum_menu=
{
	"MENU: RID\0""MENU: RID\0""MENU: RID\0""МЕНЮ: РИД",	//menu name
	8,	//number of items
	{Spectrum_menu1_easyMode,Spectrum_menu1_acqtime, Spectrum_menu1_savespectrum,Spectrum_menu1_openspectrum,Spectrum_menu1_deletespec,Spectrum_menu1_gaincode,Spectrum_menu1_lowlimit,Spectrum_menu1_highlimit},
	{Spectrum_menu1_easyMode_onUpdate,Spectrum_menu1_acqtime_onUpdate,Spectrum_menu1_savespectrum_onUpdate,Spectrum_menu1_openspectrum_onUpdate,Spectrum_menu1_deletespec_onUpdate, Spectrum_menu1_gaincode_onUpdate,Spectrum_menu1_lowlimit_onUpdate,Spectrum_menu1_highlimit_onUpdate}
};


const struct tagMode modes_SpectrumMode=
{
	Spectrum_NameOnUpdate,//"RID\0""РИД",
	RGB(63,40,0),
	Spectrum_LeftOnUpdate,//"marker\rleft\0""маркер\rвлево",
	Spectrum_RightOnUpdate,//"marker\rright\0""маркер\rвправо",
	Spectrum_UpOnUpdate,//"acquir\0""набор",
	Spectrum_DownOnUpdate,//"menu\0""меню",
	Spectrum_OnActivate,
	Spectrum_OnLeft,
	Spectrum_OnRight,
	Spectrum_OnUp,
	Spectrum_OnDown,
	Spectrum_OnIdle,
	Spectrum_OnShow,
	Spectrum_OnExit,
	Spectrum_OnPowerDown,
	Spectrum_OnWakeUp,
	Spectrum_OnTimer
};


const struct tagMode modes_RID_PSW_Mode=
{
	Spectrum_NameOnUpdate,//"RID\0""РИД",
	RGB(63,40,0),
	RID_PSW_LeftOnUpdate,//"marker\rleft\0""маркер\rвлево",
	RID_PSW_RightOnUpdate,//"marker\rright\0""маркер\rвправо",
	RID_PSW_UpOnUpdate,//"acquir\0""набор",
	RID_PSW_DownOnUpdate,//"menu\0""меню",
	RID_PSW_OnActivate,
	RID_PSW_OnLeft,
	RID_PSW_OnRight,
	RID_PSW_OnUp,
	RID_PSW_OnDown,
	Spectrum_OnIdle,
	RID_PSW_OnShow,
	Spectrum_OnExit,
	RID_PSW_OnPowerDown,
	Spectrum_OnWakeUp,
	Spectrum_OnTimer
};






const char* Spectrum_NameOnUpdate(void)//"RID adv\0""РИД расш",
{
	return "RID\0""RID\0""RID\0""РИД";
}
const char* Spectrum_LeftOnUpdate(void)//"marker\rleft\0""маркер\rвлево",
{
	switch(spectrumControl.iSpectrumControlMode)
	{
	case enum_scm_spec_view:
		return "log\0""log\0""log\0""лог";
	case enum_scm_marker_movment:
		return "left\0""left\0""left\0""влево";
	case enum_scm_spec_acquire:
		return "start\0""start\0""start\0""старт";
	case enum_scm_spec_wider:
		return "in\0""in\0""in\0""ближе";
	case enum_scm_spec_higher:
		return "higher\0""higher\0""higher\0""выше";
	case enum_scm_spec_movment:
		return "left\0""left\0""left\0""влево";
	case enum_scm_spec_window:
		return "left\0""left\0""left\0""влево";
	default:
		exception(__FILE__,__FUNCTION__,__LINE__,"Internal error!");
	};
	return NULL;
}
const char* Spectrum_RightOnUpdate(void)//"marker\rright\0""маркер\rвправо",
{
	switch(spectrumControl.iSpectrumControlMode)
	{
	case enum_scm_spec_view:
		return "normal\0""normal\0""normal\0""нормал";
	case enum_scm_marker_movment:
		return "right\0""right\0""right\0""вправо";
	case enum_scm_spec_acquire:
		return "stop\0""stop\0""stop\0""стоп";
	case enum_scm_spec_wider:
		return "out\0""out\0""out\0""дальше";
	case enum_scm_spec_higher:
		return "lower\0""lower\0""lower\0""ниже";
	case enum_scm_spec_movment:
		return "right\0""right\0""right\0""вправо";
	case enum_scm_spec_window:
		return "right\0""right\0""right\0""вправо";
	default:
		exception(__FILE__,__FUNCTION__,__LINE__,"Internal error!");
	};
	return NULL;
}


//режимы работы со спектром
//отображаемое слово и есть текущий режим
const char* Spectrum_UpOnUpdate(void)
{
	switch(spectrumControl.iSpectrumControlMode)
	{
	case enum_scm_spec_view:
		return "view\0""view\0""view\0""вид";
	case enum_scm_marker_movment:
		return "marker\0""marker\0""marker\0""маркер";
	case enum_scm_spec_acquire:
		return "acquir\0""acquir\0""acquir\0""набор";
	case enum_scm_spec_wider:
		return "zoom\0""zoom\0""zoom\0""масшт";
	case enum_scm_spec_movment:
		return "shift\0""shift\0""shift\0""сдвиг";
	case enum_scm_spec_higher:
		return "yscale\0""yscale\0""yscale\0""высота";
	case enum_scm_spec_window:
		return "window\0""window\0""window\0""окно";
	default:
		exception(__FILE__,__FUNCTION__,__LINE__,"Internal error!");
	};
	return NULL;
}
const char* Spectrum_DownOnUpdate(void)//"menu\0""меню",
{
	return "menu\0""menu\0""menu\0""меню";
}








//switch to easy mode
BOOL Spectrum_menu1_easyMode(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return (BOOL)-1;
}

/*
"SPRD mode\rAcquire time\ritem1\ritem2\ritem3\ritem4\ritem5\ritem6\ritem7\0"
"Режим СПРД\rВремя набора\rпункт1\rпункт2\rпункт3\rпункт4\rпункт5\rпункт6\rпункт7";
*/





//calc count on current spectrum
//if two markers then calc between them
void Spectrum_calcCount(void)
{
	DWORD sum=0;
	int s1,s2;
	s1 = 0;
	s2 = CHANNELS-1;
	if(spectrumControl.iMarkerChannel2!=-1)
	{
		if(spectrumControl.iMarkerChannel2<spectrumControl.iMarkerChannel)
		{
			s1 = spectrumControl.iMarkerChannel2;
			s2 = spectrumControl.iMarkerChannel;
		}else
		{
			s2 = spectrumControl.iMarkerChannel2;
			s1 = spectrumControl.iMarkerChannel;
		}
	}
	for(int i=s1;i<=s2;i++)
		sum+=spectrumControl.pShowSpectrum->dwarSpectrum[i];
	spectrumControl.pShowSpectrum->dwCount = sum;
}


void Spectrum_clear(void)
{
	spectrumControl.acqSpectrum.wRealTime = clockData.dwTotalSecondsFromStart;
	spectrumControl.acqSpectrum.wAcqTime=0;
	for(int i=0;i<CHANNELS;i++)
		spectrumControl.acqSpectrum.dwarSpectrum[i]=0;	//by 4 bytes on channel
}

void Spectrum_Init(void)
{
	spectrumControl.bSpectrumInDots = 0;	//in line by default
	spectrumControl.iMarkerChannel = 220;
	spectrumControl.acqSpectrum.dwCount = 0;
	spectrumControl.pShowSpectrum = &spectrumControl.acqSpectrum;
	Spectrum_clear();
	
	spectrumControl.iAcquiringTime = MAX_ACQ_TIME;
	
	spectrumControl.bHasEnergy = FALSE;
	spectrumControl.bHasSigma = FALSE;
	
	spectrumControl.iSpectrumControlMode = enum_scm_marker_movment;
	spectrumControl.iMarkerChannel2 = -1;	//one marker
	
	spectrumControl.iViewChannelFrom = 0;
	spectrumControl.iViewChannelTo = CHANNELS-1;
	spectrumControl.iHighMul = 0;
	
	spectrumControl.bLogView = FALSE;
	
	memset(spectrumControl.acqSpectrum.report, 0, sizeof(spectrumControl.acqSpectrum.report));

	memset(spectrumControl.peakProcRes,0,sizeof(spectrumControl.peakProcRes));

	spectrumControl.bSpectrumEmergSaved = FALSE;
}




BOOL Spectrum_OnActivate(void)
{
	Modes_createTimer(1000);
	spectrumControl.pShowSpectrum = &spectrumControl.acqSpectrum;
	spectrumControl.iSpectrumControlMode = enum_scm_marker_movment;
	spectrumControl.iViewChannelFrom = 0;
	spectrumControl.iViewChannelTo = CHANNELS-1;
	spectrumControl.iHighMul = 0;
	spectrumControl.iMarkerChannel2 = -1;
	memset(spectrumControl.peakProcRes,0,sizeof(spectrumControl.peakProcRes));
	spectrumControl.bLeftRightClicked = FALSE;
	
	spectrumControl.bStopAcq = FALSE;
	spectrumControl.iStopAcq = 0;
	spectrumControl.bLogView = FALSE;

	InterProc_readStatus();

	Modes_updateMode();
	return 1;
}

BOOL Spectrum_OnTimer(void)
{
	if(!InterProc_isDataFinalReady(&interProcControl.rsModbus.sbtStatus))
		return 1;
	
	if(InterProc_isDataFinalReady(&interProcControl.rsModbus.sarSpectrum) &&
	   ((interProcControl.btStatus&0x02) ||
		(spectrumControl.bStopAcq && spectrumControl.iStopAcq>0)))
	{
		--spectrumControl.iStopAcq;
		
		identify_identify(FALSE);
	
		Spectrum_calcCount();
		Spectrum_peakProc();
	
		Modes_OnShow();
	}

	if((interProcControl.btStatus&0x02))//acquiring in progress
	{
		InterProc_readAcqTime();
		InterProc_readSpectrum();
		InterProc_readMeasurementRegs();
		spectrumControl.bStopAcq = FALSE;
	}else if(!spectrumControl.bStopAcq)
	{//have to stop acquiring, it necessary to acq spec for 2 seconds after it is stoped
		spectrumControl.iStopAcq = 2;
		spectrumControl.bStopAcq = TRUE;
		spectrumControl.acqSpectrum.wRealTime = clockData.dwTotalSecondsFromStart - spectrumControl.acqSpectrum.wRealTime;
		SoundControl_BeepSeq(beepSeq_OK);
	}

	//save spectrum if acquired in RID mode
	if(powerControl.bBatteryAlarm /*low battery before*/
	   && !spectrumControl.bSpectrumEmergSaved
		   )
	{//save spectrum with auto name
		powerControl.bBatteryAlarm = 0;	//to allow file save
		SPRDMode_saveAutoSpec();
		powerControl.bBatteryAlarm = 1;
		InterProc_stopSpectrumAcq();
		spectrumControl.bSpectrumEmergSaved = TRUE;
	}
	
	InterProc_readStatus();
	
	return 1;
}


BOOL Spectrum_OnLeft(void)
{
	int step;
	spectrumControl.bLeftRightClicked = TRUE;
	switch(spectrumControl.iSpectrumControlMode)
	{
	case enum_scm_spec_window:	//window mode
	case enum_scm_marker_movment:	//marker control
		Spectrum_showMarker();
		step = (keyboard_safe.keyLeft.pressedTime>>9)+1;
		if(step>20)step = 20;
		spectrumControl.iMarkerChannel-=step;
		if(spectrumControl.iMarkerChannel<spectrumControl.iViewChannelFrom)
			spectrumControl.iMarkerChannel += (spectrumControl.iViewChannelTo-spectrumControl.iViewChannelFrom+1);
		Spectrum_showMarker();
		Spectrum_showMarkerData();
		if(spectrumControl.iMarkerChannel2!=-1)
		{
			Spectrum_calcCount();
			Spectrum_showAcqData();
		}
		Spectrum_peakProc();
		Spectrum_showPeakProcResult();
		break;
	case enum_scm_spec_view:
		spectrumControl.bLogView = TRUE;
		Modes_OnShow();
		break;
	case enum_scm_spec_acquire:
		Spectrum_startAcq();
		//19/03/2010 switch to marker mode
		spectrumControl.iSpectrumControlMode = enum_scm_marker_movment;
		Modes_showButtons();
		break;		
	case enum_scm_spec_wider:	//make view wider
		spectrumControl.iHighMul = 0;	//clear high adjust
		if((spectrumControl.iViewChannelTo-spectrumControl.iViewChannelFrom+1)>SPECTRUM_WIN_WIDTH)
		{
			spectrumControl.iViewChannelFrom = (spectrumControl.iMarkerChannel+spectrumControl.iViewChannelFrom)/2;
			spectrumControl.iViewChannelTo = (spectrumControl.iViewChannelTo+spectrumControl.iMarkerChannel)/2;
			step = (spectrumControl.iViewChannelTo-spectrumControl.iViewChannelFrom+1);
			while(step<SPECTRUM_WIN_WIDTH)
			{
				step = (SPECTRUM_WIN_WIDTH-step)/2+1;
				spectrumControl.iViewChannelFrom -= step;
				if(spectrumControl.iViewChannelFrom<0)
					spectrumControl.iViewChannelFrom = 0;
				spectrumControl.iViewChannelTo += step;
				if(spectrumControl.iViewChannelTo>(CHANNELS-1))
					spectrumControl.iViewChannelTo=CHANNELS-1;
				step = (spectrumControl.iViewChannelTo-spectrumControl.iViewChannelFrom+1);
			};
			Modes_OnShow();
		}
		break;
	case enum_scm_spec_higher:	//make view higher
		if(spectrumControl.iHighMul<8)
		{
			++spectrumControl.iHighMul;
			Modes_OnShow();
		}
		break;
	case enum_scm_spec_movment:	//move spectrum left right
		spectrumControl.iHighMul = 0;	//clear high adjust
		if(spectrumControl.iViewChannelFrom>0)
		{
			spectrumControl.iViewChannelFrom-=10;
			spectrumControl.iViewChannelTo-=10;
			if(spectrumControl.iViewChannelFrom<0)
			{
				spectrumControl.iViewChannelTo-=spectrumControl.iViewChannelFrom;
				spectrumControl.iViewChannelFrom=0;
			}
			if(spectrumControl.iMarkerChannel<spectrumControl.iViewChannelFrom)
				spectrumControl.iMarkerChannel=spectrumControl.iViewChannelFrom;
			else
			if(spectrumControl.iMarkerChannel>spectrumControl.iViewChannelTo)
				spectrumControl.iMarkerChannel=spectrumControl.iViewChannelTo;
			Modes_OnShow();
		}
		break;
	default:
		exception(__FILE__,__FUNCTION__,__LINE__,"Internal error!");
	};
	return 1;
}
BOOL Spectrum_OnRight(void)
{
	int step;
	spectrumControl.bLeftRightClicked = TRUE;
	switch(spectrumControl.iSpectrumControlMode)
	{
	case enum_scm_spec_window:	//window mode
	case enum_scm_marker_movment:
		Spectrum_showMarker();
		step = (keyboard_safe.keyRight.pressedTime>>9)+1;
		if(step>20)step = 20;
		spectrumControl.iMarkerChannel+=step;
		if(spectrumControl.iMarkerChannel>spectrumControl.iViewChannelTo)
			spectrumControl.iMarkerChannel -= (spectrumControl.iViewChannelTo-spectrumControl.iViewChannelFrom+1);
		Spectrum_showMarker();
		Spectrum_showMarkerData();
		if(spectrumControl.iMarkerChannel2!=-1)
		{
			Spectrum_calcCount();
			Spectrum_showAcqData();
		}
		Spectrum_peakProc();
		Spectrum_showPeakProcResult();
		break;
	case enum_scm_spec_view:
		spectrumControl.bLogView = FALSE;
		Modes_OnShow();
		break;
	case enum_scm_spec_acquire:
		InterProc_stopSpectrumAcq();
		//19/03/2010 switch to marker mode
		spectrumControl.iSpectrumControlMode = enum_scm_marker_movment;
		Modes_showButtons();
		break;		
	case enum_scm_spec_wider:
		spectrumControl.iHighMul = 0;	//clear high adjust
		if((spectrumControl.iViewChannelTo-spectrumControl.iViewChannelFrom+1)<CHANNELS)
		{
			spectrumControl.iViewChannelFrom -= (spectrumControl.iMarkerChannel-spectrumControl.iViewChannelFrom)/2;
			spectrumControl.iViewChannelTo += (spectrumControl.iViewChannelTo-spectrumControl.iMarkerChannel)/2;
			if(spectrumControl.iViewChannelFrom<0)
				spectrumControl.iViewChannelFrom = 0;
			if(spectrumControl.iViewChannelTo>CHANNELS-1)
				spectrumControl.iViewChannelTo=CHANNELS-1;
			Modes_OnShow();
		}
		break;
	case enum_scm_spec_higher:
		if(spectrumControl.iHighMul>0)
		{
			--spectrumControl.iHighMul;
			Modes_OnShow();
		}
		break;
	case enum_scm_spec_movment:
		spectrumControl.iHighMul = 0;	//clear high adjust
		if(spectrumControl.iViewChannelTo<(CHANNELS-1))
		{
			spectrumControl.iViewChannelFrom+=10;
			spectrumControl.iViewChannelTo+=10;
			if(spectrumControl.iViewChannelTo>(CHANNELS-1))
			{
				int raz = spectrumControl.iViewChannelTo-(CHANNELS-1);
				spectrumControl.iViewChannelFrom-=raz;
				spectrumControl.iViewChannelTo-=raz;
			}
			if(spectrumControl.iMarkerChannel<spectrumControl.iViewChannelFrom)
				spectrumControl.iMarkerChannel=spectrumControl.iViewChannelFrom;
			else
			if(spectrumControl.iMarkerChannel>spectrumControl.iViewChannelTo)
				spectrumControl.iMarkerChannel=spectrumControl.iViewChannelTo;
			Modes_OnShow();
		}
		break;
	default:
		exception(__FILE__,__FUNCTION__,__LINE__,"Internal error!");
	};
	return 1;
}





//переключение режимов работы со спектром
BOOL Spectrum_OnUp(void)
{
	//здесь реализован механизм перехода на подрежим упр маркером если было исп др подрежимов
	//если не было, то переход дальше по подрежимам
	if(spectrumControl.bLeftRightClicked && spectrumControl.iSpectrumControlMode>0)
	{
		spectrumControl.iSpectrumControlMode = enum_scm_marker_movment;
	}else
	{
		do
		{
			if(++spectrumControl.iSpectrumControlMode>=MAX_SPEC_CTRL_MODES)
				spectrumControl.iSpectrumControlMode = enum_scm_marker_movment;
		}while(spectrumControl.iSpectrumControlMode==enum_scm_spec_movment &&
			   (spectrumControl.iViewChannelTo-spectrumControl.iViewChannelFrom+1)==CHANNELS);
	}
	if(spectrumControl.iSpectrumControlMode==enum_scm_spec_window)
	{//place second marker	
		spectrumControl.iMarkerChannel2 = spectrumControl.iMarkerChannel;
		spectrumControl.iMarkerChannel += 4;
		if(spectrumControl.iMarkerChannel>spectrumControl.iViewChannelTo)
			spectrumControl.iMarkerChannel -= 8;
		Spectrum_calcCount();
	}else if(spectrumControl.iMarkerChannel2 != -1)
	{//remove second marker
		spectrumControl.iMarkerChannel2 = -1;
		Spectrum_calcCount();
	}
	spectrumControl.bLeftRightClicked = FALSE;
	Modes_showButtons();
	return (BOOL)1;
}


//common procedure of start spectrum acquiring
void Spectrum_startAcq(void)
{
	Spectrum_startAcq_ex(spectrumControl.iAcquiringTime);
}


//common procedure of start spectrum acquiring
//copy to spectrum date time stamp
//copy to spectrum gps data
//copy to spectrum temperature
void Spectrum_startAcq_ex(int acqTime)
{
	spectrumControl.pShowSpectrum = &spectrumControl.acqSpectrum;
	spectrumControl.acqSpectrum.wRealTime = clockData.dwTotalSecondsFromStart;
	InterProc_resetSpectrum();
	InterProc_setAcqTime((WORD)acqTime);
	InterProc_startSpectrumAcq();
	//update datetime, gps, temperature value
	memcpy((void*)&spectrumControl.pShowSpectrum->dateTime, (const void*)&clockData.dateTime, sizeof(clockData.dateTime));
}


BOOL Spectrum_OnDown(void)
{
	Modes_activateMenu(&spectrum_menu);
	return 1;
}
BOOL Spectrum_OnIdle(void)
{
	return 1;
}
BOOL Spectrum_OnShow(void)
{
	Spectrum_showModeScreen();
	return 1;
}
BOOL Spectrum_OnExit(void)
{
	//возвр набираемый спектр как основной
	spectrumControl.pShowSpectrum = &spectrumControl.acqSpectrum;
	return 1;
}
BOOL Spectrum_OnWakeUp(void)
{
	return 1;
}
BOOL Spectrum_OnPowerDown(void)
{
	return 0;	//disable enter power down
}

void Spectrum_showModeScreen(void)
{
	Spectrum_showAcqData();
	Spectrum_showPeakProcResult();
	Spectrum_showSpectrum();
	if(modeControl.pMode==&modes_SpectrumMode)
	{
		Spectrum_showMarker();
		Spectrum_showMarkerData();
	}
	Spectrum_showID();
}





void Spectrum_showID(void)
{
	Display_setTextColor(ORANGE);	//set text color
	Display_setTextWin(X_SCREEN_SIZE/2,MODE_USER_TOP+56,X_SCREEN_SIZE/2,82);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(LEFT);
	Display_setCurrentFont(fnt8x16);	//set current font
	Display_setTextLineClear(1);
	Display_checkForClearLine();
	Display_outputText(identifyControl.report);
	Display_setTextLineClear(0);
}




void Spectrum_showPeakProcResult(void)
{
	Display_setTextColor(ORANGE);	//set text color
	Display_setTextWin(0,MODE_USER_TOP+18,X_SCREEN_SIZE,36);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(LEFT);
	Display_setCurrentFont(fnt8x16);	//set current font
	Display_setTextLineClear(1);
	Display_checkForClearLine();
	Display_outputText(spectrumControl.peakProcRes);
	Display_setTextLineClear(0);
}












void Spectrum_showAcqData(void)
{
	Display_setTextColor(GREEN);	//set text color
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,18);	//set text window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(LEFT);
	Display_setCurrentFont(fnt8x16);	//set current font
	float cps = SPRDModeControl.fCps;
	UINT acqt = spectrumControl.pShowSpectrum->wAcqTime;
	UINT cnt = spectrumControl.pShowSpectrum->dwCount;
		
	char buf[50];
	sprintf(buf,"%u s   %u cnt   %.1f cps",(UINT)acqt, (UINT)cnt, cps);
	Display_clearTextWin(10);
	Display_outputText(buf);
}

void Spectrum_showMarkerData(void)
{
	Display_setTextColor(GREEN);	//set text color
	Display_setTextWin(0,Y_SCREEN_MAX-48,X_SCREEN_SIZE,16);	//set text window
	Display_setTextWrap(0);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(LEFT);
	Display_setCurrentFont(fnt8x16);	//set current font
	char buf[50];
	sprintf(buf,"%u ch   %u keV   %u cnt",(UINT)spectrumControl.iMarkerChannel, (UINT)spectrumControl.warEnergy[spectrumControl.iMarkerChannel], (UINT)spectrumControl.pShowSpectrum->dwarSpectrum[spectrumControl.iMarkerChannel]);
	Display_clearTextWin(100);
	Display_outputText(buf);
}


void Spectrum_showSpectrum(void)
{
	DWORD val, valmax=0;
	//find maximum to scale
	for(int i=spectrumControl.iViewChannelFrom;i<=spectrumControl.iViewChannelTo;i++)
	{
		val = spectrumControl.pShowSpectrum->dwarSpectrum[i];
		
		if(spectrumControl.bLogView && val)
		{
			val = (DWORD)(100.0*log((float)val));
		}
		if(val>valmax)valmax=val;
	}
	
	RECT rect = {SPECTRUM_WIN_LEFT, SPECTRUM_WIN_TOP, SPECTRUM_WIN_LEFT+SPECTRUM_WIN_WIDTH-1,SPECTRUM_WIN_BOTTOM};
	Display_clearRect(rect,100);
	
	if(!valmax)return;
	
	//Display_set_display_entrymode(1/*bRGBmode*/, 1/*bUpToDown*/);
	
	int x=SPECTRUM_WIN_LEFT,y,yy;
	int y2;
	
	y=yy=y2=SPECTRUM_WIN_BOTTOM;

	
	int dx = SPECTRUM_WIN_WIDTH;	//width of spectrum win in dots
	int dy = spectrumControl.iViewChannelTo - spectrumControl.iViewChannelFrom +1;				//width of spectrum win in channels
	if(SPECTRUM_WIN_WIDTH<dy)
	{
		int aux = dx;
		dx = dy;
		dy = aux;
	}
	int denom = dx<<1;
	int t = dy<<1;
	int e = -dx;

	DWORD highDiv = 0;
	DWORD highMul = 0;
	while(valmax>SPECTRUM_WIN_HEIGHT-1)
	{
		highDiv++;
		valmax>>=1;
	}
	while(valmax<SPECTRUM_WIN_HEIGHT/2-1)
	{
		highMul++;
		valmax<<=1;
	}
		
	highMul += spectrumControl.iHighMul;

	if(spectrumControl.iViewChannelFrom!=0)
	{
		val = spectrumControl.pShowSpectrum->dwarSpectrum[spectrumControl.iViewChannelFrom];
		
		if(spectrumControl.bLogView && val)
		{
			val = (DWORD)(100.0*log((float)val));
		}
		y2 = (DWORD)SPECTRUM_WIN_BOTTOM-(DWORD)((val<<highMul)>>highDiv);
		if(y2>SPECTRUM_WIN_BOTTOM)
			y2 = SPECTRUM_WIN_BOTTOM;
		else
		if(y2<SPECTRUM_WIN_TOP)
			y2 = SPECTRUM_WIN_TOP;
	}

	
	if(spectrumControl.bSpectrumInDots)
	{
		for(int i=spectrumControl.iViewChannelFrom;i<=spectrumControl.iViewChannelTo;i++)
		{
		val = spectrumControl.pShowSpectrum->dwarSpectrum[i];
		
			if(spectrumControl.bLogView && val)
			{
				val = (DWORD)(100.0*log((float)val));
			}
			y = (DWORD)SPECTRUM_WIN_BOTTOM-(DWORD)((val<<highMul)>>highDiv);
			if(y >= SPECTRUM_WIN_TOP && y<=SPECTRUM_WIN_BOTTOM)
				Display_dot((int)x,(int)y,YELLOW);
			//calc to go to the next x position		
			if((e+=t)>0)
			{
				x++;
				e-=denom;
			}
		}
	}else
	{
		valmax = 0;
		for(int i=spectrumControl.iViewChannelFrom;i<=spectrumControl.iViewChannelTo;i++)
		{
			val = spectrumControl.pShowSpectrum->dwarSpectrum[i];
			if(val>valmax)
				valmax=val;
			//calc to go to the next x position		
			if((e+=t)>0)
			{
				val = valmax;
				valmax=0;
				if(spectrumControl.bLogView && val)
				{
					val = (DWORD)(100.0*log((float)val));
				}
				yy = (DWORD)SPECTRUM_WIN_BOTTOM-(DWORD)((val<<highMul)>>highDiv);
				e-=denom;
				if(yy>SPECTRUM_WIN_BOTTOM)
					yy = SPECTRUM_WIN_BOTTOM;
				else if(yy<SPECTRUM_WIN_TOP)
					yy = SPECTRUM_WIN_TOP;
				Display_drawVLine(x,y2,yy, YELLOW);
				x++;
				y2=yy;
			}
		}
	}	
}

/*
show basic marker and second marker if it is placed
markers have diff colors
*/
void Spectrum_showMarker(void)
{
	int ch = (spectrumControl.iViewChannelTo-spectrumControl.iViewChannelFrom+1);
	int x = SPECTRUM_WIN_LEFT + (int)((float)((spectrumControl.iMarkerChannel-spectrumControl.iViewChannelFrom)*SPECTRUM_WIN_WIDTH)/(float)ch+0.5);
	Display_drawVLine_xor(x,SPECTRUM_WIN_BOTTOM,SPECTRUM_WIN_TOP, YELLOW);
	if(spectrumControl.iMarkerChannel2!=-1)
	{
		x = SPECTRUM_WIN_LEFT + (int)((float)((spectrumControl.iMarkerChannel2-spectrumControl.iViewChannelFrom)*SPECTRUM_WIN_WIDTH)/(float)ch+0.5);
		if(x>=SPECTRUM_WIN_LEFT && x<(SPECTRUM_WIN_LEFT+SPECTRUM_WIN_WIDTH))
			Display_drawVLine_xor(x,SPECTRUM_WIN_BOTTOM,SPECTRUM_WIN_TOP, YELLOW);
	}
}


//set acq time
//item that enable another mode must return -1
BOOL Spectrum_menu1_acqtime(void)
{
	EditMode_EditInt_ex("Spectrum acquiring time\0""Spectrum acquiring time\0""Spectrum acquiring time\0""Время набора спектра",
					 spectrumControl.iAcquiringTime,
					 0,
					 MAX_ACQ_TIME,
					 "second\0""second\0""second\0""секунд",
					 Spectrum_menu1_acqtime_edit_done,
					 TRUE);
	return (BOOL)-1;
}

void Spectrum_menu1_acqtime_edit_done(BOOL bOK)
{
	if(bOK)
	{//confirm edition
		spectrumControl.iAcquiringTime = atoi(EditModeControl.edit_buf);
		//start acquiring
		Spectrum_startAcq();
	}
	Modes_setActiveMode(&modes_SpectrumMode);
}







//save spectrum
BOOL Spectrum_menu1_savespectrum(void)
{
	EditMode_EditText("Save spectrum\rEnter spectrum name\0""Save spectrum\rEnter spectrum name\0""Save spectrum\rEnter spectrum name\0""Сохранение спектра\rВведите имя спектра",
					  FILE_NAME_SZ-1,
					  Spectrum_menu1_savespectrum_done);
	return (BOOL)-1;
}

void Spectrum_menu1_savespectrum_done(BOOL bOK)
{
	if(bOK)
	{
		//count realtime
		if(!spectrumControl.bStopAcq)
			spectrumControl.acqSpectrum.wRealTime = clockData.dwTotalSecondsFromStart-spectrumControl.acqSpectrum.wRealTime;
		int iret = Spectrum_save(EditModeControl.edit_buf, FALSE);
		if(iret==0)
		{//nomemory
			SoundControl_BeepSeq(beepSeq_NOK);
			SETUPMode_clear_memory();
			return;
		}else if(iret==-1)
		{
			SoundControl_BeepSeq(beepSeq_WARN);
			//filename changed, we should say that
			Spectrum_showMessageOnFileNamechange(EditModeControl.edit_buf);
			return;
		}else
			SoundControl_BeepSeq(beepSeq_OK);
	}
	Modes_setActiveMode(&modes_SpectrumMode);
}


BOOL Spectrum_menu1_gaincode(void)
{
//	spectrumControl.pCurMode = modeControl.pMode;
	EditMode_EditFloat("Amplifier gain code\0""Amplifier gain code\0""Amplifier gain code\0""Код усиления усилителя",
					 interProcControl.rsModbus.fStabGain,
					 0.0,
					 4095.0,
					 "\0""\0""\0""",
					 "%.0f",
					 Spectrum_menu1_gaincode_edit_done);
	return (BOOL)-1;
}

void Spectrum_menu1_gaincode_edit_done(BOOL bOK)
{
	if(bOK)
	{
		interProcControl.rsModbus.fStabGain = (float)atof(EditModeControl.edit_buf);
		InterProc_setGain();
		Spectrum_startAcq();		
	}
	Modes_setActiveMode(&modes_SpectrumMode);
}










BOOL Spectrum_menu1_lowlimit(void)
{
	EditMode_EditInt("Lower limit\0""Lower limit\0""Lower limit\0""Нижний порог",
					 interProcControl.rsModbus.wdLowLimit,
					 0,
					 300,
					 "\0""\0""\0""",
					 Spectrum_menu1_lowlimit_edit_done);
	return (BOOL)-1;
}

void Spectrum_menu1_lowlimit_edit_done(BOOL bOK)
{
	if(bOK)
	{
		interProcControl.rsModbus.wdLowLimit = (WORD)atoi(EditModeControl.edit_buf);
		InterProc_setLowLimit(interProcControl.rsModbus.wdLowLimit);
		Spectrum_startAcq();		
		if(!ini_write_system_ini_int("interProcControl", "wdLowLimit", interProcControl.rsModbus.wdLowLimit))
		{
			;//!!!!!!!!!error
		}
	}
	Modes_setActiveMode(&modes_SpectrumMode);
}

BOOL Spectrum_menu1_highlimit(void)
{
	EditMode_EditInt("Upper limit\0""Upper limit\0""Upper limit\0""Верхний порог",
					 interProcControl.rsModbus.wdHighLimit,
					 3000,
					 4095,
					 "\0""\0""\0""",
					 Spectrum_menu1_highlimit_edit_done);
	return (BOOL)-1;
}

void Spectrum_menu1_highlimit_edit_done(BOOL bOK)
{
	if(bOK)
	{
		interProcControl.rsModbus.wdHighLimit = (WORD)atoi(EditModeControl.edit_buf);
		InterProc_setHighLimit(interProcControl.rsModbus.wdHighLimit);
		Spectrum_startAcq();		
		if(!ini_write_system_ini_int("interProcControl", "wdHighLimit", interProcControl.rsModbus.wdHighLimit))
		{
			;//!!!!!!!!!error
		}
	}
	Modes_setActiveMode(&modes_SpectrumMode);
}


















//switch to easy mode item update
const char* Spectrum_menu1_easyMode_onUpdate(void)
{
#ifdef _IAEA
	return "Search mode\0""Search mode\0""Search mode\0""Режим Поиск";
#else
	return "SPRD mode\0""SPRD mode\0""SPRD mode\0""Режим СПРД";
#endif
}


const char* Spectrum_menu1_acqtime_onUpdate(void)
{
	if(modeControl.pMode!=&modes_SpectrumMode)
		return NULL;
	else
		return "Acquiring time\0""Acquiring time\0""Acquiring time\0""Время набора";
}


//сохранить можно только набираемый спектр
const char* Spectrum_menu1_savespectrum_onUpdate(void)
{
	if(spectrumControl.pShowSpectrum == &spectrumControl.opnSpectrum)
		return NULL;	//в режиме просмотра файла спектра нельзя его сохранить
	else
		return "Save spectrum\0""Save spectrum\0""Save spectrum\0""Сохранить спектр";
}
const char* Spectrum_menu1_openspectrum_onUpdate(void)
{
	return "Open spectrum\0""Open spectrum\0""Open spectrum\0""Открыть спектр";
}
const char* Spectrum_menu1_gaincode_onUpdate(void)
{
	if(modeControl.pMode!=&modes_SpectrumMode)
		return NULL;
	else
		return "Gain code\0""Gain code\0""Gain code\0""Код усиления";
}
const char* Spectrum_menu1_lowlimit_onUpdate(void)
{
	if(modeControl.pMode!=&modes_SpectrumMode)
		return NULL;
	else
		return "Lower limit\0""Lower limit\0""Lower limit\0""Нижний порог";
}
const char* Spectrum_menu1_highlimit_onUpdate(void)
{
	if(modeControl.pMode!=&modes_SpectrumMode)
		return NULL;
	else
		return "Upper limit\0""Upper limit\0""Upper limit\0""Верхний порог";
}




















//показать сообщение что имя файла изменилось
void Spectrum_showMessageOnFileNamechange(char* pFileName)
{
	MessageModeControl.pDrawFunction = Spectrum_showMessageOnFileNamechange_onDraw;
	MessageModeControl.pRetFunction = Spectrum_showMessageOnFileNamechange_onExit;
	MessageModeControl.pParam = (const void*)pFileName;
	Modes_setActiveMode(&modes_MessageMode);
}
//выход из режима сообщения 
void Spectrum_showMessageOnFileNamechange_onExit(void)
{
	Modes_setActiveMode(&modes_SpectrumMode);
}
//отбражение текстовой инфы сообщения
void Spectrum_showMessageOnFileNamechange_onDraw(const void* pParam)
{
	Display_outputTextByLang("Spectrum Name exists!\r\0""Spectrum Name exists!\r\0""Spectrum Name exists!\r\0""Имя спектра существует!\r");
	Display_outputTextByLang("Spectrum Name changed to\r\r\0""Spectrum Name changed to\r\r\0""Spectrum Name changed to\r\r\0""Имя спектра изменено на\r\r");
	Display_outputText((const char*)pParam);
}


//generate new file name because this one is exists
//model: filename#001 and so on
void Spectrum_newfilename(char *pFileName)
{
	int filenum = 0;
	int len = strlen(pFileName);
	//look for #
	char* psym = strchr(pFileName, '#');
	int shift;
	int a1,a2,a3;
	if(psym!=NULL)
	{
		shift = (pFileName+FILE_NAME_SZ)-psym;
		if(shift<4)
			psym = NULL;
	}
	if(psym==NULL)
	{//fisrt instance of this file name
		shift = FILE_NAME_SZ-len;
		if(shift<4)
		{//len is not enough
			len-=(4-shift);
		}
		psym = &pFileName[len];
		psym[0] = '#';
		len+=4;
	}else
	{
		//get digits
		a1 = (int)psym[1];
		a2 = (int)psym[2];
		a3 = (int)psym[3];
		if(a1<'0' || a1>'9')a1='0';
		if(a2<'0' || a2>'9')a2='0';
		if(a3<'0' || a3>'9')a3='0';
		filenum = (a1-'0')*100 + (a2-'0')*10 + (a3-'0');
	}

	filenum++;
	a1 = (int)(filenum/100)+'0';
	a2 = (int)(filenum%100)/10+'0';
	a3 = (int)(filenum%10)+'0';
	psym[1] = (char)a1;
	psym[2] = (char)a2;
	psym[3] = (char)a3;
	if(len<FILE_NAME_SZ)
		psym[4]=0;
}
	

















//compress and save spectrum to file
//ret 1 if ok
//ret 0 if no memory
//ret -1 file name changed
//ret -2 battery low
int Spectrum_save(char* pFileName, BOOL bOverwrite)
{
	if(powerControl.bBatteryAlarm)return -2;	//battery alarm
	
	int iret = 1;
	
	//check that file exists
	HFILE hfile = filesystem_find_file(pFileName, "spz");
	if(hfile!=NULL && !bOverwrite)
	{//file exists
		//do no allow to rewrite file
		//change the name of spectrum
		int count = 777;
		do
		{
			PowerControl_kickWatchDog();
			Spectrum_newfilename(pFileName);
			hfile = filesystem_find_file(pFileName, "spz");
		}while(hfile!=NULL && --count);//file exists

		iret = -1;
	}

	
	hfile = filesystem_create_file(pFileName, /*name of the file, will be found and rewrite*/
						"spz", /*ext of the file*/
						bOverwrite	/*1 then exists file will be overwritten*/
					  );
	if(hfile==NULL)return 0;//!!!!!!!! have to show MSG, not exception
//		exception(__FILE__,__FUNCTION__,__LINE__,"No free memory");
	int file_pos = 0;

	
	
	BYTE buf[256];
	BYTE* pBuf = buf;
	int len = 0;
	memcpy(pBuf, &spectrumControl.pShowSpectrum->wAcqTime, sizeof(spectrumControl.pShowSpectrum->wAcqTime));
	pBuf+=sizeof(spectrumControl.pShowSpectrum->wAcqTime);
	len+=sizeof(spectrumControl.pShowSpectrum->wAcqTime);
	memcpy(pBuf, &spectrumControl.pShowSpectrum->dateTime, sizeof(spectrumControl.pShowSpectrum->dateTime));
	pBuf+=sizeof(spectrumControl.pShowSpectrum->dateTime);
	len+=sizeof(spectrumControl.pShowSpectrum->dateTime);
	memcpy(pBuf, &spectrumControl.pShowSpectrum->commonGPS, sizeof(spectrumControl.pShowSpectrum->commonGPS));
	pBuf+=sizeof(spectrumControl.pShowSpectrum->commonGPS);
	len+=sizeof(spectrumControl.pShowSpectrum->commonGPS);
	memcpy(pBuf, &spectrumControl.pShowSpectrum->fTemperature, sizeof(spectrumControl.pShowSpectrum->fTemperature));
	pBuf+=sizeof(spectrumControl.pShowSpectrum->fTemperature);
	len+=sizeof(spectrumControl.pShowSpectrum->fTemperature);
	
	char temprep[MAX_REPORT_SYMS];
	memset(temprep,0,sizeof(temprep));
	char tempnuc[NUCLNAMELENGTH*3];
	int reti;
	for(int i=0;i<identifyControl.NUCLNUM;i++)
	{
		reti = identify_getnuclidetxt(i, tempnuc);
		if(reti==0)continue;	//никлид не идентифицирован
		if(reti==-1)break;//	the end
		if((MAX_REPORT_SYMS-strlen(temprep))<=strlen(tempnuc))break;	//больше места нет в буфере
		strncat(temprep, tempnuc, NUCLNAMELENGTH*3);
	}
	
	memcpy(pBuf, &temprep, sizeof(temprep));
	pBuf+=sizeof(temprep);
	len+=sizeof(temprep);
	
	long pval = spectrumControl.pShowSpectrum->dwarSpectrum[0];
	long nval;
	long rval;
	//store first channel as absolute value
	memcpy(pBuf, &pval, sizeof(pval));
	pBuf+=sizeof(pval);
	len+=sizeof(pval);
	for(int i=1;i<CHANNELS;i++)
	{
		if(len>252)
		{//buffer is full, will save in file
			int ret = filesystem_file_put(hfile, /*file descriptor = number of file in file record table*/
									&file_pos, /*it is counter or written bytes, input: start from 0, as output ret pointer for the next non written byte*/
									  (BYTE*)buf, /*send buffer*/
									  len);/*buffer len in bytes*/
			if(ret==E_FAIL)
			{
				filesystem_delete_file(hfile);
				return 0;//!!!!!!!! have to show MSG, not exception
			}
				//exception(__FILE__,__FUNCTION__,__LINE__,"No free memory");
			len = 0;
			pBuf = buf;
		}

		nval = (long)spectrumControl.pShowSpectrum->dwarSpectrum[i];
		rval = nval-pval;
		pval = nval;
		if(rval>32767 || rval<-32768)
		{//3 bytes on channel + 1 mark byte
			*pBuf++ = 0x7f;
			memcpy(pBuf, &rval, 3);
			pBuf+=3;
			len+=4;
		}else if(rval>126 || rval<-127)
		{//2 bytes on channel + 1 mark byte
			*pBuf++ = 0x80;
			memcpy(pBuf, &rval, 2);
			pBuf+=2;
			len+=3;
		}else
		{//1 byte on channel
			*pBuf++=(signed char)rval;
			len++;
		}
	}

	if(len>0)
	{//buffer is full, will save in file
		int ret = filesystem_file_put(hfile, /*file descriptor = number of file in file record table*/
								&file_pos, /*it is counter or written bytes, input: start from 0, as output ret pointer for the next non written byte*/
								  (BYTE*)buf, /*send buffer*/
								  len);/*buffer len in bytes*/
		if(ret==E_FAIL)//!!!!!!!! have to show MSG, not exception
		{
			filesystem_delete_file(hfile);
			return 0;//!!!!!!!! have to show MSG, not exception
		}
//			exception(__FILE__,__FUNCTION__,__LINE__,"No free memory");
	}


	
	//вторая версия файла спектра
	//save doserate
	len = 0;
	pBuf = buf;
	memcpy(pBuf, &spectrumControl.pShowSpectrum->fDoserate, sizeof(spectrumControl.pShowSpectrum->fDoserate));
	pBuf+=sizeof(spectrumControl.pShowSpectrum->fDoserate);
	len+=sizeof(spectrumControl.pShowSpectrum->fDoserate);
	memcpy(pBuf, &spectrumControl.pShowSpectrum->wRealTime, sizeof(spectrumControl.pShowSpectrum->wRealTime));
	pBuf+=sizeof(spectrumControl.pShowSpectrum->wRealTime);
	len+=sizeof(spectrumControl.pShowSpectrum->wRealTime);
	memcpy(pBuf, &spectrumControl.pShowSpectrum->fCps, sizeof(spectrumControl.pShowSpectrum->fCps));
	pBuf+=sizeof(spectrumControl.pShowSpectrum->fCps);
	len+=sizeof(spectrumControl.pShowSpectrum->fCps);
	
	if(len>0)
	{//buffer is full, will save in file
		int ret = filesystem_file_put(hfile, /*file descriptor = number of file in file record table*/
								&file_pos, /*it is counter or written bytes, input: start from 0, as output ret pointer for the next non written byte*/
								  (BYTE*)buf, /*send buffer*/
								  len);/*buffer len in bytes*/
		if(ret==E_FAIL)//!!!!!!!! have to show MSG, not exception
		{
			filesystem_delete_file(hfile);
			return 0;//!!!!!!!! have to show MSG, not exception
		}
//			exception(__FILE__,__FUNCTION__,__LINE__,"No free memory");
	}

	return iret;
}



//open spectrum from file
//ret E_FAIL if no file
//ret S_OK if OK
int Spectrum_open(char* pFileName)
{
	HFILE hfile = filesystem_open_file(pFileName, /*name of the file, will be found*/
						   "spz" /*ext of the file*/
							   );
	return Spectrum_open_ex(hfile);
}

//open spectrum from HFILE
int Spectrum_open_ex(HFILE hfile)
{
	if(hfile==NULL)//!!!!!!!! must be message showing not exception
		exception(__FILE__,__FUNCTION__,__LINE__,"No file");
	
	int file_pos = 0;

	BYTE buf[256];
	BYTE* pBuf = buf;
	int len = 0;
	int expectedlen = sizeof(spectrumControl.opnSpectrum.wAcqTime)+sizeof(spectrumControl.opnSpectrum.dateTime)+
		sizeof(spectrumControl.opnSpectrum.commonGPS)+sizeof(spectrumControl.opnSpectrum.fTemperature);

	
	int rlen = filesystem_file_get(hfile, /*file descriptor = number of file in file record table*/
							&file_pos, /*it is counter or written bytes, input: start from 0, as output ret pointer for the next non written byte*/
							  (BYTE*)buf, /*send buffer*/
							  sizeof(buf));/*buffer len in bytes*/
	if(rlen==E_FAIL || rlen<expectedlen)//!!!!!!!! have to show MSG, not exception
		exception(__FILE__,__FUNCTION__,__LINE__,"Invalid file");
		
	memcpy(&spectrumControl.opnSpectrum.wAcqTime, pBuf, sizeof(spectrumControl.opnSpectrum.wAcqTime));
	pBuf+=sizeof(spectrumControl.opnSpectrum.wAcqTime);
	len+=sizeof(spectrumControl.opnSpectrum.wAcqTime);
	memcpy(&spectrumControl.opnSpectrum.dateTime, pBuf, sizeof(spectrumControl.opnSpectrum.dateTime));
	pBuf+=sizeof(spectrumControl.opnSpectrum.dateTime);
	len+=sizeof(spectrumControl.opnSpectrum.dateTime);
	memcpy(&spectrumControl.opnSpectrum.commonGPS, pBuf, sizeof(spectrumControl.opnSpectrum.commonGPS));
	pBuf+=sizeof(spectrumControl.opnSpectrum.commonGPS);
	len+=sizeof(spectrumControl.opnSpectrum.commonGPS);
	memcpy(&spectrumControl.opnSpectrum.fTemperature, pBuf, sizeof(spectrumControl.opnSpectrum.fTemperature));
	pBuf+=sizeof(spectrumControl.opnSpectrum.fTemperature);
	len+=sizeof(spectrumControl.opnSpectrum.fTemperature);
	memcpy(&spectrumControl.opnSpectrum.report, pBuf, sizeof(spectrumControl.opnSpectrum.report));
	pBuf+=sizeof(spectrumControl.opnSpectrum.report);
	len+=sizeof(spectrumControl.opnSpectrum.report);
	
	long pval;
	long nval;
	long rval;
	signed char bval;
	memcpy(&pval, pBuf, sizeof(pval));
	spectrumControl.opnSpectrum.dwarSpectrum[0] = pval;
	pBuf+=sizeof(pval);
	len+=sizeof(pval);
	for(int i=1;i<CHANNELS;i++)
	{
		if(len>252)
		{//buffer is full, will load next one
			file_pos=file_pos-rlen+len;
			rlen = filesystem_file_get(hfile, /*file descriptor = number of file in file record table*/
									&file_pos, /*it is counter or written bytes, input: start from 0, as output ret pointer for the next non written byte*/
									  (BYTE*)buf, /*send buffer*/
									  sizeof(buf));/*buffer len in bytes*/
			if(rlen==E_FAIL)//!!!!!!!! have to show MSG, not exception
				exception(__FILE__,__FUNCTION__,__LINE__,"Invalid spectrum file");
			len = 0;
			pBuf = buf;
		}

		bval = *pBuf++;
		if(bval==0x7f)
		{//3 bytes
			rval = 0;
			memcpy(&rval, pBuf, 3);
			if(rval&0x00800000)
				rval|=0xff000000;
			pBuf+=3;
			len+=4;
		}else if(bval==-0x80)
		{//2 bytes
			rval = 0;
			memcpy(&rval, pBuf, 2);
			if(rval&0x00008000)
				rval|=0xffff0000;
			pBuf+=2;
			len+=3;
		}else
		{
			rval = (long)bval;
			len++;
		}
		nval = pval+rval;
		spectrumControl.opnSpectrum.dwarSpectrum[i] = nval;
		pval = nval;
	}
	Spectrum_calcCount();
	return S_OK;
}








//read energy calibr from energy.cal
//ret E_FAIL if error
int Spectrum_read_energy_cal(void)
{
	//read energy
	HFILE hfile = filesystem_open_file("energy", /*name of the file, will be found*/
		   "cal" /*ext of the file*/
			   );
	if(hfile==NULL)
		return E_FAIL;
	int items = ini_retrieveTable(hfile, spectrumControl.warChEnTable);
	if(items==0)return E_FAIL;
	spline_calcSpline(spectrumControl.warChEnTable, items, spectrumControl.warEnergy, 1.0);
	spectrumControl.bHasEnergy = TRUE;
	return S_OK;
}


//read sigma calibr from sigma.cal
//ret E_FAIL if error
int Spectrum_read_sigma_cal(void)
{
	short i;
	//read sigma
	HFILE hfile = filesystem_open_file("sigma", /*name of the file, will be found*/
		   "cal" /*ext of the file*/
			   );
	if(hfile==NULL)
		return E_FAIL;
	int items = ini_retrieveTable(hfile, spectrumControl.warChSiTable);
	if(items==0)return E_FAIL;
	spline_calcSpline(spectrumControl.warChSiTable, items, spectrumControl.warSigma, 256.0);
	
	//create sigma for identification that is in 1.5 thinner
#ifdef _THIN_SIGMA
	for(i=0;i<CHANNELS;i++)
		spectrumControl.warSigma[i]=spectrumControl.warSigma[i]/SIGMA_THIN_FACTOR_M*SIGMA_THIN_FACTOR_D;
#endif	//#ifdef _THIN_SIGMA
	
	spectrumControl.bHasSigma = TRUE;
	return S_OK;
}






/////////////////////////////////////////////////////////////////////////////////////////////



//=0 if error
BOOL Spectrum_peakProc_ex(float *position, float* sigma)
{
	long pos = (long)(*position);

	//в окне двух сигм с каждой стороны будем искать максимум
	long sig = 2*spectrumControl.warSigma[pos]/256
#ifdef _THIN_SIGMA
				*SIGMA_THIN_FACTOR_M/SIGMA_THIN_FACTOR_D
#endif	//#ifdef _THIN_SIGMA
		;
	
	long left = pos-sig;
	long right = pos+sig;
	if(left<1)left = 1;
	if(right>=CHANNELS-1)right = CHANNELS-2;
	if(left>right)return FALSE;
	
	long val, max=0;
	for(int i=left;i<=right;i++)
	{
		val = spectrumControl.pShowSpectrum->dwarSpectrum[i];
		if(val>max)
		{
			max = val;
			pos = i;
		}
	}
	//нашли максимум теперь средневзвешенным найдем центр пика по окну 2 сигмы с каждой стороны
	sig = 2*spectrumControl.warSigma[pos]/256
#ifdef _THIN_SIGMA
				*SIGMA_THIN_FACTOR_M/SIGMA_THIN_FACTOR_D
#endif	//#ifdef _THIN_SIGMA
		;
	left = pos-sig;
	right = pos+sig;
	if(left<1)left = 1;
	if(right>=CHANNELS-1)right = CHANNELS-2;
	if(left>right)return FALSE;
	
	long long summ = 0;
	long long msumm = 0;
	long long mean;
	for(int i=left;i<=right;i++)
	{
		mean = (long long)spectrumControl.pShowSpectrum->dwarSpectrum[i];
		mean*=mean;
		summ+=mean;
		msumm+=mean*i;
	}
	
	if(summ<=0)return FALSE;	//empty spectrum

	//определили центр по средневзвешенному
	//теперь задаем края по 3,5 сигмы с каждой стороы и обрабатываем пик
	*position = (float)msumm/(float)summ;
	pos = (long)(*position+0.5);
	
	sig = 3.5*spectrumControl.warSigma[pos]/256
#ifdef _THIN_SIGMA
				*SIGMA_THIN_FACTOR_M/SIGMA_THIN_FACTOR_D
#endif	//#ifdef _THIN_SIGMA
		;
	left = pos-sig;
	right = pos+sig;
	//обязательно с первого по последний -1 канал чтобы сделать усреднение по краям по три точки
	if(left<1)left = 1;
	if(right>=CHANNELS-1)
		right = CHANNELS-2;
	return Spectrum_QuickPeakCalculation(left,right,position,sigma);
}



//быстрая обработка пика
//задаем левую и правую границу и положение центра,
//возврат точная позиция и сигма
//если возврат FALSE то расчет не удался
BOOL Spectrum_QuickPeakCalculation(long left, long right, float *position, float *sigma)
{
	float s,d2,d,d1,k,b;
	if(right-left<=0)return FALSE;
	long lng = (long)(spectrumControl.pShowSpectrum->dwarSpectrum[left-1]+spectrumControl.pShowSpectrum->dwarSpectrum[left]+spectrumControl.pShowSpectrum->dwarSpectrum[left+1]);
	k=(float)((long)(spectrumControl.pShowSpectrum->dwarSpectrum[right-1]+spectrumControl.pShowSpectrum->dwarSpectrum[right]+spectrumControl.pShowSpectrum->dwarSpectrum[right+1])-
			  lng)/(float)(3*(right-left));
	b=(float)lng/3.0-k*left;
	d=0; d1=0; d2=0;
	for(long i=left; i<=right; i++){
		s=((float)spectrumControl.pShowSpectrum->dwarSpectrum[i]-k*i-b); if(s<0) s=0;
		d+=s; s*=i; d1+=s; s*=i; d2+=s;
	}
	if(d<=0)return FALSE;
	(*position)=d1/d;
	k=(d2/d-(*position)*(*position));
	if(k<=0)return FALSE;
	(*sigma)=sqrt(k);
	return TRUE;
}


//process peak using current calibration
//get from center 3 sigmas and process it
void Spectrum_peakProc(void)
{
	memset(spectrumControl.peakProcRes,0,sizeof(spectrumControl.peakProcRes));
	
	if(!spectrumControl.bHasSigma || !spectrumControl.bHasEnergy)return;	//no calibration
	
	float position = spectrumControl.iMarkerChannel;
	float sigma;
	if(spectrumControl.iMarkerChannel2!=-1)
	{//two markers mode, mean that peak is in center
		position = (spectrumControl.iMarkerChannel2+position)/2.0;
	}
	if(Spectrum_peakProc_ex(&position,&sigma))
	{
		const float sqrt2ln2=200.0*1.17774100225; //full width on half height = 2*sqrt2ln2*sigma

		//14.04.2016 added again to be like SpectEx		
		//26/02/2014 commented because we dont need energy resoultion in energies any more
		//06/07/2012 added calculation of reosultion in energies
		if(spectrumControl.bHasEnergy)
		{
			float epos = identify_EnergyFromChannel(position);
			float wd = identify_EnergyFromChannel(position+sigma)-epos;
			sprintf(spectrumControl.peakProcRes, "N=%.1f R=%.1f%%\0", position, wd*sqrt2ln2/epos);
		}else
		{//no energy
			sprintf(spectrumControl.peakProcRes, "N=%.1f R=%.1f%%\0", position, sigma*sqrt2ln2/position);
		}
		//
		
		int en = (int)(identify_EnergyFromChannel(position)+0.5);
		sprintf(spectrumControl.peakProcRes+strlen(spectrumControl.peakProcRes), " E=%u keV\0", (int)en);
		
		int is = position-sigma*3.5, ie = position+sigma*3.5;
		if(is<0)is = 0;
		if(ie>=CHANNELS)ie = CHANNELS-1;
		DWORD dwarea=0;
		for(;is<=ie;is++)dwarea+=spectrumControl.pShowSpectrum->dwarSpectrum[is];
		
		char buf[100];
		memset(buf,0,sizeof(buf));
		
#ifdef BNC
		switch(modeControl.bLang)
		{
		case enu_lang_french:
			strcpy(buf, "\rPA=%u cnt DR=%.3f mrem/h\0");
			break;
		case enu_lang_german:
			strcpy(buf, "\rPA=%u cnt DR=%.3f mrem/h\0");
			break;
		case enu_lang_russian:
			strcpy(buf, "\rПП=%u cnt МД=%.3f mrem/h\0");
			break;
		default:
			strcpy(buf, "\rPA=%u cnt DR=%.3f mrem/h\0");
		}
#else		
		switch(modeControl.bLang)
		{
		case enu_lang_french:
			strcpy(buf, "\rPA=%u cnt DR=%.2f µSv/h\0");
			break;
		case enu_lang_german:
			strcpy(buf, "\rPA=%u cnt DR=%.2f µSv/h\0");
			break;
		case enu_lang_russian:
			strcpy(buf, "\rПП=%u cnt МД=%.2f µSv/h\0");
			break;
		default:
			strcpy(buf, "\rPA=%u cnt DR=%.2f µSv/h\0");
		}
#endif	
		
		
		sprintf(spectrumControl.peakProcRes+strlen(spectrumControl.peakProcRes), buf, (UINT)dwarea, (float)SPRDModeControl.fDoserate);
		
	}else
	{
		spectrumControl.peakProcRes[0]='\r';
	}
}


//setup energy windows to second processor
void Spectrum_setupDoseWindowTable(void)
{
	if(!spectrumControl.bHasEnergy)return;	//no energy
	Spectrum_makeEnergyWins();
	InterProc_setWinTable();
}


//окна включительно справа
#pragma pack(1)
const WORD spectrumDoserateEnergyWin[SD_WIN_SIZE]=
{37,47,57,67,102,139,176,222,268,314,369,424,470,543,904,1472,60000};
#pragma pack()


//make energy windows from energy calibration
void Spectrum_makeEnergyWins(void)
{
	int i,k=0;
	int rz,rz2=-100;
	WORD * spec = spectrumControl.wins1;
	WORD * ener = spectrumControl.warEnergy;
	const WORD * sdew = spectrumDoserateEnergyWin;
	WORD ewin = *sdew;
	spec[k++] = 1;	//start first window from this channel
	for(i=0;i<CHANNELS && k<SD_WIN_SIZE;i++)
	{
		rz = *ener++ - ewin;
		if(rz>=0)
		{
			ewin = *++sdew;
			spec[k++] = -rz2<=rz?i-1:i;
			rz=-100;
		}
		rz2 = rz;
	}
	spec[k] = 0xffff;
}




//open spectrum
BOOL Spectrum_menu1_openspectrum(void)
{
	FileListModeControl.pOnNextPage = Spectrum_menu1_openspectrum_onNextPage;
	FileListModeControl.pOnPrevPage = Spectrum_menu1_openspectrum_onPrevPage;
	FileListModeControl.pRetFunction = Spectrum_menu1_openspectrum_done;
	FileListModeControl.iItemsNum = filesystem_get_dir(
					"spz",
					FileListModeControl.listItems,
					MAX_ITEMS,
					0);
	Modes_setActiveMode(&modes_FileListMode);
	return (BOOL)-1;
}

void Spectrum_menu1_openspectrum_done(BOOL bOK)
{
	Modes_setActiveMode(&modes_SpectrumMode);
	if(bOK)
	{
		Spectrum_open(FileListModeControl.listItems[FileListModeControl.iMarkerPos].name);
		spectrumControl.pShowSpectrum = &spectrumControl.opnSpectrum;
		Modes_updateMode();
	}
}

BOOL Spectrum_menu1_openspectrum_onNextPage(void)
{
	if(FileListModeControl.iItemsNum<MAX_ITEMS)return FALSE;
	FileListModeControl.iItemsNum = filesystem_get_dir(
					"spz",
					FileListModeControl.listItems,
					MAX_ITEMS,
					FileListModeControl.iPage*MAX_ITEMS+MAX_ITEMS);
	if(FileListModeControl.iItemsNum)
		FileListModeControl.iPage++;
	else
	{
	FileListModeControl.iItemsNum = filesystem_get_dir(
					"spz",
					FileListModeControl.listItems,
					MAX_ITEMS,
					FileListModeControl.iPage*MAX_ITEMS);
	}
	return (FileListModeControl.iItemsNum>0);
}


BOOL Spectrum_menu1_openspectrum_onPrevPage(void)
{
	if(FileListModeControl.iPage==0)return FALSE;
	FileListModeControl.iItemsNum = filesystem_get_dir(
					"spz",
					FileListModeControl.listItems,
					MAX_ITEMS,
					FileListModeControl.iPage*MAX_ITEMS-MAX_ITEMS);
	if(FileListModeControl.iItemsNum)
		FileListModeControl.iPage--;
	else
	{
	FileListModeControl.iItemsNum = filesystem_get_dir(
					"spz",
					FileListModeControl.listItems,
					MAX_ITEMS,
					FileListModeControl.iPage*MAX_ITEMS);
	}
	return (FileListModeControl.iItemsNum>0);
}




///////////////////////////////////////////////////////////////////////////////
//RID PSW mode

const char* RID_PSW_LeftOnUpdate(void)//"marker\rleft\0""маркер\rвлево",
{
	return "back\0""back\0""back\0""назад";
}
const char* RID_PSW_RightOnUpdate(void)//"marker\rright\0""маркер\rвправо",
{
	return "t/c\0""t/c\0""t/c\0""т/с";
}


//режимы работы со спектром
//отображаемое слово и есть текущий режим
const char* RID_PSW_UpOnUpdate(void)
{
	return "login\0""login\0""login\0""пароль";
}
const char* RID_PSW_DownOnUpdate(void)//"menu\0""меню",
{
	return NULL;
}

BOOL RID_PSW_OnActivate(void)
{
	Modes_updateMode();
	return 1;
}


BOOL RID_PSW_OnLeft(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return 1;
}

BOOL RID_PSW_OnRight(void)
{
	Modes_setActiveMode(&modes_TCMode);
	return 1;
}

BOOL RID_PSW_OnUp(void)
{
	//enter expert mode
	EditMode_EditInt("Expert mode\rEnter password\0""Expert mode\rEnter password\0""Expert mode\rEnter password\0""Экспертный режим\rВведите пароль",
						  0,
						  0,
						  9999,
						 "\0""\0""\0""",
						  SPRDMode_PSW_done);
	return (BOOL)-1;
}

BOOL RID_PSW_OnDown(void)
{
	return 1;
}



BOOL RID_PSW_OnShow(void)
{
	Display_setTextColor(YELLOW);	//set text color
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,100);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(CENTER);
	Display_setCurrentFont(fnt16x16);
	Display_clearTextWin(10);
	Display_outputTextByLang("Expert mode for spectrum analysis. Password protected. Press LOGIN to enter password.\0""Expert mode for spectrum analysis. Password protected. Press LOGIN to enter password.\0""Expert mode for spectrum analysis. Password protected. Press LOGIN to enter password.\0""Экспертный режим анализа спектра. Защищен паролем. Нажмите ПАРОЛЬ для входа.");
	return 1;
}

BOOL RID_PSW_OnPowerDown(void)
{
	return 1;
}










//delete autospectra
BOOL Spectrum_menu1_deletespec(void)
{
	return SETUPMode_menu1_del_autospz();
}

const char* Spectrum_menu1_deletespec_onUpdate(void)
{
	if(modeControl.pMode!=&modes_SpectrumMode)
		return NULL;
	else
		return "Delete autospectra\0""Delete autospectra\0""Delete autospectra\0""Удалить автоспектры";
}

#ifdef _DIFF_FILTER
//фильтр диф нелинейности
void Spectrum_DiffFilter(void)
{
	long val=0;
	long val2=0;
	for(int i=66;i<CHANNELS-32;i+=64)
	{
		val = (spectrumControl.acqSpectrum.dwarSpectrum[i-1]+spectrumControl.acqSpectrum.dwarSpectrum[i+1])/2;
		val2 = spectrumControl.acqSpectrum.dwarSpectrum[i]-val;
		if(val2>65535 || val2*val2>=16*val)
		{//есть дифнелинейность
			spectrumControl.acqSpectrum.dwarSpectrum[i]-=val2;
			spectrumControl.acqSpectrum.dwarSpectrum[i+32]+=val2;
		}
	}					
}
#endif	//#ifdef _DIFF_FILTER