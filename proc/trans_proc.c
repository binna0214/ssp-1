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
*  PROGRAM ID :       trans_proc.c                                             *
*                                                                              *
*  DESCRIPTION:       �����������Ǵ� �� ����� ����ϰ� �ű�ȯ��������         *
*                     �����Ѵ�.                                                *
*                                                                              *
*  ENTRY POINT:       None                                                     *
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
* 2005/07/13 Solution Team  Boohyeon Jeon Initial Release                      *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#include "../system/bus_type.h"
#include "card_proc_util.h"
#include "card_proc.h"

#include "trans_proc.h"

/*******************************************************************************
*  Macro Definition                                                            *
*******************************************************************************/
/*
 * ��� �ʱⰪ ���� ��ũ�� ����
 */
#define MAX_FARE					100000			// �ִ밡�ɿ��
#define SC_PREPAY_MAX_BAL			500000			// �ż���ī�� �ִ밡���ܾ�
#define MIF_PREPAY_MAX_BAL			70000			// ������ī�� �ִ밡���ܾ�
#define MIN_BAL						250				// �ּҰ����ܾ�
#define MAX_PREV_UNCHARGED_FARE		900				// �ִ밡��������¡���ݾ�
#define MAX_XFER_ENABLE_CNT			5				// �ִ�ȯ�°���Ƚ��
#define MAX_MIF_TOUR_DAILY_USE_CNT	20				// ������ī�� �ִ��ϴ������Ƚ��

/*
 * ��ȯ�»��� ���� ��ũ�� ����
 * - �ŷ������� �ִ�⺻���3�� 1byte�� �����Ͽ� �����
 */
#define NOT_XFER_CAUSE_XFER_TIME_ELAPSED			0x01	// �ð��ʰ�
#define NOT_XFER_CAUSE_XFER_CNT_ELAPSED				0X02	// ȯ��Ƚ���ʰ�
#define NOT_XFER_CAUSE_INVALID_TRANSP_METHOD_CODE	0x03	// �������Ʋ��
#define NOT_XFER_CAUSE_DIFF_MIF_TERM_GROUP_CODE		0x04	// �׷��ڵ�Ʋ��
#define NOT_XFER_CAUSE_DIFF_MULTI_GET_ON_INFO		0x05	// �°����ٸ�
#define NOT_XFER_CAUSE_SAME_TERM					0x06	// ��������
#define NOT_XFER_CAUSE_NOT_TAG_IN_EXT				0x07	// �������±�
#define NOT_XFER_CAUSE_INVALID_XFER_APPLY_INFO		0x08	// ȯ��������������

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short GetEntExtType( TRANS_INFO *pstTransInfo, byte *pbEntExtType );
static short ProcEnt( TRANS_INFO *pstTransInfo );
static bool IsXfer( TRANS_INFO *pstTransInfo );
static bool IsValidXferFromXferApplyInfo( time_t tPrevEntExtDatetime,
	time_t tNewEntExtDatetime, byte bAccXferCnt, byte *pbNonXferCause );
static bool IsValidXferFromMultiEnt( TRANS_INFO *pstTransInfo );
static short ProcExt( TRANS_INFO *pstTransInfo );
static dword CalcDist( byte *abEntStationID, byte *abExtStationID );
static dword CalcFare( byte bCardType, byte bUserType, word wTranspMethodCode,
	time_t tEntExtDtime, bool boolIsXfer, bool boolIsBaseFare );
static void GetDisExtraAmtRate( byte bCardType, byte bUserType,
	time_t tEntExtDtime, bool boolIsXfer, bool boolIsBaseFare,
	int *pnDisExtraAmt, float *pfDiscExtraRate );
static bool SearchDisExtraInfo( byte *abDisExtraTypeID, bool boolIsBaseFare,
	int *pnDisExtraAmt, float *pfDisExtraRate );
static short ProcCityTourEnt( TRANS_INFO *pstTransInfo );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       TransProc                                                *
*                                                                              *
*  DESCRIPTION:       ī����������ü�� �����͸� �Է¹޾� �̸� �������� ������  *
*                     ������ �Ǵ��ϰ� ����� ����Ѵ�. �׸��� �ű�ȯ��������   *
*                     �����Ͽ� �̸� �ٽ� ī����������ü�� �����Ѵ�.            *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü                            *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_NO_VOICE - ����� 100,000�� �̻��� ���    *
*                        (�������� ������ 3ȸ ���)                            *
*                     ERR_CARD_PROC_CANNOT_USE - ������ī���̸鼭 �ܾ���       *
*                         500,000�� �̻��� ���                                *
*                     ERR_CARD_PROC_TAG_IN_EXT - �ּ����±װ��ɽð� �̸��� ���*
*                     ERR_CARD_PROC_ALREADY_PROCESSED - �ּ���������ɽð�     *
*                         ������ ���                                          *
*                     ERR_CARD_PROC_INSUFFICIENT_BAL - ����ī���̸鼭 �ܾ���   *
*                         �����ؾ��ϴ� ��ݺ��� ���� ���                      *
*                     ERR_CARD_PROC_RETAG_CARD - ���ν������� �°��� ���� 0    *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short TransProc( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;
	byte bEntExtType = 0;
	byte abNowDate[8] = {0, };

	/*
	 * ��Ƽ��������� ��� ����ó��
	 */
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == TRUE )
	{
		sResult = ProcCityTourEnt( pstTransInfo );
		return sResult;
	}

	// �ż���ī���̸鼭 �ŷ��� �ܾ��� 500,000�� �̻��� ��� ////////////////////
	// - ����� �� ���� ī���Դϴ�.
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY &&
		 pstTransInfo->stPrevXferInfo.dwBal > SC_PREPAY_MAX_BAL )
	{
		printf( "[TransProc] �ż���ī���̸鼭 �ܾ��� 500,000�� �ʰ� - " );
		printf( "����� �� ���� ī���Դϴ�\n" );
		return ERR_CARD_PROC_CANNOT_USE;
	}

	// ������ī���̸鼭 �ŷ��� �ܾ��� 70,000�� �̻��� ��� /////////////////////
	// - ����� �� ���� ī���Դϴ�.
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY &&
		 pstTransInfo->stPrevXferInfo.dwBal > MIF_PREPAY_MAX_BAL )
	{
		printf( "[TransProc] ������ī���̸鼭 �ܾ��� 70,000�� �ʰ� - " );
		printf( "����� �� ���� ī���Դϴ�\n" );
		return ERR_CARD_PROC_CANNOT_USE;
	}

	/*
	 * �����������Ǵ�
	 */
	sResult = GetEntExtType( pstTransInfo, &bEntExtType );
	if ( sResult != SUCCESS )
	{
		return sResult;
	}

	// ����ó�� ////////////////////////////////////////////////////////////////
	if ( bEntExtType == ENT )
	{
		// ������ī���� ���
		if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
		{
			// ���ʻ���� ���� ���Ⱓ�� ���� ī���� ���
			// 'ī�� ��ȿ�Ⱓ�� �������ϴ�' ���� ���
			if ( IsValidMifTourExpiryDate( pstTransInfo->tMifTourFirstUseDtime,
					pstTransInfo->wMifTourCardType,
					pstTransInfo->stNewXferInfo.tEntExtDtime ) == FALSE )
			{
				printf( "[TransProc] ���ʻ���Ͻ� ���� ī�������� ���� " );
				printf( "�������� �ʰ��Ͽ����Ƿ� ī������ �Ұ���\n" );
				return ERR_CARD_PROC_EXPIRED_CARD;
			}

			// �������� ���� ī���� ���
			// 'ī�� ��ȿ�Ⱓ�� �������ϴ�' ���� ���
			TimeT2ASCDate( pstTransInfo->stNewXferInfo.tEntExtDtime, abNowDate);
			if ( memcmp( abNowDate, pstTransInfo->abMifTourExpiryDate, 8 ) > 0 )
			{
				printf( "[TransProc] ������ī�� ������ ���� ��� ����\n" );
				return ERR_CARD_PROC_EXPIRED_CARD;
			}
		}

		// '����'�� ��� ���ν������� �°����� ���� 0�̸�
		// 'ī�带 �ٽ� ���ּ���' ����
		if ( pstTransInfo->abUserCnt[0] +
			 pstTransInfo->abUserCnt[1] +
			 pstTransInfo->abUserCnt[2] == 0 )
		{
			printf( "[TransProc] ���ν������� �°����� ���� 0 - ī�带 �ٽ� " );
			printf( "���ּ���" );
			return ERR_CARD_PROC_RETAG_CARD;
		}

		// '����'�� ��� �߰���������θ� FALSE�� �ʱ�ȭ��
		gstMultiEntInfo.boolIsAddEnt = FALSE;

		// ����ó��
		sResult = ProcEnt( pstTransInfo );
	}
	// ����ó�� ////////////////////////////////////////////////////////////////
	else
	{
		// ������ ��/�����ð� ���������� �߻��ϸ�
		// �����ð��� 1�ʸ� ���� ���� ����
		if ( pstTransInfo->stNewXferInfo.tEntExtDtime <
				pstTransInfo->stPrevXferInfo.tEntExtDtime )
		{
			printf( "[TransProc] �������ð��� �����Ǿ� ������\n" );
			if ( abs( pstTransInfo->stPrevXferInfo.tEntExtDtime -
					pstTransInfo->stNewXferInfo.tEntExtDtime ) > 20 )
			{
				printf( "[TransProc] ������ �������ð��� 20�� �̻��̹Ƿ� " );
				printf( "�����ð����� �÷��� ����\n" );
				pstTransInfo->boolIsAdjustExtDtime = TRUE;
			}
			pstTransInfo->stNewXferInfo.tEntExtDtime =
				pstTransInfo->stPrevXferInfo.tEntExtDtime + 1;
		}
		
		// ����ó��
		sResult = ProcExt( pstTransInfo );
	}
	if ( sResult != SUCCESS )
	{
		return sResult;
	}

	// ����� 100,000�� �̻��� ��� - �������� ������ 3ȸ ��� /////////////////
	if ( pstTransInfo->stNewXferInfo.dwFare > MAX_FARE )
	{
		printf( "[TransProc] ����� 100,000�� �̻� - �������� ������ 3ȸ " );
		printf( "���\n" );
		return ERR_CARD_PROC_NO_VOICE;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetEntExtType                                            *
*                                                                              *
*  DESCRIPTION:       �Էµ� ī����������ü�� �̿��Ͽ� ������������ �Ǵ��Ѵ�.  *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü�� ������                   *
*                     pbEntExtType - ������������ �����ϱ� ���� ����Ʈ ������  *
*                                    [ENT / EXT]                               *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_TAG_IN_EXT - �ּ����±װ��ɽð� �̸��� ���*
*                     ERR_CARD_PROC_ALREADY_PROCESSED - �ּ���������ɽð�     *
*                         ������ ���                                          *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short GetEntExtType( TRANS_INFO *pstTransInfo, byte *pbEntExtType )
{
	int nRetagSec = 0;				// ���±׽ð� (��)

	int nMaxRidingSec = 0;			// �ִ�������ɽð�   (����1�ð�/�Ϲ�3�ð�)
	int nMinAbroadSec = 0;			// �ּ���������ɽð� (����10��/�Ϲ�20��)
	int nMinRetagSec = 0;			// �ּ����±װ��ɽð�

	bool gboolIsSameStation = FALSE;// ����ȯ�������� �������
									// ���� �������ǵ��� ������ ����
	bool gboolIsSameTerm = FALSE;	// ����ȯ�������� �͹̳�ID�� ����
									// ������ �͹̳�ID�� ���������� ����

#ifdef TEST_ALWAYS_ENT
	return ENT;
#endif

	// �ִ�������ɽð� �� �ּ���������ɽð� ���� /////////////////////////////
	// ���������� ���
	if ( IsVillageBus( gstVehicleParm.wTranspMethodCode ) )
	{
		nMaxRidingSec = 3600;		// �ִ�������ɽð�   1�ð�
		nMinAbroadSec = 600;		// �ּ���������ɽð� 10��
	}
	// �Ϲݹ����� ���
	else
	{
		nMaxRidingSec = 18000;		// �ִ�������ɽð�   5�ð�
		nMinAbroadSec = 1200;		// �ּ���������ɽð� 20��
	}

	// �ּ����±װ��ɽð� ���� /////////////////////////////////////////////////
	// �����ܸ��� �Ǵ� �����ܸ����� (���� �Ǵ� ����)�� ��� 5��
	if ( gboolIsMainTerm == TRUE ||
		 IsVillageBus( gstVehicleParm.wTranspMethodCode ) ||
		 IsWideAreaBus( gstVehicleParm.wTranspMethodCode ) )
	{
		nMinRetagSec = 5;
	}
	// �����ܸ��� & �׿� ��������� ��� 15��
	else
	{
		nMinRetagSec = 15;
	}

	// ����ȯ�������� ������� ���� �������� ���� ������ ���� ���� /////////////
	if ( memcmp( gpstSharedInfo->abNowStationID,
		 pstTransInfo->stPrevXferInfo.abStationID,
		 sizeof( gpstSharedInfo->abNowStationID ) ) == 0 )
		gboolIsSameStation = TRUE;
	else
		gboolIsSameStation = FALSE;

	// ����ȯ�������� �͹̳�ID�� ���� ������ �͹̳�ID�� �������� ���� ���� /////
	if ( pstTransInfo->stPrevXferInfo.dwTermID ==
			GetDWORDFromASC( gpstSharedInfo->abMainTermID,
		 		sizeof( gpstSharedInfo->abMainTermID ) ) )
	{
		gboolIsSameTerm = TRUE;
	}
	else
	{
		gboolIsSameTerm = FALSE;
	}

	// ���±׽ð� ���� /////////////////////////////////////////////////////////
	nRetagSec = pstTransInfo->stNewXferInfo.tEntExtDtime -
				pstTransInfo->stPrevXferInfo.tEntExtDtime;

	// ���±׽ð��� ������ �ִ� ��� 15�ʷ� ����
	if ( nRetagSec < 0 )
		nRetagSec = 15;

	// �߰����� ���� ���� //////////////////////////////////////////////////////
	// 1. �߰��������ΰ� FALSE�̰�
	// 2. ����ȯ�������� ������������ '����'�̰�
	// 3. ���ν��� �����Ǿ� ������
	// 4. ����ȯ�������� �ܸ���ID�� �� �ܸ���ID�� �����ϰ�
	// 5. ī���ȣLOG�� ī���ȣ�� �����ϴ� ���
	if ( !gstMultiEntInfo.boolIsAddEnt &&
		 IsEnt( pstTransInfo->stPrevXferInfo.bEntExtType ) &&
		 gstMultiEntInfo.boolIsMultiEnt &&
		 gboolIsSameTerm &&
		 IsExistCardNoLog( pstTransInfo->abCardNo ) )
	{
		gstMultiEntInfo.boolIsAddEnt = TRUE;
	}

	DebugOut( "\n" );
	DebugOut( "#############################################################" );
	DebugOut( "##################\n" );
	DebugOut( "# ���������� �Ǵ�\n" );
	DebugOut( "#\n" );
	DebugOut( "# �⺻������\n" );
	DebugOut( "#     - ���±׽ð�                           : %5d (sec)\n",
		nRetagSec );
	DebugOut( "#\n" );
	DebugOut( "#     - �ִ�������ɽð�                     : %5d (sec)\n",
		nMaxRidingSec );
	DebugOut( "#     - �ּ���������ɽð�                   : %5d (sec)\n",
		nMinAbroadSec );
	DebugOut( "#     - �ּ����±װ��ɽð�                   : %5d (sec)\n",
		nMinRetagSec );
	DebugOut( "#\n" );
	DebugOut( "#     - �߰������ ����                      : %s\n",
		GetBoolString( gstMultiEntInfo.boolIsAddEnt ) );
	DebugOut( "#     - ���������۱� �Է��� ���� ���ν� ���� : %s\n",
		GetBoolString( gstMultiEntInfo.boolIsMultiEnt ) );
	DebugOut( "#     - ���� �ܸ��� ����                     : %s\n",
		GetBoolString( gboolIsSameTerm ) );
	DebugOut( "#     - ���� ������ ����                     : %s\n",
		GetBoolString( gboolIsSameStation ) );
	DebugOut( "#\n" );
	DebugOut( "# �Ǵܰ���\n" );

	// ���ϴܸ��� //////////////////////////////////////////////////////////////
	if ( gboolIsSameTerm == TRUE )
	{

		DebugOut( "#     - ���ϴܸ���\n" );

		// �ܸ���׷��ڵ尡 Ÿ�õ� �ڵ��̸� '����' /////////////////////////////
		if ( pstTransInfo->bMifTermGroupCode == 0x02 ||
			 pstTransInfo->bMifTermGroupCode == 0x03 ||
			 pstTransInfo->bMifTermGroupCode == 0x04 ||
			 pstTransInfo->bMifTermGroupCode == 0x06 ||
			 pstTransInfo->bMifTermGroupCode == 0x07 ||
			 ( pstTransInfo->bMifTermGroupCode == 0x05 &&
			   pstTransInfo->tMifEntExtDtime !=
				pstTransInfo->stPrevXferInfo.tEntExtDtime ) )
		{
			DebugOut( "#     - �� �ܸ���׷��ڵ尡 Ÿ�õ� �ڵ�\n" );
			DebugOut( "#     - ������� : ����\n" );

			*pbEntExtType = ENT;

			return SUCCESS;
		}

		// ���� ������������ '����' ////////////////////////////////////////////
		if ( IsEnt( pstTransInfo->stPrevXferInfo.bEntExtType ) )
		{
			DebugOut( "#     - ���� ������������ '����'\n" );

			// ���±׽ð��� �ִ�������ɽð� �̻� //////////////////////////////
			if ( nRetagSec >= nMaxRidingSec )
			{

				DebugOut( "#     - ���±׽ð��� �ִ�������ɽð� �̻�\n" );
				DebugOut( "#     - ������� : ����\n" );

				*pbEntExtType = ENT;

				return SUCCESS;
			}
			// ���±׽ð��� �ִ�������ɽð� �̸� //////////////////////////////
			else
			{

				DebugOut( "#     - ���±׽ð��� �ִ�������ɽð� �̸�\n" );

				// �߰������ //////////////////////////////////////////////////
				if ( gstMultiEntInfo.boolIsAddEnt )
				{
					DebugOut( "#     - �߰������\n" );
					DebugOut( "#     - ������� : ����\n" );

					*pbEntExtType = EXT;

					return SUCCESS;
				}
				// ���ν� //////////////////////////////////////////////////////
				else if ( gstMultiEntInfo.boolIsMultiEnt )
				{
					DebugOut( "#     - ���ν�\n" );
					DebugOut( "#     - ������� : ����\n" );

					*pbEntExtType = ENT;

					return SUCCESS;
				}
				// �߰�������� ���νµ� �ƴ� //////////////////////////////////
				else
				{
					DebugOut( "#     - �߰�������� ���νµ� �ƴ�\n" );

					// �ּ����±װ��ɽð��̻� �Ǵ� �ٸ������� //////////////////
					if ( nRetagSec >= nMinRetagSec || !gboolIsSameStation )
					{
						DebugOut( "#     - �ּ����±װ��ɽð��̻� �Ǵ� " );
						DebugOut( "�ٸ�������\n" );
						DebugOut( "#     - ������� : ����\n" );

						*pbEntExtType = EXT;

						return SUCCESS;
					}
					else
					{
						DebugOut( "#     - �ּ����±װ��ɽð��̸�\n" );
						DebugOut( "#     - ������� : ������ī�带" );
						DebugOut( "���ּ���\n" );

						return ERR_CARD_PROC_TAG_IN_EXT;
					}
				}
			}
		}
		// ���� ������������ '����' ////////////////////////////////////////////
		else
		{

			DebugOut( "#     - ���� ������������ '����'\n" );

			// �ּ���������ɽð��̻� �Ǵ� ���ν� �Ǵ� �߰������ //////////////
			if ( nRetagSec >= nMinAbroadSec ||
				gstMultiEntInfo.boolIsMultiEnt ||
				gstMultiEntInfo.boolIsAddEnt )
			{

				DebugOut( "#     - �ּ���������ɽð��̻� �Ǵ� ���ν� " );
				DebugOut( "�Ǵ� ���� �� �����\n" );
				DebugOut( "#     - ������� : ����\n" );

				*pbEntExtType = ENT;

				return SUCCESS;
			}
			// �ּ���������ɽð��̸��̸鼭 ���ν¾ƴ� /////////////////////////
			else
			{

				DebugOut( "#     - �ּ���������ɽð��̸��̸鼭 " );
				DebugOut( "���ν¾ƴ�\n" );
				DebugOut( "#     - ������� : �̹�ó����ī���Դϴ�\n" );

				return ERR_CARD_PROC_ALREADY_PROCESSED;
			}
		}
	}
	// �ٸ��ܸ��� //////////////////////////////////////////////////////////////
	else
	{

		DebugOut( "#     - �ٸ��ܸ���\n" );
		DebugOut( "#     - ������� : ����\n" );

		*pbEntExtType = ENT;

		return SUCCESS;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcEnt                                                  *
*                                                                              *
*  DESCRIPTION:       ������ ��� ����� ����ϰ� �ű�ȯ�������� �����Ѵ�.     *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü�� ������                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_INSUFFICIENT_BAL - ����ī���̸鼭 �ܾ���   *
*                         �����ؾ��ϴ� ��ݺ��� ���� ���                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short ProcEnt( TRANS_INFO *pstTransInfo )
{
	int i = 0;
	bool boolIsPenalty = FALSE;
	bool boolIsXfer = FALSE;
	dword awMaxBaseFare[3] = {0, };

	DebugOut( "#\n" );
	DebugOut( "# ��������\n" );

	// ���Ƽ ó�� /////////////////////////////////////////////////////////////
	// ���� ������� ���Ƽ �߻� ����
	//     1) ������ ����� ��������� ���� �Ҽ� &&
	//     2) ȯ�´���Ƚ���� 0���� ŭ &&
	//     3) ���� ������������ ����
	if ( IsSeoulTransp( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) &&
		 pstTransInfo->stPrevXferInfo.bAccXferCnt > 0 &&
		 IsEnt( pstTransInfo->stPrevXferInfo.bEntExtType ) )
	{
		dword dwSumOfPrevBaseFare = 0;		// ���� ������� �⺻����� ��

		// ���Ƽ ����� ���Ͽ� ���� ������� �⺻����� ���� ���
		for ( i = 0; i < 3; i++ )
			dwSumOfPrevBaseFare += CalcFare( pstTransInfo->bCardType,
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_TYPE],
				pstTransInfo->stPrevXferInfo.wTranspMethodCode,
				pstTransInfo->stPrevXferInfo.tEntExtDtime,
				TRUE,
				TRUE ) *
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_CNT];

		// ���� ������� �⺻����� ���� ���� ������ܿ��� ������ ��ݺ���
		// ū ��� (���� ���� �ݾ��� �����ϴ� ���)
		// - ���ι��� �ݾ��� �ű�ȯ�������� �������Ƽ������� ����
		if ( dwSumOfPrevBaseFare > pstTransInfo->stPrevXferInfo.dwFare )
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare =
				dwSumOfPrevBaseFare - pstTransInfo->stPrevXferInfo.dwFare;
		// ���ι��� �ݾ��� �������� �ʴ� ���
		// - �ű�ȯ�������� �������Ƽ����� 0���� ����
		else
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare = 0;

		// �ű�ȯ�������� ��ݿ� �������Ƽ����� �ΰ�
		pstTransInfo->stNewXferInfo.dwFare +=
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare;

		// ���Ƽ �߻� ���θ� TRUE�� ����
		boolIsPenalty = TRUE;

		DebugOut( "#     - ���Ƽ �߻�                          : %5u (��)\n",
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare );
	}
	// ����� ������� ���Ƽ �߻� ����
	//     1) ������ ����� ��������� ���� �ƴ� &&
	//     2) ����� ������� ���Ƽ ��������ڵ忡 0�� �ƴ� ���� ����
	else if ( !IsSeoulTransp( pstTransInfo->stPrevXferInfo.wTranspMethodCode )
		&& pstTransInfo->stPrevXferInfo.wIncheonPenaltyPrevTMCode != 0 )
	{
		dword dwSumOfPrevBaseFare = 0;		// ���� ������� �⺻����� ��

		// ���Ƽ ����� ���Ͽ� ���� ������� �⺻����� ���� ���
		for ( i = 0; i < 3; i++ )
		{
			dwSumOfPrevBaseFare += CalcFare( pstTransInfo->bCardType,
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_TYPE],
				pstTransInfo->stPrevXferInfo.wIncheonPenaltyPrevTMCode,
				pstTransInfo->stPrevXferInfo.tIncheonPenaltyPrevDtime,
				TRUE,
				TRUE ) *
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_CNT];
		}

		// ���� ������� �⺻����� ���� ���� ������ܿ��� ������ ��ݺ���
		// ū ��� (���� ���� �ݾ��� �����ϴ� ���)
		// - ���ι��� �ݾ��� �ű�ȯ�������� �������Ƽ������� ����
		if ( dwSumOfPrevBaseFare >
			 pstTransInfo->stPrevXferInfo.dwIncheonPenaltyPrevFare )
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare =
				dwSumOfPrevBaseFare -
				pstTransInfo->stPrevXferInfo.dwIncheonPenaltyPrevFare;
		// ���ι��� �ݾ��� �������� �ʴ� ���
		// - �ű�ȯ�������� �������Ƽ����� 0���� ����
		else
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare = 0;

		// �ű�ȯ�������� ��ݿ� �������Ƽ����� �ΰ�
		pstTransInfo->stNewXferInfo.dwFare +=
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare;

		// ���Ƽ �߻� ���θ� TRUE�� ����
		boolIsPenalty = TRUE;

		DebugOut( "#     - ���Ƽ �߻� (����� ȯ��)            : %5u (��)\n",
			pstTransInfo->stNewXferInfo.wPrevPenaltyFare );
	}

	// ��ī���� ��� �°�2, 3 �����ŷ��� ���ҵ� �ִ�⺻��� ���� //////////////
	// (������ī��� ���ν��� ����)
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
		 pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT ||
		 pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_POSTPAY )
	{
		float afDiscountRate[3] = {0, };
		int nTempBaseFare = 0;
		int nDiscount = 0;

		for ( i = 0; i < 3; i++ )
	   		GetDisExtraAmtRate( pstTransInfo->bCardType,
	   			pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_TYPE],
	   			pstTransInfo->stPrevXferInfo.tEntExtDtime,
	   			TRUE,
	   			TRUE,
	   			&nDiscount,
	   			&afDiscountRate[i] );

		if ( pstTransInfo->stPrevXferInfo.abMultiEntInfo[0][USER_CNT] != 0 )
		{
			nTempBaseFare =
				( pstTransInfo->stPrevXferInfo.awMaxBaseFare[0] /
				 pstTransInfo->stPrevXferInfo.abMultiEntInfo[0][USER_CNT] ) /
				( 1 + afDiscountRate[0] / 100 );
			pstTransInfo->stPrevXferInfo.awMaxBaseFare[1] =
				( nTempBaseFare + ( int )( ( float )nTempBaseFare *
				( float )afDiscountRate[1] / 100.0 ) ) *
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[1][USER_CNT];
			pstTransInfo->stPrevXferInfo.awMaxBaseFare[2] =
				( nTempBaseFare + ( int )( ( float )nTempBaseFare *
				( float )afDiscountRate[2] / 100.0 ) ) *
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[2][USER_CNT];
		}

		DebugOut( "#     - ������ ��ī�� �°�1 �ִ�⺻���     : %5u (��)\n",
			pstTransInfo->stPrevXferInfo.awMaxBaseFare[0] );
		DebugOut( "#     - ������ ��ī�� �°�2 �ִ�⺻���     : %5u (��)\n",
			pstTransInfo->stPrevXferInfo.awMaxBaseFare[1] );
		DebugOut( "#     - ������ ��ī�� �°�3 �ִ�⺻���     : %5u (��)\n",
			pstTransInfo->stPrevXferInfo.awMaxBaseFare[2] );
	}

	// ȯ�¿��� �Ǵ� ///////////////////////////////////////////////////////////
	if ( ( boolIsXfer = IsXfer( pstTransInfo ) ) )
	{
		DebugOut( "#     - ȯ��\n" );

		// ȯ�´���Ƚ�� ////////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.bAccXferCnt =
			pstTransInfo->stPrevXferInfo.bAccXferCnt + 1;

		// ȯ���Ϸù�ȣ ////////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.bXferSeqNo =
			pstTransInfo->stPrevXferInfo.bXferSeqNo;

		// ȯ�³������̵��Ÿ� //////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.dwAccDistInXfer =
			pstTransInfo->stPrevXferInfo.dwAccDistInXfer;

		// ȯ�³������̿�ݾ� //////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.dwAccAmtInXfer =
			pstTransInfo->stPrevXferInfo.dwAccAmtInXfer;

		// ���ν°ŷ����� //////////////////////////////////////////////////////
		memcpy( pstTransInfo->stNewXferInfo.abMultiEntInfo,
			pstTransInfo->stPrevXferInfo.abMultiEntInfo,
			sizeof( pstTransInfo->stNewXferInfo.abMultiEntInfo ) );

		// �� �°����� �⺻��ݰ�� �� �°� ������������ID ���� ////////////////
		memset( awMaxBaseFare, 0, sizeof( awMaxBaseFare ) );
		for ( i = 0; i < 3 && pstTransInfo->abUserCnt[i] != 0; i++ )
		{
			// ��ݰ�� ////////////////////////////////////////////////////////
			awMaxBaseFare[i] = CalcFare( pstTransInfo->bCardType,
				pstTransInfo->abUserType[i],
				gstVehicleParm.wTranspMethodCode,
				pstTransInfo->stNewXferInfo.tEntExtDtime,
				TRUE,
				TRUE ) * pstTransInfo->abUserCnt[i];
			// �°� ������������ID /////////////////////////////////////////////
			CreateDisExtraTypeID( pstTransInfo->bCardType,
				pstTransInfo->abUserType[i],
				pstTransInfo->stNewXferInfo.tEntExtDtime,
				TRUE,
				pstTransInfo->abDisExtraTypeID[i] );
		}

		// �°� �����ŷ��� ���ҵ� �ִ�⺻��� /////////////////////////////////
		for ( i = 0; i < 3; i++ )
		{
			if ( pstTransInfo->stPrevXferInfo.awMaxBaseFare[i] <
				awMaxBaseFare[i] )
			{
				pstTransInfo->stNewXferInfo.awMaxBaseFare[i] = awMaxBaseFare[i];
				pstTransInfo->stNewXferInfo.dwFare += awMaxBaseFare[i] -
					pstTransInfo->stPrevXferInfo.awMaxBaseFare[i];
			}
			else
			{
				pstTransInfo->stNewXferInfo.awMaxBaseFare[i] =
					pstTransInfo->stPrevXferInfo.awMaxBaseFare[i];
			}
		}

		// ��¡����� //////////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.wPrevUnchargedFare =
			pstTransInfo->stPrevXferInfo.wPrevUnchargedFare;

		// ȯ�³��̿���ܱ⺻������� //////////////////////////////////////////
		pstTransInfo->stNewXferInfo.wTotalBaseFareInXfer =
			pstTransInfo->stPrevXferInfo.wTotalBaseFareInXfer;

		// �������/���������� /////////////////////////////////////////////////
		// ���� ��������� �������϶�
		if ( IsBus( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) )
		{
			if ( pstTransInfo->stPrevXferInfo.bEntExtType ==
				 XFER_EXT_AFTER_INCHEON )
				pstTransInfo->stNewXferInfo.bEntExtType =
					XFER_ENT_AFTER_INCHEON;
			else
				pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT_AFTER_BUS;
		}
		// ���� ��������� ö�����϶�
		else if ( IsSubway( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) )
		{
			if ( pstTransInfo->stPrevXferInfo.bEntExtType ==
				 XFER_EXT_AFTER_INCHEON )
				pstTransInfo->stNewXferInfo.bEntExtType =
					XFER_ENT_AFTER_INCHEON;
			else
				pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT_AFTER_SUBWAY;
		}
		else
			pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT;
	}
	else
	{
		DebugOut( "#     - ȯ�¾ƴ� %u\n", pstTransInfo->bNonXferCause );

		// ȯ�´���Ƚ�� ////////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.bAccXferCnt = 0;

		// ȯ���Ϸù�ȣ ////////////////////////////////////////////////////////
		// ȯ���Ϸù�ȣ�� 1 ~ 99 ������ ���� �����̼� ��
		// ��, ������ī���� ��� 1 ~ 15 ������ ���� �����̼� ��
		pstTransInfo->stNewXferInfo.bXferSeqNo =
			( pstTransInfo->stPrevXferInfo.bXferSeqNo + 1 ) % 100;
		if ( ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
			   pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT ) &&
			 pstTransInfo->stNewXferInfo.bXferSeqNo >= 16 )
			pstTransInfo->stNewXferInfo.bXferSeqNo = 1;
		if ( pstTransInfo->stNewXferInfo.bXferSeqNo == 0 )
			pstTransInfo->stNewXferInfo.bXferSeqNo = 1;

		// ȯ�³������̵��Ÿ� //////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.dwAccDistInXfer = 0;

		// ȯ�³������̿�ݾ� //////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.dwAccAmtInXfer = 0;

		// ���ν°ŷ����� //////////////////////////////////////////////////////
		for ( i = 0; i < 3; i++ )
		{
			pstTransInfo->stNewXferInfo.abMultiEntInfo[i][USER_TYPE] =
				pstTransInfo->abUserType[i];
			pstTransInfo->stNewXferInfo.abMultiEntInfo[i][USER_CNT] =
				pstTransInfo->abUserCnt[i];
		}

		// �� �°����� �⺻��ݰ�� �� �°� ������������ID ���� ////////////////
		memset( awMaxBaseFare, 0, sizeof( awMaxBaseFare ) );
		for ( i = 0; i < 3 && pstTransInfo->abUserCnt[i] != 0; i++ )
		{
			// �⺻��ݰ�� ////////////////////////////////////////////////////
			awMaxBaseFare[i] = CalcFare( pstTransInfo->bCardType,
				pstTransInfo->abUserType[i],
				gstVehicleParm.wTranspMethodCode,
				pstTransInfo->stNewXferInfo.tEntExtDtime,
				FALSE,
				TRUE ) * pstTransInfo->abUserCnt[i];

			// �°� ������������ID /////////////////////////////////////////////
			CreateDisExtraTypeID( pstTransInfo->bCardType,
				pstTransInfo->abUserType[i],
				pstTransInfo->stNewXferInfo.tEntExtDtime,
				FALSE,
				pstTransInfo->abDisExtraTypeID[i] );

			DebugOut( "#     - �°�%d �⺻��� �� ��������ID         : ", i );
			DebugOut( "%5ld (��), ", awMaxBaseFare[i] );
			DebugOutlnASC( "", pstTransInfo->abDisExtraTypeID[i], 6 );
		}

		// �°� �����ŷ��� ���ҵ� �ִ�⺻��� /////////////////////////////////
		for ( i = 0; i < 3; i++ )
		{
			pstTransInfo->stNewXferInfo.awMaxBaseFare[i] = awMaxBaseFare[i];
			pstTransInfo->stNewXferInfo.dwFare += awMaxBaseFare[i];
		}

		// ��¡����� //////////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.wPrevUnchargedFare = 0;

		// ȯ�³��̿���ܱ⺻������� //////////////////////////////////////////
		pstTransInfo->stNewXferInfo.wTotalBaseFareInXfer = 0;

		// �������/���������� /////////////////////////////////////////////////
		// ���� ��������� �������϶�
		if ( IsBus( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) )
			pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT_AFTER_BUS;
		// ���� ��������� ����ö���϶�
		else if ( IsSubway( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) )
			pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT_AFTER_SUBWAY;
		else
			pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT;
	}

	// ��� 10������ ���� //////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwFare =
		pstTransInfo->stNewXferInfo.dwFare / 10 * 10;

	// ȯ�³������̿�ݾ� //////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwAccAmtInXfer +=
		pstTransInfo->stNewXferInfo.dwFare;

	// ȯ�³��̿���ܱ⺻������� //////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wTotalBaseFareInXfer += awMaxBaseFare[0] +
		awMaxBaseFare[1] +
		awMaxBaseFare[2] +
		pstTransInfo->stNewXferInfo.wPrevPenaltyFare;

	// �Ѵ������ݾ� //////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwTotalAccUseAmt =
		pstTransInfo->stPrevXferInfo.dwTotalAccUseAmt +
		pstTransInfo->stNewXferInfo.dwFare;

	// �Ѵ�������Ƚ�� //////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wTotalAccEntCnt =
		pstTransInfo->stPrevXferInfo.wTotalAccEntCnt + 1;

	// ������ID ////////////////////////////////////////////////////////////////
	memcpy( pstTransInfo->stNewXferInfo.abStationID,
		gpstSharedInfo->abNowStationID, 7 );

	// ����������� ////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wTranspMethodCode =
		gstVehicleParm.wTranspMethodCode;

	// �͹̳�ID ////////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwTermID =
		GetDWORDFromASC( gpstSharedInfo->abMainTermID,
		sizeof( gpstSharedInfo->abMainTermID ) );

	// ���Ÿ�	( ����:100M( ��:10400M ==> '0104' ) ////////////////////////////
	pstTransInfo->dwDist = 0;

	// ������ī�� ���������������� �������� ////////////////////////////////////
	pstTransInfo->stNewXferInfo.boolIsPrevExt =
		!IsEnt( pstTransInfo->stPrevXferInfo.bEntExtType );

	// ������ī���� ��� ī���ܾ� ���� /////////////////////////////////////////
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		DebugOut( "#     - ������ī��\n" );

		pstTransInfo->bMifTourUseCnt = 0;
		if ( boolIsPenalty == TRUE ) pstTransInfo->bMifTourUseCnt++;
		if ( boolIsXfer == FALSE ) pstTransInfo->bMifTourUseCnt++;

		DebugOut( "#     - �̹� ���������� ������ī�� ���Ƚ��  : %u ȸ\n",
			pstTransInfo->bMifTourUseCnt );

		if ( IsSameDate( pstTransInfo->stPrevXferInfo.tEntExtDtime,
				pstTransInfo->stNewXferInfo.tEntExtDtime ) )
		{
			DebugOut( "#     - ������ -> �ϴ������Ƚ���� Ƚ���� �ջ�\n" );

			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt =
				pstTransInfo->stPrevXferInfo.bMifTourDailyAccUseCnt +
				pstTransInfo->bMifTourUseCnt;
		}
		else
		{
			DebugOut( "#     - �ٸ��� -> �ϴ������Ƚ���� �̹� Ƚ���� ����\n" );

			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt =
				pstTransInfo->bMifTourUseCnt;
		}

		if ( pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt >
				MAX_MIF_TOUR_DAILY_USE_CNT )
		{
			DebugOut( "#     - �ϴ������Ƚ�� �ʰ�\n" );
			return ERR_CARD_PROC_INSUFFICIENT_BAL;
		}

		// �Ѵ������Ƚ�� ó��
		pstTransInfo->stNewXferInfo.dwMifTourTotalAccUseCnt =
			pstTransInfo->stPrevXferInfo.dwMifTourTotalAccUseCnt +
			pstTransInfo->bMifTourUseCnt;

	}
	// ����ī���� ��� ī���ܾ� ���� ///////////////////////////////////////////
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
		 pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY )
	{

		dword dwTempBal = 0;

		DebugOut( "#     - ����ī��\n" );

		memcpy( &dwTempBal, &pstTransInfo->stPrevXferInfo.dwBal,
			sizeof( dwTempBal ) );

		DebugOut( "#     - �������� ���                      : %5lu (��)\n",
			pstTransInfo->stNewXferInfo.dwFare );
		DebugOut( "#     - �ܾ�                                 : %5lu (��)\n",
			dwTempBal );

		// �ܾ׺��� ������ �߻��ϴ� ��� ///////////////////////////////////////
		// 1. ����� �ܾ׺��� ū ���
		// 2. ������ �ܾ��� 250�� �̸��� ���
		if ( ( int )pstTransInfo->stNewXferInfo.dwFare > dwTempBal ||
			 dwTempBal < MIN_BAL )
		{

			DebugOut( "#     - �ܾ׺��� ����\n" );

#ifndef TEST_NOT_CHECK_MIN_BAL
			return ERR_CARD_PROC_INSUFFICIENT_BAL;
#else
			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stPrevXferInfo.dwBal -
				pstTransInfo->stNewXferInfo.dwFare;
#endif
		}
		else
		{

			DebugOut( "#     - ���������� �ܾ� ����\n" );

			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stPrevXferInfo.dwBal -
				pstTransInfo->stNewXferInfo.dwFare;
		}
	}
	// �ĺ�ī�� �� ��ġ��ī���� ��� ī���ܾ� ���� /////////////////////////////
	else
	{

		DebugOut( "#     - �ĺ�ī�� �Ǵ� ��ġ��ī��\n" );

		if ( IsSameMonth( pstTransInfo->stPrevXferInfo.tEntExtDtime,
			 pstTransInfo->stNewXferInfo.tEntExtDtime ) )
		{

			DebugOut( "#     - ���Ͽ� -> �ܾ׿� ����� �ջ�\n" );

			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stPrevXferInfo.dwBal +
				pstTransInfo->stNewXferInfo.dwFare;
		}
		else
		{

			DebugOut( "#     - �ٸ��� -> �ܾ��� ������� ����\n" );

			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stNewXferInfo.dwFare;
			pstTransInfo->boolIsChangeMonth = TRUE;
		}
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsXfer                                                   *
*                                                                              *
*  DESCRIPTION:       �̹� ������ ȯ�������� ���θ� �Ǻ��Ͽ� �����ϰ�,         *
*                     ȯ�ºҰ������ڵ带 �����Ѵ�.                             *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü�� ������                   *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - ȯ��                                              *
*                     FALSE - ȯ�¾ƴ�                                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-11                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static bool IsXfer( TRANS_INFO *pstTransInfo )
{
	// �ð��ʰ� �Ǵ� ȯ��Ƚ�� �ʰ��̸� ȯ�� X
	if ( !IsValidXferFromXferApplyInfo(
			pstTransInfo->stPrevXferInfo.tEntExtDtime,
			pstTransInfo->stNewXferInfo.tEntExtDtime,
			pstTransInfo->stPrevXferInfo.bAccXferCnt,
			&pstTransInfo->bNonXferCause ) )
	{
		return FALSE;
	}

	// ���������̸� ȯ�� X
	if ( pstTransInfo->stPrevXferInfo.dwTermID ==
		 GetDWORDFromASC( gpstSharedInfo->abMainTermID, 9 ) )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_SAME_TERM;
		return FALSE;
	}

	// �������±׸� ȯ�� X
	if ( IsEnt( pstTransInfo->stPrevXferInfo.bEntExtType ) )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_NOT_TAG_IN_EXT;
		return FALSE;
	}

	// ���� �Ǵ� ���� ��������ڵ尡 '��������'�̸� ȯ�� X
	if ( IsWideAreaBus( gstVehicleParm.wTranspMethodCode ) ||
		 IsWideAreaBus( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_INVALID_TRANSP_METHOD_CODE;
		return FALSE;
	}

	// ���� ��������ڵ尡 ('�������' || '����ö��')�� �ƴϸ� ȯ�� X
	if ( !IsSeoulBusSubway( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_INVALID_TRANSP_METHOD_CODE;
		return FALSE;
	}

	// �ܸ���׷��ڵ尡 Ÿ�õ� �ڵ��̸� ȯ�� X
	if ( pstTransInfo->bMifTermGroupCode == 0x02 ||
		 pstTransInfo->bMifTermGroupCode == 0x03 ||
		 pstTransInfo->bMifTermGroupCode == 0x04 ||
		 pstTransInfo->bMifTermGroupCode == 0x06 ||
		 pstTransInfo->bMifTermGroupCode == 0x07 )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_DIFF_MIF_TERM_GROUP_CODE;
		return FALSE;
	}

	// �ܸ���׷��ڵ尡 5�̸鼭 �̿�ð��� �ٸ��� ȯ�� X
	if ( pstTransInfo->bMifTermGroupCode == 0x05 &&
		 pstTransInfo->tMifEntExtDtime !=
			pstTransInfo->stPrevXferInfo.tEntExtDtime )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_DIFF_MIF_TERM_GROUP_CODE;
		return FALSE;
	}

	// ���ν� ȯ�������� �޶����� ȯ�� X
	if ( !IsValidXferFromMultiEnt( pstTransInfo ) )
	{
		pstTransInfo->bNonXferCause = NOT_XFER_CAUSE_DIFF_MULTI_GET_ON_INFO;
		return FALSE;
	}

	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidXferFromXferApplyInfo                             *
*                                                                              *
*  DESCRIPTION:       ȯ������������ ����Ͽ� �̹� ������ ȯ�������� ���θ�    *
*                     �Ǻ��Ͽ� �����ϰ�, ȯ�ºҰ������ڵ带 �����Ѵ�.          *
*                                                                              *
*  INPUT PARAMETERS:  tPrevEntExtDatetime - �����������Ͻ�                     *
*                     tNewEntExtDatetime - �űԽ������Ͻ�                      *
*                     bAccXferCnt - ȯ�´���Ƚ��                               *
*                     pbNonXferCause - ȯ�ºҰ������ڵ带 ���� ����Ʈ ������   *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - ȯ��                                              *
*                     FALSE - ȯ�¾ƴ�                                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static bool IsValidXferFromXferApplyInfo( time_t tPrevEntExtDatetime,
	time_t tNewEntExtDatetime, byte bAccXferCnt, byte *pbNonXferCause )
{
	byte abBuf[14] = {0, };
	word wNowTime = 0;
//	dword i = 0;
//	bool boolIsSuccessFind = FALSE;
//	bool boolIsChanged = FALSE;
//	byte bTodayCode = '1';
	int nRetagSec = 0;
	dword dwOverTime = 0;

//	static XFER_APPLY_INFO *gpstCurrentXferApplyInfo = NULL;

	TimeT2ASCDtime( tNewEntExtDatetime, abBuf );
	wNowTime = GetWORDFromASC( &abBuf[8], 4 );
/*
	if ( tNewEntExtDatetime != 0 )
	{
		for ( i = 0; i < gstHolidayInfoHeader.dwRecordCnt; i++ )
		{
			if ( memcmp( abBuf, gpstHolidayInfo[i].abHolidayDate,
					sizeof( gpstHolidayInfo[i].abHolidayDate ) ) == 0 )
			{
				bTodayCode = gpstHolidayInfo[i].bHolidayClassCode;
				break;
			}
		}
	}

	// �� ȯ������������ ��ȿ���� �˻� /////////////////////////////////////////
	if ( gpstCurrentXferApplyInfo == NULL )
		boolIsChanged = TRUE;
	else if ( ( gpstCurrentXferApplyInfo->wXferApplyStartTime <
			  gpstCurrentXferApplyInfo->wXferApplyEndTime ) &&
			 ( wNowTime < gpstCurrentXferApplyInfo->wXferApplyStartTime ||
			  wNowTime >= gpstCurrentXferApplyInfo->wXferApplyEndTime ) )
		boolIsChanged = TRUE;
	else if ( ( gpstCurrentXferApplyInfo->wXferApplyStartTime >
			  gpstCurrentXferApplyInfo->wXferApplyEndTime ) &&
			 ( wNowTime < gpstCurrentXferApplyInfo->wXferApplyStartTime &&
			  wNowTime >= gpstCurrentXferApplyInfo->wXferApplyEndTime ) )
		boolIsChanged = TRUE;

	// ȯ������������ ����� ��� //////////////////////////////////////////////
	if ( boolIsChanged )
	{
		boolIsSuccessFind = FALSE;
		for ( i = 0; i < gstXferApplyInfoHeader.dwRecordCnt &&
				!boolIsSuccessFind; i++ )
		{
			if ( bTodayCode == gpstXferApplyInfo[i].bHolidayClassCode &&
				tNewEntExtDatetime > gpstXferApplyInfo[i].tApplyDtime )
			{
				gpstCurrentXferApplyInfo = &( gpstXferApplyInfo[i] );

				if ( ( ( gpstCurrentXferApplyInfo->wXferApplyStartTime <
					  gpstCurrentXferApplyInfo->wXferApplyEndTime ) &&
					 ( wNowTime >=
					  gpstCurrentXferApplyInfo->wXferApplyStartTime &&
					  wNowTime <
					  gpstCurrentXferApplyInfo->wXferApplyEndTime ) ) ||
					( ( gpstCurrentXferApplyInfo->wXferApplyStartTime >
					  gpstCurrentXferApplyInfo->wXferApplyEndTime ) &&
					 !( wNowTime <
					   gpstCurrentXferApplyInfo->wXferApplyStartTime &&
					   wNowTime >=
					   gpstCurrentXferApplyInfo->wXferApplyEndTime ) ) )
				{
					boolIsSuccessFind = TRUE;
				}
			}
		}

		// ���ǿ� �´� ȯ������������ �������� �����Ƿ� ������ ùž�� ó��
		if ( !boolIsSuccessFind )
		{
			// ȯ���������� ������ �ʱ�ȭ
			gpstCurrentXferApplyInfo = NULL;

			// ��ȯ�»��� ����
			*pbNonXferCause = NOT_XFER_CAUSE_INVALID_XFER_APPLY_INFO;

			// ȯ�°��ɽð� �ϵ��ڵ�
			if ( wNowTime >= 700 && wNowTime < 2100 )
				dwOverTime = 33 * 60;
			else
				dwOverTime = 66 * 60;

			// ���±׽ð� ���
			nRetagSec = tNewEntExtDatetime - tPrevEntExtDatetime;

			// ���±׽ð��� ������ �ִ� ��� 10�ʷ� ����
			if ( nRetagSec < 0 )
				nRetagSec = 10;

			// �ϵ��ڵ��� ȯ�°��ɽð��� ȯ�°���Ƚ�� ���Ͽ� ȯ�¿��� �Ǵ�
			if ( nRetagSec <= dwOverTime && bAccXferCnt < 4 )
				return TRUE;

			return FALSE;
		}
	}
*/
	// ���±׽ð� ���
	nRetagSec = tNewEntExtDatetime - tPrevEntExtDatetime;

	// ���±׽ð��� ������ �ִ� ��� 10�ʷ� ����
	if ( nRetagSec < 0 )
		nRetagSec = 15;

	// ȯ�°��ɽð� �ϵ��ڵ�
	if ( wNowTime >= 700 && wNowTime < 2100 )
		dwOverTime = 33 * 60;
	else
		dwOverTime = 66 * 60;
	if ( nRetagSec > dwOverTime )
	{
		*pbNonXferCause = NOT_XFER_CAUSE_XFER_TIME_ELAPSED;
		return FALSE;
	}

	// ȯ�°���Ƚ�� �ϵ��ڵ�
	if ( bAccXferCnt >= MAX_XFER_ENABLE_CNT - 1 )
	{
		*pbNonXferCause = NOT_XFER_CAUSE_XFER_CNT_ELAPSED;
		return FALSE;
	}

	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidXferFromMultiEnt                                  *
*                                                                              *
*  DESCRIPTION:       ���� ���ν������� ���� ���ν������� ���Ͽ� ȯ�¿��θ�  *
*                     �Ǻ��Ͽ� �����Ѵ�.                                       *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü                            *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - ȯ��                                              *
*                     FALSE - ȯ�¾ƴ�                                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static bool IsValidXferFromMultiEnt( TRANS_INFO *pstTransInfo )
{
	bool boolResult = TRUE;
	byte i = 0;
	byte j = 0;

	// ���ν� ȯ������ �˻� ////////////////////////////////////////////////////
	for ( i = 0; i < 3  && boolResult; i++ )
	{
		boolResult = FALSE;
		for ( j = 0; j < 3; j ++ )
		{
			if ( pstTransInfo->abUserType[i] ==
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[j][USER_TYPE] &&
				pstTransInfo->abUserCnt[i] ==
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[j][USER_CNT] )
			{
				boolResult = TRUE;
			}
		}
	}

	for ( j = 0; j < 3 && boolResult; j++ )
	{
		boolResult = FALSE;
		for ( i = 0; i < 3; i++ )
		{
			if ( pstTransInfo->abUserType[i] ==
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[j][USER_TYPE] &&
				pstTransInfo->abUserCnt[i] ==
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[j][USER_CNT] )
			{
				boolResult = TRUE;
			}
		}
	}

	return boolResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ProcExt                                                  *
*                                                                              *
*  DESCRIPTION:       ������ ��� ����� ����ϰ� �ű�ȯ�������� �����Ѵ�.     *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü�� ������                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_INSUFFICIENT_BAL - ����ī���̸鼭 �ܾ���   *
*                         �����ؾ��ϴ� ��ݺ��� ���� ���                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short ProcExt( TRANS_INFO *pstTransInfo )
{

	int i = 0;
	int anDiscount[3] = {0, };		// ���αݾ� �迭
	float afDiscountRate[3] = {0.0, };
									// ������ �迭
	bool boolIsXfer = FALSE;		// ȯ�¿���
	dword dwDist = 0;				// �̵��Ÿ�

	DebugOut( "#\n" );
	DebugOut( "# ��������\n" );

	// ������¡���ݾ��� 900���� ũ�ų� �������� 0�� �ƴϸ� 0���� ������
	if ( pstTransInfo->stPrevXferInfo.wPrevUnchargedFare >
			MAX_PREV_UNCHARGED_FARE ||
		pstTransInfo->stPrevXferInfo.wPrevUnchargedFare % 10 != 0 )
	{
		pstTransInfo->stPrevXferInfo.wPrevUnchargedFare = 0;
	}

	// ȯ�¿��� �Ǵ�
	if ( pstTransInfo->stPrevXferInfo.bAccXferCnt > 0 )
		boolIsXfer = TRUE;

	// ������ �� ���αݾ� ��� /////////////////////////////////////////////////
	for ( i = 0; i < 3 &&
		pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_CNT] != 0; i++ )
	{
		GetDisExtraAmtRate( pstTransInfo->bCardType,
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_TYPE],
				pstTransInfo->stPrevXferInfo.tEntExtDtime,
				boolIsXfer,
			 	FALSE,
			 	&anDiscount[i],
			 	&afDiscountRate[i] );
		CreateDisExtraTypeID( pstTransInfo->bCardType,
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_TYPE],
				pstTransInfo->stNewXferInfo.tEntExtDtime,
				boolIsXfer,
				pstTransInfo->abDisExtraTypeID[i] );
	}

	// �����̵��Ÿ� ��� ///////////////////////////////////////////////////////
	dwDist = CalcDist( pstTransInfo->stPrevXferInfo.abStationID,
							gpstSharedInfo->abNowStationID );

	DebugOut( "#     - �����̵��Ÿ�                         : %5lu ( m )\n",
		dwDist );
	DebugOut( "#     - ȯ�³��̵��Ÿ�                       : %5lu ( m )\n",
		( pstTransInfo->stPrevXferInfo.dwAccDistInXfer + dwDist ) );

	// ���Ÿ� ����
	pstTransInfo->dwDist = dwDist;

	// ȯ���̸鼭 ȯ�³��̵��Ÿ��� �⺻�Ÿ��� �ʰ��ϴ� ��� ////////////////////
	if ( pstTransInfo->stPrevXferInfo.dwAccDistInXfer + dwDist >
		gstNewFareInfo.dwBaseDist && boolIsXfer )
	{

		int nUseDist = 0;			// ȯ�³������̵��Ÿ����� �⺻�Ÿ� �ʰ���
		int nPrevDistUnit = 0;		// ���������Ÿ�
		int nNewDistUnit = 0;		// �űԴ����Ÿ�

		DebugOut( "#     - ȯ�³��̵��Ÿ��� �⺻�Ÿ��� �ʰ�\n" );

		if ( gstNewFareInfo.dwAddedDist != 0 )
		{
			// ���������Ÿ� ���
			nUseDist = ( ( int )pstTransInfo->stPrevXferInfo.dwAccDistInXfer -
				( int )gstNewFareInfo.dwBaseDist ) - 1;
			nPrevDistUnit = nUseDist / ( int )gstNewFareInfo.dwAddedDist + 1;

			// �űԴ����Ÿ� ���
			// (�켱 ȯ�³������̵��Ÿ��� �ű��̵��Ÿ��� ��� ��ģ �Ÿ��� ���)
			nNewDistUnit = ( (
				( int )pstTransInfo->stPrevXferInfo.dwAccDistInXfer +
				( int )dwDist - ( int )gstNewFareInfo.dwBaseDist ) - 1 ) /
				( int )gstNewFareInfo.dwAddedDist + 1;

			// �������Ÿ����� �⺻�Ÿ��� �� �Ÿ��� 0���� �۰ų�
			// 1ȸ ȯ���̸鼭 ( ����ö�� �� ��õȯ�� ) �� ������ �ƴ� ���
			// ���������Ÿ��� 0���� ����
			if ( nUseDist < 0 ||
				( pstTransInfo->stPrevXferInfo.bAccXferCnt == 1 &&
				 ( pstTransInfo->stPrevXferInfo.bEntExtType !=
				   		XFER_ENT_AFTER_SUBWAY &&
				   pstTransInfo->stPrevXferInfo.bEntExtType !=
				   		XFER_ENT_AFTER_INCHEON ) ) )
			{
				nPrevDistUnit = 0;
			}
		}

		// ( ȯ�³������̵��Ÿ��� �ջ��ߴ� ) �űԴ����Ÿ����� ���������Ÿ��� ��
		nNewDistUnit -= nPrevDistUnit;

		DebugOut( "#     - �űԴ����Ÿ�                         : %5d\n",
			nNewDistUnit );

		// �űԴ����Ÿ��� 0���� ū ���
		if ( nNewDistUnit > 0 )
		{
			dword dwFare = 0;

			for ( i = 0; i < 3 &&
				pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_CNT] != 0;
				i++ )
			{

				dwFare = nNewDistUnit * gstNewFareInfo.dwAddedFare *
					pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_CNT];

				if ( afDiscountRate[i] != 0 )
					dwFare += dwFare * afDiscountRate[i] / 100;
				else
					dwFare += anDiscount[i];

				pstTransInfo->stNewXferInfo.dwFare += dwFare;
			}

			// 10������ ����
			pstTransInfo->stNewXferInfo.dwFare =
				pstTransInfo->stNewXferInfo.dwFare / 10 * 10;
		}
	}

	// ��¡������� ���� ��¡����� + ������� ����
	pstTransInfo->stNewXferInfo.wPrevUnchargedFare =
		pstTransInfo->stPrevXferInfo.wPrevUnchargedFare +
		pstTransInfo->stNewXferInfo.dwFare;

	// ����� 0���� ����
	pstTransInfo->stNewXferInfo.dwFare = 0;

	// ��¡����� �߻� /////////////////////////////////////////////////////////
	if ( pstTransInfo->stPrevXferInfo.dwAccAmtInXfer +
		pstTransInfo->stNewXferInfo.wPrevUnchargedFare >
		pstTransInfo->stPrevXferInfo.wTotalBaseFareInXfer )
	{

		word wTempUnchargedFare = 0;		// �ӽ� ��¡�����
		wTempUnchargedFare = pstTransInfo->stPrevXferInfo.dwAccAmtInXfer +
							 pstTransInfo->stNewXferInfo.wPrevUnchargedFare -
							 pstTransInfo->stPrevXferInfo.wTotalBaseFareInXfer;
		pstTransInfo->stNewXferInfo.dwFare =
			pstTransInfo->stNewXferInfo.wPrevUnchargedFare - wTempUnchargedFare;
		pstTransInfo->stNewXferInfo.wPrevUnchargedFare = wTempUnchargedFare;
	}
	// ��¡����� �̹߻� ///////////////////////////////////////////////////////
	else
	{
		pstTransInfo->stNewXferInfo.dwFare =
			pstTransInfo->stNewXferInfo.wPrevUnchargedFare;
		pstTransInfo->stNewXferInfo.wPrevUnchargedFare = 0;
	}

	DebugOut( "#     - ȯ�³��̿���ܱ⺻�������           : %5u (��)\n",
		pstTransInfo->stPrevXferInfo.wTotalBaseFareInXfer );
	DebugOut( "#     - ���                                 : %5lu (��)\n",
		pstTransInfo->stNewXferInfo.dwFare );
	DebugOut( "#     - ��¡�����                           : %5u (��)\n",
		pstTransInfo->stNewXferInfo.wPrevUnchargedFare );

	// �������/���������� ���� ////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.bEntExtType =
		pstTransInfo->stPrevXferInfo.bEntExtType | XFER_EXT;

	// ȯ�´���Ƚ�� ////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.bAccXferCnt =
		pstTransInfo->stPrevXferInfo.bAccXferCnt;

	// ȯ���Ϸù�ȣ ////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.bXferSeqNo =
		pstTransInfo->stPrevXferInfo.bXferSeqNo;

	// ������ID ////////////////////////////////////////////////////////////////
	memcpy( pstTransInfo->stNewXferInfo.abStationID,
		gpstSharedInfo->abNowStationID, 7 );

	// ����������� ////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wTranspMethodCode =
		gstVehicleParm.wTranspMethodCode;

	// ȯ�³������̵��Ÿ� //////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwAccDistInXfer =
		pstTransInfo->stPrevXferInfo.dwAccDistInXfer + dwDist;

	// ȯ�³������̿�ݾ� //////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwAccAmtInXfer =
		pstTransInfo->stPrevXferInfo.dwAccAmtInXfer +
		pstTransInfo->stNewXferInfo.dwFare;

	// �͹̳�ID ////////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwTermID =
		GetDWORDFromASC( gpstSharedInfo->abMainTermID, 9 );

	// ���ν°ŷ����� //////////////////////////////////////////////////////////
	for ( i = 0; i < 3; i++ )
	{
		pstTransInfo->stNewXferInfo.abMultiEntInfo[i][USER_TYPE] =
			pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_TYPE];
		pstTransInfo->stNewXferInfo.abMultiEntInfo[i][USER_CNT] =
			pstTransInfo->stPrevXferInfo.abMultiEntInfo[i][USER_CNT];
	}

	// �Ѵ�������Ƚ�� //////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wTotalAccEntCnt =
		pstTransInfo->stPrevXferInfo.wTotalAccEntCnt;

	// �Ѵ������ݾ� //////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.dwTotalAccUseAmt =
		pstTransInfo->stPrevXferInfo.dwTotalAccUseAmt +
		pstTransInfo->stNewXferInfo.dwFare;

	// �°� �����ŷ��� ���ҵ� �ִ�⺻��� /////////////////////////////////////
	memcpy( pstTransInfo->stNewXferInfo.awMaxBaseFare,
		pstTransInfo->stPrevXferInfo.awMaxBaseFare,
		sizeof( pstTransInfo->stNewXferInfo.awMaxBaseFare ) );

	// ȯ�³��̿���ܱ⺻������� //////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wTotalBaseFareInXfer =
		pstTransInfo->stPrevXferInfo.wTotalBaseFareInXfer;

	// ���Ƽ��� //////////////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.wPrevPenaltyFare = 0;

	// ������ī�� ���������������� �������� ////////////////////////////////////
	pstTransInfo->stNewXferInfo.boolIsPrevExt =
		!IsEnt( pstTransInfo->stPrevXferInfo.bEntExtType );

	// ������ī���� ��� ī���ܾ� ���� /////////////////////////////////////////
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		DebugOut( "#     - ������ī��\n" );

		pstTransInfo->bMifTourUseCnt = 0;

		// �ϴ������Ƚ��
		if ( IsSameDate( pstTransInfo->stPrevXferInfo.tEntExtDtime,
				pstTransInfo->stNewXferInfo.tEntExtDtime ) )
		{
			DebugOut( "#     - ������ -> �ϴ������Ƚ���� �����ϰ� ����\n" );

			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt =
				pstTransInfo->stPrevXferInfo.bMifTourDailyAccUseCnt;
		}
		else
		{
			DebugOut( "#     - �ٸ��� -> �ϴ������Ƚ���� 0���� �ʱ�ȭ\n" );

			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt =
				pstTransInfo->bMifTourUseCnt;
		}


		// �Ѵ������Ƚ��
		pstTransInfo->stNewXferInfo.dwMifTourTotalAccUseCnt =
			pstTransInfo->stPrevXferInfo.dwMifTourTotalAccUseCnt;

	}
	// ����ī���� ��� ī���ܾ� ���� ///////////////////////////////////////////
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
		pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY )
	{
		dword dwTempBal = 0;

		DebugOut( "#     - ����ī��\n" );

		memcpy( &dwTempBal, &pstTransInfo->stPrevXferInfo.dwBal, 4 );

		DebugOut( "#     - �������� ���                      : %5lu (��)\n",
			pstTransInfo->stNewXferInfo.dwFare );
		DebugOut( "#     - �ܾ�                                 : %5lu (��)\n",
			dwTempBal );

		if ( ( int )pstTransInfo->stNewXferInfo.dwFare > dwTempBal )
		{

			DebugOut( "#     - �ܾ׺��� ����\n" );

			return ERR_CARD_PROC_INSUFFICIENT_BAL;
		}
		else
		{
			DebugOut( "#     - ���������� �ܾ� ����\n" );

			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stPrevXferInfo.dwBal -
				pstTransInfo->stNewXferInfo.dwFare;
		}
	}
	// �ĺ�ī�� �� ��ġ��ī���� ��� ī���ܾ� ���� /////////////////////////////
	else
	{

		DebugOut( "#     - �ĺ�ī�� �Ǵ� ��ġ��ī��\n" );

		if ( IsSameMonth( pstTransInfo->stPrevXferInfo.tEntExtDtime,
			pstTransInfo->stNewXferInfo.tEntExtDtime ) )
		{

			DebugOut( "#     - ���Ͽ� -> �ܾ׿� ����� �ջ�\n" );

			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stPrevXferInfo.dwBal +
				pstTransInfo->stNewXferInfo.dwFare;
		}
		else
		{

			DebugOut( "#     - �ٸ��� -> �ܾ��� ������� ����\n" );

			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stNewXferInfo.dwFare;
			pstTransInfo->boolIsChangeMonth = TRUE;
		}
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CalcDist                                                 *
*                                                                              *
*  DESCRIPTION:       �� ����������� �Ÿ��� �����������κ��� ����Ͽ�         *
*                     �����Ѵ�.                                                *
*                                                                              *
*  INPUT PARAMETERS:  abEntStationID - ����������ID                            *
*                     abExtStationID - ����������ID                            *
*                                                                              *
*  RETURN/EXIT VALUE: dword - �� ����������� �Ÿ�                             *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static dword CalcDist( byte *abEntStationID, byte *abExtStationID )
{
	word i = 0;
	word j = 0;
	dword dwMaxDist = 0;
	dword dwEntDist = 0;
	dword dwExtDist = 0;
	int nDist = 0;
	dword dwAnotherDist = 0;
	bool boolIsSuccessFind = FALSE;

	// ���������� �ε��� ������ ��� �Ÿ��� 0�� ����
	if ( gstStationInfoHeader.dwRecordCnt == 0 )
	{
		return 0;
	}

	if ( abEntStationID == NULL || abExtStationID == NULL )
	{
		DebugOut( "[CalcDist] �Ÿ����� ������ID�� NULL�� �Էµ�\n" );
		ErrRet( ERR_CARD_PROC_STATION_ID );
		return 0;
	}

	dwMaxDist = gpstStationInfo[gstStationInfoHeader.dwRecordCnt - 1].
					dwDistFromFirstStation;

	boolIsSuccessFind = FALSE;
	for ( i = 0; i < gstStationInfoHeader.dwRecordCnt && !boolIsSuccessFind;
		i++ )
	{
		if ( memcmp( abEntStationID, gpstStationInfo[i].abStationID,
			 sizeof( gpstStationInfo[j].abStationID ) ) == 0 )
		{
			dwEntDist = gpstStationInfo[i].dwDistFromFirstStation;
			boolIsSuccessFind = TRUE;
		}
	}

	if ( !boolIsSuccessFind )
	{
		DebugOut( "[CalcDist] �Ÿ����� ����������ID�� �������� ����\n" );
		ErrRet( ERR_CARD_PROC_STATION_ID );
		return 0;
	}

	boolIsSuccessFind = FALSE;
	for ( j = 0; j < gstStationInfoHeader.dwRecordCnt && !boolIsSuccessFind;
		j++ )
	{
		if ( memcmp( abExtStationID, gpstStationInfo[j].abStationID,
			 sizeof( gpstStationInfo[j].abStationID ) ) == 0 )
		{
			dwExtDist = gpstStationInfo[j].dwDistFromFirstStation;
			boolIsSuccessFind = TRUE;
		}
	}

	if ( !boolIsSuccessFind ) {
		DebugOut( "[CalcDist] �Ÿ����� ����������ID�� �������� ����\n" );
		ErrRet( ERR_CARD_PROC_STATION_ID );
		return 0;
	}

	nDist = dwExtDist - dwEntDist;

	if ( nDist < 0 )
	{
		dwAnotherDist = dwMaxDist - dwEntDist + dwExtDist;	// �Ǵٸ� �н�
		if ( ( -1 * nDist ) > dwAnotherDist )
		{
			nDist = dwAnotherDist;
			DebugOut( "#     - �������� ���� (�ִ�Ÿ� : %lu, ", dwMaxDist );
			DebugOut( "�����Ÿ� : %lu, �����Ÿ� : %lu)\n", dwEntDist,
				dwExtDist );
		}
		else
		{
			nDist *= -1;
			DebugOut( "#     - ������ ����   (�ִ�Ÿ� : %lu, ", dwMaxDist );
			DebugOut( "�����Ÿ� : %lu, �����Ÿ� : %lu)\n", dwEntDist,
				dwExtDist );
		}
	}

	return nDist;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CalcFare                                                 *
*                                                                              *
*  DESCRIPTION:       �Էµ� ���ǿ� �ش��ϴ� �⺻��� �� ������/���αݾ���     *
*                     ������ ����� ����Ͽ� �����Ѵ�.                         *
*                                                                              *
*  INPUT PARAMETERS:  bCardType - ī������                                     *
*                     bUserType - ���������                                   *
*                     wTranspMethodCode - ��������ڵ�                         *
*                     tEntExtDtime - �̿�ð�                                  *
*                     boolIsXfer - ȯ�¿���                                    *
*                     boolIsBaseFare - �⺻��ݿ���                            *
*                                                                              *
*  RETURN/EXIT VALUE: dword - ���� ���                                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static dword CalcFare( byte bCardType, byte bUserType, word wTranspMethodCode,
	time_t tEntExtDtime, bool boolIsXfer, bool boolIsBaseFare )
{
	dword dwFare = 0;
	int nDiscount = 0;
	float fDiscountRate = 0.0;

	// �⺻����� ������ ///////////////////////////////////////////////////////
	dwFare = GetBaseFare( bCardType, bUserType, wTranspMethodCode );

	// ������ �� ���αݾ��� ������ /////////////////////////////////////////////
	GetDisExtraAmtRate( bCardType, bUserType, tEntExtDtime, boolIsXfer,
		boolIsBaseFare, &nDiscount, &fDiscountRate );

	// ���������̸� ���������ݾ� ���
	if ( IsWideAreaBus( wTranspMethodCode ) )
		dwFare += nDiscount;
	// �����������ƴϸ� ���������� ���
	else
		dwFare += dwFare * fDiscountRate / 100;

	return dwFare;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetBaseFare                                              *
*                                                                              *
*  DESCRIPTION:       �Էµ� ���ǿ� �ش��ϴ� �⺻����� �����Ѵ�.              *
*                                                                              *
*  INPUT PARAMETERS:  bCardType - ī������                                     *
*                     bUserType - ���������                                   *
*                     wTranspMethodCode - ��������ڵ�                         *
*                                                                              *
*  RETURN/EXIT VALUE: dword - �⺻���                                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
dword GetBaseFare( byte bCardType, byte bUserType, word wTranspMethodCode )
{
	dword dwBaseFare = 0;				// �⺻���

	// ������ ��� /////////////////////////////////////////////////////////////
	if ( bCardType == TRANS_CARDTYPE_CASH ||
		 bCardType == TRANS_CARDTYPE_STUDENT_TOKEN )
	{
		switch ( bUserType )
		{
			case USERTYPE_ADULT:
			case USERTYPE_TEST:
				dwBaseFare = gstNewFareInfo.dwAdultCashEntFare;
				break;
			case USERTYPE_YOUNG:
				dwBaseFare = gstNewFareInfo.dwYoungCashEntFare;
				break;
			case USERTYPE_CHILD:
				dwBaseFare = gstNewFareInfo.dwChildCashEntFare;
				break;
		}
	}
	// ������������� ���������� ������������� ������ ��� ////////////////////
	else if ( wTranspMethodCode == gstVehicleParm.wTranspMethodCode )
	{
		dwBaseFare = gstNewFareInfo.dwBaseFare;

#ifdef TEST_TRANS_0_WON
		dwBaseFare = 0;
#endif

#ifdef TEST_TRANS_10_WON
		dwBaseFare = 10;
#endif
	}
	// �� ���� ��� ////////////////////////////////////////////////////////////
	else
	{
		dwBaseFare = GetHardCodedBaseFare( wTranspMethodCode );
	}

	return dwBaseFare;
}

dword GetHardCodedBaseFare( word wTranspMethodCode )
{
	dword dwBaseFare = 0;

	switch ( wTranspMethodCode )
	{
		case 101:		dwBaseFare = 800;		break;
		case 102:		dwBaseFare = 800;		break;
		case 103:		dwBaseFare = 800;		break;
		case 104:		dwBaseFare = 500;		break;
		case 105:		dwBaseFare = 500;		break;
		case 110:		dwBaseFare = 800;		break;
		case 115:		dwBaseFare = 800;		break;
		case 120:		dwBaseFare = 800;		break;
		case 121:		dwBaseFare = 500;		break;
		case 130:		dwBaseFare = 1400;		break;
		case 140:		dwBaseFare = 500;		break;
		case 200:		dwBaseFare = 800;		break;
		case 201:		dwBaseFare = 800;		break;
		case 202:		dwBaseFare = 800;		break;
		case 203:		dwBaseFare = 800;		break;
		case 204:		dwBaseFare = 800;		break;
		case 122:		dwBaseFare = 600;		break;
		case 131:		dwBaseFare = 1200;		break;
		case 151:		dwBaseFare = 600;		break;
		default:
			dwBaseFare = 0;
			DebugOut( "�����ڵ忡 �´� ���ʿ����� ����\n" );
	}

	return dwBaseFare;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetDisExtraAmtRate                                       *
*                                                                              *
*  DESCRIPTION:       �Էµ� ���ǿ� �ش��ϴ� ����������/���������ݾ���         *
*                     �����´�.                                                *
*                                                                              *
*  INPUT PARAMETERS:  bCardType - ī������                                     *
*                     bUserType - ���������                                   *
*                     tEntExtDtime - �̿�ð�                                  *
*                     boolIsXfer - ȯ�¿���                                    *
*                     boolIsBaseFare - �⺻��ݿ���                            *
*                     pnDisExtraAmt - ���������ݾ��� �������� ���� ������      *
*                     pfDiscExtraRate - ������������ �������� ���� ������      *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void GetDisExtraAmtRate( byte bCardType, byte bUserType,
	time_t tEntExtDtime, bool boolIsXfer, bool boolIsBaseFare,
	int *pnDisExtraAmt, float *pfDiscExtraRate )
{
	bool boolSearchSuccess = FALSE;
	byte abDisExtraTypeID[7] = {0, };

	*pnDisExtraAmt = 0;
	*pfDiscExtraRate = 0.0;

	// �ű��������� ��� ///////////////////////////////////////////////////////
	if ( gstVehicleParm.wTranspMethodCode == 131 )
	{
		switch ( bUserType )
		{
			case USERTYPE_ADULT:
				*pnDisExtraAmt = 0;
				break;
			case USERTYPE_YOUNG:
				*pnDisExtraAmt = -200;
				break;
			case USERTYPE_CHILD:
				*pnDisExtraAmt = -300;
				break;
			default:
				*pnDisExtraAmt = 0;
				break;
		}
		*pfDiscExtraRate = 0.0;
		return;
	}

	// ������������ID�� ������ /////////////////////////////////////////////////
	CreateDisExtraTypeID( bCardType,
					bUserType,
					tEntExtDtime,
					boolIsXfer,
					abDisExtraTypeID );

	// ���������������� ������ �� ���ξ� �˻� //////////////////////////////////
	boolSearchSuccess = SearchDisExtraInfo( abDisExtraTypeID,
		boolIsBaseFare, pnDisExtraAmt, pfDiscExtraRate );

	// �˻��� ������ ��� �ϵ��ڵ��� ������ ������ �� ���αݾ� ���� ////////////
	if ( !boolSearchSuccess )
	{
		switch ( bUserType )
		{
			case USERTYPE_YOUNG:
				*pnDisExtraAmt = -280;
				*pfDiscExtraRate = -20.0;
				break;
			case USERTYPE_CHILD:
				*pnDisExtraAmt = -400;
				*pfDiscExtraRate = -50.0;
				break;
			case USERTYPE_ADULT:
			default:
				*pnDisExtraAmt = 0;
				*pfDiscExtraRate = 0.0;
				break;
		}
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateDisExtraTypeID                                     *
*                                                                              *
*  DESCRIPTION:       �Էµ� ���ǿ� �ش��ϴ� ������������ID�� �����Ѵ�.        *
*                                                                              *
*  INPUT PARAMETERS:  bCardType - ī������                                     *
*                     bUserType - ���������                                   *
*                     tEntExtDtime - �̿�ð�                                  *
*                     boolIsXfer - ȯ�¿���                                    *
*                     abDisExtraTypeID - ������������ID�� �������� ���� ������ *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
void CreateDisExtraTypeID( byte bCardType, byte bUserType,
	time_t tEntExtDtime, bool boolIsXfer, byte *abDisExtraTypeID )
{
	int i = 0;
	byte hour = 0;
	byte abEntExtDate[8] = {0, };

	abDisExtraTypeID[0] = '1';
	if ( tEntExtDtime != 0 )
	{
		TimeT2ASCDate( tEntExtDtime, abEntExtDate );
		for ( i = 0; i < gstHolidayInfoHeader.dwRecordCnt; i++ )
		{
			if ( memcmp( abEntExtDate, gpstHolidayInfo[i].abHolidayDate,
					sizeof( gpstHolidayInfo[i].abHolidayDate ) ) == 0 )
			{
				abDisExtraTypeID[0] = gpstHolidayInfo[i].bHolidayClassCode;
				break;
			}
		}
	}

	hour = GetHourFromTimeT( tEntExtDtime );
	if ( hour == 0 ) hour = 24;	// 00�ô� �����ϱ�� �ؼ� �߰�
	abDisExtraTypeID[1] = hour / 10 + '0';
	abDisExtraTypeID[2] = hour % 10 + '0';

	if ( boolIsXfer )
		abDisExtraTypeID[3] = '1';
	else
		abDisExtraTypeID[3] = '0';

	if ( bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
		 bCardType == TRANS_CARDTYPE_DEPOSIT ||
		 bCardType == TRANS_CARDTYPE_MIF_POSTPAY )
	{
		abDisExtraTypeID[1] = '0';
		abDisExtraTypeID[2] = '0';
	}

	if ( bCardType == TRANS_CARDTYPE_CASH ||
		 bCardType == TRANS_CARDTYPE_STUDENT_TOKEN )
	{
		abDisExtraTypeID[1] = '0';
		abDisExtraTypeID[2] = '0';
		abDisExtraTypeID[3] = '0';
	}

	abDisExtraTypeID[4] = bUserType / 10 + '0';
	abDisExtraTypeID[5] = bUserType % 10 + '0';
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SearchDisExtraInfo                                       *
*                                                                              *
*  DESCRIPTION:       �������������� �˻��Ͽ� ���������ݾװ� ������������      *
*                     �����Ѵ�.                                                *
*                                                                              *
*  INPUT PARAMETERS:  abDisExtraTypeID - ������������ID                        *
*                     boolIsBaseFare - �⺻��ݿ���                            *
*                     pnDisExtraAmt - ���������ݾ��� �������� ���� ������      *
*                     pfDisExtraRate - ������������ �������� ���� ������       *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - �˻�����                                          *
*                     FALSE - �˻�����                                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-24                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static bool SearchDisExtraInfo( byte *abDisExtraTypeID, bool boolIsBaseFare,
	int *pnDisExtraAmt, float *pfDisExtraRate )
{
	dword i = 0;
	byte bBaseAdd = '0';

	*pnDisExtraAmt = 0;
	*pfDisExtraRate = 0.0;

	// �������������� �ε忡 ������ ��� ������ FALSE ����
	if ( gstDisExtraInfoHeader.dwRecordCnt == 0 )
	{
		return FALSE;
	}

	if ( boolIsBaseFare == TRUE )
		bBaseAdd = '1';
	else
		bBaseAdd = '2';

	// ������������ID �� ����������������ڵ�(�⺻���/�߰���� �ʵ�)�� ���� ///
	// �������������� �˻� /////////////////////////////////////////////////////
	for ( i = 0; i < gstDisExtraInfoHeader.dwRecordCnt; i++ )
		if ( memcmp( abDisExtraTypeID, gpstDisExtraInfo[i].abDisExtraTypeID, 6 )
			== 0 && gpstDisExtraInfo[i].bDisExtraApplyCode == bBaseAdd )
		{
			*pnDisExtraAmt = gpstDisExtraInfo[i].nDisExtraAmt;
			*pfDisExtraRate = gpstDisExtraInfo[i].fDisExtraRate;
			return TRUE;
		}

	// ���� �˻����� �����ϴ� ��� /////////////////////////////////////////////
	// �⺻���/�߰���� �ʵ尡 default( '0' ) ���� ���� �������������� �˻� ///
	for ( i = 0; i < gstDisExtraInfoHeader.dwRecordCnt; i++ )
		if ( memcmp( abDisExtraTypeID, gpstDisExtraInfo[i].abDisExtraTypeID,
			 sizeof( gpstDisExtraInfo[i].abDisExtraTypeID ) ) == 0 &&
			 gpstDisExtraInfo[i].bDisExtraApplyCode == '0' )
		{
			*pnDisExtraAmt = gpstDisExtraInfo[i].nDisExtraAmt;
			*pfDisExtraRate = gpstDisExtraInfo[i].fDisExtraRate;
			return TRUE;
		}

	return FALSE;
}

static short ProcCityTourEnt( TRANS_INFO *pstTransInfo )
{
	byte i = 0;
	byte abNowDate[8] = {0, };

	DebugOut( "\n" );
	DebugOut( "#############################################################" );
	DebugOut( "##################\n" );
	DebugOut( "# ��Ƽ������� ó��\n" );
	DebugOut( "#\n" );

	// �������/���������� /////////////////////////////////////////////////////
	pstTransInfo->stNewXferInfo.bEntExtType = XFER_ENT;

	// ������ID ////////////////////////////////////////////////////////////////
	memcpy( pstTransInfo->stNewXferInfo.abStationID,
		gpstSharedInfo->abNowStationID, 7 );

	// ���ν°ŷ����� //////////////////////////////////////////////////////////
	for ( i = 0; i < 3; i++ )
	{
		pstTransInfo->stNewXferInfo.abMultiEntInfo[i][USER_TYPE] =
			pstTransInfo->abUserType[i];
		pstTransInfo->stNewXferInfo.abMultiEntInfo[i][USER_CNT] =
			pstTransInfo->abUserCnt[i];
	}

	// �� �°����� �⺻��ݰ�� �� �°� ������������ID ���� ////////////////////
	for ( i = 0; i < 3 && pstTransInfo->abUserCnt[i] != 0; i++ )
	{
		// �°� ������������ID /////////////////////////////////////////////////
		CreateDisExtraTypeID( pstTransInfo->bCardType,
			pstTransInfo->abUserType[i],
			pstTransInfo->stNewXferInfo.tEntExtDtime,
			TRUE,
			pstTransInfo->abDisExtraTypeID[i] );
	}

	// �ż���ī���� ���
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY )
	{
		DebugOut( "# �ż���ī��\n" );

		// ��� ////////////////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.dwFare =
			gstCityTourBusTicketInfo.dwTicketAmt;

		DebugOut( "#     - �������� ���                      : %5lu (��)\n",
			pstTransInfo->stNewXferInfo.dwFare );
		DebugOut( "#     - �ܾ�                                 : %5lu (��)\n",
			pstTransInfo->stPrevXferInfo.dwBal );

		if ( pstTransInfo->stNewXferInfo.dwFare >
			 pstTransInfo->stPrevXferInfo.dwBal )
		{

			DebugOut( "#     - �ܾ׺��� ����\n" );

			return ERR_CARD_PROC_INSUFFICIENT_BAL;
		}
		else
		{

			DebugOut( "#     - ���������� �ܾ� ����\n" );

			// �ܾ� ////////////////////////////////////////////////////////////
			pstTransInfo->stNewXferInfo.dwBal =
				pstTransInfo->stPrevXferInfo.dwBal -
				pstTransInfo->stNewXferInfo.dwFare;
		}
	}
	// ������ī���� ���
	else
	{
		DebugOut( "# ������ī��\n" );

		// ���ʻ���� ���� ���Ⱓ�� ���� ī���� ���
		// 'ī�� ��ȿ�Ⱓ�� �������ϴ�' ���� ���
		if ( IsValidMifTourExpiryDate( pstTransInfo->tMifTourFirstUseDtime,
				pstTransInfo->wMifTourCardType,
				pstTransInfo->stNewXferInfo.tEntExtDtime ) == FALSE )
		{
			printf( "[TransProc] ���ʻ���Ͻ� ���� ī�������� ���� " );
			printf( "�������� �ʰ��Ͽ����Ƿ� ī������ �Ұ���\n" );
			return ERR_CARD_PROC_EXPIRED_CARD;
		}

		// �������� ���� ī���� ���
		// 'ī�� ��ȿ�Ⱓ�� �������ϴ�' ���� ���
		TimeT2ASCDate( pstTransInfo->stNewXferInfo.tEntExtDtime, abNowDate);
		if ( memcmp( abNowDate, pstTransInfo->abMifTourExpiryDate, 8 ) > 0 )
		{
			printf( "[TransProc] ������ī�� ������ ���� ��� ����\n" );
			return ERR_CARD_PROC_EXPIRED_CARD;
		}

		pstTransInfo->bMifTourUseCnt = 0;

		// �ϴ������Ƚ�� //////////////////////////////////////////////////////
		if ( IsSameDate( pstTransInfo->stPrevXferInfo.tEntExtDtime,
				pstTransInfo->stNewXferInfo.tEntExtDtime ) )
		{
			DebugOut( "#     - ������ -> �ϴ������Ƚ���� Ƚ���� �ջ�\n" );

			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt =
				pstTransInfo->stPrevXferInfo.bMifTourDailyAccUseCnt;
		}
		else
		{
			DebugOut( "#     - �ٸ��� -> �ϴ������Ƚ���� �̹� Ƚ���� ����\n" );

			pstTransInfo->stNewXferInfo.bMifTourDailyAccUseCnt =
				pstTransInfo->bMifTourUseCnt;
		}

		// �Ѵ������Ƚ�� //////////////////////////////////////////////////////
		pstTransInfo->stNewXferInfo.dwMifTourTotalAccUseCnt =
			pstTransInfo->stPrevXferInfo.dwMifTourTotalAccUseCnt;
	}

	return SUCCESS;
}

