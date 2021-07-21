//packspectrum.c
//pack spectrum algorithm


#include <string.h>
#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <math.h>

#include "types.h"
#include "packspectrum.h"
#include "spectrum.h"
#include "powercontrol.h"
#include "identify.h"


struct tagPackSpectrum packspec;


void packspectrum_init(void)
{
	packspec.packed = (BYTE*)&identifyControl.BufSpec[0];
	packspec.unpacked = (BYTE*)&identifyControl.SDs[0];
}



BOOL UnpackSuperCompressSpectrum(DWORD szbin, DWORD* pszretbin)
{
	//super compress, remove zeros
	DWORD newsz=0;
	BYTE val;
	for(int i=0;i<szbin;i++)
	{
		if(newsz>=sizeof(identifyControl.SDs))return FALSE;
		val = packspec.packed[i];
		/*
		0x81,x - next x bytes packed as zero values//: 0-zero, 1-END of SEQ
		0x81,0x00 - 0x81
		0x82 - next bytes packed as 2 bit values: 0-zero, 1-one, 3-minus one, 2-END of SEQ
		0x82,0x80 - 0x82
		0x84 - next bytes packed as 4 bit values: 0-zero, 1-one, 2-two, 3-three, 4-four, 5-five, 6-six, 7-seven, 15--1,14--2,13--3,12--4,11--5,10--6,9--7,8-END of SEQ
		0x84,0x80 - 0x84
		*/
		if(i==szbin-1)
		{//last byte
			packspec.unpacked[newsz++] = val;
		}else
		{
			i++;
			switch(val)
			{
			case 0x81:
				if(packspec.packed[i]==0x00)
				{
					packspec.unpacked[newsz++] = val;
				}else
				{
					val = packspec.packed[i];
					if(newsz+val>sizeof(identifyControl.SDs))return FALSE;
					for(int j=0;j<val;j++)
						packspec.unpacked[newsz++] = 0;
				}
				break;
			case 0x82:
				if(packspec.packed[i]==0x80)
				{
					packspec.unpacked[newsz++] = val;
				}else
				{
					BYTE bits=0;
					int nBits = 0;
					val = packspec.packed[i];
					do
					{
						if(nBits==8)
						{
							val = packspec.packed[++i];
							nBits=0;
						}
						bits = (val>>6)&0x03;
						val<<=2;
						nBits+=2;
						switch(bits)
						{
						case 0x00:
							packspec.unpacked[newsz++] = 0;
							break;
						case 0x01:
							packspec.unpacked[newsz++] = 1;
							break;
						case 0x03:
							packspec.unpacked[newsz++] = 0xff;
							break;
						default:;
						}
						if(newsz>=sizeof(identifyControl.SDs))return FALSE;
					}while(bits!=0x02);
				}
				break;
			case 0x84:
				if(packspec.packed[i]==0x80)
				{
					packspec.unpacked[newsz++] = val;
				}else
				{
					BYTE bits=0;
					int nBits = 0;
					val = packspec.packed[i];
					do
					{
						if(nBits==8)
						{
							val = packspec.packed[++i];
							nBits=0;
						}
						bits = (val>>4)&0x0f;
						val<<=4;
						nBits+=4;
						switch(bits)
						{
						case 0x00:
							packspec.unpacked[newsz++] = 0;
							break;
						case 0x01:
							packspec.unpacked[newsz++] = 1;
							break;
						case 0x02:
							packspec.unpacked[newsz++] = 2;
							break;
						case 0x03:
							packspec.unpacked[newsz++] = 3;
							break;
						case 0x04:
							packspec.unpacked[newsz++] = 4;
							break;
						case 0x05:
							packspec.unpacked[newsz++] = 5;
							break;
						case 0x06:
							packspec.unpacked[newsz++] = 6;
							break;
						case 0x07:
							packspec.unpacked[newsz++] = 7;
							break;
						case 0x0f:
							packspec.unpacked[newsz++] = 0xff;
							break;
						case 0x0e:
							packspec.unpacked[newsz++] = 0xfe;
							break;
						case 0x0d:
							packspec.unpacked[newsz++] = 0xfd;
							break;
						case 0x0c:
							packspec.unpacked[newsz++] = 0xfc;
							break;
						case 0x0b:
							packspec.unpacked[newsz++] = 0xfb;
							break;
						case 0x0a:
							packspec.unpacked[newsz++] = 0xfa;
							break;
						case 0x09:
							packspec.unpacked[newsz++] = 0xf9;
							break;
						default:;
						}
						if(newsz>=sizeof(identifyControl.SDs))return FALSE;
					}while(bits!=0x08);
				}
				break;
			default:
				i--;
				packspec.unpacked[newsz++] = val;
				;
			}
		}
	}

	*pszretbin = newsz;
	return TRUE;
}


//restore all periodical 0x80 with period 3
BOOL Restore0x80(DWORD* pszbin)
{
	DWORD newsz=0;
	memset((void*)&packspec.maskdata[0], 0, sizeof(packspec.maskdata));

	BYTE val;
	//восстанавливаем периодические 0x80 в данных
	for(int i=0;i<*pszbin;i++)
	{
		val = packspec.unpacked[i];
		//	0x80,x - next x bytes as 0x80 with period 3 bytes
		if(val==0x80 && packspec.unpacked[++i]!=0)
		{
			//number of bytes of 0x80
			while (packspec.maskdata[newsz] != 0)newsz++;
			int cnt = packspec.unpacked[i];
			for(int k=0;k<cnt;k++)
			{
				if(newsz+k*3>=sizeof(identifyControl.BufSpec))return FALSE;
				packspec.maskdata[newsz+k*3] = 0x8000;
				packspec.packed[newsz+k*3] = 0x80;
			}
			newsz++;
		}else
		{
			while(packspec.maskdata[newsz]&0x8000)newsz++;
			if(newsz>=sizeof(identifyControl.BufSpec))return FALSE;
			packspec.maskdata[newsz] = 0x8000;
			packspec.packed[newsz++] = val;
		}
	}
	memcpy(packspec.unpacked, packspec.packed, newsz);
	*pszbin=newsz;

	return TRUE;
}



//remove all periodical 0x80 with period 3
void remove0x80(DWORD* pszbin)
{

	DWORD newsz=0;
//	for(int i=0;i<sizeof(packspec.maskdata);i++)
	//	packspec.maskdata[i]=0;
	memset((void*)&packspec.maskdata[0], 0, sizeof(packspec.maskdata));
	BYTE val;
	//убираем периодические 0x80 в данных
	for(int i=0;i<*pszbin;i++)
	{
		if(packspec.maskdata[i]&0x8000)continue;
		val = packspec.unpacked[i];
		/*
		0x80,x - next x bytes as 0x80 with period 3 bytes
		0x80,0 - 0x80
		*/
		if(val==0x80)
		{
			//number of bytes of 0x80
			int cnt0=countSeq0x80(packspec.unpacked, *pszbin, i);
			//memory usage
			const int m0 = 3;
			if(cnt0>=m0)
			{//m1 wins
				packspec.packed[newsz++] = 0x80;
				packspec.packed[newsz++] = cnt0;
				//mark as occupaed
				for(int j=0;j<cnt0;j++)
					packspec.maskdata[i+j*3] = 0x8000;
			}else
			{
				packspec.packed[newsz++] = val;
				packspec.packed[newsz++] = 0x00;
				packspec.maskdata[i] = 0x8000;
			}
		}else
		{
			packspec.packed[newsz++] = val;
			packspec.maskdata[i] = 0x8000;
		}
	}
	memcpy(packspec.unpacked, packspec.packed, newsz);
	*pszbin=newsz;
}



//count number of bytes as 0x80 and period
int  countSeq0x80(BYTE* pbindata, DWORD sz, int index)
{
	int cnt=1;
	BYTE val = pbindata[index];
	//period must be 3
	for(int i=index+3;i<sz;i+=3)
	{
		if(val!=pbindata[i])break;
		if(packspec.maskdata[i]&0x8000)break;
		cnt++;
		if(cnt==255)break;
	}
	return cnt;
}


//compress soectrum in buffer to bin data
BOOL superCompressSpectrum(DWORD* pszbin)
{
	remove0x80(pszbin);
	//super compress, remove zeros
	DWORD newsz=0;
	for(int i=0;i<sizeof(packspec.maskdata)/sizeof(packspec.maskdata[0]);i++)
	{
		packspec.maskdata[i] = 0x7fff;	//по маске: 0x8000 признак занятости, 0x7fff - индекс
	}
	BYTE val;
	int cnt1;
	const float m1 = 2.0;	//number of bytes to pack in

	for(int i=0;i<*pszbin;i++)
	{
		if(packspec.maskdata[i]&0x8000)continue;
		val = packspec.unpacked[i];
		/*
		0x81,x - next x bytes packed as zero values
		0x81,0x00 - 0x81
		0x82 - next bytes packed as 2 bit values: 0-zero, 1-one, 3-minus one, 2-END of SEQ
		0x82,0x80 - 0x82
		0x84 - next bytes packed as 4 bit values: 0-zero, 1-one, 2-two, 3-three, 4-four, 5-five, 6-six, 7-seven, 15--1,14--2,13--3,12--4,11--5,10--6,9--7,8-END of SEQ
		0x84,0x80 - 0x84
		*/
		switch(val)
		{
		case 0x81:
			packspec.maskdata[newsz] = (WORD)i | (packspec.maskdata[newsz]&0x8000);
			packspec.packed[newsz++] = val;
			packspec.maskdata[newsz] = (WORD)i | (packspec.maskdata[newsz]&0x8000);
			packspec.packed[newsz++] = 0;
			packspec.maskdata[i] |= 0x8000;
			break;
		case 0x82:
		case 0x84:
			packspec.maskdata[newsz] = (WORD)i | (packspec.maskdata[newsz]&0x8000);
			packspec.packed[newsz++] = val;
			packspec.maskdata[newsz] = (WORD)i | (packspec.maskdata[newsz]&0x8000);
			packspec.packed[newsz++] = 0x80;
			packspec.maskdata[i] |= 0x8000;
			break;
		default:
			//number of bytes of zeros
			cnt1=countSeq1(*pszbin, i);
			//memory usage
			if((float)cnt1/m1>=8.0)
			{//m1 wins
				packspec.maskdata[newsz] = (WORD)i | (packspec.maskdata[newsz]&0x8000);
				packspec.packed[newsz++] = 0x81;
				packspec.maskdata[newsz] = (WORD)i | (packspec.maskdata[newsz]&0x8000);
				packspec.packed[newsz++] = cnt1;
				//mark as occupaed
				for(int v=0;v<cnt1;v++)
					packspec.maskdata[i+v] |= 0x8000;
				i+=cnt1-1;
			}
		}
	}

	PowerControl_kickWatchDog();

	for(int i=0;i<*pszbin;i++)
	{
		if(packspec.maskdata[i]&0x8000)continue;
		val = packspec.unpacked[i];
		/*
		0x81,x - next x bytes packed as zero values
		0x81,0x00 - 0x81
		0x82 - next bytes packed as 2 bit values: 0-zero, 1-one, 3-minus one, 2-END of SEQ
		0x82,0x80 - 0x82
		0x84 - next bytes packed as 4 bit values: 0-zero, 1-one, 2-two, 3-three, 4-four, 5-five, 6-six, 7-seven, 15--1,14--2,13--3,12--4,11--5,10--6,9--7,8-END of SEQ
		0x84,0x80 - 0x84
		*/
		//number of bytes can be packed in 2 bits
		int cnt2=countSeq2(*pszbin, i);
		//memory usage
		int m2 = (cnt2/4+(cnt2%4?1:0))+1;
		float c2 = (float)cnt2/(float)m2;
		if(c2>=2.7)
		{//m2 wins
			packspec.maskdata[newsz] = (WORD)i | (packspec.maskdata[newsz]&0x8000);
			packspec.packed[newsz++] = 0x82;
			int newsz2=packSeq2(*pszbin, i , cnt2, newsz);
			for(int j=0;j<newsz2-newsz;j++)
				packspec.maskdata[newsz+j] = (WORD)(i+j) | (packspec.maskdata[newsz+j]&0x8000);
			newsz = newsz2;
			//mark as occupaed
			for(int v=0;v<cnt2;v++)
				packspec.maskdata[i+v] |= 0x8000;
			i+=cnt2-1;
		}
	}

	PowerControl_kickWatchDog();

	for(int i=0;i<*pszbin;i++)
	{
		if(packspec.maskdata[i]&0x8000)continue;
		val = packspec.unpacked[i];
		/*
		0x81,x - next x bytes packed as zero values
		0x81,0x00 - 0x81
		0x82 - next bytes packed as 2 bit values: 0-zero, 1-one, 3-minus one, 2-END of SEQ
		0x82,0x80 - 0x82
		0x84 - next bytes packed as 4 bit values: 0-zero, 1-one, 2-two, 3-three, 4-four, 5-five, 6-six, 7-seven, 15--1,14--2,13--3,12--4,11--5,10--6,9--7,8-END of SEQ
		0x84,0x80 - 0x84
		*/
		//number of bytes can be packed in 4 bits
		int cnt4=countSeq4(*pszbin, i);
		//memory usage
		int m4 = (cnt4/2+(cnt4%2?1:0))+1;
		float c4 = (float)cnt4/(float)m4;
		if(c4>1)
		{//m4 wins
			packspec.maskdata[newsz] = (WORD)i | (packspec.maskdata[newsz]&0x8000);
			packspec.packed[newsz++] = 0x84;
			int newsz2=packSeq4(*pszbin, i , cnt4, newsz);
			for(int j=0;j<newsz2-newsz;j++)
				packspec.maskdata[newsz+j] = (WORD)(i+j) | (packspec.maskdata[newsz+j]&0x8000);
			newsz = newsz2;
			//mark as occupaed
			for(int v=0;v<cnt4;v++)
				packspec.maskdata[i+v] |= 0x8000;
			i+=cnt4-1;
		}else
		{//not distributed
			packspec.maskdata[newsz] = (WORD)i | (packspec.maskdata[newsz]&0x8000);
			packspec.packed[newsz++] = val;
		}
	}

	PowerControl_kickWatchDog();

	if(newsz>sizeof(identifyControl.SDs))
	{
		return FALSE;
	}

	int j=0;
	for(short m=0;m<*pszbin;m++)
	{
		for(int n=0;n<newsz;n++)
		{
			if((packspec.maskdata[n]&0x7fff)==m)
			{
				packspec.unpacked[j++] = packspec.packed[n];
				if(n<newsz-1 && (packspec.maskdata[n+1]&0x7fff)==m)
				{
					packspec.unpacked[j++] = packspec.packed[n+1];
				}
				break;
			}
		}
	}

	*pszbin=newsz;
	return TRUE;
}





//count number of bytes as zeros
int  countSeq1(DWORD sz, int index)
{
	int cnt=0;
	for(int i=index;i<sz;i++)
	{
		if(packspec.maskdata[i]&0x8000)break;
		if(packspec.unpacked[i]!=0)break;
		cnt++;
		if(cnt==255)break;
	}
	return cnt;
}

//count number of bytes as 2 bit value
int  countSeq2(DWORD sz, int index)
{
	int cnt=0;
	BYTE val;
	for(int i=index;i<sz;i++)
	{
		if(packspec.maskdata[i]&0x8000)break;
		val=packspec.unpacked[i];
		if(val>1 && val<0xff)
			break;
		cnt++;
	}
	return cnt;
}

//pack number of bytes that can be packed in 2 bits
int packSeq2(DWORD sz, int indexBD, int cnt, int indexNBD)
{
	BYTE val;
	int bits=0;
	BYTE nval=0x0;
	while(cnt--)
	{
		val=packspec.unpacked[indexBD++];
		bits++;
		nval=nval<<2;
		nval=nval | (val&3);
		if(bits==4)
		{
			packspec.packed[indexNBD++]=nval;
			nval=0x0;
			bits=0;
		}
	}
	if(bits==0)
		nval = 0x80;
	else if(bits==3)
	{
		nval=nval<<2;
		nval |= 0x02;
	}else if(bits==2)
	{
		nval=nval<<2;
		nval=nval<<2;
		nval |= 0x08;
	}else if(bits==1)
	{
		nval=nval<<2;
		nval=nval<<2;
		nval=nval<<2;
		nval |= 0x20;
	}
	packspec.packed[indexNBD++]=nval;
	return indexNBD;
}



//count number of bytes as 4 bit value
int  countSeq4(DWORD sz, int index)
{
	int cnt=0;
	BYTE val;
	for(int i=index;i<sz;i++)
	{
		if(packspec.maskdata[i]&0x8000)break;
		val=packspec.unpacked[i];
		if(val>7 && val<0xf9)
			break;
		cnt++;
	}
	return cnt;
}



//pack number of bytes that can be packed in 4 bits
int packSeq4(DWORD sz, int indexBD, int cnt, int indexNBD)
{
	BYTE val;
	int bits=0;
	BYTE nval=0x0;
	while(cnt--)
	{
		val=packspec.unpacked[indexBD++];
		bits++;
		nval=nval<<4;
		nval=nval | (val&0xf);
		if(bits==2)
		{
			packspec.packed[indexNBD++]=nval;
			nval=0x0;
			bits=0;
		}
	}
	if(bits==0)
		nval = 0x80;
	else if(bits==1)
	{
		nval=nval<<4;
		nval |= 0x08;
	}
	packspec.packed[indexNBD++]=nval;
	return indexNBD;
}

