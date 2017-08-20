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
*  PROGRAM ID :       dcs_comm.c	                                           *
*                                                                              *
*  DESCRIPTION:       ����ý��۰��� ����� ���� �Լ����� �����Ѵ�.            *
*                                                                              *
*  ENTRY POINT:     void InitPkt( USER_PKT_MSG* stUserMsg );				   *
*					short SendUsrPkt2DCS( 	int fdSock,						   *
*                             				int nTimeOut,					   *
*                             				int nRetryCnt,					   *
*                             				USER_PKT_MSG* stUsrPktMsg );	   *
*					short RecvUsrPktFromDCS( int fdSock,					   *
*                                			 int nTimeOut,					   *
*                                			 int nRetryCnt,	 				   *
*                                			 USER_PKT_MSG* stUsrPktMsg );	   *
*					int OpenSession( char *pchIP, char *pchPort );			   *
*					short CloseSession( int fdSock );	  					   *
*					short AuthDCS( 	int fdSock,								   *
*                      				char* pchSessionCd,						   *
*                      				USER_PKT_MSG* pstSendUsrPktMsg,	 		   *
*                      				USER_PKT_MSG* pstRecvUsrPktMsg );		   *
*					short SendNRecvUsrPkt2DCS( int fdSock,					   *
*                          					   USER_PKT_MSG* pstSendUsrPktMsg, *
*                          					   USER_PKT_MSG* pstRecvUsrPktMsg )*
*					short SendRS( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg );*
*					short SendEOS( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg )*
*					short SendEOF( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg )*
*					short RecvRS( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg );*
*					short RecvACK( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg )*
*					short GetDownFileIndex( char* pchCmd );					   *
*					void GetRecvFileName( USER_PKT_MSG* pstRecvUsrPktMsg,	   *
*                             			  PKT_HEADER_INFO*   pstPktHeaderInfo, *
*                             			  char*  achRecvFileName );			   *
*					short ReSetupTerm( void );								   *
*					short SetupTerm( void );								   *
*					void *DCSComm( void *arg );								   *
*                                                                              *
*  INPUT FILES:     None                                                       *
*                                                                              *
*  OUTPUT FILES:      c_op_par.dat - ���������Ķ���� ����                     *
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
#include "../system/device_interface.h"
#include "../proc/file_mgt.h"
#include "../proc/main.h"
#include "des.h"
#include "wlan.h"
#include "socket_comm.h"
#include "dcs_comm.h"
#include "download_dcs_comm.h"
#include "upload_dcs_comm.h"
#include "../proc/reconcile_file_mgt.h"
#include "../proc/load_parameter_file_mgt.h"
#include "../proc/upload_file_mgt.h"
#include "../proc/version_file_mgt.h"
#include "../proc/version_mgt.h"
#include "../proc/main_process_busTerm.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
#define DCS_AUTH_CMD                    "ENQ00"		// ������û cmd
#define DCS_AUTH_CMD1                   "AA000"		// ���� �ʱ� key
#define DCS_AUTH_CMD2                   "AB000"		// Ŭ���̾�Ʈ ���� key
#define DCS_AUTH_CMD3                   "AC000"		// ���� ���� key

#define RESPONSE_CMD                    "RS000"
#define ACK_CMD                         "ACK00"
#define RESPONSE_STATUS_MSG             "0000"
#define LEN_UNTIL_REAL_SEND_DATA    	19

#define DEC_CLIENT_RAND_NO          	"9876543213333333"
#define CHILD_DEFAULT_ID            	"0000000000"

#define MASTER_BL_BACKUP_FILE           "c_fi_bl.backup"
#define MASTER_PREPAY_PL__BACKUP_FILE   "c_fa_pl.backup"
#define MASTER_POSTPAY_PL__BACKUP_FILE  "c_fd_pl.backup"
#define MASTER_AI_BACKUP_FILE           "c_fi_ai.backup"
#define KPDAPPLY_FLAG_BACKUP_FILE       "driverdn.backup"

#define RANDOM_NO_LENGTH               	16
#define KEY_LENGTH                  	16

byte gbDCSCommType = 0;						// �����������
											// 1: ������������ �ٿ�ε�
											// 2: ���� ���ε�
											// 3: ���� �ٿ�ε�
char    achDCSCurrDtime[15];				// ����PC �ð�
static  long    lSendSeq2DCS = -1;
static  byte    abRecvSeqNo[5];
static  int     nTermIDLength = sizeof( gpstSharedInfo->abMainTermID );
static  int     nPSAMIDLength = sizeof( gpstSharedInfo->abMainPSAMID );
static  int     nPSAMIDBCDLength    = sizeof( gpstSharedInfo->abMainPSAMID )/2;

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short SendPkt2DCS( int fdSock, int nTimeOutSec, int nRetryCnt,
	PKT_MSG* stPktMsg );
static short RecvPktFromDCS( int fdSock, int nTimeOutSec, int nRetryCnt,
	PKT_MSG* stPktMsg );

static short DisplayDCSCommCnt( int nDCSCommCnt );

static bool IsValidACAuthMsg( USER_PKT_MSG* stRecvUsrPktMsg, 
							  int* pnSecondKeyIdx );

static void SetABAuthMsg( USER_PKT_MSG* stSendUsrPktMsg,
		                  USER_PKT_MSG* stRecvUsrPktMsg,
		                  char* pchSessionCd, int* pnSecondKeyIdx );

static short ReqDCSAuth( int fdSock,
		                 USER_PKT_MSG* stSendUsrPktMsg,
		                 USER_PKT_MSG* stRecvUsrPktMsg );

static short GetAuthIndex( char* pchRandomNo );

static short ChkNDownOperParmFile( void );

static void InitNextVerApplFlag( void );

static void DelBackupFile( void );

static void DelFileOnReset( void );

static void ReqKeysetRegist( void );

static bool ConnDCS( void );

static void ReqApplyNextVer( byte bCmd,
		                     bool boolIsApplyNextVer,
		                     bool boolIsApplyNextVerParm,
		                     bool boolIsApplyNextVerAppl,
		                     bool boolIsApplyNextVerVoice,
		                     bool boolIsApplyNextVerDriverAppl);



/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InitPkt                                                  *
*                                                                              *
*  DESCRIPTION :      ��Ŷ�� ������ �ʱ�ȭ�Ѵ�.                                *
*                                                                              *
*  INPUT PARAMETERS:  USER_PKT_MSG* stUserMsg                                  *
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
void InitPkt( USER_PKT_MSG* stUserMsg )
{
    byte*   pbDataPtr;
    int     nMaxDataSize;

    pbDataPtr       = stUserMsg->pbRealSendRecvData;
    nMaxDataSize    = stUserMsg->nMaxDataSize;

    memset( stUserMsg, 0x00, sizeof( USER_PKT_MSG ) );

    stUserMsg->pbRealSendRecvData        = pbDataPtr;
    stUserMsg->nMaxDataSize              = nMaxDataSize;

    memset( stUserMsg->pbRealSendRecvData, 0x00, nMaxDataSize);

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendUsrPkt2DCS                                           *
*                                                                              *
*  DESCRIPTION :     USER_PKT_MSG�� PKT_MSG�� ��ȯ�� �ش� ������ ���� ��Ŷ��     *
*                       �۽��Ѵ�.                                              *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       int nTimeOut,                                          *
*                       int nRetryCnt,                                         *
*                       USER_PKT_MSG* stUsrPktMsg                              *
*                                                                              *
*  RETURN/EXIT VALUE:  SendPkt2DCS( fdSock,  nTimeOut,  nRetryCnt, &stPktMsg ) *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SendUsrPkt2DCS( int fdSock,
                      int nTimeOut,
                      int nRetryCnt,
                      USER_PKT_MSG* stUsrPktMsg )
{
    PKT_MSG     stPktMsg            = { 0, };
    dword       dwUsrMsgSize;

    lSendSeq2DCS++;

    stPktMsg.bSTX = STX;
    stPktMsg.bETX = ETX;
    stPktMsg.chEncryptionYN = stUsrPktMsg->chEncryptionYN;

    memcpy( stPktMsg.abConnSeqNo,
            (char*)&lSendSeq2DCS,
            sizeof( stPktMsg.abConnSeqNo ) );
    memcpy( stPktMsg.achConnCmd,
            stUsrPktMsg->achConnCmd,
            sizeof( stPktMsg.achConnCmd ) );

    dwUsrMsgSize = stUsrPktMsg->lDataSize;
    memcpy( stPktMsg.achDataSize,
            (char*)&dwUsrMsgSize,
            sizeof( stPktMsg.achDataSize ) );

    dwUsrMsgSize = stUsrPktMsg->lDataSize +
                  ( sizeof( PKT_MSG ) - sizeof( stPktMsg.pbRealSendRecvData ) );
    memcpy( stPktMsg.achPktSize,
            (char*)&dwUsrMsgSize,
            sizeof( stPktMsg.achPktSize ) );

    stPktMsg.pbRealSendRecvData = stUsrPktMsg->pbRealSendRecvData;

    return SendPkt2DCS( fdSock,  nTimeOut,  nRetryCnt, &stPktMsg );

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendPkt2DCS                                              *
*                                                                              *
*  DESCRIPTION :      �ش� ��Ŷ�� ���� BCC�� �����ϰ� �ش� �������� �޼����� �۽� *
*                       �Ѵ�.                                                  *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       int nTimeOut,                                          *
*                       int nRetryCnt,                                         *
*                       PKT_MSG* stPktMsg                                      *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_SEND                                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SendPkt2DCS( int fdSock, int nTimeOutSec, int nRetryCnt,
	PKT_MSG* stPktMsg )
{
    short sResult = SUCCESS;
    long lDataSize = 0;
    long lPktSize = 0;
    byte abSendBuf[2048] = {0, };

    /*
     *  ��Ŷ ��ü�� ��ü size
     */
	memcpy( (char*)&lPktSize,
		stPktMsg->achPktSize,
		sizeof( stPktMsg->achPktSize ) );

    /*
     *  ������ size
     */
	memcpy( (char*)&lDataSize,
		stPktMsg->achDataSize,
		sizeof( stPktMsg->achDataSize ) );

    memset( abSendBuf, 0x00, lPktSize );

    /*
     *  ������ �������� �������� ���� copy
     */
    memcpy( abSendBuf, (void*)&stPktMsg->bSTX, LEN_UNTIL_REAL_SEND_DATA );

    /*
     *  ������ copy
     */
	memcpy( abSendBuf + LEN_UNTIL_REAL_SEND_DATA,
		(void*)stPktMsg->pbRealSendRecvData,
		lDataSize );

    /*
     *  ������ ������ �������� ���� copy
     */
    memcpy( abSendBuf + LEN_UNTIL_REAL_SEND_DATA + lDataSize,
		(void *)&stPktMsg->bETX,
		sizeof( stPktMsg->bETX ) );

    /*
     *  BCC����
     */
    abSendBuf[lPktSize-1] = MakeBCC( &abSendBuf[1], lPktSize-3 );
	
    while ( nRetryCnt-- )
    {
        sResult = SockSendPkt( fdSock, nTimeOutSec, lPktSize, abSendBuf );
        if ( sResult == SUCCESS )
        {
            break;
        }
    }

    return sResult;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvUsrPktFromDCS                                        *
*                                                                              *
*  DESCRIPTION :      �ش� socket���κ��� ��Ŷ�� ���Ź޴´�                      *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       int nTimeOut,                                          *
*                       int nRetryCnt,                                         *
*                       USER_PKT_MSG* stUsrPktMsg                              *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                       ERR_PACKET_STX                                         *
*                       ERR_PACKET_ETX                                         *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short RecvUsrPktFromDCS( int fdSock,
                         int nTimeOut,
                         int nRetryCnt,
                         USER_PKT_MSG* stUsrPktMsg )
{
    short sResult = SUCCESS;
    PKT_MSG stPktMsg;

    stPktMsg.pbRealSendRecvData = stUsrPktMsg->pbRealSendRecvData;
    sResult = RecvPktFromDCS( fdSock, nTimeOut, nRetryCnt, &stPktMsg );
    if ( sResult < 0 )
    {
//    	printf("[RecvUsrPktFromDCS] RecvPktFromDCS() ����\n");
        return sResult;
    }

    if ( stPktMsg.bSTX != STX )
    {
    	LogDCS("[RecvUsrPktFromDCS] STX�� ���� ����\n");
    	printf("[R_NOT_STX]");
		fflush( stdout );
        return ErrRet( ERR_PACKET_STX );
    }

    if ( stPktMsg.bETX != ETX )
    {
    	LogDCS("[RecvUsrPktFromDCS] ETX�� ������ ����\n");
    	printf("[R_NOT_ETX]");
		fflush( stdout );
        return ErrRet( ERR_PACKET_ETX );
    }

    stUsrPktMsg->chEncryptionYN = stPktMsg.chEncryptionYN;
    memcpy( stUsrPktMsg->achConnCmd,
		stPktMsg.achConnCmd,
		sizeof( stPktMsg.achConnCmd ) );

	memcpy( abRecvSeqNo,
		stPktMsg.abConnSeqNo,
		sizeof( stPktMsg.abConnSeqNo ) );
	memcpy( (char*)&stUsrPktMsg->lDataSize,
		stPktMsg.achDataSize,
		sizeof( stPktMsg.achDataSize ) );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvPktFromDCS                                           *
*                                                                              *
*  DESCRIPTION :      �ش� ��Ĺ���� �޼����� ���Ź޴´�.                         *
*                                                                              *
*  INPUT PARAMETERS: int fdSock, int nTimeOut, int nRetryCnt, PKT_MSG* stPktMsg*
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_PACKET_BCC                                         *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short RecvPktFromDCS( int fdSock, int nTimeOutSec, int nRetryCnt,
	PKT_MSG* stPktMsg )
{
    short sResult = SUCCESS;
    int nPktSize = 0;
    byte abRecvBuf[MAX_RECV_PKT_SIZE] = {0, };

    while ( nRetryCnt-- )
    {
        sResult = SockRecvPkt( fdSock, nTimeOutSec, &nPktSize, abRecvBuf );
        if ( sResult == SUCCESS )
        {
            break;
        }
    }
    if ( sResult < SUCCESS )
    {
        return sResult;
    }

    /*
     *  STX �������� ETX���� ������ BCCüũ ����� �ȴ�.
     */
    if ( abRecvBuf[nPktSize-1] != MakeBCC( &abRecvBuf[1], nPktSize - 3 ) )
    {
		LogDCS( "[RecvPktFromDCS] BCC ����\n" );
        printf( "[R_BCC]" );
		fflush( stdout );
        return ErrRet( ERR_PACKET_BCC );
    }

    memcpy( &stPktMsg->bSTX, abRecvBuf, LEN_UNTIL_REAL_SEND_DATA );
    memcpy( stPktMsg->pbRealSendRecvData,
		abRecvBuf + LEN_UNTIL_REAL_SEND_DATA,
		nPktSize - 21 );

    memcpy( &stPktMsg->bETX, abRecvBuf + nPktSize - 2, 2 );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      OpenSession                                              *
*                                                                              *
*  DESCRIPTION :      ������ �����Ѵ�.                                          *
*                                                                              *
*  INPUT PARAMETERS:  char *pchIP, char *pchPort                               *
*                                                                              *
*  RETURN/EXIT VALUE:     OpenSock( pchIP, pchPort )                           *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
int OpenSession( char *pchIP, char *pchPort )
{
	/*
	 *	�۽ż���, receive���� �ʱ�ȭ
	 */
    lSendSeq2DCS = 0;
    memset( abRecvSeqNo, 0, sizeof( abRecvSeqNo ) );

    return OpenSock( pchIP, pchPort );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CloseSession                                             *
*                                                                              *
*  DESCRIPTION :      ������ �ݴ´�.                                            *
*                                                                              *
*  INPUT PARAMETERS:  int fdSock                                               *
*                                                                              *
*  RETURN/EXIT VALUE:     CloseSock( fdSock )                                  *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short CloseSession( int fdSock )
{
	short sResult = SUCCESS;

    lSendSeq2DCS = -1;
    memset( abRecvSeqNo, 0, sizeof( abRecvSeqNo ) );

	sResult = CloseSock( fdSock );

	sleep( 1 );

    return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DisplayDCSCommCnt                                        *
*                                                                              *
*  DESCRIPTION :      �������Ƚ���� FND�� ���÷��� �Ѵ�.                      *
*                                                                              *
*  INPUT PARAMETERS:  int nDCSCommCnt                                          *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short DisplayDCSCommCnt( int nDCSCommCnt )
{
    int     nRecCnt         = 0;                    // ��ü ���ڵ� ��
    int     nSendWaitCnt    = 0;                    // �۽��� ���� ��

    char    achDCSCommCnt[7]    = { 0, };
    char    achReconcileCnt[7]  = { 0, };

    nSendWaitCnt = GetReconcileCnt( &nRecCnt );     // �۽������ϼ� /���ڵ� ��

    if ( nSendWaitCnt < 0 )
    {
        nRecCnt = 0;
        nSendWaitCnt = 0;
    }

    sprintf( achDCSCommCnt, "%6d", nDCSCommCnt );
    sprintf( achReconcileCnt, "%03d%03d", nRecCnt, nSendWaitCnt );

    DisplayDWORDInUpFND( GetDWORDFromASC( achReconcileCnt,
                                          sizeof( achReconcileCnt ) -1 ) );
    usleep( 20000 );

    DisplayASCInDownFND( achDCSCommCnt );

    return SUCCESS;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      AuthDCS                                                  *
*                                                                              *
*  DESCRIPTION :      ����ý������� ������ �޴´�.                              *
*                                                                              *
*  INPUT PARAMETERS:  int fdSock,                                              *
*                     char* pchSessionCd,                                      *
*                     USER_PKT_MSG* stSendUsrPktMsg,                           *
*                     USER_PKT_MSG* stRecvUsrPktMsg                            *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                         ERR_DCS_AUTH                                         *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short AuthDCS( int fdSock,
               char* pchSessionCd,
               USER_PKT_MSG* pstSendUsrPktMsg,
               USER_PKT_MSG* pstRecvUsrPktMsg )
{
    int     nSecondKeyIdx;

    DisplayCommUpDownMsg( 0, GetWORDFromASC( pchSessionCd, 2 ) );
	gbDCSCommType = GetWORDFromASC( pchSessionCd, 2 );

    InitPkt( pstSendUsrPktMsg );
    InitPkt( pstRecvUsrPktMsg );

    /*
     *  �������� ��û
     */
    if ( ReqDCSAuth( fdSock, pstSendUsrPktMsg, pstRecvUsrPktMsg ) != SUCCESS )
    {
    	printf( "[AuthDCS] ReqDCSAuth() ����\n" );
        return ERR_DCS_AUTH;
    }

    /*
     *  AB �޼��� ����
     */
    SetABAuthMsg( pstSendUsrPktMsg, 
    			  pstRecvUsrPktMsg, 
    			  pchSessionCd, 
    			  &nSecondKeyIdx );

    /*
     *  AB �޼��� �۽� & AC �޼��� ����
     */
    if ( SendNRecvUsrPkt2DCS( fdSock, pstSendUsrPktMsg, pstRecvUsrPktMsg )
         != SUCCESS )
    {
    	printf( "[AuthDCS] 'AB' �޽��� �۽� & 'AC' �޽��� ���� ����\n" );
        return ErrRet( ERR_DCS_AUTH );
    }

    /*
     *  AC �޼��� validation
     */
    if ( IsValidACAuthMsg( pstRecvUsrPktMsg, &nSecondKeyIdx ) == FALSE )
    {
    	printf( "[AuthDCS] 'AC' �޼��� validation ����\n" );
        return ErrRet( ERR_DCS_AUTH );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      IsValidACAuthMsg                                         *
*                                                                              *
*  DESCRIPTION :      AC�޼����� validation�Ѵ�.                                *
*                                                                              *
*  INPUT PARAMETERS:  USER_PKT_MSG* stRecvUsrPktMsg                            *
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
static bool IsValidACAuthMsg( USER_PKT_MSG* stRecvUsrPktMsg, 
							  int* pnSecondKeyIdx )
{
    char    achEncSrvRandNo[RANDOM_NO_LENGTH + 1]  	= { 0, };
    char    achDecSrvRandNo[RANDOM_NO_LENGTH + 1]  	= { 0, };
    char    achDecClntRandNo[RANDOM_NO_LENGTH + 1] 	= { 0, };
    int     i;
    int     nMsgLength                          	= RANDOM_NO_LENGTH;
    int     nCompFlag				  				= 0;
    
    /*
     *  ���ŵ� AC �޼��� validation
     */
    if ( memcmp( stRecvUsrPktMsg->achConnCmd,
                 DCS_AUTH_CMD3,
                 sizeof( stRecvUsrPktMsg->achConnCmd )
               ) != 0 )
    {
        return ErrRet( ERR_DCS_AUTH );
    }

    memcpy( achDecClntRandNo, DEC_CLIENT_RAND_NO, RANDOM_NO_LENGTH );

    /*
     *  ���� ������ �ص��Ѵ�.
     */
    memcpy( achEncSrvRandNo, stRecvUsrPktMsg->pbRealSendRecvData, KEY_LENGTH );
    TransDecrypt( achEncSrvRandNo,
                  achDecSrvRandNo,
                  &nMsgLength,
                  achFixedKey[*pnSecondKeyIdx] );

    /*
     *  ���� ������ Ŭ���̾�Ʈ ������ ���Ѵ�.
     */
    for ( i = 0; i < RANDOM_NO_LENGTH; i++ )
    {

        if ( achDecClntRandNo[i] != achDecSrvRandNo[i] )
        {
            nCompFlag = 1;
            break;
        }
    }

    if ( nCompFlag == 1 )
    {
        return FALSE;
    }

    return TRUE;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetABAuthMsg                                             *
*                                                                              *
*  DESCRIPTION :      AB ���� �޼����� �����Ѵ�.                                *
*                                                                              *
*  INPUT PARAMETERS:  USER_PKT_MSG* stSendUsrPktMsg                            *
*                     USER_PKT_MSG* stRecvUsrPktMsg                            *
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
static void SetABAuthMsg( USER_PKT_MSG* stSendUsrPktMsg,
	                      USER_PKT_MSG* stRecvUsrPktMsg,
	                      char* pchSessionCd, 
	                      int* pnSecondKeyIdx )
{
    char    achEncSrvRandNo[RANDOM_NO_LENGTH + 1]  = { 0, };
    char    achDecSrvRandNo[RANDOM_NO_LENGTH + 1]  = { 0, };
    char    achDecClntRandNo[RANDOM_NO_LENGTH + 1] = { 0, };
    char    achEncClntRandNo[RANDOM_NO_LENGTH + 1] = { 0, };
    int     nFirstKeyIdx;
    int     nTmpAdd         					= 0;
    int     nMsgLength                          = RANDOM_NO_LENGTH;

    time_t  tCurrDtime;
    byte    abTmp[11];

    /*
     *  ��ȣȭ�� ���� ����
     */
    memcpy( achEncSrvRandNo, stRecvUsrPktMsg->pbRealSendRecvData, KEY_LENGTH );

    /*
     *  DCS Current Dtime
     */
    memcpy( achDCSCurrDtime,
            stRecvUsrPktMsg->pbRealSendRecvData + KEY_LENGTH,
            sizeof( achDCSCurrDtime ) - 1 );

    /*
     *  key ����
     */
    TransDecrypt( achEncSrvRandNo,
                  achDecSrvRandNo,
                  &nMsgLength,
                  achInitialKey );      // �ʱ� key�� decryption
    nFirstKeyIdx = GetAuthIndex( achDecSrvRandNo );

    TransEncrypt( achDecSrvRandNo,
                  achEncSrvRandNo,
                  &nMsgLength,
                  achFixedKey[nFirstKeyIdx] );  // first key�� encryption

    memcpy( achDecClntRandNo, DEC_CLIENT_RAND_NO, RANDOM_NO_LENGTH );

    *pnSecondKeyIdx = GetAuthIndex( achDecClntRandNo );
    *pnSecondKeyIdx = ( nFirstKeyIdx + *pnSecondKeyIdx ) % 10 ;

    TransEncrypt( achDecClntRandNo,
                  achEncClntRandNo,
                  &nMsgLength,
                  achFixedKey[nFirstKeyIdx] );  // first key�� encryption

    /*
     *  AB �޼��� ����
     */
    stSendUsrPktMsg->chEncryptionYN = ENCRYPTION;
    memcpy( stSendUsrPktMsg->achConnCmd, DCS_AUTH_CMD2, COMMAND_LENGTH );

    memcpy( stSendUsrPktMsg->pbRealSendRecvData,
            achEncSrvRandNo,
            KEY_LENGTH );               // ��ȣȭ�� ������ ����
    nTmpAdd = nTmpAdd + KEY_LENGTH;

    memcpy( stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
            achEncClntRandNo,
            KEY_LENGTH );               // ��ȣȭ�� Ŭ���̾�Ʈ�� ����
    nTmpAdd = nTmpAdd + KEY_LENGTH;

    memcpy( stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
            pchSessionCd,
            JOB_SESSION_CODE_LEN );
    nTmpAdd = nTmpAdd + JOB_SESSION_CODE_LEN;              // session code

    memcpy( stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
             gpstSharedInfo->abMainTermID,
             nTermIDLength );           // Main terminal ID
    nTmpAdd = nTmpAdd + nTermIDLength;

    memcpy( stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
             gstCommInfo.abVehicleID,
             sizeof( gstCommInfo.abVehicleID ) );    // ���� ID
    nTmpAdd = nTmpAdd + sizeof( gstVehicleParm.abVehicleID );

    memcpy( stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
            gstVehicleParm.abTranspBizrID,
            sizeof( gstVehicleParm.abTranspBizrID ) );
                                        // transportation Business company ID
    nTmpAdd = nTmpAdd + sizeof( gstVehicleParm.abTranspBizrID );

    ASC2BCD( gpstSharedInfo->abMainPSAMID,
             stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
             nPSAMIDLength );           // main terminal SAM ID
    nTmpAdd = nTmpAdd + nPSAMIDBCDLength;

    memcpy( stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
            &gbSubTermCnt,
            2 );                        // sub terminal count

    nTmpAdd = nTmpAdd + 2;
    ASC2BCD( gpstSharedInfo->abSubPSAMID[0],
             stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
             nPSAMIDLength );           // sub terminal 1  SAM ID
    nTmpAdd = nTmpAdd + nPSAMIDBCDLength;

    ASC2BCD( gpstSharedInfo->abSubPSAMID[1],
             stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
             nPSAMIDLength );           // sub terminal 2  SAM ID
    nTmpAdd = nTmpAdd + nPSAMIDBCDLength;

    ASC2BCD( gpstSharedInfo->abSubPSAMID[2],
             stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd,
             nPSAMIDLength );           // sub terminal 3  SAM ID
    nTmpAdd = nTmpAdd + nPSAMIDBCDLength;

    memcpy( abTmp, CHILD_DEFAULT_ID, 10 );  // driver operator ID
    ASC2BCD( abTmp, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd, 10 );
    nTmpAdd = nTmpAdd + ( 10 / 2 );

    memcpy( abTmp, gpstSharedInfo->abSubTermID[0], nTermIDLength );
                                        // subterminal 1  ID
    abTmp[nTermIDLength] = 0x30;
    ASC2BCD( abTmp, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd, 10 );
    nTmpAdd = nTmpAdd + ( 10 / 2 );

    memcpy( abTmp, gpstSharedInfo->abSubTermID[1], nTermIDLength );
                                        // subterminal 2  ID
    abTmp[nTermIDLength] = 0x30;
    ASC2BCD( abTmp, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd, 10 );
    nTmpAdd = nTmpAdd + ( 10 / 2 );

    memcpy( abTmp, gpstSharedInfo->abSubTermID[2], nTermIDLength );
                                        // subterminal 3  ID
    abTmp[nTermIDLength] = 0x30;
    ASC2BCD( abTmp, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd, 10 );
    nTmpAdd = nTmpAdd + ( 10 / 2 );

    memcpy( abTmp, CHILD_DEFAULT_ID, 10 );  // receipt Printer ID
    memcpy ( abTmp, MAIN_RELEASE_VER, 4 );
    ASC2BCD( abTmp, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd, 10 );
    nTmpAdd = nTmpAdd + ( 10 / 2 );

    memcpy( abTmp, CHILD_DEFAULT_ID, 10 );  // GPS  ID
    ASC2BCD( abTmp, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd, 10 );
    nTmpAdd = nTmpAdd + ( 10 / 2 );

    GetRTCTime( &tCurrDtime );
    TimeT2ASCDtime( tCurrDtime, stSendUsrPktMsg->pbRealSendRecvData + nTmpAdd );
    nTmpAdd = nTmpAdd + 14;

    stSendUsrPktMsg->lDataSize = nTmpAdd;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReqDCSAuth                                               *
*                                                                              *
*  DESCRIPTION :      ����ý������� ���� ��û�� �Ѵ�.                           *
*                                                                              *
*  INPUT PARAMETERS:  int fdSock                                               *
*                     USER_PKT_MSG* stSendUsrPktMsg                            *
*                     USER_PKT_MSG* stRecvUsrPktMsg                            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_DCS_AUTH                                           *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short ReqDCSAuth( int fdSock,
		                 USER_PKT_MSG* pstSendUsrPktMsg,
		                 USER_PKT_MSG* pstRecvUsrPktMsg )
{
	short sResult = SUCCESS;

    /*
     *  ���� ��û ���� ����(ENQ)
     */
    pstSendUsrPktMsg->chEncryptionYN = ENCRYPTION;       // ��ȣȭ ����
    memcpy( pstSendUsrPktMsg->achConnCmd,
            DCS_AUTH_CMD,
            sizeof( pstSendUsrPktMsg->achConnCmd ) );
    pstSendUsrPktMsg->lDataSize = 0;

	sResult = SendNRecvUsrPkt2DCS( fdSock, pstSendUsrPktMsg, pstRecvUsrPktMsg );
    if ( sResult != SUCCESS )
    {
    	printf( "[ReqDCSAuth] 'ENQ' �޽��� �۽� & 'AA' �޽��� ���� ����\n" );
        return ErrRet( ERR_DCS_AUTH );
    }


    /*
     *  "AA"�޼��� ���� COMMAND ��
     */
    if ( memcmp( pstRecvUsrPktMsg->achConnCmd,
                 DCS_AUTH_CMD1,
                 sizeof( pstSendUsrPktMsg->achConnCmd )
               ) != 0 )
    {
    	printf( "[ReqDCSAuth] 'AA' �޽��� �ƴ�\n" );
        return ErrRet( ERR_DCS_AUTH );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetAuthIndex                                             *
*                                                                              *
*  DESCRIPTION :      ���� �ε��� ���ϱ�                                        *
*                                                                              *
*  INPUT PARAMETERS:  char* pchRandomNo                                        *
*                                                                              *
*  RETURN/EXIT VALUE:     nAuthIdx                                             *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short GetAuthIndex( char* pchRandomNo )
{
    int i;
    int j;

    char achHexTable[17] = {"0123456789ABCDEF"};

    int nAuthIdx;
    int nHexTotal = 0;


    for ( i = 0 ; i < RANDOM_NO_LENGTH ; i++ )
    {
        for ( j = 0 ; j < 16 ; j++ )
        {
            if ( pchRandomNo[i] == achHexTable[j] )
            {
                nHexTotal += j;
                break;
            }
        }
    }

    nAuthIdx = nHexTotal % 10;
    return nAuthIdx;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendNRecvUsrPkt2DCS                                      *
*                                                                              *
*  DESCRIPTION :      ��Ŷ�� �۽� �� ���� �Ѵ�.                                 *
*                                                                              *
*  INPUT PARAMETERS:  int fdSock                                               *
*                     USER_PKT_MSG* stSendUsrPktMsg                            *
*                     USER_PKT_MSG* stRecvUsrPktMsg                            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_SEND                                        *
*                       ERR_PACKET_STX                                         *
*                       ERR_PACKET_ETX                                         *
*                       ERR_PACKET_BCC                                         *
*                       ERR_SOCKET_TIMEOUT                                     *
*                       ERR_SOCKET_RECV                                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SendNRecvUsrPkt2DCS( int fdSock,
                           USER_PKT_MSG* stSendUsrPktMsg,
                           USER_PKT_MSG* stRecvUsrPktMsg )
{
    short sRetVal = SUCCESS;

    sRetVal = SendUsrPkt2DCS( fdSock, TIMEOUT, MAX_RETRY_COUNT,
		stSendUsrPktMsg );
    if ( sRetVal != SUCCESS )
    {
        return -1;
    }

    sRetVal = RecvUsrPktFromDCS( fdSock, TIMEOUT, MAX_RETRY_COUNT,
		stRecvUsrPktMsg );
    if ( sRetVal != SUCCESS )
    {
        return -1;
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReSetupTerm                                              *
*                                                                              *
*  DESCRIPTION :      �ܸ��⸦ �缳ġ�ϴ� �Լ��� tc_leap, setup, ���� BLPL, ���� *
*                     �� ���۱� �÷��� ������ ������ ��� ������ �����ϰ� �����  *
*                     ���Ӱ� ������ �ٿ�ε� �޴´�.                             *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short ReSetupTerm( void )
{
	short       sReturnVal          = -1;
	byte i = 0;
	char        achCmdData[6]       = { 0, };
	bool        boolIsApplyOperParm = FALSE;    // ��Ķ���� �ٿ�� ����
	VER_INFO    stTmpVerInfo;
	time_t tStartDtime = 0;
	time_t tEndDtime = 0;

	time( &tStartDtime );
	printf( "[ReSetupTerm] ReSetupTerm ���� ------------------------------------------------\n" );

    /*
     *  tc_leap.dat, setup.dat, ���� BLPL����, ������ ���۱� �÷��� ������
     *  ������ ��� ������ �����Ѵ�.
     */
    DelBackupFile();
    DelFileOnReset();

    InitNextVerApplFlag();  // �̷��������뿩�� �ʱ�ȭ

	/*
	 *  ��Ķ���� ���� ����
	 */
	for ( i = 0; i < 3; i++ )
    {
    	if ( ConnDCS() == TRUE )
    	{
    		break;
    	}
	}
	if ( i >= 3 )
	{
		printf( "[ReSetupTerm] ConnDCS() ����\n" );
		time( &tEndDtime );
		printf( "[ReSetupTerm] ����ð� : %d sec\n", (int)( tEndDtime - tStartDtime ) );
		printf( "[ReSetupTerm] ReSetupTerm ���� ------------------------------------------------\n" );
		return -1;
	}

	for ( i = 0; i < 3; i++ )
    {
    	sReturnVal = DownVehicleParmFile();	// ������������ ����
		if ( sReturnVal == SUCCESS )
		{
			break;
		}
	}
	if ( i >= 3 )
	{
		printf( "[ReSetupTerm] DownVehicleParmFile() ����\n" );
		time( &tEndDtime );
		printf( "[ReSetupTerm] ����ð� : %d sec\n", (int)( tEndDtime - tStartDtime ) );
		printf( "[ReSetupTerm] ReSetupTerm ���� ------------------------------------------------\n" );
		return -1;
	}

	LoadVehicleParm();     // ���� �Ķ���� ���� �ε�

    CreateInstallInfoFile();            // �ν��� ���� ����

    InitVerInfo();                      // �������� �ʱ�ȭ
    LoadVerInfo( &stTmpVerInfo );       // �������� �ε�

    sReturnVal = DownFromDCS();			// ����ý������� ���� �ٿ�ε�
    if ( sReturnVal != SUCCESS )
    {
    	LogDCS("[ReSetupTerm] DownFromDCS() ����\n");
    	printf("[ReSetupTerm] DownFromDCS() ����\n");
    }
    ApplyNextVer();                     // �̷�������� ����

    if ( SetUploadVerInfo() < 0 )       // ����� �������� ����
    {
        DebugOut( "\r\n SetUploadVerInfo Fail[%d] \r\n", sReturnVal );
        LogWrite( ERR_SET_UPLOAD_VER );
    }

    /*
     *  �缳ġ����� ����
     *  [0] - re-apply request
     *  [1] - parameter Loadrequest
     *  [2] - file Load request
     *  [3] - voice file Load
     *  [4] - driver oper. Load
     *  [5] - operator parameter
     */
    achCmdData[0] = GetASCNoFromDEC( SUCCESS );
    achCmdData[1] = GetASCNoFromDEC( boolIsApplyNextVerParm );
    achCmdData[2] = GetASCNoFromDEC( boolIsApplyNextVerAppl );
    achCmdData[3] = GetASCNoFromDEC( boolIsApplyNextVerVoice );
    achCmdData[4] = GetASCNoFromDEC( boolIsApplyNextVerDriverAppl );
    achCmdData[5] = GetASCNoFromDEC( boolIsApplyOperParm );

    SetSharedDataforCmd( CMD_RESETUP, achCmdData, sizeof( achCmdData ) );

    InitNextVerApplFlag();  // �̷��������뿩�� �ʱ�ȭ

	ctrl_event_info_write( EVENT_RESETUP_TERM );

	time( &tEndDtime );
	printf( "[ReSetupTerm] ����ð� : %d sec\n", (int)( tEndDtime - tStartDtime ) );
	printf( "[ReSetupTerm] ReSetupTerm ���� ------------------------------------------------\n" );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetupTerm                                                *
*                                                                              *
*  DESCRIPTION :      �ܸ��� ���ý� ����Ǹ� ��Ķ���� ������ üũ�Ͽ� ������  *
*                       �ٿ�ε��ϰ� �ν������ϰ� ���������� �����ϰ�             *
*                       ���� ���ε�� �ٿ�ε带 ���� �� keyset ��� ��û�� �Ѵ�  *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SetupTerm( void )
{
    short       sReturnVal          = SUCCESS;
    short       sInstallFileExistYN = SUCCESS;
    char        achCmdData[5]       = { 0, };
    VER_INFO    stTmpVerInfo;
	byte i = 0;
	time_t tStartDtime = 0;
	time_t tEndDtime = 0;

	time( &tStartDtime );
	printf( "[SetupTerm] SetupTerm ���� ----------------------------------------------------\n" );

	DelBackupFile();							// backup���� ����

	InitIP( NETWORK_DEVICE );
	for ( i = 0; i < 3; i++ )
    {
    	if ( ConnDCS() == TRUE )
    	{
    		break;
    	}
	}
	if ( i >= 3 )
	{
		printf( "[SetupTerm] ConnDCS() ����\n" );
	}

	sReturnVal = ChkNDownOperParmFile();		// �������� üũ �� �ٿ�ε�

    sReturnVal = LoadVehicleParm();				// �������� �ε�
    if ( sReturnVal < SUCCESS )
    {
        /*
         *  �����޸𸮿� �۾� ���� ����
         */
        achCmdData[0] = CMD_FAIL_RES;
        SetSharedDataforCmd( CMD_SETUP, achCmdData, sizeof( achCmdData )  );

		printf( "[SetupTerm] �������� �ε� ����\n" );
		time( &tEndDtime );
		printf( "[SetupTerm] ����ð� : %d sec\n", (int)( tEndDtime - tStartDtime ) );
		printf( "[SetupTerm] SetupTerm ���� ----------------------------------------------------\n" );

        return sReturnVal;
    }

    /*
     *  �ν��������� �����ϴ��� üũ
     *  �����Ѵٸ� ���� ���ý� ����� ����� �� �Ȱ����� �����Ͽ�
     *  ���������� �ٽ� üũ�Ѵ�.
     */
    sInstallFileExistYN = access( INSTALL_INFO_FILE, F_OK );
    CreateInstallInfoFile();				// �ν������� ����

	sReturnVal = Upload2DCS();				// ����ý������� ���� ���ε�
	if ( sReturnVal != SUCCESS )
	{
		LogDCS( "[SetupTerm] Upload2DCS() ����\n" );
		printf( "[SetupTerm] Upload2DCS() ����\n" );
	}

    if ( sInstallFileExistYN == SUCCESS )
    {
        /*
         *  �������� �ε�� BLPL ������ ���翩�� �� �̾�ޱ������� �����Ѵ�.
         */
        LoadVerInfo( &stTmpVerInfo );
        SaveVerFile();
    }

    sReturnVal = DownFromDCS();			// ����ý������� ���� �ٿ�ε�
    if ( sReturnVal != SUCCESS )
    {
    	LogDCS("[SetupTerm] DownFromDCS() ����\n");
    	printf("[SetupTerm] DownFromDCS() ����\n");
    }
    ApplyNextVer();						// �̷�������� ����

    if ( boolIsApplyNextVer == TRUE )	//�̷���������� ������ �Ǿ��ٸ�
    {
        if ( SetUploadVerInfo() < 0 )	// ����۽ſ� ���������� ����
        {
            printf("[SetupTerm] SetUploadVerInfo() ����\n");
        }
    }

    /*
     *  �¾������ �������ش�.
     *  [0] - re-apply request
     *  [1] - parameter Loadrequest
     *  [2] - file Load request
     *  [3] - voice file Load
     *  [4] - driver oper. Load
     */
    achCmdData[0] = GetASCNoFromDEC( SUCCESS );
    achCmdData[1] = GetASCNoFromDEC( boolIsApplyNextVerParm );
    achCmdData[2] = GetASCNoFromDEC( boolIsApplyNextVerAppl );
    achCmdData[3] = GetASCNoFromDEC( boolIsApplyNextVerVoice );
    achCmdData[4] = GetASCNoFromDEC( boolIsApplyNextVerDriverAppl );
    SetSharedDataforCmd( CMD_SETUP, achCmdData, sizeof( achCmdData ) );

	printf( "[SetupTerm] �����     ���뿩�� : %s\n", GetBoolString( boolIsApplyNextVerParm ) );
	printf( "[SetupTerm] ���α׷�     ���뿩�� : %s\n", GetBoolString( boolIsApplyNextVerAppl ) );
	printf( "[SetupTerm] ��������     ���뿩�� : %s\n", GetBoolString( boolIsApplyNextVerVoice ) );
	printf( "[SetupTerm] ���������۱� ���뿩�� : %s\n", GetBoolString( boolIsApplyNextVerDriverAppl ) );
	
    InitNextVerApplFlag();  // �̷��������뿩�� �ʱ�ȭ
    ReqKeysetRegist();      // keyset ��Ͽ�û

	time( &tEndDtime );
	printf( "[SetupTerm] ����ð� : %d sec\n", (int)( tEndDtime - tStartDtime ) );
	printf( "[SetupTerm] SetupTerm ���� ----------------------------------------------------\n" );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ChkNDownOperParmFile                                     *
*                                                                              *
*  DESCRIPTION :      ��ſ����� �Ͽ� ����κ��� ��Ķ���� ������ �ٿ�ε� �ް�*
*                       ���н� ���и� ����Ѵ�.                                 *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*						ERR_DCS_SETUP										   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short ChkNDownOperParmFile( void )
{
    short sResult = SUCCESS;

	if ( IsExistFile( VEHICLE_PARM_FILE ) == FALSE )
	{
		printf( "[ChkNDownOperParmFile] �������������� �������� �ʾ� �ٿ�ε���\n" );
		sResult = DownVehicleParmFile();		// ������������ ����
        if ( sResult != SUCCESS )
        {
			printf( "[ChkNDownOperParmFile] DownVehicleParmFile() ����\n" );
            return ERR_DCS_SETUP;
        }
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InitNextVerApplFlag                                      *
*                                                                              *
*  DESCRIPTION :      �̷����� ���뿩�� �÷��� �ʱ�ȭ                            *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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
static void InitNextVerApplFlag( void )
{

   /*
    *   �̷����� ���뿩�� �ʱ�ȭ
    */
    boolIsApplyNextVer              = FALSE;
    boolIsApplyNextVerParm          = FALSE;
    boolIsApplyNextVerVoice         = FALSE;
    boolIsApplyNextVerAppl          = FALSE;
    boolIsApplyNextVerDriverAppl    = FALSE;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DelBackupFile                                            *
*                                                                              *
*  DESCRIPTION :      tc_leap, setup, ���� BLPL���ϰ� ���������۱����α׷� ����  *
*                       �÷��������� ��� ������ �����Ѵ�.                       *
*                       �缳ġ�ÿ� ������ ����ϴµ� �缳ġ �� ������ �� ��츦   *
*                       ���                                                   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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
static void DelBackupFile( void )
{

    /*
     *  �缳ġ�� �������� ��� backup ������ ������
     *
     */
//    unlink( TC_LEAP_BACKUP_FILE );
//    unlink( SETUP_BACKUP_FILE );
    unlink( MASTER_BL_BACKUP_FILE );
    unlink( MASTER_PREPAY_PL__BACKUP_FILE );
    unlink( MASTER_POSTPAY_PL__BACKUP_FILE );
    unlink( MASTER_AI_BACKUP_FILE );
//    unlink( KPDAPPLY_FLAG_BACKUP_FILE );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DelFileOnReset                                           *
*                                                                              *
*  DESCRIPTION :      tc_leap, setup, ���� BLPL, ������ ���۱� �÷��� ������     *
*                       ������ ��� ������ ����                                 *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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
static void DelFileOnReset( void )
{

    rename( TC_LEAP_FILE,           TC_LEAP_BACKUP_FILE );
    rename( SETUP_FILE,             SETUP_BACKUP_FILE );
    rename( MASTER_BL_FILE,         MASTER_BL_BACKUP_FILE );
    rename( MASTER_PREPAY_PL_FILE,  MASTER_PREPAY_PL__BACKUP_FILE );
    rename( MASTER_POSTPAY_PL_FILE, MASTER_POSTPAY_PL__BACKUP_FILE );
    rename( MASTER_AI_FILE,         MASTER_AI_BACKUP_FILE );
    rename( KPDAPPLY_FLAGFILE,      KPDAPPLY_FLAG_BACKUP_FILE );

    system( "rm *.dat" );
    system( "rm *.trn" );
    system( "rm *.tmp" );
    system( "rm *.flg" );
    system( "rm 2* " );
    system( "rm *.cfg " );

    rename( TC_LEAP_BACKUP_FILE,            TC_LEAP_FILE );
    rename( SETUP_BACKUP_FILE,              SETUP_FILE );
    rename( MASTER_BL_BACKUP_FILE,          MASTER_BL_FILE );
    rename( MASTER_PREPAY_PL__BACKUP_FILE,  MASTER_PREPAY_PL_FILE );
    rename( MASTER_POSTPAY_PL__BACKUP_FILE, MASTER_POSTPAY_PL_FILE );
    rename( MASTER_AI_BACKUP_FILE,          MASTER_AI_FILE );
    rename( KPDAPPLY_FLAG_BACKUP_FILE,      KPDAPPLY_FLAGFILE );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReqKeysetRegist                                          *
*                                                                              *
*  DESCRIPTION :      ���� ���μ����� keyset����� ��û�ϰ� ó�� ���нÿ� SAM���� *
*                       ���������� �ʱ�ȭ���ش�.                                *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void ReqKeysetRegist( void )
{
    short       sReturnVal 		= SUCCESS;
    byte        SharedMemoryCmd;
    char        achCmdData[5]       = { 0, };
    word        wCmdDataSize        = 0;

    /*
     *  keyset ��� ��û
     */
    achCmdData[0] = CMD_REQUEST;
    sReturnVal = SetSharedCmdnDataLooping( CMD_KEYSET_IDCENTER_WRITE,
                                           &achCmdData[0],
                                           sizeof( achCmdData[0] ) );

    usleep( 500000 );

    while( ( achCmdData[0] != CMD_FAIL_RES ) &&
           ( achCmdData[0] != CMD_SUCCESS_RES ) )
    {

        GetSharedCmd( &SharedMemoryCmd, achCmdData, &wCmdDataSize );
        DebugOut( "��������?=>[%d]", achCmdData[0] );

        /*
         *  ó�� �Ϸ��� ���
         */
        if( ( SharedMemoryCmd == CMD_KEYSET_IDCENTER_WRITE ) &&
            ( achCmdData[0] == CMD_FAIL_RES ||
              achCmdData[0] == CMD_SUCCESS_RES ) )
        {
            /*
             *  ��û���� clear
             */
            ClearSharedCmdnData  ( CMD_KEYSET_IDCENTER_WRITE );

            if( achCmdData[0] == CMD_FAIL_RES )
            {
                DebugOut( "K command ���� ���� \n" );
                boolIsRegistMainTermPSAM = FALSE;

                /*
                 *  SAM���� ���������� �ʱ�ȭ�Ͽ� ������Ž� SAM���� ������ �ٽ�
                 *  ���� �޴´�
                 */
                InitSAMRegistInfoVer( );
            }
            else if( achCmdData[0] == CMD_SUCCESS_RES )
            {
                DebugOut( "K command ���� ���� \n" );
                boolIsRegistMainTermPSAM = TRUE;
            }

            break;
        }
        else if ( SharedMemoryCmd != CMD_KEYSET_IDCENTER_WRITE )
        {
            break;
        }

        usleep( 500000 );

    }
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendRS                                                   *
*                                                                              *
*  DESCRIPTION :      ����ý������� Response�� �۽�                            *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock                                             *
*                       USER_PKT_MSG* pstSendUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_SEND                                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SendRS( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg )
{

    RS_COMMAND *pRsCommand = (RS_COMMAND *)pstSendUsrPktMsg->pbRealSendRecvData;

    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    memcpy( pstSendUsrPktMsg->achConnCmd, RESPONSE_CMD, COMMAND_LENGTH );

    memcpy( pRsCommand->abMainTermID,
            gpstSharedInfo->abMainTermID,
            nTermIDLength );
    memcpy( pRsCommand->abRecvSeqNo,
            abRecvSeqNo,
            sizeof( pRsCommand->abRecvSeqNo ) );
    memcpy( pRsCommand->abRecvStatusCode,
            RESPONSE_STATUS_MSG,
            sizeof( pRsCommand->abRecvStatusCode ) );
    memset( pRsCommand->abRecvStatusMsg,
            SPACE,
            sizeof( pRsCommand->abRecvStatusMsg ) );

    pstSendUsrPktMsg->lDataSize = sizeof( RS_COMMAND );


    if ( SendUsrPkt2DCS( fdSock, TIMEOUT, MAX_RETRY_COUNT, pstSendUsrPktMsg )
                       != SUCCESS )
    {
        return ErrRet( ERR_SOCKET_RS_SEND );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendEOS                                                  *
*                                                                              *
*  DESCRIPTION :      ����ý������� EOS�� �۽�                                 *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock                                             *
*                       USER_PKT_MSG* pstSendUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_SEND                                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SendEOS( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg )
{

    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    memcpy( pstSendUsrPktMsg->achConnCmd, EOS_CMD, COMMAND_LENGTH );
    pstSendUsrPktMsg->lDataSize = 0;

    if ( SendUsrPkt2DCS( fdSock, TIMEOUT, MAX_RETRY_COUNT, pstSendUsrPktMsg )
         != SUCCESS )
    {
        return ErrRet( ERR_SOCKET_SEND );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendEOF                                                  *
*                                                                              *
*  DESCRIPTION :      ����ý������� EOF�� �۽�                                 *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock                                             *
*                       USER_PKT_MSG* pstSendUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_SEND                                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SendEOF( int fdSock, USER_PKT_MSG* pstSendUsrPktMsg )
{

    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    pstSendUsrPktMsg->lDataSize = 0;
    memcpy( pstSendUsrPktMsg->achConnCmd, EOF_CMD, COMMAND_LENGTH );

    if ( SendUsrPkt2DCS( fdSock, TIMEOUT, MAX_RETRY_COUNT, pstSendUsrPktMsg )
         != SUCCESS )
    {
        return ErrRet( ERR_SOCKET_SEND );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvRS                                                   *
*                                                                              *
*  DESCRIPTION :      ����ý������� Response�� ����                            *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock                                             *
*                       USER_PKT_MSG* pstRecvUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_RS_RECV                                     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short RecvRS( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg )
{
    short   sReturnVal  = SUCCESS;

    sReturnVal = RecvUsrPktFromDCS( fdSock,
                                    TIMEOUT,
                                    MAX_RETRY_COUNT,
                                    pstRecvUsrPktMsg );

    if ( sReturnVal == SUCCESS )
    {
        if ( memcmp( pstRecvUsrPktMsg ->achConnCmd, RESPONSE_CMD, COMMAND_LENGTH
                   ) != SUCCESS )
        {
            sReturnVal = ERR_SOCKET_RS_RECV;
        }
    }
    else
    {
        sReturnVal = ERR_SOCKET_RS_RECV;
    }

    return ErrRet( sReturnVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvACK                                                  *
*                                                                              *
*  DESCRIPTION :      ����ý������� ACK�� ����                                 *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock                                             *
*                       USER_PKT_MSG* pstRecvUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_SOCKET_ACK_RECV                                    *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS : 0334���� ���ĺ��� ACK�� ���� �޴´�.                               *
*                                                                              *
*******************************************************************************/
short RecvACK( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg )
{
    short   sReturnVal  = SUCCESS;

    /*
     *  0334���� ���ĺ��� ACK�� ���� �޴´�.
     */
    if ( ( sReturnVal = RecvUsrPktFromDCS( fdSock,
                                           TIMEOUT,
                                           MIN_RETRY_COUNT,
                                           pstRecvUsrPktMsg )
         ) == SUCCESS )
    {
        if ( memcmp( pstRecvUsrPktMsg ->achConnCmd, ACK_CMD, COMMAND_LENGTH
                   ) != 0 )
        {
            sReturnVal = ERR_SOCKET_ACK_RECV;
        }
    }
    else
    {
        sReturnVal = ERR_SOCKET_ACK_RECV;
    }

    return ErrRet( sReturnVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ConnDCS                                                  *
*                                                                              *
*  DESCRIPTION :      ����ý��ۿ� LEAP ������ �ϰ� IP�� ���´�.               *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     boolIsCommAble - ��Ű��� ����                        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static bool ConnDCS( void )
{
	byte i = 0;
	int nResult = 0;

	for ( i = 0; i < 3; i++ )
	{
		if ( i != 0 )
		{
			printf( "[ConnDCS] LEAPMain() ��õ�\n" );
		}
		nResult = LEAPMain();
		if ( nResult >= 0 )
		{
			break;
		}
	}

	if ( nResult < 0 )
	{
    	printf( "[ConnDCS] LEAPMain() ����\n" );
		return FALSE;
	}

	for ( i = 0; i < 3; i++ )
	{
		if ( i != 0 )
		{
			printf( "[ConnDCS] SetLocalIP() ��õ�\n" );
		}
		nResult = SetLocalIP();
		if ( nResult >= 0 )
		{
			break;
		}
	}

	if ( nResult < 0 )
	{
    	printf( "[ConnDCS] SetLocalIP() ����\n" );
		return FALSE;
	}

	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DCSComm                                                  *
*                                                                              *
*  DESCRIPTION :      ����ý��ۿ� ���ε�� �ٿ�ε� �� ���� ������ ����� ������ *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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
void *DCSComm( void *arg )
{
	short sResult = SUCCESS;
	byte i = 0;
	time_t tStartDtime = 0;
	time_t tEndDtime = 0;
	bool boolIsConnected = FALSE;

	time( &tStartDtime );
	printf( "\n" );
    printf( "[DCSComm] ������� Thread ���� ////////////////////////////////////////////////\n" );
	LogDCS( "[�������] Thread ����\n" );

    /*
     *  �̷����� ���� ����
     */
    ApplyNextVer();

    /*
     *  ������� ����Ƚ�� FND�� ���÷���
     */
    DisplayDCSCommCnt( nDCSCommSuccCnt );

    /*
     *  ����ý��ۿ� �����Ͽ� ���ε�� �ٿ�ε带 �����Ѵ�.
     */
	boolIsConnected = ConnDCS();
	if ( boolIsConnected == FALSE )
	{
		LogDCS("[DCSComm] ConnDCS() ����\n");
		printf("[DCSComm] ConnDCS() ����\n");
	}

    if ( boolIsConnected == TRUE )
    {
        /*
         *  ���ε� ����
         */
		for ( i = 0; i < 3; i++ )
	    {
	    	sResult = Upload2DCS();	// ������������ ����
			if ( sResult == SUCCESS )
			{
				break;
			}
		}
		if ( sResult != SUCCESS )
		{
			LogDCS("[DCSComm] Upload2DCS() ����\n");
			printf("[DCSComm] Upload2DCS() ����\n");
		}

        /*
         *  ������� ����Ƚ�� FND�� ���÷���
         */
        DisplayDCSCommCnt( nDCSCommSuccCnt );

        /*
         *  �ٿ�ε� ����
         */
		sResult = DownFromDCS();
		if ( sResult == SUCCESS )
        {
            /*
             *  ������� ����Ƚ�� ����
             */
            nDCSCommSuccCnt++;
        }
		else
		{
			LogDCS("[DCSComm] DownFromDCS() ����\n");
			printf("[DCSComm] DownFromDCS() ����\n");
		}
		

        /*
         *  �̷����� ���� ����
         */
        ApplyNextVer();        // next -> curr

    }

    if ( boolIsApplyNextVer == TRUE )
    {
        ReqApplyNextVer( CMD_NEW_CONF_IMG,
                         boolIsApplyNextVer,
                         boolIsApplyNextVerParm,
                         boolIsApplyNextVerAppl,
                         boolIsApplyNextVerVoice,
                         boolIsApplyNextVerDriverAppl );
    }

    /*
     *  ������μ��������� � �Ķ���� ������ load�Ѵ�.
     */
    LoadVehicleParm();

    /*
     *  TermComm���� ���� Thread �ڵ鸵 ������ ����
     */
    boolIsDCSThreadComplete = TRUE;
    boolIsDCSThreadStartEnable = TRUE;
	
	time( &tEndDtime );
	printf( "[DCSComm] ����ð� : %d sec\n", (int)( tEndDtime - tStartDtime ) );
	printf( "[DCSComm] ������� Thread ���� ////////////////////////////////////////////////\n" );
	printf( "\n" );
	LogDCS( "[�������] Thread ����\n" );

	TimerStart( &gtDCSCommStart );

    return (void*)NULL;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReqApplyNextVer                                          *
*                                                                              *
*  DESCRIPTION :      �����޸𸮿� �̷�������� ������ ��û�Ѵ�.               *
*                                                                              *
*  INPUT PARAMETERS:  byte bCmd,                                               *
*                     boolIsApplyNextVer,                                      *
*                     boolIsApplyNextVerParm,                                  *
*                     boolIsApplyNextVerAppl,                                  *
*                     boolIsApplyNextVerVoice,                                 *
*                     boolIsApplyNextVerDriverAppl                             *
*                                                                              *
*  RETURN/EXIT VALUE:     void                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2006-03-16                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void ReqApplyNextVer( byte bCmd,
                      bool boolIsApplyNextVer,
                      bool boolIsApplyNextVerParm,
                      bool boolIsApplyNextVerAppl,
                      bool boolIsApplyNextVerVoice,
                      bool boolIsApplyNextVerDriverAppl)
{

    char    achCmdData[5]   = { 0, };
    word   wCmdDataSize;

    /*
     *  ASCII ������ ������ ����
     */
    achCmdData[0] = boolIsApplyNextVer + 0x30;        // re-apply request
    achCmdData[1] = boolIsApplyNextVerParm + 0x30;    // parameter Loadrequest
    achCmdData[2] = boolIsApplyNextVerAppl + 0x30;    // file Load request
    achCmdData[3] = boolIsApplyNextVerVoice + 0x30;   // voice file Load request
    achCmdData[4] = boolIsApplyNextVerDriverAppl + 0x30;// driver operator Load

    /*
     *  �����޸𸮿� ����Ǵ� ������ �������ش�.
     */
    SetSharedCmdnDataLooping( bCmd,
                              achCmdData,
                              sizeof( achCmdData ) );

    /*
     *  ���ο��� ó�� �Ϸ��ϸ� ������ ������ clear���ش�.
     */
    while( TRUE )
    {
        /*
         *  ó���Ϸ��ߴ��� üũ
         */
        GetSharedCmd( &bCmd, achCmdData, &wCmdDataSize );

        /*
         *  ó�� ����
         */
        if ( ( bCmd  == bCmd ) && ( achCmdData[0] == CMD_SUCCESS_RES ) )
        {
            /*
             *  ��û���� clear
             */
            ClearSharedCmdnData( CMD_NEW_CONF_IMG );

            /*
             *  ���� flag�� �ʱ�ȭ
             */
            boolIsApplyNextVer              = FALSE;
            boolIsApplyNextVerParm          = FALSE;
            boolIsApplyNextVerVoice         = FALSE;
            boolIsApplyNextVerAppl          = FALSE;
            boolIsApplyNextVerDriverAppl    = FALSE;

            break;

        }
        /*
         *  ó�� ����
         */
        else if ( ( bCmd  == CMD_NEW_CONF_IMG ) &&
                  ( achCmdData[0] == CMD_FAIL_RES ) )
        {
            /*
             *  ��û���� clear
             */
            ClearSharedCmdnData( CMD_NEW_CONF_IMG );

            break;
        }

        usleep( 500000 );
    }
}
