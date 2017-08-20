/*******************************************************************************
*                                                                              *
*                      KSCC - Bus Embeded System                               *
*                                                                              *
*            All rights reserved. No part of this publication may be           *
*            reproduced, stored in a retrieval system or transmitted           *
*            in any form or by any means  -  electronic, mechanical,           *
*            photocopying, recording, or otherwise, without the prior          *
*            written permission of LG CNS.                                     *
*                                                                              *
********************************************************************************
*                                                                              *
*  PROGRAM ID :       main_Load_CriteriaInfo.c                                 *
*                                                                              *
*  DESCRIPTION:       MAIN ���α׷��� �����ϱ����� �⺻���� Load.              *
*                                                                              *
*  ENTRY POINT:     short OperParmNBasicInfo (void)                            *
*                   short VoiceVerLoad( void )								   *
*                                                                              *
*  INPUT FILES:     None                                                       *
*                                                                              *
*  OUTPUT FILES:    None                                                       *
*                                                                              *
*  SPECIAL LOGIC:   None                                                       *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2006/03/27 F/W Dev Team GwanYul Kim  Initial Release                         *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include <sys/stat.h>
#include <fcntl.h> 

/*******************************************************************************
*  Inclusion of User Header Files                                              *
*******************************************************************************/
#include "main_load_criteriaInfo.h"
#include "Load_parameter_file_mgt.h"
#include "main_process_busTerm.h"
#include "main_environ_init.h"
#include "main.h"
#include "Blpl_proc.h"
#include "../system/device_interface.h"
#include "../system/gps.h"

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short CheckNRecvVehicleParmFile(  byte* pabCmdSharedData  );
static bool IsRTCOK( void );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenDeviceFiles                                          *
*                                                                              *
*  DESCRIPTION :      �ֺ���� DEVICE OPEN                                     *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author  : Gwan Yul Kim                                                      *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short OperParmNBasicInfo ( NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv )
{
	short sRetVal = 0;
	byte abCmdSharedData[41] = { 0, };
	time_t tNowDtime = 0;
	
	VoiceVerLoad();
	if ( gboolIsMainTerm == TRUE )
	{
		IsRTCOK();
		
		sRetVal = CheckNRecvVehicleParmFile( abCmdSharedData );
		if ( sRetVal < 0 ) return sRetVal;
		
		/*
		 * �ſ����, ��F/W����, DRIVER IMAGE���� üũ
		 */
		abCmdSharedData[0] = 0x00;	
		CheckNewCriteriaInfoRecv( abCmdSharedData, 4, pstNewImgNVoiceRecv );
	}		
	else
	{
		if ( SUCCESS != access(VEHICLE_PARM_FILE, F_OK) )
		{
			return ErrRet( ERR_FILE_OPEN | GetFileNo(VEHICLE_PARM_FILE) );
		}
	}
	
	/*
	 * �ʱ� ������������ �ʱ�ȭ
	*/
	InitOperParmInfo();
	
	/*
	 * DCS���� Para������ ConfInfo Reloading
	*/
	LoadOperParmInfo();

	if ( gboolIsMainTerm == TRUE )
	{
		/*
		 * ������ȣ�� ȭ�鿡 DISPLAY
		 */
		DisplayVehicleID();	
		
		/*
		 * �ʿ�� BLPL Merge����
		 */
		if ( gpstSharedInfo->boolIsBLPLMergeNow != TRUE )
		{
			BLPLMerge();			
			sleep( 1 );
		}
	}

	// �����ܸ����� ��쿡�� üũ
	if ( gboolIsMainTerm == TRUE )
	{
		// ������ �����̸� "8001" �˶��ڵ带 ������ ����
		if ( IsExistFile( CONTROL_TRANS_FILE ) == TRUE )
		{
			printf( "[OperParmNBasicInfo] ������ ����\n" );
			ctrl_event_info_write( EVENT_BOOTING_DURING_DRIVING );
		}

		// RTC �ð��� �ϵ��ڵ��� �ð��� ����� "8003" �˶��ڵ带 ������ ����
		GetRTCTime( &tNowDtime );
		if ( tNowDtime < GetTimeTFromASCDtime( "20060101000000" ) ||
			 tNowDtime > GetTimeTFromASCDtime( "20100101000000" ) )
		{
			printf( "[OperParmNBasicInfo] RTC �ð� ����\n" );
			ctrl_event_info_write( EVENT_RTC_ERROR );
		}
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckNRecvVehicleParmFile                                *
*                                                                              *
*  DESCRIPTION :      TermComm Process���� Setup�� ��û��                      *
*                                                                              *
*  INPUT PARAMETERS:  SUCCESS                                                  *
*                     - FAIL                                                   *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author  : Gwan Yul Kim                                                      *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CheckNRecvVehicleParmFile( byte* pabCmdSharedData )
{
	short sRetVal = 0;
	byte bCmd = 0;
	byte abCmdSharedData[41] = { 0, };
	word wCmdDataSize = 0;
	byte bRequestResult = 0;
		
	memset( abCmdSharedData, 0x00, sizeof(abCmdSharedData) );
	abCmdSharedData[0] = CMD_REQUEST;	
	SetSharedCmdnDataLooping( CMD_SETUP, abCmdSharedData, 1 );
	
	bRequestResult = 0;
	while ( bRequestResult != CMD_SUCCESS_RES && 
			bRequestResult != CMD_FAIL_RES )
	{
		bCmd = '0';
		memset( abCmdSharedData, 0x00, sizeof(abCmdSharedData) );
		wCmdDataSize = 0;

		sRetVal = GetSharedCmd( &bCmd, abCmdSharedData, &wCmdDataSize );		
		if ( sRetVal == SUCCESS )
		{
			if ( bCmd == CMD_SETUP )
			{
				bRequestResult = abCmdSharedData[0];
			}
		}

		usleep( 1000000 );					// 1�� SLEEP
	}

	ClearSharedCmdnData( CMD_SETUP );

	if ( bRequestResult == CMD_FAIL_RES )
	{
		return ( ERR_FILE_OPEN | GetFileNo( VEHICLE_PARM_FILE ) );
	}

	memcpy( pabCmdSharedData, &abCmdSharedData[1], 40 );
		
	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      IsRTCOK                                                  *
*                                                                              *
*  DESCRIPTION :      �ܸ��� RTC���� CHECK��                                   *
*                                                                              *
*  INPUT PARAMETERS:  SUCCESS                                                  *
*                     - FAIL                                                   *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author  : Gwan Yul Kim                                                      *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static bool IsRTCOK( void )
{
	short sRetValue = 0;
	time_t tNowTime = 0;

	while( TRUE )
	{
		sRetValue = GetRTCTime( &tNowTime );
		if ( sRetValue < 2 )
		{
			if ( tNowTime > GetTimeTFromASCDtime("20050901000101") )
			{
				DebugOut( "\n====<LoadingInitConfInfo>RTC OK.====\n" );
				/*
				 * RTC OK
				*/
				break;
			}
		}

		DebugOut( "\n====<MAIN>RTC������. ���������.====\n" );
		DisplayASCInUpFND( FND_INIT_MSG );
		DisplayASCInDownFND( FND_INIT_MSG );
		Buzzer( 5, 50000 );

		/* 
		 * Msg of Term Time Checking
		*/
		VoiceOut( VOICE_CHECK_TERM_TIME );
		if ( nCommGPSThreadID != 0 )
		{
			/*
			 * 2Minute of GPS Setting
			*/
			sleep( 2 );
		}
		else
		{
			GpsTimeCheck();
			sleep( 3 );
		}
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      VoiceVerLoad                                             *
*                                                                              *
*  DESCRIPTION:       Reads MainTerm/ SubTerm VoiceFile Version.               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - success                                              *
*                     (-) - fail                                               *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short VoiceVerLoad( void )
{
	short sRetVal = 0 ;
	int fdVoiceVer = -1;
	int nIndex = 0;

	char achVoiceVer[5];

	memset( achVoiceVer,0x00,sizeof(achVoiceVer) );

	sRetVal = access( VOICEAPPLY_VERSION, F_OK );
	if ( sRetVal != SUCCESS )
	{
		DebugOut( "\r\n ���� ���� ������ ����\r\n" );
		return ErrRet( ERR_FILE_OPEN );
	}

	fdVoiceVer = open( VOICEAPPLY_VERSION, O_RDONLY );
	if ( fdVoiceVer < 0 )
	{
		return ErrRet( ERR_FILE_OPEN );
	}

	memset( achVoiceVer, 0x00, sizeof(achVoiceVer) );
	sRetVal = read( fdVoiceVer, achVoiceVer, 4 );
	close( fdVoiceVer );
	if ( sRetVal < SUCCESS )
	{
		DebugOut( "\r\n ���� ���� ���� read Fail\r\n" );
		return ErrRet( ERR_FILE_OPEN );
	}

	if ( gboolIsMainTerm == TRUE )
	{
		memcpy( gpstSharedInfo->abMainVoiceVer, achVoiceVer,
			sizeof(gpstSharedInfo->abMainVoiceVer) );
	}
	else
	{
		for ( nIndex = 0; nIndex < gbSubTermCnt; nIndex++ )
		{
			memcpy( gpstSharedInfo->abSubVoiceVer[nIndex], achVoiceVer,
			sizeof(gpstSharedInfo->abSubVoiceVer[nIndex]) );
		}
	}
	
	/*
	 * My Voice File Version
	*/
	memcpy( gstMyTermInfo.abVoiceVer, achVoiceVer, 4 );

	return SUCCESS;
}

