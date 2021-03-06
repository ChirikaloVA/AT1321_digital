//sound.c


//#include <stdio.h>
//#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>

#include "sound.h"
#include "syncObj.h"
#include "powerControl.h"
#include "display.h"
#include "interProc.h"
#include "interrupts.h"
#include "SPRD_mode.h"



//?????? ?????? ??????? ? ????????? ???????? ??? ???? ?????????
const UINT sample_addr[SOUND_NUM+1]=
{
1,
20,
40,
60,
80,
100,
120
};



const WORD beepSeq_NOK[]={500, SOUND_FREQ_1_DO, 500, 1/*quite*/, 0};



//const WORD beepSeq_NOK[]={500, SOUND_FREQ_1_RE, 500, SOUND_FREQ_1_DO, 0};

const WORD beepSeq_WARN[]={250,3*SOUND_FREQ_1_MI,250,3*SOUND_FREQ_1_DO,250,3*SOUND_FREQ_1_MI,250,3*SOUND_FREQ_1_DO,0};



const WORD beepSeq_OK[]={125,SOUND_FREQ_1_DO,250,SOUND_FREQ_1_MI,0};


//const WORD beepSeq_OK[]={125,3*SOUND_FREQ_1_DO,250,3*SOUND_FREQ_1_MI,0};





//D#(rediez) G#(soldiez) A#(LAdiez) C(DO)
//const WORD beepSeq_ON[]={
//125,1000,
//125,1100,
//0
//};
const WORD beepSeq_ON[]={
125,2*SOUND_FREQ_1_FA,
125,4*SOUND_FREQ_1_FA,
0
};
const WORD beepSeq_OFF[]={
125,4*SOUND_FREQ_1_FA,
125,2*SOUND_FREQ_1_FA,
0
};



/*
const WORD beepSeq_ON[]={
125,3*SOUND_FREQ_1_DO,
125,3*SOUND_FREQ_1_MI,
250,3*SOUND_FREQ_1_LA,0
};
const WORD beepSeq_OFF[]={125,3*SOUND_FREQ_1_MI,125,3*SOUND_FREQ_1_RE,250,3*SOUND_FREQ_1_DO,0};
*/
const WORD beepSeq_LOWBAT[]={125,SOUND_FREQ_1_DO*3,125,SOUND_FREQ_1_MI*3,250,SOUND_FREQ_1_DO*3,0};


struct tagSoundData soundControl;






void SoundControl_Init(void)
{
	
	soundControl.bSounding = FALSE;
	soundControl.flg = 0;
	soundControl.addr = 0;
  //========adjust sound initial values
//	DIR_SND = 1;	//pin for output


//	PINMODE8_bit.P4_4 = 0x2;	//neithrt pull up and down mode
	
	SoundControl_timerInit();

//	T1TCR_bit.CE = 0;	//do not enable timer
  //======================
	
	
	//laud control
//	DIR_CS_R = 1;   //????? ?????????????
	
	//vibra adjust
	DIR_M_ON = 1;
	SET_M_ON;

//	sound_ISD4004_Init();	
	SoundControl_PWM_Init();
	
	soundControl.bSound = SNDST_SOUND;
	
	
	//beeper
	//PINSEL8_bit.P4_9 = 0; //EMC A9
	//DIR_BEEP = 1;
	
	SoundControl_StopBeep();
	
	soundControl.bPlayClick = FALSE;

}













void SoundControl_turnON(void)
{
	SET_SND_AU;
//	sound_ISD4004_PowerUp();
	PowerControl_sleep(25);
}


void SoundControl_turnOFF(void)
{
//	sound_ISD4004_StopPwdn();
	CLR_SND_AU;
	SoundControl_timerOFF();
}



//int for freq generation
__arm void _INT_SoundTimer1(void)
{
	__ir_bits stru = {0};
	if(T1IR_bit.MR1INT)
	{//stop sound timer
		//process search alarm
/*		if(SPRDModeControl.iAlarmTimer>0)
		{
			--SPRDModeControl.iAlarmTimer;
			T1MR1 = T1TC+ALARM_INTERVAL;
		}else
			T1MCR_bit.MR1I = 0;*/
		
		
		//clear TIMER1 interrupt status
		stru.MR1INT = 1;
	}
	if(T1IR_bit.MR0INT)
	{//sounding

		//??????? ??????
//		PWM1TCR_bit.PWMEN = !PWM1TCR_bit.PWMEN;
//		SoundControl_PWMstop();
		T1MR0 = T1TC+soundControl.timerVal;
		stru.MR0INT = 1;
	}
	if(T1IR_bit.MR2INT)
	{
		//clear TIMER1 interrupt status
		stru.MR2INT = 1;
	}
	if(T1IR_bit.MR3INT)
	{
		//clear TIMER1 interrupt status
		stru.MR3INT = 1;
	}
	T1IR = *(ULONG*)&stru;
}





void SoundControl_timerOFF(void)
{
	T1MCR_bit.MR0I = 0;
}

void SoundControl_timerInit(void)
{
	PCONP_bit.PCTIM1 = 1;	//?????? ???????
	
	T1TCR_bit.CE = 0;
	__mcr_bits t1mcr;
	t1mcr.MR0I = 0; //enable interrupt for sounding
	t1mcr.MR0R = 0; //reset counter after reach comparator
	t1mcr.MR0S = 0; //dont stop timer after reach value
	t1mcr.MR1I = 0; //enable interrupt for sounding
	t1mcr.MR1R = 0; //reset counter after reach comparator
	t1mcr.MR1S = 0; //dont stop timer after reach value
	t1mcr.MR2I = 0; //enable interrupt for sounding
	t1mcr.MR2R = 0; //reset counter after reach comparator
	t1mcr.MR2S = 0; //dont stop timer after reach value
	t1mcr.MR3I = 0; //enable interrupt for sounding
	t1mcr.MR3R = 0; //reset counter after reach comparator
	t1mcr.MR3S = 0; //dont stop timer after reach value
	T1MCR_bit = t1mcr;
	T1TCR_bit.CR = 1;	//reset timer
	T1TCR_bit.CR = 0;

	T1PR = HW_FREQ/1000*SOUND_TIMER_VAL-1;
	T1PC = 0;

	T1TCR_bit.CE = 1;
	
}










void SoundControl_BeepSeq(const WORD* pBeepSeq /*array of words, first word length, second freq and so on until 0*/
						  )
{
	soundControl.pBeepSeq = pBeepSeq;
	SouncControl_PlaySequence();
}

//play sequence of beeps
void SouncControl_PlaySequence(void)
{
	if(soundControl.bSound!=SNDST_SOUND || powerControl.bBatteryAlarm //sound only if battery charged
	   && (soundControl.pBeepSeq<beepSeq_LOWBAT || soundControl.pBeepSeq>=(beepSeq_LOWBAT+sizeof(beepSeq_LOWBAT))))//or only low battery sound
		soundControl.pBeepSeq = NULL;
	//check to play sequence
	if(soundControl.pBeepSeq)
	{
		WORD len = *soundControl.pBeepSeq++;
		if(len>0)
		{
                  
                    {
                      WORD freq = *soundControl.pBeepSeq++;
                      SoundControl_Beep(len, freq);
                    }
                   
                  
//			WORD freq = *soundControl.pBeepSeq++;
//			SoundControl_Beep(len, freq);
		}else
			soundControl.pBeepSeq = NULL;
	}
	if(soundControl.pBeepSeq==NULL)
        {
		SoundControl_StopBeep();
                soundControl.bSounding = FALSE;
                
        }
}


//alarm by sounding or vibration
void SoundControl_Alarm_intcall(DWORD ms, DWORD freq)
{
//  unsigned int idx;
  Display_BlinkREDLED(ms);
  
  //27/08/2012
  if(powerControl.bBatteryAlarm)
  {
//    SET_ISD_INT;
    if(soundControl.bSounding == FALSE)
    {
      ms = 30;
      SoundControl_PlayVibro(ms);
    }
//    CLR_ISD_INT;
    return;//low battery no sound
  }
  
  if(soundControl.bSound==SNDST_SOUND)
  {
    
    SoundControl_Beep(ms, freq);
    
//    if( powerControl.batV_snd <= 2.0)
//    {
////      if(ms<20)
//        ms = 20;
//      SoundControl_PlayVibro(ms);
//    }
//    powerControl.bControlBatSnd = TRUE;
//    powerControl.bControlBatSndRdy = FALSE;
//    PowerControl_startADC_intcall();
//    
//    for(idx = 0; idx < 10; ++idx)
//    {
////      SET_ISD_INT;
//      pause(70);
////      CLR_ISD_INT;
//      if(powerControl.bControlBat)
//      {//battery control 
//        powerControl.bControlBat = FALSE;
//        if( powerControl.batV > 2.0)
//        {
//          if(powerControl.batV > 2.2)
//          {
//            SoundControl_Beep(ms, freq);
//          }
//          else
//          {
//            SoundControl_Beep(30, 700);
//          }
//        }
//        else
//        {
//          if(ms<100)
//            ms = 100;
//          SoundControl_PlayVibro(ms);
//        }
//        break;
//      }
//    }
   
   
  }else if(soundControl.bSound==SNDST_VIBRO)
  {
//    if(ms<30)
      ms = 30;
    SoundControl_PlayVibro(ms);
  }
}

//ms min is 80
void SoundControl_Beep(DWORD ms, DWORD freq)
{
  //	if(T2MCR_bit.MR2I)return;	//already playing
  if(ms<INTERPROC_TIMER_VAL)
  {
    exception(__FILE__,__FUNCTION__,__LINE__,"beep delay is out of range");
  }
  ms/=INTERPROC_TIMER_VAL;
  powerControl.bControlBatSnd = TRUE;
  powerControl.bControlBatSndRdy = FALSE;
  PowerControl_startADC_intcall();              //???. ??? ??? ?????. ????????
  
  SoundControl_setbeep(freq);
  
  //????: 07/03/2012
  //????????? ?????? 3.9: ????????? ???? ??? ??????? (T1TCR_bit.CE = 0; T1TCR_bit.CE = 1;)
 
  T2TCR_bit.CE = 0;
  T2MR2 = T2TC+ms;
  T2MCR_bit.MR2I = 1; //enable interrupt for vibra
  T2TCR_bit.CE = 1;
  
}

//start beep on frequensy
void SoundControl_setbeep(DWORD freq)
{
  unsigned int idx;
  SoundControl_PWMstop();
  for(idx = 0; idx < 10; ++idx)
  {
    //      SET_ISD_INT;
    pause(100);
    //      CLR_ISD_INT;
    if(powerControl.bControlBatSndRdy)
    {//battery control 
      
      break;
    }
  }
  if( powerControl.batV_snd > 2.0)
  {
    
    PWM1MR0 = HW_FREQ/freq-1;
    SoundControl_PWMset_1chena5(HW_FREQ/2/freq-1);
    SoundControl_PWMstart();
    soundControl.bSounding = TRUE;
  }
  else
  {
    //    SET_ISD_INT;
    pause(10);
    //    CLR_ISD_INT;
  }
  
}



void SoundControl_showSoundVibro(int x)
{
	if(soundControl.bSound==SNDST_SOUND)
		Display_output_bmp(x,0, (BITMAPFILEHEADER*)bmp_sound);
	else if(soundControl.bSound==SNDST_VIBRO)
		Display_output_bmp(x,0, (BITMAPFILEHEADER*)bmp_vibro);
	else
		Display_output_bmp(x,0, (BITMAPFILEHEADER*)bmp_silent);
}


void SoundControl_PlayVibro(DWORD ms)
{
//????: 07/03/2012
//????????? ?????? 3.9: ????????????? ????????? ?????? ??-?? ???????? ? ???????? ????? ????? ?? ?????? ?????? (?????)
//	if(T2MCR_bit.MR2I)return;	//already playing
	if(ms<INTERPROC_TIMER_VAL)
	{
		exception(__FILE__,__FUNCTION__,__LINE__,"vibro delay is out of range");
	}
	ms/=INTERPROC_TIMER_VAL;
	SoundControl_StartVibro();
        soundControl.bSounding = TRUE;
//????: 07/03/2012
//????????? ?????? 3.9: ????????? ???? ??? ??????? (T1TCR_bit.CE = 0; T1TCR_bit.CE = 1;)
	T2TCR_bit.CE = 0;
	T2MR2 = T2TC+ms;
	T2MCR_bit.MR2I = 1; //enable interrupt for vibra
	T2TCR_bit.CE = 1;
}



void SoundControl_StartVibro(void)
{
	CLR_M_ON;
}

void SoundControl_StopVibro(void)
{
	SET_M_ON;
}


/*
void SoundControl_StartBeep(void)
{
	CLR_BEEP;
}
*/

void SoundControl_StopBeep(void)
{
  powerControl.bControlBatSnd = FALSE;
  SoundControl_PWMstop();
}




///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////PWM for sound reasons/////////////////////////////
void SoundControl_PWM_Init(void)
{
	PCONP_bit.PCPWM1 = 1;	//turn on power on pwm

	//===== ???????? PWM ======================================

//	DIR_ISD_INT = 0;

	FIO2DIR_bit.P2_4 = 1;
	PINSEL4_bit.P2_4 = 0x01;  //????? P2.4 ??????? ??????? PWM5 FUL
	
	PWM1PR = 0x0;          //???????????? PWM ?? ????????????

	PWM1PCR_bit.PWMSEL5 = 0;  //??? ??????? ????????? ? ?????? 5 PWM ????.
	
	PWM1PCR_bit.PWMENA5 = 1;      //???????? ????? ? ?????? 5 PWM
	PWM1MCR = 0x0;   //????? ???????? ?? ??????????
	PWM1LER_bit.EM5L = 1;
}


//??????? ????????? ?????? PWM
void  SoundControl_PWMstart(void)
{
	PWM1TCR_bit.CR = 1;          //????? ???????? ? ???????????? PWM
	PWM1TCR_bit.CE = 1;          //?????????? ???????? ? ???????????? PWM
	PWM1TCR_bit.CR = 0;          //?????? ????? ???????? ? ???????????? PWM
}
//??????? ?????????? ?????? PWM
void  SoundControl_PWMstop(void)
{
	PWM1TCR_bit.CR = 1;          //????? ???????? ? ???????????? PWM
	SoundControl_PWMset_1chena5(0);
	PWM1TCR_bit.PWMEN  = 0;      //?????????? ?????? PWM
}


//=================================================
//??????? ??????? ?????????? PWM ? ?????? 4
//t_PWM - ???????? ??????? PWM ? ???????? pclk
//=================================================
void SoundControl_PWMset_1chena5(unsigned int t_PWM)
{
	PWM1TCR_bit.PWMEN  = 1;      //????????? ????? PWM
	PWM1MR5 = t_PWM;          //??????? ??????? PWM
	PWM1LER_bit.EM5L = 1;
}
