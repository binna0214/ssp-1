#include "../system/bus_type.h"
#include "card_proc.h"
#include "card_proc_util.h"
#include "blpl_proc.h"
#include "trans_proc.h"

// BL / PL üũ�� �߻� ���� ////////////////////////////////////////////////////
#define CHECK_BL					0		// BLüũ ���ĺ�ī��
#define CHECK_PL					1		// PLüũ ���ĺ�ī��
#define CHECK_PL_READ_SECTOR5_ERR	2		// PLüũ ���ĺ�ī���ӿ���
											// SECTOR5 READ�� �����ϴ� ���
#define CHECK_PL_ALIAS_ERR			3		// PLüũ ���ĺ�ī��������
											// alias��ȣ�� ������ ����� ���
#define CHECK_PL_CICC_ERR			4		// CICC�ڵ� ISAMüũ ����

static void MifPostpaySetBasicCardInfo( TRANS_INFO *pstTransInfo,
	MIF_POSTPAY_SECTOR0 *pstMifPostpaySector0,
	MIF_POSTPAY_SECTOR12 *pstMifPostpaySector12 );
static short MifPostpayCheckValidCard( TRANS_INFO *pstTransInfo,
	byte bIssuerCode, byte *abExpiryDate );
static short MifPostpayCheckBLPL( TRANS_INFO *pstTransInfo, byte bIssuerCode );
static byte MifPostpayGetBLPLType( dword dwChipSerialNo, byte bIssuerCode,
	dword *pdwAliasNo, byte *pbSavedPLValue );
static void MifPostpaySetCardInfoStruct( TRANS_INFO *pstTransInfo,
	MIF_POSTPAY_OLD_XFER_DATA *pstMifPostpayOldXferData,
	COMMON_XFER_DATA *pstCommonXferData );
static short MifPostpayIsBLCard( byte *abCardNo, byte bIssuerCode,
	byte *pbChkBLResult );
static void MifPostpayBuildOldXferInfo( TRANS_INFO *pstTransInfo );


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayRead                                           *
*                                                                              *
*  DESCRIPTION:       ���ĺ�ī�带 READ�ϰ� ��ȿ��üũ �� PL/BLüũ�� ������ ��*
*                     ī����������ü�� �����Ѵ�.                               *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü�� ������                   *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_RETAG_CARD - ī�带 �ٽ� ���ֽʽÿ�.       *
*                     ERR_CARD_PROC_CANNOT_USE - ����� �� ���� ī���Դϴ�.    *
*                     ERR_CARD_PROC_EXPIRED_CARD - ī�� ��ȿ�Ⱓ�� �������ϴ�. *
*                     ERR_CARD_PROC_NOT_APPROV - �̽��� ī���Դϴ�.            *
*                     ERR_CARD_PROC_LOG - �����α׿� �����ϴ� ī���� ���      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short MifPostpayRead( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;

	MIF_POSTPAY_SECTOR0 stMifPostpaySector0;
	MIF_POSTPAY_SECTOR12 stMifPostpaySector12;
	MIF_POSTPAY_OLD_XFER_DATA stMifPostpayOldXferData;
	COMMON_XFER_DATA stCommonXferData;
byte abTempCardNo[21] = {0, };
	// ���ĺ�ī�� �⺻����READ �� SAMüũ //////////////////////////////////////
	sResult = MifPostpayReadBasicInfo( pstTransInfo->dwChipSerialNo,
		&stMifPostpaySector0, &stMifPostpaySector12 );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayRead] ���ĺ�ī�� �⺻����READ �� SAMüũ ����\n" );
		return ERR_CARD_PROC_RETAG_CARD;		// 'ī�带 �ٽ� ���ּ���'
	}

	// ���ĺ�ī�� �⺻���� ���� ////////////////////////////////////////////////
	MifPostpaySetBasicCardInfo( pstTransInfo, &stMifPostpaySector0,
		&stMifPostpaySector12 );
memcpy(abTempCardNo, pstTransInfo->abCardNo, 20);
LogMain("���� : %s\n", abTempCardNo);
	// �׽�Ʈī���� ��� ���Ұ� ó�� /////////////////////////////////////////
	if ( stMifPostpaySector0.bCardType == '9' )
	{
		// �׽�Ʈī���� ��� ���Ұ� ó��
#ifndef TEST_NOT_CHECK_TEST_CARD
		printf( "[MifPostpayRead] �׽�Ʈ ī���� ��� ���Ұ� ó��\n" );
		return ERR_CARD_PROC_CANNOT_USE;		// '����� �� ���� ī���Դϴ�'
#endif
	}

	// ���ĺ�ī�� ��ȿ�� üũ //////////////////////////////////////////////////
	if ( IsAllZero( stMifPostpaySector12.abMifPostpayBlockBinary48,
			sizeof( stMifPostpaySector12.abMifPostpayBlockBinary48 ) ) ||
		 IsAllFF( stMifPostpaySector12.abMifPostpayBlockBinary48,
		 	sizeof( stMifPostpaySector12.abMifPostpayBlockBinary48 ) ) )
	{
		printf( "[MifPostpayRead] BLOCK48 ���� ���� (��� 0x00 or 0xFF)\n" );
		return ERR_CARD_PROC_CANNOT_USE;		// '����� �� ���� ī���Դϴ�'
	}

	sResult = MifPostpayCheckValidCard( pstTransInfo,
		stMifPostpaySector0.bIssuerCode,
		stMifPostpaySector12.abExpiryDate );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_MIF_POSTPAY_INVALID_CARD_NO:
			case ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER_VALID_PERIOD:
				return ERR_CARD_PROC_CANNOT_USE;
												// '����� �� ���� ī���Դϴ�'
			case ERR_CARD_PROC_MIF_POSTPAY_EXPIRE:
				return ERR_CARD_PROC_EXPIRED_CARD;
												// 'ī�� ��ȿ�Ⱓ�� �������ϴ�'
			case ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER:
				return ERR_CARD_PROC_NOT_APPROV;
												// '�̽��� ī���Դϴ�'
			default:
				return ERR_CARD_PROC_RETAG_CARD;
												// 'ī�带 �ٽ� ���ּ���'
		}
	}

#ifndef TEST_NOT_CHECK_BLPL
	// ���ĺ�ī�� BL/PL üũ ///////////////////////////////////////////////////
	sResult = MifPostpayCheckBLPL( pstTransInfo,
		stMifPostpaySector0.bIssuerCode );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayRead] ���ĺ�ī�� BL/PL üũ ����\n" );
		switch ( sResult )
		{
			case ERR_CARD_PROC_NOT_APPROV:
			case ERR_CARD_PROC_CANNOT_USE:
			case ERR_CARD_PROC_RETAG_CARD:
				return sResult;
			default:
				return ERR_CARD_PROC_RETAG_CARD;
												// 'ī�带 �ٽ� ���ּ���'
		}
	}
#endif

	// ����LOG����Ʈ�� �����ϴ� ī������ Ȯ�� //////////////////////////////////
	// (�����ܸ����� ��� BL/PL üũ�� ���� �����ܸ����� ���������� �������Ƿ�,
	//  �� �۾��� �ݵ�� BL/PL üũ ������ �����Ͽ��� ��)
	sResult = SearchCardErrLog( pstTransInfo );
	if ( sResult == SUCCESS )
	{
		DebugOut( "[MifPostpayRead] ����LOG����Ʈ�� �����ϴ� ī��\n" );
		return ERR_CARD_PROC_LOG;
	}

	// ���ĺ�ī�� ȯ������ READ ////////////////////////////////////////////////
	sResult =  MifPostpayReadXferInfo( pstTransInfo->dwChipSerialNo,
		stMifPostpaySector0.bIssuerCode, &stCommonXferData );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayRead] ���ĺ�ī�� ȯ������ READ ����\n" );
		return ERR_CARD_PROC_RETAG_CARD;		// 'ī�带 �ٽ� ���ּ���'
	}

	// ���ĺ�ī�� ��ȯ������ READ //////////////////////////////////////////////
	sResult =  MifPostpayReadOldXferInfo( pstTransInfo->dwChipSerialNo,
		&stMifPostpayOldXferData );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayRead] ���ĺ�ī�� ��ȯ������ READ ����\n" );
		return ERR_CARD_PROC_RETAG_CARD;		// 'ī�带 �ٽ� ���ּ���'
	}

	// ī����������ü ���� /////////////////////////////////////////////////////
	MifPostpaySetCardInfoStruct( pstTransInfo, &stMifPostpayOldXferData,
		&stCommonXferData );

	return SUCCESS;
}

static void MifPostpaySetBasicCardInfo( TRANS_INFO *pstTransInfo,
	MIF_POSTPAY_SECTOR0 *pstMifPostpaySector0,
	MIF_POSTPAY_SECTOR12 *pstMifPostpaySector12 )
{
	// ī�������� ���ĺ�ī��� ���� ////////////////////////////////////////////
	pstTransInfo->bCardType = TRANS_CARDTYPE_MIF_POSTPAY;

	// ī���ȣ ���� ///////////////////////////////////////////////////////////
	memset( pstTransInfo->abCardNo, 'F', sizeof( pstTransInfo->abCardNo ) );
	// �ＺAmexī���� ��� ī���ȣ�� 15�ڸ�
	if ( pstMifPostpaySector0->bIssuerCode == ISS_SS &&
		pstMifPostpaySector12->abCardNo[0] == '3' )
	{
		memcpy( pstTransInfo->abCardNo, pstMifPostpaySector12->abCardNo, 15 );
	}
	// �׿� ī���� ��� ī���ȣ�� 16�ڸ�
	else
	{
		memcpy( pstTransInfo->abCardNo, pstMifPostpaySector12->abCardNo, 16 );
	}

	// ��������� ���� /////////////////////////////////////////////////////////
	pstTransInfo->bPLUserType = USERTYPE_ADULT;
	pstTransInfo->bCardUserType = USERTYPE_ADULT;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayCheckValidCard                                 *
*                                                                              *
*  DESCRIPTION:       ���ĺ�ī�� ��ȿ�� üũ�� �����Ѵ�.                       *
*                     (ī���ȣ üũ, ��ȿ�Ⱓ üũ, prefix üũ)              *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü�� ������                   *
*                     bIssuerCode - ������ڵ�                                 *
*                     abExpiryDate - ��ȿ�Ⱓ (YYYYMM)                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_MIF_POSTPAY_INVALID_CARD_NO - ī���ȣ ����*
*                     ERR_CARD_PROC_MIF_POSTPAY_EXPIRE - ��ȿ�Ⱓ ����         *
*                     ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER - prefix ����   *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short MifPostpayCheckValidCard( TRANS_INFO *pstTransInfo,
	byte bIssuerCode, byte *abExpiryDate )
{
	if ( !IsValidPostpayCardNo( pstTransInfo->abCardNo, bIssuerCode ) )
	{
		printf( "[MifPostpayCheckValidCard] ī���ȣ ����\n" );
		return ERR_CARD_PROC_MIF_POSTPAY_INVALID_CARD_NO;
	}

#ifndef TEST_NOT_CHECK_EXPIRY_DATE
	if ( !IsValidExpiryDate( abExpiryDate,
		pstTransInfo->stNewXferInfo.tEntExtDtime ) )
	{
		printf( "[MifPostpayCheckValidCard] ��ȿ�Ⱓ ����\n" );
		return ERR_CARD_PROC_MIF_POSTPAY_EXPIRE;
	}
#endif

	if ( !IsValidPostpayIssuer( pstTransInfo->abCardNo ) )
	{
		printf( "[MifPostpayCheckValidCard] �ĺҹ���� üũ ����\n" );
		return ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER;
	}

	if ( !IsValidIssuerValidPeriod( pstTransInfo->abCardNo,
		abExpiryDate ) )
	{
		printf( "[MifPostpayCheckValidCard] �ĺҹ������ȿ�Ⱓ üũ ����\n" );
		return ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER_VALID_PERIOD;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayCheckBLPL                                      *
*                                                                              *
*  DESCRIPTION:       ���ĺ�ī�� BL / PL üũ�� �����Ѵ�.                      *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī����������ü�� ������                   *
*                     bIssuerCode - ������ڵ�                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_CANNOT_USE - BCī�� �Ϻ� prefix�� ���     *
*                     ERR_CARD_PROC_NOT_APPROV - NLī��                        *
*                     ERR_CARD_PROC_RETAG_CARD - ��Ÿ ������ ���±� ��û       *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short MifPostpayCheckBLPL( TRANS_INFO *pstTransInfo, byte bIssuerCode )
{
	short sResult = SUCCESS;
	byte bBLPLType = 0;
	byte bResult = 0;
	byte bSavedPLValue = 0;
	dword dwTempAliasNo = 0;

	bBLPLType = MifPostpayGetBLPLType( pstTransInfo->dwChipSerialNo,
		bIssuerCode, &dwTempAliasNo, &bSavedPLValue );
	pstTransInfo->dwAliasNo = dwTempAliasNo;

	switch ( bBLPLType )
	{
		case CHECK_BL:
		case CHECK_PL_ALIAS_ERR:
			DebugOut( "[MifPostpayCheckBLPL] BLüũī��\n" );
			if ( bIssuerCode == ISS_LG )
			{
				printf( "[MifPostpayCheckBLPL] ����ī���̹Ƿ� NL ����\n" );
				return ERR_CARD_PROC_NOT_APPROV;
			}
			if ( IsBCCardInvalidBIN( pstTransInfo->abCardNo ) )
			{
				printf( "[MifPostpayCheckBLPL] BCī�� ���Ұ� " );
				printf( "BIN�̹Ƿ� '����� �� ���� ī���Դϴ�' ���� ���\n" );
				return ERR_CARD_PROC_CANNOT_USE;
			}

			sResult =
				MifPostpayIsBLCard( pstTransInfo->abCardNo, bIssuerCode,
					&bResult );
			if ( sResult != SUCCESS )
			{
				return sResult;
			}

			// BL üũ ����� 1 �̸� '�̽��� ī���Դϴ�' ����
			if ( bResult == 1 )
				return ERR_CARD_PROC_NOT_APPROV;

			// BL üũ ����� 0, 1�� �ƴϸ� 'ī�带 �ٽ� ���ּ���' ����
			if ( bResult >= 2 )
			{
				printf( "[MifPostpayCheckBLPL] ���ĺ�ī�� BLüũ ����� ");
				printf( "0, 1�� �ƴ�\n" );
				return ERR_CARD_PROC_RETAG_CARD;
			}

			break;
		case CHECK_PL:
			DebugOut( "[MifPostpayCheckBLPL] PLüũī��\n" );
			sResult = SearchPLinBus( pstTransInfo->abCardNo, pstTransInfo->dwAliasNo,
				&bResult );
			if ( sResult != SUCCESS )
			{
				// ���ĺ�ī�� PLüũ ȣ�� ��ü�� ������ ��� ī�忡 ����Ǿ�
				// �ִ� ���� ���� PLüũ ����� ����Ѵ�.
				bResult = bSavedPLValue;
			}
			else
			{
				DebugOut( "[MifPostpayCheckBLPL] PL üũ ��� : %u\n",
					bResult );
				DebugOut( "[MifPostpayCheckBLPL] ���� PL ��� : %u\n",
					bSavedPLValue );

				// PL üũ ����� BLOCK21�� ����� ���� PL ����� ���� /////////
				if ( bResult == bSavedPLValue )
				{
					DebugOut( "[MifPostpayCheckBLPL] PL üũ ����� " );
					DebugOut( "BLOCK21�� ����� ���� PL ����� �����ϹǷ� " );
					DebugOut( "���� BLOCK21�� ���ο� PL üũ ����� " );
					DebugOut( "WRITE���� ����\n" );
				}
				// PL üũ ����� BLOCK21�� ����� ���� PL ����� �ٸ� /////////
				else
				{
					MIF_POSTPAY_BLOCK21 stMifPostpayBlock21;

					DebugOut( "[MifPostpayCheckBLPL] PL üũ ����� " );
					DebugOut( "BLOCK21�� ����� ���� PL ����� �ٸ��Ƿ� " );
					DebugOut( "BLOCK21�� ���ο� PL üũ ����� WRITE��\n" );

					// ���ĺ�ī�� PLüũ�� ������ ��� �� ����� BLOCK21��
					// �����Ѵ�.
					memset( &stMifPostpayBlock21, 0,
						sizeof( MIF_POSTPAY_BLOCK21 ) );
					stMifPostpayBlock21.bSavedPLValue = bResult;
					// WRITE ����� �����Ѵ�. - �����ص� �׸�, �����ص� �׸�
					MifPostpayWriteBlock21( pstTransInfo->dwChipSerialNo,
						&stMifPostpayBlock21 );
				}
			}

			// PL üũ ����� 0 �̸� '�̽��� ī���Դϴ�' ����
			if ( bResult == 0 )
				return ERR_CARD_PROC_NOT_APPROV;

			// PL üũ ����� 0, 1�� �ƴϸ� 'ī�带 �ٽ� ���ּ���' ����
			if ( bResult >= 2 )
			{
				printf( "[MifPostpayCheckBLPL] ���ĺ�ī�� PLüũ ����� " );
				printf( "0, 1�� �ƴ�\n" );
				return ERR_CARD_PROC_RETAG_CARD;
			}

			break;
		case CHECK_PL_READ_SECTOR5_ERR:
			printf( "[MifPostpayCheckBLPL] PL SECTOR5 READ ����\n" );
			return ERR_CARD_PROC_RETAG_CARD;
			break;
		case CHECK_PL_CICC_ERR:
			printf( "[MifPostpayCheckBLPL] PL CICC �ڵ� ����\n" );
			return ERR_CARD_PROC_RETAG_CARD;
			break;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayGetBLPLType                                    *
*                                                                              *
*  DESCRIPTION:       ���ĺ�ī�� BL�� üũ���� PL�� üũ���� �Ǵ��Ͽ� �����Ѵ�.*
*                                                                              *
*  INPUT PARAMETERS:  dwChipSerialNo - Ĩ�ø����ȣ                            *
*                     pdwAliasNo - alias��ȣ�� �����ϱ� ���� ������            *
*                     bIssuerCode - ������ڵ�                                 *
*                     pbSavedPLValue - ī�忡 ����� PLüũ ���               *
*                                                                              *
*  RETURN/EXIT VALUE: CHECK_BL - BL üũ ī��                                  *
*                     CHECK_PL - PL üũ ī��                                  *
*                     CHECK_PL_READ_SECTOR5_ERR - PL üũ ī���ӿ��� �ұ��ϰ�  *
*                         SECTOR5 READ�� �����ϴ� ���                         *
*                     CHECK_PL_ALIAS_ERR - PL üũ ī���̸鼭 alias ���� ����  *
*                     CHECK_PL_CICC_ERR - PL üũ ī���̸鼭 CICC üũ ����    *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static byte MifPostpayGetBLPLType( dword dwChipSerialNo, byte bIssuerCode,
	dword *pdwAliasNo, byte *pbSavedPLValue )
{
	short sResult = SUCCESS;
	MIF_POSTPAY_SECTOR5 stMifPostpaySector5;

	if ( bIssuerCode == ISS_KM )
	{
		memset( ( byte * )pdwAliasNo, 0, sizeof( dword ) );
		return CHECK_BL;
	}

	sResult = MifPostpayReadSector5( dwChipSerialNo, &stMifPostpaySector5 );
	if ( sResult != SUCCESS )
	{
		return CHECK_PL_READ_SECTOR5_ERR;
	}

#ifdef TEST_MIF_POSTPAY_BLOCK20_ALL_ZERO
	memset( stMifPostpaySector5.abMifPostpayBlockBinary20, 0,
		sizeof( stMifPostpaySector5.abMifPostpayBlockBinary20 ) );
#endif

	// SECTOR5�� ����Ű�� ���������� �߱޵��� ���� ���
	// 0xffffffffffff ���� Ű�� READ�� ������ ��찡 ���� �� ����.
	// ������ �� ��� SECTOR5���� �� BLOCK�� ����
	// ��� 0x00 �̰ų� 0xff �� ������ �����ǹǷ� �̿� ���� ó���� ������.
	if ( IsAllZero( stMifPostpaySector5.abMifPostpayBlockBinary20,
			sizeof( stMifPostpaySector5.abMifPostpayBlockBinary20 ) ) ||
		 IsAllFF( stMifPostpaySector5.abMifPostpayBlockBinary20,
		 	sizeof( stMifPostpaySector5.abMifPostpayBlockBinary20 ) ) )
	{
		return CHECK_BL;
	}

	sResult = ISAMSetCardCSN( dwChipSerialNo );
	if ( sResult != SUCCESS )
	{
		return CHECK_PL_CICC_ERR;
	}

	sResult = ISAMCheckCICC( stMifPostpaySector5.abMifPostpayBlockBinary20 );
	if ( sResult != SUCCESS )
	{
		return CHECK_PL_CICC_ERR;
	}

#ifdef TEST_MIF_POSTPAY_INVALID_ALIAS
	return CHECK_PL_ALIAS_ERR;
#endif
	// ALIAS��ȣ�� ���ĺ�ī�� ����( 30000001 ~ 90000000 )���� ��� ���� ó��
	if ( stMifPostpaySector5.dwAliasNo < 30000001 ||
		 stMifPostpaySector5.dwAliasNo > 90000000 )
	{
		return CHECK_PL_ALIAS_ERR;
	}

	memcpy( pdwAliasNo, &stMifPostpaySector5.dwAliasNo, sizeof( dword ) );
	*pbSavedPLValue = stMifPostpaySector5.stMifPostpayBlock21.bSavedPLValue;

	return CHECK_PL;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpaySetCardInfoStruct                              *
*                                                                              *
*  DESCRIPTION:       ī��κ��� ���� ������ �̿��Ͽ� ī������ ����ü��        *
*                     �����Ѵ�.                                                *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī������ ����ü ������                    *
*                     pstMifPostpayOldXferData - ���ĺ�ī�� ��ȯ������         *
*                         READ ���                                            *
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
static void MifPostpaySetCardInfoStruct( TRANS_INFO *pstTransInfo,
	MIF_POSTPAY_OLD_XFER_DATA *pstMifPostpayOldXferData,
	COMMON_XFER_DATA *pstCommonXferData )
{
	// ��ī�� �ܸ���׷��ڵ� ( ��ȯ�¿��� ��� )
	pstTransInfo->bMifTermGroupCode = pstMifPostpayOldXferData->bTranspTypeCode;

	// ��ī�� �̿�ð� ( ��ȯ�¿������ )
	pstTransInfo->tMifEntExtDtime = pstMifPostpayOldXferData->tUseDtime;

	// ����ȯ������
	memcpy( &pstTransInfo->stPrevXferInfo, pstCommonXferData,
		sizeof( COMMON_XFER_DATA ) );

	// ���ĺ�ī�� ��ȯ������
	memcpy( &pstTransInfo->stOldXferInfo, pstMifPostpayOldXferData,
		sizeof( MIF_POSTPAY_OLD_XFER_DATA ) );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayIsBLCard                                       *
*                                                                              *
*  DESCRIPTION:       BL üũ�� �����ϰ� ����� �����Ѵ�.                      *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - ī���ȣ                                      *
*                     bIssuerCode - ������ڵ�                                 *
*                     pbChkBLResult - BLüũ ����� �������� ���� ������       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     NOT SUCCESS - SearchBL() �Լ��� �����ϴ� ����            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short MifPostpayIsBLCard( byte *abCardNo, byte bIssuerCode,
	byte *pbChkBLResult )
{
	short sResult = SUCCESS;
	dword dwCardNum = 0;
	byte abPrefix[6] = {0, };
	byte abTempBuf[9] = {0, };

	memcpy( abPrefix, abCardNo, sizeof( abPrefix ) );

	// �ＺAMEXī�� ////////////////////////////////////////////////////////////
	if ( IsSamsungAmexCard( abCardNo ) )
	{
		dwCardNum = GetDWORDFromASC( &abCardNo[6], 8 );
	}
	// �Ｚ����ī�� ////////////////////////////////////////////////////////////
	else if ( abCardNo[0] == '9' &&
		( bIssuerCode == 0x00 || bIssuerCode == 0x02 ||
		  bIssuerCode == ISS_SS ) )
	{
		memcpy( abTempBuf, &abCardNo[6], 7 );
		memcpy( &abTempBuf[7], &abCardNo[14], 2 );
		dwCardNum = GetDWORDFromASC( abTempBuf, 9 );
	}
	// �׿� ī�� ///////////////////////////////////////////////////////////////
	else
	{
		dwCardNum = GetDWORDFromASC( &abCardNo[6], 9 );
	}

	sResult = SearchBLinBus( abCardNo, abPrefix, dwCardNum, pbChkBLResult );
	if ( sResult == ERR_BL_MASTER_BL_FILE_NOT_EXIST )
	{
		printf( "[MifPostpayIsBLCard] �ĺ�BL�� ������\n" );
		return ERR_CARD_PROC_NOT_APPROV;
	}
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayIsBLCard] ��Ÿ �� �� ���� ������ �ĺ�BLüũ " );
		printf( "����\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayWrite                                          *
*                                                                              *
*  DESCRIPTION:       ������ �ű�ȯ�������� ī�忡 WRITE�Ѵ�.                  *
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
short MifPostpayWrite( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;

	pstTransInfo->stNewXferInfo.bMifPostpayRecSeqNo =
		( pstTransInfo->stPrevXferInfo.bMifPostpayRecSeqNo + 1 ) % 256;

	pstTransInfo->stNewXferInfo.bMifPostpayReadXferSectorNo =
		pstTransInfo->stPrevXferInfo.bMifPostpayReadXferSectorNo;

	// ���ĺ�ī�� ��ȯ������ ���� //////////////////////////////////////////////
	MifPostpayBuildOldXferInfo( pstTransInfo );

	// ���ĺ�ī�� ȯ������WRITE ////////////////////////////////////////////////
	sResult = MifPostpayWriteXferInfo( pstTransInfo->dwChipSerialNo,
		&pstTransInfo->stNewXferInfo );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayWrite] ���ĺ�ī�� ȯ������WRITE ����\n" );
		AddCardErrLog( sResult, pstTransInfo );
		return ERR_CARD_PROC_RETAG_CARD;
	}

#ifdef TEST_WRITE_SLEEP
	printf( "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n" );
	sleep( 2 );
#endif

	// ���ĺ�ī�� ��ȯ������WRITE //////////////////////////////////////////////
	sResult = MifPostpayWriteOldXferInfo( pstTransInfo->dwChipSerialNo,
		&pstTransInfo->stOldXferInfo );
	if ( sResult != SUCCESS )
	{
		printf( "[MifPostpayWrite] ���ĺ�ī�� ��ȯ������WRITE ����\n" );
		AddCardErrLog( sResult, pstTransInfo );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// ��ó���� ī���� ��� ����LOG�κ��� �ش� ī�������� ���� /////////////////
	if ( pstTransInfo->sErrCode != SUCCESS )
		DeleteCardErrLog( pstTransInfo->abCardNo );

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       MifPostpayBuildOldXferInfo                               *
*                                                                              *
*  DESCRIPTION:       ī������ ����ü�� ���������� ������ �̿��Ͽ�             *
*                     ī�忡 WRITE�� ���ĺ�ī�� ��ȯ�¿����� ������ �����Ѵ�.  *
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
static void MifPostpayBuildOldXferInfo( TRANS_INFO *pstTransInfo )
{
	// 2. �̿뿪�ڵ�
	// TODO : �̿뿪�ڵ� ���� ��� Ȯ��
	memset( pstTransInfo->stOldXferInfo.abUseStationID, 0xFF,
		sizeof( pstTransInfo->stOldXferInfo.abUseStationID ) );

	// 3. ����������
	if ( IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
	{
		pstTransInfo->stOldXferInfo.bEntExtType = 0;	// ���� 0, ���� 1
	}
	else
	{
		pstTransInfo->stOldXferInfo.bEntExtType = 1;	// ���� 0, ���� 1
	}
	// 4. �̿���ܱ���
	pstTransInfo->stOldXferInfo.bTranspTypeCode = 1;	// ö�� 0, ���� 1

	// 5. Format
	pstTransInfo->stOldXferInfo.bFormat = 1;

	// 6. ����
	pstTransInfo->stOldXferInfo.bRegion = 0;

	// 7. �����κ���ȯ�±���
	if ( pstTransInfo->stNewXferInfo.bAccXferCnt > 0 )
	{
		pstTransInfo->stOldXferInfo.bXfer = 1;
	}
	else
	{
		pstTransInfo->stOldXferInfo.bXfer = 0;
	}

	// 8. ����봩���ݾ�
	if ( !IsSameMonth( pstTransInfo->stOldXferInfo.tUseDtime,
		pstTransInfo->stNewXferInfo.tEntExtDtime ) )
	{
		pstTransInfo->stOldXferInfo.dwBal = 0;
	}
	pstTransInfo->stOldXferInfo.dwBal += pstTransInfo->stNewXferInfo.dwFare;

	// 9. ���Ƚ��
	pstTransInfo->stOldXferInfo.wUseCnt =
		( pstTransInfo->stOldXferInfo.wUseCnt + 1 ) % 0xFFFF;

	// 10. ����Ʈ��ȣ
	pstTransInfo->stOldXferInfo.bGateNo = 0;

	// 11. ���꿩��
	pstTransInfo->stOldXferInfo.bSettm = 0;

	// 1. �̿�ð�
	pstTransInfo->stOldXferInfo.tUseDtime =
		pstTransInfo->stNewXferInfo.tEntExtDtime;
}

