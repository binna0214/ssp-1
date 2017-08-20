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
*  PROGRAM ID :       download_file_mgt.c                                     *
*                                                                              *
*  DESCRIPTION:       �� ���α׷��� ���迡�� �ٿ�ε� ���� ������ ����� �����ϴ� *
*						downloadinfo.dat�� CRD�� �̾�ޱ� ������ �����ϴ� 	   *
*						downfilelink.dat�� CU ���, BLPL�� �ٿ�ε� ���ο�	   *
*						SAM ������ �ٿ�ε� ���θ� üũ�ϴ� ����� �����Ѵ�.	   *
*                                                                              *
*  ENTRY POINT:     short WriteDownFileList( DOWN_FILE_INFO* pstDownFileInfo );*
*					short WriteDownFileInfo( char* pachFileName,  			   *
*											 byte* pabFileVer, 				   *
*											 char chDownStatus, 			   *
*											 int nRecvdFileSize );			   *
*					short ReadDownFileList( char* pchFileName,				   *
*											DOWN_FILE_INFO* stDownFileInfo );  *
*					short DelDownFileList( char* pchFileName,				   *
*										   DOWN_FILE_INFO* stDownFileInfo );   *
*					short WriteRelayRecvFile( FILE_RELAY_RECV_INFO* 		   *
*													pstFileRelayRecvInfo );    *
*					short UpdateRelayRecvFile(  FILE* fdRelayRecv,			   *
*                            	FILE_RELAY_RECV_INFO* 	pstFileRelayRecvInfo );*
*					void WriteBLPLDownInfo( void );							   *
*				    static short CheckSAMInfoDown( void );                     *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       downloadinfo.dat  - ���迡�� �ٿ�ε� ���� ������ ���     *
*                     tmp_c_ch_bl.dat   - �ٿ���� ���� BL���ϸ�( ������Ʈ ���� )*
*                     tmp_c_ch_pl.dat   - �ٿ���� ���� PL���ϸ�( ������Ʈ ���� )*
*                     tmp_c_ch_ai.dat   - �ٿ���� ���� AI���ϸ�( ������Ʈ ���� )*
*                                                                              *
*  OUTPUT FILES:      downloadinfo.dat  - ���迡�� �ٿ�ε���� ������ ���      *
*                     downfilelink.dat  - �̾�ޱ� ������ ����                  *
*                     v_bl_be.dat       - ���� BL�� �ٿ�ޱ� ������ ��������     *
*                     v_pl_be.dat       - ���� PL�� �ٿ�ޱ� ������ ��������     *
*                     v_ai_be.dat       - ���� AI�� �ٿ�ޱ� ������ ��������     *
*                     bl_pl_ctrl.flg    - BLPL�� �ٿ�θ� �������ִ� ����      *
*										  blpl_proc.c���� ������				   *
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
#include "download_file_mgt.h"
#include "main_process_busTerm.h"
#include "version_mgt.h"
#include "version_file_mgt.h"
#include "main.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
#define	FILE_NAME_LENGTH		50

#define     BLPL_DOWN       0x01        // BLPL download
#define     CMD_REQ         '1'         // �����޸𸮿� ��û��
#define     CMD_FAIL        '9'         // �����޸𸮿� ��û�� ó�� ����
#define     CMD_COMP        '0'         // �����޸𸮿� ��û�� ó�� �Ϸ�







/*
 *	�̷����� ������ ���������ؾ��ϴ��� ����
 *	FALSE- cannot apply, TRUE - can apply

bool boolIsApplyNextVer 			= 0;
bool boolIsApplyNextVerParm 		= 0;	// �Ķ���� ������ ��ε� ��û
bool boolIsApplyNextVerVoice 		= 0;	// ���������� ROM�� write
bool boolIsApplyNextVerAppl 		= 0;	// ���α׷� �����
bool boolIsApplyNextVerDriverAppl 	= 0;	// ������ ���۱⿡ ���α׷� upload */

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short WriteBLPLDownFlag( char* pachMergeChk );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteDownFileList                                        *
*                                                                              *
*  DESCRIPTION :      downloadinfo.dat�� �ٿ�ε� ���� ���� ������ write         *
*                                                                              *
*  INPUT PARAMETERS:  DOWN_FILE_INFO* pstDownFileInfo                          *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( DOWN_FILE )                 *
*                       ERR_FILE_WRITE | GetFileNo( DOWN_FILE )                *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteDownFileList( DOWN_FILE_INFO* pstDownFileInfo )
{
    short           sReturnVal          = SUCCESS;
    int             fdDownFile;

    fdDownFile = open( DOWN_FILE, O_WRONLY | O_CREAT | O_APPEND, OPENMODE );

    if ( fdDownFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( DOWN_FILE ) );
    }

    if ( write( fdDownFile, (void*)pstDownFileInfo, sizeof( DOWN_FILE_INFO ) )
         != sizeof( DOWN_FILE_INFO ) )
    {
        sReturnVal = ERR_FILE_WRITE | GetFileNo( DOWN_FILE );
    }

    close( fdDownFile );

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteDownFileInfo                                        *
*                                                                              *
*  DESCRIPTION :      �Ķ���� ������ �ٿ�ε��� ����ü�� �����Ͽ� 			   *
*						downloadinfo.dat�� �ٿ�ε� ���� ���� ������ write      *
*                                                                              *
*  INPUT PARAMETERS:  char* pachFileName,  									   *
*					  byte* pabFileVer, 									   *
*					  char chDownStatus, 									   *
*					  int nRecvdFileSize                          			   *
*                                                                              *
*  RETURN/EXIT VALUE:   sReturnVal                                             *
*           			ErrRet( ERR_FILE_OPEN | GetFileNo( DOWN_FILE )         *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteDownFileInfo( char* pachFileName,  
						 byte* pabFileVer, 
						 char chDownStatus, 
						 int nRecvdFileSize )
{
    int 		fdFile;
    short 		sReturnVal 	= SUCCESS;
    DOWN_FILE_INFO  	stDownFileInfo;     	// �ٿ�ε����� ���� ����ü

    memset( &stDownFileInfo, 0, sizeof( DOWN_FILE_INFO ) );
	
    strcpy( stDownFileInfo.achFileName, pachFileName );
    memcpy( stDownFileInfo.abFileVer,
          	pabFileVer,
          	sizeof( stDownFileInfo.abFileVer ) );
    stDownFileInfo.chDownStatus = chDownStatus;
    DWORD2LITTLE( nRecvdFileSize, stDownFileInfo.abDownSize );
    
    fdFile = open( DOWN_FILE, O_WRONLY | O_CREAT | O_APPEND, OPENMODE );
	
    if ( fdFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( DOWN_FILE ) );
    }

    if ( ( write( fdFile, &stDownFileInfo, sizeof( DOWN_FILE_INFO ) )
                ) != sizeof( DOWN_FILE_INFO ) )
    {
        sReturnVal = ERR_FILE_WRITE | GetFileNo( DOWN_FILE );
    }

    close( fdFile );

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      ReadDownFileList                                         *
*                                                                              *
*  DESCRIPTION :      downloadinfo.dat����  pchFileName�� ���� ������ read      *
*                                                                              *
*  INPUT PARAMETERS:  char* pchFileName, - ���� ���ϸ�                          *
*                     DOWN_FILE_INFO* stDownFileInfo - �о���� ����            *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS  - �ش������� ã�� ���                         *
*                       ERR_FILE_OPEN | GetFileNo( DOWN_FILE )                 *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( DOWN_FILE )       *
*                       ERR_FILE_READ | GetFileNo( DOWN_FILE )                 *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short ReadDownFileList( char* pchFileName, DOWN_FILE_INFO* pstDownFileInfo )
{
    short           sReturnVal          = FALSE;
    int             fdDownFile;
    DOWN_FILE_INFO  stReadDownFileInfo;
    int             nReadByte           = 0;

    fdDownFile = open( DOWN_FILE, O_RDONLY, OPENMODE );

    if ( fdDownFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( DOWN_FILE ) );
    }

    while( TRUE )
    {
        nReadByte = read( fdDownFile,
                          &stReadDownFileInfo,
                          sizeof( DOWN_FILE_INFO ) );

        if ( nReadByte == 0 )
        {
            sReturnVal = ERR_FILE_DATA_NOT_FOUND | GetFileNo( DOWN_FILE );
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( DOWN_FILE );
            break;
        }

        if ( nReadByte > 0 && nReadByte < sizeof( DOWN_FILE_INFO ) )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( DOWN_FILE );
            break;
        }
        
        /*
         *  �ش� ������ ã���� ������ copy�ϰ� break��
         */
        if ( strcmp( pchFileName, stReadDownFileInfo.achFileName ) == 0 )
        {
            memcpy( pstDownFileInfo,
                    &stReadDownFileInfo,
                    sizeof( DOWN_FILE_INFO) );

            sReturnVal = SUCCESS;
            break;
        }

    }


    close( fdDownFile );

    return sReturnVal;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DelDownFileList                                          *
*                                                                              *
*  DESCRIPTION :      downloadinfo.dat���� pchFileName�� ã�� �ش� ���ڵ� ����  *
*                                                                              *
*  INPUT PARAMETERS:  char* pchFileName, - ���� ���ϸ�                          *
*                     DOWN_FILE_INFO* stDownFileInfo - ���� ������ ����ü       *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( DOWN_FILE )                 *
*                       ERR_FILE_OPEN | GetFileNo( DOWN_FILE_BACKUP_FILE )     *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( DOWN_FILE )       *
*                       ERR_FILE_READ | GetFileNo( DOWN_FILE )                 *
*                       ERR_FILE_WRITE | GetFileNo( DOWN_FILE_BACKUP_FILE )    *
*                       ERR_FILE_REMOVE | GetFileNo( DOWN_FILE )               *
*                       ERR_FILE_RENAME | GetFileNo( DOWN_FILE_BACKUP_FILE )   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short DelDownFileList( char* pchFileName, DOWN_FILE_INFO* pstDownFileInfo )
{
    short           sReturnVal          = SUCCESS;
    int             fdSrcFile;
    int             fdDestFile;

    int             nReadByte           = 0;
    DOWN_FILE_INFO  stReadDownFileInfo;
    
    /*
     *  downloadinfo.dat open
     */
    fdSrcFile = open( DOWN_FILE, O_RDONLY, OPENMODE );

    if ( fdSrcFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( DOWN_FILE ) );
    }

    /*
     *  downloadino.dat.back open
     */
    fdDestFile = open( DOWN_FILE_BACKUP_FILE, O_WRONLY | O_CREAT, OPENMODE );

    if ( fdDestFile < 0 )
    {
        close( fdSrcFile );
        return ErrRet( ERR_FILE_OPEN | GetFileNo( DOWN_FILE_BACKUP_FILE ) );
    }

    while( TRUE )
    {
        nReadByte = read( fdSrcFile,
                          &stReadDownFileInfo,
                          sizeof( DOWN_FILE_INFO ) );

        if ( nReadByte == 0 )
        {
            sReturnVal = ERR_FILE_DATA_NOT_FOUND | GetFileNo( DOWN_FILE );
            break;
        }

        if ( nReadByte < 0 )
        {
            sReturnVal = ERR_FILE_READ | GetFileNo( DOWN_FILE );
            break;
        }
        
        /*
         *  �ش� ������ ã���� �����͸� copy�� �� continue
         */
        if ( strcmp( pchFileName, stReadDownFileInfo.achFileName ) == 0 )
        {
            memcpy( pstDownFileInfo,
                    &stReadDownFileInfo,
                    sizeof( DOWN_FILE_INFO ) );
            continue;
        }

        nReadByte = write( fdDestFile,
                           &stReadDownFileInfo,
                           sizeof( DOWN_FILE_INFO ) );

        if ( nReadByte < sizeof( DOWN_FILE_INFO ) )
        {
            sReturnVal = ERR_FILE_WRITE | GetFileNo( DOWN_FILE_BACKUP_FILE );
            break;
        }
    }

    close( fdSrcFile );
    close( fdDestFile );

    if ( ( remove( DOWN_FILE ) ) < 0 )
    {
        sReturnVal = ERR_FILE_REMOVE | GetFileNo( DOWN_FILE );
        return sReturnVal;
    }

    if ( ( rename( DOWN_FILE_BACKUP_FILE, DOWN_FILE ) ) < 0 )
    {
        sReturnVal = ERR_FILE_RENAME | GetFileNo( DOWN_FILE_BACKUP_FILE );
    }

    return sReturnVal;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteRelayRecvFile                                       *
*                                                                              *
*  DESCRIPTION :      �̾�ޱ� ����(pstFileRelayRecvInfo)�� downfilelink.dat��  *
*					    write                  								   *
*                                                                              *
*  INPUT PARAMETERS:  FILE_RELAY_RECV_INFO* pstFileRelayRecvInfo               *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( RELAY_DOWN_INFO_FILE )      *
*                       ERR_FILE_WRITE | GetFileNo( RELAY_DOWN_INFO_FILE )     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteRelayRecvFile( FILE_RELAY_RECV_INFO* pstFileRelayRecvInfo )
{
    short           sReturnVal          = SUCCESS;
    int             fdRelayRecvFile;

    fdRelayRecvFile = open( RELAY_DOWN_INFO_FILE,
                            O_WRONLY | O_CREAT,
                            OPENMODE );

    if ( fdRelayRecvFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( RELAY_DOWN_INFO_FILE ) );
    }

    lseek ( fdRelayRecvFile, 0, SEEK_SET );

    if ( write( fdRelayRecvFile,
                (void*)pstFileRelayRecvInfo,
                sizeof( RELAY_DOWN_INFO_FILE ) )
         != sizeof( RELAY_DOWN_INFO_FILE ) )
    {
        sReturnVal = ERR_FILE_WRITE | GetFileNo( RELAY_DOWN_INFO_FILE );
    }

    close( fdRelayRecvFile );

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      UpdateRelayRecvFile                                      *
*                                                                              *
*  DESCRIPTION :      �̾�ޱ� �߿� �̾�ޱ� ����(pstFileRelayRecvInfo)�� update *
*                                                                              *
*  INPUT PARAMETERS:  FILE *fdRelayRecv,  - downfilelink.dat                   *
*                     FILE_RELAY_RECV_INFO* pstFileRelayRecvInfo               *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_WRITE | GetFileNo( RELAY_DOWN_INFO_FILE )     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short UpdateRelayRecvFile(  FILE*					fdRelayRecv,
                            FILE_RELAY_RECV_INFO* 	pstFileRelayRecvInfo )
{
    short           sReturnVal          = SUCCESS;

    fseek( fdRelayRecv, 0L, SEEK_SET );

    if ( fwrite( pstFileRelayRecvInfo,
                 sizeof( FILE_RELAY_RECV_INFO ),
                 1,
                 fdRelayRecv
               ) != 1 )
    {
        sReturnVal = ERR_FILE_WRITE | GetFileNo( RELAY_DOWN_INFO_FILE );
    }

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckBLPLDown                                            *
*                                                                              *
*  DESCRIPTION :      BLPL������ �ٿ�ε� ������ ���� ������ ���Ϸ� �����  	   *
*						bl_pl_ctrl.flg�� �ٿ� ���� flag�� �����.               *
*						BL ���� ���� - v_bl_be.dat							   *
*						PL ���� ���� - v_pl_be.dat							   *
*						AI ���� ���� - v_ai_be.dat							   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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
void WriteBLPLDownInfo( void )
{
    short	sReturnVal	= SUCCESS;
    /*
     *  achMergeChk [0] - BL
     *  achMergeChk [1] - PL
     *  achMergeChk [2] - AI
     */
    char    		achMergeChk[3]      = { 0, };
	DOWN_FILE_INFO	stReadDownFileInfo;
	
	/*
	 *	���� BL �ٿ�ε� ���� üũ
	 */
    sReturnVal = ReadDownFileList( DOWN_ING_UPDATE_BL_FILE, 
    							   &stReadDownFileInfo );
    
    /*
     *	���� BL�� �ٿ�ε� �޾Ұ� �ٿ�ε尡 �Ϸ�Ǿ��ٸ� ���������� write�ϰ�
     *	download flag�� write�Ѵ�.
     */
    if ( sReturnVal == SUCCESS && 
    	 stReadDownFileInfo.chDownStatus == DOWN_COMPL )
    {

        achMergeChk[0] = BLPL_DOWN;

        /*
         *	�ٿ�ε� ���� ������ �������� write
         */
        sReturnVal= WriteBLPLBefVer( UPDATE_BL_BEFORE_VER_FILE,
									 stVerInfo.abUpdateBLVer,
			                         sizeof( stVerInfo.abUpdateBLVer ) );

		if ( sReturnVal < SUCCESS )
		{
			DelDownFileList( DOWN_ING_UPDATE_BL_FILE, &stReadDownFileInfo );
			unlink ( DOWN_ING_UPDATE_BL_FILE );
		}
		
        /*
         *	BLPL �ٿ�ε� flag�� write
         */
		sReturnVal = WriteBLPLDownFlag( achMergeChk );
		
		if ( sReturnVal < SUCCESS )
		{
			DelDownFileList( DOWN_ING_UPDATE_BL_FILE, &stReadDownFileInfo );
			unlink ( DOWN_ING_UPDATE_BL_FILE );
			unlink ( UPDATE_BL_BEFORE_VER_FILE );
		}

	}
	
	/*
	 *	���� PL �ٿ�ε� ���� üũ
	 */
    sReturnVal = ReadDownFileList( DOWN_ING_UPDATE_PL_FILE, 
    							   &stReadDownFileInfo );
    
    /*
     *	���� PL�� �ٿ�ε� �޾Ұ� �ٿ�ε尡 �Ϸ�Ǿ��ٸ� ���������� write�ϰ�
     *	download flag�� write�Ѵ�.
     */
    if ( sReturnVal == SUCCESS && 
    	 stReadDownFileInfo.chDownStatus == DOWN_COMPL )
    {

        achMergeChk[1] = BLPL_DOWN;

        /*
         *	�ٿ�ε� ���� ������ �������� write
         */
		sReturnVal= WriteBLPLBefVer( UPDATE_PL_BEFORE_VER_FILE,
									 stVerInfo.abUpdatePLVer,
			                         sizeof( stVerInfo.abUpdatePLVer ) );

		if ( sReturnVal < SUCCESS )
		{
			DelDownFileList( DOWN_ING_UPDATE_PL_FILE, &stReadDownFileInfo );
			unlink ( DOWN_ING_UPDATE_PL_FILE );
		}
		
        /*
         *	BLPL �ٿ�ε� flag�� write
         */
		sReturnVal = WriteBLPLDownFlag( achMergeChk );
		
		if ( sReturnVal < SUCCESS )
		{
			DelDownFileList( DOWN_ING_UPDATE_PL_FILE, &stReadDownFileInfo );
			unlink ( DOWN_ING_UPDATE_PL_FILE );
			unlink ( UPDATE_PL_BEFORE_VER_FILE );
		}
	}
	
	/*
	 *	���� AI �ٿ�ε� ���� üũ
	 */
    sReturnVal = ReadDownFileList( DOWN_ING_UPDATE_AI_FILE, 
    							   &stReadDownFileInfo );
    
    /*
     *	���� AI�� �ٿ�ε� �޾Ұ� �ٿ�ε尡 �Ϸ�Ǿ��ٸ� ���������� write�ϰ�
     *	download flag�� write�Ѵ�.
     */
    if ( sReturnVal == SUCCESS && 
    	 stReadDownFileInfo.chDownStatus == DOWN_COMPL )
    {

        achMergeChk[2] = BLPL_DOWN;
        
        /*
         *	�ٿ�ε� ���� ������ �������� write
         */
		sReturnVal= WriteBLPLBefVer( UPDATE_AI_BEFORE_VER_FILE,
									 stVerInfo.abUpdateAIVer,
			                         sizeof( stVerInfo.abUpdateAIVer ) );

		if ( sReturnVal < SUCCESS )
		{
			DelDownFileList( DOWN_ING_UPDATE_AI_FILE, &stReadDownFileInfo );
			unlink ( DOWN_ING_UPDATE_AI_FILE );
		}
		
        /*
         *	BLPL �ٿ�ε� flag�� write
         */
		sReturnVal = WriteBLPLDownFlag( achMergeChk );
		
		if ( sReturnVal < SUCCESS )
		{
			DelDownFileList( DOWN_ING_UPDATE_AI_FILE, &stReadDownFileInfo );
			unlink ( DOWN_ING_UPDATE_AI_FILE );
			unlink ( UPDATE_AI_BEFORE_VER_FILE );
		}
	}
	
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteBLPLDownFlag                                        *
*                                                                              *
*  DESCRIPTION :      merge_flag.dat���Ͽ� BL/PL/AI download ���θ� write       *
*					  BLPL�� ��� �� ������ �ٿ�ε� ������ update�ϰ� �Ǿ�����  *
*					  �Ƿ� flag�� �� byte�� write�ؾ���( ������ �ٿ���� flag��  *
*					  ���� �� ��� �����Ƿ� )									   *
*                                                                              *
*  INPUT PARAMETERS:  	char* pachMergeChk  - BL/PL/AI download ����           *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( MERGE_FLAG_FILE )           *
*                       ERR_FILE_WRITE | GetFileNo( MERGE_FLAG_FILE )          *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short WriteBLPLDownFlag( char* pachMergeChk )
{
	short   sReturnVal          = SUCCESS;
    int     fdMergeFlagFile     = 0;  
    
    /*
     *  Merge Flag File ����
     */
    if ( pachMergeChk[0] == BLPL_DOWN ||
         pachMergeChk[1] == BLPL_DOWN ||
         pachMergeChk[2] == BLPL_DOWN )
    {
    	
        fdMergeFlagFile = open( MERGE_FLAG_FILE, O_RDWR | O_CREAT, OPENMODE );

        if ( fdMergeFlagFile < 0 )
        {
            return ErrRet( ERR_FILE_OPEN | GetFileNo( MERGE_FLAG_FILE ) );
        }

		/*
		 *	���� BL �ٿ�ε� �޾��� ��� 
		 */
        if ( pachMergeChk[0] == BLPL_DOWN )
        {
            lseek( fdMergeFlagFile, 0, SEEK_SET );
            
            sReturnVal = write( fdMergeFlagFile,
                                &pachMergeChk[0],
                                sizeof( pachMergeChk[0] ) );

            if ( sReturnVal < sizeof( pachMergeChk[0] ) )
            {
            	LogDCS( "[WriteBLPLDownFlag] ����BL ���� FLAG WRITE ����\n" );
				printf( "[WriteBLPLDownFlag] ����BL ���� FLAG WRITE ����\n" );
            	close( fdMergeFlagFile );
            	return ErrRet( ERR_FILE_WRITE | GetFileNo( MERGE_FLAG_FILE ) );
                
            }
        }

		/*
		 *	���� PL �ٿ�ε� �޾��� ��� 
		 */
        if ( pachMergeChk[1] == BLPL_DOWN )
        {
            lseek( fdMergeFlagFile, 1, SEEK_SET );
            sReturnVal = write( fdMergeFlagFile,
                                &pachMergeChk[1],
                                sizeof( pachMergeChk[1] ) );

            if ( sReturnVal < sizeof( pachMergeChk[1] ) )
            {
            	LogDCS( "[WriteBLPLDownFlag] ����PL ���� FLAG WRITE ����\n" );
				printf( "[WriteBLPLDownFlag] ����PL ���� FLAG WRITE ����\n" );
            	close( fdMergeFlagFile );
            	return ErrRet( ERR_FILE_WRITE | GetFileNo( MERGE_FLAG_FILE ) );
                
            }
        }

		/*
		 *	���� AI �ٿ�ε� �޾��� ��� 
		 */
        if ( pachMergeChk[2] == BLPL_DOWN )
        {
            lseek( fdMergeFlagFile, 2, SEEK_SET );
            sReturnVal = write( fdMergeFlagFile,
                                &pachMergeChk[2],
                                sizeof( pachMergeChk[2] ) );

            if ( sReturnVal < sizeof( pachMergeChk[2] ) )
            {
            	LogDCS( "[WriteBLPLDownFlag] �����ż���PL ���� FLAG WRITE ����\n" );
				printf( "[WriteBLPLDownFlag] �����ż���PL ���� FLAG WRITE ����\n" );
            	close( fdMergeFlagFile );
            	return ErrRet( ERR_FILE_WRITE | GetFileNo( MERGE_FLAG_FILE ) );
                
            }
        }

        close( fdMergeFlagFile );
    }
    
    return sReturnVal;
	
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CheckSAMInfoDown                                         *
*                                                                              *
*  DESCRIPTION :      SAM������ �ٿ�ε� �޾Ҵ��� ���� üũ 		               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( DOWN_FILE )                 *
*                       ERR_FILE_DATA_NOT_FOUND | GetFileNo( DOWN_FILE )       *
*                       ERR_FILE_READ | GetFileNo( DOWN_FILE )                 *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short CheckSAMInfoDown( void )
{
    short           sReturnVal          = SUCCESS;

    DOWN_FILE_INFO  stDownFileInfo;                 // download info
    // tmp_c_idcenter.dat ���ϸ�
    char            achDownFileName[FILE_NAME_LENGTH] = { 0, };   

    byte            SharedMemoryCmd;
    char            achCmdData[6]       = { 0, };
    word            wCmdDataSize;
    int             i                   = 0;

    /*
     *  tmp_c_idcenter.dat ���ϸ� ����
     */
    sprintf( achDownFileName, "%s", TMP_EPURSE_ISSUER_REGIST_INFO_FILE );

    /*
     *  id_center�� keyset �ι� ����
     */
    for ( i = 0 ; i < 2 ; i++ )
    {
        /*
         *  download���� read
         */
        sReturnVal = ReadDownFileList( achDownFileName, &stDownFileInfo );

        /*
         *  �ٿ��� �޾Ҵٸ�
         */
        if ( sReturnVal == SUCCESS )
        {
            achCmdData[0] = CMD_REQ;

            while( TRUE )
            {
                /*
                 *  keyset ��� ��û
                 */
                sReturnVal = SetSharedCmdnData( CMD_KEYSET_IDCENTER_WRITE,
                                                &achCmdData[0],
                                                sizeof( achCmdData[0] ) );

                if( sReturnVal == SUCCESS )
                {
                    break;
                }
            }

            while( TRUE )
            {
                /*
                 *  ó�� ��� üũ
                 */
                GetSharedCmd( &SharedMemoryCmd, achCmdData, &wCmdDataSize );

                /*
                 *  ó���� ������ ���
                 */
                if( ( SharedMemoryCmd == CMD_KEYSET_IDCENTER_WRITE )
                    && ( achCmdData[ 0 ] == CMD_FAIL
                         || achCmdData[ 0 ] == CMD_COMP ) )
                {
                    /*
                     *  ��û ������ clear
                     */
                    ClearSharedCmdnData( CMD_KEYSET_IDCENTER_WRITE );

                    if( achCmdData[0] == CMD_FAIL )
                    {
                        /*
                         *  main term PSAM ��� ���� ����
                         */
                        boolIsRegistMainTermPSAM = FALSE;

                        /*
                         *  SAM version������ �ʱ�ȭ���Ѽ� ���� ������Ž�
                         *  ���� �ٿ�ε� ���� �� �ֵ��� �Ѵ�.
                         */
                        InitSAMRegistInfoVer( );
                    }
                    else if( achCmdData[0] == CMD_COMP )
                    {
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

            break;

        }

        /*
         *  tmp_c_keyset.dat���� ���ϸ� ����
         */
        sprintf( achDownFileName, "%s", TMP_PSAM_KEYSET_INFO_FILE );
    }

    return sReturnVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteOperParmFile                                        *
*                                                                              *
*  DESCRIPTION :      ����ý��ۿ��� �ٿ���� �Ķ���� ������ write              *
*                                                                              *
*  INPUT PARAMETERS:    byte* pbBuff    - �ٿ�ε� ���� �޼���                  *
*                       long lBuffSize  - �޼����� size                        *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( VEHICLE_PARM_FILE )         *
*                       ERR_FILE_WRITE | GetFileNo( VEHICLE_PARM_FILE )        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short WriteOperParmFile( byte* pbBuff, long lBuffSize )
{
    int     fdOperParmFile;

    /*
     *  � �Ķ���� ���� ����
     */

    fdOperParmFile = open( VEHICLE_PARM_FILE, O_WRONLY | O_CREAT, OPENMODE );

    if ( fdOperParmFile < 0 )
    {
        return ErrRet( ERR_FILE_OPEN | GetFileNo( VEHICLE_PARM_FILE ) );
    }

    /*
     *  ��Ķ���� ���� write
     */
    if ( write( fdOperParmFile, pbBuff, lBuffSize ) != lBuffSize )
    {
        close ( fdOperParmFile );
        return ErrRet( ERR_FILE_WRITE | GetFileNo( VEHICLE_PARM_FILE ) );
    }

    close ( fdOperParmFile );

    return SUCCESS;
}
