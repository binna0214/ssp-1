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
*  PROGRAM ID :       reconcile_file_mgt.c                                    *
*                                                                              *
*  DESCRIPTION:       �� ���α׷��� �������� ������ CRUD�� ����� ���� �Ѵ�.      *
*                                                                              *
*  ENTRY POINT:       WriteReconcileFileList                                   *
*                     ReadReconcileFileList                                    *
*                     UpdateReconcileFileList                                  *
*                     DelReconcileFileList                                     *
*                     CopyReconcileFile                                        *
*                     LoadReconcileFileList                                    *
*                     GetReconcileCnt                                          *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       reconcileinfo.dat                                        *
*                                                                              *
*  OUTPUT FILES:      reconcileinfo.dat                                        *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2005/09/03 Solution Team Mi Hyun Noh  Initial Release                        *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "reconcile_file_mgt.h"


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteReconcileFileList                                   *
*                                                                              *
*  DESCRIPTION :      �������� ���Ͽ� ����ý��ۿ� ���ε��� ���ϸ��� write�Ѵ�.   *
*                                                                              *
*  INPUT PARAMETERS:  RECONCILE_DATA* stReconcileData - write�� reconcile����ü *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE )  *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_WRITE | GetFileNo( RECONCILE_FILE )           *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteReconcileFileList( RECONCILE_DATA* pstReconcileData )
{
    short           sReturnVal          = SUCCESS;
    int             fdReconcileFile;
    RECONCILE_DATA  stReadReconcileData;        // �о���� �������� ����ü
    long            lOffset             = 0;    // read ���� ��ġ
    int             nReadByte          = 0;    // read�� byte��
    bool            boolIsWrite         = FALSE;// write ����

    fdReconcileFile = open( RECONCILE_FILE, O_RDWR | O_CREAT, OPENMODE );

    if ( fdReconcileFile < 0 )
    {
        /*
         *  file open�� ������ �߻��ϸ� �����ڵ�� �Բ� �����ڵ带 ����
         */
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    /*
     *  reconcile file�� write�ϱ� ���� lock�� �Ǵ�.
     */
    LockFile( fdReconcileFile );

	/*
	 *	������ ��ϵ��� ��� update�ϵ��� �Ѵ�.
	 */
    while( TRUE )
    {
        /*
         *  �ŷ����� ������ �۽Ŵ�� ������ ���
         */
        if ( pstReconcileData->bSendStatus == RECONCILE_SEND_WAIT )
        {
            break;
        }

        nReadByte = read( fdReconcileFile,
                          (void*)&stReadReconcileData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            sReturnVal = ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE );
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        /*
         *  write�� ���ϸ�� read�� ���ϸ��� ���� ��� �����
         */
        if ( strcmp( pstReconcileData->achFileName,
                     stReadReconcileData.achFileName
                   ) == 0 )
        {
            /*
             *  read�� ���� ��ġ ã��
             */
            lseek ( fdReconcileFile, lOffset, SEEK_SET );

            if ( ( write( fdReconcileFile,
                          (void*)pstReconcileData,
                          sizeof( RECONCILE_DATA ) )
                        ) != sizeof( RECONCILE_DATA ) )
            {
                sReturnVal = ERR_FILE_WRITE | GetFileNo( RECONCILE_FILE );
            }

            boolIsWrite = TRUE;
            break;

        }

        /*
         *  ���� read ���� ��ġ ���
         */
        lOffset += sizeof( RECONCILE_DATA );
    }

    /*
     *  ����Ⱑ �� �� ���
     */
    if ( boolIsWrite == FALSE )
    {
        /*
         *  ������ �� ��ġ ã��
         */
        lseek ( fdReconcileFile, 0, SEEK_END );

        if ( ( write( fdReconcileFile,
                      (void*)pstReconcileData,
                      sizeof( RECONCILE_DATA ) )
                    ) != sizeof( RECONCILE_DATA ) )
        {
            sReturnVal = ERR_FILE_WRITE | GetFileNo( RECONCILE_FILE );
        }
    }

    // ��������� Ǭ��.
    UnlockFile( fdReconcileFile );

    close( fdReconcileFile );

    return ErrRet( sReturnVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReadReconcileFileList                                    *
*                                                                              *
*  DESCRIPTION :      �ش� ���ϸ��� �������� ���ڵ带 �д´�.                    *
*                                                                              *
*  INPUT PARAMETERS:  char* pchFileName, - read�� ���ϸ�                        *
*                     RECONCILE_DATA* pstReconcileData  - �о���� ����ü       *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE )  *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short ReadReconcileFileList( char* pchFileName,
                             RECONCILE_DATA* pstReconcileData )
{
    short           sReturnVal          = SUCCESS;
    int             fdReconcileFile;
    RECONCILE_DATA  stReadReconcileData;            // �о���� �������� ����ü
    int             nReadByte           = 0;        // read�� byte��

    fdReconcileFile = open( RECONCILE_FILE, O_RDONLY, OPENMODE );

    if ( fdReconcileFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    while( TRUE )
    {
        nReadByte = read( fdReconcileFile,
                          (void*)&stReadReconcileData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            sReturnVal =  ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE );
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        /*
         *  read�� ���ϸ�� read�� ���ϸ��� ���� ���
         */
        if ( strcmp( pchFileName,
                     stReadReconcileData.achFileName
                   ) == 0  )
        {
            memcpy( pstReconcileData,
                    &stReadReconcileData,
                    sizeof( RECONCILE_DATA ) );
            break;
        }
    }

    close( fdReconcileFile );

    return ErrRet( sReturnVal );

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      UpdateReconcileFileList                                  *
*                                                                              *
*  DESCRIPTION :      ������ ��ϵ� �������� ���ڵ带 ã�� update�Ѵ�.           *
*                                                                              *
*  INPUT PARAMETERS:  char* pchFileName, - update�� ���ϸ�                     *
*                     RECONCILE_DATA* pstReconcileData  - ������Ʈ�� ����ü     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE )  *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_WRITE | GetFileNo( RECONCILE_FILE )           *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short UpdateReconcileFileList( char* pchFileName,
                               RECONCILE_DATA* pstReconcileData )
{
    short           sReturnVal          = SUCCESS;
    int             fdReconcileFile;
    RECONCILE_DATA  stReadReconcileData;            // �о���� �������� ����ü
    int             nReadByte           = 0;        // read�� byte��
    long            lOffset             = 0;        // read ���� ��ġ

    fdReconcileFile = open( RECONCILE_FILE, O_RDWR, OPENMODE );

    if ( fdReconcileFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    /*
     *  reconcile file�� write�ϱ� ���� lock�� �Ǵ�.
     */
    LockFile( fdReconcileFile );

    while( TRUE )
    {
        nReadByte = read( fdReconcileFile,
                          (void*)&stReadReconcileData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            sReturnVal = ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE );
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        /*
         *  update ���ϸ�� read�� ���ϸ��� ���� ��� �����
         */
        if ( strcmp( pchFileName,
                     stReadReconcileData.achFileName
                   ) == 0 )
        {
            /*
             *  read�� ���� ��ġ ã��
             */
            lseek ( fdReconcileFile, lOffset, SEEK_SET );

            if ( ( write( fdReconcileFile,
                          (void*)pstReconcileData,
                          sizeof( RECONCILE_DATA ) )
                        ) != sizeof( RECONCILE_DATA ) )
            {
                sReturnVal = ERR_FILE_WRITE | GetFileNo( RECONCILE_FILE );
            }

            break;

        }

        /*
         *  ���� read ���� ��ġ ���
         */
        lOffset += sizeof( RECONCILE_DATA );
    }

    // ��������� Ǭ��.
    UnlockFile( fdReconcileFile );

    close( fdReconcileFile );

    return ErrRet( sReturnVal );

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DelReconcileFileList                                     *
*                                                                              *
*  DESCRIPTION :      �ش� ���ϸ��� ���ڵ带 �����Ѵ�.		                   *
*                                                                              *
*  INPUT PARAMETERS:  char* pchFileName, - ������ ���ϸ�                        *
*                     RECONCILE_DATA* pstReconcileData  - ������ ������ ����ü  *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS,                                               *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE )  *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_WRITE | GetFileNo( RECONCILE_BACKUP_FILE )    *
*                       ERR_FILE_REMOVE | GetFileNo( RECONCILE_FILE )          *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short DelReconcileFileList( char* pchFileName,
                            RECONCILE_DATA* pstReconcileData )
{
    short           sReturnVal          = SUCCESS;
    int             fdSourceFile;                   // reconcileinfo.dat
    int             fdDestFile;                     // reconcileinfo.dat.back

    char            chTmpReconcileFileName[40];     // reconcileinfo.dat.back
    RECONCILE_DATA  stReadSourceReconcileData;
    int             nReadByte          = 0;        // read�� byte��

    /*
     *  �ӽ����ϸ� reconcileinfo.dat.back ����
     */
    sprintf( chTmpReconcileFileName, "%s.back", RECONCILE_FILE );

    /*
     *   reconcileinfo.dat file open
     */
    fdSourceFile = open( RECONCILE_FILE, O_RDONLY, OPENMODE );

    if ( fdSourceFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    /*
     *  reconcileinfo.dat.back file open
     */
    fdDestFile = open( chTmpReconcileFileName, O_WRONLY | O_CREAT, OPENMODE );

    if ( fdDestFile < 0 )
    {
        close( fdSourceFile );
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_BACKUP_FILE ) );
    }

    while( TRUE )
    {

        nReadByte = read( fdSourceFile,
                          (void*)&stReadSourceReconcileData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            sReturnVal = ERR_FILE_DATA_NOT_FOUND | GetFileNo( RECONCILE_FILE );
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        /*
         *  ������ ���ϸ��� �о���� ���ϸ�� ������
         */
        if ( strcmp( pchFileName,
                     stReadSourceReconcileData.achFileName
                    ) == 0 )
        {
            memcpy( pstReconcileData,
                    &stReadSourceReconcileData,
                    sizeof( RECONCILE_DATA ) );
            continue;
        }

        /*
         *  �ش� ���ϸ��� ������ ������ write
         */
        nReadByte = write( fdDestFile,
                           (void*)&stReadSourceReconcileData,
                           sizeof( RECONCILE_DATA ) );

        if ( nReadByte < sizeof( RECONCILE_DATA ) )
        {
            sReturnVal = ERR_FILE_WRITE | GetFileNo( RECONCILE_BACKUP_FILE );
            break;
        }
    }

    close( fdSourceFile );
    close( fdDestFile );

	/*
	 *	��� ������ ������ �����Ѵ�.
	 */
    if ( ( remove( RECONCILE_FILE ) ) < 0 )
    {
        sReturnVal = ERR_FILE_REMOVE | GetFileNo( RECONCILE_FILE );
        return ErrRet( sReturnVal );
    }

	/*
	 *	��� ������ ������ �����Ϸ� rename
	 */
    if ( ( rename( chTmpReconcileFileName, RECONCILE_FILE ) ) < 0 )
    {
        sReturnVal = ERR_FILE_RENAME | GetFileNo( RECONCILE_BACKUP_FILE );
    }


    return ErrRet( sReturnVal );

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CopyReconcileFile                                        *
*                                                                              *
*  DESCRIPTION :      �������� ����� pchTmpFileName�� copy�Ѵ�.			       *
*                                                                              *
*  INPUT PARAMETERS:  char* pchTmpFileName                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_WRITE | GetFileNo( RECONCILE_TMP_FILE )       *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS : �������� �۾� ���� ������ tmp���Ϸ� copy�ؼ� �۾��Ѵ�.              *
*                                                                              *
*******************************************************************************/
short CopyReconcileFile( char* pchTmpFileName )
{
    short           sReturnVal          = SUCCESS;
    int             fdSourceFile;                   // reconcileinfo.dat
    int             fdDestFile;                     // reconcileinfo.dat.back
    RECONCILE_DATA  stReadReconcileData;
    int             nReadByte          = 0;        // read�� byte��

    /*
     *  reconcileinfo.dat file open
     */
    fdSourceFile = open( RECONCILE_FILE, O_RDONLY, OPENMODE );

    if ( fdSourceFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    /*
     *  reconcileinfo.dat.tmp file open
     */
    fdDestFile = open( pchTmpFileName, O_WRONLY | O_CREAT | O_TRUNC, OPENMODE );

    if ( fdDestFile < 0 )
    {
        close( fdSourceFile );
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_TMP_FILE ) );
    }

    while( TRUE )
    {
        nReadByte = read (  fdSourceFile,
                            (void*)&stReadReconcileData,
                            sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        nReadByte = write( fdDestFile,
                          (void*)&stReadReconcileData,
                           sizeof( RECONCILE_DATA ) );

        if ( nReadByte < sizeof( RECONCILE_DATA ) )
        {
            sReturnVal = ERR_FILE_WRITE | GetFileNo( RECONCILE_TMP_FILE );
            break;
        }

    }

    close( fdSourceFile );
    close( fdDestFile );

    return ErrRet( sReturnVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadReconcileFileList                                    *
*                                                                              *
*  DESCRIPTION :      �������� ����� �ε��Ѵ�.				                   *
*                                                                              *
*  INPUT PARAMETERS:    RECONCILE_DATA* pstReconcileData,					   *
*						int* nReconcileCnt, - reconcile�� ��ϵ� ���ڵ� ��      *
*                       int* pnFileCnt,     - upload�� ����                    *
*                       int* pnMsgCnt       - �� upload ����                   *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadReconcileFileList( RECONCILE_DATA* pstReconcileData,
							 int* nReconcileCnt,
							 int* pnFileCnt,
							 int* pnMsgCnt )
{

    short           sReturnVal          = SUCCESS;
    int             fdReconcileFile;
    RECONCILE_DATA  stReadReconcileData;
    int             nReadByte          = 0;        // read�� byte��

    *nReconcileCnt  = 0;
    *pnFileCnt      = 0;
    *pnMsgCnt       = 0;

    fdReconcileFile = open( RECONCILE_FILE, O_RDONLY, OPENMODE );

    if ( fdReconcileFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    while( TRUE )
    {
        nReadByte = read( fdReconcileFile,
                          (void*)&stReadReconcileData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        *nReconcileCnt += 1;

        /*
         *  load reconcileinfo.dat into RECONCILE_DATA_LOAD
         */
        memcpy( &(pstReconcileData[*nReconcileCnt-1]),
                      &stReadReconcileData,
                sizeof( RECONCILE_DATA ) );

        switch ( stReadReconcileData.bSendStatus )
        {
            case RECONCILE_SEND_WAIT:       // TR wait
            case RECONCILE_RESEND_REQ:      // TR resend request
            case RECONCILE_SEND_SETUP:      // setup registration File
            case RECONCILE_SEND_ERR_LOG:    // terminal error data File
            case RECONCILE_SEND_VERSION:    // terminal Version File
            case RECONCILE_SEND_GPS:        // GPS
            case RECONCILE_SEND_GPSLOG:     // GPS LOG
            case RECONCILE_SEND_GPSLOG2:    // GPS LOG2
            case RECONCILE_SEND_TERM_LOG:   // terminal log
            case RECONCILE_SEND_STATUS_LOG: // status log
                *pnFileCnt += 1;
                break;
            case RECONCILE_SEND_COMP:   // TR send completion Recincile wait
            case RECONCILE_RESEND_COMP: // TR resend completion Reconcile wait
                *pnMsgCnt += 1;
                break;
        }
    }

    close( fdReconcileFile );

    return sReturnVal;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetReconcileCnt                                          *
*                                                                              *
*  DESCRIPTION :      �������Ͽ� ��ϵ� ���ڵ� ������ �ŷ������� �۽Ŵ�� �����  *
*						������ count�Ѵ�.				                       *
*                                                                              *
*  INPUT PARAMETERS:  int *pnRecCnt   - reconcileinfo.dat�� ��ϵ� ���ڵ� ��    *
*                                                                              *
*  RETURN/EXIT VALUE:   nSendWaitCnt    - upload�� �ŷ����� ��                  *
*                       ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE )            *
*                       ERR_FILE_READ | GetFileNo( RECONCILE_FILE )            *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short GetReconcileCnt( int *pnRecCnt )
{
 
    int             fdReconcileFile;
    RECONCILE_DATA  stReadReconcileData;
    int             nReadByte           = 0;        // read�� byte��
    short          sSendWaitCnt        = 0;

    DebugOut( "GetReconcileCnt start \n" );

    fdReconcileFile = open( RECONCILE_FILE, O_RDONLY, OPENMODE );

    if ( fdReconcileFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RECONCILE_FILE ) );
    }

    while( TRUE )
    {
        nReadByte = read( fdReconcileFile,
                                    &stReadReconcileData,
                          sizeof( RECONCILE_DATA ) );

        if ( nReadByte == 0 )
        {
            break;
        }

        if ( nReadByte < 0 )
        {
            sSendWaitCnt = ERR_FILE_READ | GetFileNo( RECONCILE_FILE );
            break;
        }

        *pnRecCnt = *pnRecCnt + 1;

        /*
         *  ���ε� ������� �ŷ����� ��
         */
        if ( stReadReconcileData.bSendStatus == RECONCILE_SEND_WAIT )
        {
            sSendWaitCnt++;
        }

    }

    close( fdReconcileFile );

    return sSendWaitCnt;

}
