//NMEA_Parser.c




#include <stdio.h>
#include <ctype.h>
//#include <inarm.h>
#include <iolpc2388.h>
#include <string.h>

#include "types.h"

#include "NMEA_Parser.h"


struct tagNMEAParserControl NMEAParserControl;



#define MAXFIELD	25		// maximum field length

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void NMEAParser_Init(void)
{
	NMEAParserControl.m_nState = NP_STATE_SOM;
	NMEAParserControl.m_dwCommandCount = 0;
	NMEAParser_Reset();
	NMEAParserControl.commonGPS.Lat = 0;
	NMEAParserControl.commonGPS.Lon = 0;
	NMEAParserControl.commonGPS.Alt = 0;
	NMEAParserControl.commonGPS.Spd = 0;
	NMEAParserControl.commonGPS.Dir = 0;
	NMEAParserControl.commonGPS.Vld = 0;
	NMEAParserControl.commonGPS.Sat = 0;
}


///////////////////////////////////////////////////////////////////////////////
// ParseBuffer:	Parse the supplied buffer for NMEA sentence information. Since
//				the parser is a state machine, partial NMEA sentence data may
//				be supplied where the next time this method is called, the
//				rest of the partial NMEA sentence will complete	the sentence.
//
//				NOTE:
//
// Returned:	TRUE all the time....
///////////////////////////////////////////////////////////////////////////////
void NMEAParser_ParseBuffer(BYTE *pBuff, DWORD dwLen)
{
	NMEAParserControl.m_nState = NP_STATE_SOM;
	NMEAParserControl.m_pCommand[0] = (BYTE)0;
	for(DWORD i = 0; i < dwLen; i++)
	{
		NMEAParser_ProcessNMEA(pBuff[i]);
	}
	NMEAParserControl.commonGPS.Vld = 0;
	if(NMEAParserControl.m_btGSAFixMode>=1)
	{
		double dbl = NMEAParserControl.m_dGGALatitude;
		NMEAParserControl.commonGPS.Lat = dbl;
//		str1G.Format(_T("Lat=%.4f "),dbl);
		dbl = NMEAParserControl.m_dGGALongitude;
		NMEAParserControl.commonGPS.Lon = dbl;
//		str2G.Format(_T("Lon=%.4f "),dbl);

		if(NMEAParserControl.m_btGSAFixMode>=2)
			NMEAParserControl.commonGPS.Vld++;

		if(NMEAParserControl.m_btGSAFixMode==3)
		{
			NMEAParserControl.commonGPS.Vld++;
			dbl = NMEAParserControl.m_dGGAAltitude;
			NMEAParserControl.commonGPS.Alt = (float)dbl;
	//		str3G.Format(_T("Alt=%.4f "),dbl);
		}
		if(NMEAParserControl.m_btRMCDataValid!='V')
		{
			NMEAParserControl.commonGPS.Vld++;
			NMEAParserControl.commonGPS.Vld++;
			dbl = NMEAParserControl.m_dRMCGroundSpeed*1.852;
			NMEAParserControl.commonGPS.Spd = (float)dbl;
	//		str4G.Format(_T("Spd=%.1f "),dbl);
			dbl = NMEAParserControl.m_dRMCCourse;
			NMEAParserControl.commonGPS.Dir = (float)dbl;
	//		str6G.Format(_T("Dir=%.1f "), dbl);
		}
//		str5G.Format(_T("Sat=%d"),(int)NMEAParserControl.m_btGGANumOfSatsInUse);
	}
}

///////////////////////////////////////////////////////////////////////////////
// ProcessNMEA: This method is the main state machine which processes individual
//				bytes from the buffer and parses a NMEA sentence. A typical
//				sentence is constructed as:
//
//					$CMD,DDDD,DDDD,....DD*CS<CR><LF>
//
//				Where:
//						'$'			HEX 24 Start of sentence
//						'CMD'		Address/NMEA command
//						',DDDD'		Zero or more data fields
//						'*CS'		Checksum field
//						<CR><LF>	Hex 0d 0A End of sentence
//
//				When a valid sentence is received, this function sends the
//				NMEA command and data to the ProcessCommand method for
//				individual field parsing.
//
//				NOTE:
//						
///////////////////////////////////////////////////////////////////////////////
void NMEAParser_ProcessNMEA(BYTE btData)
{
	switch(NMEAParserControl.m_nState)
	{
		case NP_STATE_SOM :
			if(btData == '$')
			{
				NMEAParserControl.m_btChecksum = 0;			// reset checksum
				NMEAParserControl.m_wIndex = 0;				// reset index
				NMEAParserControl.m_nState = NP_STATE_CMD;
			}
		break;

		case NP_STATE_CMD :
			if(btData != ',' && btData != '*')
			{
				// Check for command overflow
				if(NMEAParserControl.m_wIndex >= (WORD)NP_MAX_CMD_LEN)
				{
					NMEAParserControl.m_nState = NP_STATE_SOM;
				}else
				{
					NMEAParserControl.m_pCommand[NMEAParserControl.m_wIndex++] = btData;
					NMEAParserControl.m_btChecksum ^= btData;

					// Check for command overflow
					if(NMEAParserControl.m_wIndex >= (WORD)NP_MAX_CMD_LEN)
					{
						NMEAParserControl.m_nState = NP_STATE_SOM;
					}
				}
			}
			else
			{
				// Check for command overflow
				if(NMEAParserControl.m_wIndex >= (WORD)NP_MAX_CMD_LEN)
				{
					NMEAParserControl.m_nState = NP_STATE_SOM;
				}else
				{
					NMEAParserControl.m_pCommand[NMEAParserControl.m_wIndex] = '\0';	// terminate command
					NMEAParserControl.m_btChecksum ^= btData;
					NMEAParserControl.m_wIndex = 0;
					NMEAParserControl.m_nState = NP_STATE_DATA;		// goto get data state
				}
			}
		break;

		case NP_STATE_DATA :
			if(btData == '*') // checksum flag?
			{
				if(NMEAParserControl.m_wIndex >= (WORD)NP_MAX_DATA_LEN) // Check for buffer overflow
				{
					NMEAParserControl.m_nState = NP_STATE_SOM;
				}else
				{
					NMEAParserControl.m_pData[NMEAParserControl.m_wIndex] = '\0';
					NMEAParserControl.m_nState = NP_STATE_CHECKSUM_1;
				}
			}
			else // no checksum flag, store data
			{
				//
				// Check for end of sentence with no checksum
				//
				if(NMEAParserControl.m_wIndex >= (WORD)NP_MAX_DATA_LEN) // Check for buffer overflow
				{
					NMEAParserControl.m_nState = NP_STATE_SOM;
				}else
				{
					if(btData == '\r')
					{
						NMEAParserControl.m_pData[NMEAParserControl.m_wIndex] = '\0';
						NMEAParser_ProcessCommand(NMEAParserControl.m_pCommand, NMEAParserControl.m_pData);
						NMEAParserControl.m_nState = NP_STATE_SOM;
						return;
					}

					//
					// Store data and calculate checksum
					//
					NMEAParserControl.m_btChecksum ^= btData;
					NMEAParserControl.m_pData[NMEAParserControl.m_wIndex] = btData;
					if(++NMEAParserControl.m_wIndex >= (WORD)NP_MAX_DATA_LEN) // Check for buffer overflow
					{
						NMEAParserControl.m_nState = NP_STATE_SOM;
					}
				}
			}
		break;

		case NP_STATE_CHECKSUM_1 :
			if( (btData - '0') <= 9)
			{
				NMEAParserControl.m_btReceivedChecksum = (btData - '0') << 4;
			}
			else
			{
				NMEAParserControl.m_btReceivedChecksum = (btData - 'A' + 10) << 4;
			}

			NMEAParserControl.m_nState = NP_STATE_CHECKSUM_2;

		break;

		case NP_STATE_CHECKSUM_2 :
			if( (btData - '0') <= 9)
			{
				NMEAParserControl.m_btReceivedChecksum |= (btData - '0');
			}
			else
			{
				NMEAParserControl.m_btReceivedChecksum |= (btData - 'A' + 10);
			}

			if(NMEAParserControl.m_btChecksum == NMEAParserControl.m_btReceivedChecksum)
			{
				NMEAParser_ProcessCommand(NMEAParserControl.m_pCommand, NMEAParserControl.m_pData);
			}

			NMEAParserControl.m_nState = NP_STATE_SOM;
		break;

		default :
			NMEAParserControl.m_nState = NP_STATE_SOM;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Process NMEA sentence - Use the NMEA address (*pCommand) and call the
// appropriate sentense data prossor.
///////////////////////////////////////////////////////////////////////////////
BOOL NMEAParser_ProcessCommand(BYTE *pCommand, BYTE *pData)
{
	if( strcmp((char *)pCommand, "GPGGA") == NULL )
	{
		NMEAParser_ProcessGPGGA(pData);
	}
	else if( strcmp((char *)pCommand, "GPGSA") == NULL )
	{
		NMEAParser_ProcessGPGSA(pData);
	}
	else if( strcmp((char *)pCommand, "GPRMC") == NULL )
	{
		NMEAParser_ProcessGPRMC(pData);
	}
	//
	// GPGSV
	//
	else if( strcmp((char *)pCommand, "GPGSV") == NULL )
	{
		NMEAParser_ProcessGPGSV(pData);
	}

/*	//
	// GPRMB
	//
	else if( strcmp((char *)pCommand, "GPRMB") == NULL )
	{
		NMEAParser_ProcessGPRMB(pData);
	}*/

	//
	// GPRMC
	//
/*	else if( strcmp((char *)pCommand, "GPRMC") == NULL )
	{
		NMEAParser_ProcessGPRMC(pData);
	}*/

	//
	// GPZDA
	//
	else if( strcmp((char *)pCommand, "GPZDA") == NULL )
	{
		NMEAParser_ProcessGPZDA(pData);
	}

	NMEAParserControl.m_dwCommandCount++;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Name:		NMEAParser_GetField
//
// Description:	This function will get the specified field in a NMEA string.
//
// Entry:		BYTE *pData -		Pointer to NMEA string
//				BYTE *pField -		pointer to returned field
//				int nfieldNum -		Field offset to get
//				int nMaxFieldLen -	Maximum of bytes pFiled can handle
///////////////////////////////////////////////////////////////////////////////
BOOL NMEAParser_GetField(BYTE *pData, BYTE *pField, int nFieldNum, int nMaxFieldLen)
{
	//
	// Validate params
	//
	if(pData == NULL || pField == NULL || nMaxFieldLen <= 0)
	{
		return FALSE;
	}

	//
	// Go to the beginning of the selected field
	//
	int i = 0;
	int nField = 0;
	while(nField != nFieldNum && pData[i])
	{
		if(pData[i] == ',')
		{
			nField++;
		}

		i++;

		if(pData[i] == NULL)
		{
			pField[0] = '\0';
			return FALSE;
		}
	}

	if(pData[i] == ',' || pData[i] == '*')
	{
		pField[0] = '\0';
		return FALSE;
	}

	//
	// copy field from pData to Field
	//
	int i2 = 0;
	while(pData[i] != ',' && pData[i] != '*' && pData[i])
	{
		pField[i2] = pData[i];
		i2++; i++;

		//
		// check if field is too big to fit on passed parameter. If it is,
		// crop returned field to its max length.
		//
		if(i2 >= nMaxFieldLen)
		{
			i2 = nMaxFieldLen-1;
			break;
		}
	}
	pField[i2] = '\0';

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Reset: Reset all NMEA data to start-up default values.
///////////////////////////////////////////////////////////////////////////////
void NMEAParser_Reset()
{
	int i;

	//
	// GPGGA Data
	//
	NMEAParserControl.m_btGGAHour = 0;					//
	NMEAParserControl.m_btGGAMinute = 0;					//
	NMEAParserControl.m_btGGASecond = 0;					//
	NMEAParserControl.m_dGGALatitude = 0.0;				// < 0 = South, > 0 = North
	NMEAParserControl.m_dGGALongitude = 0.0;				// < 0 = West, > 0 = East
	NMEAParserControl.m_btGGAGPSQuality = 0;				// 0 = fix not available, 1 = GPS sps mode, 2 = Differential GPS, SPS mode, fix valid, 3 = GPS PPS mode, fix valid
	NMEAParserControl.m_btGGANumOfSatsInUse = 0;			//
	NMEAParserControl.m_dGGAHDOP = 0.0;					//
	NMEAParserControl.m_dGGAAltitude = 0.0;				// Altitude: mean-sea-level (geoid) meters
	NMEAParserControl.m_dwGGACount = 0;					//
	NMEAParserControl.m_nGGAOldVSpeedSeconds = 0;			//
	NMEAParserControl.m_dGGAOldVSpeedAlt = 0.0;			//
	NMEAParserControl.m_dGGAVertSpeed = 0.0;				//

	//
	// GPGSA
	//
	NMEAParserControl.m_btGSAMode = 'M';					// M = manual, A = automatic 2D/3D
	NMEAParserControl.m_btGSAFixMode = 1;					// 1 = fix not available, 2 = 2D, 3 = 3D
	for(i = 0; i < NP_MAX_CHAN; i++)
	{
		NMEAParserControl.m_wGSASatsInSolution[i] = 0;	// ID of sats in solution
	}
	NMEAParserControl.m_dGSAPDOP = 0.0;					//
	NMEAParserControl.m_dGSAHDOP = 0.0;					//
	NMEAParserControl.m_dGSAVDOP = 0.0;					//
	NMEAParserControl.m_dwGSACount = 0;					//

	//
	// GPGSV
	//
	NMEAParserControl.m_btGSVTotalNumOfMsg = 0;			//
	NMEAParserControl.m_wGSVTotalNumSatsInView = 0;		//
	for(i = 0; i < NP_MAX_CHAN; i++)
	{
		NMEAParserControl.m_GSVSatInfo[i].m_wAzimuth = 0;
		NMEAParserControl.m_GSVSatInfo[i].m_wElevation = 0;
		NMEAParserControl.m_GSVSatInfo[i].m_wPRN = 0;
		NMEAParserControl.m_GSVSatInfo[i].m_wSignalQuality = 0;
		NMEAParserControl.m_GSVSatInfo[i].m_bUsedInSolution = FALSE;
	}
	NMEAParserControl.m_dwGSVCount = 0;

	//
	// GPRMB
	//
	NMEAParserControl.m_btRMBDataStatus = 'V';			// A = data valid, V = navigation receiver warning
	NMEAParserControl.m_dRMBCrosstrackError = 0.0;		// nautical miles
	NMEAParserControl.m_btRMBDirectionToSteer = '?';		// L/R
	NMEAParserControl.m_lpszRMBOriginWaypoint[0] = '\0';	// Origin Waypoint ID
	NMEAParserControl.m_lpszRMBDestWaypoint[0] = '\0';	// Destination waypoint ID
	NMEAParserControl.m_dRMBDestLatitude = 0.0;			// destination waypoint latitude
	NMEAParserControl.m_dRMBDestLongitude = 0.0;			// destination waypoint longitude
	NMEAParserControl.m_dRMBRangeToDest = 0.0;			// Range to destination nautical mi
	NMEAParserControl.m_dRMBBearingToDest = 0.0;			// Bearing to destination, degrees TRUE
	NMEAParserControl.m_dRMBDestClosingVelocity = 0.0;	// Destination closing velocity, knots
	NMEAParserControl.m_btRMBArrivalStatus = 'V';			// A = arrival circle entered, V = not entered
	NMEAParserControl.m_dwRMBCount = 0;					//

	//
	// GPRMC
	//
	NMEAParserControl.m_btRMCHour = 0;					//
	NMEAParserControl.m_btRMCMinute = 0;					//
	NMEAParserControl.m_btRMCSecond = 0;					//
	NMEAParserControl.m_btRMCDataValid = 'V';				// A = Data valid, V = navigation rx warning
	NMEAParserControl.m_dRMCLatitude = 0.0;				// current latitude
	NMEAParserControl.m_dRMCLongitude = 0.0;				// current longitude
	NMEAParserControl.m_dRMCGroundSpeed = 0.0;			// speed over ground, knots
	NMEAParserControl.m_dRMCCourse = 0.0;					// course over ground, degrees TRUE
	NMEAParserControl.m_btRMCDay = 1;						//
	NMEAParserControl.m_btRMCMonth = 1;					//
	NMEAParserControl.m_wRMCYear = 2000;					//
	NMEAParserControl.m_dRMCMagVar = 0.0;					// magnitic variation, degrees East(+)/West(-)
	NMEAParserControl.m_dwRMCCount = 0;					//

	//
	// GPZDA
	//
	NMEAParserControl.m_btZDAHour = 0;					//
	NMEAParserControl.m_btZDAMinute = 0;					//
	NMEAParserControl.m_btZDASecond = 0;					//
	NMEAParserControl.m_btZDADay = 1;						// 1 - 31
	NMEAParserControl.m_btZDAMonth = 1;					// 1 - 12
	NMEAParserControl.m_wZDAYear = 2000;					//
	NMEAParserControl.m_btZDALocalZoneHour = 0;			// 0 to +/- 13
	NMEAParserControl.m_btZDALocalZoneMinute = 0;			// 0 - 59
	NMEAParserControl.m_dwZDACount = 0;					//
}

///////////////////////////////////////////////////////////////////////////////
// Check to see if supplied satellite ID is used in the GPS solution.
// Retruned:	BOOL -	TRUE if satellate ID is used in solution
//						FALSE if not used in solution.
///////////////////////////////////////////////////////////////////////////////
BOOL NMEAParser_IsSatUsedInSolution(WORD wSatID)
{
	if(wSatID == 0) return FALSE;
	for(int i = 0; i < 12; i++)
	{
		if(wSatID == NMEAParserControl.m_wGSASatsInSolution[i])
		{
			return TRUE;
		}
	}

	return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void NMEAParser_ProcessGPGGA(BYTE *pData)
{
	BYTE pField[MAXFIELD];
	char pBuff[10];

	//
	// Time
	//
	if(NMEAParser_GetField(pData, pField, 0, MAXFIELD))
	{
		// Hour
		pBuff[0] = pField[0];
		pBuff[1] = pField[1];
		pBuff[2] = '\0';
		NMEAParserControl.m_btGGAHour = (BYTE)atoi(pBuff);

		// minute
		pBuff[0] = pField[2];
		pBuff[1] = pField[3];
		pBuff[2] = '\0';
		NMEAParserControl.m_btGGAMinute = (BYTE)atoi(pBuff);

		// Second
		pBuff[0] = pField[4];
		pBuff[1] = pField[5];
		pBuff[2] = '\0';
		NMEAParserControl.m_btGGASecond = (BYTE)atoi(pBuff);
	}

	//
	// Latitude
	//
	if(NMEAParser_GetField(pData, pField, 1, MAXFIELD))
	{
		NMEAParserControl.m_dGGALatitude = (double)atof((char *)pField+2) / 60.0;
		pField[2] = '\0';
		NMEAParserControl.m_dGGALatitude += (double)atof((char *)pField);

	}
	if(NMEAParser_GetField(pData, pField, 2, MAXFIELD))
	{
		if(pField[0] == 'S')
		{
			NMEAParserControl.m_dGGALatitude = -NMEAParserControl.m_dGGALatitude;
		}
	}

	//
	// Longitude
	//
	if(NMEAParser_GetField(pData, pField, 3, MAXFIELD))
	{
		NMEAParserControl.m_dGGALongitude = atof((char *)pField+3) / 60.0;
		pField[3] = '\0';
		NMEAParserControl.m_dGGALongitude += atof((char *)pField);
	}
	if(NMEAParser_GetField(pData, pField, 4, MAXFIELD))
	{
		if(pField[0] == 'W')
		{
			NMEAParserControl.m_dGGALongitude = -NMEAParserControl.m_dGGALongitude;
		}
	}

	//
	// GPS quality
	//
	if(NMEAParser_GetField(pData, pField, 5, MAXFIELD))
	{
		NMEAParserControl.m_btGGAGPSQuality = pField[0] - '0';
	}

	//
	// Satellites in use
	//
	if(NMEAParser_GetField(pData, pField, 6, MAXFIELD))
	{
		pBuff[0] = pField[0];
		pBuff[1] = pField[1];
		pBuff[2] = '\0';
		NMEAParserControl.m_btGGANumOfSatsInUse = (BYTE)atoi(pBuff);
	}

	//
	// HDOP
	//
	if(NMEAParser_GetField(pData, pField, 7, MAXFIELD))
	{
		NMEAParserControl.m_dGGAHDOP = atof((char *)pField);
	}
	
	//
	// Altitude
	//
	if(NMEAParser_GetField(pData, pField, 8, MAXFIELD))
	{
		NMEAParserControl.m_dGGAAltitude = atof((char *)pField);
	}

	//
	// Durive vertical speed (bonus)
	//
	int nSeconds = (int)NMEAParserControl.m_btGGAMinute * 60 + (int)NMEAParserControl.m_btGGASecond;
	if(nSeconds > NMEAParserControl.m_nGGAOldVSpeedSeconds)
	{
		double dDiff = (double)(NMEAParserControl.m_nGGAOldVSpeedSeconds-nSeconds);
		double dVal = dDiff/60.0;
		if(dVal != 0.0)
		{
			NMEAParserControl.m_dGGAVertSpeed = (NMEAParserControl.m_dGGAOldVSpeedAlt - NMEAParserControl.m_dGGAAltitude) / dVal;
		}
	}
	NMEAParserControl.m_dGGAOldVSpeedAlt = NMEAParserControl.m_dGGAAltitude;
	NMEAParserControl.m_nGGAOldVSpeedSeconds = nSeconds;

	NMEAParserControl.m_dwGGACount++;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void NMEAParser_ProcessGPGSA(BYTE *pData)
{
	BYTE pField[MAXFIELD];
	char pBuff[10];

	//
	// Mode
	//
	if(NMEAParser_GetField(pData, pField, 0, MAXFIELD))
	{
		NMEAParserControl.m_btGSAMode = pField[0];
	}

	//
	// Fix Mode
	//
	if(NMEAParser_GetField(pData, pField, 1, MAXFIELD))
	{
		NMEAParserControl.m_btGSAFixMode = pField[0] - '0';
	}

	//
	// Active satellites
	//
	for(int i = 0; i < 12; i++)
	{
		if(NMEAParser_GetField(pData, pField, 2 + i, MAXFIELD))
		{
			pBuff[0] = pField[0];
			pBuff[1] = pField[1];
			pBuff[2] = '\0';
			NMEAParserControl.m_wGSASatsInSolution[i] = (WORD)atoi(pBuff);
		}
		else
		{
			NMEAParserControl.m_wGSASatsInSolution[i] = 0;
		}
	}

	//
	// PDOP
	//
	if(NMEAParser_GetField(pData, pField, 14, MAXFIELD))
	{
		NMEAParserControl.m_dGSAPDOP = atof((char *)pField);
	}
	else
	{
		NMEAParserControl.m_dGSAPDOP = 0.0;
	}

	//
	// HDOP
	//
	if(NMEAParser_GetField(pData, pField, 15, MAXFIELD))
	{
		NMEAParserControl.m_dGSAHDOP = atof((char *)pField);
	}
	else
	{
		NMEAParserControl.m_dGSAHDOP = 0.0;
	}

	//
	// VDOP
	//
	if(NMEAParser_GetField(pData, pField, 16, MAXFIELD))
	{
		NMEAParserControl.m_dGSAVDOP = atof((char *)pField);
	}
	else
	{
		NMEAParserControl.m_dGSAVDOP = 0.0;
	}

	NMEAParserControl.m_dwGSACount++;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void NMEAParser_ProcessGPGSV(BYTE *pData)
{
	INT nTotalNumOfMsg=0, nMsgNum=0;
	BYTE pField[MAXFIELD];

	//
	// Total number of messages
	//
	if(NMEAParser_GetField(pData, pField, 0, MAXFIELD))
	{
		nTotalNumOfMsg = atoi((char *)pField);

		//
		// Make sure that the nTotalNumOfMsg is valid. This is used to
		// calculate indexes into an array. I've seen corrept NMEA strings
		// with no checksum set this to large values.
		//
		if(nTotalNumOfMsg > 9 || nTotalNumOfMsg < 0) return;
	}
	if(nTotalNumOfMsg < 1 || nTotalNumOfMsg*4 >= NP_MAX_CHAN)
	{
		return;
	}

	//
	// message number
	//
	if(NMEAParser_GetField(pData, pField, 1, MAXFIELD))
	{
		nMsgNum = atoi((char *)pField);
	}
	//
	// Make sure that the message number is valid. This is used to
	// calculate indexes into an array
	//
	if(nMsgNum > 9 || nMsgNum < 1) return;

	//
	// Total satellites in view
	//
	if(NMEAParser_GetField(pData, pField, 2, MAXFIELD))
	{
		NMEAParserControl.m_wGSVTotalNumSatsInView = (WORD)atoi((char *)pField);
	}

	//
	// Satelite data
	//
	for(int i = 0; i < 4; i++)
	{
		// Satellite ID
		if(NMEAParser_GetField(pData, pField, 3 + 4*i, MAXFIELD))
		{
			NMEAParserControl.m_GSVSatInfo[i+(nMsgNum-1)*4].m_wPRN = (WORD)atoi((char *)pField);
		}
		else
		{
			NMEAParserControl.m_GSVSatInfo[i+(nMsgNum-1)*4].m_wPRN = 0;
		}

		// Elevarion
		if(NMEAParser_GetField(pData, pField, 4 + 4*i, MAXFIELD))
		{
			NMEAParserControl.m_GSVSatInfo[i+(nMsgNum-1)*4].m_wElevation = (WORD)atoi((char *)pField);
		}
		else
		{
			NMEAParserControl.m_GSVSatInfo[i+(nMsgNum-1)*4].m_wElevation = 0;
		}

		// Azimuth
		if(NMEAParser_GetField(pData, pField, 5 + 4*i, MAXFIELD))
		{
			NMEAParserControl.m_GSVSatInfo[i+(nMsgNum-1)*4].m_wAzimuth = (WORD)atoi((char *)pField);
		}
		else
		{
			NMEAParserControl.m_GSVSatInfo[i+(nMsgNum-1)*4].m_wAzimuth = 0;
		}

		// SNR
		if(NMEAParser_GetField(pData, pField, 6 + 4*i, MAXFIELD))
		{
			NMEAParserControl.m_GSVSatInfo[i+(nMsgNum-1)*4].m_wSignalQuality = (WORD)atoi((char *)pField);
		}
		else
		{
			NMEAParserControl.m_GSVSatInfo[i+(nMsgNum-1)*4].m_wSignalQuality = 0;
		}

		//
		// Update "used in solution" (NMEAParserControl.m_bUsedInSolution) flag. This is base
		// on the GSA message and is an added convenience for post processing
		//
		NMEAParserControl.m_GSVSatInfo[i+(nMsgNum-1)*4].m_bUsedInSolution = NMEAParser_IsSatUsedInSolution(NMEAParserControl.m_GSVSatInfo[i+(nMsgNum-1)*4].m_wPRN);
	}

	NMEAParserControl.m_dwGSVCount++;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void NMEAParser_ProcessGPRMB(BYTE *pData)
{
	BYTE pField[MAXFIELD];

	//
	// Data status
	//
	if(NMEAParser_GetField(pData, pField, 0, MAXFIELD))
	{
		NMEAParserControl.m_btRMBDataStatus = pField[0];
	}
	else
	{
		NMEAParserControl.m_btRMBDataStatus = 'V';
	}

	//
	// Cross track error
	//
	if(NMEAParser_GetField(pData, pField, 1, MAXFIELD))
	{
		NMEAParserControl.m_dRMBCrosstrackError = atof((char *)pField);
	}
	else
	{
		NMEAParserControl.m_dRMBCrosstrackError = 0.0;
	}

	//
	// Direction to steer
	//
	if(NMEAParser_GetField(pData, pField, 2, MAXFIELD))
	{
		NMEAParserControl.m_btRMBDirectionToSteer = pField[0];
	}
	else
	{
		NMEAParserControl.m_btRMBDirectionToSteer = '?';
	}

	//
	// Orgin waypoint ID
	//
	if(NMEAParser_GetField(pData, pField, 3, MAXFIELD))
	{
		strcpy(NMEAParserControl.m_lpszRMBOriginWaypoint, (char *)pField);
	}
	else
	{
		NMEAParserControl.m_lpszRMBOriginWaypoint[0] = '\0';
	}

	//
	// Destination waypoint ID
	//
	if(NMEAParser_GetField(pData, pField, 4, MAXFIELD))
	{
		strcpy(NMEAParserControl.m_lpszRMBDestWaypoint, (char *)pField);
	}
	else
	{
		NMEAParserControl.m_lpszRMBDestWaypoint[0] = '\0';
	}

	//
	// Destination latitude
	//
	if(NMEAParser_GetField(pData, pField, 5, MAXFIELD))
	{
		NMEAParserControl.m_dRMBDestLatitude = atof((char *)pField+2) / 60.0;
		pField[2] = '\0';
		NMEAParserControl.m_dRMBDestLatitude += atof((char *)pField);

	}
	if(NMEAParser_GetField(pData, pField, 6, MAXFIELD))
	{
		if(pField[0] == 'S')
		{
			NMEAParserControl.m_dRMBDestLatitude = -NMEAParserControl.m_dRMBDestLatitude;
		}
	}

	//
	// Destination Longitude
	//
	if(NMEAParser_GetField(pData, pField, 7, MAXFIELD))
	{
		NMEAParserControl.m_dRMBDestLongitude = atof((char *)pField+3) / 60.0;
		pField[3] = '\0';
		NMEAParserControl.m_dRMBDestLongitude += atof((char *)pField);
	}
	if(NMEAParser_GetField(pData, pField, 8, MAXFIELD))
	{
		if(pField[0] == 'W')
		{
			NMEAParserControl.m_dRMBDestLongitude = -NMEAParserControl.m_dRMBDestLongitude;
		}
	}

	//
	// Range to destination nautical mi
	//
	if(NMEAParser_GetField(pData, pField, 9, MAXFIELD))
	{
		NMEAParserControl.m_dRMBRangeToDest = atof((char *)pField);
	}
	else
	{
		NMEAParserControl.m_dRMBCrosstrackError = 0.0;
	}

	//
	// Bearing to destination degrees TRUE
	//
	if(NMEAParser_GetField(pData, pField, 10, MAXFIELD))
	{
		NMEAParserControl.m_dRMBBearingToDest = atof((char *)pField);
	}
	else
	{
		NMEAParserControl.m_dRMBBearingToDest = 0.0;
	}

	//
	// Closing velocity
	//
	if(NMEAParser_GetField(pData, pField, 11, MAXFIELD))
	{
		NMEAParserControl.m_dRMBDestClosingVelocity = atof((char *)pField);
	}
	else
	{
		NMEAParserControl.m_dRMBDestClosingVelocity = 0.0;
	}

	//
	// Arrival status
	//
	if(NMEAParser_GetField(pData, pField, 12, MAXFIELD))
	{
		NMEAParserControl.m_btRMBArrivalStatus = pField[0];
	}
	else
	{
		NMEAParserControl.m_dRMBDestClosingVelocity = 'V';
	}

	NMEAParserControl.m_dwRMBCount++;

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void NMEAParser_ProcessGPRMC(BYTE *pData)
{
	char pBuff[10];
	BYTE pField[MAXFIELD];

	//
	// Time
	//
	if(NMEAParser_GetField(pData, pField, 0, MAXFIELD))
	{
		// Hour
		pBuff[0] = pField[0];
		pBuff[1] = pField[1];
		pBuff[2] = '\0';
		NMEAParserControl.m_btRMCHour = (BYTE)atoi(pBuff);

		// minute
		pBuff[0] = pField[2];
		pBuff[1] = pField[3];
		pBuff[2] = '\0';
		NMEAParserControl.m_btRMCMinute = (BYTE)atoi(pBuff);

		// Second
		pBuff[0] = pField[4];
		pBuff[1] = pField[5];
		pBuff[2] = '\0';
		NMEAParserControl.m_btRMCSecond = (BYTE)atoi(pBuff);
	}

	//
	// Data valid
	//
	if(NMEAParser_GetField(pData, pField, 1, MAXFIELD))
	{
		NMEAParserControl.m_btRMCDataValid = pField[0];
	}
	else
	{
		NMEAParserControl.m_btRMCDataValid = (BYTE)(char)'V';
	}

	//
	// latitude
	//
	if(NMEAParser_GetField(pData, pField, 2, MAXFIELD))
	{
		NMEAParserControl.m_dRMCLatitude = atof((char *)pField+2) / 60.0;
		pField[2] = '\0';
		NMEAParserControl.m_dRMCLatitude += atof((char *)pField);

	}
	if(NMEAParser_GetField(pData, pField, 3, MAXFIELD))
	{
		if(pField[0] == 'S')
		{
			NMEAParserControl.m_dRMCLatitude = -NMEAParserControl.m_dRMCLatitude;
		}
	}

	//
	// Longitude
	//
	if(NMEAParser_GetField(pData, pField, 4, MAXFIELD))
	{
		NMEAParserControl.m_dRMCLongitude = atof((char *)pField+3) / 60.0;
		pField[3] = '\0';
		NMEAParserControl.m_dRMCLongitude += atof((char *)pField);
	}
	if(NMEAParser_GetField(pData, pField, 5, MAXFIELD))
	{
		if(pField[0] == 'W')
		{
			NMEAParserControl.m_dRMCLongitude = -NMEAParserControl.m_dRMCLongitude;
		}
	}

	//
	// Ground speed
	//
	if(NMEAParser_GetField(pData, pField, 6, MAXFIELD))
	{
		NMEAParserControl.m_dRMCGroundSpeed = atof((char *)pField);
	}
	else
	{
		NMEAParserControl.m_dRMCGroundSpeed = 0.0;
		NMEAParserControl.m_btRMCDataValid = (BYTE)(char)'V';
	}

	//
	// course over ground, degrees TRUE
	//
	if(NMEAParser_GetField(pData, pField, 7, MAXFIELD))
	{
		NMEAParserControl.m_dRMCCourse = atof((char *)pField);
	}
	else
	{
		NMEAParserControl.m_dRMCCourse = 0.0;
		NMEAParserControl.m_btRMCDataValid = (BYTE)(char)'V';
	}

	//
	// Date
	//
	if(NMEAParser_GetField(pData, pField, 8, MAXFIELD))
	{
		// Day
		pBuff[0] = pField[0];
		pBuff[1] = pField[1];
		pBuff[2] = '\0';
		NMEAParserControl.m_btRMCDay = (BYTE)atoi(pBuff);

		// Month
		pBuff[0] = pField[2];
		pBuff[1] = pField[3];
		pBuff[2] = '\0';
		NMEAParserControl.m_btRMCMonth = (BYTE)atoi(pBuff);

		// Year (Only two digits. I wonder why?)
		pBuff[0] = pField[4];
		pBuff[1] = pField[5];
		pBuff[2] = '\0';
		NMEAParserControl.m_wRMCYear = (WORD)atoi(pBuff);
		NMEAParserControl.m_wRMCYear += 2000;				// make 4 digit date -- What assumptions should be made here?
	}

	//
	// course over ground, degrees TRUE
	//
	if(NMEAParser_GetField(pData, pField, 9, MAXFIELD))
	{
		NMEAParserControl.m_dRMCMagVar = atof((char *)pField);
	}
	else
	{
		NMEAParserControl.m_dRMCMagVar = 0.0;
	}
	if(NMEAParser_GetField(pData, pField, 10, MAXFIELD))
	{
		if(pField[0] == 'W')
		{
			NMEAParserControl.m_dRMCMagVar = -NMEAParserControl.m_dRMCMagVar;
		}
	}

	NMEAParserControl.m_dwRMCCount++;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void NMEAParser_ProcessGPZDA(BYTE *pData)
{
	char pBuff[10];
	BYTE pField[MAXFIELD];

	//
	// Time
	//
	if(NMEAParser_GetField(pData, pField, 0, MAXFIELD))
	{
		// Hour
		pBuff[0] = pField[0];
		pBuff[1] = pField[1];
		pBuff[2] = '\0';
		NMEAParserControl.m_btZDAHour = (BYTE)atoi(pBuff);

		// minute
		pBuff[0] = pField[2];
		pBuff[1] = pField[3];
		pBuff[2] = '\0';
		NMEAParserControl.m_btZDAMinute = (BYTE)atoi(pBuff);

		// Second
		pBuff[0] = pField[4];
		pBuff[1] = pField[5];
		pBuff[2] = '\0';
		NMEAParserControl.m_btZDASecond = (BYTE)atoi(pBuff);
	}

	//
	// Day
	//
	if(NMEAParser_GetField(pData, pField, 1, MAXFIELD))
	{
		NMEAParserControl.m_btZDADay = (BYTE)atoi((char *)pField);
	}
	else
	{
		NMEAParserControl.m_btZDADay = 1;
	}

	//
	// Month
	//
	if(NMEAParser_GetField(pData, pField, 2, MAXFIELD))
	{
		NMEAParserControl.m_btZDAMonth = (BYTE)atoi((char *)pField);
	}
	else
	{
		NMEAParserControl.m_btZDAMonth = 1;
	}

	//
	// Year
	//
	if(NMEAParser_GetField(pData, pField, 3, MAXFIELD))
	{
		NMEAParserControl.m_wZDAYear = (WORD)atoi((char *)pField);
	}
	else
	{
		NMEAParserControl.m_wZDAYear = 1;
	}

	//
	// Local zone hour
	//
	if(NMEAParser_GetField(pData, pField, 4, MAXFIELD))
	{
		NMEAParserControl.m_btZDALocalZoneHour = (BYTE)atoi((char *)pField);
	}
	else
	{
		NMEAParserControl.m_btZDALocalZoneHour = 0;
	}

	//
	// Local zone hour
	//
	if(NMEAParser_GetField(pData, pField, 5, MAXFIELD))
	{
		NMEAParserControl.m_btZDALocalZoneMinute = (BYTE)atoi((char *)pField);
	}
	else
	{
		NMEAParserControl.m_btZDALocalZoneMinute = 0;
	}

	NMEAParserControl.m_dwZDACount++;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
