/*******************************************************************************
*                                                                             *
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
*  PROGRAM ID :       term_comm.c                                              *
*                                                                              *
*  DESCRIPTION:       ���������� ����� ó���ϴ� �������μ���                    *
*                                                                              *
*  ENTRY POINT:       CommProc()                                               *
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
/*  ���������� �߿��� �ٿ�ε� ���� //////////////////////////////////////////// 
*																			   *	
* 	���߿������ ���� ���������� ������������� �������� ���� ����������     *
* 	���� ����ϴ� ���������� ������ ������ �Ǻ��Ͽ� ������������ ��� ���߿��� *
*   �� �ٿ�ε��Ͽ� ���׷��̵� �Ѵ�. ������������ ����ϴ� ��쿡�� ���������� *
*   ó���Լ� MainTermProc()/SubTermProc()�� ����Ǹ� ������ ���� ��� �ٿ�ε� *
* 	�� �翬�� �ȴ�.                                                            *
*                                                                              *
*	������� )                                                                 *
*	������ ó���Լ�(MainTermProc)�������� ������ ���������� ���Ȯ�� ��        *
*	���������� �߿���� �ٿ�ε��ϴ� �Լ��� UpdateSubTermImg�Լ���             *
*	���� ���� �����ϰ� �Ͽ� ó���Ѵ�.                                          *
*                                                                              *
*	                                                                           *
*	UpdateSubTermImg�Լ��� ������������ ���·� ������ ����üũ ��ɾ ������ *
*	�ȴ�.                                                                      *
*	( ����.                                                                    *
*	   1) ������������ ����ϴ� �߿���� ����üũ(V) ��ɾ� ���۽� 40 byte��   *
*	      �����ϱ� �Ǵµ�                                                      *
*          STX LEN LEN CMD DEVNO ������(30b) ETX CRC CRC CRC CRC               *
*	      �����ͺκп��� DEVNO(1b),CSAMID(4b),PSAMID(8b),�����ܸ���ID(9),      *
*         0x00(8b)�� ����                                                      *
*	                                                                           *
*       2) ���߿������ UpdateSubTermImg�� ChkSubTermImgVerOldProtcol() �Լ� *
*          ���� �Ʒ��� ���� ������������ ����ϴ� ��ó�� �����ϰ� ��Ŷ�� ����  *
*          �ϵ� �����Ϳ� 0x00(8b)�κп� ���������(4b)�� �־ �����ϰ� �ȴ�. *
*	      STX LEN LEN CMD DEVNO ������(30b) ETX CRC CRC CRC CRC                *
*	      �����ͺκп��� DEVNO(1b),CSAMID(4b),PSAMID(8b),�����ܸ���ID(9),      *
*         ���������(4b),0x00(4b)�� ����.                                      *
*	)                                                                          *
*	                                                                           *
*	�����⿡����                                                               *
*	1)������������ ����ϴ� �߿���� �ڽ��� ���� 4byte�� �������� ����         *
*	2)������������ ����ϴ� �߿���� ���� 4byte�̿ܿ� "new"��� 3byte��        *
*	  �߰��� ���������μ� �ڽ��� ������������ ����ϴ� �߿������� �����⿡     *
*     �˸��� �ȴ�.                                                             *
*                                                                              *
*	                                                                           *
*	�����Ⱑ ������������ ����ϴ� �߿���� �Ǹ��� �Ǹ� ������������ �̿��Ͽ�  *
*	������������ ����ϴ� �߿�� ���������μ� ���׷��̵尡 �Ϸ� �ȴ�.        *
*                                                                              *
*	�����Ⱑ ������������ ����ϴ� �߿���� �Ǹ�Ǹ� UpdateSubTermImg�Լ���    *
*   �����ϰ� ������ó���Լ�(MainTermProc)������ ������ ���Ͽ� ���׷��̵尡   *
*   �ǵ��� �������� ���μ����� ���� ó���ϵ��� �Ѵ�.                           *
*                                                                              *
*	�ݸ� �����Ⱑ ������������ ����ϴ� �߿����̰� �����Ⱑ ������������ ���  *
*   �ϴ� �߿����ΰ��                                                          *
*	( �ʱ������ ������ �߻��Ͽ� ������������ ����ϴ� 03xx���� �߿��       *
*	������ 04xx�� �ؼ� �����⿡ ������ ��쿡 �߻��Ѵ�)                        *
*    ����üũ ��ɾ ������������ ����ϴ� �߿�� �����ϸ� ������������    *
*    ����ϴ� �߿���� �����ϰ� ������ �� �ֵ��� ���߿����� �����⿡ ������    *
*    �Ǿ������� ������ ���� ��� ������������ ����ϴ� 04xx�߿�� �ٿ�ε�� *
*    �����Ҽ� �ִ� �κ��� ���߿��� �����Ǿ� �־�� �Ѵ�.                       *
*	                                                                           *
*                                                                              *
*	SendPkt/RecvPkt�Լ��� �������Ⱑ �������� ����ϴ� �Լ���                  *
*	���������ݿ� �°� SendPkt�ϱ����ؼ� �����⿡���� boolIsOldProtocol ��������*
*	�� �̸� �����Ͽ� ���������ݷ� �۽��ؾ����� �˷��ش�.                       *
*	                                                                           *
*	���������ݿ� �°� RecvPkt�ϱ����ؼ�	�����⿡���� 1)2)�� ���ǿ� �����ϴ�    *
*   ��� ���������ݷ� �����ϰ� �ȴ�.                                           *
*	1) Cmd�� 'V'�̰� ���ŵ����ͻ���� 40byte�� ��� - UpdateSubTermImg()���� *
*      ���� ����Ȯ�ν�                                                         *
*	2) Cmd�� 'D'�� ��� - UpdateSubTermImg()���� ���������ݷ� �߿��� �ٿ�ε��* 
*	                                                                           *
*	�̴� ��/���������� ��� �߿���� �ۼ��� �ִ���Ŷ����� �ٸ��� �����̴�.*
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/

#include "dcs_comm.h"
#include "term_comm.h"
#include "term_comm_mainterm.h"
#include "term_comm_subterm.h"
#include "../proc/main.h"
#include "../proc/file_mgt.h"
#include "../proc/write_trans_data.h"
#include "../proc/blpl_proc.h"
#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "../proc/card_proc_util.h"

MAINSUB_COMM_USR_PACKET stSendPkt; /* Send Data */
MAINSUB_COMM_USR_PACKET stRecvPkt; /* Receviced Data */

SUB_VERSION_INFO_LOAD stSubVerInfo;/* SubTerminal Version Infomaiton */

/* Result of adding KeySet/IDCenter to mainterm PSAM */
//static MAINTERM_PSAM_ADD_RESULT stAddMainTermPSAM;

/* Result of adding KeySet/IDCenter to subterm PSAM */
SUBTERM_PSAM_ADD_RESULT  stSubTermPSAMAddResult;

/* �����Ⱑ �������� ����üũ��ɿ� ���� ���������ݷ� ������ �ؾ��ϴ��� ����   */
bool boolIsRespVerNeedByOldProtocol = FALSE;
/* ��Ŷ�۽�/���Ž� ������������ ����Ͽ� �������ϴ��� ���� */
bool boolIsOldProtocol;  

/* old protocol���� stSendPkt.abData��� ����ϱ� ���� buffer */
 byte abSendData[1024] = { 0, };  
/* old protocol���� stRecvPkt.abData��� ����ϱ� ���� buffer */
 byte abRecvData[1048] = { 0, };  

bool boolIsMainSubCommFileDownIng;
/* 
 * ������ ������μ������� ���� ����ǰ� �ִ� ��ɾ� ���� ����
 */
byte bCurrCmd; 
/* 
 * �����ܸ��� ���� 
 */
int nIndex = 0;  

/*******************************************************************************
*  Declaration of function prototype                                           *
*******************************************************************************/
short UpdateSubTermImg( void );
short IsCmdValidCheck( byte bCmd );
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CommProc                                                 *
*                                                                              *
*  DESCRIPTION:       CommProc is main part of processing communication between*
*                     mainterminal and subterminal                             *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:           Following function open rs422 communication channel      *
*                     , try channel test and then call main part according     *
*                     to the terminal type                                     *
*******************************************************************************/
void CommProc( void )
{
    short sRetVal = SUCCESS;
    int nErrCnt = 0;

	/* 
	 * ��������� ��Ʈ Open - RS422 
	 */
    while(1)
    {
    	LogTerm( "CommOpen \n" );
        sRetVal = CommOpen( DEV_MAINSUB ); /* Open Serial Channel */
        if ( sRetVal < 0 )
        {
            nErrCnt++;
            /* Try 3 times to open it */
            if ( nErrCnt == MAX_TRIAL_COUNT_OPEN_UART )
            {
                ErrProc( sRetVal);
            }
        }
        else
        {
            break;
        }
    }

    /* 
     * �ܸ��� ���ϱ��п� ���� ó�� 
     */	
    if ( gboolIsMainTerm == TRUE ) 
    {
	    /* 
	     * �������� ��� �����Ⱑ 03xx���� �������߿������� ���� Ȯ���ϰ� 
	     * �������� ��� �ٿ�ε�� �����⸦ ������Ʈ ���ְ� ���� ó���Լ� ���� 
	     */	
		/* 
		 * ������ ������ Ȯ�� �� �������ϰ�� �Ź������� �ٿ�ε� ó�� 
		 */ 	     
		UpdateSubTermImg();

		/* 
		 * ������ ó�� �Լ� ����
		 */
        MainTermProc(); 
    }
    else  
    {
	    /* 
	     * �������� ��� 
	     */          
        SubTermProc();  /* ������ ó�� �Լ� ����*///////////////////////////////
    }
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       UpdateSubTermImg                                         *
*                                                                              *
*  DESCRIPTION:       ������ ������ Ȯ�� �� �������ϰ�� �Ź������� �ٿ�ε�   *
*                     �� �����Ѵ�.                                             *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���༺��                                       *
*                     SendPkt,RecvPkt �Լ��� �����ڵ带 �״�� ����            *                      *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:           														   *
*                                                                              *
*******************************************************************************/
short UpdateSubTermImg( void )
{
    int nDevNo = 0;     /* sub Term No. */
    int nRetryCnt = 0;
    short sRetVal = SUCCESS;
    char achNewVerYn[3] = { 0, }; /* ������ ���������� ��뿩�� */
	int  nSubIDInfoExistYN = 0;

	LogTerm( "UpdateSubTermImg \n" );
        
    /* 
     * ������ ���۱⿡ "�۾���.." ǥ�� 
     */    
    gpstSharedInfo->boolIsKpdLock = TRUE;

	if ( gpstSharedInfo->boolIsDriveNow == TRUE)
	{
		ctrl_event_info_write( "9001" );
	}
	
    /* 
     * SendPkt/RecvPkt �Լ������� ������������ ����Ͽ� �ۼ����ؾ� ���� �˷��ִ�
     * Flag ���� 
     */
    boolIsOldProtocol = TRUE;
	
	/* 
	 * �����ܸ��� ��ȣ �ʱⰪ ���� 
	 */
    nDevNo = 1; 
    DebugOut( "gbSubTermCnt=>[%d]\n", gbSubTermCnt );

	nSubIDInfoExistYN = access( SUBTERM_ID_FILENAME, F_OK );  // subid.dat

    /* 
     * �������������Ŀ� ���� ������ ���� üũ ��ɾ� ����  
     * - �����ܸ��� ����(gbSubTermCnt)��ŭ �ݺ�
     */
    while ( nDevNo <= gbSubTermCnt )
    {
    	if ( nSubIDInfoExistYN == 0 )
    	{
	        nRetryCnt = 3; /* ��õ� Ƚ�� */
    	}
		else
		{
			nRetryCnt = 2; /* ��õ� Ƚ�� */
		}

        while( nRetryCnt-- )
        {
#ifdef TEST_SYNC_FW        
            printf( "[%d] �� ������ ���������ݻ�뿩�� üũ ����\n", nDevNo );
#endif
			usleep( 100000 );
            sRetVal = ChkSubTermImgVerOldProtcol( nDevNo, achNewVerYn );

            if ( sRetVal != SUCCESS )
            {
                printf( "������ üũ �� ����- [%x]\n", sRetVal );
            }
            else
            {
                printf( "[%d] �� ������ ������ üũ ����\n",nDevNo );
                break;
            }
        }
        /* 
         * ������ܸ��� ��ȣ ����
         */
        nDevNo++;
    }


    nDevNo = 1;

    while ( nDevNo <= gbSubTermCnt )
    {
        nRetryCnt = 5;

        /* 
         * �����Ⱑ �����������̸� ������ ������ �����̹����� bus100
         * �� ������� �ٿ�ε�                      
         */             
        if ( achNewVerYn[nDevNo-1] == SUBTERM_USE_OLD_PROTOCOL )
		{
           while( nRetryCnt-- )
            {
                printf( "[%d] �� ������ �ٿ�ε� �õ� : %dȸ/�ִ�10ȸ\n",
                          nDevNo, 5-nRetryCnt );
				/* 
				 * bus100�� ��¥�� ����(18byte)�� ���� ������� ���α׷� �ٿ�ε� 
				 */
                sRetVal = SendSubTermImgOldProtcol( nDevNo, BUS_EXECUTE_FILE );
                if ( sRetVal != SUCCESS )
                {
                    printf( "[%d]�� ������� ���������� ��� f/w �ٿ�ε� �� ����\n",nDevNo );
                    tcflush( nfdMSC, TCIOFLUSH );          
                    sleep(2);
                }
                else
                {
                    printf( "[%d]�� ������� ���������� ��� f/w �ٿ�ε� ����\n",nDevNo );
                    tcflush( nfdMSC, TCIOFLUSH );             
                    if (  gbSubTermCnt > 1 )
                    {
			            /* 
			             * ������ ���α׷� �ٿ�ε�� �ٿ�ε� ������
			             * ���ýð� Ȯ���� ���� 50�ʰ� sleep�� �ش�
			             * ���� - �ð��� ���� ���� ��� ���� �ܸ����� 
			             *        �ٿ�ε尡 ���������� ���� �ʴ� ������ �߻�
			             */                         
                        sleep( 50 );
                    }
					else
					{
					   /* 
                        * �����Ⱑ 1���ΰ�쿡�� 1���� �����Ⱑ �����ϴ� �ð���ŭ
					    */
						sleep( 25 );
					}
                    break;
                }
            }
        }
        else
        {
            printf( "[%d] �� ������ ����", nDevNo );
            PrintlnASC( ":", gpstSharedInfo->abSubVer[nDevNo-1], 4 );
        }
        /* 
         * ������ܸ��� ��ȣ ����
         */
        nDevNo++;
    }


    /* 
     * SendPkt/RecvPkt �Լ������� ������������ ����Ͽ� �ۼ����ؾ� ���� �˷��ִ�
     * Flag ���� 
     */

	 boolIsOldProtocol = FALSE;

	/* 
     * 0401 ���� c_ex_pro.dat�� ���迡�� ������� �������� �ʱ�� ������Ƿ�
     * ������ c_ex_pro.dat�� �������� ��� �ѹ��� �������־�� ��.
	 */
	system("rm c_ex_pro.dat");

	return SUCCESS;
}




/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendPkt												   *
*                                                                              *
*  DESCRIPTION:       ���۱���ü�� �����͸� �������ݿ� �°� ��Ŷ�� �����Ͽ�    *
*                     �����Ѵ�.												   *
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
*  REMARKS: �� �������� ��Ŷ������ �Ʒ��� ����.                                *
*			STX LEN LEN CMD DEVNO SEQ SEQ DATA(N byte) ETX CRC CRC CRC CRC     *
*           �� �������� ��Ŷ������ �Ʒ��� ����.                                *
*			STX LEN LEN CMD DEVNO SEQ SEQ DATA(N byte) ETX CRC CRC CRC CRC     *
*																			   *		
*          																	   *
*                                                                              *
*******************************************************************************/
short SendPkt( void )
{
    byte abSendBuffer[1100] = { 0, }; /* �۽ŵ����� ���� */
    int nSendBufferLen = 0;           /* �۽ŵ����� ���� ���� */  
    dword dwCommCrc;                  /* CRC �� */
    short sRetVal;
    /*
     * �� �������ݷ� ����(03xx ����)
     */
   /* if ( ( boolIsOldProtocol == TRUE ) ||
         ( stSendPkt.nDataSize == 1 &&
           stSendPkt.bCmd == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) ||
         ( stSendPkt.nDataSize == 17 &&
           stSendPkt.bCmd == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) )
           */     
	         
    if ( boolIsOldProtocol == TRUE )	
    {

        DebugOut( "�� �������ݷ� send==\n" );

        nSendBufferLen = stSendPkt.nDataSize + 10;
		/* 1. STX - 1byte */
        abSendBuffer[0] = STX;   
		/* 2. �����ͱ��� - 2byte */		
        abSendBuffer[1] = ((stSendPkt.nDataSize + 2) & 0xFF00)>>8; 
        abSendBuffer[2] = (stSendPkt.nDataSize + 2) & 0x00FF;
		/* 3. bCmd(��ɾ�)- 1byte */			
        abSendBuffer[3] = stSendPkt.bCmd;  
		/* 4. bDevNo(�ܸ����ȣ) - 1byte */		
        abSendBuffer[4] = stSendPkt.bDevNo;   

		/* �۽�Buffer�� ������ Copy */
        if ( boolIsMainSubCommFileDownIng == TRUE ) 
        {
			/* 
			 * ���������α׷����� �ٿ�ε带 ��Ÿ���� Flag�� ���� �Ǿ��ִٸ� 
			 * stSendPkt����ü���� abData��ſ� abSendData����ü�� ����Ѵ�.
			 * ���� - ���������ݰ� �ִ���Ŷ����� Ʋ���� ������ 
			 */        
            memcpy( &abSendBuffer[5], abSendData, stSendPkt.nDataSize );
        }
        else
        {
			/* 
			 * ���������α׷����� �ٿ�ε� ���� �ƴѰ��� 
			 * �״�� stSendPkt����ü �̿�
			 */
            memcpy( &abSendBuffer[5], stSendPkt.abData, stSendPkt.nDataSize );
        }

    }
   /*
    * �� �������ݷ� ����(04xx ����) 
    */
    else
    {
        nSendBufferLen = stSendPkt.nDataSize + 12;

		/* 1. STX - 1byte*/
        abSendBuffer[0] = STX; 
		/* 2. �����ͱ��� - 2byte*/
        abSendBuffer[1] = ((stSendPkt.nDataSize) & 0xFF00)>>8; 
        abSendBuffer[2] = (stSendPkt.nDataSize ) & 0x00FF;
		/* 3. bCmd(��ɾ�)- 1byte*/		
        abSendBuffer[3] = stSendPkt.bCmd; 
		/* 4. bDevNo(�ܸ����ȣ) - 1byte */
        abSendBuffer[4] = stSendPkt.bDevNo; 

		/* 5. Sequence - 2byte	*/
        memcpy( &abSendBuffer[5], ( byte*)&stSendPkt.wSeqNo, 2);
        /*
		 * 6. �����͸� ������ �����ŭ �۽Ź��ۿ� Copy         
 		 * �����Ͱ� ���� ��� ��, �����ͻ���� 0�ΰ�쿡�� ���������� ������
 		 * ������ �ƹ��͵� ���� �ʴ´�.
		 */
        if ( stSendPkt.nDataSize != 0 )
        {
            memcpy( &abSendBuffer[7], stSendPkt.abData, stSendPkt.nDataSize );
        }
    }

	/* 6. ETX - 1byte	*/  
    abSendBuffer[nSendBufferLen-5] = ETX; 
	/* 7. ��Ŷ CRC - 4byte	*/
    dwCommCrc = MakeCRC32( abSendBuffer, nSendBufferLen-4 );
    memcpy( &abSendBuffer[nSendBufferLen-4], ( byte *)&dwCommCrc, 4 );

   /*
   	* �۽ŵ����� ������ �����͸� ����
   	*/
    sRetVal = CommSend( nfdMSC, abSendBuffer, nSendBufferLen );
	//PrintlnBCD("Send Data", abSendBuffer, nSendBufferLen);
    if ( sRetVal < 0 )
    {
       /*
       	* ���۽��н� ���۵����� ����ü �ʱ�ȭ 
       	*/
        memset( &stSendPkt, 0x00, sizeof(stSendPkt) );
        return ErrRet( sRetVal );
    }

    return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvPkt												   *
*                                                                              *
*  DESCRIPTION:       ���۱���ü�� �����͸� �������ݿ� �°� ��Ŷ�� �����Ͽ�    *
*                     �����Ѵ�.												   *
*																			   *
*  INPUT PARAMETERS:  int nTimeOut - ���� TimeOut�ð�                          *
*                     int nDevNo   - �ܸ��� ��ȣ                               *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
* 					  ERR_MAINSUB_COMM_STX_IN_PKT - STX����      			   *
*                     ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT 					   *
*                      - �����Ⱑ �۽��� ������ ��ٸ��� �ܸ��Ⱑ �ƴ� Ÿ      *
*                        �ܸ��� ��Ŷ���� ���� 								   *
* 				      ERR_MAINSUB_COMM_NOT_MINE_PKT							   *
*                      - �����Ⱑ �ڽ��� �ܸ����ȣ�� �ٸ� ��Ŷ���� ����       *
*                     ERR_MAINSUB_COMM_INVALID_LENGTH                          *
*                      - ������ ���� ����                                      *
*                     ERR_MAINSUB_COMM_SEQ_IN_PKT                              *
*                      - ������ ������ ����                                    *
*                 	  ERR_MAINSUB_COMM_ETX_IN_PKT                              *
*                      - ETX ����                                              *
* 					  ERR_MAINSUB_COMM_CRC_IN_PKT                              *
*                      - CRC ����                                              *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:     													           *
*                                                                              *
*******************************************************************************/
short RecvPkt( int nTimeOut, int nDevNo )
{

    byte abTmpBuff[25] = { 0, };    /* �ӽ� ����  */
    byte abRecvBuff[1048]= { 0, };  /* ���ŵ����� ���� */
    int nRecvDataLen;				/* ���ŵ����� ���� */
    int nRecvBuffLen;				/* ���Ź��� ���� */
    word wSeqNo;					/* ������ ��ȣ */
    dword dwCommCRC;				/* CRC�� */
    short sRetVal = SUCCESS;
    short sRetVal1 = SUCCESS;

    int nEtxPoint = 5;              /* �������ݻ󿡼� ETX�� ��ġ - ������ 5��° */
	/* 
	 * ������ �߿���(03xx)������ ��ſ��� CRC, BCC�� ȥ��Ǿ� ���Ǿ���
	 * ���� �������μ������� ���ý� �ʱ⿡ CRC�� ����ϴ� �ܸ����� ��쿡��
	 * ��ɾ� 'C'�� ���ϸ��� �����Ͽ� �ڽ��� CRC�� ���� �ܸ����ΰ��� ���濡��
	 * �˷��ְ� �Ǿ��־���.
	 * ���� �Ź��������� ��Ŷ�� �����Ͽ� 'C'�� ��쿡�� boolIsCcmdRecv
	 * ������ flag���� ����Ͽ� �ٽ��ѹ� �����͸� ���� �ϵ��� ó���Ͽ���
	 */
    bool boolIsCcmdRecv = FALSE;    /* CRC Command('C') ���ſ��� Flag */

   /* 
	* ���ŵ����� ����ü �ʱ�ȭ
	*/
    memset( &stRecvPkt, 0, sizeof( stRecvPkt ) );

   /* 
	* do ~ while( C ��ɾ� �����ΰ�� ) ������ Loop���� 
	*/
    do
    {
        /*
         * 1. ������ ���� ���� - 1byte//////////////////////////////////////////
         */
        DebugOut( "\n***************************************************" );
        sRetVal = CommRecv( nfdMSC, abTmpBuff, 1, nTimeOut );
        /*
         * ������ ���� ���� ���н� 
         */
        if ( sRetVal < 0 )  
        {
            DebugOut( "\n    [STX  ����]���� => [%d]\n", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof( stRecvPkt ) );
            tcflush( nfdMSC, TCIOFLUSH );
            DebugOut( "***************************************************\n" );
            return ErrRet( sRetVal );
        }
        else
        {
            DebugOut( "\n    [STX  ����]����[%02x]", abTmpBuff[0] );
        }

        /*
         * ������ �ּ� ���Ž� STX ���ſ��� ���� 
         */
        if ( abTmpBuff[0] != 0x02 )
        {
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush ( nfdMSC, TCIOFLUSH );
            DebugOut( "STX �����߻� [%02x]\n", abTmpBuff[0] );
            return ErrRet( ERR_MAINSUB_COMM_STX_IN_PKT );    
        }

        /*
         * 2. Length ���� - 2 byte//////////////////////////////////////////////
         */
        sRetVal = CommRecv( nfdMSC, &abTmpBuff[1], 2, nTimeOut );

        if ( sRetVal < 0 )
        {
            DebugOut( "\n    sRetVal 2 [%d]", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush ( nfdMSC, TCIOFLUSH );
            DebugOut( "\n    [LEN  ����]����\n" );
            DebugOut( "***************************************************\n" );
            return ErrRet( sRetVal );

        }
        else
        {
          DebugOut( "\n    [LEN  ����]����[%02x][%02x]",
                    abTmpBuff[1], abTmpBuff[2] );
        }

        /*
         * 3. Command ���� - 1 byte/////////////////////////////////////////////
         */
        sRetVal = CommRecv( nfdMSC, &abTmpBuff[3], 1, nTimeOut );	
        if ( sRetVal != SUCCESS )
        {
            DebugOut( "\n    sRetVal 3 [%d]", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush( nfdMSC, TCIOFLUSH );
            DebugOut( "\n    [CMD  ����]����(NegativeValue)\n" );
            DebugOut( "***************************************************\n" );
            return ErrRet( sRetVal );
        }


        /*
         * Command ���� �� ����  
         */            
        sRetVal = IsCmdValidCheck( abTmpBuff[3] );
		if ( sRetVal != SUCCESS )
		{
            DebugOut( "\n    sRetVal 3 [%d]", sRetVal );
            DebugOut( "\n    [CMD  ����]����(ValidationFail)[ch:%c][hex:%x]\n",
                       abTmpBuff[3],abTmpBuff[3] );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush( nfdMSC, TCIOFLUSH );
            DebugOut( "***********************************************\n" );
            return ErrRet( sRetVal );
        }
		
        /*
         * C command���� üũ 
         */ 
		if ( abTmpBuff[3] == MAIN_SUB_COMM_CHK_CRC )
		{
			/*
			* C Command�� ���� ��� �ٽ� �ѹ� ������ �����ϵ��� flagó��
			*/
		    DebugOut( "\n    [CMD  ����]����[%c]", abTmpBuff[3] );
            boolIsCcmdRecv = TRUE;
		}
		else
		{
			DebugOut( "\n    [CMD  ����]����[%c]", abTmpBuff[3] );
		    boolIsCcmdRecv = FALSE;
		}
		
        /*
         * 4. �ܸ��� ��ȣ(devNo) ���� - 1byte///////////////////////////////////
         */         
        sRetVal = CommRecv( nfdMSC, &abTmpBuff[4], 1, nTimeOut );
		
        /*
         * ���ŵ� �ܸ����ȣ�� 1,2,3�� �ƴ� ��쿡�� ����ó��
         */ 
        if ( sRetVal < 0  ||  abTmpBuff[4] < '1' || abTmpBuff[4] > '3' )
        {
            DebugOut( "\r\n sRetVal 4 [%d]", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush( nfdMSC, TCIOFLUSH );
            DebugOut( "\n    [devNo����]����\n" );
            DebugOut( "***************************************************\n" );
            return ErrRet( sRetVal );
        }
		
		/* 
		 * �ܸ��� ��ȣ ����
		 */
        if ( gboolIsMainTerm == TRUE  )
		{
		 	/* �����ܸ����� ��� - ���� �۽��� �ܸ����ȣ�� �� */
            if( nDevNo != abTmpBuff[4]-'0' ) 
            {
	            LogTerm("[DevNo����]�������� ���� [%02x]\n", abTmpBuff[4] );
	            DebugOut("[DevNo����]�������� ���� [%02x]\n", abTmpBuff[4] );
	            sRetVal1 = ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT;
            }
        }
        else
        {
       		/* �����ܸ����� ��� - �� �ܸ��� ��ȣ�� �� */
 			if ( abTmpBuff[4]-'0' != gbSubTermNo ) 
 			{
 				DebugOut( "    [DevNo����]Ÿ ��ȣ[%02x]\n", abTmpBuff[4] );
            	//tcflush( nfdMSC, TCIOFLUSH );
            	//DebugOut( "    [���� ���]Ÿ �������ȣ\n" );
            	//DebugOut( "***************************************************\n" );
            	sRetVal1 = ERR_MAINSUB_COMM_NOT_MINE_PKT;	
 			}
        }
        DebugOut( "\n    [DevNo����]����[%c�� �ܸ���]", abTmpBuff[4] );
        

        /*
         * �� �������ݷ� �߿�� �ٿ�ε� �Ϸ��� ��ɾ����� Ȯ���ϱ� ����
         * �� ���������� Data�κ� ���� �������� ���ŵ����� ���̸� ���.
         * �� ���������� STX LEN LEN CMD DEVNO DATA ETX CRC CRC CRC CRC.
         * �� SendPkt���� �����ͻ���� +2�� ���� �ֱ� ������ �����ϴ� �ʿ�����
         * + 8�� �ؼ� ��ü ���ŵ����� ���̷� �����.
         * 
         */
        nRecvDataLen = ( ( abTmpBuff[1] << 8 ) | abTmpBuff[2] ) + 8;
        /*
         * �����⿡���� ������������ �̿��Ͽ� �����ؾ��ϴ��� ���θ� 
         * boolIsOldProtocol�� �����ϰ�
         * �����⿡���� ������ ���̰� 40�̰� ����� MAIN_SUB_COMM_SUB_TERM_IMG_VER
         * �Ǵ� ����� MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD�� ���� �����Ѵ�.
         */			
		if ( ( boolIsOldProtocol== TRUE && gboolIsMainTerm == TRUE ) ||
			 ( 
		       (( nRecvDataLen == 40 && abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) ||
                ( abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD ))
                &&
               ( gboolIsMainTerm == FALSE )
             )
           )
        {
            /* ������� �Ź��� ������ ��� �ʱ⿡ ���������ݷ� �߿������ üũ
             * ��ɾ ������� �����µ� �����⿡����  
			 * RecvPkt()��� ���Ž� �Ʒ��κп���  TRUE�� �����ϰ� �ȴ�.
			 * ������ ���α׷� ������ �����ϴ� �Լ��� SubRecvImgVer �����
			 * boolIsMainTermPreVer�� TRUE�� ��쿡�� ������� ������ ������ 
			 * ���� ���Ͽ� �������� Rollback ������ �Ǵ��ϰ� �ȴ�.
			 */
            boolIsRespVerNeedByOldProtocol = TRUE;
			/* old protocol�󿡼��� �� size */
            nRecvBuffLen = nRecvDataLen;    

	        /*
	         * ���Ź��� ���� ����  
	         */
	        if ( nRecvBuffLen > MAX_PKT_SIZE_OLD )
	        {
	            DebugOut( "\r\n data size error [%d]", nRecvBuffLen );
	            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
	            tcflush ( nfdMSC, TCIOFLUSH );
	            DebugOut( "    [LENGTH ����]����\n" );
	            DebugOut( "***************************************************\n" );
	            return ErrRet( ERR_MAINSUB_COMM_INVALID_LENGTH );
	        }			
        }
        else
        {
            boolIsRespVerNeedByOldProtocol = FALSE;
		   /*
			* ������������ STX LEN LEN CMD DEVNO SEQ SEQ DATA ETX CRC CRC CRC CRC. 
			* DATA�κ��� �����ϸ� 12byte�� �� �����Ƿ� ��ü ���۱��̴� 
			* �����ͱ��� + 12�� �ϰ� �ȴ�.
			*/
            nRecvBuffLen = ( (abTmpBuff[1] << 8) | abTmpBuff[2] ) + 12;
		   /*
	         * ���Ź��� ���� ����  
	         */
	        if ( nRecvBuffLen > MAX_PKT_SIZE )
	        {
	            DebugOut( "\r\n data size error [%d]", nRecvBuffLen );
	            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
	            tcflush ( nfdMSC, TCIOFLUSH );
	            DebugOut( "    [LENGTH ����]����\n" );
	            DebugOut( "***************************************************\n" );
	            return ErrRet( ERR_MAINSUB_COMM_INVALID_LENGTH );
	        }		   
        }

        memcpy( abRecvBuff, abTmpBuff, 5 );

        /*
         * 5. ������ ��Ŷ(SEQ/������/ETX/CRC)����///////////////////////////////   
         */
        sRetVal = CommRecv( nfdMSC, &abRecvBuff[5], nRecvBuffLen-5, nTimeOut );
		/*
         * ������ �κ� ���� �� ����    
         */
        /*
         * 1) �ڽ��� ��Ŷ�� �ƴ� ��� return
         *    �ڽ��� ��Ŷ������ DevNo�� �����Ͽ� �����ϸ� �˼� ������
         *    ���ۿ� �����͸� ������ �ʱ� ���� ��� �����͸� �������Ŀ�
         *    �񱳸� ���ְ� �ȴ�.
         *    �׷��� DevNo���Ž� �ڱ� ��Ŷ�� �ƴϸ� ������� 
         *    ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT, ������� ERR_MAINSUB_COMM_NOT_MINE_PKT
         *    �����ڵ带 ������ ������ ���⼭ ���ϰ� �Ǵ� ����.    
         */
        if ( sRetVal1 == ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT )
        {
        	/* �����ܸ����� ��� - �۽��� �ܸ��⿡�� ������ �ܸ��Ⱑ �ƴ� 
        	 * �ٸ� �ܸ��⿡�� �����Ͱ� ���ŵ� ��� -�������� �����ڵ� ����
        	 */ 
            printf( "    [DATA ����]ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT\n" );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush( nfdMSC, TCIOFLUSH );
            return ErrRet( sRetVal1 );
        } 
        else if ( sRetVal1 == ERR_MAINSUB_COMM_NOT_MINE_PKT )
        {
        	/* �����ܸ����� ��� - ��������Ŷ�� �ڽ��� �ܸ����ȣ�� Ʋ�� ���� 
        	 * �ڽ��� ������ ��Ŷ�� �ƴѰ��� - Not Mine Pkt �����ڵ� ����
        	 */             
            return ErrRet( sRetVal1 );
        }
		
        /*
         * 2) ������ �κ� ���Ž� ����ó��
         */
        if ( sRetVal < 0 )
        {
            printf( "\r\n sRetVal 5 [%d]", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof( stRecvPkt ) );
            tcflush ( nfdMSC, TCIOFLUSH );
            printf( "    [DATA ����]����\n" );
            printf( "***************************************************\n" );
            return ErrRet( sRetVal );
        }
        DebugOut( "\n    [DATA ����]����-����:%d\n", nRecvBuffLen-5 );


        /*
         *  Sequence ���� - ������������ ��쿡�� üũ 
         */    
		if ( ( boolIsOldProtocol== TRUE && gboolIsMainTerm == TRUE ) ||
			 ( 
		       (( nRecvDataLen == 40 && abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) ||
                ( abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD ))
                &&
               ( gboolIsMainTerm == FALSE )
             )
           )
        {
            DebugOut( "    [SeqNo üũ] Skip!\n" );
        }
        else
        {
            memcpy( ( byte*)&wSeqNo, &abRecvBuff[5], 2 );
            DebugOut( "[���� SEQ] %d �� ��Ŷ", wSeqNo );
	        /*
	         *  Sequence Range üũ 
	         */			
            if ( wSeqNo > 9999 )
            {
                DebugOut( "[SEQ ����]SEQ �����߻�\n" );
                return ErrRet( ERR_MAINSUB_COMM_SEQ_IN_PKT );
            }
            else
            {
	        /*
	         *  Sequence�� ������Ŷ����ü�� ����
	         */	            
                stRecvPkt.wSeqNo = wSeqNo;
            }

        }
        /*
         *  ETX üũ
         */ 
        if ( abRecvBuff[nRecvBuffLen-nEtxPoint] != ETX )
        {
        	///printf( "etx 1- %x\n",  abRecvBuff[nRecvBuffLen-nEtxPoint]);
			///printf( "etx 2- %x\n",abRecvBuff[nRecvBuffLen-nEtxPoint+1]);
			///printf( "etx -1- %x\n",abRecvBuff[nRecvBuffLen-nEtxPoint-1]);
			
            memset( &stRecvPkt, 0x00, sizeof( stRecvPkt ) );
            DebugOut( "\n    [DATA ����]ETX �����߻� \n" );
            tcflush ( nfdMSC, TCIOFLUSH );
            DebugOut( "***************************************************\n" );

            return ErrRet( ERR_MAINSUB_COMM_ETX_IN_PKT );    
        }

        /*
         * CRC üũ
         */       
        dwCommCRC = MakeCRC32( abRecvBuff, nRecvBuffLen-4 );

        if ( memcmp( &abRecvBuff[nRecvBuffLen-4], ( byte*)&dwCommCRC, 4 ) != 0 )
        {
            int i = 0;

            for ( i = 0 ; i < nRecvBuffLen ; i++ )
            {
                DebugOut( "%02x ", abRecvBuff[i] );
            }

            DebugOutlnBCD( "    [DATA ����]CRC �����߻� ���� CRC     ==> ",
                           &abRecvBuff[nRecvBuffLen-4], 4 );
            DebugOutlnBCD( "    [DATA ����]CRC �����߻� �����  CRC ==> ",
                           ( byte*)&dwCommCRC, 4 );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            DebugOut( "\n    [DATA ����]CRC �����߻� \n" );
            tcflush( nfdMSC, TCIOFLUSH );

            return ErrRet( ERR_MAINSUB_COMM_CRC_IN_PKT );
        }

    }
    while ( boolIsCcmdRecv == TRUE );

    /*
     * ���� data ����ü�� �� ����
     */      
    stRecvPkt.bCmd = abRecvBuff[3];                /* ��ɾ� ���� */
    stRecvPkt.bDevNo = abRecvBuff[4] - '0';		   /* �ܸ����ȣ ���� */
    /*
     * �����ͻ����� ��ŭ �����͸� ���� ������ ����ü�� ����
     */ 
    if ( ( boolIsOldProtocol == TRUE && gboolIsMainTerm == TRUE ) ||
         ( ( nRecvDataLen == 40 && abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) &&
           ( gboolIsMainTerm == FALSE ) )
       )
    {
        /* 
         *������������ ������ ���۱��̿��� -10���ذ��� ���� ������ ������
		 * STX(1) LEN(2) LEN(3) CMD(4) DEVNO(5) DATA ETX(6) CRC(7)CRC(8)CRC(9)CRC(10)
		 */
        stRecvPkt.nDataSize = nRecvDataLen - 10;

        if ( stRecvPkt.nDataSize != 0 )
        {
        	/* 
	         * 6 byte���� ���ŵ� �������̹Ƿ� 
			 */        
            memcpy( stRecvPkt.abData, &abRecvBuff[5], stRecvPkt.nDataSize );
        }
    }
	else if (( abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD )&&
		     ( gboolIsMainTerm == FALSE )
		    )
	{
	   /* 
	    * �� ���������� �̿��Ͽ� �����⿡�� �߿���ٿ�ε带 �ϴ� ��쿡��
	    * �ִ���Ŷ�������� ���̰� �־� stRecvPkt.abData��� abRecvData�� 
	    * ����Ѵ�.
	    */
		stRecvPkt.nDataSize = nRecvDataLen - 10;		
		if ( stRecvPkt.nDataSize != 0 )
        {
            memcpy( abRecvData, &abRecvBuff[5], stRecvPkt.nDataSize );
	    }   
    }
    else
    {
		/* 
         * �� ���������� ������ ���۱��̿��� -12 ���ذ��� ���� ������ ������
		 * STX(1) LEN(2) LEN(3) CMD(4) DEVNO(5) SEQ(6) SEQ(7) DATA ETX(8) CRC(9)CRC(10)CRC(11)CRC(12)
		 */    
        stRecvPkt.nDataSize = nRecvBuffLen - 12 ;

        if ( stRecvPkt.nDataSize != 0 )
        {
        	/* 
	         * 8 byte���� ���ŵ� �������̹Ƿ� 
			 */
            memcpy( stRecvPkt.abData, &abRecvBuff[7], stRecvPkt.nDataSize );
        }
    }

    return sRetVal;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendFile                             				       *
*                                                                              *
*  DESCRIPTION:       ������ �����Ѵ�.   								       *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo- �ܸ����ȣ 				                   *
*                     char* pchFileName - �۽Ŵ�� ���ϸ� 				       *
* 																			   *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                     ERR_MAINSUB_COMM_FILE_NOT_FOUND_EOT_SEND                 *
*                      - EOT������ ���� 		                        	   *
*                     ERR_MAINSUB_COMM_FILE_NOT_FOUND                          *
*                      - ��û���� ������                                       *
*                     ERR_MAINSUB_COMM_PARM_DOWN_NAK                           *
*                      - ������Ŷ���ۿ� ���� NAK����                           *
*                     ERR_MAINSUB_COMM_FILE_NOT_FOUND_RECV                     *
*                      - EOT���� �� ���ſ���                                   *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:       	 �۽��� ������ �������� �ʴ� ��� EOT ����                 *
*                    �۽��� ������ �����ϴ� ��쿡�� ������ ������ ��Ŷ��      *
*                    ������ ��Ŷ�� �ǹ̷� EOT�� ����                           *
*                    �� ��Ŷ ���۸��� ACK�μ� ��Ŷ���ۿϷḦ Ȯ���Ѵ�.         *
*																			   *
*******************************************************************************/
short SendFile( int nDevNo, char* pchFileName )
{
    short sRetVal = SUCCESS;
    int fdFile;
    bool boolIsFileEnd = FALSE;   /* ������ ���� ��Ÿ���� Flag */         
	int nByte;					  /* �۽����Ͽ��� ���� Byte�� */
    word wSeqNo = 0;			  /* Sequence ��ȣ */
	bool boolIsEOTUse = FALSE;
    usleep( 100000 );

	/*
	 * ������ ���� ����
	 */
	fdFile = open( pchFileName, O_RDONLY, OPENMODE );

    if ( fdFile < 0 )
    {
        printf( "\r\n ���� ���� �ȵ�\r\n" );
	   /*
	   	* TD���� ������ �ƴ� ��쿡�� ���� ������ �ȵǸ� 
	   	*/
		if ( bCurrCmd != MAIN_SUB_COMM_GET_TRANS_FILE )
		{
			return -1;
		}
	   /*
	   	* ������ ������ ������ �ȵǴ� ��쿡�� ������ �������� �ʴ� ���� ����
	   	* ���������� ��û�� �ʿ� EOT�� ������ �ȴ�. 
	   	*/
   		/* Cmd(Ŀ�ǵ�) : �����⿡�� ������ Ŀ�ǵ� */
        stSendPkt.bCmd = bCurrCmd; 
		/* bDevNo(�ܸ����ȣ) : �ܸ����ȣ + ASCII 0  */ 
        stSendPkt.bDevNo = nDevNo + '0';
		/* wSeqNo(������ ��ȣ) : 0  */ 
        stSendPkt.wSeqNo = 0;   
		/* nDataSize(�����ͻ�����) : 1  */ 		
        stSendPkt.nDataSize = 1;
		/* abData(������) : �����ͺκп� EOT  */ 		
        stSendPkt.abData[0] = EOT;

		boolIsEOTUse = TRUE;
	   /*
	   	* EOT���� 
	   	*/
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            printf( "\r\n  ���� �������� �ʾ� EOT Send �� Error \r\n" );
            sRetVal =  ERR_MAINSUB_COMM_FILE_NOT_FOUND_EOT_SEND;
            return ErrRet( sRetVal );
        }
	   /*
	   	* EOT���ۿ� ���� �������
	   	*/
        sRetVal = RecvPkt( 3000, nDevNo );
        if ( sRetVal < 0 )
        {
            printf( "\r\n  ���� �������� �ʾ�  EOT Send �� Recv Error \r\n" );
            sRetVal =  ERR_MAINSUB_COMM_FILE_NOT_FOUND_RECV;
            return ErrRet( sRetVal );
        }
	   /*
	   	* EOT���ۿ� ���� �������� ACK �Ǵ� NAK ���� 
	   	*/
        if ( ( stRecvPkt.bCmd == ACK ) && ( boolIsEOTUse == TRUE ) )
        {
            printf( "\r\n ���� �������� �ʾ� EOT Send�� ACK ���ſϷ�!\r\n" );
            sRetVal =  ERR_MAINSUB_COMM_FILE_NOT_FOUND;
            return ErrRet( sRetVal );
        }
        else if ( stRecvPkt.bCmd == NAK )
        {
            printf( "NAK ����\n" );
            sRetVal =  ERR_MAINSUB_COMM_PARM_DOWN_NAK;
            return ErrRet( sRetVal );
        }

    }
   /*
   	* �������� Loop ���� 
   	*/
    while( TRUE )
    {
    	printf( "." );						// ���������� �ֿܼ� ǥ���Ѵ�.
    	fflush( stdout );

		/*
		 * ������ ���� �ִ� 1024byte���� �����Ϳ����� �б�  
		 */    
        nByte = read( fdFile, stSendPkt.abData, DOWN_DATA_SIZE );
	   /*
	   	* �о� ���� �����Ͱ� �����ϴ� ��� ��Ŷ ����
	   	*/  
	   	if ( nByte > 0 )
        {
            /* Cmd(Ŀ�ǵ�) : ���� �ܸ��Ⱑ �������� Ŀ�ǵ� */
            stSendPkt.bCmd = bCurrCmd; 
 			/* bDevNo(�ܸ����ȣ) : �ܸ����ȣ + ASCII 0   */			
            stSendPkt.bDevNo = nDevNo + '0'; 
			/* nDataSize(�����ͻ�����) : ���Ͽ��� ���� byte  */ 					
            stSendPkt.nDataSize = nByte;     
			/* wSeqNo(������ ��ȣ) : ��Ŷ�� ������ ��ȣ  */ 
            stSendPkt.wSeqNo = wSeqNo;        
        }
	   /*
	   	* �о� ���� �����Ͱ� 0�� ��� ��, ������ ���ΰ�� EOT ��Ŷ ����
	   	*/  		
        else if ( nByte == 0 ) 
        {
            DebugOut( "ȭ�ϳ����� ������ %d \n", nByte );
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

	   /*
	   	* ������Ŷ ����
	   	*/ 	
        sRetVal = RecvPkt( 3000, nDevNo );
        if ( boolIsFileEnd == TRUE )
        {
            DebugOut( "\n[RecvPkt-EOT����-from %d�� �ܸ���]", nIndex+  1 );
        }
        else
        {
            DebugOut( "\n[RecvPkt-from %d�� �ܸ���]", nIndex + 1 );
        }
		
        if ( sRetVal < 0 )
        {
            DebugOut( "\r\n ���� ������ �������[%d]\r\n", sRetVal );
            break;
        }
	   /*
	   	* ���� ���ۿϷ� 
	   	* - ������ ���ۿϷᰡ �Ǿ����� �Ǵ��ϴ� �������δ� ���ϳ��� �����ϰ�
	   	*   ACK�� ���ŵ� ����� �Ѵ�. 
	   	*/ 
        if ( ( stRecvPkt.bCmd == ACK ) && ( boolIsFileEnd == TRUE) )
        {
            printf( "/\n" );					// ���ۿϷḦ �ֿܼ� ǥ���Ѵ�.
            sRetVal = SUCCESS;
            break;
        }
	   /*
	   	* ���� ������ NAK �������� ����
	   	*/ 		
        else if ( stRecvPkt.bCmd == NAK )
        {
            sRetVal = ERR_MAINSUB_COMM_PARM_DOWN_NAK;
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

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RecvFile                             				       *
*                                                                              *
*  DESCRIPTION:       ������ �����Ѵ�.   								       *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo- �ܸ����ȣ 				                   *
*                     char* pchFileName - ������Ŷ�� ������ ���ϸ�             *
* 																			   *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                     ERR_MAINSUB_COMM_SEQ_DURING_RECV_FIL                     *
*                      - ���ϼ��Ž� ������ ����                         	   *
*                     ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST                      *
*                      - ��û���� ������                                       *
* 																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:       	 �������ۿ�û�� ���� �ܸ��⿡���� ������ ���� ��� ������  *
*                    EOT�� ������ �ǰ� ������ ������ ��Ŷ���� EOT�� ������ �ȴ�*
*					 ���̴� �������ۿ�û�� ù ������ EOT�� ���� ��û������     *
*                    �������� �ʴ� ���̸� ù ���� ���� EOT�� ���� ����������   *
*                    �Ϸ�Ǿ����� �ǹ��Ѵ�.  								   *
*                    �����ϸ�, 												   *
*                    ���������� ��û�� �ܸ��⿡���� EOT�� ���ŵ� ��쿡��      *
*                    ù ��Ŷ���ſ� EOT�� �� ���- ��û�� ������ ���ٴ� �ǹ�    *
*                    ù ��Ŷ�������� EOT�� �� ���- ������ �Ϸ� �Ǿ��ٴ� �ǹ�  *
*																			   *
*******************************************************************************/
short RecvFile( int nDevNo, char* pchFileName )
{
    int fdFile;
    short sRetVal = SUCCESS;
    int nRecvCnt = 0;         /* ��Ŷ���� Count */
    word nPreSeqNo = 0;		  /* ���� Sequence ��ȣ */

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
    	printf( "." );						// ���������� �ֿܼ� ǥ���Ѵ�.
    	fflush( stdout );

	   /*
	    * ��Ŷ����
	    */
        sRetVal = RecvPkt( 8000, nDevNo );

        if ( sRetVal < 0 )
        {
            printf( "/r/n file recv Error 1: [%x] /r/n", sRetVal );
            break;
        }
	   /*
	    * ���� ��Ŷ�� Sequence Check
	    * ���� ���Ž������� ���� ��Ŷ������ + 1���� ����, 0�� �ƴ��� üũ
	    */
        if ( ( stRecvPkt.wSeqNo - nPreSeqNo != 1 ) && ( stRecvPkt.wSeqNo != 0 ) )
        {
             DebugOut( "��Ŷ Sequence Error!! ���ſ�����Ŷ��ȣ : %d ��, ",
                       nPreSeqNo+1 );
             DebugOut( "����������Ŷ��ȣ : %d �� ", stRecvPkt.wSeqNo );
             sRetVal = ERR_MAINSUB_COMM_SEQ_DURING_RECV_FILE;
             break;
        }

	   /* 
	    * ��Ŷ���� Count ���� - ù ��Ŷ ������ �����ϱ� ����
	    */
        nRecvCnt++;
	   /* 
	    * ��Ŷ���Ž� ACK������ ���� ������ ���� 
	    */
        stSendPkt.bCmd = ACK;
        stSendPkt.bDevNo = nDevNo + '0';
        stSendPkt.nDataSize = 0;
        stSendPkt.wSeqNo = 0;
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
        if ( ( stRecvPkt.nDataSize == 1 ) && ( stRecvPkt.abData[0] == EOT ) )
        {
            if ( nRecvCnt == 1 )  // YYYYMMDDHHMMSS.tmp
            {
                /* ù ��Ŷ������ ���� EOT�� �ǹ̴� ��û���� ���ٴ� ������ */
                DebugOut( "\r\n[����]�����ܸ��⿡ ��û�� ������ �����ϴ�.\r\n" );
                sRetVal = ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST;
            }
            else
            {
                /* ù ��Ŷ������ �ƴѰ���  EOT�� �ǹ̴� �������ۿϷ� */
                DebugOut( "\r\n[����]���ۿϷ�\r\n" );
            }
            break;
        }
	    /* 
	     *  ��Ŷ���ŵ����� ���Ͽ� ���� 
	     */
        write( fdFile, stRecvPkt.abData, stRecvPkt.nDataSize );
		/* 
	     *  ���� ��Ŷ���Ž� ������ üũ�� ���� ���� �������� ����
	     */
        nPreSeqNo = stRecvPkt.wSeqNo;
    }

    close( fdFile );

	if ( sRetVal == SUCCESS )
		printf( "/" );						// ���ۿϷḦ �ֿܼ� ǥ���Ѵ�.
	else
		printf( "e\n" );

    return sRetVal;
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateACK                             				   *
*                                                                              *
*  DESCRIPTION:       ACK�����͸� �����Ѵ�.								       *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo- �ܸ����ȣ 				                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:       	  �� �������ݰ� �� �������ݿ� ���� ���� ACK ������ �Ҽ�    *
*                     �ֵ��� ����. 											   *
*																			   *
* 					  04xx������ �����α׷��� �����⿡�� ������� ��������     *
*                     ������ Ȯ���ϰ� �����α׷����� ������Ʈ�ϱ�����          *
*                     UpdateSubTermImg()�Լ��� �����ϰ� �Ǵµ� �� �Լ�������   *
*                     ���������ݷ� ����Ȯ�θ�ɾ�('V')�� ������� ������ �ȴ�. *
*                     �̰�� �����⿡���� nDataSize�� 30 �̰� Cmd ��           *
*                     MAIN_SUB_COMM_SUB_TERM_IMG_VER�ΰ��� ���� �� ��Ŷ��      *
*                     UpdateSubTermImg()�Լ��� ���������� �Ǻ� ���������ݷ�    *
*                     ������ ���ְ� �ȴ�. 									   *
*                     														   *
*                     UpdateSubTermImg()�Լ��� ����Ȯ�θ�ɾ�('V')�� ������    *
*                     �ƴѰ�쿡�� ��, nDataSize�� 30�� �ƴϰų� ��ɾ      *
*                     MAIN_SUB_COMM_SUB_TERM_IMG_VER�� �ƴѰ���              *
*                     ���������ݷ� ACK�� ������ �ȴ�.      					   *                                              * 
*                                                                              *
*******************************************************************************/
short CreateACK( int nDevNo )
{
    short sRetVal = SUCCESS;

	/* 
	 * �� �������ݷ� ������ ��� ���������ݿ� �°� ������Ŷ���� 
	 */
    if ( boolIsRespVerNeedByOldProtocol == TRUE )
    {
   		/* Cmd(Ŀ�ǵ�) : �����⿡�� ������ Ŀ�ǵ� */
        stSendPkt.bCmd = stRecvPkt.bCmd;     
		/* bDevNo(�ܸ����ȣ) : �ܸ����ȣ + ASCII 0  */ 
        stSendPkt.bDevNo = stRecvPkt.bDevNo + '0';
		/* wSeqNo(������ ��ȣ) : 0  */ 
        stSendPkt.wSeqNo = 0;
		/* nDataSize(�����ͻ�����) : 1  */ 		
        stSendPkt.nDataSize = 1;
		/* abData(������) : �����ͺκп� ACK  */ 		
        stSendPkt.abData[0] = ACK;
		/* ������ SendPkt()�� ���������ݷ� ����ϵ��� Flag Set */ 		
		boolIsOldProtocol = TRUE;		
    }
    else
    {
		/* 
		 * �� �������ݷ� ������ ��� ���������ݿ� �°� ������Ŷ���� 
		 * �� �������ݰ� �ٸ� ���� Ŀ�ǵ�κп� ACK�� ����ϰ� 
		 * ���� �����ͻ���� �������� ũ�⸦ ��Ÿ���ٴ� ���̴�.
		 * ACK������ ��� �����Ͱ� �����Ƿ� �����ͻ���� 0 ��.
		 */   
	 	/* bCmd(Ŀ�ǵ�) : Ŀ�ǵ�κп� ACK  */ 
        stSendPkt.bCmd = ACK;
		/* bDevNo(�ܸ����ȣ) : �ܸ����ȣ + ASCII 0  */ 		
        stSendPkt.bDevNo = nDevNo + '0';
		/* wSeqNo(������ ��ȣ) : 0  */ 		
        stSendPkt.wSeqNo = 0;
		/* nDataSize(�����ͻ�����) : 0  */ 			
        stSendPkt.nDataSize = 0;
		/* ������ SendPkt()�� ���������ݷ� ����ϵ��� Flag Set */ 		
		boolIsOldProtocol = FALSE;	
    }

    return ErrRet( sRetVal );
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateNAK                                    		       *
*                                                                              *
*  DESCRIPTION:       NAK���䵥���͸� �����Ѵ�.				                   *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo - �ܸ��� ��ȣ    		                       *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short CreateNAK( int nDevNo )
{
    short sRetVal = SUCCESS;
   /*
	* �۽� ��Ŷ����ü�� �� ���� 
	*/
	/* bCmd(Ŀ�ǵ�) : Ŀ�ǵ�κп� NAK  */ 	
    stSendPkt.bCmd = NAK; 	
	/* bDevNo(�ܸ����ȣ) : �ܸ����ȣ + ASCII 0 */    
    stSendPkt.bDevNo = nDevNo + '0';	
	/* Sequence No */
    stSendPkt.wSeqNo = 0;	
	/* Data Size */	
    stSendPkt.nDataSize = 0;			

    DebugOut( "create NAK \n" );

    return ErrRet( sRetVal ); 
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CloseMainSubComm                                         *
*                                                                              *
*  DESCRIPTION:       �������� ���ä���� �ݴ´�. 	    				       *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short CloseMainSubComm( void )
{
    close( nfdMSC );
    return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsCmdValidCheck                            		       *
*                                                                              *
*  DESCRIPTION:       ������κ��� ������ �������� Command�κ��� �����Ѵ�.     *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  byte  bCmd - ������ Command                              *
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
short IsCmdValidCheck( byte bCmd )
{
    short sRetVal = SUCCESS;

    switch ( bCmd )
    {
        case MAIN_SUB_COMM_SUB_TERM_IMG_VER :
        case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN :
		case MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD:
        case MAIN_SUB_COMM_GET_SUB_TERM_ID :
        case MAIN_SUB_COMM_SUB_TERM_PARM_DOWN :
        case MAIN_SUB_COMM_SUB_TERM_VOICE_VER :
        case MAIN_SUB_COMM_SUB_TERM_VOIC_DOWN :
        case MAIN_SUB_COMM_REGIST_KEYSET :
        case MAIN_SUB_COMM_POLLING :
        case MAIN_SUB_COMM_CHK_CRC :
        case MAIN_SUB_COMM_REQ_PL_SEARCH :
        case MAIN_SUB_COMM_REQ_BL_SEARCH :
        case MAIN_SUB_COMM_GET_TRANS_CONT :
        case MAIN_SUB_COMM_ASSGN_SUB_TERM_ID :
        case MAIN_SUB_COMM_GET_TRANS_FILE :
        case ACK :
        case NAK :
        case ETB : return ErrRet( sRetVal );
        default  : return -1;
    }
}




