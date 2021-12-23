//sound.h

#ifndef _SOUND_H
#define _SOUND_H

#include <iolpc2388.h>
#include "types.h"


//1 ms
#define SOUND_TIMER_VAL 10


#define ALARM_INTERVAL 300


#define SOUND_NUM 6

enum _ENU_SOUNDS
{
	SND_TURNON,
	SND_TURNOFF,
	SND_POWERDOWN,
	SND_LOWBATTERY,
	SND_CLICK,
	SND_OK,
};


enum _ENU_SND_STATE
{
	SNDST_VIBRO=0x0,
	SNDST_SOUND=0x1,
	SNDST_SILENT=0x2,
};

extern const UINT sample_addr[SOUND_NUM+1];


//#define DIR_SND FIO4DIR_bit.P4_4


//shut down of audio amplifier
#define DIR_SND_AU FIO4DIR_bit.P4_13
#define SET_SND_AU MY_FIO4SET(B_13)
#define CLR_SND_AU MY_FIO4CLR(B_13)
#define PIN_SND_AU FIO4PIN_bit.P4_13




//INT input from ISD4004
#define DIR_ISD_INT FIO0DIR_bit.P0_31
#define SET_ISD_INT MY_FIO0SET(B_31)
#define CLR_ISD_INT MY_FIO0CLR(B_31)
#define PIN_ISD_INT FIO0PIN_bit.P0_31


//ISD4004
/*#define DIR_ISD_RAC FIO0DIR_bit.P0_31
#define SET_ISD_RAC MY_FIO0SET(B_31)
#define CLR_ISD_RAC MY_FIO0CLR(B_31)
#define PIN_ISD_RAC FIO0PIN_bit.P0_31*/

//motor control
#define DIR_M_ON FIO4DIR_bit.P4_14
#define SET_M_ON MY_FIO4SET(B_14)
#define CLR_M_ON MY_FIO4CLR(B_14)
#define PIN_M_ON FIO4PIN_bit.P4_14

//beep control
/*#define DIR_BEEP FIO4DIR_bit.P4_9
#define SET_BEEP MY_FIO4SET(B_9)
#define CLR_BEEP MY_FIO4CLR(B_9)
#define PIN_BEEP FIO4PIN_bit.P4_9*/





/*#define DIR_CS_R FIO1DIR_bit.P1_31
#define SET_CS_R MY_FIO1SET(B_31)//IO1SET_bit.P1_31
#define CLR_CS_R MY_FIO1CLR(B_31)//IO1CLR_bit.P1_31
*/



/*
Это RIFF файл фирмы Microsoft. Он используется в Windows. Поэтому весьма "популярен". Он похож на AIFF - формат Apple, который используется для хранения высококачественного звука инструментов, он так же используется на SGI. Он похож, но не совместим.

Вначале идет заголовок RIFF файла:

typedef struct {
	char id[4]; //- идентификатор файла = "RIFF" = 0x46464952
	long len;   //- длина файла без этого заголовка
} IDRiff;


Сам WAV-файл может состоять из нескольких кусков, а эти куски, могут содержать по несколько выборок, или какую-то другую информацию. Но чаще всего, пока - всегда, часть одна и выборка одна.

Заголовок куска WAV:

typedef struct {
	char id[4];  //- идентификатор = "WAVE" = 0x45564157
	char fmt[4]; //- идентификатор = "fmt " = 0x20746D66
	long len;    //- длина этого куска WAV - файла,
} IDChuckWave;


За ним не посредственно кусок WAV:

typedef struct {
        int type;//   - тип звуковых данных, бывает - !!!
//			1 - просто выборка;
	//		0x101 - IBM mu-law;
      //                  0x102 - IBM a-law;
        //                0x103 - ADPCM.
        int channels; //- число каналов 1/2 - !!!
        long SamplesPerSec; //- частота выборки - !!!
        long AvgBytesPerSec; //- частота выдачи байтов
        int align; //- выравнивание
        int bits; //- число бит на выборку  - !!!
} IDWave;


Помеченные - особо необходимы. Далее идентификатор выборки:

 typedef struct {
	char id[4]; - идентификатор ="data" =0x61746164
        long len;   - длина выборки ( кратно 2 )
} IDSampleWave;
Выборок в куске может быть несколько.
*/

/*
#pragma pack(1)

typedef const struct tagWAV{
	char id_riff[4];
    long len_riff;

	char id_chuck[4];
	char fmt[4];
	long len_chuck;

	short  type;
	short  channels;
	long freq;
	long bytes;
	short align;
	short bits;

	char id_data[4];
	long len_data;
} TitleWave;


#pragma pack()
*/


void SoundControl_laud_up(void);
void SoundControl_laud_down(void);




//FIRST OCTAVE in Hz
#define SOUND_FREQ_1_DO  262
#define SOUND_FREQ_1_DO_DIEZ  277
#define SOUND_FREQ_1_RE  294
#define SOUND_FREQ_1_RE_DIEZ  311
#define SOUND_FREQ_1_MI  330
#define SOUND_FREQ_1_FA  349
#define SOUND_FREQ_1_FA_DIEZ  370
#define SOUND_FREQ_1_SOL  392
#define SOUND_FREQ_1_SOL_DIEZ  415
#define SOUND_FREQ_1_LA  440
#define SOUND_FREQ_1_LA_DIEZ  466
#define SOUND_FREQ_1_SI  494

#define SOUND_TONES 7


const extern WORD beepSeq_NOK[];

extern const WORD beepSeq_OK[];
extern const WORD beepSeq_ON[];
extern const WORD beepSeq_OFF[];
extern const WORD beepSeq_LOWBAT[];
const extern WORD beepSeq_WARN[];



struct tagSoundData
{
	//myte mask used to retreive bit from byte array
//	BYTE sample_byte_mask;
	//byte array of bit wav
//	const BYTE* volatile pSample_buffer;
	//len of wav in bits
//	volatile ULONG sample_len;
	//freq of sampling
//	volatile ULONG sample_freq;
	//=1 will sound, =0 vibro
	volatile enum _ENU_SND_STATE bSound;
	volatile ULONG timerVal;	//addon for timer for preset freq
	volatile BYTE flg;
	volatile WORD addr;
	volatile BOOL bPlayClick;	//if 1 then play one click
	volatile BOOL bSounding;
	volatile const WORD* pBeepSeq;	//array of words, first word length, second freq and so on until 0, if NULL then no playing
};

extern struct tagSoundData soundControl;






//extern const ULONG noty[3][12];

__arm void _INT_SoundTimer1(void);
//__arm void _INT_SoundControlTimer2(void);
void SoundControl_Init(void);
//void SoundControl_soundNote(int octave, int note);
//void SoundControl_sound(ULONG timerVal);






//void SoundControl_soundkHz(ULONG Hz);

//void SoundControl_playSampleEx(void);
//extern ULONG sample_len;
//BOOL SoundControl_findSampleDataStart(const TitleWave* pSample);

//void SoundControl_playSample(int i, BOOL bWaitFor);


void SoundControl_turnOFF(void);
void SoundControl_turnON(void);



void SoundControl_timerOFF(void);
void SoundControl_timerInit(void);
void SoundControl_showSoundVibro(int x);
void SoundControl_StopVibro(void);
void SoundControl_StartVibro(void);
void SoundControl_PlayVibro(DWORD ms);


//void sound_playSample(UINT orderPos);

void SoundControl_Beep(DWORD ms, DWORD freq);
void SoundControl_Alarm_intcall(DWORD ms, DWORD freq);

//void SoundControl_StartBeep(void);

void SoundControl_StopBeep(void);


void SoundControl_PWMset_1chena5(unsigned int t_PWM);
void  SoundControl_PWMstop(void);
void  SoundControl_PWMstart(void);
void SoundControl_PWM_Init(void);

//void sound_ISD4004_PlayCur(void);

void SoundControl_setbeep(DWORD freq);
void SoundControl_BeepSeq(const WORD* pBeepSeq);
//play sequence of beeps
void SouncControl_PlaySequence(void);

#endif	//#ifndef _SOUND_H
