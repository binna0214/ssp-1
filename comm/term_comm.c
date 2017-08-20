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
*  DESCRIPTION:       승하차간의 통신을 처리하는 메인프로세스                    *
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
* 2006/04/17 F/W Dev. Team  wangura       파일분리 및 추가 구조화              *
*                                                                              *
*******************************************************************************/
/*  승하차간의 펌웨어 다운로드 구성 //////////////////////////////////////////// 
*																			   *	
* 	신펌웨어에서는 버스 승하차간의 통신프로토콜의 변경으로 인해 승하차간의     *
* 	현재 사용하는 프로토콜이 구인지 신인지 판별하여 구프로토콜인 경우 신펌웨어 *
*   를 다운로드하여 업그레이드 한다. 신프로토콜을 사용하는 경우에는 승하차간의 *
*   처리함수 MainTermProc()/SubTermProc()이 실행되면 버전이 낮을 경우 다운로드 *
* 	가 당연히 된다.                                                            *
*                                                                              *
*	지원방법 )                                                                 *
*	승차기 처리함수(MainTermProc)시작전에 하차기 구프로토콜 사용확인 및        *
*	신프로토콜 펌웨어로 다운로드하는 함수인 UpdateSubTermImg함수를             *
*	가장 먼저 실행하게 하여 처리한다.                                          *
*                                                                              *
*	                                                                           *
*	UpdateSubTermImg함수는 구프로토콜의 형태로 하차기 버전체크 명령어를 보내게 *
*	된다.                                                                      *
*	( 참고.                                                                    *
*	   1) 구프로토콜을 사용하는 펌웨어는 버전체크(V) 명령어 전송시 40 byte를   *
*	      전송하기 되는데                                                      *
*          STX LEN LEN CMD DEVNO 데이터(30b) ETX CRC CRC CRC CRC               *
*	      데이터부분에는 DEVNO(1b),CSAMID(4b),PSAMID(8b),승차단말기ID(9),      *
*         0x00(8b)로 구성                                                      *
*	                                                                           *
*       2) 신펌웨어에서는 UpdateSubTermImg내 ChkSubTermImgVerOldProtcol() 함수 *
*          에서 아래와 같이 구프로토콜을 사용하는 것처럼 동일하게 패킷을 구성  *
*          하되 데이터에 0x00(8b)부분에 승차기버전(4b)을 넣어서 전송하게 된다. *
*	      STX LEN LEN CMD DEVNO 데이터(30b) ETX CRC CRC CRC CRC                *
*	      데이터부분에는 DEVNO(1b),CSAMID(4b),PSAMID(8b),승차단말기ID(9),      *
*         승차기버전(4b),0x00(4b)로 구성.                                      *
*	)                                                                          *
*	                                                                           *
*	하차기에서는                                                               *
*	1)구프로토콜을 사용하는 펌웨어는 자신의 버전 4byte를 응답으로 전송         *
*	2)신프로토콜을 사용하는 펌웨어는 버전 4byte이외에 "new"라는 3byte를        *
*	  추가로 전송함으로서 자신이 신프로토콜을 사용하는 펌웨어임을 승차기에     *
*     알리게 된다.                                                             *
*                                                                              *
*	                                                                           *
*	승차기가 구프로토콜을 사용하는 펌웨어로 판명이 되면 구프로토콜을 이용하여  *
*	신프로토콜을 사용하는 펌웨어를 전송함으로서 업그레이드가 완료 된다.        *
*                                                                              *
*	승차기가 신프로토콜을 사용하는 펌웨어로 판명되면 UpdateSubTermImg함수를    *
*   종료하고 승차기처리함수(MainTermProc)내에서 버전을 비교하여 업그레이드가   *
*   되도록 정상적인 프로세스를 통해 처리하도록 한다.                           *
*                                                                              *
*	반면 승차기가 구프로토콜을 사용하는 펌웨어이고 하차기가 신프로토콜을 사용  *
*   하는 펌웨어인경우                                                          *
*	( 초기배포시 문제가 발생하여 구프로토콜을 사용하는 03xx대의 펌웨어를       *
*	버전만 04xx로 해서 승차기에 배포한 경우에 발생한다)                        *
*    버전체크 명령어를 신프로토콜을 사용하는 펌웨어가 수신하면 구프로토콜을    *
*    사용하는 펌웨어와 동일하게 응답할 수 있도록 신펌웨어의 하차기에 구현이    *
*    되어있으며 버전이 낮은 경우 구프로토콜을 사용하는 04xx펌웨어를 다운로드시 *
*    수신할수 있는 부분이 신펌웨어 구현되어 있어야 한다.                       *
*	                                                                           *
*                                                                              *
*	SendPkt/RecvPkt함수는 승하차기가 공통으로 사용하는 함수로                  *
*	구프로토콜에 맞게 SendPkt하기위해서 승차기에서는 boolIsOldProtocol 전역변수*
*	를 미리 설정하여 구프로토콜로 송신해야함을 알려준다.                       *
*	                                                                           *
*	구프로토콜에 맞게 RecvPkt하기위해서	하차기에서는 1)2)의 조건에 만족하는    *
*   경우 구프로토콜로 수신하게 된다.                                           *
*	1) Cmd가 'V'이고 수신데이터사이즈가 40byte인 경우 - UpdateSubTermImg()에서 *
*      하차 버전확인시                                                         *
*	2) Cmd가 'D'인 경우 - UpdateSubTermImg()에서 구프로토콜로 펌웨어 다운로드시* 
*	                                                                           *
*	이는 구/신프로토콜 사용 펌웨어간이 송수신 최대패킷사이즈가 다르기 때문이다.*
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

/* 하차기가 승차기의 버전체크명령에 대해 구프로토콜로 응답을 해야하는지 여부   */
bool boolIsRespVerNeedByOldProtocol = FALSE;
/* 패킷송신/수신시 구프로토콜을 사용하여 보내야하는지 여부 */
bool boolIsOldProtocol;  

/* old protocol에서 stSendPkt.abData대신 사용하기 위한 buffer */
 byte abSendData[1024] = { 0, };  
/* old protocol에서 stRecvPkt.abData대신 사용하기 위한 buffer */
 byte abRecvData[1048] = { 0, };  

bool boolIsMainSubCommFileDownIng;
/* 
 * 승하차 통신프로세스에서 현재 실행되고 있는 명령어 저장 변수
 */
byte bCurrCmd; 
/* 
 * 하차단말기 갯수 
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
	 * 승하차통신 포트 Open - RS422 
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
     * 단말기 승하구분에 따라 처리 
     */	
    if ( gboolIsMainTerm == TRUE ) 
    {
	    /* 
	     * 승차기인 경우 하차기가 03xx대의 구버전펌웨어인지 먼저 확인하고 
	     * 구버전일 경우 다운로드로 하차기를 업데이트 해주고 하차 처리함수 시작 
	     */	
		/* 
		 * 하차기 구버전 확인 및 구버전일경우 신버전으로 다운로드 처리 
		 */ 	     
		UpdateSubTermImg();

		/* 
		 * 승차기 처리 함수 시작
		 */
        MainTermProc(); 
    }
    else  
    {
	    /* 
	     * 하차기인 경우 
	     */          
        SubTermProc();  /* 하차기 처리 함수 시작*///////////////////////////////
    }
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       UpdateSubTermImg                                         *
*                                                                              *
*  DESCRIPTION:       하차기 구버전 확인 및 구버전일경우 신버전으로 다운로드   *
*                     를 실행한다.                                             *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행성공                                       *
*                     SendPkt,RecvPkt 함수의 에러코드를 그대로 리턴            *                      *
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
    char achNewVerYn[3] = { 0, }; /* 하차기 신프로토콜 사용여부 */
	int  nSubIDInfoExistYN = 0;

	LogTerm( "UpdateSubTermImg \n" );
        
    /* 
     * 운전자 조작기에 "작업중.." 표시 
     */    
    gpstSharedInfo->boolIsKpdLock = TRUE;

	if ( gpstSharedInfo->boolIsDriveNow == TRUE)
	{
		ctrl_event_info_write( "9001" );
	}
	
    /* 
     * SendPkt/RecvPkt 함수내에서 구프로토콜을 사용하여 송수신해야 함을 알려주는
     * Flag 설정 
     */
    boolIsOldProtocol = TRUE;
	
	/* 
	 * 하차단말기 번호 초기값 설정 
	 */
    nDevNo = 1; 
    DebugOut( "gbSubTermCnt=>[%d]\n", gbSubTermCnt );

	nSubIDInfoExistYN = access( SUBTERM_ID_FILENAME, F_OK );  // subid.dat

    /* 
     * 구프로토콜형식에 맞춰 하차기 버젼 체크 명령어 실행  
     * - 하차단말기 갯수(gbSubTermCnt)만큼 반복
     */
    while ( nDevNo <= gbSubTermCnt )
    {
    	if ( nSubIDInfoExistYN == 0 )
    	{
	        nRetryCnt = 3; /* 재시도 횟수 */
    	}
		else
		{
			nRetryCnt = 2; /* 재시도 횟수 */
		}

        while( nRetryCnt-- )
        {
#ifdef TEST_SYNC_FW        
            printf( "[%d] 번 하차기 구프로토콜사용여부 체크 시작\n", nDevNo );
#endif
			usleep( 100000 );
            sRetVal = ChkSubTermImgVerOldProtcol( nDevNo, achNewVerYn );

            if ( sRetVal != SUCCESS )
            {
                printf( "구버젼 체크 중 오류- [%x]\n", sRetVal );
            }
            else
            {
                printf( "[%d] 번 하차기 구버젼 체크 성공\n",nDevNo );
                break;
            }
        }
        /* 
         * 하차기단말기 번호 증가
         */
        nDevNo++;
    }


    nDevNo = 1;

    while ( nDevNo <= gbSubTermCnt )
    {
        nRetryCnt = 5;

        /* 
         * 하차기가 구프로토콜이면 무조건 승차기 실행이미지인 bus100
         * 을 하차기로 다운로드                      
         */             
        if ( achNewVerYn[nDevNo-1] == SUBTERM_USE_OLD_PROTOCOL )
		{
           while( nRetryCnt-- )
            {
                printf( "[%d] 번 하차기 다운로드 시도 : %d회/최대10회\n",
                          nDevNo, 5-nRetryCnt );
				/* 
				 * bus100에 날짜와 버전(18byte)를 더해 하차기로 프로그램 다운로드 
				 */
                sRetVal = SendSubTermImgOldProtcol( nDevNo, BUS_EXECUTE_FILE );
                if ( sRetVal != SUCCESS )
                {
                    printf( "[%d]번 하차기로 신프로토콜 사용 f/w 다운로드 중 오류\n",nDevNo );
                    tcflush( nfdMSC, TCIOFLUSH );          
                    sleep(2);
                }
                else
                {
                    printf( "[%d]번 하차기로 신프로토콜 사용 f/w 다운로드 성공\n",nDevNo );
                    tcflush( nfdMSC, TCIOFLUSH );             
                    if (  gbSubTermCnt > 1 )
                    {
			            /* 
			             * 하차기 프로그램 다운로드시 다운로드 성공후
			             * 부팅시간 확보를 위해 50초간 sleep을 준다
			             * 이유 - 시간을 주지 않을 경우 다음 단말기의 
			             *        다운로드가 정상적으로 되지 않는 현상이 발생
			             */                         
                        sleep( 50 );
                    }
					else
					{
					   /* 
                        * 하차기가 1대인경우에는 1대의 하차기가 부팅하는 시간만큼
					    */
						sleep( 25 );
					}
                    break;
                }
            }
        }
        else
        {
            printf( "[%d] 번 하차기 버젼", nDevNo );
            PrintlnASC( ":", gpstSharedInfo->abSubVer[nDevNo-1], 4 );
        }
        /* 
         * 하차기단말기 번호 증가
         */
        nDevNo++;
    }


    /* 
     * SendPkt/RecvPkt 함수내에서 구프로토콜을 사용하여 송수신해야 함을 알려주는
     * Flag 해제 
     */

	 boolIsOldProtocol = FALSE;

	/* 
     * 0401 이후 c_ex_pro.dat는 집계에서 승차기로 전송하지 않기로 洋像많퓐�
     * 기존의 c_ex_pro.dat가 남아있을 경우 한번은 삭제해주어야 함.
	 */
	system("rm c_ex_pro.dat");

	return SUCCESS;
}




/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SendPkt												   *
*                                                                              *
*  DESCRIPTION:       전송구조체의 데이터를 프로토콜에 맞게 패킷을 생성하여    *
*                     전송한다.												   *
*																			   *
*  INPUT PARAMETERS:  void 									                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 																			   *
*        																	   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS: 신 프로토콜 패킷구조는 아래와 같다.                                *
*			STX LEN LEN CMD DEVNO SEQ SEQ DATA(N byte) ETX CRC CRC CRC CRC     *
*           구 프로토콜 패킷구조는 아래와 같다.                                *
*			STX LEN LEN CMD DEVNO SEQ SEQ DATA(N byte) ETX CRC CRC CRC CRC     *
*																			   *		
*          																	   *
*                                                                              *
*******************************************************************************/
short SendPkt( void )
{
    byte abSendBuffer[1100] = { 0, }; /* 송신데이터 버퍼 */
    int nSendBufferLen = 0;           /* 송신데이터 버퍼 길이 */  
    dword dwCommCrc;                  /* CRC 값 */
    short sRetVal;
    /*
     * 구 프로토콜로 전송(03xx 버전)
     */
   /* if ( ( boolIsOldProtocol == TRUE ) ||
         ( stSendPkt.nDataSize == 1 &&
           stSendPkt.bCmd == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) ||
         ( stSendPkt.nDataSize == 17 &&
           stSendPkt.bCmd == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) )
           */     
	         
    if ( boolIsOldProtocol == TRUE )	
    {

        DebugOut( "구 프로토콜로 send==\n" );

        nSendBufferLen = stSendPkt.nDataSize + 10;
		/* 1. STX - 1byte */
        abSendBuffer[0] = STX;   
		/* 2. 데이터길이 - 2byte */		
        abSendBuffer[1] = ((stSendPkt.nDataSize + 2) & 0xFF00)>>8; 
        abSendBuffer[2] = (stSendPkt.nDataSize + 2) & 0x00FF;
		/* 3. bCmd(명령어)- 1byte */			
        abSendBuffer[3] = stSendPkt.bCmd;  
		/* 4. bDevNo(단말기번호) - 1byte */		
        abSendBuffer[4] = stSendPkt.bDevNo;   

		/* 송신Buffer에 데이터 Copy */
        if ( boolIsMainSubCommFileDownIng == TRUE ) 
        {
			/* 
			 * 하차기프로그램파일 다운로드를 나타내는 Flag이 설정 되어있다면 
			 * stSendPkt구조체내의 abData대신에 abSendData구조체를 사용한다.
			 * 이유 - 구프로토콜과 최대패킷사이즈가 틀리기 때문에 
			 */        
            memcpy( &abSendBuffer[5], abSendData, stSendPkt.nDataSize );
        }
        else
        {
			/* 
			 * 하차기프로그램파일 다운로드 중이 아닌경우는 
			 * 그대로 stSendPkt구조체 이용
			 */
            memcpy( &abSendBuffer[5], stSendPkt.abData, stSendPkt.nDataSize );
        }

    }
   /*
    * 신 프로토콜로 전송(04xx 버전) 
    */
    else
    {
        nSendBufferLen = stSendPkt.nDataSize + 12;

		/* 1. STX - 1byte*/
        abSendBuffer[0] = STX; 
		/* 2. 데이터길이 - 2byte*/
        abSendBuffer[1] = ((stSendPkt.nDataSize) & 0xFF00)>>8; 
        abSendBuffer[2] = (stSendPkt.nDataSize ) & 0x00FF;
		/* 3. bCmd(명령어)- 1byte*/		
        abSendBuffer[3] = stSendPkt.bCmd; 
		/* 4. bDevNo(단말기번호) - 1byte */
        abSendBuffer[4] = stSendPkt.bDevNo; 

		/* 5. Sequence - 2byte	*/
        memcpy( &abSendBuffer[5], ( byte*)&stSendPkt.wSeqNo, 2);
        /*
		 * 6. 데이터를 데이터 사이즈만큼 송신버퍼에 Copy         
 		 * 데이터가 없는 경우 즉, 데이터사이즈가 0인경우에는 프로토콜의 데이터
 		 * 영역에 아무것도 쓰지 않는다.
		 */
        if ( stSendPkt.nDataSize != 0 )
        {
            memcpy( &abSendBuffer[7], stSendPkt.abData, stSendPkt.nDataSize );
        }
    }

	/* 6. ETX - 1byte	*/  
    abSendBuffer[nSendBufferLen-5] = ETX; 
	/* 7. 패킷 CRC - 4byte	*/
    dwCommCrc = MakeCRC32( abSendBuffer, nSendBufferLen-4 );
    memcpy( &abSendBuffer[nSendBufferLen-4], ( byte *)&dwCommCrc, 4 );

   /*
   	* 송신데이터 버퍼의 데이터를 전송
   	*/
    sRetVal = CommSend( nfdMSC, abSendBuffer, nSendBufferLen );
	//PrintlnBCD("Send Data", abSendBuffer, nSendBufferLen);
    if ( sRetVal < 0 )
    {
       /*
       	* 전송실패시 전송데이터 구조체 초기화 
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
*  DESCRIPTION:       전송구조체의 데이터를 프로토콜에 맞게 패킷을 생성하여    *
*                     전송한다.												   *
*																			   *
*  INPUT PARAMETERS:  int nTimeOut - 수신 TimeOut시간                          *
*                     int nDevNo   - 단말기 번호                               *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
* 					  ERR_MAINSUB_COMM_STX_IN_PKT - STX에러      			   *
*                     ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT 					   *
*                      - 승차기가 송신후 수신을 기다리는 단말기가 아닌 타      *
*                        단말기 패킷수신 에러 								   *
* 				      ERR_MAINSUB_COMM_NOT_MINE_PKT							   *
*                      - 하차기가 자신의 단말기번호와 다른 패킷수신 에러       *
*                     ERR_MAINSUB_COMM_INVALID_LENGTH                          *
*                      - 데이터 길이 에러                                      *
*                     ERR_MAINSUB_COMM_SEQ_IN_PKT                              *
*                      - 데이터 시퀀스 에러                                    *
*                 	  ERR_MAINSUB_COMM_ETX_IN_PKT                              *
*                      - ETX 에러                                              *
* 					  ERR_MAINSUB_COMM_CRC_IN_PKT                              *
*                      - CRC 에러                                              *
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

    byte abTmpBuff[25] = { 0, };    /* 임시 버퍼  */
    byte abRecvBuff[1048]= { 0, };  /* 수신데이터 버퍼 */
    int nRecvDataLen;				/* 수신데이터 길이 */
    int nRecvBuffLen;				/* 수신버퍼 길이 */
    word wSeqNo;					/* 시퀀스 번호 */
    dword dwCommCRC;				/* CRC값 */
    short sRetVal = SUCCESS;
    short sRetVal1 = SUCCESS;

    int nEtxPoint = 5;              /* 프로토콜상에서 ETX의 위치 - 끝에서 5번째 */
	/* 
	 * 구버전 펌웨어(03xx)에서는 통신에서 CRC, BCC가 혼재되어 사용되었고
	 * 따라서 메인프로세스에서 부팅시 초기에 CRC를 사용하는 단말기인 경우에는
	 * 명령어 'C'를 제일먼저 전송하여 자신이 CRC를 쓰는 단말기인것을 상대방에게
	 * 알려주게 되어있었음.
	 * 따라서 신버전에서는 패킷을 수신하여 'C'일 경우에는 boolIsCcmdRecv
	 * 변수를 flag으로 사용하여 다시한번 데이터를 수신 하도록 처리하였음
	 */
    bool boolIsCcmdRecv = FALSE;    /* CRC Command('C') 수신여부 Flag */

   /* 
	* 수신데이터 구조체 초기화
	*/
    memset( &stRecvPkt, 0, sizeof( stRecvPkt ) );

   /* 
	* do ~ while( C 명령어 수신인경우 ) 구조의 Loop시작 
	*/
    do
    {
        /*
         * 1. 데이터 최초 수신 - 1byte//////////////////////////////////////////
         */
        DebugOut( "\n***************************************************" );
        sRetVal = CommRecv( nfdMSC, abTmpBuff, 1, nTimeOut );
        /*
         * 데이터 최초 수신 실패시 
         */
        if ( sRetVal < 0 )  
        {
            DebugOut( "\n    [STX  수신]실패 => [%d]\n", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof( stRecvPkt ) );
            tcflush( nfdMSC, TCIOFLUSH );
            DebugOut( "***************************************************\n" );
            return ErrRet( sRetVal );
        }
        else
        {
            DebugOut( "\n    [STX  수신]성공[%02x]", abTmpBuff[0] );
        }

        /*
         * 데이터 최소 수신시 STX 수신여부 검증 
         */
        if ( abTmpBuff[0] != 0x02 )
        {
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush ( nfdMSC, TCIOFLUSH );
            DebugOut( "STX 에러발생 [%02x]\n", abTmpBuff[0] );
            return ErrRet( ERR_MAINSUB_COMM_STX_IN_PKT );    
        }

        /*
         * 2. Length 수신 - 2 byte//////////////////////////////////////////////
         */
        sRetVal = CommRecv( nfdMSC, &abTmpBuff[1], 2, nTimeOut );

        if ( sRetVal < 0 )
        {
            DebugOut( "\n    sRetVal 2 [%d]", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush ( nfdMSC, TCIOFLUSH );
            DebugOut( "\n    [LEN  수신]실패\n" );
            DebugOut( "***************************************************\n" );
            return ErrRet( sRetVal );

        }
        else
        {
          DebugOut( "\n    [LEN  수신]성공[%02x][%02x]",
                    abTmpBuff[1], abTmpBuff[2] );
        }

        /*
         * 3. Command 수신 - 1 byte/////////////////////////////////////////////
         */
        sRetVal = CommRecv( nfdMSC, &abTmpBuff[3], 1, nTimeOut );	
        if ( sRetVal != SUCCESS )
        {
            DebugOut( "\n    sRetVal 3 [%d]", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush( nfdMSC, TCIOFLUSH );
            DebugOut( "\n    [CMD  수신]실패(NegativeValue)\n" );
            DebugOut( "***************************************************\n" );
            return ErrRet( sRetVal );
        }


        /*
         * Command 수신 후 검증  
         */            
        sRetVal = IsCmdValidCheck( abTmpBuff[3] );
		if ( sRetVal != SUCCESS )
		{
            DebugOut( "\n    sRetVal 3 [%d]", sRetVal );
            DebugOut( "\n    [CMD  수신]실패(ValidationFail)[ch:%c][hex:%x]\n",
                       abTmpBuff[3],abTmpBuff[3] );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush( nfdMSC, TCIOFLUSH );
            DebugOut( "***********************************************\n" );
            return ErrRet( sRetVal );
        }
		
        /*
         * C command여부 체크 
         */ 
		if ( abTmpBuff[3] == MAIN_SUB_COMM_CHK_CRC )
		{
			/*
			* C Command를 받은 경우 다시 한번 데이터 수신하도록 flag처리
			*/
		    DebugOut( "\n    [CMD  수신]성공[%c]", abTmpBuff[3] );
            boolIsCcmdRecv = TRUE;
		}
		else
		{
			DebugOut( "\n    [CMD  수신]성공[%c]", abTmpBuff[3] );
		    boolIsCcmdRecv = FALSE;
		}
		
        /*
         * 4. 단말기 번호(devNo) 수신 - 1byte///////////////////////////////////
         */         
        sRetVal = CommRecv( nfdMSC, &abTmpBuff[4], 1, nTimeOut );
		
        /*
         * 수신된 단말기번호가 1,2,3이 아닌 경우에도 에러처리
         */ 
        if ( sRetVal < 0  ||  abTmpBuff[4] < '1' || abTmpBuff[4] > '3' )
        {
            DebugOut( "\r\n sRetVal 4 [%d]", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush( nfdMSC, TCIOFLUSH );
            DebugOut( "\n    [devNo수신]실패\n" );
            DebugOut( "***************************************************\n" );
            return ErrRet( sRetVal );
        }
		
		/* 
		 * 단말기 번호 검증
		 */
        if ( gboolIsMainTerm == TRUE  )
		{
		 	/* 승차단말기인 경우 - 내가 송신한 단말기번호와 비교 */
            if( nDevNo != abTmpBuff[4]-'0' ) 
            {
	            LogTerm("[DevNo수신]프로토콜 에러 [%02x]\n", abTmpBuff[4] );
	            DebugOut("[DevNo수신]프로토콜 에러 [%02x]\n", abTmpBuff[4] );
	            sRetVal1 = ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT;
            }
        }
        else
        {
       		/* 하차단말기인 경우 - 내 단말기 번호와 비교 */
 			if ( abTmpBuff[4]-'0' != gbSubTermNo ) 
 			{
 				DebugOut( "    [DevNo수신]타 신호[%02x]\n", abTmpBuff[4] );
            	//tcflush( nfdMSC, TCIOFLUSH );
            	//DebugOut( "    [수신 결과]타 하차기신호\n" );
            	//DebugOut( "***************************************************\n" );
            	sRetVal1 = ERR_MAINSUB_COMM_NOT_MINE_PKT;	
 			}
        }
        DebugOut( "\n    [DevNo수신]성공[%c번 단말기]", abTmpBuff[4] );
        

        /*
         * 구 프로토콜로 펌웨어를 다운로드 하려는 명령어인지 확인하기 위해
         * 구 프로토콜의 Data부분 길이 계산방법대로 수신데이터 길이를 계산.
         * 구 프로토콜은 STX LEN LEN CMD DEVNO DATA ETX CRC CRC CRC CRC.
         * 구 SendPkt에서 데이터사이즈에 +2한 값을 넣기 때문에 수신하는 쪽에서는
         * + 8을 해서 전체 수신데이터 길이로 계산함.
         * 
         */
        nRecvDataLen = ( ( abTmpBuff[1] << 8 ) | abTmpBuff[2] ) + 8;
        /*
         * 승차기에서는 구프로토콜을 이용하여 수신해야하는지 여부를 
         * boolIsOldProtocol로 구분하고
         * 하차기에서는 데이터 길이가 40이고 명령이 MAIN_SUB_COMM_SUB_TERM_IMG_VER
         * 또는 명령이 MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD인 경우로 구분한다.
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
            /* 승차기는 신버전 구버전 모두 초기에 구프로토콜로 펌웨어버전 체크
             * 명령어를 하차기로 보내는데 하차기에서는  
			 * RecvPkt()사용 수신시 아래부분에서  TRUE로 셋팅하게 된다.
			 * 승차기 프로그램 버전을 수신하는 함수인 SubRecvImgVer 실행시
			 * boolIsMainTermPreVer가 TRUE인 경우에는 하차기와 승차기 버전을 
			 * 서로 비교하여 하차기의 Rollback 실행을 판단하게 된다.
			 */
            boolIsRespVerNeedByOldProtocol = TRUE;
			/* old protocol상에서의 총 size */
            nRecvBuffLen = nRecvDataLen;    

	        /*
	         * 수신버퍼 길이 검증  
	         */
	        if ( nRecvBuffLen > MAX_PKT_SIZE_OLD )
	        {
	            DebugOut( "\r\n data size error [%d]", nRecvBuffLen );
	            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
	            tcflush ( nfdMSC, TCIOFLUSH );
	            DebugOut( "    [LENGTH 수신]실패\n" );
	            DebugOut( "***************************************************\n" );
	            return ErrRet( ERR_MAINSUB_COMM_INVALID_LENGTH );
	        }			
        }
        else
        {
            boolIsRespVerNeedByOldProtocol = FALSE;
		   /*
			* 신프로토콜은 STX LEN LEN CMD DEVNO SEQ SEQ DATA ETX CRC CRC CRC CRC. 
			* DATA부분을 제외하면 12byte가 더 있으므로 전체 버퍼길이는 
			* 데이터길이 + 12를 하게 된다.
			*/
            nRecvBuffLen = ( (abTmpBuff[1] << 8) | abTmpBuff[2] ) + 12;
		   /*
	         * 수신버퍼 길이 검증  
	         */
	        if ( nRecvBuffLen > MAX_PKT_SIZE )
	        {
	            DebugOut( "\r\n data size error [%d]", nRecvBuffLen );
	            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
	            tcflush ( nfdMSC, TCIOFLUSH );
	            DebugOut( "    [LENGTH 수신]실패\n" );
	            DebugOut( "***************************************************\n" );
	            return ErrRet( ERR_MAINSUB_COMM_INVALID_LENGTH );
	        }		   
        }

        memcpy( abRecvBuff, abTmpBuff, 5 );

        /*
         * 5. 나머지 패킷(SEQ/데이터/ETX/CRC)수신///////////////////////////////   
         */
        sRetVal = CommRecv( nfdMSC, &abRecvBuff[5], nRecvBuffLen-5, nTimeOut );
		/*
         * 데이터 부분 수신 후 검증    
         */
        /*
         * 1) 자신의 패킷이 아닌 경우 return
         *    자신의 패킷인지는 DevNo만 수신하여 검증하면 알수 있지만
         *    버퍼에 데이터를 남기지 않기 위해 모든 데이터를 수신한후에
         *    비교를 해주게 된다.
         *    그래서 DevNo수신시 자기 패킷이 아니면 승차기는 
         *    ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT, 하차기는 ERR_MAINSUB_COMM_NOT_MINE_PKT
         *    에러코드를 변수에 저장후 여기서 비교하게 되는 것임.    
         */
        if ( sRetVal1 == ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT )
        {
        	/* 승차단말기인 경우 - 송신한 단말기에서 수신할 단말기가 아닌 
        	 * 다른 단말기에서 데이터가 수신된 경우 -프로토콜 에러코드 리턴
        	 */ 
            printf( "    [DATA 수신]ERR_MAINSUB_COMM_NOT_CURR_PROT_PKT\n" );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            tcflush( nfdMSC, TCIOFLUSH );
            return ErrRet( sRetVal1 );
        } 
        else if ( sRetVal1 == ERR_MAINSUB_COMM_NOT_MINE_PKT )
        {
        	/* 하차단말기의 경우 - 수신한패킷이 자신의 단말기번호와 틀린 경우로 
        	 * 자신이 수신할 패킷이 아닌경우로 - Not Mine Pkt 에러코드 리턴
        	 */             
            return ErrRet( sRetVal1 );
        }
		
        /*
         * 2) 데이터 부분 수신시 에러처리
         */
        if ( sRetVal < 0 )
        {
            printf( "\r\n sRetVal 5 [%d]", sRetVal );
            memset( &stRecvPkt, 0x00, sizeof( stRecvPkt ) );
            tcflush ( nfdMSC, TCIOFLUSH );
            printf( "    [DATA 수신]실패\n" );
            printf( "***************************************************\n" );
            return ErrRet( sRetVal );
        }
        DebugOut( "\n    [DATA 수신]성공-길이:%d\n", nRecvBuffLen-5 );


        /*
         *  Sequence 검증 - 신프로토콜인 경우에만 체크 
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
            DebugOut( "    [SeqNo 체크] Skip!\n" );
        }
        else
        {
            memcpy( ( byte*)&wSeqNo, &abRecvBuff[5], 2 );
            DebugOut( "[수신 SEQ] %d 번 패킷", wSeqNo );
	        /*
	         *  Sequence Range 체크 
	         */			
            if ( wSeqNo > 9999 )
            {
                DebugOut( "[SEQ 검증]SEQ 에러발생\n" );
                return ErrRet( ERR_MAINSUB_COMM_SEQ_IN_PKT );
            }
            else
            {
	        /*
	         *  Sequence를 수신패킷구조체에 저장
	         */	            
                stRecvPkt.wSeqNo = wSeqNo;
            }

        }
        /*
         *  ETX 체크
         */ 
        if ( abRecvBuff[nRecvBuffLen-nEtxPoint] != ETX )
        {
        	///printf( "etx 1- %x\n",  abRecvBuff[nRecvBuffLen-nEtxPoint]);
			///printf( "etx 2- %x\n",abRecvBuff[nRecvBuffLen-nEtxPoint+1]);
			///printf( "etx -1- %x\n",abRecvBuff[nRecvBuffLen-nEtxPoint-1]);
			
            memset( &stRecvPkt, 0x00, sizeof( stRecvPkt ) );
            DebugOut( "\n    [DATA 검증]ETX 에러발생 \n" );
            tcflush ( nfdMSC, TCIOFLUSH );
            DebugOut( "***************************************************\n" );

            return ErrRet( ERR_MAINSUB_COMM_ETX_IN_PKT );    
        }

        /*
         * CRC 체크
         */       
        dwCommCRC = MakeCRC32( abRecvBuff, nRecvBuffLen-4 );

        if ( memcmp( &abRecvBuff[nRecvBuffLen-4], ( byte*)&dwCommCRC, 4 ) != 0 )
        {
            int i = 0;

            for ( i = 0 ; i < nRecvBuffLen ; i++ )
            {
                DebugOut( "%02x ", abRecvBuff[i] );
            }

            DebugOutlnBCD( "    [DATA 검증]CRC 에러발생 받은 CRC     ==> ",
                           &abRecvBuff[nRecvBuffLen-4], 4 );
            DebugOutlnBCD( "    [DATA 검증]CRC 에러발생 계산한  CRC ==> ",
                           ( byte*)&dwCommCRC, 4 );
            memset( &stRecvPkt, 0x00, sizeof(stRecvPkt) );
            DebugOut( "\n    [DATA 검증]CRC 에러발생 \n" );
            tcflush( nfdMSC, TCIOFLUSH );

            return ErrRet( ERR_MAINSUB_COMM_CRC_IN_PKT );
        }

    }
    while ( boolIsCcmdRecv == TRUE );

    /*
     * 수신 data 구조체에 값 저장
     */      
    stRecvPkt.bCmd = abRecvBuff[3];                /* 명령어 저장 */
    stRecvPkt.bDevNo = abRecvBuff[4] - '0';		   /* 단말기번호 저장 */
    /*
     * 데이터사이즈 만큼 데이터를 수신 데이터 구조체에 저장
     */ 
    if ( ( boolIsOldProtocol == TRUE && gboolIsMainTerm == TRUE ) ||
         ( ( nRecvDataLen == 40 && abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_VER ) &&
           ( gboolIsMainTerm == FALSE ) )
       )
    {
        /* 
         *구프로토콜은 데이터 버퍼길이에서 -10해준것이 실제 데이터 사이즈
		 * STX(1) LEN(2) LEN(3) CMD(4) DEVNO(5) DATA ETX(6) CRC(7)CRC(8)CRC(9)CRC(10)
		 */
        stRecvPkt.nDataSize = nRecvDataLen - 10;

        if ( stRecvPkt.nDataSize != 0 )
        {
        	/* 
	         * 6 byte부터 수신된 데이터이므로 
			 */        
            memcpy( stRecvPkt.abData, &abRecvBuff[5], stRecvPkt.nDataSize );
        }
    }
	else if (( abTmpBuff[3] == MAIN_SUB_COMM_SUB_TERM_IMG_DOWN_OLD )&&
		     ( gboolIsMainTerm == FALSE )
		    )
	{
	   /* 
	    * 구 프로토콜을 이용하여 승차기에서 펌웨어다운로드를 하는 경우에는
	    * 최대패킷사이즈의 차이가 있어 stRecvPkt.abData대신 abRecvData를 
	    * 사용한다.
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
         * 신 프로토콜은 데이터 버퍼길이에서 -12 해준것이 실제 데이터 사이즈
		 * STX(1) LEN(2) LEN(3) CMD(4) DEVNO(5) SEQ(6) SEQ(7) DATA ETX(8) CRC(9)CRC(10)CRC(11)CRC(12)
		 */    
        stRecvPkt.nDataSize = nRecvBuffLen - 12 ;

        if ( stRecvPkt.nDataSize != 0 )
        {
        	/* 
	         * 8 byte부터 수신된 데이터이므로 
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
*  DESCRIPTION:       파일을 전송한다.   								       *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo- 단말기번호 				                   *
*                     char* pchFileName - 송신대상 파일명 				       *
* 																			   *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                     ERR_MAINSUB_COMM_FILE_NOT_FOUND_EOT_SEND                 *
*                      - EOT전송중 에러 		                        	   *
*                     ERR_MAINSUB_COMM_FILE_NOT_FOUND                          *
*                      - 요청파일 미존재                                       *
*                     ERR_MAINSUB_COMM_PARM_DOWN_NAK                           *
*                      - 수신패킷전송에 대한 NAK응답                           *
*                     ERR_MAINSUB_COMM_FILE_NOT_FOUND_RECV                     *
*                      - EOT전송 후 수신에러                                   *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:       	 송신할 파일이 존재하지 않는 경우 EOT 전송                 *
*                    송신할 파일이 존재하는 경우에는 파일의 마지막 패킷에      *
*                    마지막 패킷의 의미로 EOT를 전송                           *
*                    각 패킷 전송마다 ACK로서 패킷전송완료를 확인한다.         *
*																			   *
*******************************************************************************/
short SendFile( int nDevNo, char* pchFileName )
{
    short sRetVal = SUCCESS;
    int fdFile;
    bool boolIsFileEnd = FALSE;   /* 파일의 끝을 나타내는 Flag */         
	int nByte;					  /* 송신파일에서 읽은 Byte수 */
    word wSeqNo = 0;			  /* Sequence 번호 */
	bool boolIsEOTUse = FALSE;
    usleep( 100000 );

	/*
	 * 전송할 파일 열기
	 */
	fdFile = open( pchFileName, O_RDONLY, OPENMODE );

    if ( fdFile < 0 )
    {
        printf( "\r\n 파일 오픈 안됨\r\n" );
	   /*
	   	* TD파일 전송이 아닌 경우에는 파일 오픈이 안되면 
	   	*/
		if ( bCurrCmd != MAIN_SUB_COMM_GET_TRANS_FILE )
		{
			return -1;
		}
	   /*
	   	* 전송할 파일이 오픈이 안되는 경우에는 파일이 존재하지 않는 경우로 간수
	   	* 파일전송을 요청한 쪽에 EOT를 보내게 된다. 
	   	*/
   		/* Cmd(커맨드) : 하차기에서 수신한 커맨드 */
        stSendPkt.bCmd = bCurrCmd; 
		/* bDevNo(단말기번호) : 단말기번호 + ASCII 0  */ 
        stSendPkt.bDevNo = nDevNo + '0';
		/* wSeqNo(시퀀스 번호) : 0  */ 
        stSendPkt.wSeqNo = 0;   
		/* nDataSize(데이터사이즈) : 1  */ 		
        stSendPkt.nDataSize = 1;
		/* abData(데이터) : 데이터부분에 EOT  */ 		
        stSendPkt.abData[0] = EOT;

		boolIsEOTUse = TRUE;
	   /*
	   	* EOT전송 
	   	*/
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            printf( "\r\n  파일 존재하지 않아 EOT Send 중 Error \r\n" );
            sRetVal =  ERR_MAINSUB_COMM_FILE_NOT_FOUND_EOT_SEND;
            return ErrRet( sRetVal );
        }
	   /*
	   	* EOT전송에 대한 응답수신
	   	*/
        sRetVal = RecvPkt( 3000, nDevNo );
        if ( sRetVal < 0 )
        {
            printf( "\r\n  파일 존재하지 않아  EOT Send 후 Recv Error \r\n" );
            sRetVal =  ERR_MAINSUB_COMM_FILE_NOT_FOUND_RECV;
            return ErrRet( sRetVal );
        }
	   /*
	   	* EOT전송에 대한 응답으로 ACK 또는 NAK 수신 
	   	*/
        if ( ( stRecvPkt.bCmd == ACK ) && ( boolIsEOTUse == TRUE ) )
        {
            printf( "\r\n 파일 존재하지 않아 EOT Send후 ACK 수신완료!\r\n" );
            sRetVal =  ERR_MAINSUB_COMM_FILE_NOT_FOUND;
            return ErrRet( sRetVal );
        }
        else if ( stRecvPkt.bCmd == NAK )
        {
            printf( "NAK 수신\n" );
            sRetVal =  ERR_MAINSUB_COMM_PARM_DOWN_NAK;
            return ErrRet( sRetVal );
        }

    }
   /*
   	* 파일전송 Loop 시작 
   	*/
    while( TRUE )
    {
    	printf( "." );						// 전송중임을 콘솔에 표시한다.
    	fflush( stdout );

		/*
		 * 전송할 파일 최대 1024byte까지 데이터영역에 읽기  
		 */    
        nByte = read( fdFile, stSendPkt.abData, DOWN_DATA_SIZE );
	   /*
	   	* 읽어 들인 데이터가 존재하는 경우 패킷 생성
	   	*/  
	   	if ( nByte > 0 )
        {
            /* Cmd(커맨드) : 현재 단말기가 실행중인 커맨드 */
            stSendPkt.bCmd = bCurrCmd; 
 			/* bDevNo(단말기번호) : 단말기번호 + ASCII 0   */			
            stSendPkt.bDevNo = nDevNo + '0'; 
			/* nDataSize(데이터사이즈) : 파일에서 읽은 byte  */ 					
            stSendPkt.nDataSize = nByte;     
			/* wSeqNo(시퀀스 번호) : 패킷의 시퀀스 번호  */ 
            stSendPkt.wSeqNo = wSeqNo;        
        }
	   /*
	   	* 읽어 들인 데이터가 0인 경우 즉, 파일의 끝인경우 EOT 패킷 생성
	   	*/  		
        else if ( nByte == 0 ) 
        {
            DebugOut( "화일끝으로 들어오다 %d \n", nByte );
			/* Cmd(커맨드) : 현재 단말기가 실행중인 커맨드 */			
            stSendPkt.bCmd = bCurrCmd;
			/* bDevNo(단말기번호) : 단말기번호 + ASCII 0   */					
            stSendPkt.bDevNo = nDevNo + '0';
			/* nDataSize(데이터사이즈) : 1  */ 			
            stSendPkt.nDataSize = 1;
			/* wSeqNo(시퀀스 번호) : 패킷의 시퀀스 번호  */ 		
            stSendPkt.wSeqNo = wSeqNo;
			/* abData[0] : 데이터에 EOT를 넣어 수신측에서 파일끝을 표시 */ 
            stSendPkt.abData[0] = EOT;
		   /*
		   	* 파일 끝 표시 
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
            DebugOut( "\n[SendPkt-EOTto %d번 단말기]", nIndex + 1 );
        }
        else
        {
            DebugOut( "\n[SendPkt-to %d번 단말기]", nIndex + 1 );
        }
        DebugOut( "[송신 SEQ] %d 번 패킷", wSeqNo );

	   /*
	   	* 패킷 전송 
	   	*/ 			
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            break;
        }

	   /*
	   	* 응답패킷 수신
	   	*/ 	
        sRetVal = RecvPkt( 3000, nDevNo );
        if ( boolIsFileEnd == TRUE )
        {
            DebugOut( "\n[RecvPkt-EOT응답-from %d번 단말기]", nIndex+  1 );
        }
        else
        {
            DebugOut( "\n[RecvPkt-from %d번 단말기]", nIndex + 1 );
        }
		
        if ( sRetVal < 0 )
        {
            DebugOut( "\r\n 파일 전송중 응답없음[%d]\r\n", sRetVal );
            break;
        }
	   /*
	   	* 파일 전송완료 
	   	* - 파일이 전송완료가 되었음을 판단하는 조건으로는 파일끝에 도달하고
	   	*   ACK가 수신된 경우라야 한다. 
	   	*/ 
        if ( ( stRecvPkt.bCmd == ACK ) && ( boolIsFileEnd == TRUE) )
        {
            printf( "/\n" );					// 전송완료를 콘솔에 표시한다.
            sRetVal = SUCCESS;
            break;
        }
	   /*
	   	* 파일 전송중 NAK 응답으로 종료
	   	*/ 		
        else if ( stRecvPkt.bCmd == NAK )
        {
            sRetVal = ERR_MAINSUB_COMM_PARM_DOWN_NAK;
            break;
        }
	   /*
	   	* 패킷 Sequence 번호 증가
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
*  DESCRIPTION:       파일을 수신한다.   								       *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo- 단말기번호 				                   *
*                     char* pchFileName - 수신패킷을 저장할 파일명             *
* 																			   *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*                     ERR_MAINSUB_COMM_SEQ_DURING_RECV_FIL                     *
*                      - 파일수신시 시퀀스 에러                         	   *
*                     ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST                      *
*                      - 요청파일 미존재                                       *
* 																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:       	 파일전송요청을 받은 단말기에서는 파일이 없는 경우 응답을  *
*                    EOT로 보내게 되고 파일의 마지막 패킷에도 EOT를 보내게 된다*
*					 차이는 파일전송요청후 첫 응답이 EOT가 오면 요청파일이     *
*                    존재하지 않는 것이며 첫 응답 이후 EOT가 오면 파일전송이   *
*                    완료되었음을 의미한다.  								   *
*                    정리하면, 												   *
*                    파일전송을 요청한 단말기에서는 EOT가 수신된 경우에는      *
*                    첫 패킷수신에 EOT가 온 경우- 요청한 파일이 없다는 의미    *
*                    첫 패킷수신이후 EOT가 온 경우- 전송이 완료 되었다는 의미  *
*																			   *
*******************************************************************************/
short RecvFile( int nDevNo, char* pchFileName )
{
    int fdFile;
    short sRetVal = SUCCESS;
    int nRecvCnt = 0;         /* 패킷수신 Count */
    word nPreSeqNo = 0;		  /* 이전 Sequence 번호 */

   /*
    * 수신된 파일을 저장할 파일 생성/열기
    */
    fdFile = open( pchFileName, O_WRONLY | O_CREAT | O_TRUNC, OPENMODE );

    if ( fdFile < 0 )
    {
    	printf( " %s file open fail\n", pchFileName);
        close( fdFile );
        return -1;
    }
   /*
    * 패킷수신 및 저장 Loop 
    */
    while( 1 )
    {
    	printf( "." );						// 수신중임을 콘솔에 표시한다.
    	fflush( stdout );

	   /*
	    * 패킷수신
	    */
        sRetVal = RecvPkt( 8000, nDevNo );

        if ( sRetVal < 0 )
        {
            printf( "/r/n file recv Error 1: [%x] /r/n", sRetVal );
            break;
        }
	   /*
	    * 파일 패킷의 Sequence Check
	    * 현재 수신시퀀스가 이전 패킷시퀀스 + 1인지 여부, 0이 아닌지 체크
	    */
        if ( ( stRecvPkt.wSeqNo - nPreSeqNo != 1 ) && ( stRecvPkt.wSeqNo != 0 ) )
        {
             DebugOut( "패킷 Sequence Error!! 수신예정패킷번호 : %d 번, ",
                       nPreSeqNo+1 );
             DebugOut( "실제수신패킷번호 : %d 번 ", stRecvPkt.wSeqNo );
             sRetVal = ERR_MAINSUB_COMM_SEQ_DURING_RECV_FILE;
             break;
        }

	   /* 
	    * 패킷수신 Count 증가 - 첫 패킷 수신을 구분하기 위해
	    */
        nRecvCnt++;
	   /* 
	    * 패킷수신시 ACK응답을 위한 데이터 생성 
	    */
        stSendPkt.bCmd = ACK;
        stSendPkt.bDevNo = nDevNo + '0';
        stSendPkt.nDataSize = 0;
        stSendPkt.wSeqNo = 0;
	   /* 
	    * ACK전송 
	    */
        sRetVal = SendPkt();
        if ( sRetVal < 0 )
        {
            DebugOut( "/r/n SendPkt Error  : [2] /r/n" );
            break;
        }

	   /* 
	    *  EOT수신여부 체크 
	    */
        if ( ( stRecvPkt.nDataSize == 1 ) && ( stRecvPkt.abData[0] == EOT ) )
        {
            if ( nRecvCnt == 1 )  // YYYYMMDDHHMMSS.tmp
            {
                /* 첫 패킷수신인 경우는 EOT의 의미는 요청파일 없다는 응답임 */
                DebugOut( "\r\n[파일]하차단말기에 요청한 파일이 없습니다.\r\n" );
                sRetVal = ERR_MAINSUB_COMM_REQ_FILE_NOT_EXIST;
            }
            else
            {
                /* 첫 패킷수신이 아닌경우는  EOT의 의미는 파일전송완료 */
                DebugOut( "\r\n[파일]전송완료\r\n" );
            }
            break;
        }
	    /* 
	     *  패킷수신데이터 파일에 저장 
	     */
        write( fdFile, stRecvPkt.abData, stRecvPkt.nDataSize );
		/* 
	     *  다음 패킷수신시 시퀀스 체크를 위해 현재 시퀀스를 저장
	     */
        nPreSeqNo = stRecvPkt.wSeqNo;
    }

    close( fdFile );

	if ( sRetVal == SUCCESS )
		printf( "/" );						// 전송완료를 콘솔에 표시한다.
	else
		printf( "e\n" );

    return sRetVal;
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateACK                             				   *
*                                                                              *
*  DESCRIPTION:       ACK데이터를 생성한다.								       *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo- 단말기번호 				                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:       	  구 프로토콜과 신 프로토콜에 대해 각각 ACK 응답을 할수    *
*                     있도록 구성. 											   *
*																			   *
* 					  04xx버전의 신프로그램은 승차기에서 가장먼저 하차기의     *
*                     버전을 확인하고 신프로그램으로 업데이트하기위해          *
*                     UpdateSubTermImg()함수를 실행하게 되는데 이 함수내에서   *
*                     구프로토콜로 버전확인명령어('V')를 하차기로 보내게 된다. *
*                     이경우 하차기에서는 nDataSize가 30 이고 Cmd 가           *
*                     MAIN_SUB_COMM_SUB_TERM_IMG_VER인것을 보고 이 패킷은      *
*                     UpdateSubTermImg()함수가 보낸것으로 판별 구프로토콜로    *
*                     응답을 해주게 된다. 									   *
*                     														   *
*                     UpdateSubTermImg()함수가 버전확인명령어('V')를 보낸게    *
*                     아닌경우에는 즉, nDataSize가 30이 아니거나 명령어가      *
*                     MAIN_SUB_COMM_SUB_TERM_IMG_VER가 아닌경우로              *
*                     신프로토콜로 ACK를 보내게 된다.      					   *                                              * 
*                                                                              *
*******************************************************************************/
short CreateACK( int nDevNo )
{
    short sRetVal = SUCCESS;

	/* 
	 * 구 프로토콜로 수신한 경우 구프로토콜에 맞게 응답패킷구성 
	 */
    if ( boolIsRespVerNeedByOldProtocol == TRUE )
    {
   		/* Cmd(커맨드) : 하차기에서 수신한 커맨드 */
        stSendPkt.bCmd = stRecvPkt.bCmd;     
		/* bDevNo(단말기번호) : 단말기번호 + ASCII 0  */ 
        stSendPkt.bDevNo = stRecvPkt.bDevNo + '0';
		/* wSeqNo(시퀀스 번호) : 0  */ 
        stSendPkt.wSeqNo = 0;
		/* nDataSize(데이터사이즈) : 1  */ 		
        stSendPkt.nDataSize = 1;
		/* abData(데이터) : 데이터부분에 ACK  */ 		
        stSendPkt.abData[0] = ACK;
		/* 하차기 SendPkt()시 구프로토콜로 사용하도록 Flag Set */ 		
		boolIsOldProtocol = TRUE;		
    }
    else
    {
		/* 
		 * 신 프로토콜로 수신한 경우 신프로토콜에 맞게 응답패킷구성 
		 * 구 프로토콜과 다른 점은 커맨드부분에 ACK를 사용하고 
		 * 실제 데이터사이즈도 데이터의 크기를 나타낸다는 점이다.
		 * ACK응답의 경우 데이터가 없으므로 데이터사이즈가 0 임.
		 */   
	 	/* bCmd(커맨드) : 커맨드부분에 ACK  */ 
        stSendPkt.bCmd = ACK;
		/* bDevNo(단말기번호) : 단말기번호 + ASCII 0  */ 		
        stSendPkt.bDevNo = nDevNo + '0';
		/* wSeqNo(시퀀스 번호) : 0  */ 		
        stSendPkt.wSeqNo = 0;
		/* nDataSize(데이터사이즈) : 0  */ 			
        stSendPkt.nDataSize = 0;
		/* 하차기 SendPkt()시 신프로토콜로 사용하도록 Flag Set */ 		
		boolIsOldProtocol = FALSE;	
    }

    return ErrRet( sRetVal );
}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateNAK                                    		       *
*                                                                              *
*  DESCRIPTION:       NAK응답데이터를 생성한다.				                   *
*                                                                              *
*  INPUT PARAMETERS:  int nDevNo - 단말기 번호    		                       *
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
	* 송신 패킷구조체에 값 설정 
	*/
	/* bCmd(커맨드) : 커맨드부분에 NAK  */ 	
    stSendPkt.bCmd = NAK; 	
	/* bDevNo(단말기번호) : 단말기번호 + ASCII 0 */    
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
*  DESCRIPTION:       승하차간 통신채널을 닫는다. 	    				       *
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
*  DESCRIPTION:       하차기로부터 수신한 데이터중 Command부분을 검증한다.     *
*                   														   *
*																			   *
*  INPUT PARAMETERS:  byte  bCmd - 수신한 Command                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - 실행 성공 	                                   *
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




