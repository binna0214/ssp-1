/*******************************************************************************
*                                                                              *
*                      KSCC - Bus Embeded System                               *
*                                                                              *
*            All rights reserved-> No part of this publication may be          *
*            reproduced, stored in a retrieval system or transmitted           *
*            in any form or by any means  -  electronic, mechanical,           *
*            photocopying, recording, or otherwise, without the prior          *
*            written permission of LG CNS.                                     *
*                                                                              *
********************************************************************************
*                                                                              *
*  PROGRAM ID :       term_comm_mainterm.c                                     *
*                                                                              *
*  DESCRIPTION:       �����⿡��  ����� ����ϴ� ���α׷����� �������� ������ *
*                     ��ɾ ������� �����ϸ� ��������� ���� �����带 ���� *
*                     �Ͽ� ��������� �����Ѵ�.				 				   *
*                                                                              *
*  ENTRY POINT:       MainTermProc()                                           *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  INPUT FILES:       None                                                     *
*                                                                              *
*  OUTPUT FILES:      None                                                     *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ----------------------------------------------------------------- *
* 2005/09/27 Solution Team  woolim        Initial Release                      *
* 2006/04/17 F/W Dev. Team  wangura       ���Ϻи� �� �߰� ����ȭ              *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/
#include "dcs_comm.h"
#include "term_comm.h"
#include "term_comm_mainterm.h"
#include "../proc/main.h"
#include "../proc/load_parameter_file_mgt.h"
#include "../proc/version_mgt.h"
#include "../proc/write_trans_data.h"
#include "../proc/blpl_proc.h"
#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "../proc/card_proc_util.h"
#include "../proc/main_process_busTerm.h"

/*******************************************************************************
*  Definition of Macro                                                         *
*******************************************************************************/

/* 
 * �����ⰳ���� ������� �� CMD�����ֱ�( LoopCnt) - 3�� ���� �����ϵ��� ����
 * �����Ͽ� LoopCount�� ����
 * 0�� : 12500000, 1�� : 6000, 2�� :  5000, 3�� : 4000
 */
 
#define COMM_MAINSUB_DCS_ALLOW_CNT  \
	(( gbSubTermCnt == 0 )? 70000000 : \
	  (( gbSubTermCnt == 1 )? 1500 : 500 + ( 3 - gbSubTermCnt ) * 280 ))

/*******************************************************************************
*  Declaration of Global Variables inside this module                          *
*******************************************************************************/
/*******************************************************************************
*  Declaration of Global Variables for CommProc process                        *
*******************************************************************************/
/* 
 * ���� ���ÿ��θ� ��Ÿ���� Flag ���� 
 */	
bool boolIsBootNow = TRUE;   

/*
 * ���� ��������� 1ȸ�� ������ �ʿ��ϹǷ� ���������� ����࿩�� ���� 
 */
static bool boolIsDoDaeSa= FALSE;
/*
 * ���������۱⿡�� ���� �������(gpstSharedInfo->dwDriveChangeCnt)�� �ޱ����� ����
 * ��) gpstSharedInfo->dwDriveChangeCnt�� 2�� ����� �����Ѵ�.
 *     ���� �ʱ���ý� : 0 
 *     1ȸ ���� �� �������� - 2 
 *     2ȸ ���� �� ���� - 4 
 *     ....
 */
static word wDriveChangeCnt = 0; 

/* 
 * ��ɾ 'V'���� 'K' �ѽ���Ŭ�� ��� �����ߴ��� ���� 
 */
static bool boolIsCmdOneCycle = FALSE;

/* 
 * BL Check Result �� �˻���� �������п��� 
 */
static byte bBLCheckResult;   
static short bBLCheckStatus;
	
/* 
 * PL Check Result �� �˻���� �������п��� 
 */
static byte bPLCheckResult;  
static short bPLCheckStatus; 
/* 
 * Card Error Log ���� ����  
 */
static bool boolIsCardErrLog ;  
/* 
 * ������ card info 
 */
static TRANS_INFO stCardErrInfo;    
/*******************************************************************************
*  Declaration of Global Variables for DCS thread                              *
*******************************************************************************/
/* 
 * thread id for DCS Comm 
 */
static pthread_t nCommDCSThreadID = 0;  
/* 
 * ������� ������ ���Ῡ��  
 */
bool boolIsDCSThreadComplete = FALSE;  
/* 
 * ������� ������ ���� ���డ�ɿ��� 
 */
bool boolIsDCSThreadStartEnable = TRUE;
/* 
 * ������� ����Ƚ�� 
 */ 
int nDCSCommSuccCnt = 0; 			   
/*
 * ��������� ���� �������� �������� ���� 
 */    
bool boolIsGetLEAPPasswd = FALSE;  /* tc_leap.dat���� Password, �ܸ��� ID Load */    
bool boolIsLoadInstallInfo = FALSE;/* ��ġ��������(SetUp.dat)���� ���� IP, ���� ID Load */	
/* 
 * �������� PSAM�� ����κ��� ���� Keyset/idcenter��� �������� 
 */
bool boolIsRegistMainTermPSAM = FALSE; 

/* 
 * ������� ���� ������ ���� �ð� 
 */
time_t gtDCSCommStart = 0;
int nDCSCommIntv = 180;

/*******************************************************************************
*  Declaration of Function Prototype                                           *
*******************************************************************************/
static short CommMain2Sub( int nIndex );
static short CreateSendData( byte bCmd, int nDevNo );
static short CreateReqImgVer( int nDevNo );
static short CreateDownImg( int nDevNo );
static short CreateReqSubTermID( int nDevNo );
static short CreateReqVoiceVer( int nDevNo );
static short CreateDownVoice( int nDevNo );
static short CreateDownPar( int nDevNo );
static short CreateReqTDFile( int nDevNo );
static short CreateReqKeySet( int nDevNo );
static short CreatePoll( int nDevNo );
static short CreateBLResult( int nDevNo );
static short CreatePLResult( int nDevNo );
static short CreatePutSubTermID( int nDevNo );
static short SendParameterFile( int nDevNo );
static short ProcRecvData( void );
static short RecvNAKResp( void );
static short RecvACKResp( void );
static short RecvImgVer( void );
static short RecvSubTermID( void );
static short RecvVoiceVer( void );
static short RecvKeySet( void );
static short RecvBLCheckReq( void );
static short RecvPLCheckReq( void );
static short RecvTransDataOnPolling( void );
static short RecvTDFileFromSubTerm( int nDevNo, byte *pbFileName );
static short SendSubTermImgFileOldProtocol( int nDevNo, char* pchFileName );
static short CheckSubTermID( byte *pbCheckID );
static void CheckDCSCommParameterReq( bool *boolIsDCSReady, byte *pcCmdData );
short Polling2SubTerm( void );
short GetSubTermPSAMVer( void );
short GetSubTermId( void );
short ProcMainOrKpdProcessReq( void );
short ProcDaeSa( void );
short CreateDCSThread( void );	
short ProcMainSubCmdComm( void );
short ProcInitDCSComm( void );
short SendSubTermImgFile( int nDevNo, char* pchFileName );
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MainTermProc                                             *
*                                                                              *
*  DESCRIPTION:       �ܸ��Ⱑ �������� ��� ������� ����� ����ϴ� ���κκ� *
*                     ���� �����⿡�� �����⿡ �ʿ��� ���� �Ǵ� ���ϵ��� �̸�  *
*					  �Ծ�� ��ɾ�� ����/����� �����Ѵ�.                    *
*                     ���� ��������� ���� ���������� �����ϰ� thread���·�    *
*                     ��������� �����Ѵ�.                                     *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:          														   *
*                                                                              *
*******************************************************************************/
void MainTermProc( void )
{

	bool boolIsDriveCloseYN = TRUE;
	time_t  tCurrTime;

	byte bCmd = 0;
	byte abCmdData[64] = { 0, };
	word wCmdDataSize = 0;

    LogTerm( "====MainTermProc start!!!!====\n\n" );	

    /* 
     * ��������� ���� �����ʿ� ���� ���� 
     */    
	ProcInitDCSComm();

	LogTerm( "====MainTermProc 1!!!!====\n" );	
	
    /* Power On�� ��� SetUp()�Լ����� ������Ž��� ���� ���������� 
     * PSAMID�� �ܸ���ID�� �ֽ����� �����ϰ� �־�� �Ѵ�.  
     * ����, command v�� g�� �����Ѵ�. 
     */
    /* CMD V�� �̿��� ������ PSAMID üũ�� PSAMID�� ����Ǿ��� ���
     * IDCENTER,KEYSET�� �ʱ�ȭ�ϰ� SIMID.FLG�� ���� WRITE�Ѵ� 
     * ���� - ������� ���� �������ܸ�����  PSAMID ���� ����  
     */
	GetSubTermPSAMVer();
	LogTerm( "====MainTermProc 2!!!!====\n" );	
	
	/* 
	 * CMD G�� �̿��� ������ ID�� ����������޸𸮿� LOAD�Ѵ� 
	 * ���� - ������� ���� �����ܸ����� id���� ���� 
	 */
	GetSubTermId();
	LogTerm( "====MainTermProc 3!!!!====\n" );	
	

    /* ��������ֱ� �ʱⰪ ���� 
     * - Boot�ÿ� �ٷ� ���迡 �����ϱ� ���� �ʱⰪ�� ���ӽ����ֱ�� ����  
     */
    LogTerm( "������ ���� : [%d]\n", gbSubTermCnt );

	/* 
	 * ������ ���� Loop ���� 
	 */    
    while(1)
    {

		/* 
		 * �⺻ ���� 
		 */
		Polling2SubTerm();
		
		/*
		 * �������� �Ǵ� ���ý� ó��
		 */
        if (( boolIsBootNow == TRUE )||
            ( gpstSharedInfo->boolIsDriveNow == FALSE )) 
        {
            /*
             * Ÿ ���μ���(����,���������۱� ���μ���)���� �����޸𸮸� ����
             * ������μ����� �۾� ��û�� �̸� ó���Ѵ�.
             */
			ProcMainOrKpdProcessReq();			
            
			/* 
			 * ��������� ���� �������� ���н� 
			 */
            if ( ( boolIsGetLEAPPasswd == FALSE ) ||
                 ( boolIsLoadInstallInfo == FALSE ) )
            {
                /* 
                 * ������ ���۱��� "�۾����Դϴ�" �޼��� ���� 
                 */
                gpstSharedInfo->boolIsKpdLock = FALSE; 
				/*
				 * �˶��� �︮�� ������� ���ɻ��� ��� 
                 */
                gpstSharedInfo->boolIsReadyToDrive = TRUE; 
                continue;
            }

            /* 
             * �� ��������� �����ʱ�ȭ 
             */
            if ( wDriveChangeCnt != gpstSharedInfo->dwDriveChangeCnt )
            {            	
                printf( " POWER ON �Ѱ�츦 �����ϰ�� �� ��������� \n\n\n" );
                wDriveChangeCnt = gpstSharedInfo->dwDriveChangeCnt;
				boolIsDriveCloseYN = TRUE;
                boolIsDoDaeSa = FALSE;    
            }

			GetSharedCmd( &bCmd, abCmdData, &wCmdDataSize );
			if ( bCmd == CMD_START_STOP ||
				 gpstSharedInfo->boolIsDriveNow == TRUE )
			{
				printf( "[MainTermProc] �������/���� �õ��� �Ǵ� �̹� �������̹Ƿ� ������� �õ����� ����\n" );
				continue;
			}

            /* 
             * ������� �� CMD����ð� ���� 
             */
	     	if ( boolIsDriveCloseYN == TRUE ||
				 ( boolIsDCSThreadStartEnable == TRUE &&
				   TimerCheck( nDCSCommIntv, gtDCSCommStart ) == 0 ) )
            {
			    GetRTCTime( &tCurrTime );
				PrintlnTimeT( "[MainTermProc] ������Žð� ���� : ", tCurrTime );
			
			   /*
				* ����۾� ���� 
				*/
				ProcDaeSa();
			   
			   /*
				* ������� ������ ���� 
				*/                
				CreateDCSThread();
			    boolIsDriveCloseYN = FALSE;
            }

		   /*
			* �������� ��ɾ� ���(V~K CMD) ����  
			*/
			ProcMainSubCmdComm();

		   /* �ʱ������ �ƴ��� ���� */		   
        	boolIsBootNow = FALSE;

        }
    }
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcInitDCSComm                                          *
*                                                                              *
*  DESCRIPTION:       ������ſ� �����ϱ� ������ �ʿ��� �������� �����Ѵ�.     *
*					  														   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:   ȣ��Ǵ� �Լ����� Load_parameter_file_mgt.c�� �����Ѵ�.          *
*                                                                              *
*******************************************************************************/
short ProcInitDCSComm( void )
{
	short sRetVal = SUCCESS;
	
    /* 
     * tc_leap.dat���� Password, �ܸ��� ID Load 
     */    
    if ( GetLEAPPasswd()== SUCCESS )
    {
        boolIsGetLEAPPasswd = TRUE;
    }
    /* 
     * ��ġ��������(SetUp.dat)���� ���� IP, ���� ID Load
     */	
    if ( LoadInstallInfo() == SUCCESS )
    {
        boolIsLoadInstallInfo = TRUE;
    }

    DebugOut( "boolIsGetLEAPPasswd : %d\n",boolIsGetLEAPPasswd );
    DebugOut( "boolIsLoadInstallInfo : %d\n",boolIsLoadInstallInfo );
    /*
     * �������� �Ķ���� Load
     * ���� - ������μ����� ����Process�� ������ Process�̹Ƿ� ���赿���� ���� 
     *        �������μ����� ���������Ķ���͸� ����ü�� Load�ϴ� ��ó�� �����ϰ� 
     *        ó�����ش�.
     */
    LoadVehicleParm();

	return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcMainSubCmdComm                                       *
*                                                                              *
*  DESCRIPTION:       ���������� ��ɾ� ���� ����� ó���Ѵ�.				   *
*					  														   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*         MAIN_SUB_COMM_SUB_TERM_IMG_VER  'V'     //Program Version            *
* 		MAIN_SUB_COMM_SUB_TERM_IMG_DOWN   'D'     // program Download          * 
* 		MAIN_SUB_COMM_SUB_TERM_VOICE_VER  'H'     // Voice Version 			   *
* 		MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN  'A'     // Voice Download            *
* 		MAIN_SUB_COMM_SUB_TERM_PARM_DOWN  'M'     // Parameter Download        *
* 		MAIN_SUB_COMM_GET_SUB_TERM_ID     'G'     // Sub Term ID               *
* 		MAIN_SUB_COMM_ASSGN_SUB_TERM_ID   'I'     // Assign Sub Term ID        *
* 		MAIN_SUB_COMM_REQ_BL_SEARCH     'B'       // req BL search             *
* 		MAIN_SUB_COMM_REQ_PL_SEARCH     'U'       // req PL search             * 
* 		MAIN_SUB_COMM_POLLING           'P'       // Polling                   *
* 		MAIN_SUB_COMM_GET_TRANS_CONT    'X'     // get transaction contents    * 
* 		MAIN_SUB_COMM_GET_TRANS_FILE    'T'     // get transaction file 	   *
* 		MAIN_SUB_COMM_CHK_CRC           'C'     // check CRC command - old ver *
* 		MAIN_SUB_COMM_REGIST_KEYSET     'K'     // regist keyset 			   *
*                                                                              *
*******************************************************************************/
short ProcMainSubCmdComm( void )
{	
	short sRetVal = SUCCESS;
	int nTmpIndex = -1;
	int status;


    /* ������� ���Ῡ�� Ȯ�� */
	if ( boolIsDCSThreadComplete == TRUE )
    {
    	pthread_join( nCommDCSThreadID, (void *)&status );
		
		printf( "\n" );
        printf( "[ProcMainSubCmdComm] %d���� �����ܸ��⿡ ���� CMD ���� /////////////////////////\n",
			gbSubTermCnt );
		
        /* ������ ���α׷� ����üũ ��ɾ���� ���� */
        nIndex = 0;
        bCurrCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;

        while( nIndex < gbSubTermCnt )
        {
            if ( nTmpIndex != nIndex )
            {
                nTmpIndex = nIndex;
				printf( "\n" );
				printf( "[ProcMainSubCmdComm] %d�� �����ܸ��� -------------------------------------------\n",
					nIndex + 1 );
            }

			DebugOut( "\n %d�� �������� %d�� ������ Cmd %c ����//////\n",
                      gbSubTermCnt,
                      nIndex+1,
                      bCurrCmd );

            sRetVal = CommMain2Sub( nIndex );

            /* CMD���� ���н� ����� �� ���� CMD�� JUMP *///////////////
            if ( sRetVal < 0 )
            {
                if ( sRetVal != SUCCESS )
                {
                    usleep(50000);
                    tcflush(nfdMSC, TCIOFLUSH);
                }
				
                /* �ش� CMD�� ����� */
                printf(" %c CMD ������� : %x\n �ѹ�������", bCurrCmd, sRetVal);
                sRetVal = CommMain2Sub( nIndex );	                    
                if ( sRetVal < 0 )
                {
                    if ( sRetVal != SUCCESS )
                    {
                        usleep(50000);
                        tcflush(nfdMSC, TCIOFLUSH);
                    }	                    	
                		                    	
                    switch( bCurrCmd )
                    {
                        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :
                        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :
                            bCurrCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID;
                            printf( ": V,D->G  JUMP\n\n\n" );
                            break;

                        case MAIN_SUB_COMM_GET_SUB_TERM_ID :
                            bCurrCmd = MAIN_SUB_COMM_SUB_TERM_PARM_DOWN;
                            printf( "G->M JUMP\n\n\n" );
                            break;

                        case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :
                            bCurrCmd = MAIN_SUB_COMM_SUB_TERM_VOICE_VER;
                            printf( "M->H JUMP\n\n\n" );
                            break;

                        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER :
                        case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN :
                             bCurrCmd = MAIN_SUB_COMM_REGIST_KEYSET;
                            printf( "H,A->K JUMP\n\n\n" );
                            break;

                        case MAIN_SUB_COMM_REGIST_KEYSET :
                        	printf( "K->Next JUMP\n\n\n" );
                            boolIsCmdOneCycle = TRUE;
                            break;
                    }
                }
            }

            /* ������ CMD���� üũ */
            if ( boolIsCmdOneCycle == TRUE )
            {
                nIndex++;
                bCurrCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;
                usleep( 50000 );
            }

        }
		
		if ( gbSubTermCnt == 0 )
			printf( "[ProcMainSubCmdComm] �����ܸ��� ���� -> ��ü CMD ���� /////////////////////////\n" );
		else
		{
			printf( "\n" );
			printf( "[ProcMainSubCmdComm] ��ü CMD ���� ////////////////////////////////////////////\n" );
		}
		printf( "\n" );

		TimerStart( &gtDCSCommStart );

		boolIsDCSThreadComplete = FALSE;
        gpstSharedInfo->boolIsKpdLock = FALSE;
        gpstSharedInfo->boolIsReadyToDrive = TRUE;
	
    }
	

	return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateDCSThread	                                       *
*                                                                              *
*  DESCRIPTION:       ��������� ���� �����带 �����ϰ� �����Ѵ�. 			   *
*					  														   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateDCSThread( void )
{
	short sRetVal = SUCCESS;

    /* ������� ������ ���� */
    /* ���� ������� ���࿩�� - ������/������߿� ���� �Ǵ� */	
    if ( ( boolIsGetLEAPPasswd == TRUE ) &&
         ( boolIsLoadInstallInfo == TRUE ) &&
         ( boolIsDCSThreadStartEnable == TRUE ) )
    {

        DebugOut( "\n\n\n[�������] Thread ����\n" );
        sRetVal = pthread_create( &nCommDCSThreadID, NULL, DCSComm, NULL );
        if ( sRetVal == SUCCESS )
        {
            boolIsDCSThreadComplete = FALSE;
            boolIsDCSThreadStartEnable = FALSE;
		   /*
			* ������� �� ��������ɾ� ��� ���� �ֱ� �ʱ�ȭ
			*/
			DebugOut("������� �� ��������ɾ� ��� ���� �ֱ� �ʱ�ȭ/n");
            DebugOut( "[�������] Thread ���� " );
            DebugOut( "=<TermComm> nCommDCSThreadID=%ld=\n",
                      nCommDCSThreadID );
        }
        else
        {
            DebugOut( "[�������] Thread �������� " );
        }
    }

	return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcDaeSa			                                       *
*                                                                              *
*  DESCRIPTION:       ��縦 �����Ѵ�.										   *
*					  														   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                     ������ �����ϴ� ���¿��� ���Ȯ���۾��� ������ ���      *
*                     ������ �ܸ��� ��ȣ�� �����ְ� �ý��� �����ϰ� �ȴ�.      *
*                     ���� - �����⸦ ���� �ٲ۴ٴ����ϴ� �����߻����ɼ� ����  *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short ProcDaeSa( void )
{
	short sRetVal = SUCCESS;
	short ashDaeSaResult[3] = { 1, 1, 1 }; /* ��� ����� ���� ���� */
	int nDaeSaRetryCnt = 3;           /* ����۾����� Ȯ���� ���� ��� ��õ�Ƚ��*/   
    dword dwDisplayDevNo;             /* ����۾����� ���е� �����ܸ��� ��ȣ     */   
    
	DebugOut( " ������� �� CMD����ð� ���� %d\n\n\n",wDriveChangeCnt );
	DebugOut( " ������� �� CMD����ð� ���� %u\n\n\n",gpstSharedInfo->dwDriveChangeCnt );
    gpstSharedInfo->boolIsKpdLock = TRUE;

	if ( gpstSharedInfo->boolIsDriveNow == TRUE)
	{
		ctrl_event_info_write( "9002" );
	}

    /* �ʱ��� ����-���ý� ������ϸ��� �����Ƿ�*/
    if ( wDriveChangeCnt != 0 )
    {
        if ( boolIsDoDaeSa == FALSE )
        {
            LogTerm( "[��� �۾�Ȯ�� ����]\n" );
			
            /* ������� ����-CMD T */
            nIndex = 0;
            while ( nIndex < gbSubTermCnt )
            {
				nDaeSaRetryCnt = 3;
				
	            while( nDaeSaRetryCnt-- )/*CMD T- �ִ� 3ȸ �õ�*/
	            {

	                LogTerm( "\n ����۾��ʿ俩�� Ȯ�� �õ� %d�� �ܸ��� - %d ȸ]\n",
	                        nIndex+1, 3-nDaeSaRetryCnt );

                    bCurrCmd = MAIN_SUB_COMM_GET_TRANS_FILE;
					
                    ashDaeSaResult[nIndex] = CommMain2Sub( nIndex );
                    LogTerm( "����� :%02x\n",
                             ashDaeSaResult[nIndex] );

                    sleep(1);
                    tcflush( nfdMSC, TCIOFLUSH );	
					
                   if (( ashDaeSaResult[nIndex] == SUCCESS ) ||
                       ( ashDaeSaResult[nIndex] ==
                              			ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST))
                   {
                        // ������ ���� ������ ������ ���� �߻���
                        break;
                   }
				   
                }

                nIndex++;				

            }

            /* ������ ����ְ� ����ʿ俩�� Ȯ�ν��н� ���μ��� ���� */
            /* ������ ���ٰ� �߰��� �����⸦ ���ٴ��� �� ��� ���� */
            /* ���LED 000001, �ϴ�LED���� �ܸ����ȣ�� ǥ�� */
            if (( nDaeSaRetryCnt < 0 )&&
                ( gpstSharedInfo->bPollingResCnt == 1 ))
            {
                dwDisplayDevNo =  nIndex + 1;
                DisplayASCInUpFND( "000001" );
                DisplayDWORDInDownFND( dwDisplayDevNo );
                Buzzer( 3, 50000 );
                VoiceOut( VOICE_CHECK_TERM );     // �ܸ��⸦ ���� �Ͻñ� �ٶ��ϴ�.
                gpstSharedInfo->bPollingResCnt = 2;
                sleep(5);
                LogTerm( "\r\n ���� ��� ���μ��� ����\r\n" );
                exit(0);
            }

            /* ������ۼ������� üũ*/
            nIndex = 0;
            while ( nIndex < gbSubTermCnt )
            {
                DebugOut( "%d�� ������ ashDaeSaResult : %d\n",
                          nIndex+1, ashDaeSaResult[nIndex] );

                if ( ashDaeSaResult[nIndex] == SUCCESS )
                {

                    LogTerm( "[����۾���� ����]\n" );

                    boolIsDoDaeSa = TRUE;
                    ashDaeSaResult[nIndex] = 1;
                    break;
                }
                nIndex++;
            }

            /* ����۾�����*/
            if ( boolIsDoDaeSa == TRUE )
            {
                DebugOut( "[����۾�����]/n" );
                sRetVal = RemakeTrans();

                if ( sRetVal < 0 )
                {
                    LogTerm( "[T : ����۾�����]\n" );
                }
                else
                {
                    LogTerm( "[T : ����۾��Ϸ�]\n" );
                }
            }
            else
            {
                boolIsDoDaeSa = TRUE;
                LogTerm( "\n\n[����۾���� ����]\n\n" );
            }

        }
        else
        {
            LogTerm( "\n\n[����۾� �����]\n\n" );

        }

    }
    else
    {
    	LogTerm( "[�ʱ��� ����]-���ýÿ� ������ϸ��� ���� \n\n" );
    }

	return sRetVal;

}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcMainOrKpdProcessReq                                  *
*                                                                              *
*  DESCRIPTION:       ���� Ȥ�� ���������۱� ���μ����� ��û�ϴ� �۾���        *
*					  �����ϴ��� Ȯ���ϰ� �����ϴ� ���                        *                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                     ������ �����ϴ� ���¿��� ���Ȯ���۾��� ������ ���      *
*                     ������ �ܸ��� ��ȣ�� �����ְ� �ý��� �����ϰ� �ȴ�.      *
*                     ���� - �����⸦ ���� �ٲ۴ٴ����ϴ� �����߻����ɼ� ����  *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short ProcMainOrKpdProcessReq( void )
{
	short sRetVal = SUCCESS;
	
    /* variables for get command/data in shared memory */
    byte bSharedMemoryCmd;           /* �����޸𸮳��� Command */
    char achCmdData[40] = { 0, };    /* �����޸𸮳��� Data    */
    word wCmdDataSize;               /* �����޸𸮳��� DataSize */
	bool boolIsAssgnSubTermID = TRUE;		

    /* �����޸� CMDüũ */////////////////////////////////////////////
    DebugOut("/* /n�����޸� CMDüũ */ \n" );
    GetSharedCmd( &bSharedMemoryCmd, achCmdData, &wCmdDataSize );

    if ( achCmdData[0] == CMD_REQUEST )
    {

        switch( bSharedMemoryCmd )
        {
            case CMD_SETUP :
				if ( gpstSharedInfo->boolIsDriveNow == TRUE )
				{
					ctrl_event_info_write( "9003" );
					break;
				}
				
                if ( boolIsGetLEAPPasswd == TRUE &&
                     boolIsLoadInstallInfo == TRUE )
                {
		     	 	boolIsDCSThreadStartEnable = FALSE;
		     	 	gpstSharedInfo->boolIsKpdLock = TRUE;
                    sRetVal = SetupTerm();
		      		gpstSharedInfo->boolIsKpdLock = FALSE;
					if ( sRetVal == SUCCESS )
					{
	                	achCmdData[0] = CMD_SUCCESS_RES;
	                    SetSharedDataforCmd( bSharedMemoryCmd, achCmdData, 1 );
					}
					else
					{
	                	achCmdData[0] = CMD_FAIL_RES;
	                    SetSharedDataforCmd( bSharedMemoryCmd, achCmdData, 1 );
					}
                }
                else
                {
                	achCmdData[0] = CMD_FAIL_RES;
                    SetSharedDataforCmd( bSharedMemoryCmd, achCmdData, 1 );
                }
		  		boolIsDCSThreadStartEnable = TRUE;
                break;

            case CMD_RESETUP:
				if ( gpstSharedInfo->boolIsDriveNow == TRUE )
				{
					ctrl_event_info_write( "9004" );
					break;
				}

                if ( GetLEAPPasswd()== SUCCESS )
                {
                    boolIsGetLEAPPasswd = TRUE;
                }
				else
				{
					boolIsGetLEAPPasswd = FALSE;
				}

                if ( LoadInstallInfo() == SUCCESS )
                {
                    boolIsLoadInstallInfo = TRUE;
                }
				else
				{
					boolIsLoadInstallInfo = FALSE;
				}

                if ( boolIsGetLEAPPasswd == TRUE &&
                     boolIsLoadInstallInfo == TRUE )
                {
					boolIsDCSThreadStartEnable = FALSE;
                    gpstSharedInfo->boolIsKpdLock = TRUE;
                    sRetVal = ReSetupTerm();
	   		        gpstSharedInfo->boolIsKpdLock = FALSE;
					if ( sRetVal == SUCCESS )
					{
	                	achCmdData[0] = CMD_SUCCESS_RES;
	                    SetSharedDataforCmd( bSharedMemoryCmd, achCmdData, 1 );
					}
					else
					{
	                	achCmdData[0] = CMD_FAIL_RES;
	                    SetSharedDataforCmd( bSharedMemoryCmd, achCmdData, 1 );
					}
                }
                else
                {
                	achCmdData[0] = CMD_FAIL_RES;
                    SetSharedDataforCmd( bSharedMemoryCmd, achCmdData, 1 );
                }
		 		boolIsDCSThreadStartEnable = TRUE;				
                break;

            case CMD_PARMS_RESET:

                if ( achCmdData[1] == '1' )
                {
                    CheckDCSCommParameterReq( &boolIsGetLEAPPasswd,
                                              achCmdData );
                }
                else
                {
                    CheckDCSCommParameterReq( &boolIsLoadInstallInfo,
                                              achCmdData );
                }
                break;

            case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID:
                DebugOut( "I    cmd ���� ���� \n" );
                nIndex = 0;
                boolIsAssgnSubTermID = TRUE;
                while( nIndex < gbSubTermCnt )
                {
                    bCurrCmd = MAIN_SUB_COMM_ASSGN_SUB_TERM_ID;
                    sRetVal = CommMain2Sub( nIndex );
                    if ( sRetVal < 0 )
                    {
                        boolIsAssgnSubTermID = FALSE;
                        DebugOut( "[4.�����������- I Cmd : %d \r\n",
                                sRetVal );								
                    }

                    nIndex++;
                }
                
                if ( boolIsAssgnSubTermID == FALSE )
                {
                	achCmdData[0] = CMD_FAIL_RES;
                    SetSharedDataforCmd( MAIN_SUB_COMM_ASSGN_SUB_TERM_ID, achCmdData, 1 );                
                }
                else
                {
                	achCmdData[0] = CMD_SUCCESS_RES;
                    SetSharedDataforCmd( MAIN_SUB_COMM_ASSGN_SUB_TERM_ID, achCmdData, 1 );                
                }
                break;
        }
    }
	return sRetVal;
}		
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetSubTermPSAMVer                                  	   *
*                                                                              *
*  DESCRIPTION:      ������ PSAMID üũ�� PSAMID�� ����Ǿ��� ��� 			   *
* 					 IDCENTER,KEYSET�� �ʱ�ȭ�ϰ� SIMID.FLG�� ���� WRITE�Ѵ�   *
*                    - Command V(Protocol V)                                   *
*																			   *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short GetSubTermPSAMVer( void )
{
	bool boolPSAMChkRetryNeed = FALSE;
	int nPSAMChkRetry = 3;
	short sRetVal = SUCCESS;
	int nSubIDInfoExistYN = 0;
	
//    sleep( 1); /* ���ýð� Ȯ�� */
	nSubIDInfoExistYN = access( SUBTERM_ID_FILENAME, F_OK );  // subid.dat

	if ( nSubIDInfoExistYN == 0 )
	{
        nPSAMChkRetry = 3; /* ��õ� Ƚ�� */
	}
	else
	{
		nPSAMChkRetry = 2; /* ��õ� Ƚ�� */
	}
	
	while( nPSAMChkRetry-- )/* ��õ� 2ȸ */
    { 
   	    nIndex = 0;	    	
	    while ( nIndex < gbSubTermCnt )
	    {
	        bCurrCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;
	        sRetVal = CommMain2Sub( nIndex );

	        if ( sRetVal != SUCCESS )
	        {
	        	usleep(50000);
	       		tcflush(nfdMSC, TCIOFLUSH);
				boolPSAMChkRetryNeed = TRUE;					
			    LogTerm( "[������ %d�� PSAMIDüũ����] : %x\n",nIndex+1, sRetVal);	
	        }
	        else
	        {	        	
	            LogTerm( "[������ %d�� PSAMIDüũ����]\n",nIndex+1);		            
	        }
	        nIndex++;
	    }  
	      
        if ( boolPSAMChkRetryNeed == FALSE )
        {
        	break;
        }
	}

	/*CMD V ���� �� ��� Messageȭ�鿡 ��� */
    nIndex = 0;
    PrintlnASC( "[GetSubTermPSAMVer]     �����ܸ���PSAMID : ",
		gpstSharedInfo->abMainPSAMID, 16 );

    while( nIndex < gbSubTermCnt)
    {
        printf( "[GetSubTermPSAMVer] %d�� �����ܸ���PSAMID : ", nIndex+1 );
        PrintASC( gpstSharedInfo->abSubPSAMID[nIndex], 16 );
		printf( "\n" );
        nIndex++;
    }

	return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetSubTermId                                      	   *
*                                                                              *
*  DESCRIPTION:      ������ PSAMID üũ�� PSAMID�� ����Ǿ��� ��� 			   *
* 					 IDCENTER,KEYSET�� �ʱ�ȭ�ϰ� SIMID.FLG�� ���� WRITE�Ѵ�   *
*                    - Command V(Protocol V)                                   *
*																			   *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short GetSubTermId( void )
{
	short sRetVal = SUCCESS;
    bool boolGetSubTermIdRetryNeed = FALSE;
    int nGetSubTermIdRetry = 3;
	int nSubIDInfoExistYN = 0;
	
//	sleep( 1); /* ���ýð� Ȯ�� */
	nSubIDInfoExistYN = access( SUBTERM_ID_FILENAME, F_OK );  // subid.dat

	if ( nSubIDInfoExistYN == 0 )
	{
        nGetSubTermIdRetry = 3; /* ��õ� Ƚ�� */
	}
	else
	{
		nGetSubTermIdRetry = 2; /* ��õ� Ƚ�� */
	}
	
	while( nGetSubTermIdRetry-- )/* ��õ� 2ȸ */
    { 
   	    nIndex = 0;	    	
	    while ( nIndex < gbSubTermCnt )
	    {
	        bCurrCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID;
	        sRetVal = CommMain2Sub( nIndex );
			if ( sRetVal != SUCCESS )
	        {
				usleep(50000);
		       	tcflush(nfdMSC, TCIOFLUSH);
		        boolGetSubTermIdRetryNeed = TRUE;					
			    LogTerm( "[������ %d�� �ܸ��� IDüũ����] : %x\n",nIndex+1, sRetVal);	
	        }
	        else
	        {
	            LogTerm( "[������ %d�� �ܸ��� IDüũ����]\n",nIndex+1);		            
	        }
	        nIndex++;
	    }
	      
        if ( boolGetSubTermIdRetryNeed == FALSE )
        {
        	break;
		}
	}

	nIndex = 0;
    PrintlnASC( "[GetSubTermId]     �����ܸ���ID : ",
		gpstSharedInfo->abMainTermID, 9 );

    while( nIndex < gbSubTermCnt)
    {
        printf( "[GetSubTermId] %d�� �����ܸ���ID : ", nIndex + 1 );
        PrintlnASC( "", gpstSharedInfo->abSubTermID[nIndex],
			sizeof( gpstSharedInfo->abSubTermID[nIndex] ) );
        nIndex++;
    }

	return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       Polling2SubTerm                                      	   *
*                                                                              *
*  DESCRIPTION:      �����⿡ ����ð�,���翪ID,�������,������ID,             *
*                    �ŷ��������ϸ� ���� ������ �۽��Ѵ�.                      *
*                     														   *
*																			   *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short Polling2SubTerm( void )
{
	short sRetVal = SUCCESS;
	
    /* �⺻ ���� */
    nIndex = 0;
    while ( nIndex < gbSubTermCnt )
    {
        bCurrCmd = MAIN_SUB_COMM_POLLING;
        sRetVal = CommMain2Sub( nIndex ); 
        usleep(50000);
        tcflush( nfdMSC, TCIOFLUSH );			

		usleep( 32000 );
		
		if ( sRetVal < SUCCESS )
		{
			LogTerm( "polling sRetVal=>[%02x]\n", sRetVal );
		}
        nIndex++;
    }

	return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CommMain2Sub                                      	   *
*                                                                              *
*  DESCRIPTION:      ������� ���������� ����, ����, ����, ���ŵ����� ó����   *
*                    �ϴ� �����Լ��̴�. ������ �Լ����� ȣ���Ͽ� �����Ѵ�.     *
*                   														   *
*                     														   *
*																			   *
*  INPUT PARAMETERS:  int nIndex - �����ܸ��� ��ȣ                             *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CommMain2Sub( int nIndex )
{
    short sRetVal = SUCCESS;

    if ( bCurrCmd != MAIN_SUB_COMM_REGIST_KEYSET )
    {
        boolIsCmdOneCycle = FALSE;
    }

	//printf( "%d�� ������� ���\n", nIndex + 1 );
    sRetVal = CreateSendData( bCurrCmd, nIndex + 1 );

    if ( sRetVal < 0 )
    {
    	LogTerm( "%02x command CreateSendData error �߻�( %02x )\n", 
				 bCurrCmd, sRetVal );
        return ErrRet( sRetVal );
    }

    bCurrCmd = stSendPkt.bCmd;
    sRetVal = SendPkt();

    if ( sRetVal < 0 )
    {
    	LogTerm( "%02x command SendPkt error �߻�( %02x )\n", 
				 bCurrCmd, sRetVal );
        return ErrRet( sRetVal );
    }
	
    /* ������ �ܸ�������� ��� �ܿ��ܸ����� �����ӵ��� ���� timeout ���� */
    if ( bCurrCmd == MAIN_SUB_COMM_POLLING )
    {
    	sRetVal = RecvPkt( 400, nIndex + 1 ); //receive ack �� ó��
	}
	else
	{
		sRetVal = RecvPkt( 3000, nIndex + 1 ); //receive ack �� ó��		
	}
	
    if ( sRetVal < 0 )
    {
            
        LogTerm( "%02x command RecvPkt error �߻�( %02x )\n", 
				 bCurrCmd, sRetVal );
        return ErrRet( sRetVal );
    }

    if ( stRecvPkt.bCmd == ACK )
    {
        switch( bCurrCmd )
        {
            case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :
                DisplayCommUpDownMsg( 1, 2 );
                sRetVal = SendParameterFile( nIndex + 1 );
                DisplayCommUpDownMsg( 2, 2 );
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_M_SEND_FINAL );
                }

                DebugOut( "\n[Final RecvPkt-From %d�� �ܸ���]",nIndex+1 );

                // �����ܸ��⿡�� keyset ����� ���� timeout�� �� �ش�.
                sRetVal = RecvPkt( 6000, nIndex + 1 );
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_M_RECV_FINAL );
                }

                DebugOut( "[ProcRecvData]final ack ����/n" );
                sRetVal = ProcRecvData();
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_M_PARSE_FINAL );
                }

                DebugOut( "[ProcRecvData]final ack Parse����/n" );

                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID :
                ProcRecvData();
                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :
                DisplayCommUpDownMsg( 1, 2 );
                sRetVal = SendSubTermImgFile( nIndex + 1, BUS_EXECUTE_FILE );
                DisplayCommUpDownMsg( 2, 2 );
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_D_SEND_FINAL );
                }
                sRetVal = RecvPkt( 3000, nIndex + 1 );
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_D_RECV_FINAL );
                }

                sRetVal = ProcRecvData();
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_D_PARSE_FINAL );
                }

                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN :
                DisplayCommUpDownMsg( 1, 3 );
				printf( "[CommMain2Sub] �������� ������ ���� : %s ",
					VOICE0_FILE );
                sRetVal = SendFile( nIndex + 1, VOICE0_FILE );
                DisplayCommUpDownMsg( 2, 3 );

                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_A_SEND_FINAL );
                }

                sRetVal = RecvPkt( 3000, nIndex + 1 );
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_A_RECV_FINAL );
                }

                sRetVal = ProcRecvData();
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_A_PARSE_FINAL );
                }

                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_GET_TRANS_FILE :
                DisplayCommUpDownMsg(1 , 2);
                sRetVal = RecvTDFileFromSubTerm( nIndex + 1,
                                            gpstSharedInfo->abTransFileName);
                DisplayCommUpDownMsg(2 , 2);

                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_POLLING :
                DebugOut( "[4.RecvPkt-From %d�� �ܸ���]", nIndex+1 );
                sRetVal = RecvPkt( 1000, nIndex + 1 );
                if ( sRetVal < 0 )
                {
                	LogTerm( "%02x command RecvPkt1 error �߻�( %02x )\n", 
				 			 bCurrCmd, sRetVal );
        
                    return ErrRet( ERR_MAINSUB_COMM_CMD_P_RECV_FINAL );
                }

                sRetVal = ProcRecvData();
                if ( sRetVal < 0 )
                {
                	LogTerm( "%d �� NAK send\n", nIndex + 1 );
                    // ������ ���� �Ľ� ������ ��쿡�� NAK PACKET ����
                    CreateSendData( NAK,  nIndex + 1);
                }
                else
                {
                    // ������ ���� �Ľ̼����� ��쿡��
                    // ACK,X : ACK, B,U : ��� ��Ŷ ����
                    CreateSendData( stRecvPkt.bCmd,  nIndex + 1);
                }

                DebugOut( "[7.Final SendPkt]" );
                sRetVal = SendPkt();
                if ( sRetVal < 0 )
                {
                    LogTerm( "Error ERR_MAINSUB_COMM_CMD_P_SEND_FINAL\n" );
                    return ErrRet( ERR_MAINSUB_COMM_CMD_P_SEND_FINAL );
                }
				
				else
			  	{
			       if ( boolIsCardErrLog == TRUE )
			       {
			  			DeleteCardErrLog( stCardErrInfo.abCardNo );
						boolIsCardErrLog = FALSE;
			       }
			  	}

                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_REGIST_KEYSET :
            case MAIN_SUB_COMM_SUB_TERM_VOICE_VER :
            case MAIN_SUB_COMM_GET_SUB_TERM_ID :
            case MAIN_SUB_COMM_SUB_TERM_IMG_VER :
                DebugOut( "\n[4.RecvPkt-From %d�� �ܸ���]", nIndex+1 );
                sRetVal = RecvPkt( 6000, nIndex + 1 );
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_V_RECV_FINAL );
                }

                CreateSendData( ACK,  nIndex + 1 );
                DebugOut( "[5.SendPkt-To %d�� �ܸ���]", nIndex+1 );
                sRetVal = SendPkt();
                if ( sRetVal < 0 )
                {
                    return ErrRet( ERR_MAINSUB_COMM_CMD_V_SEND_FINAL );
                }

                DebugOut( "\n[6.ProcRecvData]" );
                sRetVal = ProcRecvData();
                if ( sRetVal < 0 )
                {
                    return -1;
                }

                return ErrRet( sRetVal );

            default :
                return ErrRet( sRetVal );
        }
    }
    else // NAK
    {
        DebugOut( "Command NAK Received\n" );
        LogTerm( "Command NAK Received\n" );
        switch( stSendPkt.bCmd )
        {
            // �ٸ� cmd���� NAK�� �������� ��� ó�� �߰�
            case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID : //�����ܸ��� ID �ο� ���� ����
                DebugOut( "[4.NAK����-return]" );
                DebugOut( "[5.NAK����- I Cmd -�����޸� ���� ���� \r\n" );
                //SetSharedDataforCmd( MAIN_SUB_COMM_ASSGN_SUB_TERM_ID, "9", 1 );
                sRetVal = ERR_MAINSUB_COMM_ASSGN_SUM_TERM_ID;
                return ErrRet( sRetVal );

            case MAIN_SUB_COMM_REGIST_KEYSET :
                DebugOut( "\r\n[�����������]Cmd K���� NAK �������� \r\n" );
                boolIsCmdOneCycle = TRUE;
                return ErrRet( sRetVal );

            default :
                return ErrRet( sRetVal );
        }
    }

    return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendParameterFile			                               *
*                                                                              *
*  DESCRIPTION:       �����⿡ �ʿ��� �Ķ���� ���ϵ��� �����Ѵ�    		   *
*					  														   *
*                                                                              *
*  INPUT PARAMETERS:  INT nDevNo - �����ܸ����ȣ                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*     NEW_FARE_INFO_FILE            // �� ��� ���� 					`	   *
*     PREPAY_ISSUER_INFO_FILE       // ���� ����� ����                        *
*     POSTPAY_ISSUER_INFO_FILE      // �ĺ� ����� ����                        *
*     DIS_EXTRA_INFO_FILE           // ���� ���� ����                          *
*     HOLIDAY_INFO_FILE             // ���� ����                               *
*     XFER_TERM_INFO_FILE           // ȯ������ ����                           *
*     BUS_STATION_INFO_FILE         // ������ ���� ����                        *
*     VEHICLE_PARM_FILE             // �������� �Ķ���� ����                  *
*     ROUTE_PARM_FILE               // �뼱�� �Ķ���� ����                    *
*     XFER_APPLY_INFO_FILE          // ȯ�� ����                               *
*     ISSUER_VALID_PERIOD_INFO_FILE // �ĺ� ����� üũ ����           *
*     AUTO_CHARGE_SAM_KEYSET_INFO_FILE // �ڵ����� SAM Key Set ����            *
*     AUTO_CHARGE_PARM_INFO_FILE       //�ڵ����� Parameter ����               *
*     EPURSE_ISSUER_REGIST_INFO_FILE   // ���� ȭ��� �������                 *
*     PSAM_KEYSET_INFO_FILE            //����SAM Key Set ����                  *          
*                                                                              *
*******************************************************************************/
short SendParameterFile( int nDevNo )
{
    int nLoopNo = 0;
    short sRetVal = 0;

    byte* FileDownfile [][2] =
    {
        { "N", NEW_FARE_INFO_FILE },			/* �� ��� ���� */
        { "N", PREPAY_ISSUER_INFO_FILE },		/* ���� ����� ���� */
        { "N", POSTPAY_ISSUER_INFO_FILE },		/* �ĺ� ����� ���� */
        { "N", DIS_EXTRA_INFO_FILE },			/* ���� ���� ���� */
        { "N", HOLIDAY_INFO_FILE },				/* ���� ���� */
        { "N", XFER_TERM_INFO_FILE },			/* ȯ������ ���� */
        { "N", BUS_STATION_INFO_FILE },			/* ������ ���� ���� */
        { "N", VEHICLE_PARM_FILE },				/* �������� �Ķ���� ���� */
        { "N", ROUTE_PARM_FILE },				/* �뼱�� �Ķ���� ���� */
        { "N", XFER_APPLY_INFO_FILE },			/* ȯ�� ���� */
        { "N", ISSUER_VALID_PERIOD_INFO_FILE },	/* �ĺ� ����� üũ ����*/
        { "N", AUTO_CHARGE_SAM_KEYSET_INFO_FILE },
												/* �ڵ����� SAM Key Set ���� */
        { "N", AUTO_CHARGE_PARM_INFO_FILE },
												/* �ڵ����� Parameter ���� */
        { "N", EPURSE_ISSUER_REGIST_INFO_FILE },/* ���� ȭ��� ������� */
        { "N", PSAM_KEYSET_INFO_FILE },			/* ����SAM Key Set ���� */
        { NULL, NULL }
    };

    FileDownfile[0][0] = "Y";
    FileDownfile[1][0] = "Y";
    FileDownfile[2][0] = "Y";
    FileDownfile[3][0] = "Y";
    FileDownfile[4][0] = "Y";
    FileDownfile[5][0] = "Y";
    FileDownfile[6][0] = "Y";
    FileDownfile[7][0] = "Y";
    FileDownfile[8][0] = "Y";
    FileDownfile[9][0] = "Y";
    FileDownfile[10][0] = "Y";
    FileDownfile[11][0] = "Y";
    FileDownfile[12][0] = "Y";
    FileDownfile[13][0] = "Y";
    FileDownfile[14][0] = "Y";

    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_PARM_DOWN;
    stSendPkt.bDevNo = nDevNo + '0';

    while( FileDownfile[nLoopNo][0] != NULL ) // ������
    {
        stSendPkt.wSeqNo = 0;
        DebugOut( "file index nLoopNo : %d", nLoopNo );

        if ( memcmp( FileDownfile[nLoopNo][0], "Y", 1 ) == 0 )
        {

            if ( access( FileDownfile[nLoopNo][1], F_OK ) != 0 )
            {
                DebugOut( "\r\n ======no====== FileDownfile[%s]",
                          FileDownfile[nLoopNo][1] );
                nLoopNo++;
                continue;
            }
            else
            {
				printf( "[SendParameterFile] �������� : [%s] ",
					FileDownfile[nLoopNo][1]);
            }

            stSendPkt.nDataSize = strlen( FileDownfile[nLoopNo][1] );
            memcpy( &stSendPkt.abData[0], FileDownfile[nLoopNo][1],
                    strlen(FileDownfile[nLoopNo][1]));

            DebugOut( "\r\n�������ϸ�[%s],����:[%d]  \n",
                      FileDownfile[nLoopNo][1],
                      strlen(FileDownfile[nLoopNo][1]) );
            DebugOut( "[SendPkt- �������ϸ� -To %d�� �ܸ���]", nIndex+1 );

            sRetVal = SendPkt();
            if ( sRetVal < 0 )
            {
                LogTerm( "\r\n======result====== [%s] ���ϸ� ������ ����\n",
                          FileDownfile[nLoopNo][1]);
                nLoopNo++;
                continue;
            }

            DebugOut( "[RecvPkt- �������ϸ����� -To %d�� �ܸ���]", nIndex+1 );

            sRetVal = RecvPkt( 3000, nIndex + 1 );
            if ( sRetVal < 0 || stRecvPkt.bCmd == NAK )
            {
                LogTerm( "\r\n======RecvPkt result====== [%s] ���ϸ� ���� ����\n",
                          FileDownfile[nLoopNo][1]);
                nLoopNo++;
                continue;
            }

            sRetVal = SendFile(nDevNo, FileDownfile[nLoopNo][1]);
            if ( sRetVal < 0 )
            {
                LogTerm( "\r\n====== SendFile result====== [%s] ���� ������ ����\n",
                          FileDownfile[nLoopNo][1]);
                nLoopNo++;
                continue;
            }

        }

        nLoopNo++;
    }

    stSendPkt.bCmd = ETB;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 0;

    sRetVal = SendPkt();//�ٺ������� �˸��� ��ȣ
    if ( sRetVal < 0 )
    {
        return ErrRet( sRetVal );
    }

    return SUCCESS;

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateSendData                                      	   *
*                                                                              *
*  DESCRIPTION:      ������� ���������͸� �����Ѵ�.                           *
*                   														   *
*                     														   *
*																			   *
*  INPUT PARAMETERS:  byte bCmd - ������ ��ɾ�                                *
*                     int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateSendData( byte bCmd, int nDevNo )
{
    short sRetVal = SUCCESS;

    switch( bCmd )
    {
        case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :   //Program Version
            DebugOut( "\r\n[1.CreateDownPar]" );
            sRetVal = CreateDownPar( nDevNo );
            return ErrRet( sRetVal );

        case NAK :
            sRetVal = CreateNAK( nDevNo );
            return ErrRet( sRetVal );

        case ACK :
        case MAIN_SUB_COMM_GET_TRANS_CONT :
            sRetVal = CreateACK( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :   //Program Version
            DebugOut( "\r\n[1.CreateReqImgVer]" );
            sRetVal = CreateReqImgVer( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :   //program Download
            DebugOut( "\r\n[1.CreateDownImg]" );
            sRetVal = CreateDownImg( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_GET_SUB_TERM_ID :  //SubId ���� ���� //0330 2005.2.28
            DebugOut( "\r\n[1.CreateReqSubTermID]" );
            sRetVal = CreateReqSubTermID( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER : //Voice Version //0330 2005.2.28
            DebugOut( "\r\n[1.CreateReqVoiceVer]" );
            sRetVal = CreateReqVoiceVer( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN ://Voice Download //0330 2005.2.28
            DebugOut( "\r\n[1.CreateDownVoice]" );
            sRetVal = CreateDownVoice( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REGIST_KEYSET :   //Keyset Result
            DebugOut( "\r\n[1.CreateReqKeySet]" );
            sRetVal = CreateReqKeySet( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_POLLING :        //polling
            DebugOut( "\n[1.CreatePoll]" );
            sRetVal = CreatePoll( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REQ_BL_SEARCH :     //polling
            DebugOut( "\r\n[6.CreateBLResult]" );
            sRetVal = CreateBLResult( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REQ_PL_SEARCH :     //polling
            DebugOut( "\r\n[6.CreatePLResult]" );
            sRetVal = CreatePLResult( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_GET_TRANS_FILE :  //TD File ���ۿ䱸
            DebugOut( "\r\n[1.CreateReqTDFile]" );
            sRetVal = CreateReqTDFile( nDevNo );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID :   //���� �ܸ��� ���̵� �ο�
            DebugOut( "\r\n[1.CreatePutSubTermID]" );
            sRetVal = CreatePutSubTermID( nDevNo );
            return ErrRet( sRetVal );

        default :
            return ErrRet( sRetVal );
    }
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqImgVer                                      	   *
*                                                                              *
*  DESCRIPTION:      ������ �߿������������ ��û�ϴ� �����͸� �����Ѵ�.       *
*                    ������ PSAMID,�ܸ���ID ������ ��û�ϴ� �����Ϳ� �Ǿ    *
*                    ������.                                                   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateReqImgVer( int nDevNo )
{
    short sRetVal = SUCCESS;

    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 25;

    DebugOut( "stSendPkt.abDataSide %d:", stSendPkt.nDataSize );

    //���� sniper sam id
    memcpy( stSendPkt.abData, gpstSharedInfo->abMainPSAMID, 16 );
    //�ܸ��� ���̵�
    memcpy( &stSendPkt.abData[16], gpstSharedInfo->abMainTermID, 9 );

    DebugOutlnASC( "abMainPSAMID :", gpstSharedInfo->abMainPSAMID, 16 );
    DebugOutlnASC( "abMainTermID :", gpstSharedInfo->abMainTermID, 9 );
    DebugOutlnASC( "\n<MainTerm> CreateReqImgVer stSendPkt Ready :",
                    &stSendPkt.abData[0], 25 );

    return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqTDFile                                      	   *
*                                                                              *
*  DESCRIPTION:      ������ ��縦 ���� �ŷ����������� ��û�ϴ� �����͸� ����  *
*                    �Ѵ�.   												   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateReqTDFile( int nDevNo )
{
    short sRetVal = SUCCESS;

    stSendPkt.bCmd = MAIN_SUB_COMM_GET_TRANS_FILE;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 18;

#ifdef  TEST_DAESA
    PrintlnASC( "��ûTD���ϸ� :", gpstSharedInfo->abTransFileName, 18 );
#endif

    memcpy( stSendPkt.abData, gpstSharedInfo->abTransFileName, 18 );

    return ErrRet( sRetVal );

}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateDownPar                                      	   *
*                                                                              *
*  DESCRIPTION:      ������ �Ķ���� ���� ������ �˸��� �����͸� �����Ѵ�.     *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateDownPar( int nDevNo )
{

    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_PARM_DOWN;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 0;

    return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateDownImg                                      	   *
*                                                                              *
*  DESCRIPTION:      ������ �߿��� ������ �˸��� �����͸� �����Ѵ�.   		   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateDownImg( int nDevNo )
{

    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_DOWN;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 12;
    memcpy(  stSendPkt.abData, BUS_SUB_IMAGE_FILE, 12 );  //���� sniper sam id

    return SUCCESS;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqSubTermID                                       *
*                                                                              *
*  DESCRIPTION:       ������ID������ ��û�ϴ� �����͸� �����Ѵ�.   		       *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateReqSubTermID( int nDevNo )
{
    stSendPkt.bCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 0;

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqVoiceVer                                        *
*                                                                              *
*  DESCRIPTION:       �������������� ������ ��û�ϴ� �����͸� �����Ѵ�.        *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateReqVoiceVer( int nDevNo )
{
    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_VOICE_VER;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 0;

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqVoiceVer                                        *
*                                                                              *
*  DESCRIPTION:       �������������� ������ �˸��� �����͸� �����Ѵ�.          *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateDownVoice( int nDevNo )
{
    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 8;
    memcpy( stSendPkt.abData, VOICE0_FILE, 8 );  //���� sniper sam id

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqKeySet                                          *
*                                                                              *
*  DESCRIPTION:       Keyset/�߻��� ��ϰ�� ������ ��û�ϴ� �����͸� �����Ѵ�.*
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateReqKeySet( int nDevNo )
{
    stSendPkt.bCmd = MAIN_SUB_COMM_REGIST_KEYSET;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 0;

    return SUCCESS;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreatePutSubTermID                                       *
*                                                                              *
*  DESCRIPTION:       ������ ID�� �ο��ϱ����� ������ �˸��� �����͸� �����Ѵ�.*
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreatePutSubTermID( int nDevNo )
{
    short sRetVal = SUCCESS;
    byte bCmd;
    byte pcCmdData[40];
    word wCmdDataSize;
    byte abSubTermID[10];

    stSendPkt.bCmd = MAIN_SUB_COMM_ASSGN_SUB_TERM_ID;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;

    GetSharedCmd( &bCmd, pcCmdData, &wCmdDataSize );
  
    memcpy( abSubTermID, &pcCmdData[(nDevNo -1) * 9 + 2], 9 );
    abSubTermID[9] = 0x00;

    stSendPkt.nDataSize = strlen( abSubTermID );
    memcpy( &stSendPkt.abData[0], abSubTermID, strlen( abSubTermID ) );
	DebugOut("&stSendPkt.abData[0] : %s",&stSendPkt.abData[0]);
    return sRetVal;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreatePoll                                  			   *
*                                                                              *
*  DESCRIPTION:       �⺻���� ������ ���� �����͸� �����Ѵ�.				   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreatePoll( int nDevNo )
{
    short sRetVal = SUCCESS;
    byte abCurrTime[14] = { 0, };

    stSendPkt.bCmd = MAIN_SUB_COMM_POLLING;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 50;

    TimeT2ASCDtime(  gpstSharedInfo->tTermTime, abCurrTime );
    memcpy( stSendPkt.abData, abCurrTime, 14 );
    memcpy( &stSendPkt.abData[14], gpstSharedInfo->abNowStationID, 7 );
    stSendPkt.abData[21] = gpstSharedInfo->boolIsDriveNow;
    memcpy( &stSendPkt.abData[22], gpstSharedInfo->abMainTermID, 9 );
    memcpy( &stSendPkt.abData[31], gpstSharedInfo->abTransFileName, 18 );
	stSendPkt.abData[49] = gpstSharedInfo->gbGPSStatusFlag;

    return ErrRet( sRetVal);
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateBLResult                                  		   *
*                                                                              *
*  DESCRIPTION:       BL �˻���� ������ ���� �����͸� �����Ѵ�.			   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreateBLResult( int nDevNo )
{
    short sRetVal = SUCCESS;

    stSendPkt.bCmd = MAIN_SUB_COMM_REQ_BL_SEARCH;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;

    if ( boolIsCardErrLog == TRUE )
    {
        stSendPkt.nDataSize =  sizeof( TRANS_INFO ) + 3;
    }
    else 
    {
        stSendPkt.nDataSize =  3;
    }
	memcpy( stSendPkt.abData, &bBLCheckStatus, 2 );
	stSendPkt.abData[2] = bBLCheckResult;
	
	bBLCheckStatus = 0;
	
	if ( boolIsCardErrLog == TRUE )
    {
        memcpy( &stSendPkt.abData[3], &stCardErrInfo, sizeof( TRANS_INFO ) );
    }

//	LogTerm( "\r\ [CreateBLResult]BL Check Result ==> %d\n", bBLCheckResult  );
#ifdef TEST_BLPL_CHECK
    printf( "\r\ [CreateBLResult] BL �˻� �������� : %d,BL Check Result ==> %d\n",
              , bBLCheckStatus, bBLCheckResult  );
#endif
    return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreatePLResult                                  		   *
*                                                                              *
*  DESCRIPTION:       PL �˻���� ������ ���� �����͸� �����Ѵ�.			   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ 	                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CreatePLResult( int nDevNo )
{
    short sRetVal = SUCCESS;

    stSendPkt.bCmd = MAIN_SUB_COMM_REQ_PL_SEARCH;
    stSendPkt.bDevNo = nDevNo + '0';
    stSendPkt.wSeqNo = 0;

    if ( boolIsCardErrLog == TRUE )
    {
        stSendPkt.nDataSize =  sizeof( TRANS_INFO ) + 3;
    }
    else 
    {
        stSendPkt.nDataSize =  3;
    }
	memcpy( stSendPkt.abData, &bPLCheckStatus, 2 );	
    stSendPkt.abData[2] = bPLCheckResult;

	bPLCheckStatus = 0;

    if ( boolIsCardErrLog == TRUE )
    {
        memcpy( &stSendPkt.abData[3], &stCardErrInfo, sizeof( TRANS_INFO ) );
    }
	
//	LogTerm( "\r\ [CreatePLResult]PL Check Result ==> %02x\n", bPLCheckResult );
	
    return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcRecvData                                  		   *
*                                                                              *
*  DESCRIPTION:       ��ɾ� ����� �����Ͽ� ������κ��� ���� ������ �Ľ��ϰ� *
*                     ó���Ѵ�.												   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void 							                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short ProcRecvData( void )
{
    short sRetVal = SUCCESS;

    switch( stRecvPkt.bCmd  )
    {
       /*
        * Program Version Check
        */
        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :       
            DebugOut( "\r\nRecvImgVer\r\n" );
            sRetVal = RecvImgVer();
            return ErrRet( sRetVal );
	   /*
		* Request SubTerminal ID
		*/
        case MAIN_SUB_COMM_GET_SUB_TERM_ID :        //
            DebugOut( "\r\nRecvSubTermID \r\n" );
            sRetVal = RecvSubTermID();

            return ErrRet( sRetVal );
	   /*
		* voice Version Check
		*/
        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER :    
            DebugOut( "\r\nRecvVoiceVer\r\n" );
            sRetVal = RecvVoiceVer();

            return ErrRet( sRetVal );
	   /*
		* KeySet Result
		*/			
        case MAIN_SUB_COMM_REGIST_KEYSET :          
            DebugOut( "\r\nRecvKeySet\r\n" );
            sRetVal = RecvKeySet();
            return ErrRet( sRetVal );
	   /*
		* ACK Response
		*/	
        case ACK :
            DebugOut( "\n[4.RecvACK-From %d�� �ܸ���]",nIndex+1 );
            sRetVal = RecvACKResp();
            return ErrRet( sRetVal );
	   /*
		* NAK Response
		*/
        case NAK :
            DebugOut( "\r\n  NAK \r\n" );
            sRetVal = RecvNAKResp();
            return ErrRet( sRetVal );
	   /*
		* Polling �������� BLSearch ����
		*/
        case MAIN_SUB_COMM_REQ_BL_SEARCH :  
            DebugOut( "[5.BLüũ-main]" );
            sRetVal = RecvBLCheckReq();
			if( sRetVal < SUCCESS )
			{
				LogTerm( "RecvBLCheckReq Error %02x\n", sRetVal );
			}
            bCurrCmd = MAIN_SUB_COMM_POLLING;
            return ErrRet( sRetVal );
	   /*
		* Polling �������� PLSearch ����
		*/
        case MAIN_SUB_COMM_REQ_PL_SEARCH :  
            DebugOut( "[5.PLüũ-main]" );
            sRetVal = RecvPLCheckReq();
			if( sRetVal < SUCCESS )
			{
				LogTerm( "RecvPLCheckReq Error %02x\n", sRetVal );
			}
            bCurrCmd = MAIN_SUB_COMM_POLLING;
            return ErrRet( sRetVal );
	   /*
		* Polling �������� ��縦 ���� �ŷ���������(TransFile) ����
		*/
        case MAIN_SUB_COMM_GET_TRANS_CONT : 
            DebugOut( "[5.�ŷ���������ó��]" );
            sRetVal = RecvTransDataOnPolling();
			if( sRetVal < SUCCESS )
			{
				LogTerm( "RecvPLCheckReq Error %02x\n", sRetVal );
			}
            bCurrCmd = MAIN_SUB_COMM_POLLING;
            return ErrRet( sRetVal );
		/*
		* �� �ܿ��� ������ ������ ������ �Ľ̿����� �����ڵ� ����
		*/
        default :
            sRetVal = ERR_MAINSUB_COMM_MAIN_RECV_DATA_PARSE;
            return ErrRet( sRetVal );
    }
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvNAKResp                                  		       *
*                                                                              *
*  DESCRIPTION:       ��ɾ� ����� �����Ͽ� ������κ��� ���� NAK������       *
*                     �Ľ��ϰ�                        						   *											   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void							                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvNAKResp( void )
{
    short sRetVal = SUCCESS;

    switch( bCurrCmd  )
    {
        case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :   // parameter file down
            DebugOut( "\r\n[�����������]Cmd M���� NAK �������� \r\n" );
            DisplayCommUpDownMsg( 2, 2 );
            //������ �����ٰ� �ᱹ �ϳ��� �����ؼ� NAK�� �� ���
            bCurrCmd = MAIN_SUB_COMM_SUB_TERM_VOICE_VER;
            return SUCCESS;

        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :   // Img file down
            DebugOut( "\r\n[�����������]Cmd D���� NAK �������� \r\n" );
            bCurrCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID;
            return SUCCESS;

        case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN :   // voice file down
            DebugOut( "\r\n[�����������]Cmd A���� NAK �������� \r\n" );
            bCurrCmd = MAIN_SUB_COMM_REGIST_KEYSET;
            return SUCCESS;

        default :
            return ErrRet( sRetVal );
    }

}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvACKResp                                  		       *
*                                                                              *
*  DESCRIPTION:       ��ɾ� ����� �����Ͽ� ������κ��� ���� ACK������       *
*                     �Ľ��ϰ�                        						   *											   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvACKResp( void )
{
    int nDevNo;
    short sRetVal = SUCCESS;
    nDevNo = stRecvPkt.bDevNo;

    switch( bCurrCmd  )
    {
        case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :   // parameter file down
            DebugOut( "\r\n[����ó��]Cmd M \r\n" );
            DisplayCommUpDownMsg( 2, 2 );
            //��� ������ ���� ��� VOICE��
            bCurrCmd = MAIN_SUB_COMM_SUB_TERM_VOICE_VER;
            DebugOut( "\n[M : �Ķ�������� �ٿ�ε� �Ϸ�]\n" );
            return SUCCESS;

        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :   // img file down
            DebugOut( "\r\n[����ó��]Cmd D  \r\n" );
            DisplayCommUpDownMsg( 2, 2 );
            memcpy( gpstSharedInfo->abSubVer[nDevNo-1],
                    gpstSharedInfo->abMainVer, 4 );
            // 'G' command��
            bCurrCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID;
            DebugOut( "[D : ���α׷� �ٿ�ε� �Ϸ�]\n" );
            return SUCCESS;

        case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN :   // voice file down
            DebugOut( "\r\n[����ó��]Cmd A  \r\n" );
            DisplayCommUpDownMsg( 2, 3 );
            memcpy( gpstSharedInfo->abSubVoiceVer[nDevNo-1],
                    gpstSharedInfo->abMainVoiceVer, 4 );
            //������ K
            bCurrCmd = MAIN_SUB_COMM_REGIST_KEYSET;
            DebugOut( "[A : �������� �ٿ�ε� �Ϸ�]\n" );
            return SUCCESS;

        case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID :   //�����ܸ��� ID �ο�
            DebugOut( "\n[5.�����޸� I cmd��� ����" );
            //SetSharedDataforCmd( MAIN_SUB_COMM_ASSGN_SUB_TERM_ID, "0", 1 );
            return SUCCESS; //ErrRet( sRetVal );

        case MAIN_SUB_COMM_POLLING : // ���� ���信�� ���ؼ� ACK�� ���� ���
            DebugOut( "[5.�ܼ���������-BLPL��û/�ŷ��������� ����]" );
            gpstSharedInfo->bPollingResCnt = 1;
            return SUCCESS;

        default :
            //sRetVal = ERR_CODE;
            return ErrRet( sRetVal );
    }
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvImgVer                                  		       *
*                                                                              *
*  DESCRIPTION:       ��ɾ� ����� �����Ͽ� ������κ��� ������� �߿��� ���� *
*                     �Ľ��ϰ� ó���Ѵ�.              						   *											   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvImgVer( void )
{
    short sRetVal = SUCCESS;
    int nDevNo = 0;
    time_t tCurrTime;
    byte abCurrDtime[15] = { 0, };
    bool boolIsImgVerChangeNeeded = FALSE;

    nDevNo = stRecvPkt.bDevNo;

    GetRTCTime( &tCurrTime );
    TimeT2ASCDtime( tCurrTime, abCurrDtime );


    if ( stRecvPkt.nDataSize == 37 )
    {
        memcpy( gpstSharedInfo->abSubVer[nDevNo-1], stRecvPkt.abData, 4 );
        memcpy( gpstSharedInfo->abSubCSAMID[nDevNo-1], &stRecvPkt.abData[4], 8 );
        memcpy( gpstSharedInfo->abSubPSAMID[nDevNo-1],
                &stRecvPkt.abData[12], 16 );
        DebugOutlnASC( "SubTerm Imgage Version Received : ",
                       stRecvPkt.abData, 4 );

        memcpy( &gpstSharedInfo->abSubPSAMPort[nDevNo-1],
                &stRecvPkt.abData[28], 1 );
        DebugOut( "\nabSubPSAMPort [%d]\r\n",
                  gpstSharedInfo->abSubPSAMPort[nDevNo-1] );
        DebugOutlnASC( "PSamID",
                    gpstSharedInfo->abSubPSAMID[nDevNo-1], 16   );

        memcpy( &gpstSharedInfo->abSubISAMID[nDevNo-1],
                &stRecvPkt.abData[29], 7 );

	// TPSAM ���� 
        memcpy( &gpstSharedInfo->abSubPSAMVer[nDevNo-1],
                      &stRecvPkt.abData[36], 1 );

	DebugOut( "\n TPSAM ���� => ������ %d�� %d �̴� \n", nDevNo, 
			  gpstSharedInfo->abSubPSAMVer[nDevNo-1]);
        
        if  ( memcmp( gpstSharedInfo->abMainVer,
                      gpstSharedInfo->abSubVer[nDevNo-1], 4 )  > 0 )
        {
            //���� Current
            DebugOut( "\r\nboolIsImgVerChangeNeeded=TRUE����" );
            boolIsImgVerChangeNeeded = TRUE;
			printf( "[RecvImgVer] �����ܸ��� : " );
			PrintASC( gpstSharedInfo->abMainVer, 4 );
			printf( ", %d�� �����ܸ��� : ", nDevNo );
			PrintASC( gpstSharedInfo->abSubVer[nDevNo-1], 4 );
			printf( "\n" );
        }
        else
        {
			printf( "[RecvImgVer] �����ܸ��� : " );
			PrintASC( gpstSharedInfo->abMainVer, 4 );
			printf( ", %d�� �����ܸ��� : ", nDevNo );
			PrintASC( gpstSharedInfo->abSubVer[nDevNo-1], 4 );
			printf( "\n" );
        }

        if ( nIndex == gbSubTermCnt -1 )
        {
            DebugOut( "������ �ܸ��⿡ ���� 'V'���� ��" );
            if ( PSAMIDCompareMainWithSubTerm() == 1 )
            {
               DebugOut( "\r\n sim_flag = 0 ���� �������" );
            }
            else
            {
                DebugOut( "\r\n sim_flag = 1 ���� ������" );
            }
        }

        sRetVal = SUCCESS;
    }
    else
    {
        sRetVal = ERR_DATASIZE_SUBTERM_SEND_BY_CMD_V;
    }

    DebugOut( "\n[V : ����üũ �Ϸ�]\n" );

    if  ( boolIsImgVerChangeNeeded == TRUE )
    {
        bCurrCmd = MAIN_SUB_COMM_SUB_TERM_IMG_DOWN;
		printf( "[RecvImgVer] %d�� �����ܸ��⿡ ���� �̹��� �ٿ�ε� �ʿ�\n",
			nDevNo );
    }
    else
    {
        bCurrCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID;
		printf( "[RecvImgVer] %d�� �����ܸ��⿡ ���� �̹��� �ٿ�ε� ���ʿ�\n",
			nDevNo );
        DebugOut( "[D : �ٿ�ε� ���ʿ�]\n" );
    }


    return ErrRet( sRetVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvSubTermID                                  		   *
*                                                                              *
*  DESCRIPTION:       ��ɾ� ����� �����Ͽ� ������κ��� ������� ������ID��  *
*                     �Ľ��ϰ� ó���Ѵ�.              						   *											   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvSubTermID( void )
{
    int nDevNo;
    short sRetVal;
    byte abTmpSubID[10] = { 0, };

    nDevNo = stRecvPkt.bDevNo;

    if ( stRecvPkt.nDataSize == 9 )
    {
        //���� �ܸ��� ���̵� üũ
        memcpy( abTmpSubID, stRecvPkt.abData, 9 );
        DebugOutlnASC( "tempSubId", abTmpSubID, 9);

        if ( CheckSubTermID( abTmpSubID ) == SUCCESS )
        {
            memcpy( gpstSharedInfo->abSubTermID[nDevNo-1], abTmpSubID, 9 );
            DebugOut( "\r\n SubId[%d][%s]\r\n",
                      nDevNo-1,
                      gpstSharedInfo->abSubTermID[nDevNo-1] );
            sRetVal = SUCCESS;
            DebugOut( "[G : �����ܸ���ID ���ſϷ�]\n" );
            bCurrCmd = MAIN_SUB_COMM_SUB_TERM_PARM_DOWN;
        }
        else
        {
            DebugOut( "\r\n CheckSubTermID check��� ����-NULL��" );
            sRetVal = ERR_DATA_SUBTERM_SEND_BY_CMD_G;
        }
    }
    else
    {
        sRetVal = ERR_DATASIZE_SUBTERM_SEND_BY_CMD_G;
    }


    return ErrRet( sRetVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvVoiceVer                                  		   *
*                                                                              *
*  DESCRIPTION:       ��ɾ� ����� �����Ͽ� ������κ��� ������� ����������  *
*                     �Ľ��ϰ� ó���Ѵ�.              						   *											   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvVoiceVer( void )
{
    short sRetVal = SUCCESS;
    int nDevNo = 0;
    bool boolIsVoiceVerChangeNeeded = FALSE;

    nDevNo = stRecvPkt.bDevNo;

    if ( stRecvPkt.nDataSize == 4 )
    {
        memcpy( gpstSharedInfo->abSubVoiceVer[nDevNo-1], stRecvPkt.abData, 4 );
        DebugOut( "SubTerm Voice Version Received [%s]\n",
                  gpstSharedInfo->abSubVoiceVer[nDevNo-1]);

        if  ( memcmp( gpstSharedInfo->abMainVoiceVer,
                      gpstSharedInfo->abSubVoiceVer[nDevNo-1], 4
                    )  > 0 )  //���� Current
        {
            boolIsVoiceVerChangeNeeded = TRUE;
        }

        sRetVal = SUCCESS;
    }
    else
    {
        sRetVal = ERR_DATASIZE_SUBTERM_SEND_BY_CMD_H;
    }

    DebugOut( "[H : �������� üũ �Ϸ�]\n" );
    if  ( boolIsVoiceVerChangeNeeded == TRUE )
    {
        bCurrCmd = MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN;
    }
    else
    {
        bCurrCmd = MAIN_SUB_COMM_REGIST_KEYSET;
        DebugOut( "[A : �������� �ٿ�ε� ���ʿ�]\n" );;
    }

    return ErrRet( sRetVal );
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvKeySet                                  		       *
*                                                                              *
*  DESCRIPTION:       ��ɾ� ����� �����Ͽ� ������κ��� �������             *
*                     keyset/����� ��ϰ�� ������ �Ľ��ϰ� ó���Ѵ�. 		   *											   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvKeySet( void )
{
    short sRetVal = SUCCESS;
    int nDevNo = 0;
    int i = 0;
	/* 
	 * ������ PSAM�� ����κ��� ���� Keyset/idcenter��� �������� 
	 */ 
    bool boolIsRegistSubTermPSAM = TRUE;

    nDevNo = stRecvPkt.bDevNo;

    if ( stRecvPkt.nDataSize == 2 )
    {
        memcpy( &stSubTermPSAMAddResult.aboolIsKeySetAdded[nDevNo-1], stRecvPkt.abData, 1 );
        memcpy( &stSubTermPSAMAddResult.aboolIsIDCenterAdded[nDevNo-1],
                &stRecvPkt.abData[1], 1 );

        DebugOut( "[K : Keyset Write ��� ���� �Ϸ� : KeySet[%s] IDCenter[%s]\n",
                (( stSubTermPSAMAddResult.aboolIsKeySetAdded[nDevNo-1] == 0x01 )?
                    "����" : "����" ),
                (( stSubTermPSAMAddResult.aboolIsIDCenterAdded[nDevNo-1] == 0x01 )?
                    "����" : "����" ));

        if ( nIndex == gbSubTermCnt -1 ) //������ �ܸ��⿡ ���� 'K'���� ��
        {

            for( i = 0; i < gbSubTermCnt; i++ )
            {
                if ( ( stSubTermPSAMAddResult.aboolIsKeySetAdded[i] == FALSE )  ||
                     ( stSubTermPSAMAddResult.aboolIsIDCenterAdded[i] == FALSE ) )
                {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                    DebugOut( "[������ %d���������� ��Ͻ��й߻�]\n", i+1);
#endif
                    boolIsRegistSubTermPSAM = FALSE;
                    break;
                }
            }

            if ( ( boolIsRegistMainTermPSAM== FALSE )&&
                 ( boolIsRegistSubTermPSAM == FALSE ) )
            {
                //   �� ������� ��Ͻ���
                DebugOut( "[K : Keyset Write ������� : ��������� ��Ͻ���]\n" );
                DebugOut( "[��ü CMD����Ϸ�]////////////////////////////////\n" );
//                ctrl_event_info_write( KEYSET_FAIL3 );
                sRetVal = SUCCESS;
                /* ctrl_event_info_write�Լ��� ���ְ� ������ƾ ������ �� 
                   �����ڵ� ERR_MAINSUB_COMM_ALLTERM_KEYSET_IDCENTER_ADD_PSAM;
                   �� ��ü  */
            }
            else if ( ( boolIsRegistMainTermPSAM == FALSE )&&
                      ( boolIsRegistSubTermPSAM == TRUE ) )
            {
                //�����⸸ ��Ͻ���
                DebugOut( "[K : Keyset Write ������� : �����⸸   ��Ͻ���]\n" );
                DebugOut( "[��ü CMD����Ϸ�]////////////////////////////////\n" );
//                ctrl_event_info_write( KEYSET_FAIL1 );
                sRetVal = SUCCESS;
                /* ctrl_event_info_write�Լ��� ���ְ� ������ƾ ������ �� 
                   �����ڵ� ERR_MAINSUB_COMM_MAINTERM_KEYSET_IDCENTER_ADD_PSAM;
                   �� ��ü  */
            }
            else if ( ( boolIsRegistMainTermPSAM == TRUE )&&
                      ( boolIsRegistSubTermPSAM == FALSE ) )
            {
                //�����⸸ ��Ͻ���
                DebugOut( "[K : Keyset Write ������� : �����⸸ ��Ͻ���]\n" );
                DebugOut( "[��ü CMD����Ϸ�]///////////////////////////////\n" );
//                ctrl_event_info_write( KEYSET_FAIL2 );
                /* ctrl_event_info_write�Լ��� ���ְ� ������ƾ ������ �� 
                   �����ڵ� ERR_MAINSUB_COMM_SUBTERM_KEYSET_IDCENTER_ADD_PSAM;
                   �� ��ü  */
                sRetVal = SUCCESS;
                
            }
            else
            {
                DebugOut( "[K : Keyset Write ������� : ����]\n" );
                DebugOut( "[��ü CMD����Ϸ�]////////////////////////////////\n" );
                DebugOut( "\r\n KEYSET_IDCENTER_SUCC " );
//                ctrl_event_info_write( KEYSET_SUCC );
                sRetVal = SUCCESS;
            }

            boolIsCmdOneCycle = TRUE;
        }

        boolIsCmdOneCycle = TRUE;

    }
    else
    {
        DebugOut( "ERR_DATASIZE_SUBTERM_SEND_BY_CMD_K" );
        sRetVal = ERR_DATASIZE_SUBTERM_SEND_BY_CMD_K;
    }

    return ErrRet( sRetVal );
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvBLCheckReq                                  		   *
*                                                                              *
*  DESCRIPTION:       ������ ���� �������� ������κ��� ���� BL�˻���û�� ���� *
*                     BL�˻� �� ������� �����Ѵ�.					 		   *										   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvBLCheckReq( void )
{
    short sRetVal = SUCCESS;
    byte abPrefix[6] = { 0, };
    dword dwCardNum = 0;
    int nDevNo = 0;
    
    short sSearchErrLog = SUCCESS;

  //  printf( "\nRecvBLCheckReq\n" );
    boolIsCardErrLog = FALSE;
    nDevNo = stRecvPkt.bDevNo;

    memset( &stCardErrInfo, 0, sizeof( stCardErrInfo ) );
    memcpy( stCardErrInfo.abCardNo, stRecvPkt.abData, 20 );
    memcpy( abPrefix, &stRecvPkt.abData[20], 6 );
    memcpy( ( byte*)&dwCardNum, &stRecvPkt.abData[26], 4 );

    sSearchErrLog = SearchCardErrLog( &stCardErrInfo );
    if ( sSearchErrLog == SUCCESS )
    {
    	boolIsCardErrLog = TRUE;
    }
	 
	
    //DebugOut("SearchBL����");
    sRetVal = SearchBLinBus( stCardErrInfo.abCardNo, abPrefix, dwCardNum, &bBLCheckResult );

    if ( sRetVal != SUCCESS )
    {
    	LogTerm( "RecvBLCheckReq-SearchBLinBus error �߻�( %02x )\n", 
				 			 sRetVal );
    }
	/*
	* BL CHECK �������� ���� - ������� ������ ����
	*/
	memcpy( &bBLCheckStatus, &sRetVal, 2); 
//	printf("bBLCheckStatus-%02d", bPLCheckStatus);	

#ifdef TEST_BLPL_CHECK
    PrintlnASC( "\r\nSubTerm BL Check Card No ==> ",
                abCardNo, 20 );
    PrintlnASC( "\r\nSubTerm BL Check Card Prefix ==> ",
                abPrefix, 6 );
    printf( "\r\nSubTerm [%02d] BL Check Card CardNum ==> %ld\n",
            nDevNo, dwCardNum );
    printf( "\r\nSubTerm [%02d] BL Check Result ==> %d\n",
            nDevNo, bBLCheckResult );
#endif

    return SUCCESS;

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvPLCheckReq                                  		   *
*                                                                              *
*  DESCRIPTION:       ������ ���� �������� ������κ��� ���� PL�˻���û�� ���� *
*                     PL�˻� �� ������� �����Ѵ�.					 		   *										   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvPLCheckReq( void )
{
    short sRetVal = SUCCESS;
    dword dwAliasNo = 0;
    int nDevNo = 0;
    short sSearchErrLog = SUCCESS;

//    printf( "\nRecvPLCheckReq\n" );
    boolIsCardErrLog = FALSE;
    nDevNo = stRecvPkt.bDevNo;

    memset( &stCardErrInfo, 0, sizeof( stCardErrInfo ) );
    memcpy( stCardErrInfo.abCardNo, stRecvPkt.abData, 20);
    memcpy( ( byte*)&dwAliasNo, &stRecvPkt.abData[20], 4 );

    
    sSearchErrLog = SearchCardErrLog( &stCardErrInfo );
    if ( sSearchErrLog == SUCCESS )
    {
    	boolIsCardErrLog = TRUE;
    }

    sRetVal = SearchPL( dwAliasNo, &bPLCheckResult );

    if ( sRetVal != SUCCESS )
    {
		LogTerm( "RecvPLCheckReq-SearchPLinBus error �߻�( %02x )\n", 
				 			 sRetVal );
    }
	/*
	* PL CHECK �������� ���� - ������� ������ ����
	*/
	memcpy( &bPLCheckStatus, &sRetVal, 2); 
//	printf("bPLCheckStatus-%02d", bPLCheckStatus);
    

#ifdef TEST_BLPL_CHECK
    printf( "[term_comm]���� [%02d]�� ��û PL Check Card AliasNo ==> [%02ld,
            Result ==> %d\n\n", nDevNo, dwAliasNo, bPLCheckResult );
#endif

    return SUCCESS;
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvTransDataOnPolling                                   *
*                                                                              *
*  DESCRIPTION:       ������ ���� �������� ������κ��� ���� �ŷ�����(202BYTE) *
*                     �� �����ŷ������� �߰��Ѵ�.					 		   *										   *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void								                       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:  Write_Trans_Data.c�� WriteTransDataForCommProcess()�Լ��� ȣ���Ͽ�*
*            �������� �ŷ��������Ͽ� ��ģ��.                                   *
*                                                                              *
*******************************************************************************/
short RecvTransDataOnPolling( void )
{
    short sRetVal = SUCCESS;

    if ( stRecvPkt.nDataSize == 202 )
    {
        DebugOut( "WriteTransDataForCommProcess start\n" );
        sRetVal = WriteTransDataForCommProcess( stRecvPkt.abData );
        DebugOut( "WriteTransDataForCommProcess end\n" );
    }
    else
    {
        DebugOut( "�����ŷ����� ���� ������ 202byte�ƴ�\n" );
        sRetVal = ERR_DATASIZE_SUBTERM_SEND_BY_CMD_X;
		LogTerm( "�����ŷ����� ���� ������ 202byte�ƴ�\n" );
        //�Ǵ� ����Ÿ�� ���� ���
    }

	if ( sRetVal < 0 )
	{
		LogTerm( "�ŷ����� �����ͼ��� ���� %02x\n", sRetVal );
	}
    return ErrRet( sRetVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvTDFileFromSubTerm                            		   *
*                                                                              *
*  DESCRIPTION:       ������κ��� ��縦 ���� �ŷ����������� �����Ѵ�.        *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int  nDevNo - �����ܸ��� ��ȣ                            *
*                     byte *pFileName - ������ϸ�                             *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvTDFileFromSubTerm( int  nDevNo, byte *pFileName )
{

    short sRetVal = SUCCESS;
    char achRecvFileName[19] = { 0, };
    char achRecvFileReName[19] = { 0, };

    usleep( 100000 );
    memcpy( achRecvFileName, pFileName, 14 );
    memcpy( &achRecvFileName[14], ".tmp", 4 );

	printf( "[RecvTDFileFromSubTerm] �����κ��� ���� : %s ", achRecvFileName );

    sRetVal = RecvFile( nDevNo, achRecvFileName );

    if (( sRetVal < 0 ) && ( sRetVal != ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST ))
    {
#ifdef TEST_DAESA
        printf( "\r\n ������κ��� TD���� ����  ���� : [%d]\r\n", sRetVal );
        printf( "\r\n ���� ���� ��� ���μ��� ����\n" );
#endif
        unlink( achRecvFileName );
    }
    else if ( sRetVal == ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST)
    {
        unlink( achRecvFileName );
#ifdef  TEST_DAESA
        printf( "�������û �ŷ����� ������ ���� �ӽ�ȭ�ϻ���\n" );
#endif
    }
    else
    {
        memcpy( achRecvFileReName, pFileName, 15 );
        achRecvFileReName[15] = nDevNo+ '0' ;
        achRecvFileReName[16] = '0';

        DebugOut( "\r\n recv Temp Filerename [%s] \r\n", achRecvFileReName );
        rename( achRecvFileName, achRecvFileReName );
        DebugOut( "\r\n SubTermTDFILE End ret : [%d]", sRetVal );
#ifdef  TEST_DAESA
        printf( "\r\n TD file ���� ����\r\n" );
#endif
    }

    return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ChkSubTermImgVerOldProtcol                               *		       *
*                                                                              *
*  DESCRIPTION:       �����Ⱑ ���������ݷ� �Ǿ��ִ��� �˻��Ѵ�.               *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �����ܸ��� ��ȣ                             *
*                     char *pachhNewVerYn - ���������� ���� T: �� F:��         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short ChkSubTermImgVerOldProtcol( int nDevNo, char *pachhNewVerYn )
{
    short sRetVal;

    RS485_FILE_HEADER_PKT_OLD stFileHeaderPkt;
	memset( &stFileHeaderPkt, 0x00, sizeof(RS485_FILE_HEADER_PKT_OLD));

#ifdef TEST_SYNC_FW
    printf( "==========version check start!!!===========\n" );
#endif
    /* ����üũ ��ɾ� */
    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;
	/* �ܸ��� ��ȣ */
    stSendPkt.bDevNo = nDevNo + '0';
	/* ������ ������ */
    stSendPkt.nDataSize = sizeof( RS485_FILE_HEADER_PKT_OLD );

    /* 1.CSAMID BCD��ȯ �� stFileHeaderPkt ����ü ���� */
    ASC2BCD( gpstSharedInfo->abMainCSAMID, stFileHeaderPkt.achFileName, 8 );
	/* 2.PSAMID BCD��ȯ �� stFileHeaderPkt ����ü ���� */
    ASC2BCD( gpstSharedInfo->abMainPSAMID, &stFileHeaderPkt.achFileName[4], 16 );
	
	/* 3.�����ܸ��� ID stFileHeaderPkt ����ü ���� */
    memcpy( &stFileHeaderPkt.achFileName[12], gpstSharedInfo->abMainTermID, 9 );
    /* 4.������ ���� stFileHeaderPkt ����ü ���� */
    memcpy( &stFileHeaderPkt.achFileName[21], gpstSharedInfo->abMainVer, 4 );
	/* �۽ű���ü ������ ������ �ܸ����ȣ ������ */
    memcpy( &stSendPkt.abData[0], &stSendPkt.bDevNo, sizeof( stSendPkt.bDevNo ) );
	/* 1,2,3,4���� ������ stFileHeaderPkt����ü�� �۽ű���ü�� ������ ������ ���� */	
    memcpy( &stSendPkt.abData[1], &stFileHeaderPkt, sizeof(RS485_FILE_HEADER_PKT_OLD));

	/* 
	 * ������ �۽� 
	 */ 
    sRetVal = SendPkt();
    if ( sRetVal < 0 )
    {
        printf( "\r\n ChkSubTermImgVerOldProtcol rs485PktSend sRetVal[%d] \r\n", 
				sRetVal );
        return sRetVal;
    }

	/* 
	 * ������ ����  
	 */
    sRetVal = RecvPkt( 1000, nDevNo );
    if ( sRetVal < 0 )
    {
        printf( "\r\n ChkSubTermImgVerOldProtcol rs485PktRecv sRetVal[%d] \r\n",
				sRetVal );
        return sRetVal;
    }
	/* 
	 * �� �������ݷ� ACK���Ž� ���α׷� ���������� ������ ��/������ �Ǵ�.
	 */
    if ( stRecvPkt.nDataSize == 1 && stRecvPkt.abData[0] == ACK )
    {
#ifdef TEST_SYNC_FW    
        printf( "V Command ACK Received [%d]\n", nDevNo );
#endif
        /* 
         * Version ������ ���� 
         */
        sRetVal = RecvPkt( 1000, nDevNo );
        /* 
         * nDataSize ������ ���� 
         * - �� �������� ���������� ������� 17
         *   STX LEN LEN CMD DEVNO DATA ETX CRC CRC CRC CRC
         */
	    DebugOut( "�� �������ݷ� ���α׷� version �����\n" );
	    /*
	     *  program version copy- 4byte
	     *  new - 3byte
	     *  ? - 1byte
	     * 0000000000 - 8byte
	     * ��Ʈ��ȣ 0x32 - 1byte 
	     */
	    /*
	     *  new�� ������ ���ٿ� ������.
	     *  ���� : 0401������ ����, �� 0402,0403�� �� �����������̶�� �� �� ����.
	     *         0401�� ��������ſ� ������ ���� ���� �� �������ݷ� �����Ͽ�
	     *         0402,0403�� ���� ������ ��� ������������� ����Ʈ���� 
	     *         ��,�����θ� �Ǵ��ϱ⿡ �����Ƿ� new�� ���ٿ� ������ 04xx���
	     *         �����⿡�� new�� �������� ���Ͽ� ���������ݷ� �����Ѵ�.
	     */    
		
		/*
		 * TPSAM Ȯ�ο����� 1byte�þ�� 18byte�� �Ǿ��� 
		 */
        if ( stRecvPkt.nDataSize == 17 || stRecvPkt.nDataSize == 18 )
        {
        	printf("[%d] ��", nDevNo );
        	PrintlnASC( "������ Ver :", gpstSharedInfo->abSubVer[nDevNo-1], 4);
			/* ������ ���� �����޸𸮿� Copy  */ 
            memcpy( gpstSharedInfo->abSubVer[nDevNo-1], stRecvPkt.abData, 4 );

            if ( memcmp( &stRecvPkt.abData[4], "new", 3 ) == 0 )
            {
			    /* new��� �����Ͱ� ���� ������� ���������� ����  */            
                pachhNewVerYn[nDevNo-1] = SUBTERM_USE_NEW_PROTOCOL;
                printf( "[%d] �� ������ F/W�� ���������� ��� ����\n", nDevNo );
            }
            else
            {
				/* new��� �����Ͱ� ����������  ���������� ���� */                  
                pachhNewVerYn[nDevNo-1] = SUBTERM_USE_OLD_PROTOCOL;
                printf( "[%d] �� ������ F/W�� ���������� ��� ����\n", nDevNo );

                return sRetVal;
            }
        }
        else
        {
            printf( "VersionTest After ACK Received..\n" );
            printf( "Data = %s====\n", stRecvPkt.abData );
        }

        return SUCCESS;
    }
	
	/* 
	 * �� �������ݷ� NAK���Ž� 
	 */	
    else if ( stRecvPkt.nDataSize == -1 && stRecvPkt.abData[0] == NAK )
    {
        DebugOut ( "V Command NAK Received [%d]\n", nDevNo );
    }
	/* 
	 * �� �̿��� ���� ��� 
	 */		
    else
    {
        DebugOut( "VersionTest Received Data = %s====\n", stRecvPkt.abData );
    }

    return sRetVal;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendSubTermImg				                           *		       *
*                                                                              *
*  DESCRIPTION:       �� ���������� ����Ͽ� ������ ���α׷� �ٿ�ε� ��ɾ *
*                     �����Ѵ�.                                                *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �ܸ��� ��ȣ                                 *
*                     char* pchFileName - ������ ���������α׷� ����           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short SendSubTermImgOldProtcol( int nDevNo, char* pchFileName )
{
    short sRetVal;
    int fdFile = -1;
    RS485_FILE_HEADER_PKT_OLD fileHeaderPkt;/* ���۱���ü�� �ֱ� ���� �ӽú��� */

    /* �ʱ�ȭ */
    memset( &fileHeaderPkt, 0, sizeof( RS485_FILE_HEADER_PKT_OLD ) );
	/* 
	 * ���������ϸ� �ӽú����� ���� 
	 * 03xx�� c_ex_pro.dat������ ���� ���� ���ϸ����� ���� �ϹǷ� 
	 * �������� �� c_ex_pro.dat�� �����Ѵ�. 
	 * ������ �Ķ���� pchFileName�� bus100���� �����Ѵ�.
	 * �����⿡ bus100 + 18byte(�Ͻù���)�� �ٿ��� ������ �ϹǷ�...
	 */
    strcpy( fileHeaderPkt.achFileName, BUS_SUB_IMAGE_FILE );

    /* bCmd : ���� ���� */
    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD;
    /* bDevNo : �ܸ��� ��ȣ ���� */
    stSendPkt.bDevNo = nDevNo + '0';
    /* nDataSize : �ܸ��� ��ȣ ���� */	
    stSendPkt.nDataSize = sizeof( RS485_FILE_HEADER_PKT_OLD );

    /* abData�� bDevNo ���� */	
    memcpy( &stSendPkt.abData[0], &stSendPkt.bDevNo, sizeof( stSendPkt.bDevNo ) );
    /* abData�� �ӽú����� fileHeaderPkt�� Copy */	
    memcpy( &stSendPkt.abData[1], &fileHeaderPkt, sizeof( RS485_FILE_HEADER_PKT_OLD ) );

   /*
    * ��Ŷ ����
    */
    sRetVal = SendPkt();
    if ( sRetVal < 0 )
    {
        printf( "\r\n Upload rs485PktSend sRetVal [%d] \r\n", sRetVal );
        return ErrRet( sRetVal );
    }
   /*
    * ��Ŷ ����
    */
    sRetVal = RecvPkt( 1000, nDevNo );
    if ( sRetVal < 0 )
    {
        printf( "\r\n Upload rs485PktRecv sRetVal [%d] \r\n", sRetVal );
        return ErrRet( sRetVal );
    }
   /*
    * ACK���Ž� 
    */
    if ( stRecvPkt.nDataSize == 1 && stRecvPkt.abData[0] == ACK )
    {
        DebugOut( "ACK Received\n" );

        DisplayCommUpDownMsg( 1 , 2 );
        usleep( 100000 );
		
		/* �� ���������� �̿��Ͽ� �������� ���� */
		printf(" �� ���������� �̿��Ͽ� �������� ���� \n");
        sRetVal = SendSubTermImgFileOldProtocol( nDevNo, pchFileName );
        if ( sRetVal < 0 )
        {
            DebugOut( "\r\n SendSubImgFileOld sRetVal [%d] \r\n", sRetVal );
        }

        DisplayCommUpDownMsg( 2 , 2 );

        return ErrRet( sRetVal );
    }
   /*
    * NAK���Ž�  
    */	
    else if ( stRecvPkt.nDataSize == -1 && stRecvPkt.abData[0] == NAK )
    {
        DebugOut( "NAK Received\n" );
    }

    close( fdFile );

    return ErrRet( sRetVal );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendSubTermImgFileOldProtocol                            *		       
*                                                                              *
*  DESCRIPTION:       03xx������ ����ϴ� �����̹����� bus100�� ���Ͽ� ���ڿ�  *
*                     ������ ���ٿ� ���������ݷ� �����Ѵ�.                     *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  int nDevNo - �ܸ��� ��ȣ                                 *
*                     char* pchFileName - ������ ���������α׷� ����           *
* 																			   *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                     ������ - ���� ����                                       *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     	 ��/�� ���������� �ִ���Ŷ����� ���� �޶� �����͸�      *
*                    ���� ������ ���� stSendPkt����ü�� abData������ ���      *
*                    ���� �ʰ� abSendData�� ���� �����Ͽ� ����ϰ� �ִ�.       *
*                    �̸� �˷��ֱ����� ���������� boolIsMainSubCommFileDownIng *
*                    Flag�� ����ϰ� �ִ�.                                     *
*                                                                              *
*******************************************************************************/
short SendSubTermImgFileOldProtocol( int nDevNo, char* pchFileName )
{
    int fdFile;
    int nByte;
    short sRetVal;
    int nEOTCheck = 0;
    /* �����Ͻÿ� �������߿��� ���� */	
	byte abCurrDataNVer[18] = { 0, }; 
	time_t tCurrentDtime = 0;
   /* 
    * �� �������ݷ� �����ϱ����� ����ü �ʱ�ȭ
    */
	memset( abSendData, 0x00, sizeof( abSendData ));
   /* 
    * bus100�̹����� ��Ŷ���� ������ �ڿ� ����ð��� ���ι����� ���ٿ� ������ 
    */
   /*
    * ����ð� 14byte Copy
    */	
    GetRTCTime( &tCurrentDtime );
    TimeT2ASCDtime( tCurrentDtime, abCurrDataNVer );
    abCurrDataNVer[12] = '0';
    abCurrDataNVer[13] = '0';
   /* 
    * ���ι����� �����ϰ� ���� 4byte�� Copy
	*/
    memcpy( &abCurrDataNVer[14], MAIN_RELEASE_VER, 4 );

	
   /*
    * BUS100 ���� ���� 
    */
    fdFile = open( pchFileName, O_RDONLY, OPENMODE );
    if ( fdFile < 0 )
    {
        DebugOut( "\r\n ���� ���� �ȵ�\r\n" );
        return -1;
    }
	/*
     * BUS100������ ���� �Ͻÿ� ��������� �߰� 
     */
    write( fdFile, abCurrDataNVer, 14 );
    write( fdFile, MAIN_RELEASE_VER, 4 );

	lseek( fdFile, 0L, SEEK_SET );
		
   /*
    * ���� �ٿ�ε� ������ ��Ÿ���� �������� ����
    * TRUE :  SendPkt()�Լ������� �����͹��ۿ� abSendData���� Copy�Ͽ� ����
    * FALSE :  SendPkt()�Լ������� �����͹��ۿ� stSendPkt.abData���� Copy�Ͽ� ����
    */
    boolIsMainSubCommFileDownIng = TRUE;

	while(1)
    {
		/* ��ɾ� : ������������ �̿��� ������ �߿��� �ٿ�ε� 'D'*/
		stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD;
		/* �ܸ��� ��ȣ : �ܸ����ȣ + ASCII 0 */		
        stSendPkt.bDevNo = nDevNo + '0';   
		
        nByte = read( fdFile, abSendData, DOWN_DATA_SIZE_OLD );

        DebugOut( "nByte %d \n", nByte );

        if ( nByte > 0 )
        {
        	/* ������ ������ - read byte */        
            stSendPkt.nDataSize = nByte;
        }
        else if ( nByte == 0 )
        {
            DebugOut( "ȭ�ϳ����� ������ %d \n", nByte );
		   /*
		    * EOT ��Ŷ ����
		    */ 			
            stSendPkt.nDataSize = 1;
            abSendData[0] = EOT;
            nEOTCheck = 1;
        }
        else
        {
            sRetVal = -1;
            break;
        }

    	/*
	     * ��Ŷ ����
	     */
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            break;
        }
		/*
	     * ���� ����
	     */      
   	    sRetVal = RecvPkt( 4000, nDevNo );
        if ( sRetVal < 0 )
        {
            printf( "\r\n ���� ������ �������[%d]\r\n", sRetVal );
            break;
        }
		/*
	     * ��Ŷ�� EOT�� ����� �� ACK������ ��찡 ����ٿ�ε� ��
	     */   
        if ( stRecvPkt.nDataSize == 1 &&
             stRecvPkt.abData[0] == ACK &&
             nEOTCheck == 1 )
        {
            printf( "\r\n ���� �ٿ�ε� !\r\n" );
            sRetVal =  SUCCESS;
            break;
        }

    }

    boolIsMainSubCommFileDownIng = FALSE;
    close( fdFile );

    return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CheckSubTermID				                           *		       *
*                                                                              *
*  DESCRIPTION:       ������ �����ܸ��� ID �����͸� �����Ѵ�.                  *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  byte *pbCheckID - �����ܸ��� ID                          *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short CheckSubTermID( byte *pbCheckID )
{

    int i = 0;
    int nID = 0;

    /*
    * �����ܸ��� ID �ڸ��� ���� 
    */
    if ( strlen( pbCheckID ) != 9 )
    {
        return -1;
    }
    /*
    * �����ܸ��� ID�� �������� ���� - ������ 15�� ������.
    */
    if ( memcmp( pbCheckID, "15", 2 ) != 0 )
    {
        return -1;
    }
    /*
    * �� �ڸ��� ����(numeric)���� ����
    */
    for( i = 0; i < 9 ; i++ )
    {
        nID = GetINTFromASC( &pbCheckID[i], 1 );
        if ( nID < 0 || nID > 9 )
        {
            return -1;
        }
    }

    return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CheckDCSCommParameterReq   	                           *		       *
*                                                                              *
*  DESCRIPTION:       ������ �����ܸ��� ID �����͸� �����Ѵ�.                  *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  bool *pboolIsDCSReady					                   *
*                     byte *pbCmdData 										   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
void CheckDCSCommParameterReq( bool *pboolIsDCSReady, byte *pbCmdData )
{

    short sRetVal = SUCCESS;
    byte abCmdData[2] = { 0, };

    if ( pbCmdData[1] == CMD_REQUEST )
    {

        sRetVal = GetLEAPPasswd();
        if ( sRetVal != SUCCESS )
        {
            abCmdData[0] = CMD_FAIL_RES;
            sRetVal = SetSharedDataforCmd( CMD_PARMS_RESET, abCmdData, 1 );

        }
        else
        {
            abCmdData[0] = CMD_SUCCESS_RES;
            sRetVal = SetSharedDataforCmd( CMD_PARMS_RESET, abCmdData, 1 );

            *pboolIsDCSReady = TRUE;

        }
    }

    if ( pbCmdData[2] == CMD_REQUEST )
    {
        sRetVal = LoadInstallInfo();
        if ( sRetVal < 0 )
        {
            abCmdData[0] = CMD_FAIL_RES;
            sRetVal = SetSharedDataforCmd( CMD_PARMS_RESET, abCmdData, 1 );
        }
        else
        {
            abCmdData[0] = CMD_SUCCESS_RES;
            sRetVal = SetSharedDataforCmd( CMD_PARMS_RESET, abCmdData, 1 );
            *pboolIsDCSReady = TRUE;
        }
    }

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       PSAMIDCompareMainWithSubTerm	                           *	
*                                                                              *
*  DESCRIPTION:       ��ɾ� ����� ���� ������ �������� PSAMID �� ��������    *
*                     PSAMID�� PSAMID����(simid.flg)���� PSAMID�� ���Ѵ�.    *
*                     Ʋ����쿡�� SAM��������� �ʱ�ȭ�Ѵ�.            	   *
*																			   *
*  INPUT PARAMETERS:  void 									                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short PSAMIDCompareMainWithSubTerm( void )
{

    FILE* fdPSAM;
    int nRetVal;
	byte i = 0;
    byte abPSAMId[65] = { 0, };
    byte abMainTermPSAMIDInSharedMemory[17] = { 0, };
    byte abSubTermPSAMIDInSharedMemory[3][17] ;
    byte abMainTermPSAMIDInFile[17] = { 0, };
    byte abSubTermPSAMIDInFile[3][17] ;

    memset( abSubTermPSAMIDInSharedMemory, 0,
            sizeof( abSubTermPSAMIDInSharedMemory ) );
    memset( abSubTermPSAMIDInFile, 0, sizeof( abSubTermPSAMIDInFile ) );

    nRetVal = access( PSAMID_FILE, F_OK );

    system ( "chmod 755 /mnt/mtd8/bus/*" );

    if ( nRetVal != 0 )
    {
        if ( ( fdPSAM = fopen( PSAMID_FILE, "wb+" ) )  == NULL )
        {
            DebugOut( "\r\n  <MAIN> Error!!! open() failed\n" );
            return -1;
        }
    }
    else
    {
        DebugOut( "\r\n  7.2.4 \r\n" );
        if ( ( fdPSAM = fopen( PSAMID_FILE, "rb+" ) ) == NULL )
        {
            DebugOut( "Error!!! open() failed\n" );
            return -1;
        }

        fread( abPSAMId, sizeof( abPSAMId ), 1, fdPSAM );
    }

    /* simid.flg���� PSAMID �б� */
    memcpy( abMainTermPSAMIDInFile, abPSAMId, 16 );
    memcpy( abSubTermPSAMIDInFile[0], &abPSAMId[16], 16 );
    memcpy( abSubTermPSAMIDInFile[1], &abPSAMId[32], 16 );
    memcpy( abSubTermPSAMIDInFile[2], &abPSAMId[48], 16 );

    /*
     * �����޸𸮿��� PSAMID �б�
     */
    memcpy( abMainTermPSAMIDInSharedMemory,   gpstSharedInfo->abMainPSAMID, 16);
    memcpy( abSubTermPSAMIDInSharedMemory[0],
            gpstSharedInfo->abSubPSAMID[0], 16);
    memcpy( abSubTermPSAMIDInSharedMemory[1],
            gpstSharedInfo->abSubPSAMID[1], 16);
    memcpy( abSubTermPSAMIDInSharedMemory[2],
            gpstSharedInfo->abSubPSAMID[2], 16);

	if ( IsAllZero( abMainTermPSAMIDInSharedMemory,
		sizeof( abMainTermPSAMIDInSharedMemory ) ) )
	{
		memset( abMainTermPSAMIDInSharedMemory, '0',
			sizeof( abMainTermPSAMIDInSharedMemory ) );
	}
	for ( i = 0; i < 3; i++ )
	{
		if ( IsAllZero( abSubTermPSAMIDInSharedMemory[i],
			sizeof( abSubTermPSAMIDInSharedMemory[i] ) ) )
		{
			memset( abSubTermPSAMIDInSharedMemory[i], '0',
				sizeof( abSubTermPSAMIDInSharedMemory[i] ) );
		}
	}

    DebugOut( "\r\n <simidLoadCompare> in MainTermPSAMIDInSharedMemory [%s]",
              abMainTermPSAMIDInSharedMemory);
    DebugOut( "\r\n <simidLoadCompare> in SubTermPSAMIDInSharedMemory 0 [%s]",
              abSubTermPSAMIDInSharedMemory[0]);
    DebugOut( "\r\n <simidLoadCompare> in SubTermPSAMIDInSharedMemory 1 [%s]",
              abSubTermPSAMIDInSharedMemory[1]);
    DebugOut( "\r\n <simidLoadCompare> in SubTermPSAMIDInSharedMemory 2 [%s]",
              abSubTermPSAMIDInSharedMemory[2]);
    DebugOut( "\r\n �ʱ���ýÿ��� ���� PSAMID�� ���Ĵ� ��� ����
              in simid.flg ���� ���� �� [%s]", abPSAMId );
    /*
     * �����޸��� ���� ���ϳ� PSAMID�� ���� �ٸ��� �� 
     */
    if ( ( memcmp( abMainTermPSAMIDInSharedMemory,
                   abMainTermPSAMIDInFile, 16 ) != 0 ) ||
         ( memcmp( abSubTermPSAMIDInSharedMemory[0],
                   abSubTermPSAMIDInFile[0], 16 ) != 0 ) ||
         ( memcmp( abSubTermPSAMIDInSharedMemory[1],
                   abSubTermPSAMIDInFile[1], 16) != 0 ) ||
         ( memcmp( abSubTermPSAMIDInSharedMemory[2],
                   abSubTermPSAMIDInFile[2], 16) != 0 ) )
    {
        DebugOut( "�����޸𸮿� simid.flg�� ���� �ٸ���\n" );
        DebugOut( "\n <simidLoadCompare> in MainTermPSAMIDInSharedMemory [%s]",
                  abMainTermPSAMIDInSharedMemory);
        DebugOut( "\n <simidLoadCompare> in SubTermPSAMIDInSharedMemory 0 [%s]",
                  abSubTermPSAMIDInSharedMemory[0]);
        DebugOut( "\n <simidLoadCompare> in SubTermPSAMIDInSharedMemory 1 [%s]",
                  abSubTermPSAMIDInSharedMemory[1]);
        DebugOut( "\n <simidLoadCompare> in SubTermPSAMIDInSharedMemory 2 [%s]",
                  abSubTermPSAMIDInSharedMemory[2]);
        DebugOut( "\n <simidLoadCompare> in �����޸𸮿� �ִ°� ������ �� [%s]",
                  abPSAMId);
	    /*
	     * �����޸��� PSAMID���� ������ ���� 
	     */
        memcpy( abPSAMId,      abMainTermPSAMIDInSharedMemory,   16 );
        memcpy( &abPSAMId[16], abSubTermPSAMIDInSharedMemory[0], 16 );
        memcpy( &abPSAMId[32], abSubTermPSAMIDInSharedMemory[1], 16 );
        memcpy( &abPSAMId[48], abSubTermPSAMIDInSharedMemory[2], 16 );
	    /*
	     * SAM������� �ʱ�ȭ �� �����޸��� PSAMID�� PSAMID����(simid.flg)�� ����
	     */		
        if ( InitSAMRegistInfoVer() == SUCCESS )
        {
            fseek( fdPSAM, 0, SEEK_SET );
            fwrite( abPSAMId, sizeof(abPSAMId), 1, fdPSAM );
        }

        fclose( fdPSAM );

        return -1;
    }

    fclose( fdPSAM );

    return 1;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendSubTermImgFile                             	       *
*                                                                              *
*  DESCRIPTION:       �������� �����̹����� bus100���Ͽ� �Ͻù��� 18byte�� ����*
*                     ������� �����Ѵ�.   								       *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo- �ܸ����ȣ 				                   *
*                     char* pchFileName - �۽Ŵ�� ���ϸ� 				       *
* 																			   *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                     ERR_MAINSUB_COMM_SUBTERM_IMG_FILE_OPEN                   *
*                      - bus100�����̹��� ���� ���� ����                 	   *
*                     ERR_MAINSUB_COMM_SUBTERM_IMG_DOWN_NAK                    *
*                      - ���������� NAK�� ����                                 *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:       	 �۽��� ������ �����ϴ� ��쿡�� ������ ������ ��Ŷ��      *
*                    ������ ��Ŷ�� �ǹ̷� EOT�� ����                           *
*                    �� ��Ŷ ���۸��� ACK�μ� ��Ŷ���ۿϷḦ Ȯ���Ѵ�.         *
*																			   *
*******************************************************************************/
short SendSubTermImgFile( int nDevNo, char* pchFileName )
{
    int fdFile;
    short sRetVal = SUCCESS;
    bool boolIsFileEnd = FALSE;   /* ������ ���� ��Ÿ���� Flag */         
	int nByte;					  /* �۽����Ͽ��� ���� Byte�� */
    word wSeqNo = 0;			  /* Sequence ��ȣ */
	int nCopyLen	= 0;          /* �Ͻù���(18byte)�� ��Ŷ�� ������ ���� */
	/* 
	 * 1012byte�� ���Ͽ��� �����͸� Read �ߴ��� ���θ� ��Ÿ���� Flag
	 * ���� - bus100���� ���������� ���� ������ ũ�Ⱑ ��Ȯ�� 1012�� ��� 
	 *        EOT�������� �Ͻù���(18byte)�� �߰��� �����͸� �������־�� �ϹǷ� 
	 */ 
	bool boolIsMaxPktSendYN	= FALSE;  
    /* �����Ͻÿ� �������߿��� ���� */	
	byte abCurrDataNVer[18] = { 0, }; 
	time_t tCurrentDtime = 0;
  
    usleep( 100000 );

   /* 
    * bus100�̹����� ��Ŷ���� ������ �ڿ� ����ð��� ���ι����� ���ٿ� ������ 
    */
   /*
    * ����ð� 14byte Copy
    */	
    GetRTCTime( &tCurrentDtime );
    TimeT2ASCDtime( tCurrentDtime, abCurrDataNVer );
    abCurrDataNVer[12] = '0';
    abCurrDataNVer[13] = '0';
   /* 
    * ���ι����� �����ϰ� ���� 4byte�� Copy
	*/
    memcpy( &abCurrDataNVer[14], MAIN_RELEASE_VER, 4 );

   /*
   	* ������ ���� ����
   	*/
    fdFile = open( pchFileName, O_RDONLY, OPENMODE );

    if ( fdFile < 0 )
    {
        printf( "\r\n ���� ���� �ȵ�\r\n" );
	 	return ERR_MAINSUB_COMM_SUBTERM_IMG_FILE_OPEN;
	}
   /*
   	* �������� Loop ���� 
   	*/
    while( 1 )
    {
    	/* ��ɾ� : ������ �߿��� �ٿ�ε� 'd'*/
   		stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_DOWN;
		/* �ܸ��� ��ȣ : �ܸ����ȣ + ASCII 0 */
        stSendPkt.bDevNo = nDevNo + '0'; 
	   /*
	   	* ������ ���� �ִ� 1012byte���� �����Ϳ����� �б�  
	   	*/    
        nByte = read( fdFile, stSendPkt.abData, DOWN_DATA_SIZE );

	   /*
	    * 1. �ִ� Read size�� ���ٸ� 
	    */ 
	   	if ( nByte == DOWN_DATA_SIZE )
        {
			/* nDataSize(�����ͻ�����) : ���Ͽ��� ���� byte  */ 					
            stSendPkt.nDataSize = nByte;     
			/* wSeqNo(������ ��ȣ) : ��Ŷ�� ������ ��ȣ  */ 
            stSendPkt.wSeqNo = wSeqNo;        
			/* 1012 Byte�� ���� Flag ����*/
			boolIsMaxPktSendYN = TRUE;			
        }
	   /*
	    * 2. Read Byte�� 0���� ũ�� 1012byte ���̸� 
	    */		
		else if ( nByte > 0 && nByte < DOWN_DATA_SIZE )
		{
			/* 1024 Byte�� ���� Flag ����*/
			boolIsMaxPktSendYN = FALSE;
			/* 
			 * 1) ������ ��Ŷ�� �Ͻù����� ���Ѱ��� �ִ� Read Size���� ū ��� 
			 *    �� ��Ŷ���� ������ ����
			 */
			if ( nByte + sizeof(abCurrDataNVer) > DOWN_DATA_SIZE )	 
			{
				/* 
				 * nCopyLen : �Ͻù��� 18byte�� �����Ʈ�� ���ٿ��� 1012byte
				 *            �� �Ǵ��� ���
				 */ 			
				nCopyLen = DOWN_DATA_SIZE - nByte;
				/* nDataSize(�����ͻ�����) : 1012byte 
				 * ���Ͽ��� ���� byte�� �Ͻù��� 17byte�� n byte�� 1012�� �ǵ���
				 * ���Ʊ� ����
				 */ 
				stSendPkt.nDataSize = DOWN_DATA_SIZE;
				/* wSeqNo(������ ��ȣ) : ��Ŷ�� ������ ��ȣ  */ 
	            stSendPkt.wSeqNo = wSeqNo;      
				 /* 1-1).�����Ϳ� �Ͻù��� Byte�� nCopyLen���� ��ŭ �߰� */ 
				memcpy( &stSendPkt.abData[nByte], abCurrDataNVer, nCopyLen );

				/*
			     * ������ ������ ��Ŷ ����
			     */
		        sRetVal = SendPkt();
		        if ( sRetVal < 0 )
		        {
		            break;
		        }
				/*
			     * ������ ������ ��Ŷ ���� ����
			     */      
			    sRetVal = RecvPkt( 4000, nDevNo );
		        if ( sRetVal < 0 )
		        {
		            DebugOut( "\r\n ���� ������ �������[%d]\r\n", sRetVal );
		            break;
		        }
				/*
			     * 1-2).�Ͻù��� Byte�� nCopyLen��ŭ 1.���� �߰��ϰ� ���� �����͸�
			     *     �����Ϳ� �߰��Ͽ� ����  
			     */ 
				/* nDataSize(�����ͻ�����) : �Ͻù��� 18byte - ����Ŷ�� �߰��� byte */ 					
	            stSendPkt.nDataSize = sizeof(abCurrDataNVer)-nCopyLen; 
				/* wSeqNo(������ ��ȣ) : ��Ŷ�� ������ ��ȣ  */ 
	            stSendPkt.wSeqNo = ++wSeqNo;        			     
			    /* �����Ϳ� �Ͻù��� �ܿ� Byte�� �־� ����  */ 
				memcpy( stSendPkt.abData, 
						&abCurrDataNVer[nCopyLen],
						sizeof(abCurrDataNVer)-nCopyLen );
				
				
			}
			/* 
			 * 2) ������ ��Ŷ�� �Ͻù����� ���Ѱ��� �ִ� Read Size���� ���� ��� 
			 *    ������ ������ ��Ŷ�� �Ͻù����� ���ٿ� �����ϰ� EOT�� �����Ѵ�. 
			 */			
			else	
			{
			    /* nDataSize(�����ͻ�����) : read byte + �Ͻù��� 18byte */ 
				stSendPkt.nDataSize = nByte + sizeof(abCurrDataNVer);
				/* wSeqNo(������ ��ȣ) : ��Ŷ�� ������ ��ȣ  */ 
	            stSendPkt.wSeqNo = wSeqNo;  
				/* 2-1).���Ͽ��� ���� �����Ϳ� �Ͻù��� 18byte ��θ� Copy  */ 
				memcpy( &stSendPkt.abData[nByte], abCurrDataNVer, sizeof(abCurrDataNVer) );
			}


		}	
		
	   /*
	   	* �о� ���� �����Ͱ� 0�� ��� ��, ������ ���ΰ�� EOT ��Ŷ ����
	   	*/  		
        else if ( nByte == 0 ) 
        {
            DebugOut( "ȭ�ϳ����� ������ %d \n", nByte );        
   			/*
			* ������ ��Ŷ�� ��Ȯ�� 1012byte�� ���� ��� �Ͻù����� �����ۻ���
			* �̸��� �Ͻù����� �� ��Ŷ�� ����� ����
			*/
			if ( boolIsMaxPktSendYN == TRUE )	
			{
				stSendPkt.nDataSize = sizeof(abCurrDataNVer);
				/* �Ͻù��� ������(18byte) Copy  */				
				memcpy( stSendPkt.abData, abCurrDataNVer, sizeof(abCurrDataNVer) );
				/* wSeqNo(������ ��ȣ) : ��Ŷ�� ������ ��ȣ  */ 		
	            stSendPkt.wSeqNo = wSeqNo++;				
				
				/*
			     * �Ͻù��� ��Ŷ ����
			     */
		        sRetVal = SendPkt();
		        if ( sRetVal < 0 )
		        {
		            break;
		        }
				/*
			     * ���� ����
			     */      
			    sRetVal = RecvPkt( 4000, nDevNo );
		        if ( sRetVal < 0 )
		        {
		            DebugOut( "\r\n ���� ������ �������[%d]\r\n", sRetVal );
		            break;
		        }
			}
		   /*
		    * EOT ��Ŷ ����
		    */         

			/* Cmd(Ŀ�ǵ�) : ���� �ܸ��Ⱑ �������� Ŀ�ǵ� */			
            stSendPkt.bCmd = bCurrCmd;
			/* bDevNo(�ܸ����ȣ) : �ܸ����ȣ + ASCII 0   */					
            stSendPkt.bDevNo = nDevNo + '0';
			/* nDataSize(�����ͻ�����) : 1  */ 			
            stSendPkt.nDataSize = 1;
			/* wSeqNo(������ ��ȣ) : ��Ŷ�� ������ ��ȣ  */ 	
           	stSendPkt.wSeqNo = wSeqNo;
			/* abData[0] : �����Ϳ� EOT�� �־� ���������� ���ϳ��� ǥ�� */ 
            stSendPkt.abData[0] = EOT;
		   /*
		   	* ���� �� ǥ�� 
		   	*/ 			
            boolIsFileEnd = TRUE;
        }
        else
        {
            sRetVal = -1;
            break;
        }

        if ( boolIsFileEnd == TRUE )
        {
            DebugOut( "\n[SendPkt-EOTto %d�� �ܸ���]", nIndex + 1 );
        }
        else
        {
            DebugOut( "\n[SendPkt-to %d�� �ܸ���]", nIndex + 1 );
        }
        DebugOut( "[�۽� SEQ] %d �� ��Ŷ", wSeqNo );

	   /*
	   	* ��Ŷ ���� 
	   	*/ 			
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            break;
        }

        if ( boolIsFileEnd == TRUE )
        {
            DebugOut( "\n[RecvPkt-EOT����-from %d�� �ܸ���]", nIndex+  1 );
        }
        else
        {
            DebugOut( "\n[RecvPkt-from %d�� �ܸ���]", nIndex + 1 );
        }
	   /*
	   	* ������Ŷ ����
	   	*/ 	
        sRetVal = RecvPkt( 3000, nDevNo );
        if ( sRetVal < 0 )
        {
            printf( "\r\n ���� ������ �������[%d]\r\n", sRetVal );
            break;
        }
	   /*
	   	* ���� ���ۿϷ� 
	   	* - ������ ���ۿϷᰡ �Ǿ����� �Ǵ��ϴ� �������δ� ���ϳ��� �����ϰ�
	   	*   ACK�� ���ŵ� ����� �Ѵ�. 
	   	*/ 
        if ( ( stRecvPkt.bCmd == ACK ) && ( boolIsFileEnd == TRUE) )
        {
            printf( "\r\n ���� ���ۿϷ�! ���� ���ۿϷ�! ���� ���ۿϷ�!\n" );
            sRetVal = SUCCESS;
            break;
        }
	   /*
	   	* ���� ������ NAK �������� ����
	   	*/ 		
        else if ( stRecvPkt.bCmd == NAK )
        {
            sRetVal = ERR_MAINSUB_COMM_SUBTERM_IMG_DOWN_NAK;
            break;
        }
	   /*
	   	* ��Ŷ Sequence ��ȣ ����
	   	*/ 	
        wSeqNo++;
    }

    close( fdFile );
    return ErrRet( sRetVal );

}
















































