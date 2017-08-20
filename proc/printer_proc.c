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
*  PROGRAM ID :       printer_proc.c                                           *
*                                                                              *
*  DESCRIPTION:       This program processes Receipt Printing                  *
*                                                                              *
*  ENTRY POINT:       CommPrinter()               ** mandatory **              *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
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
* ---------- ---------------------------- ------------------------------------ *
* 2005/08/13 Solution Team  Gwan Yul Kim  Initial Release                      *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/
#include "printer_proc.h"
#include "load_parameter_file_mgt.h"
#include "version_file_mgt.h"

/*******************************************************************************
*  Declaration of Global Variables                                             *
*******************************************************************************/
static int gfdPrinterDevice = 0;			// Printer Device File Descriptor

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static void PrintOutCashEnt( char* pchRecvData );
static void PrintOutTermInfo( void );
static short WritePrinter( char* pcData, word wDataSize );
static void PrintOutStr( byte *abTitle, byte *abStr, byte bStrSize );
static void PrintOutDWORD( byte *abTitle, dword dwInput );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      CommPrinter                                              *
*                                                                              *
*  DESCRIPTION:       Printing Program Main                                    *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void CommPrinter( void )
{
	short sRetVal = 0;
	int	nReadSize = 0;
	MSGQUEUE_DATA stMsgQueueData;

	while ( 1 )
	{
		gnMsgQueue = msgget( 900000 + 1, IPC_CREAT | IPC_PERM );
		if ( gnMsgQueue >= 0 )
		{
			break;
		}
		else
		{
			perror( "Message Queue failed in CommPrinter : " );
			printf( "Msgget gnMsgQueue error \n" );
			DisplayASCInUpFND( "888888" );
			DisplayASCInDownFND( "888888" );
			Buzzer( 5, 50000 );
			VoiceOut( VOICE_CHECK_TERM );		// Msg of Checking Terminal.

			sleep( 3 );
		}
	}

	while ( 1 )
	{
		sRetVal = OpenPrinter( &gfdPrinterDevice, 38400 );
		if ( sRetVal >= 0 )
		{
			break;
		}
		else
		{
			printf( "\nPrinter Device Open Failed!!\n" );
			DisplayASCInUpFND( "888888" );
			DisplayASCInDownFND( "888888" );
			Buzzer( 5, 50000 );
			VoiceOut( VOICE_CHECK_TERM );		// Msg of Checking Terminal.

			sleep( 3 );
		}
	}

	// 2006-02-07 Gykim
	ClosePrinter( &gfdPrinterDevice );

	// Main���� �����޸� ����Ÿ�� ä����ִ� �ð�����
	sleep( 5 );

	// �޼���ť ȹ��
	memset( &stMsgQueueData, 0x00, sizeof(MSGQUEUE_DATA) );
	while ( 1 )
	{
		// �޼���ť �б�
		nReadSize = msgrcv( gnMsgQueue,
							&stMsgQueueData,
							MSGQUEUE_MAX_DATA + 1,
							0,
							0 );
	    if ( nReadSize < 0 )
	    {
			perror( "msgrcv failed\n PrinterProcess Exit!! \n" );
			printf( "\nPrinter Device Open Failed!!\n" );
			DisplayASCInUpFND( "888888" );
			DisplayASCInDownFND( "888888" );
			Buzzer( 5, 50000 );
			VoiceOut( VOICE_CHECK_TERM );		// Msg of Checking Terminal.

			sleep( 3 );
		}
		else if ( nReadSize == 0 )
		{
	    	usleep( 500 );
	    	continue;
	    }
	    else
	    {
	    	if ( stMsgQueueData.lMsgType == PRINT_RECEIPT )
	    	{
	    		PrintOutCashEnt( stMsgQueueData.achMsgData );
				usleep( 1000000 );
	    	}
	    	else if ( stMsgQueueData.lMsgType == PRINT_TERM_INFO )
	    	{
	    		PrintOutTermInfo();
	    	}
	    	memset( &stMsgQueueData, 0x00, sizeof(MSGQUEUE_DATA) );
	    }
	}

	// �޼���ť �ݳ�
	msgctl( gnMsgQueue, IPC_RMID, NULL );

	//ClosePrinter( &gfdPrinterDevice );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      PrintOutCashEnt                                          *
*                                                                              *
*  DESCRIPTION:       Print Receipt of Cash GetOn.                             *
*                                                                              *
*  INPUT PARAMETERS:  char* pchRecvData                                        *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void PrintOutCashEnt( char* pchRecvData )
{
	short sRetVal = 0;
	char achPrintBuf[255];
	char achDataBuf[255];

	MSGQUEUE_RECEIPT_DATA stMsgQueueReceipt;

	sRetVal = OpenPrinter( &gfdPrinterDevice, 38400 );

	memcpy( &stMsgQueueReceipt, pchRecvData, sizeof( MSGQUEUE_RECEIPT_DATA ) );

	sprintf( achPrintBuf,"%c%c��ȸ ������(������)%c%c", 0x11, 0x13, 0x12, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	sprintf( achPrintBuf, "    %c", 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf, 0x00, sizeof(achPrintBuf) );
	memset( achDataBuf, 0x00, sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abBizNo, 3 );
	achDataBuf[3] = '-';
	memcpy( achDataBuf + 4, stMsgQueueReceipt.abBizNo + 3, 2 );
	achDataBuf[6] = '-';
	memcpy( achDataBuf + 7, stMsgQueueReceipt.abBizNo + 5, 5 );
	sprintf( achPrintBuf, "����� ��ȣ   :  %s%c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abDateTime, 4 );
	memcpy( achDataBuf + strlen(achDataBuf), "��", strlen("��") );
	memcpy( achDataBuf + strlen(achDataBuf),
			stMsgQueueReceipt.abDateTime + 4, 2 );
	memcpy( achDataBuf + strlen(achDataBuf), "��", strlen("��") );
	memcpy( achDataBuf + strlen(achDataBuf),
			stMsgQueueReceipt.abDateTime + 6, 2 );
	memcpy( achDataBuf + strlen(achDataBuf), "��", strlen("��") );
	sprintf( achPrintBuf, "���� ����    :  %s%c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abDateTime + 8, 2 );
	memcpy( achDataBuf + strlen(achDataBuf), " : ", strlen(" : ") );
	memcpy( achDataBuf + strlen(achDataBuf),
			stMsgQueueReceipt.abDateTime + 10, 2 );
	memcpy( achDataBuf + strlen(achDataBuf), " : ", strlen(" : ") );
	memcpy( achDataBuf + strlen(achDataBuf),
			stMsgQueueReceipt.abDateTime + 12, 2 );
	sprintf( achPrintBuf, "���� �ð�    :  %s %c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abVehicleNo,
		sizeof(stMsgQueueReceipt.abVehicleNo) );
	sprintf( achPrintBuf, "���� ��ȣ    :  %s %c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abTranspMethodCodeName,
		sizeof(stMsgQueueReceipt.abTranspMethodCodeName) );
	sprintf( achPrintBuf, "���� Ÿ��    :  %s %c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abUserTypeName,
		sizeof(stMsgQueueReceipt.abUserTypeName) );
	sprintf( achPrintBuf, "�°� ����    :  %s %c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.achBusStationName,
		sizeof(stMsgQueueReceipt.achBusStationName) );
	sprintf( achPrintBuf, "�� �� ��    :  %s %c", achDataBuf, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	memset( achDataBuf,0x00,sizeof(achDataBuf) );
	memcpy( achDataBuf, stMsgQueueReceipt.abFare,
			sizeof(stMsgQueueReceipt.abFare) );
	sprintf( achPrintBuf, "��    ��    : %c%c %s��%c%c",
		0x11,0x13, achDataBuf, 0x12, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	sprintf( achPrintBuf, "      �̿��� �ּż� �����մϴ�.%c", 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	memset( achPrintBuf,0x00,sizeof(achPrintBuf) );
	sprintf( achPrintBuf, "        �ѱ� ����Ʈī�� (��)%c%c%c",
			 0x0d, 0x14, 0x0d );
	sRetVal = WritePrinter( achPrintBuf, strlen(achPrintBuf) );

	ClosePrinter( &gfdPrinterDevice );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      PrintOutTermInfo                                         *
*                                                                              *
*  DESCRIPTION:       Print Terminal Info to the Printer.                      *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void PrintOutTermInfo( void )
{
	int nIndex = 0;
	int fdData = 0;
	long lFreeMemory = 0;
	char abDataBuf[256] = {0, };
	VER_INFO stVerInfo;
	COMM_INFO stServerIP;

	memset( &stVerInfo, 0x00, sizeof( VER_INFO ) );
	memset( &stServerIP, 0x20, sizeof( COMM_INFO ) );

	OpenPrinter( &gfdPrinterDevice, 38400 );

	// �������� �ε�
	LoadVehicleParm();

	// ���������� �ε�
	LoadStationInfo();

	// �������� �ε�
	LoadVerInfo( &stVerInfo );

	// HEADER��� //////////////////////////////////////////////////////////////
	sprintf( abDataBuf, "%c%c    �ܸ��� ���� %c", 0x11, 0x13, 0x12 );
	PrintOutStr( abDataBuf, "", 0 );
	PrintOutStr( "    ", "", 0 );

	TimeT2ASCDtime( gpstSharedInfo->tTermTime, abDataBuf );
	PrintOutStr(	"��������   :  ",			abDataBuf, 14 );
	PrintOutStr(	"���� ID    :  ",			gpstSharedInfo->abMainTermID,
		sizeof( gpstSharedInfo->abMainTermID ) );

	for ( nIndex = 0; nIndex < gbSubTermCnt; nIndex++ )
	{
		PrintOutStr( "���� ID    :  ", gpstSharedInfo->abSubTermID[nIndex],
			sizeof( gpstSharedInfo->abSubTermID[nIndex] ) );
	}

	PrintOutStr(	"���������۱� ID  :  ",		gpstSharedInfo->abDriverOperatorID,
		sizeof( gpstSharedInfo->abDriverOperatorID ) );
	PrintOutStr(	"�뼱 ID    :  ",			gstVehicleParm.abRouteID,
		sizeof( gstVehicleParm.abRouteID ) );
	PrintOutStr(	"�뼱��    :  ",			gstStationInfoHeader.abTranspMethodName,
		sizeof( gstStationInfoHeader.abTranspMethodName ) );
	PrintOutStr(	"������ȣ    :  ",			gstVehicleParm.abVehicleNo,
		sizeof( gstVehicleParm.abVehicleNo ) );
	PrintOutStr(	"�������� ID    :  ",		gstVehicleParm.abTranspBizrID,
		sizeof( gstVehicleParm.abTranspBizrID ) );
	PrintOutStr(	"�������ڸ�    :  ",		gstVehicleParm.abTranspBizrNm,
		sizeof( gstVehicleParm.abTranspBizrNm ) );
	PrintOutStr(	"��������ڵ�    :  ",		gstVehicleParm.abTranspMethodCode,
		sizeof( gstVehicleParm.abTranspMethodCode ) );

	fdData = open( SETUP_FILE, O_RDONLY );
	if ( fdData < 0 )
	{
		printf("[PrintOutTermInfo] setup.dat ���� OPEN ����\n");
	}
	else
	{
		read( fdData, &stServerIP, sizeof(COMM_INFO) );
		close( fdData );
	}
	sprintf( abDataBuf, "%d.%d.%d.%d",
						GetINTFromASC(stServerIP.abDCSIPAddr, 3),
						GetINTFromASC(stServerIP.abDCSIPAddr + 3, 3),
						GetINTFromASC(stServerIP.abDCSIPAddr + 6, 3),
						GetINTFromASC(stServerIP.abDCSIPAddr + 9, 3) );
	PrintOutStr(	"���� IP    :  ",			abDataBuf,
		strlen( abDataBuf ) );
	PrintOutStr(	"��������    :  ",			MAIN_RELEASE_VER, 4 );
	for ( nIndex = 0; nIndex < gbSubTermCnt; nIndex++ )
	{
		PrintOutStr( "��������    :  ", gpstSharedInfo->abSubVer[nIndex],
			sizeof( gpstSharedInfo->abSubVer[nIndex] ) );
	}

	PrintOutStr(	"�����ڹ���    :  ",		gpstSharedInfo->abKpdVer,
		sizeof( gpstSharedInfo->abKpdVer ) );
	PrintOutStr(	"����SAMID    :  ",			gpstSharedInfo->abMainPSAMID,
		sizeof( gpstSharedInfo->abMainPSAMID ) );

	for ( nIndex = 0; nIndex < gbSubTermCnt; nIndex++ )
	{
		PrintOutStr( "����SAMID    :  ", gpstSharedInfo->abSubPSAMID[nIndex],
			sizeof( gpstSharedInfo->abSubPSAMID[nIndex] ) );
	}

	PrintOutStr(	"Ŀ�ι���    :  ",			gabKernelVer, 2 );

	lFreeMemory = MemoryCheck();
	if ( lFreeMemory < 0 )
	{
		lFreeMemory = 0;
	}
	sprintf( abDataBuf, "%ldM", lFreeMemory );
	PrintOutStr(	"�����뷮    :  ",			abDataBuf,
		strlen( abDataBuf ) );

	// �ʼ������ ////////////////////////////////////////////////////////////
	PrintOutStr( "+ �ʼ������ -----------------------", "", 0 );

	PrintOutStr(	"�������� ���� :  ",		stVerInfo.abVehicleParmVer,
		sizeof( stVerInfo.abVehicleParmVer ) );
	PrintOutStr(	"�뼱���� ���� :  ",		stVerInfo.abRouteParmVer,
		sizeof( stVerInfo.abRouteParmVer ) );
	PrintOutStr(	"������� ���� :  ",		stVerInfo.abBasicFareInfoVer,
		sizeof( stVerInfo.abBasicFareInfoVer ) );
	PrintOutStr(	"���������� ���� :  ",		stVerInfo.abBusStationInfoVer,
		sizeof( stVerInfo.abBusStationInfoVer ) );

	// ��Ÿ����� ////////////////////////////////////////////////////////////
	PrintOutStr( "+ ��Ÿ����� -----------------------", "", 0 );

	PrintOutStr(	"���ҹ���� ���� :  ",		stVerInfo.abPrepayIssuerInfoVer,
		sizeof( stVerInfo.abPrepayIssuerInfoVer ) );
	PrintOutStr(	"�ĺҹ���� ���� :  ",		stVerInfo.abPostpayIssuerInfoVer,
		sizeof( stVerInfo.abPostpayIssuerInfoVer ) );
	PrintOutStr(	"�������ȿ�Ⱓ ���� :  ",	stVerInfo.abIssuerValidPeriodInfoVer,
		sizeof( stVerInfo.abIssuerValidPeriodInfoVer ) );
	PrintOutStr(	"������������ ���� :  ",	stVerInfo.abDisExtraInfoVer,
		sizeof( stVerInfo.abDisExtraInfoVer ) );
	PrintOutStr(	"�������� ���� :  ",		stVerInfo.abHolidayInfoVer,
		sizeof( stVerInfo.abHolidayInfoVer ) );
	PrintOutStr(	"ȯ���������� ���� :  ",	stVerInfo.abXferApplyInfoVer,
		sizeof( stVerInfo.abXferApplyInfoVer ) );

	// BL/PL���� ///////////////////////////////////////////////////////////////
	PrintOutStr( "+ BL/PL���� --------------------------", "", 0 );

	PrintOutStr(	"���� BL ���� :  ",			stVerInfo.abMasterBLVer,
		sizeof( stVerInfo.abMasterBLVer ) );
	PrintOutDWORD(	"���� BL size :  ",			GetFileSize("c_fi_bl.dat") );
	PrintOutStr(	"���� BL ���� :  ",			stVerInfo.abUpdateBLVer,
		sizeof( stVerInfo.abUpdateBLVer ) );

	PrintOutStr(	"���� ������PL ���� :  ",	stVerInfo.abMasterPrepayPLVer,
		sizeof( stVerInfo.abMasterPrepayPLVer ) );
	PrintOutDWORD(	"���� ������PL size :  ",	GetFileSize("c_fa_pl.dat") );
	PrintOutStr(	"���� �ĺ�PL ���� :  ",		stVerInfo.abMasterPostpayPLVer,
		sizeof( stVerInfo.abMasterPostpayPLVer ) );
	PrintOutDWORD(	"���� �ĺ�PL size :  ",		GetFileSize("c_fd_pl.dat") );
	PrintOutStr(	"���� �ż���PL ���� :  ",	stVerInfo.abMasterAIVer,
		sizeof( stVerInfo.abMasterAIVer ) );
	PrintOutDWORD(	"���� �ż���PL size :  ",	GetFileSize("c_fi_ai.dat") );

	PrintOutStr(	"���� PL ���� :  ",			stVerInfo.abUpdatePLVer,
		sizeof( stVerInfo.abUpdatePLVer ) );
	PrintOutStr(	"���� �ż���PL ���� :  ",	stVerInfo.abUpdateAIVer,
		sizeof( stVerInfo.abUpdateAIVer ) );

	// ��Ÿ ���� ///////////////////////////////////////////////////////////////
	PrintOutStr( "+ ------------------------------------", "", 0 );

	PrintOutStr(	"�������� ���� :  ",		gpstSharedInfo->abMainVoiceVer,
		sizeof( gpstSharedInfo->abMainVoiceVer ) );
	for ( nIndex = 0; nIndex < gbSubTermCnt; nIndex++ )
	{
		PrintOutStr( "�������� ���� :  ", gpstSharedInfo->abSubVoiceVer[nIndex],
			sizeof( gpstSharedInfo->abSubVoiceVer[nIndex] ) );
	}

	fdData = open( PG_LOADER_VER_FILE, O_RDONLY );
	if ( fdData < 0 )
	{
		printf("[PrintOutTermInfo] pgver.dat ���� OPEN ����\n");
	}
	else
	{
		read( fdData, abDataBuf, 4 );
		close( fdData );
	}
	PrintOutStr(	"PgLoader ���� :  ",		abDataBuf, 4 );

	// TAIL ��� ///////////////////////////////////////////////////////////////
	sprintf( abDataBuf, "        �ѱ� ����Ʈī�� (��)%c%c", 0x0d, 0x14 );
	PrintOutStr( abDataBuf, "", 0 );

	ClosePrinter( &gfdPrinterDevice );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WritePrinter                                             *
*                                                                              *
*  DESCRIPTION:       This program writes pcData to Printer					   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  Author : GwanYul Kim	     												   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short WritePrinter( char* pcData, word wDataSize )
{
	short sRetVal = 0;

	sRetVal = write( gfdPrinterDevice, pcData, wDataSize );

	return sRetVal;
}

static void PrintOutStr( byte *abTitle, byte *abStr, byte bStrSize )
{
	byte abPrintBuf[256] = {0, };
	byte abBuf[256] = {0, };

	usleep( 50000 );
	memcpy( abBuf, abStr, bStrSize );
	sprintf( abPrintBuf, "%s%s%c", abTitle, abBuf, 0x0d );
	printf( "%s\r\n", abPrintBuf );
	WritePrinter( abPrintBuf, strlen( abPrintBuf ) );
}

static void PrintOutDWORD( byte *abTitle, dword dwInput )
{
	byte abPrintBuf[256] = {0, };

	usleep( 50000 );
	sprintf( abPrintBuf, "%s%lu%c", abTitle, dwInput, 0x0d );
	printf( "%s\r\n", abPrintBuf );
	WritePrinter( abPrintBuf, strlen( abPrintBuf ) );
}

