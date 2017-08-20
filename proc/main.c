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
*  PROGRAM ID :       main.c                                                   *
*                                                                              *
*  DESCRIPTION:       This Program starts/ends Execution, initializes Term,    *
*                     checks basic datas and loops continually.                *
*                                                                              *
*  ENTRY POINT:       main ()               ** mandatory **                    *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 1 - Program Exit                                         *
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
*    DATE                SE NAME              DESCRIPTION                      *
* ---------- ---------------------------- -------------------------------------*
* 2005/08/09 Solution Team Gwan Yul Kim  Initial Release                       *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include <unistd.h>

/*******************************************************************************
*  Inclusion of User Header Files                                              *
*******************************************************************************/
#include "../system/bus_type.h"
#include "../system/bus_define.h"
#include "../system/device_interface.h"

#include "main.h"
#include "main_environ_init.h"
#include "main_load_criteriaInfo.h"
#include "main_process_busTerm.h"
#include "main_childProcess_manage.h"
#include "file_mgt.h"
#include "../comm/socket_comm.h"

/*******************************************************************************
*  Declaration of Global Variables                                             *
*******************************************************************************/
bool gboolIsMainTerm = FALSE;		// Div of MainTerm & SubTerm
PROC_SHARED_INFO *gpstSharedInfo = NULL;	// �����޸� ������
byte gbSubTermCnt = 0;				// �����ܸ��� ����
MYTERM_INFO gstMyTermInfo;			// ���α׷��� �����Ǵ� Terminal ����
pthread_t nCommGPSThreadID = 0;

MULTI_ENT_INFO gstMultiEntInfo;			// ���ν�����
CITY_TOUR_BUS_TICKET_INFO gstCityTourBusTicketInfo;
										// ��Ƽ������� �������Է�����

// Message Queue for Printing
int gnMsgQueue = 0;

byte gabKernelVer[3];
byte gbSubTermNo;				// # of SubTerm

byte gabGPSStationID[7];				// GPS������ ������Ʈ�ϴ� ������ID
int gnGPSStatus;						// GPS Status
time_t gtDriveStartDtime;				// ������۽ð�
bool gboolIsRunningGPSThread = FALSE;	// GPS������ ���� ���� (GPS�α� �ۼ� ����)
word gwDriveCnt;						// ����Ƚ��
word gwGPSRecvRate;						// GPS ������
dword gdwDistInDrive;					// ������ �̵��� �Ÿ�
bool gboolIsEndStation = 0;				// ������������
dword gdwDelayTimeatEndStation = 0;		// ������ �����ѵ� delay time

/*******************************************************************************
*  Declaration of Module Variables                                             *
*******************************************************************************/
static bool StartVoiceMent( void );
static void RebootSystem( NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      main                                                     *
*                                                                              *
*  DESCRIPTION :      BUS���α׷� Main                                         *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 1                                                        *
*                                                                              *
*  Author : GwanYul Kim													   	   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
int main( void )
{
	bool boolIsMentAlreadyOut = FALSE;
	NEW_IMGNVOICE_RECV stNewImgNVoiceRecv;

	memset( &stNewImgNVoiceRecv, 0x00, sizeof(NEW_IMGNVOICE_RECV) );
	stNewImgNVoiceRecv.boolIsReset = FALSE;

    /*
     *  Device Open
     */
	OpenDeviceFiles();

	InitLog();
	LogMain("�ܸ������\n");

    /*
     *  ���α׷��� �����Ǳ����� �⺻ ȯ����� CHECK
     */
	CheckEnvironment();

    /*
     *   SAM �ʱ�ȭ
     */
	SAMInitialization();

    /*
     *  RF �ʱ�ȭ
     */
	RFInitialization();

    /*
     *  CHILD PROCESS CHECK
     */
	CreateChildProcess();

    /*
     *  ���迡�� ���Ź��� �⺻���� LOADING
     */
	OperParmNBasicInfo( &stNewImgNVoiceRecv );

	while( stNewImgNVoiceRecv.boolIsReset == FALSE )
	{
	    /*
	     *  ���α׷����۸޼����� BOOTING�� 1ȸ�� �����
	     */
		if ( boolIsMentAlreadyOut == FALSE )
		{
			if( StartVoiceMent() == TRUE )
			{
				boolIsMentAlreadyOut = TRUE;
			}
		}

		CheckNDisplayDynamicInfo();

	    /*
	     *  ������: CARDó��. �������: BLPL MERGE CHECK
	     */
		MainProc( &stNewImgNVoiceRecv );

	    /*
	     *  CHILD PROCESS CHECK
	     */
		CheckChildProcess();
	}

	KillChildProcess();

    /*
     *  ����PC���� �űԽ�������/�����������Ž� �������
     */
	RebootSystem( &stNewImgNVoiceRecv );

	exit( 1 );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      StartVoiceMent                                           *
*                                                                              *
*  DESCRIPTION :      "������ �����մϴ�." �޼������                          *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
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
static bool StartVoiceMent( void )
{
	bool boolIsStarted = FALSE;
	short sDateGap = 0;
	char achBuf[100] = { 0, };

	if ( gboolIsMainTerm == TRUE )
	{
	    /*
	     *  TermComm���� �����غ�ɶ����� ��ٸ�
	     */
		if ( (gpstSharedInfo->boolIsReadyToDrive == TRUE) &&
			 (gpstSharedInfo->boolIsKpdLock != TRUE) &&
			 (gpstSharedInfo->boolIsKpdImgRecv == FALSE))
		{
			sleep(1);

			if( CheckMainTermBasicInfoFile() == SUCCESS )
			{
				/*
				 * �����������¥�� 4�� �̻��̸� ���ſ�û�����޼��� ���
				 */
				memset( achBuf, 0x00, sizeof(achBuf) );
				sDateGap = GetCommSuccTimeDiff( achBuf );
				if( 4 <= sDateGap )
				{
					DebugOut("DCS �� �������� [%d]�� \n", sDateGap );
					DisplayASCInUpFND( FND_ERR_RECEIVE_LATEST_INFO );
					DisplayASCInDownFND( achBuf );
					Buzzer( 5, 50000 );

					/*
					 * �ֽ� ���������� �����Ͽ� �ֽʽÿ�.
					 */
					VoiceOut( VOICE_RECEIVE_LATEST_INFO );
					sleep( 3 );
				}

				if ( IsDriveNow() != SUCCESS )
				{
					sleep( 3 );
				}

				boolIsStarted = TRUE;
				VoiceMent2EndUser();
			}
		}
	}
	else
	{
	    /*
	     *  �����⿡���� �ٷ� ���α׷����۸޼����� �����
	     */
	    boolIsStarted = TRUE;
		VoiceMent2EndUser();
	}

	return boolIsStarted;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      RebootSystem                                             *
*                                                                              *
*  DESCRIPTION :      �ý����� �������                                        *
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
static void RebootSystem( NEW_IMGNVOICE_RECV* pstNewImgNVoiceRecv )
{
	byte abBuf[100] = { 0, };

    /*
     *  �ű� �������α׷��� ����PC���� ������
     */
	if ( pstNewImgNVoiceRecv->boolIsImgRecv == TRUE )
	{
		sprintf( abBuf, "cp %s/%s %s/%s",
			BUS_EXECUTE_DIR, BUS_EXECUTE_BACKUP_FILE,
			BUS_BACKUP_DIR, BUS_EXECUTE_BEFORE_BACKUP_FILE );
		system( abBuf );

		sprintf( abBuf, "%s/%s", BUS_EXECUTE_DIR, BUS_EXECUTE_BACKUP_FILE );
		unlink( abBuf );

		sprintf( abBuf, "cp %s/%s %s/%s",
			BUS_EXECUTE_DIR, BUS_EXECUTE_FILE,
			BUS_EXECUTE_DIR, BUS_EXECUTE_BACKUP_FILE );
		system( abBuf );
	}

	/*
	 * ���������۱� ���α׷� �ۼ����߿��� WAITING�Ѵ�.
	 */
/*	while ( gpstSharedInfo->boolIsKpdImgRecv == TRUE )
	{
		usleep(100000);
	}	*/

    /*
     *  Main Process Exit Log ����
     */
	LogWrite( ERR_SETUP_DRIVE_MAIN_EXIT );

    /*
     *  Delete Shared Memory Buffer
     */
	MemoryRemoval();

    /*
     *  Close Peripherial Device
     */
	ClosePeripheralDevices();

	unlink( STATUS_FLAG_FILE );
	system( "sync" );

	system( "reset" );
}

