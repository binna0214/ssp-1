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
*  PROGRAM ID :       term_comm_subterm.c                                      *
*                                                                              *
*  DESCRIPTION:       �����⿡�� �������� ����� ����ϴ� ���α׷����� ������  *
*                     ���� ������ ��ɾ �Ľ��Ͽ� �����⿡�� �ش� ��ɾ   *
*                     ������ �Լ��� �����Ͽ� ó���ϰ� �ȴ�.                    *
*                                                                              *
*  ENTRY POINT:       SubTermProc()                                            *
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
#include "term_comm_subterm.h"
#include "../proc/main.h"
#include "../proc/file_mgt.h"
#include "../proc/write_trans_data.h"
#include "../proc/blpl_proc.h"
#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "../proc/card_proc_util.h"
#include "../proc/main_process_busTerm.h"
#include "../proc/main_environ_init.h"
#include "../proc/check_valid_file_mgt.h"

/*******************************************************************************
*  Definition of Macro                                                         *
*******************************************************************************/
/* ������ ������ ���� �ŷ����� ũ�� */
#define SUB_TERM_TR_RECORD_SIZE     ( 202 )

/*******************************************************************************
*  Declaration of Global Variables inside this module                          *
*******************************************************************************/

static OFFLINE_KEYSET_DATA      stKeySetData;       /* KeySet Data   */
static OFFLINE_IDCENTER_DATA    stIDCenterData;     /* IDCenter Data */

static int nSubTermPollingCheckCnt;    /* Check Count for MainTerm's Polling */

/*******************************************************************************
*  Declaration of Function Prototype                                           *
*******************************************************************************/
static short SubTermProcMain( void );
static short ProcRecvPkt( short sRetVal );
static short ParseDataNProcCmd( void );
static short CreateRespData( byte bCmd );
static short CreateRespImgVer( void );
static short CreateRespSubTermID( void );
static short CreateRespVoiceVer( void );
static short CreateRespKeySet( void );
static short CreateReqPLSearch( void );
static short CreateReqBLSearch( void );
static short ProcSubTermRecvData( void );
static short ProcSubTermTransData( void );
static short ProcSubTermPolling( void );
static short SubRecvImgVer( void );
static short SubRecvSubTermID( void );
static short SubRecvPoll( void );
static short SubRecvACK( void );
static short SubRecvNAK( void );
static short SubRecvBLCheckResult( void );
static short SubRecvPLCheckResult( void );
static short SendTDFile2MainTerm( int nDevNo );
static short ImgFileRecvFromMainTerm( void );
static short VoiceFileRecvFromMainTerm( void );
static short ParameterFileRecvFromMainTerm( void );
static short CheckReqBLPLSearch( bool *pboolIsBLPLCheckReq, byte *pbBLPLCmd );
static short SetStationToSubTermial( void );
static short ImgFileRecvFromMainTermOldProtocol( void );
static short RecvFileOldProtocol( int nDevNo, char* pchFileName );
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SubTermProc                                              *
*                                                                              *
*  DESCRIPTION:       �ܸ��Ⱑ �������� ��� ������� ����� ����ϴ� ���κκ� *
*                     ���� ������ó���κ��Լ��� ȣ���ϰ� ������ ó���Ѵ�.      *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:           �������� ��Ÿ����Լ��� SubTermProcMain�� ȣ���Ѵ�.      *
*                     �������� TimeOut�� 1000ȸ �̻� �߻��� ��� �α׸�        *
*                     ����Ѵ�.                                                *
*                                                                              *
*******************************************************************************/
void SubTermProc( void )
{
    int nTimeOutCnt = 0;
    short sRetVal = SUCCESS;
    
//	PrintlnASC( "����PSAMID :", gpstSharedInfo->abSubPSAMID[gbSubTermNo-1], 16 );

    while(1)
    {    	
		/*
		 * ������ ���ó�� �����Լ� ȣ�� 
		 */
	 	sRetVal = SubTermProcMain(); 
		
        if ( sRetVal < 0 ) 
        {
            if ( (sRetVal == ERR_UART_RECV_SELECT) ||
				 (sRetVal == ERR_UART_RECV_TIMEOUT) ||
                 (sRetVal == ERR_MAINSUB_COMM_PACKET) )
            {
                DebugOut( "SubTermProcMain ��õ�" );
                nTimeOutCnt++;
            }
			/* 
			 * Ÿ�Ӿƿ� 1000ȸ �� ErrProc���� �αױ�� 
			 */
            if ( nTimeOutCnt >= MAX_TIMEOUT_COUNT )
            {
                DebugOut( "���� Timout �߻� 1000ȸ �̻�� �α׿� ���\n" );
                ErrProc( sRetVal ); 
                nTimeOutCnt = 0;
            }
        }
    }

	/*
	 * ������ ������� 
	 */	
    sRetVal = CloseMainSubComm();

    if ( sRetVal < 0 )
    {
        ErrProc(sRetVal);
    }	
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SubTermProcMain                             			   *
*                                                                              *
*  DESCRIPTION:       �������� ��źκ��� �������� ����ó���κ����� �����⿡�� *
*                     ��ŵ����͸� ����/�Ľ��Ͽ� �ش� ó���Լ��� �����Ų��.   *
*                                                                              *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: ERR_MAINSUB_COMM_SELECT                                  *
*                     ERR_UART_RECV_TIMEOUT                                    *
*                     ERR_MAINSUB_COMM_ETX_IN_PKT                              *
*                     ERR_MAINSUB_COMM_STX_IN_PKT                              *
*                     ERR_MAINSUB_COMM_CRC_IN_PKT                              *
*                                                                              *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short SubTermProcMain( void )
{
    short sRetVal = SUCCESS;
    while(1)
    {
        DebugOut( "\n=============�����ܸ��� ó�� ���� ==================" );
		/*
		 * ������κ��� �ʱ� ������ ���� ///////////////////////////////////////
		 */
        sRetVal = RecvPkt( 3000,  gbSubTermNo );
		
		/*
		* �ʱⵥ���� ���ŵ����� ����  
		*/
		sRetVal = ProcRecvPkt( sRetVal );
		if ( sRetVal != SUCCESS )
		{			
			if ( sRetVal == ERR_MAINSUB_COMM_IGNORE_IN_PKT ) 
			{
				/* 
				 * STX ETX �������� �����͸� ������Ѵ�.
				 */			
				continue;
			}
			else 
			{
				/* 
				 * TimeOut�̳� SELECT������ ��쿡�� ���� ȣ���Լ��� ����
				 * ���� : �����Լ�(SubTermProc)���� ������ ������ ����Ƚ����ŭ
				 *        ���� ��� �����α׸� ����ϱ� ���ؼ� 
				 */
				LogTerm( "[SubTermProcMain]ProcRecvPkt Error �߻� %02x\n",
						sRetVal );
				return ErrRet( sRetVal );
			}
		}

        /* 
         * ������� ACK �۽��� ���� ��Ŷ���� ///////////////////////////////////
         */
        CreateRespData( ACK );
        /* 
         * ������� ACK �۽�-�ʱⵥ���� �� ���������� �˷���////////////////////
         */
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
			LogTerm( "[SubTermProcMain]ack SendPkt Error �߻� %02x\n",
					sRetVal );
            return ErrRet( sRetVal );
        }

        /* 
         * �ʱ���ŵ����� �Ľ� �� �Ľ�Ŀ�ǵ忡 ���� ��ɾ����//////////////////
         */
        sRetVal = ParseDataNProcCmd();  
        if ( sRetVal < 0 )
        {
			LogTerm( "[SubTermProcMain]ParseDataNProcCmd Error �߻� %02x\n",
					sRetVal );
        }
		
    }
	
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcRecvPkt                             			           *
*                                                                              *
*  DESCRIPTION:       �ʱⵥ���� ���Ž� ó���� ����Ѵ�.     		           *
*                                                                              *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: ERR_MAINSUB_COMM_SELECT                                  *
*                     ERR_UART_RECV_TIMEOUT                                    *
*                     ERR_MAINSUB_COMM_ETX_IN_PKT                              *
*                     ERR_MAINSUB_COMM_STX_IN_PKT                              *
*                     ERR_MAINSUB_COMM_CRC_IN_PKT                              *
*                                                                              *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short ProcRecvPkt( short sRetVal )
{

 	switch( sRetVal )
    {
    	case SUCCESS :
	        /*
	        *  �����ΰ�� ������ ���� ��� ������ �������Ż������� üũ�ϴ� ���� �ʱ�ȭ
			*  ������ ������ ���� ��찡 8ȸ �̻� �Ǹ� Display�� 999999�� ǥ���ϰ� 
			*  nSubTermPollingCheckCnt���� 8�� �Ǿ��ְ� �ȴ�.
			*  �� ���� �����⿡ ������ ���������� ���ŵǰ� �Ǹ� �� �������� 0���� 
			*  �ʱ�ȭ�ϰ� Display�� 0���� ǥ���ؾ��Ѵ�.
	        */
	        if ( nSubTermPollingCheckCnt >= 8 )
	        {
	            DisplayASCInUpFND( "0" );
	        }
        	nSubTermPollingCheckCnt = 0;
			return SUCCESS;
			
		   /*
	        *  �Ʒ��� 5������ ��Ŷ���� ������ ERR_MAINSUB_COMM_IGNORE_IN_PKT �����ڵ带
	        *  �����ϰ� �Ǵµ� �����Լ������� �� ������ �߻��� ��쿡�� �ٽ� 
	        *  �ʱⵥ���� ������ �ϵ��� ó���ϰ� �ȴ�.
	        */			
        case ERR_MAINSUB_COMM_NOT_MINE_PKT :
        case ERR_MAINSUB_COMM_INVALID_LENGTH :
        case ERR_MAINSUB_COMM_ETX_IN_PKT :
        case ERR_MAINSUB_COMM_CRC_IN_PKT :
        case ERR_MAINSUB_COMM_STX_IN_PKT :
            DebugOut( "Error!!! Packet Wrong Data \n" );
            return ERR_MAINSUB_COMM_IGNORE_IN_PKT;
			
		   /*
	        *  �Ʒ��� 2������ ������ �ʱⵥ���� ���Ž� SELECT�Լ����� ������
	        *  TimeOut�� ���� ����̸� �� ��쿡�� �����ڵ带 �״�� �����Ѵ�.
	        *  ���� nSubTermPollingCheckCnt�� ���ؼ��� ó���� ���ش�.
	        */				
        case ERR_UART_RECV_SELECT :
        case ERR_UART_RECV_TIMEOUT :
            DebugOut( "Error!!! Select or TimeOut\n" );
		   /*
	        *  0330 2005.2.28 �������� chip write ��
	        *  nSubTermPollingCheckCnt�� ������Ű�� �ʰ� 0���� ������ش�.
	        *  �̴� ���������� chip�� write�ϴ� ��쿡�� ���������� 
	        *  ���ϴ� ��찡 �߻��ϴ��� Display�� 999999�� ǥ������ ���ڴٴ
	        *  �ǵ��̴�. �� ���������� chip�� write�� ��쿡�� �����̼���Ƚ����
	        *  ������Ű�� �ʴ´�.
	        */
	        if ( gpstSharedInfo->bVoiceApplyStatus == 2 )
            {
                nSubTermPollingCheckCnt = 0;
                usleep( 500000 );
                return ErrRet( sRetVal );
            }

            usleep( 500000 );
            nSubTermPollingCheckCnt++;
			
	        /*  
	        * �����̼���Ƚ���� 8ȸ �̻��̸� ����������� �ƴ��� ���� �����޸𸮿� 
	        * �־��ְ� 999999�� DISPLAY�� ǥ���Ѵ�.
	        */
	        if ( nSubTermPollingCheckCnt >= 8 )
            {
                /* ����������� �ƴ��� ���� �����޸𸮿� ���� */
                gpstSharedInfo->boolIsDriveNow = FALSE;

                DebugOut( "���� ���� ���� \r\n" );
                DisplayASCInUpFND( FND_ERR_MAIN_SUB_COMM_POLLING );
                nSubTermPollingCheckCnt = 8;
            }
            return ErrRet( sRetVal );

        default :
            DebugOut( "default" );
            return ErrRet( ERR_MAINSUB_COMM_PACKET );
    }
	
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ParseDataNProcCmd()                             			   *
*                                                                              *
*  DESCRIPTION:       �������� ���ŵ����͸� �Ľ��Ͽ� �����⿡�� ���� ��ɾ  *
*                     �´� ó���Լ��� ȣ���Ѵ�.								   *
*                                                                              *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                     ERR_MAINSUB_COMM_SUBTERM_CMD_D                           *
*                     ERR_MAINSUB_COMM_SUBTERM_CMD_A                           *
*                     ERR_MAINSUB_COMM_SUBTERM_CMD_M_ACK_FINAL                 *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short ParseDataNProcCmd()
{
	short sRetVal = SUCCESS;
	
	switch( stRecvPkt.bCmd )
    {
			
        /* 
         * ���α׷� �ٿ�ε�(�� ���������̿�)
         */    
        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD :
            sRetVal = ImgFileRecvFromMainTermOldProtocol();
            if ( sRetVal < 0 )
            {
                sRetVal = ERR_MAINSUB_COMM_SUBTERM_CMD_D;
            }
            return ErrRet( sRetVal );			
        /* 
         * ���α׷� �ٿ�ε�(�� ���������̿�)
         */
        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :
            sRetVal = ImgFileRecvFromMainTerm();
            if ( sRetVal < 0 )
            {
                sRetVal = ERR_MAINSUB_COMM_SUBTERM_CMD_D;
            }
            return ErrRet( sRetVal );

        /* 
         * ���� �ٿ�ε�
         */
        case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN :
            sRetVal = VoiceFileRecvFromMainTerm();
            if ( sRetVal < 0 )
            {
                sRetVal=  ERR_MAINSUB_COMM_SUBTERM_CMD_A;
            }
            return ErrRet( sRetVal );
        /* 
         * �������ʿ� ���� �ٿ�ε�
         */
        case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :
            sRetVal = ParameterFileRecvFromMainTerm();
            if ( sRetVal < 0 )
            {
                CreateNAK( gbSubTermNo );
                DebugOut( "�Ķ���������� ���������� �ٿ��������� NAK����\n" );
            }
            else
            {
                CreateACK( gbSubTermNo );
                DebugOut( "�Ķ���� ������ ���������� �ٿ��� ACK����\n" );

            }
            sRetVal = SendPkt();
            if ( sRetVal < 0 )
            {
                sRetVal=  ERR_MAINSUB_COMM_SUBTERM_CMD_M_ACK_FINAL;
            }
            ClearSharedCmdnData( CMD_NEW_CONF_IMG );

            return ErrRet( sRetVal );
        /* 
         * ���� 
         */
        case MAIN_SUB_COMM_POLLING :
            ProcSubTermRecvData();
            ProcSubTermPolling();
            return ErrRet( sRetVal );
        /* 
         * �ŷ��������� �ۼ���
         */
        case MAIN_SUB_COMM_GET_TRANS_FILE :
            SendTDFile2MainTerm( gbSubTermNo );
            return ErrRet( sRetVal );
        /* 
         * Ű�� ���/���������������۽�/�����ܸ���ID�۽�/�����ܸ������α׷� �����۽�
         */
        case MAIN_SUB_COMM_REGIST_KEYSET :
        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER :
        case MAIN_SUB_COMM_GET_SUB_TERM_ID :
        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :
			DebugOut("ProcSubTermRecvData\n");
            ProcSubTermRecvData();
			DebugOut("CreateRespData\n");
            CreateRespData( stRecvPkt.bCmd );

            SendPkt();
            if ( stRecvPkt.nDataSize == 30 &&
                 stRecvPkt.bCmd == MAIN_SUB_COMM_SUB_TERM_IMG_VER )
            {
            	boolIsOldProtocol = FALSE;				
                DebugOut( "�� ���������̹Ƿ� �̸� ��\n" );
                return ErrRet( sRetVal );
            }
			
            RecvPkt( 3000, gbSubTermNo );
            ProcSubTermRecvData();
            return ErrRet( sRetVal );
        /* 
         * �����ܸ��� ID ����
         */
        case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID :
            ProcSubTermRecvData();
            return ErrRet( sRetVal );

        default :
            return ErrRet( sRetVal );
    }
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateRespData()                             			   *
*                                                                              *
*  DESCRIPTION:       �������� ��û�����Ϳ� ���� ����(Response)�����͸�        *
*                     �����Ѵ�.                                                *
*                                                                              *
*  INPUT PARAMETERS:  byte bCmd - �����⿡�� ��û���� ���� ���                *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                     ȣ��� �Լ��� �����ڵ� ����.                             *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:           �����⿡�� ��û���� �����ɿ� ���� ���䵥���͸� �����ϴ�*
*                     �Լ����� ȣ���Ѵ�                                        *
*                                                                              *
*******************************************************************************/
short CreateRespData( byte bCmd )
{
    short sRetVal = SUCCESS;

    switch( bCmd )
    {
        /* 
         * ACK�� ������ ���䵥���� ����
         */
        case ACK :
            sRetVal = CreateACK( gbSubTermNo );
            return ErrRet( sRetVal );
        /* 
         * ���α׷� ����üũ��û�� ���� ���䵥���� ����
         */
        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :   
            printf( "[2.CreateRespImgVer]\n" );
            sRetVal = CreateRespImgVer();
            return ErrRet( sRetVal );
        /* 
         * ������ ID��û�� ���� ���䵥���� ����
         */
        case MAIN_SUB_COMM_GET_SUB_TERM_ID :  //SubId ���� ���� 0330 2005.2.28
            DebugOut( "[2.CreateRespSubTermID]" );
            sRetVal = CreateRespSubTermID();
            return ErrRet( sRetVal );
        /* 
         * ������ ����������û�� ���� ���䵥���� ����
         */
        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER : //Voice Version 0330 2005.2.28
            DebugOut( "[2.CreateRespVoiceVer]" );
            sRetVal = CreateRespVoiceVer();
            return ErrRet( sRetVal );
        /* 
         * ������ Ű�µ�Ͽ� ���� ���䵥���� ����
         */
        case MAIN_SUB_COMM_REGIST_KEYSET :      //Keyset Result
            DebugOut( "[2.CreateRespKeySet]" );
            sRetVal = CreateRespKeySet();
            return ErrRet( sRetVal );

        default :
            return ErrRet( sRetVal );
    }
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateRespImgVer()                           			   *
*                                                                              *
*  DESCRIPTION:       ��/�� �������ݷ� ���α׷� ������ �۽���Ŷ ����ü�� �ִ´�*
*                                                                              *
*  INPUT PARAMETERS:  void											           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:           stSendPkt	- �۽���Ŷ����ü                               *
*                                                                              *
*******************************************************************************/
short CreateRespImgVer( void )
{
    short sRetVal = SUCCESS;
    char achNewSendBuffer[40] = { 0, }; /* �� �������� ���������� SendBuffer */
    char achOldSendBuffer[30] = { 0, }; /* �� �������� ���������� SendBuffer */
    
   /*
	* �� �������ݷ� ���α׷������� �����  
	*/
    if ( boolIsRespVerNeedByOldProtocol == TRUE )
    {
	    printf( "�� �������ݷ� ���α׷� version �����\n" );
	    /*
	     *  program version copy
	     */
	    memcpy( achOldSendBuffer, gpstSharedInfo->abSubVer[gbSubTermNo-1], 4 );

	    /*
	     *  new�� ������ ���ٿ� ������.
	     *  ���� : 0401������ ����, �� 0402,0403�� �� �����������̶�� �� �� ����.
	     *         0401�� ��������ſ� ������ ���� ���� �� �������ݷ� �����Ͽ�
	     *         0402,0403�� ���� ������ ��� ������������� ����Ʈ���� 
	     *         ��,�����θ� �Ǵ��ϱ⿡ �����Ƿ� new�� ���ٿ� ������ 04xx���
	     *         �����⿡�� new�� �������� ���Ͽ� ���������ݷ� �����Ѵ�.
	     */    
	    memcpy( &achOldSendBuffer[4], "new", 3 );
  

		/*
		* �۽� ��Ŷ����ü�� �� ���� 
		*/
	    stSendPkt.bCmd = 'V';
	    stSendPkt.bDevNo = gbSubTermNo + '0';
	    stSendPkt.nDataSize = 17;
	    memcpy( stSendPkt.abData, achOldSendBuffer, 17 ); /* ������Buffer Copy */

	    /*
	     *  ���������� ����ϵ��� Flag ����
	     */	
		boolIsOldProtocol = TRUE;

    }
    else
   /*
	* �� �������ݷ� ���α׷������� �����  
	*/
    {
	    bCurrCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;
	    /*
	     *  program version SendBuffer�� copy
	     */		
	    memcpy( achNewSendBuffer, &gpstSharedInfo->abSubVer[gbSubTermNo-1], 4 );
	    DebugOutlnASC( "gpstSharedInfo->abSubVer[gbSubTermNo-1]=>",
	                   gpstSharedInfo->abSubVer[gbSubTermNo-1], 4 );
	    /*
	     *  CSAM ID, PSAM ID SendBuffer�� copy
	     */
	    memcpy( &achNewSendBuffer[4], &gpstSharedInfo->abSubCSAMID[gbSubTermNo-1], 8 );
	    memcpy( &achNewSendBuffer[12],
	            &gpstSharedInfo->abSubPSAMID[gbSubTermNo-1], 16 );
		
	    DebugOutlnASC( "gpstSharedInfo->abSubPSAMID=>",
	                   gpstSharedInfo->abSubPSAMID[gbSubTermNo-1], 16 );
	    /*
	     *  PSAM Port No, ISAMID SendBuffer�� copy
	     */		
	    memcpy( &achNewSendBuffer[28],
	            &gpstSharedInfo->abSubPSAMPort[gbSubTermNo-1], 1 );
	    memcpy( &achNewSendBuffer[29], &gpstSharedInfo->abSubISAMID[gbSubTermNo-1], 7 );

	    /*
	     *  TPSAM ���θ� SendBuffer�� copy 
	     */	
	    memcpy( &achNewSendBuffer[36], &gpstSharedInfo->abSubPSAMVer[gbSubTermNo-1], 1 );

	    DebugOut( "\n TPSAM ���� => ������ %d�� %d �̴� \n", gbSubTermNo, 
				   gpstSharedInfo->abSubPSAMVer[gbSubTermNo-1] );
		/*
		* �۽� ��Ŷ����ü�� �� ���� 
		*/
	    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_IMG_VER;   /* ��ɾ� 'V' */
	    stSendPkt.bDevNo = gbSubTermNo + '0';			   /* �ܸ��� ��ȣ */ 	
	    stSendPkt.wSeqNo = 0;							   /* Sequence No */
	    stSendPkt.nDataSize = 37;                          /* Data Size */
	    /*
	     *  SendBuffer�� �۽���Ŷ����ü���� �����Ϳ� copy 
	     */	
	    memcpy( stSendPkt.abData, achNewSendBuffer, 37 );
	    /*
	     *  ���������� ������� �ʵ��� Flag ����
	     */	
		boolIsOldProtocol = FALSE;

   	}

	return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateRespSubTermID()                       			   *
*                                                                              *
*  DESCRIPTION:       ������ ID�� �۽���Ŷ ����ü�� �ִ´�.                    *
*                                                                              *
*  INPUT PARAMETERS:  void											           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:          stSendPkt - �۽���Ŷ����ü		                           *
*                                                                              *
*******************************************************************************/
short CreateRespSubTermID( void )
{
    short sRetVal = SUCCESS;
    char achSendBuff[30] = { 0, }; /* SendBuffer */

	/*
	* ������ ID�� SendBuffer�� copy 
	*/
    memcpy( achSendBuff, gpstSharedInfo->abSubTermID[gbSubTermNo-1], 9 );
	PrintlnASC( "[CreateRespSubTermID] �����ܸ���ID : ",
		gpstSharedInfo->abSubTermID[gbSubTermNo-1], 9);
	/*
	* �۽� ��Ŷ����ü�� �� ���� 
	*/
    stSendPkt.bCmd = MAIN_SUB_COMM_GET_SUB_TERM_ID; /* ��ɾ� 'G' */
    stSendPkt.bDevNo = gbSubTermNo + '0';           /* �ܸ��� ��ȣ */
    stSendPkt.wSeqNo = 0;							/* Sequence No */
    stSendPkt.nDataSize = 9;						/* Data Size */
    /*
     *  SendBuffer�� �۽���Ŷ����ü���� �����Ϳ� copy 
     */	
    memcpy( stSendPkt.abData, achSendBuff, 9 );

    return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateRespVoiceVer()                       			   *
*                                                                              *
*  DESCRIPTION:       ���� ������ �۽���Ŷ ����ü�� �ִ´�.                    *
*                                                                              *
*  INPUT PARAMETERS:  void											           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:          stSendPkt - �۽���Ŷ����ü		                           *
*                                                                              *
*******************************************************************************/
short CreateRespVoiceVer( void )
{
    short sRetVal = SUCCESS;
    char achSendBuff[30] = { 0, }; /* SendBuffer */

	/*
	* ������ ���������� SendBuffer�� copy 
	*/
    memcpy( achSendBuff, gpstSharedInfo->abSubVoiceVer[gbSubTermNo-1], 4 );
	/*
	* �۽� ��Ŷ����ü�� �� ���� 
	*/
    stSendPkt.bCmd = MAIN_SUB_COMM_SUB_TERM_VOICE_VER; /* ��ɾ� 'H' */
    stSendPkt.bDevNo = gbSubTermNo + '0';			   /* �ܸ��� ��ȣ */
    stSendPkt.wSeqNo = 0;							   /* Sequence No */
    stSendPkt.nDataSize = 4;						   /* Data Size */
    /*
     *  SendBuffer�� �۽���Ŷ����ü���� �����Ϳ� copy 
     */		
    memcpy(stSendPkt.abData, achSendBuff, 4);

    return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateRespKeySet()                         			   *
*                                                                              *
*  DESCRIPTION:       ������SAM�� PSAM�� Ű��(Keyset)�� �����(IdCenter)�� ���*
*                     �� ����� �۽���Ŷ ����ü�� �ִ´�    				   *				       *
*                                                                              *
*  INPUT PARAMETERS:  void											           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:          stSendPkt - �۽���Ŷ����ü		                           *
*                                                                              *
*******************************************************************************/
short CreateRespKeySet( void )
{
    short sRetVal = SUCCESS;
    byte achSendBuff[30] = { 0, }; /* SendBuffer */

 	/* Keyset ��� ����� SendBuffer�� Copy */
    memcpy( achSendBuff, &stSubTermPSAMAddResult.aboolIsKeySetAdded[gbSubTermNo-1], 1 );
	/* IdCenter ��� ����� SendBuffer�� Copy */
    memcpy( &achSendBuff[1],
            &stSubTermPSAMAddResult.aboolIsIDCenterAdded[gbSubTermNo-1], 1 );

    stSendPkt.bCmd = MAIN_SUB_COMM_REGIST_KEYSET; /* ��ɾ� 'K' */
    stSendPkt.bDevNo = gbSubTermNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 2;
    memcpy( stSendPkt.abData, achSendBuff, 2 );

    return ErrRet( sRetVal );
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateReqBLSearch()                         			   *
*                                                                              *
*  DESCRIPTION:       �����⿡ ī�� �±׽� BL���θ� ��û�ϴ� �����͸�          *
*                     �۽���Ŷ ����ü�� �ִ´� 		        				   *				       *
*                                                                              *
*  INPUT PARAMETERS:  void											           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:          stSendPkt - �۽���Ŷ����ü		                           *
*                                                                              *
*******************************************************************************/
short CreateReqBLSearch( void )
{
    byte bCmd;
    byte pcCmdData[40]= {0,};
    word wCmdDataSize;
    short sRetVal = SUCCESS;
//    byte abPrefix[6]={0,};
//    dword dwCardNum = 0;

    sRetVal = GetSharedCmd( &bCmd, pcCmdData, &wCmdDataSize );
    stSendPkt.bCmd = MAIN_SUB_COMM_REQ_BL_SEARCH;
    stSendPkt.bDevNo = gbSubTermNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 30;

    memcpy( stSendPkt.abData, &pcCmdData[1], 30 );
    

#ifdef TEST_BLPL_CHECK
    PrintlnASC( " Req BL Cardo :",  &pcCmdData[1], 20 );
    PrintlnASC( " Req BL CardNum :",  &pcCmdData[21], 6 );
    printf( " Req BL CardNum : [%lu]..\n",  &pcCmdData[27], 4 );
#endif

    return ErrRet( sRetVal );
}

short CreateReqPLSearch( void )
{
    byte bCmd;
    byte pcCmdData[40] = { 0, };
    word wCmdDataSize;
    short sRetVal = SUCCESS;
//    dword AliasNo = 0;

    sRetVal = GetSharedCmd( &bCmd, pcCmdData, &wCmdDataSize );
    stSendPkt.bCmd = MAIN_SUB_COMM_REQ_PL_SEARCH;
    stSendPkt.bDevNo = gbSubTermNo + '0';
    stSendPkt.wSeqNo = 0;
    stSendPkt.nDataSize = 24;

    memcpy( stSendPkt.abData, &pcCmdData[1] , 24 );
    
#ifdef TEST_BLPL_CHECK
    PrintlnASC( " Req PL Cardo :",  &pcCmdData[1], 20 );
    memcpy( ( byte*)&AliasNo,&pcCmdData[21], 4 );
    printf( "[6-1]Req PL Alias No : [%lu]\n", AliasNo );
#endif

    return ErrRet( sRetVal );

}

short ProcSubTermRecvData( void )
{
    short sRetVal = SUCCESS;

    switch( stRecvPkt.bCmd  )
    {
        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :   // Program Version Check
            DebugOut( "\r\n[4.SubRecvImgVer]  V \r\n" );
            sRetVal = SubRecvImgVer();
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_GET_SUB_TERM_ID :    // Request SubTerminal ID
            DebugOut( "\r\n[ProcSubTermRecvData]  G \r\n" );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REGIST_KEYSET :      // KeySet Result
            DebugOut( "\r\n[ProcSubTermRecvData]  K \r\n" );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID :  // Assign Sub Term ID
            DebugOut( "\r\n I \r\n" );
            sRetVal = SubRecvSubTermID();
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REQ_BL_SEARCH :      // req BL search
            DebugOut( "[9.BL��û��� ó��]" );
            sRetVal = SubRecvBLCheckResult();
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REQ_PL_SEARCH :      // req PL search
            DebugOut( "[9.PL��û��� ó��]" );
            sRetVal = SubRecvPLCheckResult();
            return ErrRet( sRetVal );

        case ACK :
            DebugOut( "[ACKó��]" );
            sRetVal = SubRecvACK();
            return ErrRet( sRetVal );

        case NAK :
            DebugOut( "[NAKó��]" );
            sRetVal = SubRecvNAK();
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_POLLING :            // Polling
            DebugOut( "\n[4.SubRecvPoll]" );
            sRetVal = SubRecvPoll();
            return ErrRet( sRetVal );

        default :
            sRetVal = ERR_MAINSUB_COMM_SUB_RECV_DATA_PARSE;
            return ErrRet( sRetVal );
    }
}

short ProcSubTermTransData( void )
{
    short sRetVal = SUCCESS;
    int fdTR;
    int nByte;
    byte abTmpBuff[SUB_TERM_TR_RECORD_SIZE + 2];

    struct stat stFileStatus;

    DebugOut( "[6.ProcSubTermTransData]�ŷ��������ϻ�������" );

    // �������� �ƴ� ���
    if ( gpstSharedInfo->boolIsDriveNow == FALSE )
    {

#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
        printf( "[������ ���� ���۴��]\n" );
#endif
        DebugOut( "[7.�ܼ�����]�ܼ�����" );
        CreateACK( gbSubTermNo );
        bCurrCmd = ACK;
        sRetVal = SendPkt();

        if ( sRetVal < 0 )
        {
        	printf( "[ProcSubTermTransData] Ack send error %02x",
					sRetVal );
            return ErrRet( sRetVal );
        }

        sRetVal = RecvPkt( 3000, gbSubTermNo );
        if ( sRetVal < 0 )
        {
            printf( "[ProcSubTermTransData] recv error %02x",
					sRetVal );
            return ErrRet( sRetVal );
        }
        else
        {
            ProcSubTermRecvData();
        }

        return ErrRet( sRetVal );
    }

    SemaAlloc( SEMA_KEY_TRANS ); //0327

    fdTR = open( SUB_TERM_TRANS_FILE, O_RDWR, OPENMODE);

    if (( fstat( fdTR, &stFileStatus ) != 0 ) ||
        ( stFileStatus.st_size == 0 ) )
    {
#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
        printf( "fstat���аų� �ŷ����� Size 0" );//cmd 'p'�� ack����" );
#endif
        CreateACK( gbSubTermNo );
        bCurrCmd = ACK;
        sRetVal = SendPkt();

        if ( sRetVal < 0 )
        {
            close( fdTR );
            SemaFree( SEMA_KEY_TRANS );         //���⼭ return�� ��� ���� P
            return ErrRet( sRetVal );
        }

        sRetVal = RecvPkt( 3000, gbSubTermNo );
        if ( sRetVal < 0 )
        {
            close( fdTR );
            SemaFree( SEMA_KEY_TRANS );         ///���⼭ return�� ��� ���� P
            return ErrRet( sRetVal );
        }
        else
        {
            ProcSubTermRecvData();
            close( fdTR );
            SemaFree( SEMA_KEY_TRANS );
            return ErrRet( sRetVal );
        }

    }

#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
    printf( "fstat�����ϰ� �ŷ����� Size 1�̻�" );
#endif

    stSendPkt.bCmd = MAIN_SUB_COMM_GET_TRANS_CONT;
    stSendPkt.bDevNo = gbSubTermNo + '0';
    stSendPkt.wSeqNo = 0;
    bCurrCmd = MAIN_SUB_COMM_GET_TRANS_CONT;
    lseek( fdTR, 0L, SEEK_SET );

    while (1)
    {
		
        nByte = read( fdTR, abTmpBuff, SUB_TERM_TR_RECORD_SIZE + 1 );

        if ( nByte <= 0 )
        {
            close( fdTR );
            fdTR = open( SUB_TERM_TRANS_FILE,
                         O_RDWR | O_CREAT |O_TRUNC,
                         OPENMODE );
            close( fdTR );
            SemaFree( SEMA_KEY_TRANS );         //���⼭ return�� ��� ���� P

            return ErrRet( sRetVal );
        }

        if ( abTmpBuff[SUB_TERM_TR_RECORD_SIZE] == '1' )
        {
            // �� ���� Record Skip
            continue;
        }

        memcpy( ( byte *)stSendPkt.abData, abTmpBuff, SUB_TERM_TR_RECORD_SIZE );
        stSendPkt.nDataSize = nByte - 1;

#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
        printf( "..202byte ���۽���/n" );
#endif
        sRetVal = SendPkt();

        //0323����
        if ( sRetVal < 0 )
        {
            close( fdTR );
            SemaFree( SEMA_KEY_TRANS );         //���⼭ return�� ��� ���� P
            return ErrRet( sRetVal );
        }

        DebugOut( "[7.RecvPkt]" );
        sRetVal = RecvPkt( 5000, gbSubTermNo );

        if ( (sRetVal < 0 ) || ( stRecvPkt.bCmd == NAK ) )
        {
            tcflush( fdTR, TCIOFLUSH );
#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
            printf( "[ProcSubTermTransData] 202byte �ŷ��������� �������� ������� ���� close -����\n" );
#endif
            LogTerm( "[ProcSubTermTransData] 202byte �ŷ��������� �������� ������� ���� close -����\n" );            
	    close( fdTR );
            SemaFree( SEMA_KEY_TRANS );         //���⼭ return�� ��� ���� P
            return ErrRet( sRetVal );
        }
        else
        {
#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
            printf( "..[ProcSubTermTransData]�����⿡�� ACK�� �ͼ�
                    �ŷ��������� ����FLAG ���Ž���" );
#endif

            //�����ܸ���� ���� �Ϸ� Flag
            abTmpBuff[SUB_TERM_TR_RECORD_SIZE] = '1';
            lseek( fdTR, -( SUB_TERM_TR_RECORD_SIZE + 1 ), SEEK_CUR );
            write( fdTR, abTmpBuff, SUB_TERM_TR_RECORD_SIZE + 1 );

            nByte = read( fdTR, abTmpBuff, SUB_TERM_TR_RECORD_SIZE + 1 );

            if ( nByte <= 0 )
            {

#ifdef TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
                printf( "/t[Err!!]�����⿡�� ACK�� �ͼ� �ŷ��������� ����FLAG " );
                printf( " ���ſϷ� �� read�� ����Ʈ �̵��� �����߻��Ͽ�
                            ���� close- ����" );
#endif
                LogTerm( "[Err!!]�����⿡�� ACK->�ŷ����� ����FLAG ���ſϷ�->read�� ����Ʈ �̵��� ����->���� close- ����" );  
                close( fdTR );
                fdTR = open( SUB_TERM_TRANS_FILE,
                             O_RDWR | O_CREAT |O_TRUNC, OPENMODE );
                close( fdTR );
                SemaFree( SEMA_KEY_TRANS );     //���⼭ return�� ��� ���� P
                return ErrRet( sRetVal );
            }

            close( fdTR );
            break;
        }

    }

    SemaFree( SEMA_KEY_TRANS );     //��� ���� ��������
    bCurrCmd = MAIN_SUB_COMM_POLLING;

    return ErrRet( sRetVal );
}



short ProcSubTermPolling( void )
{
    short sRetVal = SUCCESS;
    bool boolIsBLPLCheckReq = FALSE;
    byte bBLorPLCmd = 0;
    byte pcCmdData[40] = { 0, };

    CheckReqBLPLSearch( &boolIsBLPLCheckReq, &bBLorPLCmd );

#ifdef TEST_BLPL_CHECK
    printf( "\n[5.BLPL��ûȮ��] %s\n",
            (( boolIsBLPLCheckReq == TRUE )? "����." : "����." ) );
#endif

    if ( boolIsBLPLCheckReq == TRUE )
    {
        if ( bBLorPLCmd == MAIN_SUB_COMM_REQ_BL_SEARCH )
        {
#ifdef TEST_BLPL_CHECK
            printf( "\n[6.CreateReqBLSearch]" );
#endif
            CreateReqBLSearch();
            bCurrCmd = MAIN_SUB_COMM_REQ_BL_SEARCH;
        }
        else if ( bBLorPLCmd == MAIN_SUB_COMM_REQ_PL_SEARCH )
        {
#ifdef TEST_BLPL_CHECK
            printf( "\n[6.CreateReqPLSearch]\n" );
#endif
            CreateReqPLSearch();
            bCurrCmd = MAIN_SUB_COMM_REQ_PL_SEARCH;
        }
        else
        {
            //����ó��
        }
		
#ifdef TEST_BLPL_CHECK
        printf( "\n[7.blpl SendPkt]\n" );
#endif

        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            memcpy( pcCmdData, "8", 1);
            SetSharedDataforCmd( MAIN_SUB_COMM_REQ_PL_SEARCH, pcCmdData, 1 );
			printf( "blpl check sendpkt error �߻� %02x\n",
					sRetVal );
            return ErrRet( sRetVal );
        }

#ifdef TEST_BLPL_CHECK
        printf( "[8.blpl RecvPkt]\n" );
#endif

        sRetVal = RecvPkt( 6000,  gbSubTermNo );
        if ( sRetVal >= 0 )
        {
            DebugOut( "[8.ProcSubTermRecvData]" );
            ProcSubTermRecvData();
        }
        else
        {
            memcpy( pcCmdData, "8", 1);
            SetSharedDataforCmd( MAIN_SUB_COMM_REQ_PL_SEARCH, pcCmdData, 1 );
            printf( "blpl check RecvPkt error �߻� %02x\n",
					sRetVal );
            DebugOut( " Error!! %02x", sRetVal );
        }
    }
    else
    {
#ifndef TEST_NOT_SEND_SUBTERM_TRN_ON_POLLING
        ProcSubTermTransData();
#endif
    }

    return ErrRet( sRetVal );
}


short SubRecvImgVer( void )
{
    short sRetVal = SUCCESS;
    byte abMainTermVer[5] = { 0, };
    byte abMainVer[4] = { 0, };

    memcpy( abMainVer, MAIN_RELEASE_VER, 4 ); // ������ ���� Copy

    if ( boolIsRespVerNeedByOldProtocol == TRUE )  //��  PROTOCOL�� V CMD ���� ��
    {
        // ISAMID���� ���� ����.
        //BCD2ASC(  stRecvPkt.abData, gpstSharedInfo->abMainISAMID, 4 );
        //���� sniper sam id
        BCD2ASC(  &stRecvPkt.abData[5], gpstSharedInfo->abMainPSAMID, 8 );
        DebugOutlnASC( "gpstSharedInfo->abMainPSAMID=>",
                        gpstSharedInfo->abMainPSAMID, 16 );

        //�ܸ��� ���̵�
        memcpy(  gpstSharedInfo->abMainTermID, &stRecvPkt.abData[13], 9 );
        DebugOutlnASC( "gpstSharedInfo->abMainTermID=>",
                        gpstSharedInfo->abMainTermID, 9 );

        memcpy( abMainTermVer, &stRecvPkt.abData[22], 4 );
#ifdef TEST_DOWN_AND_ROLLBACK
        PrintlnASC( "abMainTermVer=>", &stRecvPkt.abData[22], 4 );
#endif    
    }
    else    // �� protocol�� ���� ��
    {
        //���� sniper sam id
        memcpy(  gpstSharedInfo->abMainPSAMID, stRecvPkt.abData, 16 );
        //�ܸ��� ���̵�
        memcpy(  gpstSharedInfo->abMainTermID, &stRecvPkt.abData[16], 9 );
    }

    return ErrRet( sRetVal );
}

short SubRecvSubTermID( void )
{
    short sRetVal = SUCCESS;
    int fdSubID;
    byte abWriteTermID[10] = { 0, };
    byte abRecvTermID[10] = { 0, };
    byte abSubIDBuff[28] = { 0, };

    memcpy( gpstSharedInfo->abSubTermID[gbSubTermNo-1], stRecvPkt.abData, 9 );

    if ( ( fdSubID = open( SUBTERM_ID_FILENAME, O_WRONLY | O_CREAT | O_TRUNC , OPENMODE )
         ) < 0 )
    {
        DebugOut( "Error!!! open() failed\n" );
        return -1;
    }

    abSubIDBuff[0] = '1';
    memcpy( &abSubIDBuff[gbSubTermNo*9-8],
            gpstSharedInfo->abSubTermID[gbSubTermNo-1], 9 );

    write( fdSubID, abSubIDBuff, 28 );
    close( fdSubID );

    memcpy( abWriteTermID, gpstSharedInfo->abSubTermID[gbSubTermNo-1], 9 );

    //0323����
    if ( CheckWriteEEPROM( abWriteTermID, abRecvTermID ) == 0 )
    {
        DisplayDWORDInUpFND( GetDWORDFromASC( &abRecvTermID[3], 6 ) );
		Buzzer( 2, 1000000 );			// 1�� �������� 2ȸ ���� ���
        sleep(2);
        DisplayDWORDInUpFND( 0 );
    }

    return ErrRet( sRetVal );
}

short SubRecvPoll( void )
{
    short sRetVal = SUCCESS;

    sRetVal = SetStationToSubTermial();

    if ( sRetVal < 0 )
    {
    	printf( "[SubRecvPoll]SetStationToSubTermial Error �߻� %02x",
				sRetVal );
        sRetVal = ERR_MAINSUB_COMM_SUBSTATION_SET;
    }

    return ErrRet( sRetVal );
}


short SubRecvACK( void )
{
    short sRetVal = SUCCESS;

    switch( bCurrCmd )
    {
        case MAIN_SUB_COMM_GET_TRANS_CONT :   // Polling �������� BLSearch ����
            DebugOut( "[8.�ŷ�����ó���Ϸ�]" );
            return ErrRet( sRetVal );

        case ACK :   // Polling �������� PLSearch ����
            DebugOut( "[8.BLPL��û/�ŷ��������� �ܼ����� �Ϸ�]" );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :
            DebugOut( "\r\n[ó���Ϸ�]  Command V ����ó��\r\n" );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_GET_SUB_TERM_ID :
            DebugOut( "\r\n[ó���Ϸ�]  Command G ����ó��\r\n" );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER :
            DebugOut( "\r\n[ó���Ϸ�]  Command H ����ó��\r\n" );
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REGIST_KEYSET :
            DebugOut( "\r\n[ó���Ϸ�]  Command K ����ó��\r\n" );
            return ErrRet( sRetVal );
    }

    return ErrRet( sRetVal );

}


short SubRecvNAK( void )
{
    short sRetVal = SUCCESS;

    switch( bCurrCmd )
    {
        case MAIN_SUB_COMM_REQ_BL_SEARCH :   // Polling �������� BLSearch ����
            DebugOut( "[8.BL��û��� NAK]" );
            sRetVal = SubRecvBLCheckResult();
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_REQ_PL_SEARCH :   // Polling �������� PLSearch ����
            DebugOut( "[9.PL��û��� NAK]" );
            sRetVal = SubRecvPLCheckResult();
            return ErrRet( sRetVal );

        case MAIN_SUB_COMM_GET_TRANS_CONT :   // Polling �������� TransFile ����
            DebugOut( "\r\n �ŷ����� ������ �����Ⱑ ������ ������ �Ľ̿���\r\n" );
            return ErrRet( sRetVal );

        case ACK :   // Polling �������� TransFile ����
            DebugOut( "\r\n�ŷ����� ���� ��� ACK���´µ� NAK�� ��
                      ReckACK���� ���� \r\n" );
            return ErrRet( sRetVal );
    }

    return ErrRet( sRetVal );
}

short SubRecvBLCheckResult( void )
{
    short sRetVal = SUCCESS;
    byte pcCmdData[40] = { 0, };
    TRANS_INFO stCardErrInfo;
	short PLCheckStatus =0;

   /*
	* BL�� ����� 2byte�� ����-�����⿡�� ������ ����� �����ڵ带 �����ϱ� ����
	*/
    if ( stRecvPkt.bCmd == MAIN_SUB_COMM_REQ_BL_SEARCH )
    {
        if ( stRecvPkt.nDataSize > 3 )
        {
            memset( (byte *)&stCardErrInfo, 0, sizeof( TRANS_INFO ) );
	     	memcpy( &stCardErrInfo, &stRecvPkt.abData[3], sizeof( TRANS_INFO ) ); 
            AddCardErrLog( SUCCESS, &stCardErrInfo );
        }

		/*
		* üũ �������� ����
		*/
		memcpy( &PLCheckStatus, stRecvPkt.abData, 2);
		
		if ( PLCheckStatus != SUCCESS )
		{
			// �Ϸ� + ���  pcCmdData = |8|��������(2byte)|result(1byte)|
			memcpy( pcCmdData, "8", 1);	        
	        memcpy( &pcCmdData[1],&PLCheckStatus, 2 );
			memcpy( &pcCmdData[3],&stRecvPkt.abData[2], 1 );
	        SetSharedDataforCmd( MAIN_SUB_COMM_REQ_BL_SEARCH, pcCmdData, 4 );			
		}
		else
		{
    		// �Ϸ� + ���  pcCmdData = |4|��������(2byte)|result(1byte)|
	        memcpy( pcCmdData, "4", 1 );
	        memcpy( &pcCmdData[1],&PLCheckStatus, 2 );
			memcpy( &pcCmdData[3],&stRecvPkt.abData[2], 1 );
	        SetSharedDataforCmd( MAIN_SUB_COMM_REQ_BL_SEARCH, pcCmdData, 4 );
		}
    }
    else if ( stRecvPkt.bCmd == NAK )
    {
		memcpy( pcCmdData, "8", 1);
        pcCmdData[1] = NAK;
		pcCmdData[2] = NAK;			
        SetSharedDataforCmd( MAIN_SUB_COMM_REQ_BL_SEARCH, pcCmdData, 4 );
    }

    return ErrRet( sRetVal );
}


short SubRecvPLCheckResult( void )
{
    byte pcCmdData[40] = { 0, };
    TRANS_INFO stCardErrInfo;
	short CheckStatus = 0;
	
    if ( stRecvPkt.bCmd == MAIN_SUB_COMM_REQ_PL_SEARCH )
    {
        if ( stRecvPkt.nDataSize > 3 )
        {
            memset( (byte *)&stCardErrInfo, 0, sizeof( TRANS_INFO ) );
	     	memcpy( &stCardErrInfo, &stRecvPkt.abData[3], sizeof( TRANS_INFO ) ); 
            AddCardErrLog( SUCCESS, &stCardErrInfo );
        }
		/*
		* üũ �������� ����
		*/
		memcpy( &CheckStatus, stRecvPkt.abData, 2);
		
		if ( CheckStatus != SUCCESS )
		{
			// �Ϸ� + ���  pcCmdData = |8|��������(2byte)|result(1byte)|
			memcpy( pcCmdData, "8", 1);	        
	        memcpy( &pcCmdData[1],&CheckStatus, 2 );
			memcpy( &pcCmdData[3],&stRecvPkt.abData[2], 1 );
	        SetSharedDataforCmd( MAIN_SUB_COMM_REQ_PL_SEARCH, pcCmdData, 4 );
			
		}
		else
		{
    		// �Ϸ� + ���  pcCmdData = |4|��������(2byte)|result(1byte)|
	        memcpy( pcCmdData, "4", 1 );
	        memcpy( &pcCmdData[1],&CheckStatus, 2 );
			memcpy( &pcCmdData[3],&stRecvPkt.abData[2], 1 );
	        SetSharedDataforCmd( MAIN_SUB_COMM_REQ_PL_SEARCH, pcCmdData, 4 );
		}
    }
    else if ( stRecvPkt.bCmd == NAK )
    {
        memcpy( pcCmdData, "8", 1);
        pcCmdData[1] = NAK;
		pcCmdData[2] = NAK;			
        SetSharedDataforCmd( MAIN_SUB_COMM_REQ_PL_SEARCH, pcCmdData, 4 );
    }
    return 0;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendTDFile2MainTerm                                      *
*                                                                              *
*  DESCRIPTION:       Send subterminal's TD File to Mainterminal when bus      *
*                     running status is not drive                              *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo - subterminal device number like 1,2,3         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short SendTDFile2MainTerm( int nDevNo )
{
    int sRetVal = SUCCESS;
    byte abTmpFileName[30] = { 0, };
    int fdCheck = 0;

    memcpy(abTmpFileName, stRecvPkt.abData, stRecvPkt.nDataSize);

#ifdef TEST_DAESA
    printf( "\r\n �����Ⱑ ���ۿ䱸�� �ŷ��������ϸ� : [%s]\r\n", abTmpFileName );
#endif

	printf( "[SendTDFile2MainTerm] ��������ŷ��������� ������ ���� : [%s] ",
		abTmpFileName );
 
    fdCheck = open( SUB_TRANS_SEND_SUCC, O_RDWR | O_CREAT |O_TRUNC, OPENMODE );
    close( fdCheck );

    bCurrCmd = MAIN_SUB_COMM_GET_TRANS_FILE;
    sRetVal = SendFile( nDevNo, abTmpFileName );

    if ( sRetVal < 0 )
    {
#ifdef TEST_DAESA
        printf( "\r\n [SendTDFile]�ŷ����� ȭ�� ���� ���� : [%d]\r\n", sRetVal );
#endif
        if ( sRetVal == ERR_MAINSUB_COMM_FILE_NOT_FOUND )
        {
            DebugOut( "\r\n [SendTDFile] �ŷ����� ȭ�� ����" );
        }
    }
    else
    {
#ifdef TEST_DAESA
        printf( "\r\n[SendTDFile] �ŷ����� ȭ�� ���� ���� \r\n" );
#endif
        unlink( abTmpFileName );
    }

    system( "rm *.trn" );
    unlink( SUB_TRANS_SEND_SUCC );

#ifdef TEST_DAESA
    printf( "�������� ��� trn���ϻ���\n" );
#endif

    return ErrRet( sRetVal );

}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ImgFileRecvFromMainTermOldProtocol                       *		       
*                                                                              *
*  DESCRIPTION:       03xx������ ����ϴ� ���������ݷ� �������α׷� ������     *
*                     �����Ѵ�.                                                *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  void					                                   *
*                    												           *
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
*                    ���� ������ ���� stRecvPkt����ü ���� abData������ ���   *
*                    ���� �ʰ� abRecvData�� ���� �����Ͽ� ����ϰ� �ִ�.       *
*                                                                              *
*                                                                              *
*******************************************************************************/
short ImgFileRecvFromMainTermOldProtocol( void )
{
	int retVal;
	RS485_FILE_HEADER_PKT_OLD stfileHeaderPkt;
	
	char tmp_filename[13]= { 0, };
	char org_filename[13]= { 0, };

	memset( &stfileHeaderPkt, 0x00, sizeof( RS485_FILE_HEADER_PKT_OLD ));
	
	memcpy(stfileHeaderPkt.achFileName, abRecvData + 1, ( stRecvPkt.nDataSize) - 3);
	
	printf("\r\n rs485SubDResp rs485PktSend  before :\r\n");
		
	memcpy(tmp_filename,stfileHeaderPkt.achFileName,12);	
	memcpy(org_filename,stfileHeaderPkt.achFileName,12);	
	memcpy(&tmp_filename[9],"tmp",3);
	
	retVal = RecvFileOldProtocol(gbSubTermNo, tmp_filename);
	if (retVal < 0)
	{
		printf("\r\n rs485SubDResp rs485Download Fail : [%d]\r\n",retVal);	
		unlink(tmp_filename);		
	} 
	else 
	{
		rename(tmp_filename,org_filename);
//0330 2005.2.28  �������α׷� ���󿩺� Check 
		unlink("bus200");
		system("cp bus100 bus200");
		system("sync");
        unlink( STATUS_FLAG_FILE );

        ClosePeripheralDevices();
		system("reset");
		exit(1);
	}
	return retVal;
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvFileOldProtocol			                           *		       
*                                                                              *
*  DESCRIPTION:       03xx������ ����ϴ� ���������ݷ� �������α׷� ������     *
*                     �����Ѵ�.                                                *
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
*                    ���� ������ ���� stRecvPkt����ü ���� abData������ ���   *
*                    ���� �ʰ� abRecvData�� ���� �����Ͽ� ����ϰ� �ִ�.       *
*                                                                              *
*                                                                              *
*******************************************************************************/
short RecvFileOldProtocol( int nDevNo, char* pchFileName )
{
    int fdFile;
    short sRetVal = SUCCESS;

   /*
    * ���ŵ� ������ ������ ���� ����/����
    */
    fdFile = open( pchFileName, O_WRONLY | O_CREAT | O_TRUNC, OPENMODE );

    if ( fdFile < 0 )
    {
    	printf( " %s file open fail\n", pchFileName);
        close( fdFile );
        return -1;
    }
   /*
    * ��Ŷ���� �� ���� Loop 
    */
    while( 1 )
    {
	   /*
	    * ��Ŷ����
	    */
        sRetVal = RecvPkt( 8000, nDevNo );

        if ( sRetVal < 0 )
        {
            printf( "/r/n file recv Error: [1] /r/n" );
            break;
        }

	   /* 
	    * ��Ŷ���Ž� ACK������ ���� ������ ���� 
	    */
        stSendPkt.bCmd = 'D';
        stSendPkt.bDevNo = nDevNo + '0';
        stSendPkt.nDataSize = 1;
		stSendPkt.abData[0] = ACK;
	   /* 
	    * ACK���� 
	    */
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            DebugOut( "/r/n SendPkt Error  : [2] /r/n" );
            break;
        }

	   /* 
	    *  EOT���ſ��� üũ 
	    */
        if ( ( stRecvPkt.nDataSize == 1 ) && ( abRecvData[0] == EOT ) )
        {
			printf("���������ݷ� �߿��� �ٿ�ε� �Ϸ�(������� ���������� ������ ");
			break;
        }
	    /* 
	     *  ��Ŷ���ŵ����� ���Ͽ� ���� 
	     */
        write( fdFile, abRecvData, stRecvPkt.nDataSize );
		/* 
	     *  ���� ��Ŷ���Ž� ������ üũ�� ���� ���� �������� ����
	     */

    }

    close( fdFile );

    return sRetVal;
}



short ImgFileRecvFromMainTerm( void )
{
    int sRetVal =0;
    char achRecvData[30] = { 0, };
    char achTmpFileName[13] = { 0, };
    char achOrgFileName[13] = { 0, };

    memcpy( achRecvData, stRecvPkt.abData, stRecvPkt.nDataSize );
    achRecvData[stRecvPkt.nDataSize] = 0x00;

    DebugOut( "\r\n ������� ���� ������ �������α׷��� :[%s]\n", achRecvData );

    memcpy( achTmpFileName, achRecvData, strlen( achRecvData ) );
    memcpy( achOrgFileName, achRecvData, strlen( achRecvData ) );
    memcpy( &achTmpFileName[9], "tmp", 3 );

	printf( "[ImgFileRecvFromMainTerm] �������� : [%s] ", achTmpFileName );

    sRetVal = RecvFile( gbSubTermNo, achTmpFileName );

    if ( sRetVal < 0 )
    {
        printf( "\r\n Image Download Fail : [%d]\r\n", sRetVal );
        unlink( achTmpFileName );

        CreateNAK( gbSubTermNo );
        sRetVal = SendPkt();

        if ( sRetVal < 0 )
        {
            return ErrRet( sRetVal );
        }
        else
        {
            DebugOut( "������ �������α׷��� ���������� �ٿ��������� NAK����\n" );
        }

    }
    else
    {
        CreateACK( gbSubTermNo );
        sRetVal = SendPkt();

        if ( sRetVal < 0 )
        {
            return ErrRet( sRetVal );
        }
        else
        {
            DebugOut( "������ �������α׷��� ���������� �ٿ��� ACK���� �� RESET\n" );
        }

        rename( achTmpFileName, achOrgFileName );

        //0330 2005.2.28  �������α׷� ���󿩺� Check
        unlink( "bus200" );
        system( "cp bus100 bus200" );
        system( "sync" );
        unlink( STATUS_FLAG_FILE );

        ClosePeripheralDevices();
		printf("�������� �ٿ�ε� �Ϸ� - �������մϴ�.\n");
        system( "reset" );
        exit(1);
    }

    return ErrRet( sRetVal );
}

short VoiceFileRecvFromMainTerm( void )
{
    short sRetVal = SUCCESS;
    int   fdVoiceApply = 0;
    int   fdVoiceVer = 0;
    int   fdVoiceFile = 0;
    char achRecvData[30] = { 0, };
    char achTmpFileName[13] = { 0, };
    char achOrgFileName[13] = { 0, };
    char achTmpVer[5] = { 0, };

    memcpy( achRecvData, stRecvPkt.abData, stRecvPkt.nDataSize );
    achRecvData[stRecvPkt.nDataSize] = 0x00;

    DebugOut( "\r\n fileName :[%s]", achRecvData );

    memcpy( achTmpFileName, achRecvData, strlen( achRecvData ) );
    memcpy( achOrgFileName, achRecvData, strlen( achRecvData ) );
    memcpy( &achTmpFileName[5], "tmp", 3 );

	printf( "[VoiceFileRecvFromMainTerm] �������� : [%s] ", achTmpFileName );

    sRetVal = RecvFile( gbSubTermNo, achTmpFileName );
    if ( sRetVal < 0 )
    {
        printf( "\r\n rs485SubAResp rs485Download Fail : [%d]\r\n", sRetVal );
        unlink( achTmpFileName );

        CreateNAK( gbSubTermNo );
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            return ErrRet( sRetVal );
        }
        else
        {
            DebugOut( "���������� ���������� �ٿ��������� NAK����\n" );
        }
    }
    else
    {
        CreateACK( gbSubTermNo );
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            return ErrRet( sRetVal );
        }
        else
        {
            printf( "���������� ���������� �ٿ��� ACK���� �� RESET\n" );
        }

        unlink( achOrgFileName );
        rename( achTmpFileName, achOrgFileName );

        chmod( achOrgFileName, S_IRUSR|S_IWUSR );
		printf( "111\n" );
        gpstSharedInfo->bVoiceApplyStatus = 1; // ������

        /* voiceapply.dat ������ ���� */
        if (( fdVoiceApply = open( VOICEAPPLY_FLAGFILE,
                                   O_WRONLY |O_CREAT|O_TRUNC,
                                   OPENMODE )
                                 ) < 0 )
        {
            printf( "Error!!! voiceapply.dat open() failed\n" );
            return -1;
        }

        if (( fdVoiceVer = open( VOICEAPPLY_VERSION,
                                 O_WRONLY |O_CREAT|O_TRUNC,
                                 OPENMODE )
                                ) < 0)
        {
            printf( "Error!!!  voicever.dat open() failed\n" );
            close( fdVoiceApply );
            unlink( VOICEAPPLY_VERSION );
            return -1;
        }

        if (( fdVoiceFile = open( VOICE0_FILE, O_RDONLY, OPENMODE )) < 0 )
        {
            printf( "Error!!! c_v0.dat open() failed\n" );
            close( fdVoiceApply );
            close( fdVoiceVer );
            unlink( VOICEAPPLY_VERSION );
            return -1;
        }

        /* c_v0.dat.dat�� �� 4byte(��������)�� �о� voicever.dat�� ��� */
		
        sRetVal = lseek( fdVoiceFile, -4, SEEK_END );
		if ( sRetVal == -1 )
			perror("Voice File read lseek error : ");            		
        sRetVal = read( fdVoiceFile, achTmpVer, 4 );
		if ( sRetVal == -1 )
			perror("Voice File read error : ");    
        sRetVal = write( fdVoiceVer, achTmpVer, 4 );
		if ( sRetVal == -1 )
			perror("Voice Ver Write read error : "); 		

        close( fdVoiceApply );
        close( fdVoiceVer );
        close( fdVoiceFile );

        /* �������� �ٿ�ε� �Ϸῡ ���� ������ �����           */
        /* ����ý� �����⿡���� voiceapply.dat�� ������ ��쿡��
           chip�� ���������� Write�Ѵ�                           */
        printf("�������� �ٿ�ε� �Ϸ� - ������մϴ�\n");
        system( "reset" );
    }

    return sRetVal;

}


short ParameterFileRecvFromMainTerm( void )
{
    short sRetVal = SUCCESS;
    short sResult = SUCCESS;
    short sKeySetRetVal = SUCCESS;
    short sIDCenterRetVal = SUCCESS;
    char achTmpRecvFileName[30];
    char achTmpFileName[30];
    char achOrgFileName[30];

    byte bCmd = 0;
    char achCmdData[5] = { 0, };
    word wCmdDataSize = 0;

    byte abDebugCmdData[40];
    byte bDebugSharedMemoryCmd;
    word wDebugDataSize;

    while( stRecvPkt.bCmd != ETB )
    {
        memset( achTmpRecvFileName, 0, sizeof( achTmpRecvFileName ) );
        memset( achTmpFileName, 0, sizeof( achTmpFileName ) );
        memset( achOrgFileName, 0, sizeof( achOrgFileName ) );

        GetSharedCmd( &bCmd, achCmdData, &wCmdDataSize );
        if ( ( bCmd  == CMD_NEW_CONF_IMG ) &&
             ( achCmdData[0] == '0'  || achCmdData[0] == '9') )
        {
            ClearSharedCmdnData( CMD_NEW_CONF_IMG );
        }

        sRetVal = RecvPkt( 3000, gbSubTermNo ); // receive ack �� ó��
        if ( sRetVal < 0 )
        {
            return ErrRet( sRetVal );
        }

        memcpy( achTmpRecvFileName, stRecvPkt.abData, stRecvPkt.nDataSize );
        achTmpRecvFileName[stRecvPkt.nDataSize] = 0x00;

        DebugOut( "stRecvPkt.abData[0] %x\n", stRecvPkt.abData[0] );
        DebugOut( "stRecvPkt.nDataSize %d\n", stRecvPkt.nDataSize );
        DebugOutlnASC( " stRecvPkt.abData\n",
                       stRecvPkt.abData, stRecvPkt.nDataSize );
        DebugOut( "\r\n ������� ���� ���ϸ� :[%s]", achTmpRecvFileName );

        if ( stRecvPkt.bCmd == ETB )
        {
            // �Ķ�������� �ٿ� ������ ���� �����޸𸮸� �̿���
            // �������ο� �Ķ�������� RELOAD��û
            sResult = SetSharedCmdnData( CMD_NEW_CONF_IMG, "1100", 4 );

            if ( sResult < 0 )
            {
                printf( "Error!!�������ο� �Ķ�������� RELOAD��û����//////\n" );
                memset( abDebugCmdData, 0x00, sizeof( abDebugCmdData ) );
                GetSharedCmd( &bDebugSharedMemoryCmd, abDebugCmdData,
                              &wDebugDataSize );
                printf( "�Ӱ� ����ֳ�?====SharedMemoryCmd = %c====\n",
                         bDebugSharedMemoryCmd );
            }
            else
            {
                printf( "[ParameterFileRecvFromMainTerm] �������μ����� ����� �ε� ��û ����\n" );
            }        
            break;
        }

        CreateACK( gbSubTermNo );
        sRetVal = SendPkt();

        if ( sRetVal < 0 )
        {
            return ErrRet( sRetVal );
        }

        memcpy( achTmpFileName, achTmpRecvFileName,
                strlen( achTmpRecvFileName ) );
        memcpy( achOrgFileName, achTmpRecvFileName,
                strlen( achTmpRecvFileName ) );

        DebugOut( "\r\n achOrgFileName : [%s]", achOrgFileName );

        memcpy( &achTmpFileName[strlen( achTmpFileName )-3], "tmp", 3 );

		printf( "[ParameterFileRecvFromMainTerm] �������� : [%s] ",
			achTmpFileName );

        sRetVal = RecvFile( gbSubTermNo, achTmpFileName );
        if ( sRetVal < 0 )
        {
            printf( "[ParameterFileRecvFromMainTerm] �������� ���� : [%s]\n",
				achTmpFileName );
            unlink( achTmpFileName );
        }
        else
        {
			sResult = CheckValidFile( GetDCSCommFileNoByFileName( &achOrgFileName[2] ), achTmpFileName );
			if ( sResult == SUCCESS )
			{
				printf( " -> [%u][%s]\n", GetDCSCommFileNoByFileName( &achOrgFileName[2] ), achOrgFileName );
	            rename( achTmpFileName, achOrgFileName );
			}
			else
			{
				printf( " -> ��ȿ��üũ �����Ͽ� ���� [%u][%s]\n", GetDCSCommFileNoByFileName( &achOrgFileName[2] ), achOrgFileName );
				unlink( achTmpFileName );
			}

            if ( memcmp( achOrgFileName, PSAM_KEYSET_INFO_FILE, 12 ) == 0 )
            {
                // �ʱ�ȭ
                stSubTermPSAMAddResult.aboolIsKeySetAdded[gbSubTermNo-1] = FALSE;
                stSubTermPSAMAddResult.aboolIsIDCenterAdded[gbSubTermNo-1] = FALSE;

                sIDCenterRetVal = RegistOfflineID2PSAMbyEpurseIssuer();
                sKeySetRetVal = RegistOfflineKeyset2PSAMbyEpurseIssuer();

                if ( sIDCenterRetVal == SUCCESS )
                {
                    printf( "[ParameterFileRecvFromMainTerm] IDCENTER ��� ����\n" );
                    stSubTermPSAMAddResult.aboolIsIDCenterAdded[gbSubTermNo-1] = TRUE;
                }
                else
                {
                    printf( "[ParameterFileRecvFromMainTerm] IDCENTER ��� ����\n" );
                    stSubTermPSAMAddResult.aboolIsIDCenterAdded[gbSubTermNo-1] = FALSE;
                }

                if ( sKeySetRetVal == SUCCESS )
                {
                    printf( "[ParameterFileRecvFromMainTerm] KEYSET ��� ����\n" );
                    stSubTermPSAMAddResult.aboolIsKeySetAdded[gbSubTermNo-1] = TRUE;
                }
                else
                {
                    printf( "[ParameterFileRecvFromMainTerm] KEYSET ��� ����\n" );
                    stSubTermPSAMAddResult.aboolIsKeySetAdded[gbSubTermNo-1] = FALSE;
                }
            }
        }
    }

    return ErrRet( sRetVal );
}

short CheckReqBLPLSearch( bool *pboolIsBLPLCheckReq, byte *pbBLPLCmd )
{
    byte bCmd;
    byte pcCmdData[40];
    word wCmdDataSize;
    short sRetVal = SUCCESS;

    GetSharedCmd( &bCmd, pcCmdData, &wCmdDataSize );

#ifdef TEST_BLPL_CHECK
    printf( "[term_comm]recv blplreq from sharedmemory --> bCmd :
            %c, size : %d \n", bCmd, wCmdDataSize);
#endif

    if ( ( bCmd == MAIN_SUB_COMM_REQ_BL_SEARCH ) ||
         ( bCmd == MAIN_SUB_COMM_REQ_PL_SEARCH ) )
    {

        if ( pcCmdData[0] == '1' )
        {
            *pboolIsBLPLCheckReq = TRUE;
            *pbBLPLCmd = bCmd;
        }
        else
        {
            *pboolIsBLPLCheckReq = FALSE;
        }
    }
    else
    {
        *pboolIsBLPLCheckReq = FALSE;
    }

    return ErrRet( sRetVal );
}

// RTC Time ���� 
// ������, �����߿���, �����ܸ��� id, �ŷ��������� ����
short SetStationToSubTermial( void )
{
    short sRetVal = SUCCESS;
    byte abCurrTime[15] = { 0, };
    time_t tMainTermTime;

    memcpy( abCurrTime, stRecvPkt.abData , 14 );

    tMainTermTime = GetTimeTFromASCDtime( abCurrTime );
    SetRTCTime( tMainTermTime );

    memcpy( gpstSharedInfo->abNowStationID, stRecvPkt.abData + 14, 7 );
    memcpy( &( gpstSharedInfo->boolIsDriveNow) , stRecvPkt.abData + 21, 1 );
    memcpy( gpstSharedInfo->abMainTermID, stRecvPkt.abData + 22, 9 );
    memcpy( gpstSharedInfo->abTransFileName, stRecvPkt.abData + 31, 18 );
	gpstSharedInfo->gbGPSStatusFlag = stRecvPkt.abData[49];

    return ErrRet( sRetVal );
}



//******************************************************************************
//  OFFLINE ID CENTER ���
//  Return : 0 = OK, 1 = NG
//******************************************************************************
short RegistOfflineID2PSAMbyEpurseIssuer( void )
{
    FILE *fdIDCenter;
    int nRecCnt;
    int nReadSize;
    int i;
    short sRetVal = -1;
    short sRet;
    byte abTmpBuff[90] = { 0, };
    byte abTmpSAMID[17] = { 0, };

    DebugOut( "\r\n<OFFLINE_IDCENTER> ID CENTER ���\r\n " );

    fdIDCenter = fopen( EPURSE_ISSUER_REGIST_INFO_FILE, "rb" );
    if (fdIDCenter == NULL)
    {
        DebugOut( "\r\n<OFFLINE_IDCENTER> ���� ���� ����\n " );
        return ErrRet( ERR_MAINSUB_COMM_IDCENTER_FILE_OPEN );  // File not found
    }

    // �������� 7�ڸ�
    if ( ( nReadSize = fread( abTmpBuff, 7, 1, fdIDCenter ) ) != 1 )
    {
        // Format �ȸ���
        DebugOut( "\r\n<OFFLINE_IDCENTER> File Format �ȸ��� 1\n" );
        fclose( fdIDCenter );
        return ErrRet( ERR_MAINSUB_COMM_IDCENTER_FILE_APPLY_DATA );
    }

    // Record �Ǽ� 2�ڸ�
    if ( ( nReadSize = fread( abTmpBuff, 2, 1, fdIDCenter ) ) != 1 )
    {
        // Format �ȸ���
        DebugOut( "\r\n<OFFLINE_IDCENTER> File Format �ȸ��� 2\n" );
        fclose( fdIDCenter );
        return ErrRet( ERR_MAINSUB_COMM_IDCENTER_FILE_RECORD_CNT );
    }

    nRecCnt = GetDWORDFromASC( abTmpBuff, 2 );
    DebugOut( "�Ǽ���=>[%d]", nRecCnt );

    if ( nRecCnt < 1 || nRecCnt > 99 )
    {
        // Record �Ǽ��� ����
        DebugOut( "\r\n<OFFLINE_IDCENTER> Record �Ǽ��� ����\n" );
        fclose( fdIDCenter );
        return ErrRet( ERR_MAINSUB_COMM_IDCENTER_FILE_RECORD_CNT );
    }
    else
    {
        DebugOut( "\r\n<OFFLINE_IDCENTER> Record �Ǽ�[%d]\n", nRecCnt );
    }

    // PSAM���� �ٿ�ε�
    for ( i = 0; i < nRecCnt; i++ )
    {
        nReadSize = fread( &stIDCenterData, sizeof(stIDCenterData), 1, fdIDCenter );

        memset( abTmpBuff, 0x00, sizeof( abTmpBuff ) );

        if ( nReadSize != 1 )
        {
            DebugOut( "Format �ȸ���\n" );
            sRetVal = ERR_MAINSUB_COMM_IDCENTER_FILE_READ_OR_EOF;
            break; // Format �ȸ���
        }

        memcpy( abTmpBuff, stIDCenterData.abSAMID, 8 );
        memcpy( &abTmpBuff[8], stIDCenterData.abEKV, 16 );
        memcpy( &abTmpBuff[24], stIDCenterData.abSign, 4 );

        BCD2ASC( abTmpBuff, abTmpSAMID, 8 );

        if ( gboolIsMainTerm == TRUE )
        {
#ifdef TEST_IDCENTER_KEYSET_REGIST
            PrintlnASC( "\n�� PSAMID : ", gpstSharedInfo->abMainPSAMID, 16 );
#endif

            if ( memcmp( abTmpSAMID, gpstSharedInfo->abMainPSAMID, 16 ) == 0 )
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_IDCENTER> ���� �����̵�� ���� ������ �߰�\n" );
#endif
            }
            else
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_IDCENTER>SAM-ID [%s]\n", abTmpSAMID );
                printf( "<OFFLINE_IDCENTER>IDCENTER Version : [%02x] \n",
                        stIDCenterData.bIDCenter );
                printf( "<OFFLINE_IDCENTER>���� �����̵�� �ٸ� ������\n" );
#endif
                continue;
            }
        }
        else
        {
#ifdef TEST_IDCENTER_KEYSET_REGIST
            PrintlnASC( "\n�� PSAMID : ",
                        gpstSharedInfo->abSubPSAMID[gbSubTermNo-1], 16 );
#endif
            if ( memcmp( abTmpSAMID,
                         gpstSharedInfo->abSubPSAMID[gbSubTermNo-1],
                         16 ) == 0 )
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_IDCENTER> ���� �����̵�� ���� ������ �߰�\n" );
#endif
            }
            else
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_IDCENTER>SAM-ID [%s]\n", abTmpSAMID );
                printf( "<OFFLINE_IDCENTER>IDCENTER Version : [%02x] \n",
                        stIDCenterData.bIDCenter );
                printf( "<OFFLINE_IDCENTER>���� �����̵�� �ٸ� ������\n" );
#endif
                continue;
            }
        }

        sRet = PSAMAddCenterOffline( stIDCenterData.bIDCenter, abTmpBuff );

#ifdef TEST_IDCENTER_KEYSET_REGIST
        printf( "<OFFLINE_IDCENTER> ������ [%d] \n", sRet);
#endif
        if ( ( sRet == SUCCESS ) ||
             ( sRet == ERR_SAM_IF_PSAM_ENREGIST_IDCENTER ) )
        {
            sRetVal = SUCCESS;
//            printf( "IDCENTER ��� �Ϸ� \n" );
        }
        else
        {
            sRetVal = ERR_MAINSUB_COMM_ADD_IDCENTER_OFFLINE;
        }

        break;

    }

    fclose( fdIDCenter );

    return ErrRet( sRetVal );
}


//******************************************************************************
//  OFFLINE KEYSET DOWNLOAD
//  Return : 0 = OK, 1 = NG
//******************************************************************************
short RegistOfflineKeyset2PSAMbyEpurseIssuer( void )
{
    short   sRetVal = -1;
    short   sRet;
    FILE    *fdKeyset;
    int     nRecCnt;
    int     nReadSize;
    int     i;
    byte    abTmpBuff[90] = { 0, };
    byte    abTmpSAMID[17] ={ 0, };

    DebugOut( "\r\n<OFFLINE_KEYSET> KEYSET ���\r\n " );

    fdKeyset = fopen( PSAM_KEYSET_INFO_FILE, "rb" );
    if ( fdKeyset == NULL )
    {
        DebugOut( "\r\n<OFFLINE_IDCENTER> ���� ���� ���� \n" );
        return ErrRet( ERR_MAINSUB_COMM_KEYSET_FILE_OPEN);
    }

    // �������� 7�ڸ�
    if ( ( nReadSize = fread( abTmpBuff, 7, 1, fdKeyset ) ) != 1 )
    {
        // Format �ȸ���
        DebugOut( "\r\n<OFFLINE_KEYSET> �������� 7�ڸ� // Format �ȸ���\n" );
        fclose( fdKeyset );
        return ErrRet( ERR_MAINSUB_COMM_KEYSET_FILE_APPLY_DATA );
    }

    // Record �Ǽ� 2�ڸ�
    if ( ( nReadSize = fread( abTmpBuff, 2, 1, fdKeyset ) ) != 1 )
    {
        // Format �ȸ���
        DebugOut( "\r\n<OFFLINE_KEYSET> Record �Ǽ� 2�ڸ� // Format �ȸ���\n" );
        fclose( fdKeyset );
        return ErrRet( ERR_MAINSUB_COMM_KEYSET_FILE_RECORD_CNT );
    }

    nRecCnt = GetDWORDFromASC( abTmpBuff, 2 );
    DebugOut( "�Ǽ��� =>%d", nRecCnt );

    if ( nRecCnt < 1 || nRecCnt > 99 )
    {
        // Record �Ǽ��� ����
        DebugOut( "\r\n<OFFLINE_KEYSET> Record �Ǽ��� ����\n" );
        fclose( fdKeyset );
        return ErrRet( ERR_MAINSUB_COMM_KEYSET_FILE_RECORD_CNT );
    }
    else
    {
        DebugOut( "\r\n<OFFLINE_KEYSET> Record �Ǽ�[%d]\n", nRecCnt );
    }


    // SAM���� �ٿ�ε�
    for ( i = 0; i < nRecCnt; i++ )
    {
        nReadSize = fread( &stKeySetData, sizeof( stKeySetData ), 1, fdKeyset );

        memset( abTmpBuff, 0x00, sizeof( abTmpBuff ) );

        if ( nReadSize != 1 )
        {
            sRetVal = ERR_MAINSUB_COMM_KEYSET_FILE_READ_OR_EOF;
            break; // Format �ȸ���
        }

        memcpy( abTmpBuff, stKeySetData.abSAMID, 8 );
        abTmpBuff[8] =  stKeySetData.bSortKey;
        memcpy( &abTmpBuff[9], stKeySetData.abVK, 4 );
        memcpy( &abTmpBuff[13], stKeySetData.abEKV, 64 );
        memcpy( &abTmpBuff[77], stKeySetData.abSign, 4 );

        BCD2ASC( abTmpBuff, abTmpSAMID, 8 );

        if( gboolIsMainTerm == TRUE )
        {
#ifdef TEST_IDCENTER_KEYSET_REGIST
            PrintlnASC( "\n�� PSAMID : ",
                                   gpstSharedInfo->abMainPSAMID, 16 );
#endif
            if ( memcmp( abTmpSAMID, gpstSharedInfo->abMainPSAMID, 16 ) == 0 )
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
               printf( "<OFFLINE_KEYSET>  ���� �����̵�� ���� ������ �߰� \n" );
#endif
            }
            else
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_KEYSET>PSAM-ID [%s]\n", abTmpSAMID );
                printf( "<OFFLINE_KEYSET>KEYSET idcenter : [%02x] \n",
                        stKeySetData.bIDCenter );
                printf( "<OFFLINE_KEYSET>���� �����̵�� �ٸ� ������\n" );
#endif
                continue;
            }
        }
        else
        {
#ifdef TEST_IDCENTER_KEYSET_REGIST
            PrintlnASC( "\n�� PSAMID : ",
                        gpstSharedInfo->abSubPSAMID[gbSubTermNo-1], 16 );
#endif
            if ( memcmp( abTmpSAMID,
                         gpstSharedInfo->abSubPSAMID[gbSubTermNo-1],16
                       ) == 0 )
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_KEYSET> ���� �����̵� ���� \n" );
#endif
            }
            else
            {
#ifdef TEST_IDCENTER_KEYSET_REGIST
                printf( "<OFFLINE_KEYSET>SAM-ID [%s]\n", abTmpSAMID );
                printf( "<OFFLINE_KEYSET>KEYSET idcenter : [%02x] \n",
                        stKeySetData.bIDCenter );
                printf( "<OFFLINE_KEYSET>���� �����̵� Ʋ��\n" );
#endif
                continue;
            }
        }

        sRet = PSAMAddKeySetOffline( stKeySetData.bIDCenter, abTmpBuff );
#ifdef TEST_IDCENTER_KEYSET_REGIST
        printf( "\r\n<OFFLINE_KEYSET> ������ [%d] \n", sRet);
#endif
        if ( ( sRet == SUCCESS ) ||
             ( sRet == ERR_SAM_IF_PSAM_ENREGIST_IDCENTER ) )
        {
            sRetVal = SUCCESS;
//            printf( "keyset ��� �Ϸ� \n" );
        }
        else
        {
            sRetVal = ERR_MAINSUB_COMM_ADD_IDCENTER_OFFLINE;
        }
        break;

    }

    fclose( fdKeyset );

    return sRetVal;
}



















































