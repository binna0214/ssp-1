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
*  PROGRAM ID :       upload_dcs_comm.c	                                       *
*                                                                              *
*  DESCRIPTION:       ����ý������κ��� ���� ���ε带 ���� �Լ����� �����Ѵ�.    *
*                                                                              *
*  ENTRY POINT:     short DownVehicleParmFile( void );						   *
*					short DownFromDCS( void );								   *
*                                                                              *
*  INPUT FILES:     reconcileinfo.dat                                          *
*					simxinfo.dat											   *
*					reconcileinfo.dat�� ��ϵ� ����							   *
*                                                                              *
*  OUTPUT FILES:    reconcileinfo.dat                                          *
*					reconcileinfo.dat�� ��ϵ� ����							   *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2006/03/27 F/W Dev Team Mi Hyun Noh  Initial Release                         *
*                                                                              *
*******************************************************************************/
/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "dcs_comm.h"
#include "../proc/write_trans_data.h"
#include "../proc/file_mgt.h"
#include "../proc/reconcile_file_mgt.h"

/*******************************************************************************
*  Declaration of variables ( connect command )                                *
*******************************************************************************/
#define SEND_TR_FILE_CMD                "B001H"     // �ŷ����� ���� �۽�cmd
#define SEND_EVENT_FILE_CMD             "B041H"     // �������� ���� �۽�cmd
#define SEND_SETUP_FILE_CMD             "B071H"     // ��ġ���� ���� �۽�cmd
#define SEND_VER_FILE_CMD               "B039H"     // �������� ���� �۽�cmd
#define SEND_GPS_FILE_CMD               "B173H"     // GPS ���� �۽� cmd
#define SEND_GPS_FILE_LOG_CMD           "B174H"     // GPS �α����� �۽� cmd
#define SEND_GPS_FILE_LOG2_CMD          "B186H"     // GPS �α�2���� �۽�cmd
#define SEND_TERM_LOG_CMD               "B990H"     // �α� ���� �۽�cmd
#define SEND_STATUS_LOG                 "B991H"     // ���� �α� ���� �۽�cmd

#define TR_FILE_NAME_HEADER             "B001.0."   // �ŷ��������ϸ� ���
#define EVENT_FILE_NAME_HEADERD         "B041.0."   // �����������ϸ� ���
#define SETUP_FILE_NAME_HEADER          "B071.0."   // ��ġ�������ϸ� ���
#define VER_FILE_NAME_HEADER            "B039.0."   // �����������ϸ� ���
#define GPS_FILE_NAME_HEADER            "B173.0."   // GPS ���ϸ� ���
#define GPS_FILE_LOG_NAME_HEADER        "B174.0."   // GPS�α� ���ϸ� ���
#define GPS_FILE_LOG2_NAME_HEADER       "B186.0."   // GPS�α�2 ���ϸ� ���
#define TERM_LOG_NAME_HEADER            "B990.0."   // �α� ���� ���ϸ� ���
#define STATUS_NAME_HEADER              "B991.0."   // ���� �α� ���ϸ� ���

#define PERIOD  						'.'
#define SEND_RECONCILE_FILE_CMD         "B0060"     // �������� ��û
#define UPLOAD_FILE_JOB                 "02"
#define CURR_VER                    	'0'
#define INFO_TYPE_FILE              	'0'
#define FIRST_SEND                  	'C'
#define RE_SEND                     	'R'
#define EXTENSION_LENGTH            	4
#define RECONCILE_SEND_CNT          	20
#define TRANS_FILE						".trn"

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short SendFile2DCS(  int fdSock,
                            char* pchFileName,
                            USER_PKT_MSG* pstSendUsrPktMsg,
                            USER_PKT_MSG* pstRecvUsrPktMsg ,
                            bool boolIsMoreSend,
                            char chSendStatus );

static short UpdateReconcileSendResult( char* pchFileName,
                                		char chSendStatus );

static void SetSendFilePkt(  byte* pabPeadBuff,
		                     int nReadSize,
		                     USER_PKT_MSG* pstSendUsrPktMsg,
		                     char chSendStatus );

static void SetSendFileHeaderPkt(  USER_PKT_MSG* pstSendUsrPktMsg,
		                           char* pchFileName,
		                           bool boolIsMoreSend,
		                           char chSendStatus );
		                           
static short Reconcile( int fdSock,
                        USER_PKT_MSG* pstSendUsrPktMsg,
                        USER_PKT_MSG* pstRecvUsrPktMsg );

static void SetReconcilePkt( USER_PKT_MSG* pstSendUsrPktMsg, 
							 RECONCILE_DATA*  pstReadData, 
							 int nMsgCnt );

static short SendReconcileFile( int fdSock,
                                USER_PKT_MSG* pstSendUsrPktMsg,
                                USER_PKT_MSG* pstRecvUsrPktMsg );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      Upload2DCS                                               *
*                                                                              *
*  DESCRIPTION :      ����ý������� �������� ���Ͽ� ��ϵ� ���ϵ��� ���ε��Ѵ�.  *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS				                               *
*						ErrRet( ERR_SESSION_OPEN )							   *
*						ErrRet( ERR_SESSION_CLOSE )							   *
*						ErrRet( ERR_DCS_AUTH )								   *
*						ErrRet( ERR_DCS_UPLOAD )							   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short Upload2DCS( void )
{
    short   sReturnVal      = SUCCESS;
	int     fdSock;
    int     nReconcileCnt;
    int     nFileCnt;
    int     nMsgCnt;

    RECONCILE_DATA stRecData;
    
    byte abSendBuff[2048];
    byte abRecvBuff[2048];
    USER_PKT_MSG stSendUsrPktMsg;
    USER_PKT_MSG stRecvUsrPktMsg;
    RECONCILE_DATA 	stReconcileData[RECONCILE_MAX];
	
    int     i;
    int     nSentCnt        = 0;
    bool    boolIsMoreSend;
    
    /*
     * GPS LOG ������ ����� 0�� ��� �������� ���Ͽ��� �ش� ���ڵ� ����
     */
    if ( GetFileSize( GPS_LOG_FILE ) == 0 )
    {
        DelReconcileFileList( GPS_LOG_FILE, &stRecData );
    }

    /*
     * reconcile data load
     */
    sReturnVal = LoadReconcileFileList( stReconcileData, &nReconcileCnt,  &nFileCnt, &nMsgCnt );
    if ( nFileCnt == 0 && nMsgCnt == 0 )
    {
    	printf( "[Upload2DCS] ���ε��� ������ �������� ����\n" );
        return SUCCESS;
    }

    /*
     *  ���� ����
     */
    fdSock = OpenSession( gstCommInfo.abDCSIPAddr, SERVER_COMM_PORT );
    if ( fdSock < 0 )
    {
        LogDCS( "[Upload2DCS] OpenSession() ����\n" );
		printf( "[Upload2DCS] OpenSession() ����\n" );
//        CloseSession( fdSock );
        return ErrRet( ERR_SESSION_OPEN );
    }

    stSendUsrPktMsg.pbRealSendRecvData = abSendBuff;
    stRecvUsrPktMsg.pbRealSendRecvData = abRecvBuff;
    stSendUsrPktMsg.nMaxDataSize = sizeof( abSendBuff );
    stRecvUsrPktMsg.nMaxDataSize = sizeof( abRecvBuff );

    InitPkt( &stSendUsrPktMsg );
    InitPkt( &stRecvUsrPktMsg );

    /*
     *  ���� ����
     */
    if ( AuthDCS( fdSock, UPLOAD_FILE_JOB, &stSendUsrPktMsg, &stRecvUsrPktMsg
                ) != SUCCESS )
    {
        LogDCS( "[Upload2DCS] AuthDCS() ����\n" );
		printf( "[Upload2DCS] AuthDCS() ����\n" );
        CloseSession( fdSock );
		DisplayASCInDownFND( FND_READY_MSG );	// FND �ʱ�ȭ
        return ErrRet( ERR_DCS_AUTH );
    }

	/*
	 *	�������Ͽ� ��ϵ� ���ڵ�� ��ŭ ���� �۽�
	 */
    for ( i = 0 ; i < nReconcileCnt ; i++ )
    {

        if ( stReconcileData[i].bSendStatus != RECONCILE_SEND_WAIT     &&
             stReconcileData[i].bSendStatus != RECONCILE_RESEND_REQ    &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_SETUP    &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_ERR_LOG  &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_VERSION  &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_GPS      &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_GPSLOG   &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_GPSLOG2  &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_TERM_LOG &&
             stReconcileData[i].bSendStatus != RECONCILE_SEND_STATUS_LOG )
        {
            continue;
        }

        nSentCnt++;

        if ( nSentCnt == nFileCnt )     /* EOT next send File does not exist */
        {
            boolIsMoreSend = FALSE;
        }
        else                            /* EOF next send File exists */
        {
            boolIsMoreSend = TRUE;
        }

        sReturnVal = SendFile2DCS ( fdSock,
                                    stReconcileData[i].achFileName,
                                    &stSendUsrPktMsg,
                                    &stRecvUsrPktMsg,
                                    boolIsMoreSend,
                                    stReconcileData[i].bSendStatus );

		if ( sReturnVal != SUCCESS )
		{
			LogDCS( "[Upload2DCS] SendFile2DCS() ����\n" );
			printf( "[Upload2DCS] SendFile2DCS() ����\n" );
		}

        if ( boolIsMoreSend == FALSE )
        {
            break;
        }

    }

	/*
	 *	���ε� ����� �������� ���Ͽ� �ݿ�
	 */
    sReturnVal = Reconcile( fdSock, &stSendUsrPktMsg, &stRecvUsrPktMsg );
    if ( sReturnVal < SUCCESS )
    {
        return ErrRet( ERR_DCS_UPLOAD );
    }

	CloseSession( fdSock );
	DisplayASCInDownFND( FND_READY_MSG );		// FND �ʱ�ȭ

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendFile2DCS                                             *
*                                                                              *
*  DESCRIPTION :      ����ý������� �ش� ������ �۽��Ѵ�.                       *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       char* pchFileName,                                     *
*                       USER_PKT_MSG* stSendUsrPktMsg,                         *
*                       USER_PKT_MSG* stRecvUsrPktMsg ,                        *
*                       bool boolIsMoreSend,                                   *
*                       char chSendStatus                                      *
*                                                                              *
*  RETURN/EXIT VALUE:   sReturnVal                                             *
*                       ErrRet( sReturnVal )                                   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SendFile2DCS(  int fdSock,
                            char* pchFileName,
                            USER_PKT_MSG* pstSendUsrPktMsg,
                            USER_PKT_MSG* pstRecvUsrPktMsg ,
                            bool boolIsMoreSend,
                            char chSendStatus )
{

    short   sReturnVal                  = SUCCESS;
    int     fdFile;
    byte    abReadBuff[MAX_PKT_SIZE]    = { 0, };
    int     nReadSize;

	printf( "[SendFile2DCS] ����� ���ε� : [%s] ", pchFileName );

    /*
     *  ���� ���� ���� üũ�Ͽ� ������ �α׸� �����.
     */
    sReturnVal = access( pchFileName, F_OK );

    if ( sReturnVal != 0 &&
         ( chSendStatus == RECONCILE_SEND_WAIT ||
           chSendStatus == RECONCILE_RESEND_REQ ) )
    {
    	printf( "e1\n" );
        ctrl_event_info_write( TRFILE_EXIST_ERROR_EVENT );
        return ErrRet( ERR_TRFILE_NOT_EXIST );
    }

    /*
     *  TR File�� ��� �������۽ð��� update�Ѵ�.
     */
    if ( chSendStatus == RECONCILE_SEND_WAIT ||
         chSendStatus == RECONCILE_RESEND_REQ )
    {
        UpdateTransDCSRecvDtime( pchFileName );
    }

    fdFile = open( pchFileName, O_RDWR, OPENMODE );

    if ( fdFile < 0  )
    {
    	printf( "e2\n" );
        return ErrRet( ERR_FILE_OPEN | GetFileNo( pchFileName ) );
    }

    /*
     * �۽��� ������ ����� ����
     */
    SetSendFileHeaderPkt( pstSendUsrPktMsg,
                          pchFileName,
                          boolIsMoreSend,
                          chSendStatus );

    if ( SendUsrPkt2DCS( fdSock,
                         TIMEOUT,
                         MAX_RETRY_COUNT,
                         pstSendUsrPktMsg )
        != SUCCESS )
    {
    	printf( "e3\n" );
        sReturnVal = ERR_SOCKET_SEND;
        goto end;
    }

    if ( RecvUsrPktFromDCS( fdSock,
                            TIMEOUT,
                            MAX_RETRY_COUNT,
                            pstRecvUsrPktMsg  )
        != SUCCESS )
    {
    	printf( "e4\n" );
        sReturnVal = ERR_SOCKET_RECV;
        goto end;
    }

    /*
     * ���� ������ �۽�
     */
    while( TRUE )
    {
    	printf( "." );
		fflush( stdout );

        nReadSize = read( fdFile, abReadBuff, MAX_PKT_SIZE );

        if ( nReadSize == 0 )
        {
            break;
        }

        /*
         *  ���� ������ ����
         */
        SetSendFilePkt( abReadBuff, nReadSize, pstSendUsrPktMsg, chSendStatus );

        sReturnVal = SendNRecvUsrPkt2DCS( fdSock,
                                          pstSendUsrPktMsg,
                                          pstRecvUsrPktMsg );

        if ( sReturnVal != SUCCESS )
        {
        	printf( "e5\n" );
            sReturnVal = ERR_SEND_FILE;
            goto end;
        }

    }

    /*
     *  EOF �۽�
     */
    if ( SendEOF( fdSock, pstSendUsrPktMsg ) != SUCCESS )
    {
       	printf( "e6\n" );
        sReturnVal = ERR_SEND_FILE;
        goto end;
    }

    sReturnVal = RecvACK( fdSock, pstRecvUsrPktMsg );
	if ( sReturnVal != SUCCESS )
	{
       	printf( "e7\n" );
	}

end :
    close( fdFile );

    if ( sReturnVal != SUCCESS )
    {
        return ErrRet( sReturnVal );
    }

    sReturnVal = UpdateReconcileSendResult( pchFileName, chSendStatus );

	printf( "/\n" );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      UpdateReconcileSendResult                                *
*                                                                              *
*  DESCRIPTION :      �۽��� ����� �����������Ͽ� ������Ʈ �Ѵ�.                *
*                                                                              *
*  INPUT PARAMETERS:    char* pchFileName,									   *
*                  		char chSendStatus                                      *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*						ErrRet( ERR_UPDATE_SEND_RESULT )					   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short UpdateReconcileSendResult( char* pchFileName,
                                		char chSendStatus )
{
    short   			sRetVal 		= SUCCESS;
    RECONCILE_DATA      stRecFileData;

	/*
	 *	�������� ���� read
	 */
    sRetVal = ReadReconcileFileList( pchFileName, &stRecFileData );

    if ( sRetVal != SUCCESS )
    {
        return ErrRet( ERR_UPDATE_SEND_RESULT );
    }

	/*
	 *	�ŷ����� ������ ��� ���ε� ����� ������Ʈ�ϰ�
	 *  ������ ���ϵ��� �������� ��Ͽ��� �����Ѵ�.
	 */
    switch ( chSendStatus )
    {
        case RECONCILE_SEND_WAIT:                   // 0
        case RECONCILE_RESEND_REQ:                  // 3
            if ( stRecFileData.bSendStatus  == RECONCILE_SEND_WAIT )
            {
                stRecFileData.bSendStatus  = RECONCILE_SEND_COMP;
            }
            else if ( stRecFileData.bSendStatus  == RECONCILE_RESEND_REQ )
            {
                stRecFileData.bSendStatus  = RECONCILE_RESEND_COMP;
            }

            stRecFileData.nSendSeqNo++;

            sRetVal = UpdateReconcileFileList( pchFileName, &stRecFileData );
            break;

        case RECONCILE_SEND_GPS:
        case RECONCILE_SEND_VERSION:
        case RECONCILE_SEND_ERR_LOG:
        case RECONCILE_SEND_GPSLOG:
        case RECONCILE_SEND_GPSLOG2:
        case RECONCILE_SEND_TERM_LOG:
        case RECONCILE_SEND_STATUS_LOG:
            sRetVal = DelReconcileFileList( pchFileName, &stRecFileData );
            unlink ( pchFileName );
            break;

        case RECONCILE_SEND_SETUP:
            sRetVal = DelReconcileFileList( pchFileName, &stRecFileData );
            break;
    }

    if ( sRetVal != SUCCESS )
    {
        return ErrRet( ERR_UPDATE_SEND_RESULT );
    }
    return ErrRet( sRetVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetSendFilePkt                                           *
*                                                                              *
*  DESCRIPTION :      �۽��� ������ ��Ŷ�� �����Ѵ�.                             *
*                                                                              *
*  INPUT PARAMETERS:  byte* pabPeadBuff,									   *
*                     int nReadSize,									   	   *
*                     USER_PKT_MSG* pstSendUsrPktMsg,						   *
*                     char chSendStatus                                        *
*                                                                              *
*  RETURN/EXIT VALUE:     void                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void SetSendFilePkt( byte* pabPeadBuff,
                     int nReadSize,
                     USER_PKT_MSG* pstSendUsrPktMsg,
                     char chSendStatus )
{

    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    pstSendUsrPktMsg->lDataSize = nReadSize;
    memcpy( pstSendUsrPktMsg->pbRealSendRecvData,
            pabPeadBuff,
            nReadSize );

    switch ( chSendStatus  )
    {
        case RECONCILE_SEND_WAIT:                   // 0
        case RECONCILE_RESEND_REQ:                  // 3
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_TR_FILE_CMD,
                    COMMAND_LENGTH );
            break;
        case RECONCILE_SEND_ERR_LOG:                // 8
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_EVENT_FILE_CMD,
                    COMMAND_LENGTH );
            break;
        case RECONCILE_SEND_SETUP:                  // 7
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_SETUP_FILE_CMD,
                    COMMAND_LENGTH );
            break;
        case RECONCILE_SEND_VERSION:                // 9
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_VER_FILE_CMD,
                    COMMAND_LENGTH );
            break;
        case RECONCILE_SEND_GPS:                    // a
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_GPS_FILE_CMD,
                    COMMAND_LENGTH );
            break;
        case RECONCILE_SEND_GPSLOG:                 // b
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_GPS_FILE_LOG_CMD,
                    COMMAND_LENGTH );
            break;
        case RECONCILE_SEND_GPSLOG2:                // c
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_GPS_FILE_LOG2_CMD,
                    COMMAND_LENGTH );
        case RECONCILE_SEND_TERM_LOG:               // d
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_TERM_LOG_CMD,
                    COMMAND_LENGTH );
        case RECONCILE_SEND_STATUS_LOG:             // e
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_STATUS_LOG,
                    COMMAND_LENGTH );
            break;
    }
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetSendFileHeaderPkt                                     *
*                                                                              *
*  DESCRIPTION :      �۽��� ������ ��� ��Ŷ�� �����Ѵ�.                        *
*                                                                              *
*  INPUT PARAMETERS:    USER_PKT_MSG* stSendUsrPktMsg,                         *
*                       char* pchFileName,                                     *
*                       bool boolIsMoreSend,                                   *
*                       char chSendStatus                                      *
*                                                                              *
*  RETURN/EXIT VALUE:     void                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void SetSendFileHeaderPkt(  USER_PKT_MSG* pstSendUsrPktMsg,
                            char* pchFileName,
                            bool boolIsMoreSend,
                            char chSendStatus )
{

    long    lFileSize;
    time_t  tCurrDtime;
    char    aBCDCurrDtime[7]    = { 0, };
    long    lTmpPktCnt;
    char    chFileSize[5]       = { 0, };

    PKT_HEADER_INFO     stPktHeaderInfo;

    /*
     *  ���� ������ ����
     */
    lFileSize = GetFileSize( pchFileName );

    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    pstSendUsrPktMsg->lDataSize = sizeof( PKT_HEADER_INFO );

    /*
     *  ��Ŷ ��� ����
     */
    memset( &stPktHeaderInfo, 0, sizeof( PKT_HEADER_INFO ) );

    stPktHeaderInfo.bNextSendFileExistYN = GetASCNoFromDEC( boolIsMoreSend );
    memcpy( stPktHeaderInfo.abVehicleID,
            gstVehicleParm.abVehicleID,
            sizeof( gstVehicleParm.abVehicleID ) ); // vehicle ID
    stPktHeaderInfo.bDataType = INFO_TYPE_FILE;     // Data Tpye - file
    stPktHeaderInfo.bNewVerYN = CURR_VER;           // File Version Type

    /*
     * command and fileNameHeader ����
     */
    switch ( chSendStatus )
    {
        case RECONCILE_SEND_WAIT:                   // 0
        case RECONCILE_RESEND_REQ:                  // 3
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_TR_FILE_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    TR_FILE_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_ERR_LOG:
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_EVENT_FILE_CMD,
                    COMMAND_LENGTH );               // error  information file
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    EVENT_FILE_NAME_HEADERD,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_SETUP:                  // setup  information file
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_SETUP_FILE_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    SETUP_FILE_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_VERSION:               // version file
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_VER_FILE_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    VER_FILE_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_GPS:                    // GPS
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_GPS_FILE_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    GPS_FILE_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_GPSLOG:                 // GPS Log
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_GPS_FILE_LOG_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    GPS_FILE_LOG_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_GPSLOG2:                // GPS Log2
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_GPS_FILE_LOG2_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    GPS_FILE_LOG2_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_TERM_LOG:               // TERMINAL LOG
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_TERM_LOG_CMD,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    TERM_LOG_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;
        case RECONCILE_SEND_STATUS_LOG:             // TERMINAL STATUS
            memcpy( pstSendUsrPktMsg->achConnCmd,
                    SEND_STATUS_LOG,
                    COMMAND_LENGTH );
            memcpy( &stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader,
                    STATUS_NAME_HEADER,
                    sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameHeader )
                  );
            break;

    }

    /*
     *  DCS ���ϸ� ����
     */
    switch ( chSendStatus )
    {
        case RECONCILE_SEND_WAIT:
        case RECONCILE_RESEND_REQ:
            memcpy( &stPktHeaderInfo.stDCSFileName.abTranspBizrID,
                    gstVehicleParm.abTranspBizrID,
                    sizeof( stPktHeaderInfo.stDCSFileName.abTranspBizrID ) );
            stPktHeaderInfo.stDCSFileName.bSeperator1 = PERIOD;
            memcpy( stPktHeaderInfo.stDCSFileName.abMainTermID,
                    gpstSharedInfo->abMainTermID,
                    sizeof( stPktHeaderInfo.stDCSFileName.abMainTermID ) );
            stPktHeaderInfo.stDCSFileName.bSeperator2 = PERIOD;
            memcpy( &stPktHeaderInfo.stDCSFileName.abFileName,
                    pchFileName,
                    sizeof( stPktHeaderInfo.stDCSFileName.abFileName ) );

            memset ( &stPktHeaderInfo.stDCSFileName.abDCSFileNameTail,
                     SPACE,
                     sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameTail ));

            stPktHeaderInfo.stDCSFileName.abDCSFileNameTail[0] = PERIOD;

            if ( chSendStatus == RECONCILE_SEND_WAIT )
            {
                stPktHeaderInfo.stDCSFileName.abDCSFileNameTail[1] = FIRST_SEND;
            }
            else if ( chSendStatus == RECONCILE_RESEND_REQ )
            {
                stPktHeaderInfo.stDCSFileName.abDCSFileNameTail[1] = RE_SEND;
            }
            break;

        case RECONCILE_SEND_ERR_LOG:
        case RECONCILE_SEND_SETUP:              // setup  information file
        case RECONCILE_SEND_VERSION:            // version file
        case RECONCILE_SEND_GPS:
        case RECONCILE_SEND_GPSLOG:
        case RECONCILE_SEND_GPSLOG2:
        case RECONCILE_SEND_TERM_LOG:           // TERMINAL LOG
        case RECONCILE_SEND_STATUS_LOG:         // TERMINAL STATUS
            memcpy( &stPktHeaderInfo.stDCSFileName.abTranspBizrID,
                    gstVehicleParm.abTranspBizrID,
                    sizeof( stPktHeaderInfo.stDCSFileName.abTranspBizrID ) );
            stPktHeaderInfo.stDCSFileName.bSeperator1 = PERIOD;
            memcpy( stPktHeaderInfo.stDCSFileName.abMainTermID,
                    gpstSharedInfo->abMainTermID,
                    sizeof( stPktHeaderInfo.stDCSFileName.abMainTermID ) );
            stPktHeaderInfo.stDCSFileName.bSeperator2 = PERIOD;
            GetRTCTime( &tCurrDtime );
            TimeT2ASCDtime( tCurrDtime,
                            stPktHeaderInfo.stDCSFileName.abFileName );
            memset ( &stPktHeaderInfo.stDCSFileName.abDCSFileNameTail,
                     SPACE,
                     sizeof( stPktHeaderInfo.stDCSFileName.abDCSFileNameTail ));
            break;
    }

    TimeT2BCDDtime( tCurrDtime, aBCDCurrDtime );
    memcpy( stPktHeaderInfo.abFileVer,
            aBCDCurrDtime,
            sizeof( stPktHeaderInfo.abFileVer ) );  // file version

    DWORD2ASC( lFileSize, chFileSize );
    memcpy( stPktHeaderInfo.abFileSize,
            chFileSize,
            sizeof( stPktHeaderInfo.abFileSize ) ); // file size

    lTmpPktCnt = lFileSize / MAX_PKT_SIZE;
    if ( lFileSize % MAX_PKT_SIZE )
    {
        lTmpPktCnt++;
    }

    memcpy( stPktHeaderInfo.abTotalPktCnt,
            (char*)&lTmpPktCnt,
            sizeof( stPktHeaderInfo.abTotalPktCnt ) );  // tatal pkt cnt
    memcpy( stPktHeaderInfo.abFileCheckSum, "00", 2 );  // Reserved
    memset( stPktHeaderInfo.abFirstPktNo,
            0x00,
            sizeof( stPktHeaderInfo.abFirstPktNo ) );   // send start Packet NO
    memcpy( stPktHeaderInfo.achSendDtime,
            aBCDCurrDtime,
            sizeof( stPktHeaderInfo.achSendDtime ) );   // send time

    memcpy( pstSendUsrPktMsg->pbRealSendRecvData,
            &stPktHeaderInfo,
            sizeof( PKT_HEADER_INFO ) );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      Reconcile                                                *
*                                                                              *
*  DESCRIPTION :      ����� �������ϳ����� �۽��ϰ� ��������� ������ �����Ѵ�.  *
*                                                                              *
*  INPUT PARAMETERS:  	int fdSock, 										   *
*						USER_PKT_MSG* stSendUsrPktMsg,               		   *
*             			USER_PKT_MSG* stRecvUsrPktMsg                          *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*						ERR_FILE_READ | GetFileNo( RECONCILE_FILE )			   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short Reconcile( int fdSock,
                        USER_PKT_MSG* pstSendUsrPktMsg,
                        USER_PKT_MSG* pstRecvUsrPktMsg )
{
    short   sReturnVal          = SUCCESS;
    int     fdReconFile;                    // �������� ����
    int     nReadByte;
    int     nMsgCnt             = 0;        // 20���� ©�� ������ ����

    RECONCILE_DATA  stReadData;                 // �������� ���� ���� ������
    time_t          tCurrTime;
    time_t          tCommTime;

    InitPkt( pstSendUsrPktMsg );

    /*
     *  �������� ������ reconcileinfo.dat.tmp�� copy�Ͽ� �۾��Ѵ�.
     */
    CopyReconcileFile( RECONCILE_TMP_FILE );

    fdReconFile = open( RECONCILE_TMP_FILE, O_RDWR, OPENMODE );
    if ( fdReconFile < 0 )
    {
        goto end;
    }

    while( TRUE )
    {
        nReadByte = read( fdReconFile,
                          (void*)&stReadData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            if ( nMsgCnt != 0 )
            {
                sReturnVal = SendReconcileFile( fdSock, pstSendUsrPktMsg,
					pstRecvUsrPktMsg );
				if ( sReturnVal != SUCCESS )
				{
					LogDCS( "[Reconcile] SendReconcileFile() 1 ����\n" );
					printf( "[Reconcile] SendReconcileFile() 1 ����\n" );
				}
            }
            break;
        }
        else if ( nReadByte < 0 )
        {
            if ( nMsgCnt != 0 )
            {
                sReturnVal = SendReconcileFile( fdSock, pstSendUsrPktMsg,
					pstRecvUsrPktMsg );
				if ( sReturnVal != SUCCESS )
				{
					LogDCS( "[Reconcile] SendReconcileFile() 2 ����\n" );
					printf( "[Reconcile] SendReconcileFile() 2 ����\n" );
				}
            }
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        /*
         * ���ð� ���� ���� 5�ñ����� �����ʹ� �۽����� �ʴ´�.
         */
        tCommTime = stReadData.tWriteDtime -
                    ( stReadData.tWriteDtime % ( 60 * 60 * 24 ) ) +
                    ( 60 * 60 * 24 ) + ( 60 * 60 * 5 ) - (9 * 60 * 60);
		// next day 5 o'clock

        GetRTCTime( &tCurrTime );
        if ( tCurrTime < tCommTime )
        {
            continue;
        }
        else
        {
			stReadData.tWriteDtime = tCurrTime;
            UpdateReconcileFileList( stReadData.achFileName, &stReadData );
        }

        nMsgCnt++;
        SetReconcilePkt( pstSendUsrPktMsg, &stReadData, nMsgCnt );

        /*
         *  �������� �����Ͱ� 20���� �Ǹ� ����ý������� �۽�
         */
        if ( nMsgCnt == RECONCILE_SEND_CNT )     // 20
        {
            sReturnVal = SendReconcileFile( fdSock, pstSendUsrPktMsg,
				pstRecvUsrPktMsg );
			if ( sReturnVal != SUCCESS )
			{
				LogDCS( "[Reconcile] SendReconcileFile() 3 ����\n" );
				printf( "[Reconcile] SendReconcileFile() 3 ����\n" );
			}
			nMsgCnt = 0;
        }

    }

    close( fdReconFile );

    end:

    if ( SendEOS( fdSock, pstSendUsrPktMsg ) != SUCCESS )
    {
		LogDCS( "[Reconcile] SendEOS() ����\n" );
		printf( "[Reconcile] SendEOS() ����\n" );
    }

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetReconcilePkt                                          *
*                                                                              *
*  DESCRIPTION :      ����� ������ �������� �޼����� �����Ѵ�.                  *
*                                                                              *
*  INPUT PARAMETERS:  USER_PKT_MSG* pstSendUsrPktMsg, 						   *
*					  RECONCILE_DATA*  pstReadData, 						   *
*					  int nMsgCnt                                              *
*                                                                              *
*  RETURN/EXIT VALUE:   void                                                   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void SetReconcilePkt( USER_PKT_MSG* pstSendUsrPktMsg, 
							 RECONCILE_DATA*  pstReadData, 
							 int nMsgCnt )
{
    CONN_FILE_NAME  stConnFileName;         // ����� �۽ŵ� ���ϸ�
    char    achReconFileCnt[2]  = { 0, };   // ����ý������� ���� �������� ����
    byte abTempBuf[11] = {0, };

    /*
     *  reconcile message ����
     */
    memset( &stConnFileName, 0x00, sizeof( stConnFileName ) );

    memcpy( stConnFileName.achFileNameHeader,
            TR_FILE_NAME_HEADER,
            sizeof( stConnFileName.achFileNameHeader ) );
    memcpy( stConnFileName.abTranspBizrID,
            gstVehicleParm.abTranspBizrID,
            sizeof( stConnFileName.abTranspBizrID ) );
    stConnFileName.chClass1 = PERIOD;
    memcpy( stConnFileName.abMainTermID,
            gpstSharedInfo->abMainTermID,
            sizeof( stConnFileName.abMainTermID ) );
    stConnFileName.chClass2 = PERIOD;
    memcpy( stConnFileName.achFileName,
            pstReadData->achFileName,
            sizeof( stConnFileName.achFileName ) );
    memset( stConnFileName.achSpace,
            SPACE,
            sizeof( stConnFileName.achSpace ) );

    sprintf( abTempBuf, "%02d", nMsgCnt );
    memcpy( achReconFileCnt, abTempBuf, sizeof( achReconFileCnt ) );

    memcpy( pstSendUsrPktMsg->pbRealSendRecvData,
            achReconFileCnt,
            sizeof( achReconFileCnt ) );

    memcpy( &(pstSendUsrPktMsg->pbRealSendRecvData[sizeof( achReconFileCnt )+
							((sizeof( stConnFileName )+1)*(nMsgCnt-1))]),
            &stConnFileName,
            sizeof( stConnFileName ) );

	// ���� '0'�� ���̴�.
	pstSendUsrPktMsg->pbRealSendRecvData[( sizeof( achReconFileCnt )+
										   sizeof( stConnFileName ))+
							((sizeof( stConnFileName )+1)*(nMsgCnt-1))] = ZERO;
										    
    pstSendUsrPktMsg->lDataSize = ( nMsgCnt * ( sizeof( stConnFileName ) + 1 ) )
                                        + ( sizeof( achReconFileCnt )  );

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendReconcileFile                                        *
*                                                                              *
*  DESCRIPTION :      �������� ��Ŷ�� �۽��ϰ� ����ý������� ���� �����䱸�� ����*
*                     ��� �������� ���Ϸκ��� �ش� ���ڵ带 �����Ѵ�.           *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       USER_PKT_MSG* pstSendUsrPktMsg,                        *
*                       USER_PKT_MSG* pstRecvUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*						ErrRet( ERR_SEND_RECONCILE )						   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SendReconcileFile( int fdSock,
                                USER_PKT_MSG* pstSendUsrPktMsg,
                                USER_PKT_MSG* pstRecvUsrPktMsg )
{
    short   sRetVal                 = SUCCESS;
    int     nReconCnt               = 0;
    int     i;
    char    achReconFileName[19]    = { 0, };
    byte    bSendStatus;
    RECONCILE_DATA          	stReconcileData;
    RECONCILE_RESULT_PKT*    	pstReconResultPkt;

    /*
     *  �������� ���� �۽�
     */
    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    memcpy( pstSendUsrPktMsg->achConnCmd,
            SEND_RECONCILE_FILE_CMD,
            COMMAND_LENGTH );

    sRetVal = SendNRecvUsrPkt2DCS( fdSock, pstSendUsrPktMsg, pstRecvUsrPktMsg );

    if ( sRetVal != SUCCESS )
    {
        return ErrRet( ERR_SEND_RECONCILE );
    }

    /*
     *  ����κ��� ���� ����Ǽ� ����
     */
    pstReconResultPkt = (RECONCILE_RESULT_PKT*)
    						pstRecvUsrPktMsg->pbRealSendRecvData;
    nReconCnt = GetINTFromASC( pstReconResultPkt->achReconDataCnt,
                               sizeof( pstReconResultPkt->achReconDataCnt ) );

    /*
     *  ����κ��� ���� �������� ����� �ݿ��Ѵ�.
     */
    for ( i = 0 ; i < nReconCnt ; i++ )
    {
        /*
         *  ���ϸ� ����
         */
        memset( achReconFileName, 0x00, sizeof( achReconFileName ) );
        memcpy( achReconFileName,
                &(pstReconResultPkt->stReconDtailResultList[i].stReconFileName.achFileName ),
                VER_INFO_LENGTH );

        /*
         *  ������ ������ .trn�� �ٿ��� üũ
         */
        sRetVal = access( achReconFileName, F_OK );

        if ( sRetVal != SUCCESS )   // ������ ���� ���
        {
            memcpy( &achReconFileName[VER_INFO_LENGTH],
                    TRANS_FILE,
                    EXTENSION_LENGTH );
            achReconFileName[18] = 0x00;
        }

        sRetVal = ReadReconcileFileList( achReconFileName,
                                         &stReconcileData );

        /*
         *  �ش� ���ڵ尡 ������ continue
         */
        if ( sRetVal != SUCCESS )
        {
            continue;
        }

        bSendStatus = pstReconResultPkt->
        				stReconDtailResultList[i].chReconResult;

        /*
         * �۽� �Ϸ� �� ��쿡 �����������Ͽ��� �ش� ���ڵ带 �����ϰ�
         * ��۽� ��û�� ��� �������� ���Ͽ��� �ش� ���ڵ带 �������ش�.
         */
        if ( stReconcileData.bSendStatus == RECONCILE_SEND_COMP ||
             stReconcileData.bSendStatus == RECONCILE_RESEND_COMP ||
             stReconcileData.bSendStatus == RECONCILE_RESULT_RESEND )
        {
            switch ( bSendStatus )
            {
                case RECONCILE_RESULT_DEL:
                case RECONCILE_RESULT_ERR_DEL:
					printf("[SendReconcileFile] ���ϻ��� : [%s]\n", achReconFileName);
                    DelReconcileFileList( achReconFileName,
                                          &stReconcileData );
                    unlink ( achReconFileName );
                    break;

                case RECONCILE_RESULT_RESEND:
                    stReconcileData.bSendStatus = RECONCILE_RESEND_REQ;
                    UpdateReconcileFileList( achReconFileName,
                                             &stReconcileData );
                    break;
            }
        }

    }

    return SUCCESS;

}
