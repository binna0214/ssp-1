#include "../system/bus_type.h"
#include "card_proc.h"
#include "card_proc_util.h"
#include "blpl_proc.h"
#include "trans_proc.h"

// ������ī�� ī���ȣ�� ���Ե� ����� ���� define /////////////////////////////
#define CARD_USER_ADULT				'0'		// �Ϲ�
#define CARD_USER_STUDENT			'1'		// �л�
#define CARD_USER_CHILD				'2'		// ���
#define CARD_USER_TEST				'5'		// �׽�Ʈ

// block6 holder type�� ����� ���� define /////////////////////////////////////
#define HOLDER_ADULT				0		// �Ϲ�
#define HOLDER_NEW_STUDENT			1		// �ű��л�
#define HOLDER_REGISTERED_STUDENT	2		// ����л�
#define HOLDER_UNIVERSITY_STUDENT	3		// ���л�

static void MifPrepaySetBasicCardInfo( TRANS_INFO *pstTransInfo,
	MIF_PREPAY_SECTOR1 *pstMifPrepaySector1,
	MIF_PREPAY_SECTOR2 *pstMifPrepaySector2 );
static short MifPrepayCheckValidCard( byte *abCardNo, byte *abIssueDate );
static short MifPrepayCheckPL( TRANS_INFO *pstTransInfo,
	MIF_PREPAY_BLOCK6 *pstMifPrepayBlock6,
	MIF_PREPAY_BLOCK16 *pstMifPrepayBlock16 );
static void MifPrepaySetCardInfoStruct( TRANS_INFO *pstTransInfo,
	MIF_PREPAY_SECTOR1 *pstMifPrepaySector1,
	MIF_PREPAY_SECTOR2 *pstMifPrepaySector2,
	MIF_PREPAY_SECTOR3 *pstMifPrepaySector3,
	MIF_PREPAY_SECTOR4 *pstMifPrepaySector4,
	COMMON_XFER_DATA *pstCommonXferData );
static void MifPrepayBuildISAMTransReq( TRANS_INFO *pstTransInfo,
	ISAM_TRANS_REQ *pstISAMTransReq );
static void MifPrepayBuildISAMMakeTCC( TRANS_INFO *pstTransInfo,
	ISAM_MAKE_TCC *pstISAMMakeTCC );
static void MifPrepayBuildOldXferInfo( TRANS_INFO *pstTransInfo );
static void MifPrepayBuildISAMTransReq( TRANS_INFO *pstTransInfo,
	ISAM_TRANS_REQ *pstISAMTransReq );
static void MifPrepayBuildISAMMakeTCC( TRANS_INFO *pstTransInfo,
	ISAM_MAKE_TCC *pstISAMMakeTCC );
static void MifPrepayBuildOldXferInfo( TRANS_INFO *pstTransInfo );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepayRead                                            *
*                                                                              *
*  DESCRIPTION:       ������ī�带 READ�ϰ� ��ȿ��üũ �� PLüũ�� ������ ��   *
*                     ī����������ü�� �����Ѵ�.                               *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü�� ������                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_RETAG_CARD - ī�带 �ٽ� ���ּ���.         *
*                     ERR_CARD_PROC_CANNOT_USE - ����� �� ���� ī���Դϴ�.    *
*                     ERR_CARD_PROC_LOG - �����α׿� �����ϴ� ī���� ���      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short MifPrepayRead( TRANS_INFO *pstTransInfo )
{
	short sResult = 0;

	MIF_PREPAY_SECTOR1 stMifPrepaySector1;
	MIF_PREPAY_SECTOR2 stMifPrepaySector2;
	MIF_PREPAY_SECTOR3 stMifPrepaySector3;
	MIF_PREPAY_SECTOR4 stMifPrepaySector4;
	COMMON_XFER_DATA stCommonXferData;
byte abTempCardNo[21] = {0, };
	// ������ī�� �⺻����READ �� SAMüũ //////////////////////////////////////
	sResult = MifPrepayReadBasicInfo( pstTransInfo->dwChipSerialNo,
		&stMifPrepaySector1,
		&stMifPrepaySector2,
		&stMifPrepaySector3 );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayRead] ������ī�� �⺻����READ �� SAMüũ ");
		printf( "����\n" );
		switch ( sResult )
		{
			case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR1:
			case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR2:
			case ERR_PAY_LIB_MIF_PREPAY_READ_SECTOR3:
			case ERR_PAY_LIB_MIF_PREPAY_INVALID_BAL_BLOCK:
			case ERR_PAY_LIB_ISAM_SET_CARD_CSN:
			case ERR_PAY_LIB_ISAM_RECOVER_CASE1:
			case ERR_PAY_LIB_ISAM_RECOVER_CASE2:
			case ERR_PAY_LIB_ISAM_RECOVER_CASE3:
			case ERR_PAY_LIB_ISAM_RECOVER_CASE4:
			case ERR_PAY_LIB_ISAM_GET_CARD_INFO:
			case ERR_PAY_LIB_ISAM_CHK_TCC:
			case ERR_PAY_LIB_ISAM_CHK_CICC:
			case ERR_PAY_LIB_ISAM_CHK_CII:
			default:
				return ERR_CARD_PROC_RETAG_CARD;
		}
	}

	// ������ī�� �⺻���� ���� ////////////////////////////////////////////////
	MifPrepaySetBasicCardInfo( pstTransInfo, &stMifPrepaySector1,
		&stMifPrepaySector2 );
memcpy(abTempCardNo, pstTransInfo->abCardNo, 20);
LogMain("���� : %s\n", abTempCardNo);
	// �׽�Ʈī���� ��� ���Ұ� ó�� /////////////////////////////////////////
	if ( pstTransInfo->abCardNo[9] == CARD_USER_TEST )
	{
#ifndef TEST_NOT_CHECK_TEST_CARD
			printf( "[MifPrepayRead] �׽�Ʈ ī���� ��� ���Ұ� ó��\n" );
			return ERR_CARD_PROC_CANNOT_USE;	// '����� �� ���� ī���Դϴ�'
#endif
	}

	// ������ī�� ��ȿ�� üũ //////////////////////////////////////////////////
	sResult = MifPrepayCheckValidCard( pstTransInfo->abCardNo,
		stMifPrepaySector2.stMifPrepayBlock8.abIssueDate );
	if ( sResult != SUCCESS )
	{
		return sResult;
	}

	// ������ī�� PLüũ ///////////////////////////////////////////////////////
	sResult = MifPrepayCheckPL( pstTransInfo,
		&stMifPrepaySector1.stMifPrepayBlock6,
		&stMifPrepaySector4.stMifPrepayBlock16 );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayRead] PLüũ ����\n" );
		switch ( sResult )
		{
			case ERR_CARD_PROC_MIF_PREPAY_PL_ALIAS:
			case ERR_CARD_PROC_MIF_PREPAY_NL_CARD:
			case ERR_CARD_PROC_MIF_PREPAY_PL_CHECK:
				return ERR_CARD_PROC_NOT_APPROV;
			default:
				return ERR_CARD_PROC_RETAG_CARD;
		}
	}

	// ����LOG����Ʈ�� �����ϴ� ī������ Ȯ�� //////////////////////////////////
	// (�����ܸ����� ��� BL/PL üũ�� ���� �����ܸ����� ���������� �������Ƿ�,
	//  �� �۾��� �ݵ�� BL/PL üũ ������ �����Ͽ��� ��)
	sResult = SearchCardErrLog( pstTransInfo );
	if ( sResult == SUCCESS )
	{
		DebugOut( "[MifPrepayRead] ����LOG����Ʈ�� �����ϴ� ī��\n" );
		return ERR_CARD_PROC_LOG;
	}

	// ������ī�� ȯ������ READ ////////////////////////////////////////////////
	sResult = MifPrepayReadXferInfo( pstTransInfo->dwChipSerialNo,
		&stCommonXferData );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayRead] ������ī�� ȯ������ READ ����\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// ������ī�� ��ȯ������ READ //////////////////////////////////////////////
	sResult = MifPrepayReadOldXferInfo( pstTransInfo->dwChipSerialNo,
		&stMifPrepaySector4 );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayRead] ������ī�� ��ȯ������ READ ����\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// ī����������ü ���� /////////////////////////////////////////////////////
	MifPrepaySetCardInfoStruct( pstTransInfo,
		&stMifPrepaySector1, &stMifPrepaySector2, &stMifPrepaySector3,
		&stMifPrepaySector4, &stCommonXferData );

	return SUCCESS;
}

static void MifPrepaySetBasicCardInfo( TRANS_INFO *pstTransInfo,
	MIF_PREPAY_SECTOR1 *pstMifPrepaySector1,
	MIF_PREPAY_SECTOR2 *pstMifPrepaySector2 )
{
	// ī���ȣ ���� ///////////////////////////////////////////////////////////
	memcpy( pstTransInfo->abCardNo,
		pstMifPrepaySector2->stMifPrepayBlock8.abCardNo,
		sizeof( pstTransInfo->abCardNo ) );

#ifdef TEST_CARDTYPE_DEPOSIT
	if ( memcmp( pstTransInfo->abCardNo, TEST_CARDTYPE_DEPOSIT, 20 ) == 0 )
		memcpy( pstTransInfo->abCardNo, "0000280", 7 );
#endif

	// alias��ȣ ���� //////////////////////////////////////////////////////////
#ifdef TEST_MIF_PREPAY_ALIAS_0
	pstTransInfo->dwAliasNo = 0;
#else
	pstTransInfo->dwAliasNo = pstMifPrepaySector1->stMifPrepayBlock6.dwAliasNo;
#endif

	// ī�忡 ��ϵ� ��������� ���� ///////////////////////////////////////////
	// 0 : �Ϲ�
	// 1 : �ű��л�
	// 2 : ����л�
	// 3 : ���л�, �п���
	if ( pstMifPrepaySector1->stMifPrepayBlock6.bStudentCardType ==
			HOLDER_ADULT ||
		pstMifPrepaySector1->stMifPrepayBlock6.bStudentCardType ==
			HOLDER_UNIVERSITY_STUDENT )
	{
		pstTransInfo->bCardUserType = USERTYPE_ADULT;
	}
	else
	{
		pstTransInfo->bCardUserType = USERTYPE_YOUNG;
	}

	// ī���������� ////////////////////////////////////////////////////////////
	// ī���ȣ�� "0000280"�� ���۵Ǵ� ��� '��ġ��ī��'�� ����
	if ( memcmp( "0000280", pstTransInfo->abCardNo, 7 ) == 0 )
	{
		pstTransInfo->bCardType = TRANS_CARDTYPE_DEPOSIT;
	}
	// �� ���� ��� ������ī��� ����
	else
	{
		pstTransInfo->bCardType = TRANS_CARDTYPE_MIF_PREPAY;
	}
}

static short MifPrepayCheckValidCard( byte *abCardNo, byte *abIssueDate )
{
	bool boolResult = FALSE;

	// ������ī�� ī���ȣ üũ ////////////////////////////////////////////////
	boolResult = IsValidMifPrepayCardNo( abCardNo );
	if ( !boolResult )
	{
		printf( "[MifPrepayRead] ������ī�� ī���ȣ üũ ����\n" );
		return ERR_CARD_PROC_RETAG_CARD;		// 'ī�带 �ٽ� ���ּ���'
	}

	// ������ī�� ����� üũ //////////////////////////////////////////////////
	boolResult = IsValidPrepayIssuer( abCardNo );
	if ( !boolResult )
	{
		printf( "[MifPrepayRead] ������ī�� ����� üũ ����\n" );
		return ERR_CARD_PROC_NOT_APPROV;		// '�̽��� ī���Դϴ�'
	}

	// ������ī�� �߱��� üũ //////////////////////////////////////////////////
	boolResult = IsValidMifPrepayIssueDate( abCardNo, abIssueDate );
	if ( !boolResult )
	{
		printf( "[MifPrepayRead] ������ī�� �߱��� üũ ����\n" );
		return ERR_CARD_PROC_CANNOT_USE;		// '����� �� ���� ī���Դϴ�'
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepayCheckPL                                         *
*                                                                              *
*  DESCRIPTION:       PLüũ�� �����Ͽ� NLī�忩�ο� ����������� �����´�.    *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī������ ����ü ������                    *
*                     pstMifPrepayBlock6 - ������ī�� BLOCK6 ����ü ������     *
*                     pstMifPrepayBlock16 - ������ī�� BLOCK16 ����ü ������   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_MIF_PREPAY_PL_ALIAS - alias��ȣ ����       *
*                     ERR_CARD_PROC_MIF_PREPAY_PL_CHECK - PLüũ ȣ�� ����     *
*                     ERR_CARD_PROC_MIF_PREPAY_NL_CARD - NLī��                *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
// 1. ������ī���� ��� PL���� 2��Ʈ�� �����Ǹ�, �� 1��Ʈ�� NL(0)/PL(1) ����,
//    �� 1��Ʈ�� �Ϲ�(0)/û�ҳ�(1) ������ �ǹ̸� ������.
// 2. NL/PL �Ǵܽ� ������ ���� ���� ��Ȳ���� ����Ͽ��� �Ѵ�.
// 	- ��⿹ġ��ī��(prefix : "0000280")�� ��� NL/PL ��Ʈ�� �����ϰ� ������
// 	  PL�̴�.
// 	- alias��ȣ�� 1 ~ 30,000,000 �� ������ �ִ� ��� NL/PL ��Ʈ�� ����Ѵ�.
// 	- alias��ȣ�� 1 ~ 30,000,000 �� ���� �ܿ� �ִ� ��� �Ϲ����� ó���Ѵ�.
// 2. �Ϲ�/û�ҳ� �Ǵܽ� ������ ���� ���� ��Ȳ���� ����Ͽ��� �Ѵ�.
// 	- KSCC ������ī��( prefix : "10007", "10008" )�� �Ϲ�/û�ҳ� ��Ʈ�� �״��
// 	  �����Ѵ�.
// 	- �� �� ī���� ��� ī���ȣ�� ��ϵ� ���������(ī�� 10��° �ڸ�)��
// 	  �׽�Ʈ(5)�̸� �׽�Ʈī���̴�.
// 	- �� �� ī���� ��� ī���ȣ�� ��ϵ� ���������(ī�� 10��° �ڸ�)��
// 	  û�ҳ�(1)�� �ƴϸ� �Ϲ�/û�ҳ� ��Ʈ�� �����ϰ� ������ �Ϲ��̴�.
// 	- �� �� ī���� ��� ī���ȣ�� ��ϵ� ���������(ī�� 10��° �ڸ�)��
// 	  û�ҳ�(1)�̸� �Ϲ�/û�ҳ� ��Ʈ�� û�ҳ��̰ų� BLOCK6�� �����ڱ�����
// 	  �ű��л��̸� û�ҳ��̰� �� ���� ��� �Ϲ��̴�.
static short MifPrepayCheckPL( TRANS_INFO *pstTransInfo,
	MIF_PREPAY_BLOCK6 *pstMifPrepayBlock6,
	MIF_PREPAY_BLOCK16 *pstMifPrepayBlock16 )
{
	short sResult = 0;
	byte bPLValue = 0;

	bool boolIsPL = FALSE;
	bool boolIsYoung = FALSE;

#ifdef TEST_NOT_CHECK_BLPL
	pstTransInfo->bPLUserType = USERTYPE_ADULT;
	return SUCCESS;
#endif

	// BLOCK6�� BCC�� ��ȿ���� �ʰų� //////////////////////////////////////////
	// ī�忡 ��ϵ� alias ��ȣ�� ������ ����� ���
	if ( pstMifPrepayBlock6->boolIsValidBCC == FALSE ||
		 pstTransInfo->dwAliasNo < 1 ||
		 pstTransInfo->dwAliasNo > 30000000 )
	{
		// ����簡 '10007' �Ǵ� '10008'�̸� ������ NL
		if ( memcmp( pstTransInfo->abCardNo, "10007", 5 ) == 0 ||
			 memcmp( pstTransInfo->abCardNo, "10008", 5 ) == 0 )
		{
			return ERR_CARD_PROC_MIF_PREPAY_NL_CARD;
		}
		// �� ���� ��쿡�� ������ �Ϲ�
		else
		{
			pstTransInfo->bPLUserType = USERTYPE_ADULT;
			return SUCCESS;
		}
	}

	// PL üũ /////////////////////////////////////////////////////////////////
	sResult = SearchPLinBus( pstTransInfo->abCardNo, pstTransInfo->dwAliasNo,
		&bPLValue );

	// PL üũ ȣ�� ��ü�� ������ ��� /////////////////////////////////////////
	// - �ַ� �����ܸ��⿡�� �����ܸ������ ��� ���� �� �߻� ����
	if ( sResult == ERR_PL_FILE_OPEN_MASTER_MIF_PREPAY_PL )
	{
		printf( "[MifPrepayCheckPL] ������PL�� ������\n" );
		return ErrRet( ERR_CARD_PROC_MIF_PREPAY_PL_CHECK );
	}
	else if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayCheckPL] ��Ÿ �� �� ���� ������ ������PLüũ " );
		printf( "����\n" );
		return ErrRet( ERR_CARD_PROC_RETAG_CARD );
	}
	else
	{
		if ( memcmp( pstTransInfo->abCardNo, "10007", 5 ) != 0 &&
			 memcmp( pstTransInfo->abCardNo, "10008", 5 ) != 0 )
		{
			DebugOut( "[MifPrepayCheckPL] eB / KSCC ī�尡 �ƴϹǷ� ������ " );
			DebugOut( "PLó��\n" );
			bPLValue |= 2;
		}

		// PL üũ ����� 0, 1, 2, 3�� �ƴϸ� 'ī�带 �ٽ� ���ּ���' ����
		if ( bPLValue >= 4 )
		{
			DebugOut( "[MifPrepayCheckPL] ������ī�� PLüũ ����� " );
			DebugOut( "0, 1, 2, 3�� �ƴ�\n" );
			return ERR_CARD_PROC_MIF_PREPAY_PL_CHECK;
		}

		if ( ( bPLValue & 0x02 ) == 0x02 )
			boolIsPL = TRUE;

		if ( ( bPLValue & 0x01 ) == 0x01 )
			boolIsYoung = TRUE;
	}

	// PL ��������� ���� //////////////////////////////////////////////////////

	// KSCC ������ī���� ��� PL�� ��ϵ� �Ϲ�/û�ҳ� ��Ʈ�� ���� ����
	// PL ��������� ����
	if ( memcmp( pstTransInfo->abCardNo, "10007", 5 ) == 0 ||
		 memcmp( pstTransInfo->abCardNo, "10008", 5 ) == 0 )
	{
		if ( boolIsYoung )
		{
			pstTransInfo->bPLUserType = USERTYPE_YOUNG;
		}
		else
		{
			pstTransInfo->bPLUserType = USERTYPE_ADULT;
		}
	}
	// �� �� ī���� ��� ī�忡 ��ϵ� ����������� �ֿ켱 �Ǵ� ������ ��
	else
	{
		// ī���ȣ�� ����������� '0'�̸� ������ �Ϲ�
		if ( pstTransInfo->abCardNo[9] == CARD_USER_ADULT )
		{
			pstTransInfo->bPLUserType = USERTYPE_ADULT;
		}
		// ī���ȣ�� ����������� '1'�̸鼭
		// BLOCK6�� ����������� '�ű��л�'�̸� ������ û�ҳ�
		else if ( pstTransInfo->abCardNo[9] == CARD_USER_STUDENT &&
				 pstMifPrepayBlock6->bStudentCardType == HOLDER_NEW_STUDENT )
		{
			pstTransInfo->bPLUserType = USERTYPE_YOUNG;
		}
		// �� ���� ��� PL�� ������ �Ǵ�
		else
		{
			if ( boolIsYoung )
			{
				pstTransInfo->bPLUserType = USERTYPE_YOUNG;
			}
			else
			{
				pstTransInfo->bPLUserType = USERTYPE_ADULT;
			}
		}
	}

	// NL/PL ���� �Ǵ� /////////////////////////////////////////////////////////
	if ( memcmp( "0000280", pstTransInfo->abCardNo, 7 ) != 0 && !boolIsPL )
	{
		return ERR_CARD_PROC_MIF_PREPAY_NL_CARD;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepaySetCardInfoStruct                               *
*                                                                              *
*  DESCRIPTION:       ī��κ��� ���� ������ �̿��Ͽ� ī������ ����ü��        *
*                     �����Ѵ�.                                                *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī������ ����ü ������                    *
*                     pstMifPrepaySector1 - ������ī�� SECTOR1 READ ���       *
*                     pstMifPrepaySector2 - ������ī�� SECTOR2 READ ���       *
*                     pstMifPrepaySector3 - ������ī�� SECTOR3 READ ���       *
*                     pstMifPrepaySector4 - ������ī�� SECTOR4 READ ���       *
*                     pstCommonXferData - ȯ������ READ ���                   *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void MifPrepaySetCardInfoStruct( TRANS_INFO *pstTransInfo,
	MIF_PREPAY_SECTOR1 *pstMifPrepaySector1,
	MIF_PREPAY_SECTOR2 *pstMifPrepaySector2,
	MIF_PREPAY_SECTOR3 *pstMifPrepaySector3,
	MIF_PREPAY_SECTOR4 *pstMifPrepaySector4,
	COMMON_XFER_DATA *pstCommonXferData )
{
	byte abBuf[14];

	// T������ī�� ����
	pstTransInfo->boolIsTCard =
		pstMifPrepaySector1->stMifPrepayBlock6.boolIsTCard;

	// ����������ī���ܾ� - ������ī��
	pstTransInfo->dwBalAfterCharge =
		pstMifPrepaySector4->stMifPrepayBlock17.dwBalAfterCharge;

	// ����������ī��ŷ��Ǽ� - ������ī��
	// ( "�����"�� Long 4�ڸ� "yyyymmdd"�� ��� )
	memset( abBuf, 0, sizeof( abBuf ) );
	TimeT2ASCDtime( pstMifPrepaySector4->stMifPrepayBlock16.tChargeDtime,
		abBuf );
	pstTransInfo->dwChargeTransCnt = GetDWORDFromASC( abBuf, 8 );

	// ���������ݾ� - ������ī��
	pstTransInfo->dwChargeAmt =
		pstMifPrepaySector4->stMifPrepayBlock16.dwChargeAmt;

	// ����������SAMID - ������ī��
	memset( pstTransInfo->abLSAMID, '0', sizeof( pstTransInfo->abLSAMID ) );
	if ( pstTransInfo->boolIsTCard == TRUE )
	{
		pstTransInfo->abLSAMID[0] = '1';
	}
	else
	{
		pstTransInfo->abLSAMID[0] = '0';
	}
	memcpy( &pstTransInfo->abLSAMID[1],
		pstMifPrepaySector4->stMifPrepayBlock16.abOLSAMID,
		sizeof( pstMifPrepaySector4->stMifPrepayBlock16.abOLSAMID ) );

	// ����������SAM�ŷ��Ϸù�ȣ - ������ī��
	// 	( "�ú���"�� Long 4�ڸ� "hhmmss"�� ��� )
	pstTransInfo->dwLSAMTransCnt = GetDWORDFromASC( &abBuf[8], 6 );

	// ������ī�� �����������ι�ȣ
	memcpy( pstTransInfo->abMifPrepayChargeAppvNop,
		pstMifPrepaySector4->stMifPrepayBlock17.abAppvNo,
		sizeof( pstMifPrepaySector4->stMifPrepayBlock17.abAppvNo ) );
	memcpy( &pstTransInfo->abMifPrepayChargeAppvNop[10],
		pstMifPrepaySector4->stMifPrepayBlock18.abChargeAppvNo,
		sizeof( pstMifPrepaySector4->stMifPrepayBlock18.abChargeAppvNo ) );

	// ��ī�� �ܸ���׷��ڵ� ( ��ȯ�¿��� ��� )
	pstTransInfo->bMifTermGroupCode =
		pstMifPrepaySector4->stMifPrepayBlock18.bLastUseTranspCode;

	// ��ī�� �̿�ð� ( ��ȯ�¿������ )
	pstTransInfo->tMifEntExtDtime =
		pstMifPrepaySector4->stMifPrepayBlock18.tLastExtDtime;

	// ����ȯ������
	memcpy( &pstTransInfo->stPrevXferInfo, pstCommonXferData,
		sizeof( COMMON_XFER_DATA ) );

	// ������ī�� BLOCK18
	memcpy( &pstTransInfo->stMifPrepayBlock18,
		&pstMifPrepaySector4->stMifPrepayBlock18,
		sizeof( MIF_PREPAY_BLOCK18 ) );

	// ����ȯ������ - �ܾ�
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		pstTransInfo->stPrevXferInfo.dwBal =
			pstMifPrepaySector4->stMifPrepayBlock18.dwDepositCardBal;
	}
	else
	{
		pstTransInfo->stPrevXferInfo.dwBal = pstMifPrepaySector2->dwBalBlock9;
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepayWrite                                           *
*                                                                              *
*  DESCRIPTION:       ������ �ű�ȯ�������� ī�忡 WRITE�ϰ� ����� ī����     *
*                     �ܾ����κ��� DECREMENT�Ѵ�.                              *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī������ ����ü                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_RETAG_CARD - ī�带 �ٽ� ���ּ���.         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short MifPrepayWrite( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;
	ISAM_TRANS_REQ stISAMTransReq;
	ISAM_MAKE_TCC stISAMMakeTCC;

	// ISAM_TRANS_REQ ���� /////////////////////////////////////////////////////
	MifPrepayBuildISAMTransReq( pstTransInfo, &stISAMTransReq );

	// ISAM_MAKE_TCC ���� //////////////////////////////////////////////////////
	MifPrepayBuildISAMMakeTCC( pstTransInfo, &stISAMMakeTCC );

	// ������ �߻��Ͽ� ��ó���ϸ鼭 ��ġ��ī�尡 �ƴ� ��� /////////////////////
	// ��ġ��ī���� ��� �ܾ����� �� ������ ���� ��� ���� ó������ �ٽ�
	// �õ��ϸ� ��
	if ( pstTransInfo->bWriteErrCnt > 0 &&
		 pstTransInfo->bCardType != TRANS_CARDTYPE_DEPOSIT )
	{
		MIF_PREPAY_SECTOR2 stMifPrepaySector2;

		DebugOut( "[MifPrepayWrite] ������ī���̸鼭 ������ �߻��� ���� " );
		DebugOut( "����\n" );

		sResult = MifPrepayReadSector2( pstTransInfo->dwChipSerialNo,
			&stMifPrepaySector2 );
		if ( sResult != SUCCESS )
		{
			return ERR_CARD_PROC_RETAG_CARD;
		}

		// ���� ī��κ��� ���� �ܾ��� ����ȯ�������� �ܾװ� ���� ���� ���
		// �̹� ������ī�� WRITE�� ��� �׼��� �Ϸ�Ǿ�����,
		// BLOCK9�� ���� DECREMENT �� �ܾ� �񱳽� ������ �߻��Ͽ��ٰ�
		// �̷�� ������ �� �ִ�. ���� �̷��� ���� �ٷ� TCC�� ������ ��
		// SUCCESS�� �����Ѵ�.
		// ��, PSAM v2 ī��� TCC�� ��������� �ʴ´�.
		if ( stMifPrepaySector2.dwBalBlock9 !=
			 pstTransInfo->stPrevXferInfo.dwBal )
		{
			if ( pstTransInfo->boolIsTCard == TRUE )
			{
				DebugOut( "[MifPrepayWrite] T������ī���� ��� TCC�� \n");
				DebugOut( "����� ����\n");
			}
			else
			{
				byte abTempLastTransInfo[16] = {0, };

				DebugOut( "[MifPrepayWrite] �ܾ��� �������̹Ƿ� TCC�� ����\n" );

				// ISAMMakeTCC ���� ������ �ݵ�� ISAMTransReq�� ����Ǿ�� ��
				ISAMTransReq( &stISAMTransReq, abTempLastTransInfo );

				// ISAM TCC ���� ///////////////////////////////////////////////
				sResult = ISAMMakeTCC( &stISAMMakeTCC,
					pstTransInfo->abMifPrepayTCC );
				if ( sResult != SUCCESS )
				{
					printf( "[MifPrepayWrite] ISAMMakeTCC() ����\n" );
					AddCardErrLog( sResult, pstTransInfo );
					return ERR_CARD_PROC_RETAG_CARD;
				}
			}

			// ��ó���� ī���� ��� ����LOG�κ��� �ش� ī�������� ���� /////////
			if ( pstTransInfo->sErrCode != SUCCESS )
			{
				DeleteCardErrLog( pstTransInfo->abCardNo );
			}

			return SUCCESS;
		}
		else
		{
			DebugOut( "[MifPrepayWrite] �ܾ��� ��ũ����Ʈ �����̹Ƿ� " );
			DebugOut( "ó������ �ٽ� ������\n" );
		}
	}

	// ������ �߻��Ͽ��� ��ġ��ī���� ��� ����� �޽��� ���
	if ( pstTransInfo->bWriteErrCnt > 0 &&
		 pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		DebugOut( "[MifPrepayWrite] ��ġ��ī���� ��� ó������ �ٽ� ������\n" );
	}

	// ������ī�� ��ȯ������ ��ȯ //////////////////////////////////////////////
	MifPrepayBuildOldXferInfo( pstTransInfo );

	// ������ī�� ��ȯ������WRITE //////////////////////////////////////////////
	sResult = MifPrepayWriteOldXferInfo( pstTransInfo->dwChipSerialNo,
		&pstTransInfo->stMifPrepayBlock18 );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayWrite] ������ī�� ��ȯ������WRITE ����\n" );
		AddCardErrLog( sResult, pstTransInfo );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// ������ī�� ȯ������WRITE ////////////////////////////////////////////////
	sResult = MifPrepayWriteXferInfo( pstTransInfo->dwChipSerialNo,
		&pstTransInfo->stNewXferInfo );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayWrite] ������ī�� ȯ������WRITE ����\n" );
		AddCardErrLog( sResult, pstTransInfo );
		return ERR_CARD_PROC_RETAG_CARD;
	}

#ifdef TEST_WRITE_SLEEP
	printf( "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n" );
	sleep( 2 );
#endif

	// ������ī�� �ܾ� DECREMENT ///////////////////////////////////////////////
	// ��ġ��ī���� ��� 0�� DECREMENT
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		sResult = MifPrepayDecrementBalance( pstTransInfo->dwChipSerialNo,
			pstTransInfo->boolIsTCard,
			0, 0xFFFFFFFF, &stISAMTransReq, &stISAMMakeTCC,
			gstMyTermInfo.abPSAMID, pstTransInfo->abMifPrepayTCC );
	}
	// �Ϲݱ�����ī���� ���
	else
	{
		sResult = MifPrepayDecrementBalance( pstTransInfo->dwChipSerialNo,
			pstTransInfo->boolIsTCard,
			pstTransInfo->stNewXferInfo.dwFare,
			pstTransInfo->stNewXferInfo.dwBal,
			&stISAMTransReq, &stISAMMakeTCC,
			gstMyTermInfo.abPSAMID, pstTransInfo->abMifPrepayTCC );
	}
	if ( sResult != SUCCESS )
	{
		printf( "[MifPrepayWrite] ������ī�� �ܾ� DECREMENT ����\n" );
		AddCardErrLog( sResult, pstTransInfo );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// ��ó���� ī���� ��� ����LOG�κ��� �ش� ī�������� ���� /////////////////
	if ( pstTransInfo->sErrCode != SUCCESS )
	{
		DeleteCardErrLog( pstTransInfo->abCardNo );
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepayBuildISAMTransReq                               *
*                                                                              *
*  DESCRIPTION:       ī������ ����ü�� ���������� ������ �̿��Ͽ� ISAM��      *
*                     TRANS REQ ��ɾ� ȣ�⿡ �ʿ��� ����ü�� �����Ѵ�.        *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī������ ����ü                           *
*                     pstISAMTransReq - ISAM TRANS REQ ��ɾ� ȣ�⿡ �ʿ���    *
*                         ����ü                                               *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-09-22                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void MifPrepayBuildISAMTransReq( TRANS_INFO *pstTransInfo,
	ISAM_TRANS_REQ *pstISAMTransReq )
{
	memset( ( byte * )pstISAMTransReq, 0, sizeof( ISAM_TRANS_REQ ) );

	// �������ð�
	pstISAMTransReq->tEntExtDtime = pstTransInfo->stNewXferInfo.tEntExtDtime;

	// �����������
	pstISAMTransReq->bTranspType = 1;				// 1: ����, 2: ö��

	// �ܸ���ID
	memcpy( pstISAMTransReq->abTermID, gstMyTermInfo.abISAMID,
		sizeof( pstISAMTransReq->abTermID ) );

	// ����������
	if ( IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
		pstISAMTransReq->bEntExtType = 1;			// 1: ����, 0: ����
	else
		pstISAMTransReq->bEntExtType = 0;

	// ������ID
	pstISAMTransReq->wStationID =
		GetWORDFromASC( pstTransInfo->stNewXferInfo.abStationID, 7 );

	// ���
	// - ��ġ��ī���� ��� 0��
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		pstISAMTransReq->dwFare = 0;
	}
	// - �Ϲ� ������ī���� ���
	else
	{
		pstISAMTransReq->dwFare = pstTransInfo->stNewXferInfo.dwFare;
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepayBuildISAMMakeTCC                                *
*                                                                              *
*  DESCRIPTION:       ī������ ����ü�� ���������� ������ �̿��Ͽ� ISAM��      *
*                     MAKE TCC ��ɾ� ȣ�⿡ �ʿ��� ����ü�� �����Ѵ�.         *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī������ ����ü                           *
*                     pstISAMMakeTCC - ISAM MAKE TCC ��ɾ� ȣ�⿡ �ʿ���      *
*                         ����ü                                               *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-09-22                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void MifPrepayBuildISAMMakeTCC( TRANS_INFO *pstTransInfo,
	ISAM_MAKE_TCC *pstISAMMakeTCC )
{
	memset( ( byte * )pstISAMMakeTCC, 0, sizeof( ISAM_MAKE_TCC ) );

	// �������ð�
	pstISAMMakeTCC->tEntExtDtime = pstTransInfo->stNewXferInfo.tEntExtDtime;

	// ISAM ID
	pstISAMMakeTCC->dwISAMID = GetDWORDFromASC( gstMyTermInfo.abISAMID, 7 );

	// ī���ȣ
	memcpy( pstISAMMakeTCC->abCardNo, pstTransInfo->abCardNo,
		sizeof( pstISAMMakeTCC->abCardNo ) );

	// ���
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		pstISAMMakeTCC->dwFare = 0;
	}
	// - �Ϲ� ������ī���� ���
	else
	{
		pstISAMMakeTCC->dwFare = pstTransInfo->stNewXferInfo.dwFare;
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPrepayBuildOldXferInfo                                *
*                                                                              *
*  DESCRIPTION:       ī������ ����ü�� ���������� ������ �̿��Ͽ�             *
*                     ī�忡 WRITE�� ������ī�� ��ȯ�¿����� ������ �����Ѵ�.  *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī������ ����ü                           *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-09-22                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void MifPrepayBuildOldXferInfo( TRANS_INFO *pstTransInfo )
{
	// ������뱳������ڵ�
	pstTransInfo->stMifPrepayBlock18.bLastUseTranspCode = 1;
												// 0: ö��, 1: ����

	// ���������ܸ���SAM ID
	if ( pstTransInfo->boolIsTCard == TRUE )
	{
		memcpy( pstTransInfo->stMifPrepayBlock18.abLastISAMID,
			&gstMyTermInfo.abPSAMID[9],
			sizeof( pstTransInfo->stMifPrepayBlock18.abLastISAMID ) );
	}
	else
	{
		memcpy( pstTransInfo->stMifPrepayBlock18.abLastISAMID,
			gstMyTermInfo.abISAMID,
			sizeof( pstTransInfo->stMifPrepayBlock18.abLastISAMID ) );
	}

	// ���������Ͻ�
	pstTransInfo->stMifPrepayBlock18.tLastExtDtime =
		pstTransInfo->stNewXferInfo.tEntExtDtime;

	// ȯ��Ƚ�� - �״�� ����
	// TODO : FLAG.xfer

	// ������������������Ƚ�� - �״�� ����

	// ī���������ȣ - 1 ����
	pstTransInfo->stMifPrepayBlock18.bCardUseSeqNo++;
	if ( pstTransInfo->stMifPrepayBlock18.bCardUseSeqNo >= 0xFF )
		pstTransInfo->stMifPrepayBlock18.bCardUseSeqNo = 0x01;

	// �������ι�ȣ - �״�� ����

	// ȯ������ó������FLAG - �켱 FALSE�� ����
	// TODO : FLAG.xfer
	pstTransInfo->stMifPrepayBlock18.boolIsXferDis = FALSE;

	// ��ġ��ī�崩���ݾ�
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		pstTransInfo->stMifPrepayBlock18.dwDepositCardBal =
			pstTransInfo->stNewXferInfo.dwBal;
	}
	// ��ġ��ī�崩���ݾ� - ��ġ��ī�� �̿��� ��� �״�� ����
}
