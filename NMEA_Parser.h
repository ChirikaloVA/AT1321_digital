//NMEAParser_Parser.h

#ifndef _NMEAParser_PARSER_H
#define _NMEAParser_PARSER_H

#include <iolpc2388.h>
#include "types.h"

#define NP_MAX_CMD_LEN			8		// maximum command length (NMEA address)
#define NP_MAX_DATA_LEN			256		// maximum data length
#define NP_MAX_CHAN				20		// maximum number of channels
#define NP_WAYPOINT_ID_LEN		32		// waypoint max string len

enum NP_STATE {
	NP_STATE_SOM =				1,		// Search for start of message
	NP_STATE_CMD,						// Get command
	NP_STATE_DATA,						// Get data
	NP_STATE_CHECKSUM_1,				// Get first checksum character
	NP_STATE_CHECKSUM_2,				// get second checksum character
};



//////////////////////////////////////////////////////////////////////
struct tagNPSatInfo
{
	WORD	m_wPRN;						//
	WORD	m_wSignalQuality;			//
	BOOL	m_bUsedInSolution;			//
	WORD	m_wAzimuth;					//
	WORD	m_wElevation;				//
};
//////////////////////////////////////////////////////////////////////



struct tagNMEAParserControl
{
//////////////////////////////////////////////////////////////////////
	BYTE m_btChecksum;					// Calculated NMEA sentence checksum
	BYTE m_btReceivedChecksum;			// Received NMEA sentence checksum (if exists)
	WORD m_wIndex;						// Index used for command and data
	BYTE m_pCommand[NP_MAX_CMD_LEN];	// NMEA command
	BYTE m_pData[NP_MAX_DATA_LEN];		// NMEA data
	enum NP_STATE m_nState;					// Current state protocol parser is in

	DWORD m_dwCommandCount;				// number of NMEA commands received (processed or not processed)

	//
	// GPGGA Data
	//
	BYTE m_btGGAHour;					//
	BYTE m_btGGAMinute;					//
	BYTE m_btGGASecond;					//
	double m_dGGALatitude;				// < 0 = South, > 0 = North
	double m_dGGALongitude;				// < 0 = West, > 0 = East
	BYTE m_btGGAGPSQuality;				// 0 = fix not available, 1 = GPS sps mode, 2 = Differential GPS, SPS mode, fix valid, 3 = GPS PPS mode, fix valid
	BYTE m_btGGANumOfSatsInUse;			//
	double m_dGGAHDOP;					//
	double m_dGGAAltitude;				// Altitude: mean-sea-level (geoid) meters
	DWORD m_dwGGACount;					//
	int m_nGGAOldVSpeedSeconds;			//
	double m_dGGAOldVSpeedAlt;			//
	double m_dGGAVertSpeed;				//

	//
	// GPGSA
	//
	BYTE m_btGSAMode;					// M = manual, A = automatic 2D/3D
	BYTE m_btGSAFixMode;				// 1 = fix not available, 2 = 2D, 3 = 3D
	WORD m_wGSASatsInSolution[NP_MAX_CHAN]; // ID of sats in solution
	double m_dGSAPDOP;					//
	double m_dGSAHDOP;					//
	double m_dGSAVDOP;					//
	DWORD m_dwGSACount;					//

	//
	// GPGSV
	//
	BYTE m_btGSVTotalNumOfMsg;			//
	WORD m_wGSVTotalNumSatsInView;		//
	struct tagNPSatInfo m_GSVSatInfo[NP_MAX_CHAN];	//
	DWORD m_dwGSVCount;					//

	//
	// GPRMB
	//
	BYTE m_btRMBDataStatus;				// A = data valid, V = navigation receiver warning
	double m_dRMBCrosstrackError;		// nautical miles
	BYTE m_btRMBDirectionToSteer;		// L/R
	char m_lpszRMBOriginWaypoint[NP_WAYPOINT_ID_LEN]; // Origin Waypoint ID
	char m_lpszRMBDestWaypoint[NP_WAYPOINT_ID_LEN]; // Destination waypoint ID
	double m_dRMBDestLatitude;			// destination waypoint latitude
	double m_dRMBDestLongitude;			// destination waypoint longitude
	double m_dRMBRangeToDest;			// Range to destination nautical mi
	double m_dRMBBearingToDest;			// Bearing to destination, degrees TRUE
	double m_dRMBDestClosingVelocity;	// Destination closing velocity, knots
	BYTE m_btRMBArrivalStatus;			// A = arrival circle entered, V = not entered
	DWORD m_dwRMBCount;					//

	//
	// GPRMC
	//
	BYTE m_btRMCHour;					//
	BYTE m_btRMCMinute;					//
	BYTE m_btRMCSecond;					//
	BYTE m_btRMCDataValid;				// A = Data valid, V = navigation rx warning
	double m_dRMCLatitude;				// current latitude
	double m_dRMCLongitude;				// current longitude
	double m_dRMCGroundSpeed;			// speed over ground, knots
	double m_dRMCCourse;				// course over ground, degrees TRUE
	BYTE m_btRMCDay;					//
	BYTE m_btRMCMonth;					//
	WORD m_wRMCYear;					//
	double m_dRMCMagVar;				// magnitic variation, degrees East(+)/West(-)
	DWORD m_dwRMCCount;					//

	//
	// GPZDA
	//
	BYTE m_btZDAHour;					//
	BYTE m_btZDAMinute;					//
	BYTE m_btZDASecond;					//
	BYTE m_btZDADay;					// 1 - 31
	BYTE m_btZDAMonth;					// 1 - 12
	WORD m_wZDAYear;					//
	BYTE m_btZDALocalZoneHour;			// 0 to +/- 13
	BYTE m_btZDALocalZoneMinute;		// 0 - 59
	DWORD m_dwZDACount;					//
	
	//common data field
	struct tagCommonGPS commonGPS;

};

extern struct tagNMEAParserControl NMEAParserControl;



	void NMEAParser_ProcessGPZDA(BYTE *pData);
	void NMEAParser_ProcessGPRMC(BYTE *pData);
	void NMEAParser_ProcessGPRMB(BYTE *pData);
	void NMEAParser_ProcessGPGSV(BYTE *pData);
	void NMEAParser_ProcessGPGSA(BYTE *pData);
	void NMEAParser_ProcessGPGGA(BYTE *pData);
	BOOL NMEAParser_IsSatUsedInSolution(WORD wSatID);
	void NMEAParser_Reset();
	BOOL NMEAParser_GetField(BYTE *pData, BYTE *pField, int nFieldNum, int nMaxFieldLen);
	BOOL NMEAParser_ProcessCommand(BYTE *pCommand, BYTE *pData);
	void NMEAParser_ProcessNMEA(BYTE btData);
	void NMEAParser_ParseBuffer(BYTE *pBuff, DWORD dwLen);

	void NMEAParser_Init(void);
	

#endif	//#ifndef _NMEAParser_PARSER_H
