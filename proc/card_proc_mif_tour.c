#include "../system/bus_type.h"
#include "card_proc.h"
#include "card_proc_util.h"
#include "blpl_proc.h"
#include "trans_proc.h"

static void MifTourSetBasicCardInfo( TRANS_INFO *pstTransInfo,
	MIF_TOUR_SECTOR1 *pstMifTourSector1,
	MIF_TOUR_BLOCK16 *pstMifTourBlock16 );
static short MifTourCheckValidCard( TRANS_INFO *pstTransInfo,
	byte *abIssueDate );

short MifTourRead( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;
	MIF_TOUR_SECTOR1 stMifTourSector1;
	MIF_TOUR_BLOCK16 stMifTourBlock16;

	// ������ī�� �⺻����READ �� üũ /////////////////////////////////////////
	sResult = MifTourReadBasicInfo( pstTransInfo->dwChipSerialNo,
		&stMifTourSector1,
		&stMifTourBlock16 );
	if ( sResult != SUCCESS )
	{
		printf( "[MifTourRead] MifTourReadBasicInfo() ����\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// ������ī�� �⺻���� ���� ////////////////////////////////////////////////
	MifTourSetBasicCardInfo( pstTransInfo,
		&stMifTourSector1,
		&stMifTourBlock16 );

	// ������ī�� ��ȿ�� üũ //////////////////////////////////////////////////
	sResult = MifTourCheckValidCard( pstTransInfo,
		stMifTourSector1.abIssueDate );
	if ( sResult != SUCCESS )
	{
		return sResult;
	}

	// ����LOG����Ʈ�� �����ϴ� ī������ Ȯ�� //////////////////////////////////
	// (�����ܸ����� ��� BL/PL üũ�� ���� �����ܸ����� ���������� �������Ƿ�,
	//  �� �۾��� �ݵ�� BL/PL üũ ������ �����Ͽ��� ��)
	sResult = SearchCardErrLog( pstTransInfo );
	if ( sResult == SUCCESS )
	{
		DebugOut( "[MifTourRead] ����LOG����Ʈ�� �����ϴ� ī��\n" );
		return ERR_CARD_PROC_LOG;
	}

	// ������ī�� ȯ������ READ ////////////////////////////////////////////////
	sResult =  MifTourReadXferInfo( pstTransInfo->dwChipSerialNo,
		&pstTransInfo->stPrevXferInfo );
	if ( sResult != SUCCESS )
	{
		printf( "[MifTourRead] ������ī�� ȯ������ READ ����\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	return SUCCESS;
}

static void MifTourSetBasicCardInfo( TRANS_INFO *pstTransInfo,
	MIF_TOUR_SECTOR1 *pstMifTourSector1,
	MIF_TOUR_BLOCK16 *pstMifTourBlock16 )
{
	// ī���ȣ ���� ///////////////////////////////////////////////////////////
	memset( pstTransInfo->abCardNo, 'F', sizeof( pstTransInfo->abCardNo ) );
	memcpy( pstTransInfo->abCardNo, pstMifTourSector1->abEpurseID,
		sizeof( pstMifTourSector1->abEpurseID ) );

	// alias��ȣ ���� //////////////////////////////////////////////////////////
	pstTransInfo->dwAliasNo = pstMifTourSector1->dwCardUserCertiID;

	// ī�忡 ��ϵ� ����� ���� ///////////////////////////////////////////////
	pstTransInfo->bCardUserType = pstMifTourSector1->bUserTypeCode;

	// ī�� ���� ///////////////////////////////////////////////////////////////
	// �ŷ������� ��ϵǴ� ī�� ������ �ƴϸ�, ���� ó�� �� ������ ���Ͽ� ����
	pstTransInfo->bCardType = TRANS_CARDTYPE_MIF_TOUR;

	// ������ī�� ���� ��� �Ͻ� ///////////////////////////////////////////////
	pstTransInfo->tMifTourFirstUseDtime = pstMifTourBlock16->tFirstUseDtime;

	// ������ī�� ������ ///////////////////////////////////////////////////////
	memcpy( pstTransInfo->abMifTourExpiryDate, pstMifTourSector1->abExpiryDate,
		sizeof( pstTransInfo->abMifTourExpiryDate ) );

	// ������ī�� ���� /////////////////////////////////////////////////////////
	pstTransInfo->wMifTourCardType = pstMifTourSector1->wTourCardType;

}

static short MifTourCheckValidCard( TRANS_INFO *pstTransInfo,
	byte *abIssueDate )
{
	bool boolResult = FALSE;
	short sResult = SUCCESS;
	byte bPLValue = 0;
	byte abCardIssuerNo[7] = {0, };

#ifndef TEST_NOT_CHECK_ISSUER
	abCardIssuerNo[0] = '3';
	memcpy( &abCardIssuerNo[1], pstTransInfo->abCardNo, 6 );
	boolResult = IsValidPrepayIssuer( abCardIssuerNo );
	if ( boolResult == FALSE )
	{
		printf( "[MifTourCheckValidCard] ������ī�� ����� üũ ����\n" );
		return ERR_CARD_PROC_NOT_APPROV;
	}
#endif

	boolResult = IsValidSCPrepayCardNo( pstTransInfo->abCardNo );
	if ( !boolResult )
	{
		printf( "[MifTourCheckValidCard] ������ī�� ī���ȣ ����\n" );
		return ERR_CARD_PROC_NOT_APPROV;
	}

	if ( pstTransInfo->tMifTourFirstUseDtime != 0 &&
		 pstTransInfo->stNewXferInfo.tEntExtDtime <
			pstTransInfo->tMifTourFirstUseDtime )
	{
		printf( "[MifTourCheckValidCard] ������ī�� ���ʻ���Ͻ� ���� ����\n" );
		return ERR_CARD_PROC_CANNOT_USE;
	}

#ifdef TEST_NOT_CHECK_PL
	pstTransInfo->bPLUserType = pstTransInfo->bCardUserType;
	return SUCCESS;
#endif

	// PL üũ /////////////////////////////////////////////////////////////////
	sResult = SearchPLinBus( pstTransInfo->abCardNo, pstTransInfo->dwAliasNo,
		&bPLValue );
	if ( sResult == ERR_PL_FILE_OPEN_MASTER_AI )
	{
		printf( "[MifTourCheckValidCard] �ż���PL�� ������\n" );
		switch ( pstTransInfo->bCardUserType )
		{
			// ī���� ����� ���� �ڵ尡 ����� ��� ��̷� ��
			case USERTYPE_CHILD:
				pstTransInfo->bPLUserType = USERTYPE_CHILD;
				break;
			// ī�� ����� ���� �ڵ尡 û�ҳ�/�л��� ���� û�ҳ� �����
			case USERTYPE_STUDENT:
			case USERTYPE_YOUNG:
				pstTransInfo->bPLUserType = USERTYPE_YOUNG;
				break;
			case USERTYPE_TEST:
				pstTransInfo->bPLUserType = USERTYPE_TEST;
				break;
			default:
				pstTransInfo->bPLUserType = USERTYPE_ADULT;
				break;
		}
		return SUCCESS;
	}
	if ( sResult != SUCCESS )
	{
		printf( "[MifTourCheckValidCard] ��Ÿ �� �� ���� ������ " );
		printf( "PL üũ ����\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	if ( bPLValue == 0 )
	{
		printf( "[MifTourCheckValidCard] NL�� ī��\n" );
		return ERR_CARD_PROC_NOT_APPROV;
	}
	else if ( bPLValue > 3 )
	{
		printf( "[MifTourCheckValidCard] PL üũ ����� 3���� ŭ\n" );
		return ERR_CARD_PROC_RETAG_CARD;
	}

	// pstTransInfo->bPLUserType ���� ��ݰ���� ���� ����� �����ڵ�� �����
	// 0338 �ҽ����� �׷��� ������ ��� ����� ���ؼ���
	// NL�� �ƴ� ī�忡 ���ؼ��� PL ������� ī���� ����ڱ��� ��
	// ����� ���� ������ ������ ����ϵ��� �Ѵٰ� ��
	// ���� ī���� ����� ������ �پ��ϰ� ������ �ǹǷ�
	// ���� ���/�����/���������� ���� ����� �ٸ��� �����ȴٸ�
	// �̺κ� �����ؾ� ��
	switch ( pstTransInfo->bCardUserType )
	{
		// ī���� ����� ���� �ڵ尡 ����� ���� PLüũ ������� ����
		case USERTYPE_CHILD:
			switch ( bPLValue )
			{
				case 1:		// P/L ��Ʈ���� û�ҳ� ���
					pstTransInfo->bPLUserType = USERTYPE_YOUNG;
					break;
				case 2:		// P/L ��Ʈ���� ��� ���
					pstTransInfo->bPLUserType = USERTYPE_CHILD;
					break;
				default:	// P/L ��Ʈ���� �Ϲ� ���
					pstTransInfo->bPLUserType = USERTYPE_ADULT;
					break;
			}
			break;
		// ī�� ����� ���� �ڵ尡 û�ҳ�/�л��� ����
		// PLüũ ������� �Ϲ��ΰ��� �Ϲݿ���̰� �������� û�ҳ� �����
		case USERTYPE_STUDENT:
		case USERTYPE_YOUNG:
			if ( bPLValue == 3 )
			{
				pstTransInfo->bPLUserType = USERTYPE_ADULT;
			}
			else
			{
				pstTransInfo->bPLUserType = USERTYPE_YOUNG;
			}
			break;
		case USERTYPE_TEST :
			pstTransInfo->bPLUserType = USERTYPE_TEST;
			break;
		default :
			pstTransInfo->bPLUserType = USERTYPE_ADULT;
			break;
	}

	return SUCCESS;
}

short MifTourWrite( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;

	pstTransInfo->stNewXferInfo.bMifPostpayRecSeqNo =
		( pstTransInfo->stPrevXferInfo.bMifPostpayRecSeqNo + 1 ) % 256;

	pstTransInfo->stNewXferInfo.bMifPostpayReadXferSectorNo =
		pstTransInfo->stPrevXferInfo.bMifPostpayReadXferSectorNo;

	// ������ī�� ���ʻ������ WRITE ///////////////////////////////////////////
	if ( pstTransInfo->tMifTourFirstUseDtime == 0 )
	{
		sResult = MifTourWriteFirstUseInfo( pstTransInfo->dwChipSerialNo,
			pstTransInfo->abCardNo, pstTransInfo->stNewXferInfo.tEntExtDtime,
			gpstSharedInfo->abMainTermID );
		if ( sResult != SUCCESS )
		{
			printf( "[MifTourWrite] ������ī�� ���ʻ������ WRITE ����\n" );
			AddCardErrLog( sResult, pstTransInfo );
			return ERR_CARD_PROC_RETAG_CARD;
		}
	}

	// ������ī�� ȯ������WRITE ////////////////////////////////////////////////
	// ��Ƽ��������� ��� ȯ�������� WRITE���� ����
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == FALSE )
	{
		sResult = MifTourWriteXferInfo( pstTransInfo->dwChipSerialNo,
			&pstTransInfo->stNewXferInfo );
		if ( sResult != SUCCESS )
		{
			printf( "[MifTourWrite] ������ī�� ȯ������WRITE ����\n" );
			AddCardErrLog( sResult, pstTransInfo );
			return ERR_CARD_PROC_RETAG_CARD;
		}
	}

	// ��ó���� ī���� ��� ����LOG�κ��� �ش� ī�������� ���� /////////////////
	if ( pstTransInfo->sErrCode != SUCCESS )
	{
		DeleteCardErrLog( pstTransInfo->abCardNo );
	}

	return SUCCESS;
}

