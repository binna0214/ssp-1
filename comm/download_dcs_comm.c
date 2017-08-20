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
*  PROGRAM ID :       download_dcs_comm.c	                                   *
*                                                                              *
*  DESCRIPTION:       ����ý������κ��� ���� �ٿ�ε带 ���� �Լ����� �����Ѵ�.  *
*                                                                              *
*  ENTRY POINT:     short DownVehicleParmFile( void );						   *
*					short DownFromDCS( void );								   *
*                                                                              *
*  INPUT FILES:     c_ve_inf.dat                                               *
*                                                                              *
*  OUTPUT FILES:    achDCSCommFile�� ��ϵ� ���ϵ�                      		   *
*					c_ve_inf.dat			- ��������						   *
*					downloadinfo.dat		- �ٿ�ε�� ���� ��� ����		   *
*					downfilelink.dat		- �̾�޴� ������ ����			   *
*					connSucc.dat			- ������� �����ð�				   *
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
#include "dcs_comm.h"
#include "../proc/upload_file_mgt.h"
#include "../proc/file_mgt.h"
#include "../proc/version_file_mgt.h"
#include "../proc/download_file_mgt.h"
#include "../proc/version_mgt.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
#define RECV_OPER_PARM_FILE_CMD         "B0710"     // ���� �ٿ�ε� ��û
#define SEND_VER_INFO_CMD               "B0390"     // �ٿ�ε����� �������� �۽�
#define RECV_OPER_PARM_JOB              "01"		// ��Ķ���� ���� ��û
#define DOWN_FILE_JOB                   "03"		// ���ϴٿ�ε� ��û
#define RELAY_RECV_CLASS_NO         	37          // �̾�ޱ� ���� ���� ��ȣ
#define MASTER_AI_FILE_INDEX_NO     	45          // ���� AI������ �ε��� ��ȣ
#define RECV_EOF                         1
#define RECV_EOT                         2
#define RECV_EOS                         3
#define DOWN_MSG_TYPE		    		'1'

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short RecvVehicleParmFile( int fdSock,
		                          USER_PKT_MSG* stSendUsrPktMsg,
		                          USER_PKT_MSG* stRecvUsrPktMsg );

static short RecvFilePkt( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg,
	bool* pboolIsMoreRecvYn );

static short SendVerInfoPkt( int fdSock,
                             bool boolIsCurrVer,
                             USER_PKT_MSG* stSendUsrPktMsg,
                             USER_PKT_MSG* stRecvUsrPktMsg );

static short RecvFileFromDCS( int fdSock,
                              USER_PKT_MSG* pstSendUsrPktMsg,
                              USER_PKT_MSG* pstRecvUsrPktMsg,
                              bool* pboolIsMoreRecvYn,
                              int* pnCurrVerFileNo,
                              int* pnNextVerFileNo );

static short WriteRecvdFile( FILE*      	fdRecvFile,
                             USER_PKT_MSG* 	pstRecvUsrPktMsg );

static void SetRelayRecvInfo( FILE_RELAY_RECV_INFO *pstFileRelayRecvInfo,
                              int nFileNo,
                              PKT_HEADER_INFO *pstPktHeaderInfo );


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DownVehicleParmFile                                      *
*                                                                              *
*  DESCRIPTION :      �������� �Ķ���� ������ ����� ���� �ٿ�ε� �޴´�        *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*						ErrRet( ERR_SESSION_OPEN )							   *
*           			ErrRet( ERR_SESSION_CLOSE )                            *
*						ErrRet( ERR_DCS_AUTH )								   *
*						ErrRet( ERR_RECV_VEHICLE_FILE )						   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short DownVehicleParmFile( void )
{
    short   sRetVal         = SUCCESS;
    int     fdSock;

    byte    abSendBuff[MAX_PKT_SIZE] = { 0, };
    byte    abRecvBuff[MAX_PKT_SIZE] = { 0, };

    USER_PKT_MSG stSendUsrPktMsg;
    USER_PKT_MSG stRecvUsrPktMsg;

    stSendUsrPktMsg.pbRealSendRecvData = abSendBuff;
    stRecvUsrPktMsg.pbRealSendRecvData = abRecvBuff;
    stSendUsrPktMsg.nMaxDataSize = sizeof( abSendBuff );
    stRecvUsrPktMsg.nMaxDataSize = sizeof( abRecvBuff );

    fdSock = OpenSession( gabDCSIPAddr, SERVER_COMM_PORT );    //���� ����
    if ( fdSock < 0 )
    {
        LogDCS( "[DownVehicleParmFile] OpenSession() ����\n" );
		printf( "[DownVehicleParmFile] OpenSession() ����\n" );
//        CloseSession( fdSock );
        return ErrRet( ERR_SESSION_OPEN );
    }

    /*
     *  ���� ����
     */
    if ( AuthDCS( fdSock, RECV_OPER_PARM_JOB, &stSendUsrPktMsg, &stRecvUsrPktMsg
                ) != SUCCESS )
    {
        LogDCS( "[DownVehicleParmFile] AuthDCS() ����\n" );
		printf( "[DownVehicleParmFile] AuthDCS() ����\n" );
		CloseSession( fdSock );
		DisplayASCInDownFND( FND_READY_MSG );	// FND �ʱ�ȭ
        return ErrRet( ERR_DCS_AUTH );
    }

    /*
     *  ���� �Ķ���� ���� ����
     */
    sRetVal = RecvVehicleParmFile( fdSock, &stSendUsrPktMsg, &stRecvUsrPktMsg );
    if( sRetVal != SUCCESS )
    {
        LogDCS( "[DownVehicleParmFile] RecvVehicleParmFile() ����\n" );
		printf( "[DownVehicleParmFile] RecvVehicleParmFile() ����\n" );
    	CloseSession( fdSock );
		DisplayASCInDownFND( FND_READY_MSG );	// FND �ʱ�ȭ
		return ErrRet( ERR_RECV_VEHICLE_FILE );
    }

    /*
     *  � �Ķ���� ���� ����
     */
    sRetVal = WriteOperParmFile( stRecvUsrPktMsg.pbRealSendRecvData,
                                 stRecvUsrPktMsg.lDataSize );
    if( sRetVal != SUCCESS )
    {
        LogDCS( "[DownVehicleParmFile] WriteOperParmFile() ����\n" );
		printf( "[DownVehicleParmFile] WriteOperParmFile() ����\n" );
    	CloseSession( fdSock );
		DisplayASCInDownFND( FND_READY_MSG );	// FND �ʱ�ȭ
        return ErrRet( ERR_RECV_VEHICLE_FILE );
    }

	/*
	 *  EOS �۽�
	 */
	SendEOS( fdSock, &stSendUsrPktMsg );

	boolIsApplyNextVer = TRUE;					// 0 - can not apply, 1 - can apply
	boolIsApplyNextVerParm = TRUE;

	CloseSession( fdSock );
	DisplayASCInDownFND( FND_READY_MSG );		// FND �ʱ�ȭ

	printf( "[DownVehicleParmFile] �������� �ٿ�ε� ����\n" );

	return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvVehicleParmFile                                      *
*                                                                              *
*  DESCRIPTION :      ����ý������� ���� �������� �Ķ���� ������ �޴´�.        *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock    - ����                                   *
*                       USER_PKT_MSG* stSendUsrPktMsg  - �۽� ��Ŷ             *
*                       USER_PKT_MSG* stRecvUsrPktMsg  - ���� ��Ŷ             *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_RECV_OPER_PARM_FILE                                *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short RecvVehicleParmFile( int fdSock,
		                          USER_PKT_MSG* pstSendUsrPktMsg,
		                          USER_PKT_MSG* pstRecvUsrPktMsg )
{
    /*
     *  ��Ķ���� ���� ��û���� ����
     */
    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;    // ��ȣȭ ����
    memcpy( pstSendUsrPktMsg->achConnCmd,                 // ��Ķ���� ��ûCMD
            RECV_OPER_PARM_FILE_CMD,
            sizeof( pstSendUsrPktMsg->achConnCmd ) );
    pstSendUsrPktMsg->lDataSize = 0;                      // data size

    memcpy( pstSendUsrPktMsg->pbRealSendRecvData,
            gstCommInfo.abVehicleID,
            sizeof( gstCommInfo.abVehicleID ) );

    /*
     *  ��Ķ���� ���� ��û �޼��� �۽� �� ����
     */
    if ( SendNRecvUsrPkt2DCS( fdSock, pstSendUsrPktMsg, pstRecvUsrPktMsg )
         != SUCCESS )
    {
        return ErrRet( ERR_RECV_OPER_PARM_FILE );
    }

    return SUCCESS;
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvFileHeader                                           *
*                                                                              *
*  DESCRIPTION :      ����ý������κ��� ������ ����� ����                      *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock                                             *
*                       USER_PKT_MSG* pstRecvUsrPktMsg                         *
*						bool* pboolIsMoreRecvYn	- �� ���� ���� �ִ��� ����	   *
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
static short RecvFilePkt( int fdSock, USER_PKT_MSG* pstRecvUsrPktMsg,
	bool* pboolIsMoreRecvYn )
{
	short sResult = SUCCESS;

    /*
     *  ���� ��� ����
     */
	sResult = RecvUsrPktFromDCS( fdSock, TIMEOUT, MAX_RETRY_COUNT,
		pstRecvUsrPktMsg );
    if ( sResult != SUCCESS )
    {
        return ErrRet( ERR_RECV_FILE );
    }

    /*
     *  EOF�� ���
     */
	if ( memcmp( pstRecvUsrPktMsg->achConnCmd, EOF_CMD,
			COMMAND_LENGTH ) == 0 )
    {
        *pboolIsMoreRecvYn = TRUE;
        return RECV_EOF;
    }
    /*
     *  EOT�� ���
     */
    else if ( memcmp( pstRecvUsrPktMsg->achConnCmd, EOT_CMD,
			COMMAND_LENGTH ) == 0 )
    {
        *pboolIsMoreRecvYn = FALSE;
        return RECV_EOT;
    }
    /*
     *  EOS�� ���
     */
    else if ( memcmp( pstRecvUsrPktMsg->achConnCmd, EOS_CMD,
			COMMAND_LENGTH ) == 0 )
    {
        *pboolIsMoreRecvYn = FALSE;
        return RECV_EOS;
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DownFromDCS                                              *
*                                                                              *
*  DESCRIPTION :      ����ý������κ��� ������ �ٿ�ε��Ѵ�.                    *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ErrRet( ERR_SESSION_OPEN )                             *
*                       ErrRet( ERR_SESSION_CLOSE )                            *
*                       ErrRet( ERR_DCS_AUTH )                                 *
*                       ErrRet( ERR_SEND_VER_IFNO )                            *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short DownFromDCS( void )
{
    short   sRetVal         = SUCCESS;
    int     fdSock;

    USER_PKT_MSG    stSendUsrPktMsg;
    USER_PKT_MSG    stRecvUsrPktMsg;
    byte            abSendBuff[MAX_PKT_SIZE];
    byte            abRecvBuff[MAX_PKT_SIZE];

    int 		nVerTypeLoop;
    bool 		boolIsMoreRecvYn;
    VER_INFO 	stTmpVerInfo;

    int 		nCurrVerFileNo = 0;
    int 		nNextVerFileNo = 0;


    LoadVerInfo( &stTmpVerInfo );			// �������� �ε�

    /*
     *  ���� ����
     */
    fdSock = OpenSession( gstCommInfo.abDCSIPAddr, SERVER_COMM_PORT );
    if ( fdSock < 0 )
    {
        LogDCS( "[DownFromDCS] OpenSession() ����\n" );
		printf( "[DownFromDCS] OpenSession() ����\n" );
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
	if ( AuthDCS( fdSock, DOWN_FILE_JOB, &stSendUsrPktMsg,
			&stRecvUsrPktMsg ) != SUCCESS )
    {
        LogDCS( "[DownFromDCS] AuthDCS() ����\n" );
		printf( "[DownFromDCS] AuthDCS() ����\n" );
		CloseSession( fdSock );
		DisplayASCInDownFND( FND_READY_MSG );	// FND �ʱ�ȭ
        return ErrRet( ERR_DCS_AUTH );
    }

    /*
     *  0 - �������
     *  1 - �̷�����
     */
    for ( nVerTypeLoop = 0 ; nVerTypeLoop < 2 ; nVerTypeLoop++ )
    {
        /*
         *  �����ܸ����� ������ ����ý������� �۽��ϰ� ������ �޴´�.
         */
        if ( SendVerInfoPkt( fdSock, 
        					 nVerTypeLoop, 
        					 &stSendUsrPktMsg, 
        					 &stRecvUsrPktMsg )
             != SUCCESS )
        {
        	CloseSession( fdSock );
			DisplayASCInDownFND( FND_READY_MSG );	// FND �ʱ�ȭ
            return ErrRet( ERR_SEND_VER_IFNO );
        }

        while( TRUE )
        {
            /*
             *  ���� ����
             */
            sRetVal = RecvFileFromDCS( fdSock,
                                       &stSendUsrPktMsg,
                                       &stRecvUsrPktMsg,
                                       &boolIsMoreRecvYn,
                                       &nCurrVerFileNo,
                                       &nNextVerFileNo );

            if ( sRetVal != SUCCESS )   // ���� ���� ������ ���
            {
		        LogDCS( "[DownFromDCS] RecvFileFromDCS() ����\n" );
				printf( "[DownFromDCS] RecvFileFromDCS() ����\n" );
                /*
                 *  �̾�ޱⰡ �ƴ� ��� �׳� �����ϰ�
                 *  �̾�ޱ��� ��� ������ update�ϰ� �����Ѵ�.
                 */
                if ( nCurrVerFileNo >= RELAY_RECV_CLASS_NO )
                {
                	UpdateVerFile();    // ���� ������Ʈ
                }
				
                break;
                goto end;
            }
            else    // ���� ���� ������ ��� 
            {
                if ( boolIsMoreRecvYn == FALSE )		// �� ���� ���� ������ 
                {
                    UpdateVerFile();
                    break;
                }
                else       								// �� ���� ���� �����ϸ� 
                {
                    if ( (  ( nCurrVerFileNo >= RELAY_RECV_CLASS_NO &&
                              ( nNextVerFileNo == 0 ) )  ||
                            nNextVerFileNo >= RELAY_RECV_CLASS_NO )
                            )	// �̾�ޱ� ���� ���� ��ȣ �̻��� ��� 
                    {
                        UpdateVerFile();
                    }

                }

            }

        }
    }

    end :

	/*
	 *	�̷���������� ����� ��� ���ε�� ���������� �����Ѵ�.
	 */
    if ( boolIsApplyNextVer == TRUE )
    {
        SetUploadVerInfo();
    }

    CloseSession( fdSock );
	DisplayASCInDownFND( FND_READY_MSG );	// FND �ʱ�ȭ

    WriteCommSuccTime();

    return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SendVerInfoPkt                                           *
*                                                                              *
*  DESCRIPTION :      ���������� �����Ͽ� ����ý������� �۽��ϰ� ������ �޴´�.  *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       bool boolIsCurrVer,                                    *
*                       USER_PKT_MSG* stSendPktMsg,                            *
*                       USER_PKT_MSG* stRecvPktMsg                             *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ErrRet( ERR_SEND_VER_IFNO )                            *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SendVerInfoPkt( int fdSock,
                             bool boolIsCurrVer,
                             USER_PKT_MSG* pstSendUsrPktMsg,
                             USER_PKT_MSG* pstRecvUsrPktMsg )
{
	if ( boolIsCurrVer == 0 )
	{
		printf( "[SendVerInfoPkt] ���� ���������� ����� ����\n" );
	}
	else
	{
		printf( "[SendVerInfoPkt] �̷� ���������� ����� ����\n" );
	}

    pstSendUsrPktMsg->chEncryptionYN = NOT_ENCRYPTION;
    memcpy( pstSendUsrPktMsg->achConnCmd, SEND_VER_INFO_CMD, COMMAND_LENGTH );

    pstSendUsrPktMsg->lDataSize =
                       CreateVerInfoPkt( boolIsCurrVer,
                                         pstSendUsrPktMsg->pbRealSendRecvData );

    if ( SendUsrPkt2DCS( fdSock, TIMEOUT, MAX_RETRY_COUNT, pstSendUsrPktMsg ) 
    	 != SUCCESS )
    {
        return ErrRet( ERR_SEND_VER_IFNO );
    }

    if ( RecvRS( fdSock, pstRecvUsrPktMsg )!= SUCCESS )
    {
        return ErrRet( ERR_SEND_VER_IFNO );
    }

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RecvFileFromDCS                                          *
*                                                                              *
*  DESCRIPTION :      ����ý������κ��� ������ �����Ѵ�.                        *
*                                                                              *
*  INPUT PARAMETERS:    int fdSock,                                            *
*                       USER_PKT_MSG* stSendUsrPktMsg,                         *
*                       USER_PKT_MSG* stRecvUsrPktMsg,                         *
*                       bool* pboolIsMoreRecvYn,                               *
*                       int* pnCurrVerFileNo,                                  *
*                       int* pnNextVerFileNo                                   *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                   ErrRet( ERR_RECV_FILE )                                    *
*                   ErrRet( ERR_FILE_OPEN | GetFileNo( achRecvFileName ) )     *
*                   ErrRet( ERR_FILE_OPEN | GetFileNo( RELAY_DOWN_INFO_FILE )  *
*                   ErrRet( ERR_FILE_WRITE | GetFileNo( achRecvFileName )      *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short RecvFileFromDCS( int fdSock,
                              USER_PKT_MSG* pstSendUsrPktMsg,
                              USER_PKT_MSG* pstRecvUsrPktMsg,
                              bool* pboolIsMoreRecvYn,
                              int* pnCurrVerFileNo,
                              int* pnNextVerFileNo )
{
    short   sRetVal         = SUCCESS;
    short   sReturnVal      = SUCCESS;
    FILE    *fdRecvFile;                        // �ٿ�ε� ���� ����
    FILE    *fdRelayRecv;                       // �̾�ޱ����� ����

    int     nFileNo         = 0;                // �ٿ�ε�� ������ �ε���
    char    achRecvFileName[30] = { 0, };       // ���ŵ� ���ϸ�
    int     nRecvdFileSize  = 0;                // ���ŵ� ������ size
	int     nPktNo          = 0;                // download pkt No.

	long lFreeMemorySize = 0;

    PKT_HEADER_INFO         stPktHeaderInfo;        // ���Ź��� ������ ���
    DOWN_FILE_INFO          stDownFileInfo;         // �ٿ�ε����� ���� ����ü
    FILE_RELAY_RECV_INFO    stFileRelayRecvInfo;    // �̾�ޱ� ���� ����ü

	time_t tStartDtime = 0;
	time_t tEndDtime = 0;

	time( &tStartDtime );

    /*
     *  ������� ����
     */
    sRetVal = RecvFilePkt( fdSock, pstRecvUsrPktMsg, pboolIsMoreRecvYn );
    if( sRetVal > SUCCESS )     // EOF(1), EOT(2), EOS(3)�� ���ŵ� ��� return
    {
        return SUCCESS;
    }
    else if ( sRetVal < SUCCESS )   // ���� ������ ���
    {
        return ErrRet( ERR_RECV_FILE );
    }

    /*
     *  ���� ��� ����
     */
    memcpy( (byte *)&stPktHeaderInfo,
            pstRecvUsrPktMsg->pbRealSendRecvData,
            sizeof( stPktHeaderInfo ) );

    /*
     *  �ٿ�� ������ �ε����� ���ϸ� ����
     */
    nFileNo = GetDownFileIndex( pstRecvUsrPktMsg->achConnCmd );
    GetRecvFileName( pstRecvUsrPktMsg, &stPktHeaderInfo, achRecvFileName );

	printf( "[RecvFileFromDCS] ����κ��� �ٿ�ε� : [%s] [FP : %lu] ",
		achRecvFileName, GetDWORDFromLITTLE( stPktHeaderInfo.abFirstPktNo ) );

    /*
     *  �ٿ�ε� �� ���� ����
     *  ù��Ŷ�� ��ȣ�� 0�� �ƴ� ��� �̾�ޱ� ��
     *  �̾�ޱ��� ��� �ٿ�ε��������Ͽ��� �ش� ���ڵ带 �����ϰ�
     *  ������� ������ size�� �����ϰ�
     *  append mode�� �ٿ�ε� �޴� ������ �����Ѵ�.
     */
    if ( GetDWORDFromLITTLE( stPktHeaderInfo.abFirstPktNo ) != SUCCESS )
    {
        DebugOut( "�̾�޴� �� %ld \n", 
				GetDWORDFromLITTLE( stPktHeaderInfo.abFirstPktNo ) );
		
        DelDownFileList( achRecvFileName, &stDownFileInfo );
        nRecvdFileSize = GetDWORDFromLITTLE( stDownFileInfo.abDownSize );

        if ( ( fdRecvFile = fopen( achRecvFileName, "ab" ) ) == NULL  )
        {
            return ErrRet( ERR_FILE_OPEN | GetFileNo( achRecvFileName ) );
        }
    }
    else
    /*
     *  �̾�ޱ� ���� �ƴ� ���
     *  write mode�� �ٿ�ε� �޴� ������ �����Ѵ�.
     */
    {
        nRecvdFileSize = 0;

        if ( ( fdRecvFile = fopen( achRecvFileName, "wb" ) ) == NULL )
        {
            return ErrRet( ERR_FILE_OPEN | GetFileNo( achRecvFileName ) );
        }

    }

    /*
     *  �̾�ޱ� ���� ������ �����Ѵ�.
     */
    if ( ( fdRelayRecv = fopen( RELAY_DOWN_INFO_FILE, "wb" ) ) == NULL )
    {
        fclose( fdRecvFile );
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RELAY_DOWN_INFO_FILE ) );
    }

    /*
     *  ��������� ���� response �۽�
     */
    sRetVal = SendRS( fdSock, pstSendUsrPktMsg );

    if ( sRetVal != SUCCESS )
    {
        fclose( fdRecvFile );
        fclose( fdRelayRecv );

        return sRetVal;
    }

    /*
     *  �̾�ޱ� ��� ������ ��� ( 38�� �̻��� ���� ) �̾�ޱ� ������ ����
     */
    if ( nFileNo > RELAY_RECV_CLASS_NO )
    {
        /*
         *  ���� �ٿ�ε� �� update�� �̾�ޱ� ������ ����
         */
        SetRelayRecvInfo( &stFileRelayRecvInfo, nFileNo, &stPktHeaderInfo );
        nPktNo = GetDWORDFromLITTLE( stPktHeaderInfo.abFirstPktNo ) - 1 ;

        if ( nPktNo < 0 )
        {
            nPktNo = 0;
        }
    }

    if ( stPktHeaderInfo.bNewVerYN == GetASCNoFromDEC( CURR ) ) // Current
    {
        *pnNextVerFileNo = 0;
        *pnCurrVerFileNo = nFileNo;
    }
    else                                        // next
    {
        *pnNextVerFileNo = nFileNo;
    }

    /*
     *  ���� ���� ����
     */
    while( TRUE )
    {
    	printf( "." );
		fflush( stdout );

        InitPkt( pstRecvUsrPktMsg );
        InitPkt( pstSendUsrPktMsg );

        /*
         *  ����Pkt ����
         */
        sReturnVal = RecvFilePkt( fdSock, pstRecvUsrPktMsg, pboolIsMoreRecvYn );

        /*
         *  EOF(1), EOT(2), EOS(3)�� ���ŵ� ��� break
         */
        if( sReturnVal > SUCCESS )
        {
            sRetVal = SUCCESS;
            break;
        }
        else if ( sReturnVal < SUCCESS )
        {
            fclose( fdRecvFile );
            fclose( fdRelayRecv );
            return ErrRet( ERR_RECV_FILE );
        }

        nRecvdFileSize += pstRecvUsrPktMsg->lDataSize;

        /*
         *  ���ŵ� pkt�� ���Ϸ� write
         */
        sRetVal = WriteRecvdFile( fdRecvFile, pstRecvUsrPktMsg );

        if ( sRetVal != SUCCESS )
        {
            fclose( fdRecvFile );
            fclose( fdRelayRecv );

			// ��ũ ���� ���� ���
        	lFreeMemorySize = MemoryCheck();
			// ��ũ ���� ������ 5MB �̸��̸� ���� ����
			if ( lFreeMemorySize >= 0 && lFreeMemorySize < 5 )
			{
				printf( "[RecvFileFromDCS] ��ũ ���� ���� �������� ���Ͽ� �ٿ���� TEMP ���� ���� : [%s] ", achRecvFileName );
				// TEMP ���� ����
				unlink( achRecvFileName );
				// �̾�ޱ� ������ ��� �̾�ޱ��������� ����
				if ( nFileNo > RELAY_RECV_CLASS_NO )
				{
					unlink( RELAY_DOWN_INFO_FILE );
				}
				// �̺�Ʈ�� ���ͷ� ����
				ctrl_event_info_write( EVENT_INSUFFICIENT_DISK_DURING_DOWNLOAD );
			}

            return sRetVal;
        }

        /*
         *  �̾�ޱ��� ������ ��� �̾�ޱ����� ������ update���ش�
         *  AI������ �̾���� ����
         */
        if ( nFileNo > RELAY_RECV_CLASS_NO &&
             nFileNo < MASTER_AI_FILE_INDEX_NO )
        {
            nPktNo++;
            DWORD2LITTLE( nPktNo, stFileRelayRecvInfo.abDownFilePktNo );

            UpdateRelayRecvFile( fdRelayRecv, &stFileRelayRecvInfo );
        }

//		fflush( fdRecvFile );
        fflush( fdRelayRecv );

        /*
         *  response �۽�
         */
        sRetVal = SendRS( fdSock, pstSendUsrPktMsg );

        if ( sRetVal != SUCCESS )
        {
            break;
        }
    }

    /*
     *  ���ϼ��� ������ ���
     */
    if ( sRetVal != SUCCESS )
    {
    	printf( "e\n" );

        /*
         *  �ٿ�ε� �޴� ������ Ÿ���� �޼����� ���(37������ ����)
         *  �ٿ�ε��������Ͽ� write���� �ʴ´�.
         */
        if ( stPktHeaderInfo.bDataType == DOWN_MSG_TYPE )
        {
            fclose( fdRecvFile );
            fclose( fdRelayRecv );
            remove( achRecvFileName );
            return sReturnVal;
        }

        WriteDownFileInfo(  achRecvFileName,
                            stPktHeaderInfo.abFileVer,
                            UNDER_DOWN,
                            nRecvdFileSize );

    }
    else
    {
    	printf( "/\n" );

        WriteDownFileInfo(  achRecvFileName,
                            stPktHeaderInfo.abFileVer,
                            DOWN_COMPL,
                            nRecvdFileSize );
    }

    fclose( fdRecvFile );
    fclose( fdRelayRecv );

	time( &tEndDtime );
	printf( "[RecvFileFromDCS] ����ð� : %d sec\n", (int)( tEndDtime - tStartDtime ) );

    return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteRecvdFile                                           *
*                                                                              *
*  DESCRIPTION :      ���ŵǴ� ���� ����                                        *
*                                                                              *
*  INPUT PARAMETERS:    File*       fdRecvFile,                                *
*                       USER_PKT_MSG* pstRecvUsrPktMsg                         *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ErrRet( ERR_FILE_WRITE | GetFileNo( achRecvFileName ) )*
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short WriteRecvdFile( FILE*      	fdRecvFile,
                             USER_PKT_MSG* 	pstRecvUsrPktMsg )
{
	int nResult = 0;

	/*
	 *  ���ŵ� ���� pkt�� write
	 */
	nResult = fwrite( pstRecvUsrPktMsg->pbRealSendRecvData,
		pstRecvUsrPktMsg->lDataSize,
		1,
		fdRecvFile );
	if ( nResult < 1 )
	{
		return ERR_FILE_WRITE;
	}

	nResult = fflush( fdRecvFile );
	if ( nResult != SUCCESS )
	{
		return ERR_FILE_WRITE;
	}

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SetRelayRecvInfo                                         *
*                                                                              *
*  DESCRIPTION :      �̾�ޱ� ������ ����                                      *
*                                                                              *
*  INPUT PARAMETERS:    FILE_RELAY_RECV_INFO *pstFileRelayRecvInfo,            *
*                       int nFileNo,                                           *
*                       PKT_HEADER_INFO *pstPktHeaderInfo                      *
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
static void SetRelayRecvInfo( FILE_RELAY_RECV_INFO *pstFileRelayRecvInfo,
                              int nFileNo,
                              PKT_HEADER_INFO *pstPktHeaderInfo )
{
    /*
     *  �̾�ޱ� ��� ������ ��� ( 38�� �̻��� ���� )
     *  �̾�ޱ� ������ ����
     */
    if ( nFileNo > RELAY_RECV_CLASS_NO )
    {
        pstFileRelayRecvInfo->chFileNo = nFileNo;

        memcpy( pstFileRelayRecvInfo->abDownFileVer,
                pstPktHeaderInfo->abFileVer,
                sizeof( pstFileRelayRecvInfo->abDownFileVer ) );

        memcpy( pstFileRelayRecvInfo->abDownFileSize,
                pstPktHeaderInfo->abFileSize,
                sizeof( pstFileRelayRecvInfo->abDownFileSize ) );

    }
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetRelayRecvFileSize                                     *
*                                                                              *
*  DESCRIPTION :      �̾�޴� ���� ������ size�� ���Ѵ�.                       *
*                                                                              *
*  INPUT PARAMETERS:  char chFileNo, long lFileSize                            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_READ | GetFileNo(  RELAY_DOWN_INFO_FILE )     *
*                       ERR_FILE_DATA_NOT_FOUND |                              *
                     		GetFileNo( RELAY_DOWN_INFO_FILE )                  *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short GetRelayRecvFileSize( char chFileNo, long *plFileSize )
{
    short   sReturnVal = SUCCESS;
    int     fdRelayRecvInfoFile;
    int     nReadByte;
    char    achFileName[50] = { 0, };

    FILE_RELAY_RECV_INFO stRelayRecvInfo;

    system( "chmod 755 /mnt/mtd8/bus/?*" );

    fdRelayRecvInfoFile = open( RELAY_DOWN_INFO_FILE, O_RDONLY, OPENMODE );

    if ( fdRelayRecvInfoFile < 0 )
    {
        return  SUCCESS;
    }

    lseek( fdRelayRecvInfoFile, 0, SEEK_SET );

    nReadByte = read( fdRelayRecvInfoFile,
                      (void*)&stRelayRecvInfo,
                      sizeof( FILE_RELAY_RECV_INFO ) );

    if ( nReadByte == 0 )
    {
        sReturnVal = ERR_FILE_READ | GetFileNo(  RELAY_DOWN_INFO_FILE );
    }

    if ( nReadByte < 0 )
    {
        sReturnVal = ERR_FILE_READ | GetFileNo( RELAY_DOWN_INFO_FILE );
    }

    if ( chFileNo != stRelayRecvInfo.chFileNo )
    {
        sReturnVal = ERR_FILE_DATA_NOT_FOUND |
                     GetFileNo( RELAY_DOWN_INFO_FILE );
    }

    close( fdRelayRecvInfoFile );

    /*
     *  ���ϸ� ����
     */
    sprintf( achFileName, "tmp_c_%s",
             achDCSCommFile[(int)chFileNo][1] );
//PrintlnASC( "���� ������ =>", achFileName, 16 );
    *plFileSize = GetFileSize( achFileName );
//printf( "������� %ld\n", GetFileSize( achFileName ));
    return sReturnVal;

}
