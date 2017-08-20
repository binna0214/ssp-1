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
*  PROGRAM ID :       card_proc.c                                              *
*                                                                              *
*  DESCRIPTION:       ī���� �±׿��� üũ�������� ó����� ��±��� ī��ó����*
*                     ���õ� ��� ������ ó���Ѵ�.                             *
*                                                                              *
*  ENTRY POINT:       void CardProc( void )                                    *
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
* 2005/07/13 Solution Team  Boohyeon Jeon Initial Release                      *
*                                                                              *
*******************************************************************************/

#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "main.h"
#include "trans_proc.h"
#include "card_proc_mif_prepay.h"
#include "card_proc_mif_postpay.h"
#include "card_proc_mif_tour.h"
#include "card_proc_sc.h"
#include "card_proc_util.h"
#include "write_trans_data.h"

#include "card_proc.h"

#define KEYPAD_PRINT_0_WON				"000001"

/*******************************************************************************
*  Declaration of Function Prototypes                                          *
*******************************************************************************/
static void DisplayResult( TRANS_INFO *pstTransInfo, short sErrCode );
static void DisplaySuccess( TRANS_INFO *pstTransInfo );
static void DisplayError( TRANS_INFO *pstTransInfo, short sErrCode );

/*******************************************************************************
*  Declaration of Global Valiables                                             *
*******************************************************************************/
word gwRetagCardCnt = 0;				// ī�� ���±� ���� Ƚ��

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CardProc                                                 *
*                                                                              *
*  DESCRIPTION:       ī���� �±׿��� üũ�������� ó����� ��±��� ī��ó����*
*                     ���õ� ��� ������ ó���Ѵ�.                             *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
void CardProc( void )
{
	short	sResult		= SUCCESS;	// �Լ� ���� ���
	byte	bCardType	= 0;		// ī�� ����
	time_t	tNowDtime	= 0;		// ���� �ð�
	TRANS_INFO stTransInfo;			// ī����������ü

	static int nTempCnt = 0;

	// ī����������ü �ʱ�ȭ
	memset( ( byte * )&stTransInfo, 0, sizeof( TRANS_INFO ) );

	// �ű�ȯ�������� �̿� �ð��� ����ð� ����
	GetRTCTime( &tNowDtime );
	stTransInfo.stNewXferInfo.tEntExtDtime = tNowDtime;

#ifdef TEST_PRINT_CARD_PROC_TIME
	InitWatch();
	StopWatch();
#endif

	// ī�������Ǻ� ////////////////////////////////////////////////////////////
	sResult = GetCardType( &bCardType, &stTransInfo.dwChipSerialNo );
	if (sResult == ERROR_CARD_NOTAG)
	{
		if (gboolIsMainTerm)
		{
			if (nTempCnt == 0)
			{
				DisplayDWORDInDownFND( 0 );
			}
	
			if (nTempCnt > 15)
			{
				DisplayASCInDownFND("");
				nTempCnt = 0;
			}
			else
			{
				nTempCnt++;
			}
		}
		return;
	}

	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERROR_CARD_NOTAG:
				return;
			case ERROR_NOT_ONECARD:
				sResult = ERR_CARD_PROC_NOT_ONE_CARD;
				break;
			case ERROR_CANNOT_USE:
				sResult = ERR_CARD_PROC_CANNOT_USE;
				break;
			default:
				sResult = ERR_CARD_PROC_RETAG_CARD;
				break;
		}
		goto DISPLAY;
	}

#ifdef TEST_PRINT_CARD_PROC_TIME
	printf( "[CardProc] ī������ �Ǻ� : %f sec\n", StopWatch() );
#endif

	// ī��ó���� ���۵Ǹ� ���������۱⿡�� ���ν� ������ �Ұ����ϵ��� �÷���
	// ������ TRUE�� ����
	gpstSharedInfo->boolIsCardProc = TRUE;

	// ī��READ ////////////////////////////////////////////////////////////////
	switch ( bCardType )
	{
		case ( CARDTYPE_MIF | CARDTYPE_PREPAY ):
			sResult = MifPrepayRead( &stTransInfo );
			break;
		case ( CARDTYPE_MIF | CARDTYPE_POSTPAY ):
			sResult = MifPostpayRead( &stTransInfo );
			break;
		case ( CARDTYPE_MIF | CARDTYPE_TOUR ):
			sResult = MifTourRead( &stTransInfo );
			break;
		case CARDTYPE_SC:
			sResult = SCRead( &stTransInfo );
			break;
		default:
			sResult = ERR_CARD_PROC_CANNOT_USE;
			goto DISPLAY;
	}

#if defined( RELEASEMODE ) && defined( TEST_PRINT_CARD_PROC_INFO )
	if ( strlen( stTransInfo.abCardNo ) != 0 &&
		 sResult != ERR_CARD_PROC_NOT_APPROV )
	{
		PrintlnASC( "\n\n## ī���ȣ : ", stTransInfo.abCardNo,
			sizeof( stTransInfo.abCardNo ) );
		printf( "\n## ����ȯ������ ##\n" );
		PrintXferInfo( &stTransInfo.stPrevXferInfo );
	}
#endif
	if ( sResult == ERR_CARD_PROC_LOG )
	{
	    printf( "[CardProc] ����LOG�� ī�������� ��ó����\n" );
	    goto WRITE;
	}
	if ( sResult != SUCCESS )
	{
		goto DISPLAY;
	}

	// ������ܺ� ���Ұ�ī�� ó�� ////////////////////////////////////////////
	// ��Ƽ�������
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == TRUE )
	{
		// ��Ƽ��������̸鼭 �ż���/������ī�尡 �ƴ� ���
		// '����� �� ���� ī���Դϴ�' ���� ���
		if ( stTransInfo.bCardType != TRANS_CARDTYPE_SC_PREPAY &&
			 stTransInfo.bCardType != TRANS_CARDTYPE_MIF_TOUR )
		{
			printf( "[CardProc] ��Ƽ��������̸鼭 �ż���/������ī�尡 �ƴ� " );
			printf( "��� '����� �� ���� ī���Դϴ�' ���� ���\n" );
			sResult = ERR_CARD_PROC_CANNOT_USE;
			goto DISPLAY;
		}

		// ��Ƽ��������̸鼭 �ż���ī���ε��� ����Է��� ���� ���� ���
		// '�������� ������ �ּ���' ���� ���
		if ( stTransInfo.bCardType == TRANS_CARDTYPE_SC_PREPAY &&
			 gstCityTourBusTicketInfo.boolIsTicketInput == FALSE )
		{
			printf( "[CardProc] ��Ƽ��������̸鼭 �ż���ī���ε��� " );
			printf( "����Է��� ���� ���� ��� '�������� ������ �ּ���' " );
			printf( "���� ���\n" );
			sResult = ERR_CARD_PROC_INPUT_TICKET;
			goto DISPLAY;
		}

		// ��Ƽ��������̸鼭 ����Է��� �� ���·� ������ī�尡 �±׵Ǵ� ���
		// '��Ƽ�н��Դϴ�' ���� ���
		if ( stTransInfo.bCardType == TRANS_CARDTYPE_MIF_TOUR &&
			 gstCityTourBusTicketInfo.boolIsTicketInput == TRUE )
		{
			printf( "[CardProc] ��Ƽ��������̸鼭 ����Է��� �� ���·� " );
			printf( "������ī�尡 �±׵Ǵ� ��� '��Ƽ�н��Դϴ�' " );
			printf( "���� ���\n" );
			sResult = ERR_CARD_PROC_CITY_PASS_CARD;
			goto DISPLAY;
		}

	}
	// ��������
	else if ( IsWideAreaBus( gstVehicleParm.wTranspMethodCode ) == TRUE )
	{
		// ���������̸鼭 ������ī���� ���
		// '����� �� ���� ī���Դϴ�' ���� ���
		if ( stTransInfo.bCardType == TRANS_CARDTYPE_MIF_TOUR )
		{
			printf( "[CardProc] ���������̸鼭 ������ī���� ��� " );
			printf( "'����� �� ���� ī���Դϴ�' ���� ���\n" );
			sResult = ERR_CARD_PROC_CANNOT_USE;
			goto DISPLAY;
		}
	}

	// ���ν����� ���� /////////////////////////////////////////////////////////
	// ��Ƽ�������
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == TRUE )
	{
		if ( gstCityTourBusTicketInfo.boolIsTicketInput == TRUE )
		{
			// ���ν����� ����
			memcpy( stTransInfo.abUserType, gstCityTourBusTicketInfo.abUserType,
				sizeof( gstCityTourBusTicketInfo.abUserType ) );
			memcpy( stTransInfo.abUserCnt, gstCityTourBusTicketInfo.abUserCnt,
				sizeof( gstCityTourBusTicketInfo.abUserCnt ) );
		}
		else
		{
			// PL����������� �����ϰ�, ����� ���� 1������ ����
			stTransInfo.abUserType[0] = stTransInfo.bPLUserType;
			stTransInfo.abUserCnt[0] = 1;
		}
	}
	else
	{
		// PL����������� ī��READ���� �������Ƿ� ī��READ �Ϸ� �� ������
		// ������ī���� ���
		if ( stTransInfo.bCardType == TRANS_CARDTYPE_MIF_TOUR )
		{
			if ( gstMultiEntInfo.boolIsMultiEnt == TRUE &&
				 ( gstMultiEntInfo.abUserCnt[0] +
				   gstMultiEntInfo.abUserCnt[1] +
				   gstMultiEntInfo.abUserCnt[2] ) != 1 )
			{
				printf( "[CardProc] ������ī���� ��� 2�� �̻��� ���ν��� " );
				printf( "�����Ǿ����Ƿ� '���ν��� �Ұ����� ī���Դϴ�' ���� " );
				printf( "���\n" );
				sResult = ERR_CARD_PROC_CANNOT_MULTI_ENT;
				goto DISPLAY;
			}
			// �°����� �ʱ�ȭ
			memset( stTransInfo.abUserType, 0, sizeof( stTransInfo.abUserType ) );
			memset( stTransInfo.abUserCnt, 0, sizeof( stTransInfo.abUserCnt ) );

			stTransInfo.abUserType[0] = stTransInfo.bPLUserType;
			stTransInfo.abUserCnt[0] = 1;
		}
		// ���ν��Է��� ���
		else if ( gstMultiEntInfo.boolIsMultiEnt == TRUE )
		{
			// ���ν����� ����
			memcpy( stTransInfo.abUserType, gstMultiEntInfo.abUserType,
				sizeof( stTransInfo.abUserType ) );
			memcpy( stTransInfo.abUserCnt, gstMultiEntInfo.abUserCnt,
				sizeof( stTransInfo.abUserCnt ) );
		}
		// ���ν¹��Է��� ���
		else
		{
			// PL����������� �����ϰ�, ����� ���� 1������ ����
			stTransInfo.abUserType[0] = stTransInfo.bPLUserType;
			stTransInfo.abUserCnt[0] = 1;
		}
	}

	PrintTransInfo( &stTransInfo );

	// �ŷ�ó�� ////////////////////////////////////////////////////////////////
	sResult = TransProc( &stTransInfo );

	// �߰��������� ��� ///////////////////////////////////////////////////////
	// 1. WRITE�� ������ �߻��Ͽ� �ٽ� �±��ϸ� ����� �߰��������� �÷��׸�
	//    ����� ( �����޸𸮿��� ����� ������ ��������� )
	// 2. TD WRITE�� �߰��������� �÷��� ������ �����
	//    ( ����/���� ����� ������ ��쿡�� )
	stTransInfo.boolIsAddEnt = gstMultiEntInfo.boolIsAddEnt;

	// ���ν¿��� ��� /////////////////////////////////////////////////////////
	stTransInfo.boolIsMultiEnt = gstMultiEntInfo.boolIsMultiEnt;
	if ( sResult != SUCCESS )
	{
		goto DISPLAY;
	}

	WRITE:

#ifdef TEST_WRITE_SLEEP
	printf( "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n" );
	printf( "WRITE START\n" );
	sleep( 2 );
#endif

	// ī��WRITE ///////////////////////////////////////////////////////////////
	switch ( bCardType )
	{
		case ( CARDTYPE_MIF | CARDTYPE_PREPAY ):
			sResult = MifPrepayWrite( &stTransInfo );
			break;
		case ( CARDTYPE_MIF | CARDTYPE_POSTPAY ):
			sResult = MifPostpayWrite( &stTransInfo );
			break;
		case ( CARDTYPE_MIF | CARDTYPE_TOUR ):
			sResult = MifTourWrite( &stTransInfo );
			break;
		case CARDTYPE_SC:
			sResult = SCWrite( &stTransInfo );
			break;
		default:
			sResult = ERR_CARD_PROC_CANNOT_USE;
			goto DISPLAY;
	}

#if defined( RELEASEMODE ) && defined( TEST_PRINT_CARD_PROC_INFO )
	printf( "\n## �ű�ȯ������ ##\n" );
	PrintXferInfo( &stTransInfo.stNewXferInfo );
#endif

	PrintTransInfo( &stTransInfo );
	if ( sResult != SUCCESS )
	{
		goto DISPLAY;
	}

#ifdef TEST_PRINT_CARD_PROC_TIME
	printf( "[CardProc] ī��ó�� �Ϸ� : %f sec\n\n", StopWatch() );
#endif

	// �߰���������/���ν¿���/���ν����� �������� ���� ////////////////////////
	// 0. WRITE������ ���Ͽ� ������ ���
	if ( stTransInfo.bWriteErrCnt > 0 )
	{
		// 1. ����� �߰��������ΰ� TRUE�̸�
		// 2. ����� ���ν¿��ΰ� TRUE�̰�
		// 2. �߰������� ���� ( ����|���� ) ��� �� ������ �����ϴ� ���
		// -> �߰��������� ���� �� ���ν����� �Է� �ð� ����
		if ( stTransInfo.boolIsAddEnt &&
			 stTransInfo.boolIsMultiEnt &&
			 !IsEnt( stTransInfo.stNewXferInfo.bEntExtType ) )
		{
			DebugOut( "[CardProc] �߰��������� ���������� ������\n" );

			gstMultiEntInfo.boolIsAddEnt = TRUE;		// �߰���������

			// main()���κ����� Ÿ�Ӿƿ����� ���� ���ν����� �ʱ�ȭ�� �����ϱ�
			// ���Ͽ� ���ν����� �Է� �ð��� ����ð����� ����
			gstMultiEntInfo.tInputDatetime = gpstSharedInfo->tTermTime;
		}

		// 1. ����� ���ν¿��ΰ� TRUE�� ���
		// -> ���ν¿���/���ν����� ����
		if ( stTransInfo.boolIsMultiEnt )
		{
			byte i = 0;

			DebugOut( "[CardProc] ���ν¿���/���ν����� ���� ���������� " );
			DebugOut( "������\n" );

			gstMultiEntInfo.boolIsMultiEnt = TRUE;		// ���ν¿���

			// ���ν�����
			for ( i = 0; i < 3; i++ )
			{
				gstMultiEntInfo.abUserType[i] = stTransInfo.abUserType[i];
				gstMultiEntInfo.abUserCnt[i] = stTransInfo.abUserCnt[i];
			}
		}
	}

	// �ŷ�������� ////////////////////////////////////////////////////////////
	WriteTransData( &stTransInfo );
LogMain("�ŷ���������\n");
	// ī���ȣLOG�� ī���ȣ �߰� /////////////////////////////////////////////
	AddCardNoLog( stTransInfo.abCardNo );

	DISPLAY:

	// ī��ó���� �Ϸ�Ǹ� ���������۱⿡�� ���ν� ������ �����ϵ��� �÷���
	// ������ FALSE�� ����
	gpstSharedInfo->boolIsCardProc = FALSE;

	// ó�������� ////////////////////////////////////////////////////////////
	DisplayResult( &stTransInfo, sResult );

	// ��Ƽ������� �������Է������� �ʱ�ȭ�� //////////////////////////////////
	memset( &gstCityTourBusTicketInfo, 0x00,
		sizeof( CITY_TOUR_BUS_TICKET_INFO) );
	// �߰������� �ƴ� ��� ���ν������� Ŭ������ //////////////////////////////
	// stTransInfo ������ �Ҹ�ǹǷ� ���� Ŭ������ �ʿ� ����
	if ( gstMultiEntInfo.boolIsAddEnt == FALSE )
	{
		memset( &gstMultiEntInfo, 0x00, sizeof( MULTI_ENT_INFO ) );
		DisplayDWORDInUpFND( 0 );
 		DisplayDWORDInDownFND( 0 );
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       DisplayResult                                            *
*                                                                              *
*  DESCRIPTION:       ī��ó���� ��� ������ �Ϸ�Ǿ�����, ó�������          *
*                     ������ ����, FND�� ����Ѵ�.                             *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü�� ������                   *
*                     sErrCode - �����ڵ�                                      *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void DisplayResult( TRANS_INFO *pstTransInfo, short sErrCode )
{
	if ( sErrCode == SUCCESS )
		DisplaySuccess( pstTransInfo );
	else
		DisplayError( pstTransInfo, sErrCode );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       DisplaySuccess                                           *
*                                                                              *
*  DESCRIPTION:       ī��ó���� ���������� �Ϸ�� ��� ó�������             *
*                     ������ ����, FND�� ����Ѵ�.                             *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü�� ������                   *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void DisplaySuccess( TRANS_INFO *pstTransInfo )
{
	byte bTempUserType = 0;
	word wTotalUserCnt = 0;
	byte abBuf[6] = {0, };

	// �߰������� ��� ó������� ������� ����
	if ( pstTransInfo->boolIsAddEnt &&
		!IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
		return;

	// ���������۱⿡ ó���ݾ� ���� ����� /////////////////////////////////////
	if ( pstTransInfo->stNewXferInfo.dwFare != 0 )
	{
		DWORD2ASCWithFillLeft0( pstTransInfo->stNewXferInfo.dwFare, abBuf,
			sizeof( abBuf ) );
		ASC2BCD( abBuf, gpstSharedInfo->abTotalFare, sizeof( abBuf ) );
	}
	else
	{
		/*
		 * ���������۱⿡ 0���� ����ϱ� ���ؼ��� "000001"�� �����Ѵ�.
		 */
		ASC2BCD( KEYPAD_PRINT_0_WON, gpstSharedInfo->abTotalFare,
			strlen( KEYPAD_PRINT_0_WON ) );
	}

	wTotalUserCnt =
		pstTransInfo->abUserCnt[0] +
		pstTransInfo->abUserCnt[1] +
		pstTransInfo->abUserCnt[2];

	// O LED ON
	OLEDOn();

	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
	 	DisplayDWORDInUpFND( pstTransInfo->bMifTourUseCnt );
		DisplayDWORDInDownFND(
			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt );
	}
	else
	{
	 	DisplayDWORDInUpFND( pstTransInfo->stNewXferInfo.dwFare );
		DisplayDWORDInDownFND( pstTransInfo->stNewXferInfo.dwBal );
	}

	// �׽�Ʈī�� //////////////////////////////////////////////////////////////
	if ( pstTransInfo->bPLUserType == USERTYPE_TEST )
	{
		DebugOut( "[DisplaySuccess] �׽�Ʈī��\n" );
		if ( gboolIsMainTerm &&
			 IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
		{
			VoiceOut( VOICE_TEST_CARD );
		}
		Buzzer( 1, 50000 );
	}
	// ���ν� ����//////////////////////////////////////////////////////////////
	else if ( pstTransInfo->boolIsMultiEnt && wTotalUserCnt > 1 &&
			  IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
	{
		DebugOut( "[DisplaySuccess] ���ν� ����\n" );

		// ȯ���̸�
		if ( pstTransInfo->stNewXferInfo.bAccXferCnt == 0 )
		{
			// ���ν� ȯ���� �ƴϸ�
			Buzzer( 1, 50000 );
			VoiceOut( VOICE_MULTI_ENT );		// ���ν��Դϴ�.
		}
		else
		{
			Buzzer( 1, 50000 );
			VoiceOut( VOICE_XFER );				// ȯ���Դϴ�.
		}
	}
	// ���� ////////////////////////////////////////////////////////////////////
	else if ( !IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
	{
		word wTotalExtUserCnt = 0;

		DebugOut( "[DisplaySuccess] ����\n" );

		wTotalExtUserCnt =
			pstTransInfo->stNewXferInfo.abMultiEntInfo[0][USER_CNT] +
			pstTransInfo->stNewXferInfo.abMultiEntInfo[1][USER_CNT] +
			pstTransInfo->stNewXferInfo.abMultiEntInfo[2][USER_CNT];

		if ( wTotalExtUserCnt != 1 )
		{
			Buzzer( 1, 50000 );
		}
		else
		{
			switch ( pstTransInfo->stNewXferInfo.abMultiEntInfo[0][USER_TYPE] )
			{
				case USERTYPE_CHILD:
				case USERTYPE_STUDENT:
				case USERTYPE_YOUNG:
					Buzzer( 2, 50000 );
					break;
				default:
					Buzzer( 1, 50000 );
			}
		}
	}
	// ȯ�� ���� ///////////////////////////////////////////////////////////////
	else if ( pstTransInfo->stNewXferInfo.bAccXferCnt > 0 )
	{
		DebugOut( "[DisplaySuccess] ȯ��\n" );

		if ( pstTransInfo->boolIsMultiEnt )
			bTempUserType = pstTransInfo->abUserType[0];
		else
			bTempUserType = pstTransInfo->bPLUserType;

		VoiceOut( VOICE_XFER );					// ȯ���Դϴ�

		// ����������� (��� || �л� || û�ҳ�)�̸� ���� 2�� ���
		if ( bTempUserType == USERTYPE_CHILD ||
			 bTempUserType == USERTYPE_STUDENT ||
			 bTempUserType == USERTYPE_YOUNG )
			Buzzer( 2, 50000 );
		else
			Buzzer( 1, 50000 );
	}
	// ���� ////////////////////////////////////////////////////////////////////
	else
	{
		DebugOut( "[DisplaySuccess] ����\n" );

		// ���ν����� ����������� �Է��� ���
		if ( pstTransInfo->boolIsMultiEnt )
		{
			bTempUserType = pstTransInfo->abUserType[0];
		}
		// ���ν� �Է��� �ƴ� ���
		else
		{
			bTempUserType = pstTransInfo->bPLUserType;
		}

		// ���� ����
		// ��, ��Ƽ����������� ������ ���� 'Thank you' ���� ���
		if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == TRUE &&
			 pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
		{
			VoiceOut( VOICE_THANK_YOU );		// Thank you
		}

		// ����������� (��� || �л� || û�ҳ�)�̸� ���� 2�� ���
		if ( bTempUserType == USERTYPE_CHILD ||
			 bTempUserType == USERTYPE_STUDENT ||
			 bTempUserType == USERTYPE_YOUNG )
			Buzzer( 2, 50000 );
		else
			Buzzer( 1, 50000 );
	}

#ifndef TEST_NOT_SLEEP_DURING_DISPLAY
	usleep( 1000000 );
#endif

	// O LED off
	OLEDOff();

 	// FND �ʱ�ȭ (0, 0)
 	DisplayDWORDInUpFND( 0 );
 	DisplayDWORDInDownFND( 0 );
LogMain("�������\n");
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       DisplayError                                             *
*                                                                              *
*  DESCRIPTION:       ī��ó������ ������ �߻��� ��� ó�������               *
*                     ������ ����, FND�� ����Ѵ�.                             *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü�� ������                   *
*                     sErrCode - �����ڵ�                                      *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void DisplayError( TRANS_INFO *pstTransInfo, short sErrCode )
{
	XLEDOn();

	Buzzer( 3, 80000 );

	switch ( sErrCode )
	{
		case ERR_CARD_PROC_NOT_ONE_CARD:
			VoiceOut( VOICE_TAG_ONE_CARD );		// ī�带 ���常 ���ֽʽÿ�.
			break;
		case ERR_CARD_PROC_CANNOT_USE:
			VoiceOut( VOICE_INVALID_CARD );		// ����� �� ���� ī���Դϴ�.
			break;
		case ERR_CARD_PROC_RETAG_CARD:
			// ���⼭ ��ϵ� ī��Ʈ�� TT�� ��ϵ�
			gwRetagCardCnt++;
			VoiceOut( VOICE_RETAG_CARD );		// ī�带 �ٽ� ���ּ���.
			break;
		case ERR_CARD_PROC_INSUFFICIENT_BAL:
			VoiceOut( VOICE_INSUFFICIENT_BAL );	// �ܾ��� �����մϴ�.
			break;
		case ERR_CARD_PROC_ALREADY_PROCESSED:
			VoiceOut( VOICE_ALREADY_PROCESSED );
												// �̹� ó���Ǿ����ϴ�.
			break;
		case ERR_CARD_PROC_EXPIRED_CARD:
			VoiceOut( VOICE_EXPIRED_CARD );		// ī�� ��ȿ�Ⱓ�� �������ϴ�.
			break;
		case ERR_CARD_PROC_TAG_IN_EXT:
			// 0329������ voicever.dat ������ �������� �ʾ�
			// gstMyTermInfo.abVoiceVer ������ ��� ����Ʈ�� 0x00���� ������
			// '������ ī�带 ���ּ���' ������ 0330 �������� �߰���
			if ( gstMyTermInfo.abVoiceVer[0] == 0x00 )
			{
				VoiceOut( VOICE_ALREADY_PROCESSED );
												// �̹� ó���Ǿ����ϴ�.
			}
			else
			{
				VoiceOut( VOICE_TAG_IN_EXT );	// ������ ī�带 ���ּ���.
			}
			break;
		case ERR_CARD_PROC_NOT_APPROV:
			VoiceOut( VOICE_NOT_APPROV );		// �̽��� ī���Դϴ�.
			break;
		case ERR_CARD_PROC_CANNOT_MULTI_ENT:
			VoiceOut( VOICE_CANNOT_MULTI_ENT_CARD );
												// ���ν��� �Ұ����� ī���Դϴ�.
			break;
		case ERR_CARD_PROC_INPUT_TICKET:
			VoiceOut( VOICE_INPUT_TICKET_INFO );
												// �������� ������ �ּ���.
			break;
		case ERR_CARD_PROC_CITY_PASS_CARD:
			VoiceOut( VOICE_CITY_PASS_CARD );	// ��Ƽ�н��Դϴ�.
			sleep( 1 );
			VoiceOut( VOICE_RETAG_CARD );		// ī�带 �ٽ� ���ּ���.
			break;
		case ERR_CARD_PROC_NO_VOICE:
			// �������� �������� 3ȸ ���
			break;
		default:
			// ���⼭ ��ϵ� ī��Ʈ�� TT�� ��ϵ�
			gwRetagCardCnt++;
			VoiceOut( VOICE_RETAG_CARD );
			break;
	}
#ifndef TEST_NOT_SLEEP_DURING_DISPLAY
	usleep( 1000000 );
#endif

	XLEDOff();
LogMain("�������\n");
}

