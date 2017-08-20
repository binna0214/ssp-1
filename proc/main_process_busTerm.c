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
*  PROGRAM ID :       mainProcessBusTerm.c                                     *
*                                                                              *
*  DESCRIPTION:       MAIN ���α׷��� �����ϱ����� �⺻���� Load.              *
*                                                                              *
*  ENTRY POINT:     void MainProc( NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv )   *
*                   short CheckNewCriteriaInfoRecv( byte* bpRecvData,          *
*                                                   word wDataSize,            *
*							NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv )          *
*                   short ReceiptPrintingReq( byte* bpRecvData,                *
*                                             word wDataSize )                 *
*                   short ClearSharedCmdnData( byte bCmd )                     *
*                   short ClearAllSharedCmdnData( void )                       *
*                   short SetSharedCmdnDataLooping( byte bCmd, char* pcCmdData,*
*                                                   word wCmdDataSize )        *
*                   short SetSharedCmdnData( byte bCmd, char* pcCmdData,       *
*                                            word wCmdDataSize )               *
*                   short SetSharedDataforCmd( byte bCmd, char* pcCmdData,     *
*                                              word wCmdDataSize )             *
*                   short GetSharedCmd( byte* bCmd, char* pcCmdData,           *
*                                       word* wCmdDataSize )                   *
*                   short WriteGPSLog2Reconcile( void )                        *
*                   int CalculateTimeDif( struct timeval* stStartTime,         *
*                                         struct timeval* stEndTime )          *
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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

/*******************************************************************************
*  Inclusion of User Header Files                                              *
*******************************************************************************/
#include "main.h"
#include "main_environ_init.h"
#include "main_load_criteriaInfo.h"
#include "main_process_busTerm.h"
#include "load_parameter_file_mgt.h"
#include "../system/device_interface.h"
#include "../comm/term_comm_subterm.h"
#include "../system/gps.h"
#include "../comm/socket_comm.h"
#include "main_childProcess_manage.h"

#include "card_proc.h"
#include "card_proc_util.h"
#include "blpl_proc.h"
#include "write_trans_data.h"
#include "reconcile_file_mgt.h"

#define CITY_TOUR_BUS_ONE_TIME_TICKET		'0'
#define CITY_TOUR_BUS_ALL_DAY_TICKET		'1'

/*******************************************************************************
*  Declaration of Global Variables                                             *
*******************************************************************************/
static byte gabBeforeStationID[8] = { 0, };
static byte gabBeforeStationName[17] = { 0, };
static bool gboolIsVoiceEndMsg = FALSE;	// �������� �����û�� ����Ŀ ��¿���
static byte gbDisplayStartEndStatus = '1';		// Display Start & Stop
static bool gboolIsSubTermFileCheck = FALSE;	// ������������ ������翩��

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static bool ProcessCmd( byte bCmd, byte* pcCmdData, word wCmdDataSize, 
						NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv );
static short ProcCityTourTicketInput( byte* bpRecvData, word wDataSize );
static short MultiGetOn( byte* bpRecvData, word wDataSize );
static short CancelMultiGetOn( byte* bpRecvData, word wDataSize );
static short DriveStartStop( byte* bpRecvData, word wDataSize );
static short StationIDCorrection( byte* bpRecvData, word wDataSize );
static short TermInfoPrintingReq( void );
static void CheckTimeOut( void );
static short DisplayStartStop2FND( void );

extern int gpsloopcheck;

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MainProc                                                 *
*                                                                              *
*  DESCRIPTION :      �����߿��� ī��ó��. �����ÿ��� BLPL Merge�� ������.   *
*                                                                              *
*  INPUT PARAMETERS:  NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv                  *
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
void MainProc( NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv )
{
	short sRetVal = 0;
	byte bCmd = 0;
	char pcCmdData[41] = { 0, };
	word wCmdDataSize = 0;

	/*
	 * �����޸𸮿��� Command ������ Ȯ����
	*/
	sRetVal = GetSharedCmd( &bCmd, pcCmdData, &wCmdDataSize );	
	if ( (SUCCESS == sRetVal) && (CMD_REQUEST == pcCmdData[0]) )
	{
		/*
		 * Command�� �����ϰ�, ��û�ϰ�� �̸� Ȯ���� ó��
		*/
		ProcessCmd( bCmd, pcCmdData, wCmdDataSize, pstNewImgNVoiceRecv );
	}

	if( gpstSharedInfo->boolIsDriveNow == TRUE )
	{
		/*
		 * ī��ó��
		*/
		CardProc();				
	}
	else if( gboolIsMainTerm == TRUE )
	{
		if( gpstSharedInfo->boolIsBLPLMergeNow != TRUE )
		{		
			/*
			 * BL/PL Merge����
			*/
			BLPLMerge();
			sleep(1);
		}
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ProcessCmd                                               *
*                                                                              *
*  DESCRIPTION :      ���������۱⿡�� ������ COMMAND�� ó����                 *
*                                                                              *
*  INPUT PARAMETERS:  byte bCmd                                                *
*                     byte* pcCmdData                                          *
*                     word wCmdDataSize                                        *
*                     NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv                  *
*                                                                              *
*  RETURN/EXIT VALUE: bool                                                     *
*                                                                              *
*  Author  : Gwan Yul Kim                                                      *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static bool ProcessCmd( byte bCmd, byte* pcCmdData, word wCmdDataSize, 
						NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv )
{
	short sRetVal = 0;
	short sRetValKeySet = 0;
	int nIndex = 0;
	byte abCmdResponse = 0x00;

    for ( nIndex = 0; nIndex < wCmdDataSize; nIndex ++ )
    {
    	pcCmdData[nIndex] = pcCmdData[nIndex + 1];
    }
    pcCmdData[nIndex] = 0x00;
    wCmdDataSize--;

	sRetVal = -1;
	switch ( bCmd )
    {
    	case CMD_INPUT_CITY_TOUR_BUS_TICKET:
			if ( gpstSharedInfo->boolIsDriveNow == TRUE )
			{
	        	sRetVal = ProcCityTourTicketInput( pcCmdData, wCmdDataSize );
			}
        	break;
			
    	/* 
    	 * ���ν������� ������ ������
    	*/
        case CMD_MULTI_GETON :
			if ( gpstSharedInfo->boolIsDriveNow == TRUE )
			{
	        	sRetVal = MultiGetOn( pcCmdData, wCmdDataSize );
			}
        	break;
			
    	/* 
    	 * ���ν����������� �ʱ�ȭ��
    	*/
        case CMD_CANCEL_MULTI_GETON :
        	if( gpstSharedInfo->boolIsDriveNow == TRUE )
        		sRetVal = CancelMultiGetOn( pcCmdData, wCmdDataSize );
        	break;
			
    	/* 
    	 * �������/����
    	*/
        case CMD_START_STOP :
        	sRetVal = DriveStartStop( pcCmdData, wCmdDataSize );
        	break;
        case CMD_STATION_CORRECT :	// Station Correct
        	if( gpstSharedInfo->boolIsDriveNow == TRUE )
        		sRetVal = StationIDCorrection( pcCmdData, wCmdDataSize );
        	break;
		case CMD_NEW_CONF_IMG :
			sRetVal = CheckNewCriteriaInfoRecv( pcCmdData, wCmdDataSize, pstNewImgNVoiceRecv );
        	break;
        case CMD_PRINT :
        	if ( pcCmdData[0] == KPDCMD_CASHENT_RECEIPT_PRINT )		// Printing of Cash Receipt
    		{
	        	sRetVal = ReceiptPrintingReq( pcCmdData, wCmdDataSize );
          	}
          	else if ( pcCmdData[0] == KPDCMD_TERMINFO_PRINT )
          	{
          		sRetVal = TermInfoPrintingReq();	// Printing of Term Info
          	}
          	break;
/*	2005-11-24 0338���������� �������� ����
		case CMD_CONFIRM_CANCEL_STATION_CORRECT :
			sRetVal = ConfirmCancelStationCorrect( pcCmdData, wCmdDataSize );
			break;	*/
		case CMD_KEYSET_IDCENTER_WRITE :
			sRetVal = RegistOfflineID2PSAMbyEpurseIssuer();
			sRetValKeySet = RegistOfflineKeyset2PSAMbyEpurseIssuer();
			if ( sRetValKeySet != SUCCESS )
			{
				sRetVal = FALSE;
			}
			break;
        default :
        	return FALSE; // ErrCmd except 'D','S','Q'
    }

	if ( sRetVal == SUCCESS )	    	
		abCmdResponse = CMD_SUCCESS_RES;
	else						    	
		abCmdResponse = CMD_FAIL_RES;

	SetSharedDataforCmd( bCmd, &abCmdResponse, 1 );
		
	return sRetVal;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckNewCriteriaInfoRecv                                 *
*                                                                              *
*  DESCRIPTION :      ����PC���� ������ ����������� �޸𸮿� LOADING��      *
*                                                                              *
*  INPUT PARAMETERS:  byte* bpRecvData                                         *
*                     word wDataSize                                           *
*                     NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv                  *
*                                                                              *
*  RETURN/EXIT VALUE: short                                                    *
*                                                                              *
*  Author  : Gwan Yul Kim                                                      *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short CheckNewCriteriaInfoRecv( byte* bpRecvData, word wDataSize, 
								NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv )
{
	short sRetVal = 0;
	int fdImgFile = 0;
	int fdVoiceFile = 0;

	int nImgCutSize = 0;
	int nSeekPos = 0;

	int nIndex = 0;
	char cBuf[50] = { 0, };
	char cBufDate[15] = { 0, };
	char achFileApplyDate[15] = { 0, };
	char achFileVer[5] = { 0, };

	time_t tUpateParmsTime = 0;

	/*
	 * Argument�� ��û����
	 * 0 : ����� �ε���û
	 * 1 : �� bus100������ ����ÿ�û
	 * 2 : �� �������ϼ����� �����û
	 * 3 : ���������۱� ������ �����û -> Keypad_Proc���� ó����
	 */
	byte bCriteriaRecv = bpRecvData[0];
	byte bImgRecv = bpRecvData[1];
	byte bVoiceFileRecv = bpRecvData[2];
	byte bKpdImgRecv = bpRecvData[3];		
	
	/*
	 * �ſ���� ����
	 */
	if ( bCriteriaRecv == CMD_REQUEST )
	{
		/*
		 * DCS���� Para������ ConfInfo Reloading
		 */
		sRetVal = LoadOperParmInfo();
		if( sRetVal == SUCCESS )
			bpRecvData[0] = CMD_SUCCESS_RES;
		else
			bpRecvData[0] = CMD_FAIL_RES;
		pstNewImgNVoiceRecv->boolIsCriteriaRecv = TRUE;
	}

	tUpateParmsTime = 0;
	GetRTCTime( &tUpateParmsTime );

	/*
	 * �� F/W ����
	 */
	if ( bImgRecv == CMD_REQUEST )
	{
		sprintf( cBuf, "chmod 755 %s/* ", BUS_EXECUTE_DIR );
		system( cBuf );		
			
		bpRecvData[1] = CMD_SUCCESS_RES;
		memset( cBuf, 0x00, sizeof(cBuf) );
		
		nImgCutSize = FILE_DATE_SIZE;
		if ( gboolIsMainTerm == TRUE )
		{
			strcpy( cBuf, BUS_MAIN_IMAGE_FILE );
		}
		else
		{
			strcpy( cBuf, BUS_SUB_IMAGE_FILE );
			nImgCutSize += FILE_VERSION_SIZE;
		}

		fdImgFile = open( cBuf, O_RDONLY );
		if ( fdImgFile >= 0 )
		{
			memset( achFileApplyDate, 0, sizeof(achFileApplyDate) );
			memset( achFileVer, 0, sizeof(achFileVer) );
			nSeekPos = lseek( fdImgFile, nImgCutSize * (-1), SEEK_END );
			read( fdImgFile, (char *)&achFileApplyDate, FILE_DATE_SIZE );
			if ( gboolIsMainTerm == FALSE )
			{
				nSeekPos = lseek( fdImgFile, (-1)*FILE_VERSION_SIZE, 
									SEEK_END );
				read( fdImgFile, (char *)&achFileVer, FILE_VERSION_SIZE );
			}
			lseek( fdImgFile, 0L, SEEK_SET );
			close( fdImgFile );

			memset( cBufDate,0x00,sizeof(cBufDate) );
			TimeT2ASCDtime( tUpateParmsTime, cBufDate );
	
	       	if ( memcmp(achFileApplyDate, cBufDate, FILE_DATE_SIZE) <= 0 )
	       	{
				pstNewImgNVoiceRecv->boolIsImgRecv = TRUE;
				pstNewImgNVoiceRecv->boolIsReset = TRUE;
			}
		}
	}

	/*
	 * Req. of VoiceFile to Flash
	 */
	if ( bVoiceFileRecv == CMD_REQUEST )
	{
		bpRecvData[2] = CMD_SUCCESS_RES;
		memset( achFileApplyDate, 0, sizeof(achFileApplyDate) );
		memset( achFileVer, 0, sizeof(achFileVer) );
		memset( cBuf,0x00,sizeof(cBuf) );

		fdVoiceFile = open( VOICE0_FILE, O_RDONLY );
		if ( fdVoiceFile >= 0 )
		{
			nSeekPos = lseek( fdVoiceFile, 
				(-1)*(FILE_DATE_SIZE + FILE_VERSION_SIZE), SEEK_END );
			read( fdVoiceFile, cBuf, FILE_DATE_SIZE + FILE_VERSION_SIZE );
			close( fdVoiceFile );
			
			for ( nIndex = 0; nIndex < (FILE_DATE_SIZE + FILE_VERSION_SIZE); nIndex++ )
			{
				if ( cBuf[nIndex] < '0' || cBuf[nIndex] > '9')
				{
					return ErrRet( ERR_DATA_VERSION );
				}
			}

			memcpy( achFileApplyDate, cBuf, FILE_DATE_SIZE );
			memcpy( achFileVer, cBuf + FILE_DATE_SIZE, FILE_VERSION_SIZE );
		}

		memset( cBuf,0x00,sizeof(cBuf) );
		TimeT2ASCDtime( tUpateParmsTime, cBuf );
		if ( memcmp(achFileApplyDate, cBuf, FILE_DATE_SIZE) <= 0 )
		{
			// My Voice File Version
			memset( cBuf, 0x00, sizeof(cBuf) );
			fdVoiceFile = open( VOICEAPPLY_VERSION, O_RDONLY );
			if ( fdVoiceFile >= 0 )
			{
				read( fdVoiceFile, cBuf, FILE_VERSION_SIZE );
				close( fdVoiceFile );
			}

			if ( memcmp(achFileVer, cBuf, FILE_VERSION_SIZE) > 0 )
			{			
				pstNewImgNVoiceRecv->boolIsVoiceFileRecv = TRUE;

				/*
				 * ���������� ���ϼ����� ��������� �ʴ´�.
				 * 2006-05-19
				 */
				//pstNewImgNVoiceRecv->boolIsReset = TRUE;
			
				fdVoiceFile = open(VOICEAPPLY_FLAGFILE, O_WRONLY |O_CREAT|O_TRUNC);
				if ( fdVoiceFile < 0 )
					ErrLogWrite( ERR_FILE_OPEN );
				else
					close( fdVoiceFile );

				fdVoiceFile = open( VOICEAPPLY_VERSION, O_WRONLY |O_CREAT|O_TRUNC );
				if ( fdVoiceFile < 0 )
				{
					ErrLogWrite( ERR_FILE_OPEN );
				}
				else
				{
					write( fdVoiceFile, achFileVer, FILE_VERSION_SIZE );
					close( fdVoiceFile );
				}
			}
		}
	}
	
	if ( bKpdImgRecv == CMD_REQUEST )
	{
		pstNewImgNVoiceRecv->boolIsKpdImgRecv = TRUE;
		gpstSharedInfo->boolIsKpdImgRecv = TRUE;
	}

	return SUCCESS;
}

static short ProcCityTourTicketInput( byte* bpRecvData, word wDataSize )
{
	word wAdultCnt = 0;
	word wYoungCnt = 0;

	memset( &gstCityTourBusTicketInfo, 0x00,
		sizeof( CITY_TOUR_BUS_TICKET_INFO ) );

	// �����������Է¿���
	gstCityTourBusTicketInfo.boolIsTicketInput = TRUE;

	// �����������Է��Ͻ�
	gstCityTourBusTicketInfo.tInputDtime = gpstSharedInfo->tTermTime;

	// 1ȸ�ǿ��� (TRUE:1ȸ��, FALSE:���ϱ�)
	if ( bpRecvData[0] == CITY_TOUR_BUS_ONE_TIME_TICKET )
	{
		gstCityTourBusTicketInfo.boolIsOneTimeTicket = TRUE;
	}
	else
	{
		gstCityTourBusTicketInfo.boolIsOneTimeTicket = FALSE;
	}

	// ����� ����/���
	wAdultCnt = GetWORDFromASC( bpRecvData + 1, 3 );
	wYoungCnt = GetWORDFromASC( bpRecvData + 4, 3 );

	if ( wAdultCnt > 0 )
	{
		gstCityTourBusTicketInfo.abUserType[0] = USERTYPE_ADULT;
		gstCityTourBusTicketInfo.abUserCnt[0] = wAdultCnt;
		if ( wYoungCnt > 0 )
		{
			gstCityTourBusTicketInfo.abUserType[1] = USERTYPE_YOUNG;
			gstCityTourBusTicketInfo.abUserCnt[1] = wYoungCnt;
		}
	}

	if ( wAdultCnt == 0 && wYoungCnt > 0 )
	{
		gstCityTourBusTicketInfo.abUserType[0] = USERTYPE_YOUNG;
		gstCityTourBusTicketInfo.abUserCnt[0] = wYoungCnt;
	}

	// ���� ������ �ݾ�
	// 1ȸ��
	if ( gstCityTourBusTicketInfo.boolIsOneTimeTicket == TRUE )
	{
		gstCityTourBusTicketInfo.dwTicketAmt =
			wAdultCnt * 4750 + wYoungCnt * 2850;
	}
	// ���ϱ�
	else
	{
		gstCityTourBusTicketInfo.dwTicketAmt =
			wAdultCnt * 9500 + wYoungCnt * 7600;
	}
	

	Buzzer( 2, (word)100000 );
	DisplayDWORDInUpFND( gstCityTourBusTicketInfo.dwTicketAmt );
	DisplayDWORDInDownFND( 0 );

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      MultiGetOn                                               *
*                                                                              *
*  DESCRIPTION:       Sets Multi Get On Structure                              *
*                                                                              *
*  INPUT PARAMETERS:  byte* bpRecvData                                         *
*                     word wDataSize                                           *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - Success                                           *
*                     FALSE - Error                                            *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short MultiGetOn( byte* bpRecvData, word wDataSize )
{
	int nAdultCnt = 0;
	int nYoungCnt = 0;
	int nChildCnt = 0;
	char achTempCnt[10];

	memset( &gstMultiEntInfo, 0x00, sizeof(MULTI_ENT_INFO) );
	memset( achTempCnt, 0x00, sizeof(achTempCnt) );
	memcpy( achTempCnt, bpRecvData, 9 );

	nAdultCnt = GetINTFromASC( bpRecvData, 3 );
	nYoungCnt = GetINTFromASC( bpRecvData + 3, 3 );
	nChildCnt = GetINTFromASC( bpRecvData + 6, 3 );

	if ( nAdultCnt > 0 )
	{
		gstMultiEntInfo.abUserType[0] = 1;
		gstMultiEntInfo.abUserCnt[0] = nAdultCnt;
		if ( nYoungCnt > 0 )
		{
			gstMultiEntInfo.abUserType[1] = 4;
			gstMultiEntInfo.abUserCnt[1] = nYoungCnt;
			if ( nChildCnt > 0 )
			{
				gstMultiEntInfo.abUserType[2] = 2;
				gstMultiEntInfo.abUserCnt[2] = nChildCnt;
			}
		}
		else if( nChildCnt > 0 )
		{
			gstMultiEntInfo.abUserType[1] = 2;
			gstMultiEntInfo.abUserCnt[1] = nChildCnt;
		}
	}

	if ( (nAdultCnt <= 0) && (nYoungCnt > 0) )
	{
		gstMultiEntInfo.abUserType[0] = 4;
		gstMultiEntInfo.abUserCnt[0] = nYoungCnt;
		if ( nChildCnt > 0 )
		{
			gstMultiEntInfo.abUserType[1] = 2;
			gstMultiEntInfo.abUserCnt[1] = nChildCnt;
		}
	}

	if ( (nAdultCnt <= 0) && (nYoungCnt <= 0) && (nChildCnt > 0) )
	{
		gstMultiEntInfo.abUserType[0] = 2;
		gstMultiEntInfo.abUserCnt[0] = nChildCnt;
	}

	gstMultiEntInfo.boolIsMultiEnt = TRUE;
	gstMultiEntInfo.tInputDatetime = gpstSharedInfo->tTermTime;

	sprintf( achTempCnt, "-%d-", nAdultCnt + nYoungCnt + nChildCnt );
	Buzzer( 2, (word)100000 );
	DisplayASCInUpFND( achTempCnt );
	DisplayDWORDInDownFND( 0 );

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CancelMultiGetOn                                         *
*                                                                              *
*  DESCRIPTION:       Sets Multi Get On Structure                              *
*                                                                              *
*  INPUT PARAMETERS:  byte* bpRecvData                                         *
*                     word wDataSize                                           *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - Success                                           *
*                     FALSE - Error                                            *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short CancelMultiGetOn( byte* bpRecvData, word wDataSize )
{
	if ( bpRecvData[0] == '0')
	{
		memset( &gstMultiEntInfo, 0x00, sizeof( MULTI_ENT_INFO ) );
		memset( &gstCityTourBusTicketInfo, 0x00,
			sizeof( CITY_TOUR_BUS_TICKET_INFO ) );
	}
	else
	{
		return ErrRet( ERR_SETUP_DRIVE_CANCEL_MULTI_GETON );
	}

	DisplayDWORDInUpFND( 0 );
	DisplayDWORDInDownFND( 0 );

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DriveStartStop                                           *
*                                                                              *
*  DESCRIPTION:       Receives Command from Keypad and Start/Stop Driving      *
*                                                                              *
*  INPUT PARAMETERS:  byte* bpRecvData                                         *
*                     word wDataSize                                           *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - Success                                           *
*                     FALSE - Error                                            *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short DriveStartStop( byte* bpRecvData, word wDataSize )
{
	short sRetVal = 0;
	int nIndex = 0;
	
	switch( bpRecvData[0] )
	{
		/* 
		 * �������
		 */
		case 0x30 :    		
			// �������̸� ����
			if ( gpstSharedInfo->boolIsDriveNow == TRUE )
			{
				printf( "[DriveStartStop] �̹� �������̹Ƿ� ������� ����\n" );
				return ERR_SETUP_DRIVE_DRIVE_START_STOP;
			}

			// �����ܸ��� �ʼ�������� �������ϴ� ���
			// - '�ֽ� ���������� �����Ͽ� �ֽʽÿ�' ���� ��� �� ����
			if ( IsExistMainTermBasicInfoFile() == FALSE )
			{
				printf( "[DriveStartStop] �ʼ�������� �������ϹǷ� ������� ����\n" );

				DisplayASCInUpFND( FND_ERR_CRITERIA_INFOFILE_NO_EXIST );
				Buzzer( 5, 50000 );
				VoiceOut( VOICE_RECEIVE_LATEST_INFO );

				ctrl_event_info_write( EVENT_DRIVE_START_WITHOUT_MANDATORY_INFO );

				return ERR_SETUP_DRIVE_DRIVE_START_STOP;
			}

			// �̹� �ε�� ���������� ������ �ִ� ���
			// - '�ܸ��⸦ ������ �ֽñ� �ٶ��ϴ�' ���� ��� �� ����
			if ( IsAllZero( gstVehicleParm.abVehicleID,
					sizeof( gstVehicleParm.abVehicleID ) ) == TRUE ||
				 IsAllZero( gstVehicleParm.abTranspMethodCode,
				 	sizeof( gstVehicleParm.abTranspMethodCode ) ) == TRUE ||
				 gstVehicleParm.wTranspMethodCode == 0 )
			{
				printf( "[DriveStartStop] �ε�� ���������� ������ �־� ������� ����\n" );

				// ������� �ٽ� �ε��Ѵ�.
				LoadOperParmInfo();

				DisplayASCInUpFND( FND_ERR_CRITERIA_INFOFILE_NO_EXIST );
				Buzzer( 5, 50000 );
				VoiceOut( VOICE_CHECK_TERM );

				ctrl_event_info_write( EVENT_DRIVE_START_INVALID_VEHICLE_INFO );

				return ERR_SETUP_DRIVE_DRIVE_START_STOP;
			}

			// ī������α� �ʱ�ȭ
			InitCardErrLog();

			memset( gpstSharedInfo->abNowStationID, 0x00,
				sizeof(gpstSharedInfo->abNowStationID) );
			memset( gpstSharedInfo->abNowStationName, 0x00,
				sizeof(gpstSharedInfo->abNowStationName) );
			if ( gstStationInfoHeader.dwRecordCnt > 0 )
			{
				memcpy( gpstSharedInfo->abNowStationID,
						gpstStationInfo[0].abStationID,
						sizeof(gpstSharedInfo->abNowStationID) );
				memcpy( gpstSharedInfo->abNowStationName,
						gpstStationInfo[0].abStationName,
						sizeof(gpstSharedInfo->abNowStationName) );
			}

			// GPS ������ ���� - GPS �������� ���� ���θ� ���� üũ�� ��
			// ������� ����
    		CreateGPSThread();

			// GPS ���� üũ�� 4�� ����
			gpsloopcheck = 4;

			// �̴��� ����� �ʱ�ȭ��
			InitIP( NETWORK_DEVICE );

			memcpy( gstVehicleParm.abDriverID, bpRecvData + 1, 6 );
			sRetVal = WriteTransHeader();
			if ( sRetVal != SUCCESS )
			{
				CheckFreeMemory();
				
				ctrl_event_info_write( TR_FILE_HEADER_ERROR_EVENT );
				DisplayASCInUpFND( FND_ERR_MSG_WRITE_MAIN_TRANS_DATA );
				Buzzer( 5, 50000 );
				VoiceOut( VOICE_CHECK_TERM );

				gpstSharedInfo->boolIsDriveNow = FALSE;

				// GPS ������ ����
				KillGPSThread();
				
				return ERR_SETUP_DRIVE_DRIVE_START_STOP;
			}

	    	/* 
	    	 * ������ �����մϴ�.
	    	 */
			VoiceOut( VOICE_START_DRIVE );
			gpstSharedInfo->boolIsDriveNow = TRUE;

	    	/* 
	    	 * Bus�� ������ �ִ��� ����
	    	 */
			gboolIsEndStation = FALSE;
			gdwDelayTimeatEndStation = 0;
			gboolIsVoiceEndMsg = FALSE;

	    	/* 
	    	 * GPS ������
	    	 */
			gwGPSRecvRate = 0;
			
	    	/* 
	    	 * ����Ƚ��
	    	 */
			gwDriveCnt = 0;

	    	/* 
	    	 * ������ �̵��� �Ÿ�
	    	 */
			gdwDistInDrive = 0;
			
			memset( gpstSharedInfo->abLanCardStrength, 0x00,
					sizeof(gpstSharedInfo->abLanCardStrength) );
			memset( gpstSharedInfo->abLanCardQuality, 0x00,
					sizeof(gpstSharedInfo->abLanCardQuality) );

			DisplayDWORDInUpFND( 0 );
			DisplayDWORDInDownFND( 0 );

			break;

    	/* 
    	 * ��������
    	 */
		case 0x31 :			
			if ( gpstSharedInfo->boolIsDriveNow == FALSE )
			{
				return ErrRet( ERR_SETUP_DRIVE_DRIVE_START_STOP );
			}
			
			KillGPSThread();
			
			sRetVal = WriteTransTail();
			if ( sRetVal != SUCCESS )
			{
				DebugOut( "WriteTransTail sRetVal=%d, \n", sRetVal );
				return sRetVal;
			}

			
			for ( nIndex = 0; nIndex < 3; nIndex++ )
			{
				if ( unlink( CONTROL_TRANS_FILE ) != 0 )
				{
					continue;
				}
				break;
			}
			
			gpstSharedInfo->boolIsDriveNow = FALSE;
			gboolIsEndStation = FALSE;
			gdwDelayTimeatEndStation = 0;

			WriteGPSLog2Reconcile();
			unlink( CASH_GETON_SEQ_NO_FILE );
			
	    	/* 
	    	 * ������ �����մϴ�.
	    	 */
			VoiceOut( VOICE_END_DRIVE );

			/*
			 * �������� count... ��������ſ��� �����.
			 */
			gpstSharedInfo->dwDriveChangeCnt += 2;
			break;

		default :
			return ErrRet( ERR_SETUP_DRIVE_DRIVE_START_STOP );
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      StationIDCorrection                                      *
*                                                                              *
*  DESCRIPTION:       Receives Command from Keypad and Start/Stop Driving      *
*                                                                              *
*  INPUT PARAMETERS:  byte* bpRecvData                                         *
*                     word wDataSize                                           *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - Success                                           *
*                     FALSE - Error                                            *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short StationIDCorrection( byte* bpRecvData, word wDataSize )
{
	word wStationCnt = 0;
	char bchStationClassCode[4];

	int nIndex = 0;
	int nInner = 0;
	int nStationIndex = 0;

	memset(bchStationClassCode,0x00,sizeof(bchStationClassCode) );

	wStationCnt = gstStationInfoHeader.dwRecordCnt;

	if ( memcmp(gpstSharedInfo->abNowStationName,FIRST_STATION_NAME,10) == 0 )
	{
		for( nIndex = 0; nIndex < wStationCnt; nIndex++ )
		{
			if ( gpstStationInfo[nIndex].wStationOrder == 1 )
			{
				memcpy( gpstSharedInfo->abNowStationID,
						gpstStationInfo[nIndex].abStationID, 7 );
				memcpy( gpstSharedInfo->abNowStationName,
						gpstStationInfo[nIndex].abStationName, 16 );

				return SUCCESS;
			}
		}
	}

	memcpy( gabBeforeStationID, gpstSharedInfo->abNowStationID,
			sizeof( gpstSharedInfo->abNowStationID) );
	memcpy( gabBeforeStationName, gpstSharedInfo->abNowStationName,
			sizeof(gpstSharedInfo->abNowStationName) );

	if ( strlen(gpstSharedInfo->abKeyStationID) == 0 )
	{
		memcpy( gpstSharedInfo->abKeyStationID,
				gpstSharedInfo->abNowStationID,
				sizeof(gpstSharedInfo->abNowStationID) );
		memcpy( gpstSharedInfo->abKeyStationName,
				gpstSharedInfo->abNowStationName,
				sizeof(gpstSharedInfo->abNowStationName) );
	}

	for( nIndex = 0; nIndex < wStationCnt ; nIndex++ )
	{
/*	2005-11-24 0338���������� ������������
		if ( memcmp(gpstSharedInfo->abKeyStationID,
			gpstStationInfo[nIndex].abStationID, 7) == 0 )	*/
		if ( memcmp(gpstSharedInfo->abNowStationID,
			gpstStationInfo[nIndex].abStationID, 7) == 0 )
		{
			nStationIndex = gpstStationInfo[nIndex].wStationOrder;
			
			if ( bpRecvData[0] == 0x31 )
			{
		    	/* 
		    	 * 1������ UP
		    	*/
				nStationIndex++;
			}
			else
			{
		    	/* 
		    	 * 1������ DOWN
		    	*/
				nStationIndex--;
			}

/*			if ( nStationIndex == 0 )
			{
				nStationIndex = wStationCnt;
			}

			if ( nStationIndex > wStationCnt  )
			{
				nStationIndex -= wStationCnt;
			}	*/

			if ( nStationIndex < 0 )
			{
				return ErrRet( ERR_SETUP_DRIVE_STATION_ID_CORRECTION );
			}

			if ( nStationIndex > wStationCnt  )
			{
				return ErrRet( ERR_SETUP_DRIVE_STATION_ID_CORRECTION );
			}

			for( nInner = 0; nInner < wStationCnt ; nInner++ )
			{
				if ( gpstStationInfo[nInner].wStationOrder == nStationIndex )
				{
/*	2005-11-24 0338���������� ������������
					//������ ���۱�� ������
					gpstSharedInfo->boolIsSetStationfromKpd = TRUE;

					memcpy( gpstSharedInfo->abKeyStationID,
							gpstStationInfo[nInner].abStationID,
							sizeof(gpstSharedInfo->abKeyStationID) );
					memcpy( gpstSharedInfo->abKeyStationName,
							gpstStationInfo[nInner].abStationName,
							sizeof(gpstSharedInfo->abKeyStationName) );	*/

					memcpy( gpstSharedInfo->abNowStationID,
							gpstStationInfo[nInner].abStationID,
							sizeof(gpstSharedInfo->abKeyStationID) );
					memcpy( gpstSharedInfo->abNowStationName,
							gpstStationInfo[nInner].abStationName,
							sizeof(gpstSharedInfo->abKeyStationName) );

					return SUCCESS;
				}
			}
		}
	}

	return ErrRet( ERR_SETUP_DRIVE_STATION_ID_CORRECTION );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReceiptPrintingReq                                       *
*                                                                              *
*  DESCRIPTION:       Printing Receipt Info to Printer Process 			       *
*                                                                              *
*  INPUT PARAMETERS:  byte* bpRecvData                                         *
*                     word wDataSize                                           *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - Success                                           *
*                     FALSE - Error                                            *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short ReceiptPrintingReq( byte* bpRecvData, word wDataSize )
{
	short sRetVal = 0;
	bool boolIsDriveNow = FALSE;

	int nAdultCnt = 0;
	int nYoungCnt = 0;
	int nChildCnt = 0;

	char achAdultCnt[11] = {0, };
	char achYoungCnt[11] = {0, };
	char achChildCnt[11] = {0, };

	int nIndex = 0;

	MSGQUEUE_DATA		  stMsgQueueData;
	MSGQUEUE_RECEIPT_DATA stMsgQueueReceipt;

	boolIsDriveNow = gpstSharedInfo->boolIsDriveNow;
	if ( boolIsDriveNow != TRUE )
	{
		return ErrRet( ERR_SETUP_DRIVE_PRINTING_REQ );
	}

	memset( &stMsgQueueData, 0x00, sizeof(MSGQUEUE_DATA) );
	memset( &stMsgQueueReceipt, 0x20, sizeof(MSGQUEUE_RECEIPT_DATA) );
	stMsgQueueData.lMsgType = PRINT_RECEIPT;

	nAdultCnt = GetINTFromASC( bpRecvData + 1, RECEIPT_ADULTCOUNT_SIZE );
	nYoungCnt = GetINTFromASC( bpRecvData + RECEIPT_ADULTCOUNT_SIZE + 1, 
		RECEIPT_YOUNGCOUNT_SIZE );
	nChildCnt = GetINTFromASC( bpRecvData + RECEIPT_ADULTCOUNT_SIZE + 
		RECEIPT_YOUNGCOUNT_SIZE + 1, RECEIPT_CHILDCOUNT_SIZE );

	/*
	 * ���� ���ݽ������
	*/
	DWORD2ASC( gstNewFareInfo.dwAdultCashEntFare, achAdultCnt );
	
	/* 
	 * û�ҳ� ���ݽ������
	*/
	DWORD2ASC( gstNewFareInfo.dwYoungCashEntFare, achYoungCnt );
	
	/* 
	 * ��� ���ݽ������
	*/
	DWORD2ASC( gstNewFareInfo.dwChildCashEntFare, achChildCnt );

	memcpy( stMsgQueueReceipt.abBizNo, gstVehicleParm.abBizNo,
			sizeof(gstVehicleParm.abBizNo) );
	TimeT2ASCDtime( gpstSharedInfo->tTermTime, stMsgQueueReceipt.abDateTime );
	memset(stMsgQueueReceipt.abVehicleNo, 0x00,
		sizeof(stMsgQueueReceipt.abVehicleNo));
	memcpy(stMsgQueueReceipt.abVehicleNo, gstVehicleParm.abVehicleNo,12);

	memset(stMsgQueueReceipt.abTranspMethodCodeName, 0x00,
		sizeof(stMsgQueueReceipt.abTranspMethodCodeName));

	switch( gstVehicleParm.wTranspMethodCode )
	{
		case 101 :
			memcpy( stMsgQueueReceipt.abTranspMethodCodeName, "������", 6 );
			break;
		case 102 :
			memcpy( stMsgQueueReceipt.abTranspMethodCodeName, "�Ϲ��¼�", 8 );
			break;
		case 103 :
			memcpy( stMsgQueueReceipt.abTranspMethodCodeName, "����¼�", 8 );
			break;
		case 104 :
			memcpy( stMsgQueueReceipt.abTranspMethodCodeName, "��ȯ��", 6 );
			break;
		case 105 :
		case 151 :
			memcpy( stMsgQueueReceipt.abTranspMethodCodeName, "����", 4 );
			break;
		case 106:
			memcpy( stMsgQueueReceipt.abTranspMethodCodeName, "����", 4 );
			break;
		case 110 :
		case 115 :
			memcpy( stMsgQueueReceipt.abTranspMethodCodeName, "����", 4 );
			break;
		case 120 :
		case 121 :
		case 122 :
			memcpy( stMsgQueueReceipt.abTranspMethodCodeName, "����", 4 );
			break;
		case 130 :
		case 131 :
			memcpy( stMsgQueueReceipt.abTranspMethodCodeName, "����", 4 );
			break;
		case 140:
			memcpy( stMsgQueueReceipt.abTranspMethodCodeName, "���ɼ�ȯ", 8 );
			break;
		default :
			break;
	}
	memset(stMsgQueueReceipt.achBusStationName, 0x00,
		sizeof(stMsgQueueReceipt.achBusStationName));
	memcpy( stMsgQueueReceipt.achBusStationName,
			gpstSharedInfo->abNowStationName,
			sizeof(stMsgQueueReceipt.achBusStationName) );
	if ( nAdultCnt > 0 )
	{
		memcpy( stMsgQueueReceipt.abUserTypeName, "�Ϲ�", 4 );
		memcpy( stMsgQueueReceipt.abFare, achAdultCnt,
			sizeof(stMsgQueueReceipt.abFare) );

		for( nIndex = 0; nIndex < nAdultCnt; nIndex++ )
		{
			memcpy( stMsgQueueData.achMsgData, &stMsgQueueReceipt,
				sizeof(MSGQUEUE_RECEIPT_DATA) );
			sRetVal = msgsnd( gnMsgQueue, &stMsgQueueData,
							  sizeof(MSGQUEUE_RECEIPT_DATA), IPC_NOWAIT );
			if ( sRetVal == -1 )
			{
				return ErrRet( ERR_SETUP_DRIVE_MSGSEND );
			}

			/*
			 * ���� ���� �ŷ�����Ÿ�� �����
			*/
			WriteCashTransData( USERTYPE_ADULT );
		}
	}

	if ( nYoungCnt > 0 )
	{
		memcpy( stMsgQueueReceipt.abUserTypeName, "û�ҳ�", 6 );
		memcpy( stMsgQueueReceipt.abFare, achYoungCnt,
			sizeof(stMsgQueueReceipt.abFare) );

		for( nIndex = 0; nIndex < nYoungCnt; nIndex++ )
		{
			DebugOut( "\r\n �л� [%d]\n", nIndex );
			memcpy( stMsgQueueData.achMsgData, &stMsgQueueReceipt,
				sizeof(MSGQUEUE_RECEIPT_DATA) );
			sRetVal = msgsnd( gnMsgQueue, &stMsgQueueData,
					  sizeof(MSGQUEUE_RECEIPT_DATA), IPC_NOWAIT );
			if ( sRetVal == -1 )
			{
				return ErrRet( ERR_SETUP_DRIVE_MSGSEND );
			}

			/*
			 * ���� ���� �ŷ�����Ÿ�� �����
			*/
			WriteCashTransData( USERTYPE_YOUNG );
		}
	}

	if ( nChildCnt > 0 )
	{
		memset( stMsgQueueReceipt.abUserTypeName, 0x20,
			sizeof(stMsgQueueReceipt.abUserTypeName) );
		memcpy( stMsgQueueReceipt.abUserTypeName, "���", 6 );
		memcpy( stMsgQueueReceipt.abFare, achChildCnt,
			sizeof(stMsgQueueReceipt.abFare) );

		for( nIndex = 0; nIndex < nChildCnt; nIndex++ )
		{
			DebugOut( "\r\n ��� [%d]\n", nIndex );
			memcpy( stMsgQueueData.achMsgData, &stMsgQueueReceipt,
				sizeof(MSGQUEUE_RECEIPT_DATA) );
			sRetVal = msgsnd( gnMsgQueue, &stMsgQueueData,
					  sizeof(MSGQUEUE_RECEIPT_DATA), IPC_NOWAIT );
			if ( sRetVal == -1 )
			{
				return ErrRet( ERR_SETUP_DRIVE_MSGSEND );
			}

			/*
			 * ���� ���� �ŷ�����Ÿ�� �����
			*/
			WriteCashTransData( USERTYPE_CHILD );
		}
	}

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      TermInfoPrintingReq                                      *
*                                                                              *
*  DESCRIPTION:       Printing Req. to Printer Process 					       *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - Success                                           *
*                     FALSE - Error                                            *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short TermInfoPrintingReq( void )
{
	char pcCmdData[41] = { 0, };
	MSGQUEUE_DATA stMsgQueueData;

	memset( &stMsgQueueData, 0x00, sizeof(MSGQUEUE_DATA) );

	stMsgQueueData.lMsgType = PRINT_TERM_INFO;
	stMsgQueueData.achMsgData[0] = '0';

	msgsnd( gnMsgQueue, &stMsgQueueData, 1, IPC_NOWAIT );

	memset( pcCmdData, 0x00, sizeof(pcCmdData) );
	pcCmdData[0] = CMD_SUCCESS_RES;
	SetSharedDataforCmd( CMD_PRINT, pcCmdData, strlen(pcCmdData) );

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCRead                                                   *
*                                                                              *
*  DESCRIPTION:       This program reads Smart Card, checks validation of card *
*                     and saves Common Structure.                              *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
/*
static short ConfirmCancelStationCorrect( byte* bpRecvData, word wDataSize )
{
	switch( bpRecvData[0] )
	{
		case 0x30 :
			 //0336 2005.6.7
			if ( strlen(gpstSharedInfo->abKeyStationID) < 7 )
			{
				memcpy( gpstSharedInfo->abKeyStationID,
						gpstSharedInfo->abNowStationID,
						sizeof(gpstSharedInfo->abNowStationID) );
				memcpy( gpstSharedInfo->abKeyStationName,
						gpstSharedInfo->abNowStationName,
						sizeof(gpstSharedInfo->abNowStationName) );
			}

			memcpy( gpstSharedInfo->abNowStationID,
					gpstSharedInfo->abKeyStationID,
					sizeof( gpstSharedInfo->abNowStationID));
			memcpy( gpstSharedInfo->abNowStationName,
					gpstSharedInfo->abKeyStationName,
					sizeof(gpstSharedInfo->abNowStationName) );

			memcpy( gabBeforeStationID,
					gpstSharedInfo->abKeyStationID,
					sizeof( gpstSharedInfo->abNowStationID) );
			memcpy( gabBeforeStationName,
					gpstSharedInfo->abKeyStationName,
					sizeof(gpstSharedInfo->abNowStationName) );

			//������ ���۱⿡�� ������ ������ �Ϸ��
			gpstSharedInfo->boolIsSetStationfromKpd = FALSE;
			break;
		case 0x31 :
			memcpy( gpstSharedInfo->abNowStationID,
					gabBeforeStationID,
					sizeof( gpstSharedInfo->abNowStationID) );
			memcpy( gpstSharedInfo->abNowStationName,
					gabBeforeStationName,
					sizeof(gpstSharedInfo->abNowStationName) );

			//������ ���۱⿡�� ������ ������ �Ϸ��
			gpstSharedInfo->boolIsSetStationfromKpd = FALSE;
			break;
	}

	memset( gpstSharedInfo->abKeyStationID, 0x00,
			sizeof(gpstSharedInfo->abKeyStationID) );
	memset( gpstSharedInfo->abKeyStationName, 0x00,
			sizeof(gpstSharedInfo->abKeyStationName) );

	return SUCCESS;
}
*/

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ClearSharedCmdnData                                      *
*                                                                              *
*  DESCRIPTION:       This program clears Process Shared Cmd. & CmdData        *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short ClearSharedCmdnData( byte bCmd )
{
	if ( bCmd != gpstSharedInfo->bCommCmd )
	{
		return ERR_SETUP_DRIVE_SET_CMD_DATA;
	}

	if ( SemaAlloc( SEMA_KEY_SHARED_CMD ) < 0 )
	{
		return ErrRet( ERR_SETUP_DRIVE_SET_CMD_DATA );
	}

	gpstSharedInfo->bCommCmd = '0';
	memset( gpstSharedInfo->abCommData, 0x00,
			sizeof(gpstSharedInfo->abCommData) );
	gpstSharedInfo->bCmdDataSize = 0;

	while ( SemaFree( SEMA_KEY_SHARED_CMD ) < 0 )
	{
		usleep( 1000 );
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ClearAllSharedCmdnData                                   *
*                                                                              *
*  DESCRIPTION:       This program clears Process Shared Cmd. & CmdData        *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short ClearAllSharedCmdnData( void )
{
	if ( SemaAlloc( SEMA_KEY_SHARED_CMD ) < 0 )
	{
		return ErrRet( ERR_SETUP_DRIVE_SET_CMD_DATA );
	}

	gpstSharedInfo->bCommCmd = '0';
	memset( gpstSharedInfo->abCommData, 0x00,
			sizeof(gpstSharedInfo->abCommData) );
	gpstSharedInfo->bCmdDataSize = 0;

	SemaFree( SEMA_KEY_SHARED_CMD );
	
	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetSharedCmdnDataLooping                                 *
*                                                                              *
*  DESCRIPTION:       This program sets Process Shared Cmd. & CmdData          *
*                                                                              *
*  INPUT PARAMETERS:  byte bCmd                                                *
*                     char* pcCmdData                                          *
*                     word wCmdDataSize                                        *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
// TODO : ���ѷ����� ���� ���� -> ����Ƚ�� �õ� �� ���н� ���� �����ϵ��� ���� �ʿ�
short SetSharedCmdnDataLooping( byte bCmd, char* pcCmdData, word wCmdDataSize )
{
	short sRetVal = -1;

	while ( sRetVal != SUCCESS )
	{
		sRetVal = SetSharedCmdnData( bCmd, pcCmdData, wCmdDataSize );
		usleep( 1000 );
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetSharedCmdnData                                        *
*                                                                              *
*  DESCRIPTION:       This program sets Process Shared Cmd. & CmdData          *
*                                                                              *
*  INPUT PARAMETERS:  byte bCmd                                                *
*                     char* pcCmdData                                          *
*                     word wCmdDataSize                                        *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SetSharedCmdnData( byte bCmd, char* pcCmdData, word wCmdDataSize )
{
	byte bLocalCmd = 0;

	bLocalCmd = gpstSharedInfo->bCommCmd;
	if ( VALID_CMD(bLocalCmd) || 
	     !VALID_CMD(bCmd) ||
	     !VALID_CMD_SIZE(wCmdDataSize) )
	{
		return ErrRet( ERR_SETUP_DRIVE_SET_CMD_DATA );
	}

	if ( SemaAlloc( SEMA_KEY_SHARED_CMD ) < 0 )
	{
		return ErrRet( ERR_SETUP_DRIVE_SET_CMD_DATA );
	}

	gpstSharedInfo->bCommCmd = bCmd;
	memcpy( gpstSharedInfo->abCommData, pcCmdData, wCmdDataSize );
	gpstSharedInfo->bCmdDataSize = wCmdDataSize;

	SemaFree( SEMA_KEY_SHARED_CMD );
	
	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetSharedDataforCmd                                      *
*                                                                              *
*  DESCRIPTION:       This program sets Process Shared Cmd. & CmdData          *
*                                                                              *
*  INPUT PARAMETERS:  byte bCmd                                                *
*                     char* pcCmdData                                          *
*                     word wCmdDataSize                                        *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SetSharedDataforCmd( byte bCmd, char* pcCmdData, word wCmdDataSize )
{
	byte bLocalCmd = 0;

	bLocalCmd = gpstSharedInfo->bCommCmd;
	if ( !VALID_CMD(bLocalCmd) || 
	     !VALID_CMD(bCmd) || 
	     !VALID_CMD_SIZE(wCmdDataSize) || 
	     (bLocalCmd != bCmd) )
	{
		return ErrRet( ERR_SETUP_DRIVE_SET_DATA_FOR_CMD );
	}

	if (SemaAlloc( SEMA_KEY_SHARED_CMD ) < 0 )
	{
		return ErrRet( ERR_SETUP_DRIVE_SET_DATA_FOR_CMD );
	}

	memcpy( gpstSharedInfo->abCommData, pcCmdData, wCmdDataSize );
	gpstSharedInfo->bCmdDataSize = wCmdDataSize;

	SemaFree( SEMA_KEY_SHARED_CMD );

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetSharedCmd                                             *
*                                                                              *
*  DESCRIPTION:       This program sets Process Shared Cmd. & CmdData          *
*                                                                              *
*  INPUT PARAMETERS:  byte bCmd                                                *
*                     char* pcCmdData                                          *
*                     word wCmdDataSize                                        *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short GetSharedCmd( byte* bCmd, char* pcCmdData, word* wCmdDataSize )
{
	byte bLocalCmd = 0x00;
	byte abLocalData[41] = { 0, };
	word wDataSize = 0;

	*bCmd = 0;
	*wCmdDataSize = 0;

	bLocalCmd = gpstSharedInfo->bCommCmd;
	wDataSize = gpstSharedInfo->bCmdDataSize;
	if ( !VALID_CMD(bLocalCmd) || !VALID_CMD_SIZE(wDataSize) )
	{
		return ErrRet( ERR_SETUP_DRIVE_GET_CMD_DATA );
	}
	
	memset( abLocalData, 0x00, sizeof(abLocalData) );
	memcpy( abLocalData, gpstSharedInfo->abCommData, 
			sizeof(gpstSharedInfo->abCommData) );

	if ( SemaAlloc( SEMA_KEY_SHARED_CMD ) < 0 )
	{
		return ErrRet( ERR_SETUP_DRIVE_GET_CMD_DATA );
	}

	*bCmd = bLocalCmd;
	*wCmdDataSize = wDataSize;
	memcpy( pcCmdData, abLocalData, wDataSize );

	SemaFree( SEMA_KEY_SHARED_CMD ) ;

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteGPSLog2Reconcile                                    *
*                                                                              *
*  DESCRIPTION:       This program writes GPS Log.                             *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteGPSLog2Reconcile( void )
{
	short sRetVal = SUCCESS;
	short sRetValLocal = 0;
	RECONCILE_DATA stGPSReconcile;

	sRetValLocal = access( GPS_LOG_FILE2, F_OK );
	if ( sRetValLocal >= 0 )
	{
		memset( &stGPSReconcile, 0x00, sizeof(RECONCILE_DATA) );
		memcpy( stGPSReconcile.achFileName, GPS_LOG_FILE2,
				sizeof(GPS_LOG_FILE2) );
		stGPSReconcile.bSendStatus = 'b';
		if ( WriteReconcileFileList(&stGPSReconcile) < 0 )
		{
			sRetVal = ERR_SETUP_DRIVE_GPS_LOG_FILE2;
			ErrRet( sRetVal );
		}
	}

	sRetValLocal = access( GPS_LOG_FILE, F_OK);
	if (sRetValLocal >= 0)
	{
		memset( &stGPSReconcile, 0x00, sizeof(RECONCILE_DATA) );
		memcpy( stGPSReconcile.achFileName, GPS_LOG_FILE,
				sizeof(GPS_LOG_FILE) );
		stGPSReconcile.bSendStatus = 'c';
		if ( WriteReconcileFileList(&stGPSReconcile) < 0 )
		{
			sRetVal = ERR_SETUP_DRIVE_GPS_LOG_FILE;
			ErrRet( sRetVal );
		}
	}

	return sRetVal;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckTimeOut                                             *
*                                                                              *
*  DESCRIPTION:       ���ν����� �� ��Ƽ��������������Է������� �ʱ�ȭ��      *
*                     ����Ѵ�. �Է� �� 10�ʰ� ������, FND�� ����ü��          *
*                     �ʱ�ȭ�Ѵ�.                                              *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void CheckTimeOut( void )
{
	dword dwTimeDiff = 0;

	// ���ν������� ������ ���
	if ( gstMultiEntInfo.boolIsMultiEnt == TRUE )
	{
		dwTimeDiff = abs( gpstSharedInfo->tTermTime -
			gstMultiEntInfo.tInputDatetime );
		if ( dwTimeDiff >= MULTI_GET_ON_TIME )
		{
			memset( &gstMultiEntInfo, 0x00, sizeof( MULTI_ENT_INFO ) );

			DisplayDWORDInUpFND( 0 );
			DisplayDWORDInDownFND( 0 );
		}
	}

	// ��Ƽ��������������Է������� ������ ���
	if ( gstCityTourBusTicketInfo.boolIsTicketInput == TRUE )
	{
		dwTimeDiff = abs( gpstSharedInfo->tTermTime -
			gstCityTourBusTicketInfo.tInputDtime );
		if ( dwTimeDiff >= MULTI_GET_ON_TIME )
		{
			memset( &gstCityTourBusTicketInfo, 0x00,
				sizeof( CITY_TOUR_BUS_TICKET_INFO) );

			DisplayDWORDInUpFND( 0 );
			DisplayDWORDInDownFND( 0 );
		}
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DisplayStartStop2FND                                     *
*                                                                              *
*  DESCRIPTION:       Control FND Display Time                                 *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short DisplayStartStop2FND( void )
{
	static struct timeval stStartTime;
	static struct timeval stEndTime;

	if ( gboolIsDisplaySwitchOn == FALSE )
	{
		return 1;
	}

	if ( gbDisplayStartEndStatus == '0' )
	{
		(void) gettimeofday( &stStartTime, (struct timezone *)0 );
		gbDisplayStartEndStatus = '1' ;
	}

	if ( gbDisplayStartEndStatus == '1')
	{
		(void) gettimeofday( &stEndTime, (struct timezone *)0 );
	}

	if ( CalculateTimeDif( &stStartTime, &stEndTime ) > 0 )
	{
		/* 
		 * ===========CLEAR ==========
		*/
		if ( gpstSharedInfo->boolIsDriveNow == TRUE )
		{
			XLEDOff();
			DisplayASCInUpFND( FND_READY_MSG );
			DisplayASCInDownFND( FND_READY_MSG );
		}
		else if ( gpstSharedInfo->boolIsDriveNow == FALSE )
		{
			XLEDOff();
			DisplayASCInUpFND( FND_READY_MSG );
			DisplayASCInDownFND( FND_READY_MSG );
		}

		gboolIsDisplaySwitchOn = FALSE;
		gbDisplayStartEndStatus = '0';
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CalculateTimeDif                                         *
*                                                                              *
*  DESCRIPTION:       Calculate Micro Time Difference                          *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 1 - Greater than 0.4 Sec                                 *
*                     (-) - Less than or Equal to 0.4 Sec                      *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
int CalculateTimeDif( struct timeval* stStartTime, struct timeval* stEndTime )
{
	struct timeval stTimeDif;
	float fTimeDif = 0.;

	stTimeDif.tv_sec = stEndTime->tv_sec - stStartTime->tv_sec;
	stTimeDif.tv_usec = stEndTime->tv_usec - stStartTime->tv_usec;
	if ( stTimeDif.tv_usec < 0 )
	{
		stTimeDif.tv_sec--;
		stTimeDif.tv_usec += 1000000;
	}

	fTimeDif = stTimeDif.tv_sec + ( (float)stTimeDif.tv_usec / 1000000. );

	if ( fTimeDif > 0.4 )
	{
		return GREATHERTHAN_POINT4SECS;
	}

	return LESSTHANOREUQAL_POINT4SECS;
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckNDisplayDynamicInfo                                 *
*                                                                              *
*  DESCRIPTION:       Check Teminal Base H/W & S/W 							   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void CheckNDisplayDynamicInfo( void )
{
	short sRetVal = 0;
    time_t tNowTime = 0;
    byte boolIsDriveNow = 0;

	CheckTimeOut();

    GetRTCTime( &tNowTime );
    gpstSharedInfo->tTermTime = tNowTime;
    boolIsDriveNow = gpstSharedInfo->boolIsDriveNow;
	if ( gboolIsMainTerm == TRUE )
	{
		DisplayStartStop2FND();
		if ( boolIsDriveNow == TRUE )
		{
			if((gboolIsVoiceEndMsg == FALSE) && (gboolIsEndStation == TRUE))
			{
				gdwDelayTimeatEndStation++;
			}
			
			gpstSharedInfo->boolIsGpsValid = s_stRMC.PosStatus;
		}
		
		if ( END_MSG_COND(gbSubTermCnt, gdwDelayTimeatEndStation) )
		{
			VoiceOut( VOICE_PRESS_END_BUTTON );
			gdwDelayTimeatEndStation = 0;
			gboolIsVoiceEndMsg = TRUE;
		}
	}
	else
	{
		if( boolIsDriveNow == FALSE )
		{
			gboolIsSubTermFileCheck = FALSE;
		}
		
		if ( (boolIsDriveNow == TRUE) && (gboolIsSubTermFileCheck == FALSE) )
		{
			sRetVal = CheckParmFilesExist();
			if ( sRetVal != SUCCESS )
			{
				SendKillAllProcSignal();
			}

			gboolIsSubTermFileCheck = TRUE;
		}

		DisplayStartStop2FND();
    	DisplaySubTermTime();
    }
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendKillAllProcSignal                                    *
*                                                                              *
*  DESCRIPTION:       This program sends KillSignal to Main & Child Process.   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void SendKillAllProcSignal( void )
{
	// sigaction. signal. killpg. kill
	DebugOut( "====All Process Killed====\n" );
	kill( 0, SIGINT );
}

