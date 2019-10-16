//stab.c

#include <stdio.h>
#include <math.h>
#include "stab_mode.h"
#include "interProc.h"
#include "display.h"
#include "spectrum.h"
#include "SPRD_mode.h"
#include "identify.h"


struct tagSTABModeControl STABModeControl;


const struct tagMode modes_STABMode=
{
	STABMode_NameOnUpdate,//
	ORANGE,	//mode color
	STABMode_LeftOnUpdate,//
	STABMode_RightOnUpdate,//
	STABMode_UpOnUpdate,//
	STABMode_DownOnUpdate,//
	STABMode_OnActivate,
	STABMode_OnLeft,
	STABMode_OnRight,
	STABMode_OnUp,
	STABMode_OnDown,
	STABMode_OnIdle,
	STABMode_OnShow,
	STABMode_OnExit,
	STABMode_OnPowerDown,
	STABMode_OnWakeUp,
	STABMode_OnTimer
};




const char* STABMode_NameOnUpdate(void)//
{
	return "STABIL\0""STABIL\0""STABIL\0""������";
}
const char* STABMode_LeftOnUpdate(void)//
{
	if(STABModeControl.stabStep==STB_END)
		return "exit\0""exit\0""exit\0""�����";
	else
		return "cancel\0""cancel\0""cancel\0""������";
}
const char* STABMode_RightOnUpdate(void)//
{
	if(STABModeControl.stabStep!=STB_BEGIN &&
	   STABModeControl.stabStep!=STB_ERROR)return NULL;	//source can not be selected in in those modes
	
	return "rad.\rsource\0""rad.\rsource\0""rad.\rsource\0""���.\r������";
}
const char* STABMode_UpOnUpdate(void)//
{
	if(STABModeControl.stabStep==STB_BEGIN && STABModeControl.bGetGain)
		return "start\0""start\0""start\0""�����";
	else
	if(STABModeControl.stabStep==STB_ERROR ||
	   STABModeControl.stabStep==STB_END)
		return "repeat\0""repeat\0""repeat\0""������";
	else
		return NULL;
}
const char* STABMode_DownOnUpdate(void)//
{
	return NULL;
}









void STABMode_Init(void)
{
	STABModeControl.bGetGain = FALSE;
	if(!spectrumControl.bHasEnergy || !spectrumControl.bHasSigma)
		STABModeControl.stabStep = STB_DISABLED;
	else
		STABModeControl.stabStep = STB_BEGIN;
	STABModeControl.bByCs137 = FALSE;

	STABModeControl.peakFactor = 0;
	STABModeControl.stab_error_cur = 0;
	STABModeControl.stab_error = 0;
	
	STABModeControl.currentPeakPos = 0;	//clear last found peak position

	STABMode_updatePeakCenter();
}


void STABMode_updatePeakCenter(void)
{
	if(STABModeControl.bByCs137)
		STABModeControl.stabilPeakCenter = identify_ChannelFromEnergyNear(662);
	else
		STABModeControl.stabilPeakCenter = identify_ChannelFromEnergyNear(1462);
}


BOOL STABMode_OnActivate(void)
{
	STABMode_Init();
	//get gain code
	InterProc_getGain();
//	InterProc_readAcqTime();
	InterProc_readSpectrumZip();
	Modes_createTimer(1000);
	Modes_updateMode();
	return 1;
}

BOOL STABMode_OnTimer(void)
{
	BOOL bret = FALSE;
	if(!InterProc_isDataFinalReady(&interProcControl.rsModbus.swdStabGain))return 1;
	if(!STABModeControl.bGetGain)
	{//save gain
		STABModeControl.bGetGain = TRUE;
		STABModeControl.fStabGainSaved = interProcControl.rsModbus.fStabGain;	//save gain to restore if need
		Modes_OnShow();
		Modes_showButtons();
	}
	
	if(!InterProc_isDataFinalReady(&interProcControl.rsModbus.sarSpectrum))return 1;

	
	switch(STABModeControl.stabStep)
	{
		case STB_WAITFORSPECTRUM:
			//��������
			if(spectrumControl.acqSpectrum.dwCount>=STABModeControl.cnt_max && spectrumControl.acqSpectrum.wAcqTime>5)
			{
				//make sigm a widther
#ifdef _THIN_SIGMA
				for(int i=0;i<CHANNELS;i++)
				{
					spectrumControl.warSigma[i] = spectrumControl.warSigma[i]*2;
				}
#endif	//#ifdef _THIN_SIGMA
				//get convolution
				bret = identify_convolute();
				//return sigma
#ifdef _THIN_SIGMA
				for(int i=0;i<CHANNELS;i++)
				{
					spectrumControl.warSigma[i] = spectrumControl.warSigma[i]/2;
				}
#endif	//#ifdef _THIN_SIGMA
				if(!bret)
				{//no calibrations
					STABModeControl.stabStep = STB_ERROR;
				}else
				{
					STABModeControl.cnt_max += 1000;
					if(STABMode_getPeakEx()==0)
					{//peak not found
						if(spectrumControl.acqSpectrum.dwCount>STABIL_CNT)
						{//�������� �������� ������ �� �������
							if(interProcControl.rsModbus.fStabGain)
							{//�������� �� 0 ������ ��� ����� ���� �� ������, ������ 0 ����� �� ��������
								//�������� ����� ������� � ������
								//��� �������� ���������
								interProcControl.rsModbus.fStabGain = 0;
								InterProc_setGain();
								
								spectrumControl.acqSpectrum.dwCount = 0;
								STABModeControl.cnt_max = 2000;
								Spectrum_startAcq_ex(MAX_ACQ_TIME);
								InterProc_getGain();
								
								STABModeControl.stabStep = STB_WAITFORSETPEAK;
							}else
							{//���� �������� ���� 0 � ��� ��� � �� ������ �� ������, ����� �� ���, 
								STABModeControl.stabStep = STB_ERROR;
							}
						}
					}else 
					{//��� ������
						STABModeControl.stabStep=STB_STABILIZATION;	//�� ������������
					}
				}
			}
		break;
		case STB_WAITFORSETPEAK:
			//���� ��������� �������� 
			if(spectrumControl.acqSpectrum.dwCount>=STABModeControl.cnt_max && spectrumControl.acqSpectrum.wAcqTime>5)
			{//������ �����, ����� ��������
				STABModeControl.currentPeakPos = 0;	//clear last found peak position
				STABModeControl.peakFactor = 0;
				STABModeControl.peakThreshold = 0;
				STABModeControl.stabStep=STB_WAITFORSPECTRUM;	//�� ������������
				spectrumControl.acqSpectrum.dwCount = 0;
				STABModeControl.cnt_max = 2000;	
				Spectrum_startAcq_ex(MAX_ACQ_TIME);
				InterProc_getGain();
			}
		break;
		case STB_STABILIZATION://�������� ������������
			STABMode_proccessPeak();
		break;
		case STB_WAIT_STAB:
			if(interProcControl.rsModbus.fStabGain>0 && interProcControl.rsModbus.fStabGain<4096)
			{//�������� � ����� �� ��, ���� �� �������� ��������� ��������
				STABModeControl.cnt_max = 2000;
				spectrumControl.acqSpectrum.dwCount = 0;
				Spectrum_startAcq_ex(MAX_ACQ_TIME);
				InterProc_getGain();
				STABModeControl.stabStep=STB_WAITFORSETPEAK;
			}else
			{//��� �� �������� ������������, �� ����� ���� ����������, ������
				STABModeControl.stabStep = STB_ERROR;
			}
		break;
		default:
			return 1;	//just exit procedure
	}

//	InterProc_readAcqTime();
	InterProc_readSpectrumZip();
	
	Modes_OnShow();
	Modes_showButtons();
	return 1;
}


BOOL STABMode_OnLeft(void)
{
	Modes_setActiveMode(&modes_SPRDMode);
	return 1;
}
BOOL STABMode_OnRight(void)
{
	InterProc_getGain();
	STABModeControl.bByCs137 = !STABModeControl.bByCs137;
	STABMode_updatePeakCenter();
	Modes_showButtons();
	Modes_OnShow();
	return 1;
}
BOOL STABMode_OnUp(void)
{
	switch(STABModeControl.stabStep)
	{
		case STB_ERROR:
		case STB_BEGIN:
		case STB_END:

			STABModeControl.currentPeakPos = 0;	//clear last found peak position
			
			STABMode_updatePeakCenter();
			
			InterProc_setStabPeak(STABModeControl.stabilPeakCenter);
			
			STABModeControl.cnt_max = 2000;
			spectrumControl.acqSpectrum.dwCount = 0;
			
			interProcControl.rsModbus.fStabGain = 0;
			InterProc_setGain();
			//05/12/2011 �����������: ������ ����� ������� ��������� �������� ����� ������� ������, ����� ���� ����� ����� ������� ����� �������� �������� � 0
			STABModeControl.stabStep = STB_WAITFORSETPEAK;//STB_WAITFORSPECTRUM;
			
			Spectrum_startAcq_ex(MAX_ACQ_TIME);
			InterProc_getGain();
//			InterProc_readAcqTime();
			InterProc_readSpectrumZip();
			
			Modes_showButtons();
		break;
		default:
		;
	}
	return 1;
}
BOOL STABMode_OnDown(void)
{
	return 1;
}
BOOL STABMode_OnIdle(void)
{
	return 1;
}
BOOL STABMode_OnShow(void)
{
	STABMode_showModeScreen();
	return 1;
}
BOOL STABMode_OnExit(void)
{
	if(STABModeControl.stabStep!=STB_END && 
	   STABModeControl.stabStep!=STB_BEGIN &&
	   STABModeControl.bGetGain)
	{
		interProcControl.rsModbus.fStabGain = STABModeControl.fStabGainSaved;
		InterProc_setGain();
	}
	return 1;
}
BOOL STABMode_OnWakeUp(void)
{
	return 1;
}
BOOL STABMode_OnPowerDown(void)
{
	return 0;	//disable enter power down
}







void STABMode_showModeScreen(void)
{
	char buf[100];
	Display_setTextWin(0,MODE_USER_TOP,X_SCREEN_SIZE,16);	//set text window
	Display_setTextXY(0,0);	//set start coords in window
	Display_setTextWrap(1);
	Display_setTextSteps(1,1);
	Display_setTextDoubleHeight(0);
	Display_setTextJustify(LEFT);
	Display_setCurrentFont(fnt16x16);
	Display_setTextLineClear(1);
	Display_checkForClearLine();
	Display_setTextColor(ORANGE);	//set text color
	Display_outputTextByLang("Stabilization\0""Stabilization\0""Stabilization\0""������������");
	Display_outputText("\r");
	Display_setTextColor(YELLOW);	//set text color
	
	switch(STABModeControl.stabStep)
	{
		case STB_BEGIN:
			if(STABModeControl.bByCs137)
			{
				Display_outputTextByLang("Place \0""Place \0""Place \0""��������� �������� � ");
				Display_setTextJustify(NONE);
				Display_setTextColor(RED);	//set text color
				Display_outputTextByLang("Cs137\0""Cs137\0""Cs137\0""Cs137");
				Display_setTextColor(YELLOW);	//set text color
				Display_outputTextByLang(" source to the upper-right corner of the device.\0"" source to the upper-right corner of the device.\0"" source to the upper-right corner of the device.\0"" � ������� ����� ����� �������.");
				Display_setTextJustify(LEFT);
			}else
			{
				Display_outputTextByLang("Place device onto \0""Place device onto \0""Place device onto \0""�������� ������ �� ");
				Display_setTextJustify(NONE);
				Display_setTextColor(RED);	//set text color
				Display_outputTextByLang("check sample.\0""check sample.\0""check sample.\0""����������� �����.");
				Display_setTextColor(YELLOW);	//set text color
				Display_setTextJustify(LEFT);
			}
			Display_outputText("\r");
			Display_outputTextByLang("Be sure there are no other radiation sources around.\0""Be sure there are no other radiation sources around.\0""Be sure there are no other radiation sources around.\0""���������, ��� ��� ������ ���������� ��������.");
			Display_outputText("\r");
		break;
		case STB_WAITFORSPECTRUM:
			Display_outputTextByLang("Spectrum acquiring...\0""Spectrum acquiring...\0""Spectrum acquiring...\0""����� �������...");
			Display_outputText("\r");
		break;
		case STB_WAIT_STAB:
		case STB_WAITFORSETPEAK:
			Display_outputTextByLang("Gain setting up...\0""Gain setting up...\0""Gain setting up...\0""��������� ��������...");
			Display_outputText("\r");
		break;
		case STB_STABILIZATION:
			Display_outputTextByLang("Stabilizing...\0""Stabilizing...\0""Stabilizing...\0""������������...");
			Display_outputText("\r");
		break;
		case STB_END:
			Display_setTextColor(GREEN);
			Display_outputTextByLang("Complete.\0""Complete.\0""Complete.\0""���������.");
			Display_outputText("\r");
		break;
		case STB_ERROR:
			Display_setTextColor(RED);
			Display_outputTextByLang("Failed! Repeat stabilization when no other radiation sources are around.\0""Failed! Repeat stabilization when no other radiation sources are around.\0""Failed! Repeat stabilization when no other radiation sources are around.\0""�� ���������! ��������� ������������ ��� ���������� ������ ���������� ��������.");
			Display_outputText("\r");
		break;
		case STB_DISABLED:
			Display_setTextColor(RED);
			Display_outputTextByLang("Disabled! No calibrations.\0""Disabled! No calibrations.\0""Disabled! No calibrations.\0""����������! ��� ����������.");
			Display_outputText("\r");
			break;
		default:;
	}
	
	if(STABModeControl.stabStep!=STB_DISABLED)
	{
		Display_setTextJustify(NONE);
		Display_outputText("\r");
		Display_outputTextByLang("Counts: \0""Counts: \0""Counts: \0""����: ");
		sprintf(buf,"%u\r", spectrumControl.acqSpectrum.dwCount);
		Display_outputText(buf);
		Display_outputTextByLang("Ref.position: \0""Ref.position: \0""Ref.position: \0""������.�������: ");
		sprintf(buf,"%.1f\r", STABModeControl.stabilPeakCenter);
		Display_outputText(buf);
		Display_outputTextByLang("Found peak: \0""Found peak: \0""Found peak: \0""������ ���: ");
		sprintf(buf,"%.1f\r", STABModeControl.currentPeakPos);
		Display_outputText(buf);
		Display_outputTextByLang("Gain code: \0""Gain code: \0""Gain code: \0""��� ��������: ");
		sprintf(buf,"%.1f", interProcControl.rsModbus.fStabGain);
		Display_outputText(buf);
	}
	
	Display_setTextJustify(LEFT);
	
	Display_clearUserPart();
	Display_setTextLineClear(0);
}




////////////////////////////

void STABMode_proccessPeak(void)
{
	if(STABModeControl.stab_error_cur<=STABModeControl.stab_error)
	{
		STABModeControl.stabStep = STB_END;//���� ������� ���������
	}else
	{
		//������� � �� ������� ��������� ����
		InterProc_setCurPeak(STABModeControl.currentPeakPos);
		//������� ������������
		InterProc_stabilize();
		//�������� ��������
		InterProc_getGain();
		//�� �������� ��������� ��������
		STABModeControl.stabStep = STB_WAIT_STAB;
	}
}

BYTE STABMode_getPeakEx(void)
{
	float fvar;
	short cmax = STABModeControl.stabilPeakCenter*1.5;
	short cmin = STABModeControl.stabilPeakCenter/2;
	BYTE i;
	short cleft, cright, ii;
	long mean, max, prev;
	short chanMax;
	prev=identifyControl.BufSpec[cmax];
	if(prev<0)
		prev=0;
	max=prev;
	chanMax=0;
	cleft=0;
	cright=0;
	ii = cmax;
	i=0;
	STABModeControl.peakFactor = 0;
	STABModeControl.peakThreshold = 0;
	STABModeControl.stab_error = 0;
	STABModeControl.stab_error_cur = 0;
	//����� ��������� � �������
	while(cmax>=cmin)
	{
		mean = identifyControl.BufSpec[--cmax];

		//����������� ������� ������
		if(mean<0)
			mean=0;
		if(mean>max && cmax>=cmin)
		{
			max=mean;
			chanMax = cmax;
			//��� ������ ��������� ���������� ����� �������
			cleft = 0;
			if(!i)
			{//��� �� ���������� ������ �������
				ii = cmax;
				i = 1;	//����� ����� ���������� �� �����������
			}
			//��� ������ ��������� ���������� ������ �������
			cright = ii;
		}else
		{
			if(prev && ((float)fabs(prev-mean)/(float)prev)<0.01)
				mean = prev;

			if(mean>prev && cmax>=cmin)
			{//����� ������ 
				if(!i)
				{//��� �� ���������� ������ �������
					ii = cmax;
					i = 1;	//����� ����� ���������� �� �����������
					if(!chanMax)//���� ������������ ���� ������ ����� ������ �����, �� �� ��������� ������ ������ ������� ��������
						max = 0;
				}
				if(!cleft)//��� �� ���������� ����� �������
				{
					cleft = cmax+1;
				}
			}else
			{
				if(mean<prev)
					i=0;	//�.�. ��� ����� �����, �� ����� �������� �� ���������� ������� ��������
				if(mean==0 || cmax<cmin)
				{//���� ������� ������ 0-���� ��� ������ ��������
					if(!cleft)
						cleft = cmax+1;
//////////
					if(chanMax>0 && cright>0)
					{
						//�������� ����������� ������ ��� ������ �������� ������� ����
						fvar = (float)max/sqrt(identifyControl.SDs[chanMax]);
						if(fvar>=BASE_PEAK_THRESHOLD)
						{
							STABModeControl.peakFactor = fvar;
							STABModeControl.currentPeakPos=chanMax;
							STABModeControl.peakThreshold = 11000;
							STABModeControl.peakThreshold*=BASE_PEAK_THRESHOLD;
							STABModeControl.peakThreshold /= STABModeControl.currentPeakPos*sqrt(STABModeControl.currentPeakPos);
							if(STABModeControl.peakThreshold>40)STABModeControl.peakThreshold=40;
	
							STABModeControl.stab_error = BASE_STAB_ERROR*BASE_PEAK_THRESHOLD/STABModeControl.peakFactor;
							if(STABModeControl.stab_error<BASE_STAB_ERROR_MIN)STABModeControl.stab_error = BASE_STAB_ERROR_MIN;
							else if(STABModeControl.stab_error>BASE_STAB_ERROR)STABModeControl.stab_error = BASE_STAB_ERROR;
	
							STABMode_getPeakPrec(cleft, cright, max);
	
							STABModeControl.stab_error_cur = fabs(1.0-STABModeControl.currentPeakPos/(float)STABModeControl.stabilPeakCenter);
	
							if(STABModeControl.peakFactor>=STABModeControl.peakThreshold || STABModeControl.cnt_max>STABIL_CNT)
								return 1;
						}
					}
////////////			
				}
			}
		}
		prev = mean;
	}
	return 0;
}

//��������� ������� ��������� ���� � ����� ���� ����
void STABMode_getPeakPrec(short cleft, short cright, float max)
{
	//���� ���������� ����������������� ����� ������ ���������
	//�� ������ � ���� �������� �������
	float fmean2, fmean, perc;
	fmean2 = fmean = 0;
	//���������� ����� ���� �� ������� � ��������� ��������
	while(cleft<=cright && (perc = (float)identifyControl.BufSpec[cleft])>0)
	{
		fmean2 += perc;
		fmean += perc*(float)cleft;
		cleft++;
	}
	if(fmean2!=0)
	{
		fmean/=fmean2;
		STABModeControl.currentPeakPos=fmean;
	}
}

