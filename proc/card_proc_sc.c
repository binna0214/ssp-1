#include "../system/bus_type.h"
#include "card_proc.h"
#include "card_proc_util.h"
#include "blpl_proc.h"
#include "trans_proc.h"

/*******************************************************************************
*  Declaration of Defines                                                      *
*******************************************************************************/

// ī�� ���� �з� ���� /////////////////////////////////////////////////////////
#define NORMAL						0		// ���� �ŷ�����
#define REPURCHASE					1		// ��ŷ�����
#define PREPAY_CANCEL				2		// ����ī�� ��Ұŷ� �� �ŷ�
#define POSTPAY_CANCEL				3		// �ĺ�ī�� ��Ұŷ� �� �ŷ�
#define REWRITE						4		// ȯ�¿��� �ٽþ���
#define ABNORMAL					5		// ������ �ŷ�
											// (SAM ���� -> �ŷ����� ���常 ��)

static short SCReadPurseInfo( SC_EF_PURSE_INFO *pstPurseInfo );
static short SCReadPurseAndTrans( SC_EF_PURSE_INFO *pstPurseInfo,
	COMMON_XFER_DATA *pstTrans,
	SC_EF_PURSE *pstPurse,
	SC_EF_PURSE *pstPurseLoad );
static short SCReadVerifyPurse( SC_EF_PURSE *pstPurse,
	SC_EF_PURSE *pstPurseLoad );
static short SCReadPayPrev( SC_EF_PURSE *pstPurse, COMMON_XFER_DATA *pstTrans );
static short SCPrepayCheckValidCard( TRANS_INFO *pstTransInfo,
	time_t tCardIssueDate );
static short SCPostpayCheckValidCard( TRANS_INFO *pstTransInfo,
	time_t tCardExpriryDate );
static void SCComTransInfo( SC_EF_PURSE_INFO *pstPurseInfo,
	SC_EF_PURSE *pstPurse,
	SC_EF_PURSE *pstPurseLoad,
	COMMON_XFER_DATA *pstCommonXferData,
	TRANS_INFO *pstTransInfo );
static short SCNormalPayment( TRANS_INFO *pstTransInfo );
static short SCRePayment( byte bWriteType, TRANS_INFO *pstTransInfo );
static void SCDeComTransInfo( TRANS_INFO *pstTransInfo,
	SC_EF_PURSE_INFO *pstPurseInfo,
	COMMON_XFER_DATA *pstCommonXferData );
static void SCSavePSAMResult( short sResult,
	PSAM_RES_TRANS *pstPSAMResult,
	TRANS_INFO *pstTransInfo );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCRead                                                   *
*                                                                              *
*  DESCRIPTION:       ��ī�� READ                                              *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_SCREAD_PURSEINFO                           *
*                     ERR_CARD_PROC_LOG                                        *
*                     ERR_CARD_PROC_SCREAD_TRANS                               *
*                     ERR_CARD_PROC_INSUFFICIENT_BAL                           *
*                     ERR_CARD_PROC_SCREAD_PURSE                               *
*                     ERR_CARD_PROC_SCREAD                                     *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SCRead( TRANS_INFO *pstTransInfo )
{
	// �Լ� ����� ����
	short 				sResult = 0;

	// ��ī���� ������������, ����������������,
	// ȯ�������� ������ ��� ����ü ����
	SC_EF_PURSE_INFO	stPurseInfo;
	COMMON_XFER_DATA	stTrans;
	SC_EF_PURSE		    stPurse;
	SC_EF_PURSE		    stPurseLoad;
byte abTempCardNo[21] = {0, };
	// ���� �ʱ�ȭ
	memset( ( byte * )&stPurseInfo, 0, sizeof( SC_EF_PURSE_INFO ) );
	memset( ( byte * )&stTrans,	 0, sizeof( COMMON_XFER_DATA ) );
	memset( ( byte * )&stPurse,	 0, sizeof( SC_EF_PURSE ) );
	memset( ( byte * )&stPurseLoad,	 0, sizeof( SC_EF_PURSE ) );
	memset( pstTransInfo->abCardNo, 'F', sizeof( pstTransInfo->abCardNo ) );

	DebugOut( "[smh] SCRead ����------------\n" );

	// Select DF - ����ī���� AID�� ������������������ �����Ͽ�
	// ������������������ ������
	sResult = SCReadPurseInfo( &stPurseInfo );

	if ( sResult != SUCCESS )
	{
		DebugOut( "SCReadPurseInfo Read Error! !\n" );
		return ErrRet( sResult );
	}

	// �������� ���� ���� ��ȯ
	// ī�� Ÿ�� ��ȯ
	memcpy( pstTransInfo->abCardNo, stPurseInfo.abEpurseID,
		sizeof( stPurseInfo.abEpurseID ) );
	pstTransInfo->dwAliasNo =
		GetDWORDFromASC( stPurseInfo.abCardUserCertiID,10 );
	if ( ( stPurseInfo.bCardType & 0xF0 ) == 0 )
		pstTransInfo->bCardType = TRANS_CARDTYPE_SC_PREPAY;
	else if ( ( stPurseInfo.bCardType & 0xF0 ) == 0x10 )
		pstTransInfo->bCardType = TRANS_CARDTYPE_SC_POSTPAY;
	pstTransInfo->bCardUserType	= stPurseInfo.bUserTypeCode;
memcpy(abTempCardNo, pstTransInfo->abCardNo, 20);
LogMain("�� : %s\n", abTempCardNo);
	DebugOut( "[SCREAD] ��ī�� PL üũ�� ������\n" );
	// ����ī�� üũ
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY  )
		sResult = SCPrepayCheckValidCard( pstTransInfo,
			stPurseInfo.tIssueDate );
	// �ĺ�ī�� üũ
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_SC_POSTPAY )
		sResult = SCPostpayCheckValidCard( pstTransInfo,
										  stPurseInfo.tExpiryDate );

	if ( sResult != SUCCESS )
	{
		DebugOut( "[SC_READ] ī�� ��ȿ�� üũ ����\n" );
		switch ( sResult )
		{
			case ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER_VALID_PERIOD:
			case ERR_CARD_PROC_MIF_POSTPAY_INVALID_CARD_NO:
			case ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER:
				return ERR_CARD_PROC_CANNOT_USE;
			case ERR_CARD_PROC_MIF_POSTPAY_EXPIRE:
				return ERR_CARD_PROC_EXPIRED_CARD;
			case ERR_CARD_PROC_NOT_APPROV:
			case ERR_CARD_PROC_CANNOT_USE:
			case ERR_CARD_PROC_RETAG_CARD:
				return sResult;
			default:
				return ERR_CARD_PROC_RETAG_CARD;
		}
	}

	// ȯ�� �� �ŷ� ���� �б� ��
	// ���������� ȯ�� ���� �̻� ���� Ȯ��
	sResult = SCReadPurseAndTrans( &stPurseInfo, &stTrans, &stPurse,
		&stPurseLoad );
	if ( sResult != SUCCESS )
	{
		if ( ( sResult == ERR_CARD_PROC_SCREAD_VERIFY_PURSE ) ||
			 ( sResult == ERR_CARD_PROC_SCREAD_PURSE_LOAD ) )
			sResult= ERR_CARD_PROC_CANNOT_USE;
		else if ( sResult == ERR_CARD_PROC_INSUFFICIENT_BAL )
		{
			sResult = ErrRet( ERR_CARD_PROC_INSUFFICIENT_BAL );
		}
		else
		{
			sResult = ErrRet( ERR_CARD_PROC_RETAG_CARD );
		}

		return ErrRet( sResult );
	}

	// ����LOG����Ʈ�� �����ϴ� ī������ Ȯ��
	sResult = SearchCardErrLog( pstTransInfo );
	if ( sResult == SUCCESS )
	{
		return ErrRet( ERR_CARD_PROC_LOG ); // ���� ó��
	}


	// ȯ������ ����ü ����
	SCComTransInfo( &stPurseInfo, &stPurse, &stPurseLoad, &stTrans,
		pstTransInfo );

	PrintTransInfo( pstTransInfo );
	DebugOut( "[smh] SCRead ��------------\n" );
	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCReadPurseInfo                                          *
*                                                                              *
*  DESCRIPTION:       ��ī�� �������� �������� �б�                            *
*                                                                              *
*  INPUT PARAMETERS:  SC_EF_PURSE_INFO *pstPurseInfo                           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_RETAG_CARD                                 *
*                     ERR_CARD_PROC_CANNOT_USE                                 *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-11-17 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCReadPurseInfo( SC_EF_PURSE_INFO *pstPurseInfo )
{
	// �Լ� ����� ����
	short 	sResult = 0;


	// Select DF - ����ī���� AID�� ������������������ �����Ͽ� ������ ������
	sResult = SCSelectFilePurseDF( pstPurseInfo );

	if ( sResult != SUCCESS )
	{
		printf( "[��ī�� �б�] ���������������� �б� ���� \n" );
		sResult = ERR_CARD_PROC_RETAG_CARD;
		return ErrRet( sResult );
	}

	// ������������ ��ȿ�� Ȯ��
	if ( IsValidSCPurseInfo( pstPurseInfo ) != TRUE )
	{
		printf( "[��ī�� �б�]���������������� ������ [E-77] !!\n" );
		// �� �ҽ����� 77 ������� �ϴ� ���� ī�� ����
		sResult = ERR_CARD_PROC_CANNOT_USE;
		return ErrRet( sResult );
	}

	// ���� �ܸ��⿡�� ��� ������ ī�� üũ
	if ( ( ( pstPurseInfo->bCardType & 0xF0 ) != 0x10 ) 	//�ĺ�ī�� �ƴ�
		 &&( ( pstPurseInfo->bCardType & 0xF0 ) != 0x00 ) )	//����ī�� �ƴ�
	{
		printf( "[��ī�� �б�]����ī�� ����/�ĺ� ī�� �ƴ� \n" );
		sResult = ERR_CARD_PROC_CANNOT_USE;
		return ErrRet( sResult );
	}

#ifndef TEST_NOT_CHECK_TEST_CARD
	if ( pstPurseInfo->bEpurseIssuerID   == 0x01 )		//�׽�Ʈ ī��
	{
		printf( "[��ī�� �б�]�׽�Ʈ ī����!\n" );
		sResult = ERR_CARD_PROC_CANNOT_USE;
		return ErrRet( sResult );
	}
#endif

	return sResult;

}



/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCReadPurseAndTrans                                      *
*                                                                              *
*  DESCRIPTION:       ��ī�� ȯ�� �� �ŷ����� �б�                             *
*                     ȯ�������� �������� �ŷ������ �ǵ��ϴ� ���             *
*                                                                              *
*  INPUT PARAMETERS:  SC_EF_PURSE_INFO *pstPurseInfo                           *
*                     COMMON_XFER_DATA *pstTrans                               *
*                     SC_EF_PURSE *pstPurse                                    *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_SCREAD_TRANS                               *
*                     ERR_CARD_PROC_SCREAD_INSUFF_FARE                         *
*                     ERR_CARD_PROC_SCREAD_PURSE                               *
*                     ERR_CARD_PROC_SCREAD_VERIFY_PURSE                        *
*                     ERR_CARD_PROC_SCREAD                                     *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCReadPurseAndTrans( SC_EF_PURSE_INFO *pstPurseInfo,
	COMMON_XFER_DATA *pstTrans,
	SC_EF_PURSE *pstPurse,
	SC_EF_PURSE *pstPurseLoad )
{
	short sResult = 0;
	bool boolIsNewCard = FALSE;

	// ȯ������ �б�
	sResult = SCReadRecordTrans( 1, pstTrans );
	if ( sResult != SUCCESS )
	{
		// �ŷ������ ������
		// ù��� ī�������� �Ǻ��ϰ�
		// ù���ī���̸� ȯ����������ü�� �ʱ�ȭ�Ѵ�.
		if ( sResult == ERR_CARD_IF_SC_NO_REC ) // New Card
		{
			memset( ( byte * )pstTrans, 0x00, sizeof( COMMON_XFER_DATA ) );
			sResult = SUCCESS;
			boolIsNewCard = TRUE;
			DebugOut( "[SMH] ȯ������ ���� ù���ī����.\n" );
		}
		else
		{
			printf( "[��ī�� �б�] ȯ������ �б� ���� ErrCode[%x]\n", sResult );
			sResult = ERR_CARD_PROC_SCREAD_TRANS;
			return ErrRet( sResult ); // ERROR_TAG_AGAIN
		}
	}
	else
		DebugOut( "[SMH] ȯ������ 1���̶� �ִ�  ī����.\n" );

	// ������������ �б�
	sResult = SCReadRecordPurse( 1, pstPurse );
	if ( sResult != SUCCESS )
	{
		// �ŷ������ ������ �ĺ��� ����ó��
		// �ĺ��� �ƴϸ� ù��� ī�������� �Ǻ��Ͽ� �ܾ׺��� ����
		// �ŷ������ ���µ� ù����� �ƴϸ� �̻��� ī�� �Ǵ�
		if ( sResult == ERR_CARD_IF_SC_NO_REC ) // New Card
		{
			 //�ĺ�ī���̸� ����ó��
			if ( ( pstPurseInfo->bCardType & 0xF0 ) == 0x10 )
			{
				memset( ( byte * )pstPurse, 0x00, sizeof( SC_EF_PURSE ) );
				sResult = SUCCESS;
			}
			else
			{
				if ( boolIsNewCard == TRUE )
				{
					printf( "[��ī�� �б�] ���� �ȵ� ù��� ī�� \n" );
					sResult = ERR_CARD_PROC_INSUFFICIENT_BAL;
					return ErrRet( sResult );
				}
				else
				{
					printf( "[��ī�� �б�] ȯ������ ���� but " );
					printf( "���ڱⰩ ���� ���� �̻��� ī��!! \n" );
					sResult = ERR_CARD_PROC_SCREAD_PURSE;
					return ErrRet( sResult );
				}
			}
		}
		else
		{
			printf( "[��ī�� �б�] �������������б� ���� ErrCode[%x]\n",
				sResult );
			sResult = ERR_CARD_PROC_RETAG_CARD;
			return ErrRet( sResult );
		}
	}

	// �ܾ� ���� ����
	pstTrans->dwBal  = pstPurse->dwEpurseBal;

	// ������������ ��ȿ�� ����
	sResult = SCReadVerifyPurse( pstPurse, pstPurseLoad );
	if ( sResult != SUCCESS )
	{
		return ErrRet( sResult );
	}

	// ��������( 0x0A ) �ƴϸ� ���� �ŷ��ݾ��� �о �����ؾ� ��
	if ( pstTrans->boolSCIsNewTransFormat != TRUE )
	{
		// �����ŷ��� �о �����ŷ��ݾ� ����
		sResult = SCReadPayPrev( pstPurse, pstTrans );
		if ( sResult != SUCCESS )
		{
			//sResult = ERR_CARD_PROC_SCREAD;
			printf( "[��ī�� �б�] �����ŷ��ݾ��б� ���� ErrCode[%x]\n",
				sResult );
			return ErrRet( sResult );
		}
	}
	return ( sResult );

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCReadVerifyPurse                                        *
*                                                                              *
*  DESCRIPTION:       �������� �ŷ������ �ùٸ��� ��ϵ� ���¸� Ȯ��          *
*                                                                              *
*  INPUT PARAMETERS:  SC_EF_PURSE *pstPurse                                    *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_SCREAD_LOAD_ERR                            *
*                     ERR_CARD_PROC_SCREAD_VERIFY_PURSE                        *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCReadVerifyPurse( SC_EF_PURSE *pstPurse,
	SC_EF_PURSE *pstPurseLoad )
{
	short	sResult = 0;
	byte	bCount;				// Record #
	dword	dwCurBal;			// ���� �ܾ�
	dword	dwBefBal;			// Second Record�� �ܾ�
	dword	dwCurUsedBal;		// First Record�� �ŷ��ݾ�
	byte	bTRT;				// �ŷ�����
	dword	dwEPCounter;		// �ŷ���ȣ
	SC_EF_PURSE stBefPurse;

	bTRT 		= pstPurse->bTransType;
	dwCurBal		= pstPurse->dwEpurseBal;
	dwEPCounter 	= pstPurse->dwEpurseTransSeqNo;
	dwCurUsedBal	= pstPurse->dwTransAmt;

	bCount  = 2;

	// ���� �� ������ �����ŷ������� �ŷ���Ͽ� �߰��� ( 0338��� �϶�� �� )
	// ������ �����̸� ����
	if (  bTRT  == TRT_LOAD )
	{
		memcpy( pstPurseLoad, pstPurse, sizeof( SC_EF_PURSE ) );
	}

	sResult = SCReadRecordPurse( bCount, &stBefPurse );

	if ( sResult != SUCCESS )
	{
		if ( sResult == ERR_CARD_IF_SC_NO_REC )
		{
			if ( dwEPCounter < 4 )
			{
				sResult = SUCCESS;	// ����( �ŷ��� 0���� )
				return sResult;
			}
			// ���� �����ÿ� �ŷ���ȣ�� 1�� ���۵Ǹ�,
			// ���������ÿ��� ��õ��� �ŷ���ȣ�� ������.
			// �� ���� �����ÿ� ������ ���� 3ȸ �̻� �߻��� ī��� ���Ұ�.
			else
			{
				printf( "[��ī�� �б�] �������� �����ÿ� ������ ���� 3ȸ " );
				printf( "�̻� �߻��� ī��� ���Ұ�.\n" );
				return ERR_CARD_PROC_SCREAD_PURSE_LOAD;
			}
		}
		else
			{
				printf( "[��ī�� �б�] ������������ �б� ���� \n " );
				return ERR_CARD_PROC_SCREAD_PURSE; // Read Error
			}
	}

	dwBefBal	= stBefPurse.dwEpurseBal;

	switch ( bTRT )
	{
//		case 0x00:	//  Not used( JCOP�� �̻��ī���� ��� all 0���� ���� )
		case TRT_PURCHASE:		// �������� ����
		case TRT_UNLOAD:		// �������� ȯ��
		case TRT_CANCELLOAD:	// �������� �������
			if ( ( dwCurBal + dwCurUsedBal ) != dwBefBal )// �ܾ� ������ī��
				sResult = ErrRet( ERR_CARD_PROC_SCREAD_VERIFY_PURSE );
			break;

		case TRT_LOAD:			// �������� ����
		case TRT_AUTOLOAD:		// �ڵ�����
		case TRT_CANCEL:		// ������ ���� �ŷ� ���
			if ( ( dwCurBal - dwCurUsedBal ) != dwBefBal )// �ܾ� ������ī��
				sResult = ErrRet( ERR_CARD_PROC_SCREAD_VERIFY_PURSE );
			break;
		// �ĺ�ī��� ���ʿ� �ܾ��� Clear �ǹǷ� Check�ϸ� �ȵ�.
		case TRT_PURCHASE_POST:	// �ĺ�ī�� �ŷ�
		case TRT_CANCEL_POST:	// �ĺ�ī�� �ŷ� ���
			sResult = SUCCESS;
			break;
		default:
			sResult= ErrRet( ERR_CARD_PROC_SCREAD_VERIFY_PURSE );
			break;
	}

	if ( sResult == ERR_CARD_PROC_SCREAD_VERIFY_PURSE )
	{
		printf( "[��ī�� �б�] �ܾ� ������ : �ŷ����� [%02x]", bTRT );
		printf( " ���ܾ� = %ld  �����ŷ��ݾ� = %ld ", dwCurBal, dwCurUsedBal );
		printf( " �����ܾ� = %ld\n", dwBefBal );
	}
	else if ( (  bTRT  != TRT_LOAD ) && ( stBefPurse.bTransType == TRT_LOAD ) )
	{
		memcpy( pstPurseLoad, &stBefPurse, sizeof( SC_EF_PURSE ) );
	}

	return ( sResult );

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCReadPayPrev                                            *
*                                                                              *
*  DESCRIPTION:       ������������ ���� �ŷ��ݾ��� �����Ѵ�                    *
*                     ȯ�������� ���� �ŷ��ݾ��� ���� ���,                    *
*                     ������ ������ ���� �ŷ��� ����/ȯ�� ���� �ŷ��� ��쿣   *
*                     ���� �����̳��ö����� ��� ���ڵ��ȣ �����ϸ鼭 ����    *
*                                                                              *
*  INPUT PARAMETERS:  SC_EF_PURSE *pstPurse, COMMON_XFER_DATA *pstTrans        *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_SCREAD_PAY_PREV                            *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCReadPayPrev( SC_EF_PURSE *pstPurse, COMMON_XFER_DATA *pstTrans )
{
	byte 		bCount;
	bool		boolIsLoopEnd = FALSE;
	short 		sResult = 0;
	SC_EF_PURSE stPrevPurse;

	switch ( pstPurse->bTransType )
	{
		case 0x00:					// �ŷ���� ����
		case TRT_PURCHASE:			// �������� ����
		case TRT_PURCHASE_POST:		// �ĺ�ī�� �ŷ�
			pstTrans->dwFare = pstPurse->dwTransAmt;
			sResult = SUCCESS;
			return ( sResult );
	}

	for ( bCount = 0x02; boolIsLoopEnd == FALSE; bCount++ )
	{
		memset( ( byte * )&stPrevPurse,	 0, sizeof( SC_EF_PURSE ) );
		sResult = SCReadRecordPurse( bCount, &stPrevPurse );

		if ( sResult == SUCCESS )
		{
			switch ( stPrevPurse.bTransType )
			{
				case TRT_PURCHASE:		// �������� ����
				case TRT_PURCHASE_POST:	// �ĺ�ī�� �ŷ�
					boolIsLoopEnd = TRUE;
					pstTrans->dwFare =  stPrevPurse.dwTransAmt;
			}
		}
		else
		{
			if ( sResult == ERR_CARD_IF_SC_NO_REC )  // Record ����
			{
				pstTrans->dwFare =  0;
				boolIsLoopEnd = TRUE;
				sResult = SUCCESS;
			}
			else
			{
				boolIsLoopEnd = TRUE;
				sResult = ErrRet( ERR_CARD_PROC_SCREAD_PAY_PREV );
			}
		}
	}
	return ( sResult );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCPrepayCheckValidCard                                   *
*                                                                              *
*  DESCRIPTION:       �ż���ī�� ��ȿ��üũ                                    *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_SCREAD_PREPAY_CHECK                        *
*                     ERR_CARD_PROC_SCREAD_NL_CARD                             *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCPrepayCheckValidCard( TRANS_INFO *pstTransInfo,
	time_t tCardIssueDate )
{
	short sResult = 0;
	byte  bPLValue = 0;
	byte  abCardIssuerNo[7] = {0, };
	bool  boolResult;

	// �ż���ī�嵵 ����� üũ 0338���� ����
	// �ż���ī��� ������ī��� �޸� ����� �ڵ尡 ī���ȣ �� 6�ڸ��̹Ƿ�
	// �տ� '3'+ ī���ȣ �� 6�ڸ��� ����� �ڵ�� �����Ѵٰ� ��...
	// ����� ������ī��� 20�ڸ� �� �� 7�ڸ��� ����� �ڵ���
	abCardIssuerNo[0] = '3';
	memcpy( &abCardIssuerNo[1], pstTransInfo->abCardNo, 6 );

	DebugOutlnASC( "�ż��� ����� ID ==> ", abCardIssuerNo, 7 );

	// ��Ƽ��������� ���
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) )
	{
		// �����ID�� '3101'�� �ƴ� ��� '�̽���ī���Դϴ�' ���� ���
		if ( memcmp( abCardIssuerNo, "3101", 4 ) != 0 )
		{
			printf( "[SCPrepayCheckValidCard] ������ī�� ����� '3101' �ƴ� " );
			printf( "����\n" );
			return ERR_CARD_PROC_NOT_APPROV;
		}
	}

	boolResult = IsValidPrepayIssuer( abCardIssuerNo );
	if ( !boolResult )
	{
		printf( "[SCPrepayCheckValidCard] �ż���ī�� ����� üũ ����\n" );
		return ERR_CARD_PROC_NOT_APPROV;
	}

	boolResult = IsValidSCPrepayCardNo( pstTransInfo->abCardNo );
	if ( !boolResult )
	{
		printf( "[SCPrepayCheckValidCard] �ż���ī�� ī���ȣ ����\n" );
		return ERR_CARD_PROC_NOT_APPROV;
	}

	boolResult = IsValidSCPrepayIssueDate( pstTransInfo->abCardNo,
		tCardIssueDate );
	if ( !boolResult )
	{
		printf( "[SCPrepayCheckValidCard] �ż���ī�� �߱��� üũ ����\n" );
		return ERR_CARD_PROC_CANNOT_USE;
	}

#ifndef TEST_NOT_CHECK_BLPL
	// PL üũ
	sResult = SearchPLinBus( pstTransInfo->abCardNo, pstTransInfo->dwAliasNo,
		&bPLValue );

	if ( sResult == ERR_PL_FILE_OPEN_MASTER_AI )
	{
		printf( "[SCPostpayCheckValidCard] �ż���PL�� ������\n" );
		switch ( pstTransInfo->bCardUserType )
		{
			// ī���� ����� ���� �ڵ尡 ����� ��� ��̷� ��
			case USERTYPE_CHILD :
				pstTransInfo->bPLUserType = USERTYPE_CHILD;
				break;
			// ī�� ����� ���� �ڵ尡 û�ҳ�/�л��� ���� û�ҳ� �����
			case USERTYPE_STUDENT:
				pstTransInfo->bPLUserType = USERTYPE_YOUNG;
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
	if ( sResult != SUCCESS )
	{
		printf( "[SCPostpayCheckValidCard] ��Ÿ �� �� ���� ������ " );
		printf( "�ĺ�PLüũ ����\n" );
		return ErrRet( ERR_CARD_PROC_RETAG_CARD );
	}

	if ( bPLValue == 0 )
	{
		printf( "[SCPrepayCheckValidCard] NL�� ī���Դϴ�\n" );
		sResult = ERR_CARD_PROC_NOT_APPROV;
		return ErrRet( sResult );
	}
	else if ( bPLValue > 3 )
	{
		printf( "[SCPrepayCheckValidCard] �ż���ī�� PL üũ ����� ==> [%x] ",
			bPLValue );
		printf( "PL ����� �̻� ��\n" );
		sResult = ERR_CARD_PROC_SCREAD_PREPAY_CHECK;
		return ErrRet( sResult );
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
		case USERTYPE_CHILD :
			switch ( bPLValue )
			{
				case 1 : // P/L ��Ʈ���� û�ҳ� ���
					pstTransInfo->bPLUserType = USERTYPE_YOUNG;
					break;
				case 2 : // P/L ��Ʈ���� ��� ���
					pstTransInfo->bPLUserType = USERTYPE_CHILD;
					break;
				case 3 : // P/L ��Ʈ���� �Ϲ� ���
					pstTransInfo->bPLUserType = USERTYPE_ADULT;
					break;
			}
			break;
		// ī�� ����� ���� �ڵ尡 û�ҳ�/�л��� ����
		// PLüũ ������� �Ϲ��ΰ��� �Ϲݿ���̰� �������� û�ҳ� �����
		case USERTYPE_STUDENT:
		case USERTYPE_YOUNG:
			if ( bPLValue == 3 )
				pstTransInfo->bPLUserType = USERTYPE_ADULT;
			else
				pstTransInfo->bPLUserType = USERTYPE_YOUNG;
			break;
		case USERTYPE_TEST :
			pstTransInfo->bPLUserType = USERTYPE_TEST;
			break;
		default :
			pstTransInfo->bPLUserType = USERTYPE_ADULT;
			break;
	}
#endif

	return ( sResult );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCPostpayCheckValidCard                                  *
*                                                                              *
*  DESCRIPTION:       ���ĺ�ī�� ��ȿ��üũ                                    *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo, byte * abCardExpriryDate       *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_CARD_PROC_SCREAD_POSTPAY_CHECK                       *
*                     ERR_CARD_PROC_SCREAD_NL_CARD                             *
*                     ERR_CARD_PROC_SCREAD                                     *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCPostpayCheckValidCard( TRANS_INFO *pstTransInfo,
	time_t tCardExpriryDate )
{
	short sResult = 0;
	byte abTempExpiryDate[8];
	byte abExpiryDate[6];
	byte bPLValue;
	time_t tNowDate;
	bool boolValidCardNo;


	// ī���ȣ ��ȿ�� üũ
	// ����� ������ �˼� ��� �׳� ����
	if ( pstTransInfo->abCardNo[15] == 'F' )
		boolValidCardNo = IsValidAmexCardNo( pstTransInfo->abCardNo );
	else
		boolValidCardNo = IsValidISOCardNo( pstTransInfo->abCardNo );

	if ( boolValidCardNo != TRUE )
	{
		DebugOut( "[SCPostpayCheckValidCard] ī���ȣ ����\n" );
		return ERR_CARD_PROC_MIF_POSTPAY_INVALID_CARD_NO;

	}

	TimeT2ASCDate( tCardExpriryDate, abTempExpiryDate );
	memcpy( abExpiryDate, abTempExpiryDate, sizeof( abExpiryDate ) );
	tNowDate = pstTransInfo->stNewXferInfo.tEntExtDtime;

#ifndef TEST_NOT_CHECK_EXPIRY_DATE
	// ī�� ��ȿ�Ⱓ üũ
	if ( !IsValidExpiryDate( abExpiryDate, tNowDate ) )
	{
		DebugOut( "[SCPostpayCheckValidCard] ��ȿ�Ⱓ ����\n" );
		return ErrRet( ERR_CARD_PROC_MIF_POSTPAY_EXPIRE );
	}
#endif

	// ����� üũ
	if ( !IsValidPostpayIssuer( pstTransInfo->abCardNo ) )
	{
		DebugOut( "[SCPostpayCheckValidCard] prefix ����\n" );
		return ErrRet( ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER );
	}

	// ����� ��ȿ�Ⱓ üũ
	if ( !IsValidIssuerValidPeriod( pstTransInfo->abCardNo, abExpiryDate ) )
	{
		DebugOut( "[SCPostpayCheckValidCard] �ĺҹ������ȿ�Ⱓ üũ ����\n" );
		return ErrRet( ERR_CARD_PROC_MIF_POSTPAY_INVALID_ISSUER_VALID_PERIOD );
	}

#ifndef TEST_NOT_CHECK_BLPL
	// PL üũ
	sResult = SearchPLinBus( pstTransInfo->abCardNo, pstTransInfo->dwAliasNo,
		&bPLValue );

	// ��� ���нÿ� ���ĺ�ī��� �̽��� ó��
	if ( sResult == ERR_PL_FILE_OPEN_MASTER_POSTPAY_PL )
	{
		printf( "[SCPostpayCheckValidCard] �ĺ�PL�� ������\n" );
		return ErrRet( ERR_CARD_PROC_NOT_APPROV );
	}
	if ( sResult != SUCCESS )
	{
		printf( "[SCPostpayCheckValidCard] ��Ÿ �� �� ���� ������ " );
		printf( "�ĺ�PLüũ ����\n" );
		return ErrRet( ERR_CARD_PROC_RETAG_CARD );
	}

	if ( bPLValue == 0 ) // NL ī�� ó��
	{
		DebugOut( "[SCPostpayCheckValidCard] NL�� ī���Դϴ�\n" );
		sResult = ErrRet( ERR_CARD_PROC_NOT_APPROV );
	}
	else if ( bPLValue > 1 ) // PLüũ���� �̻��ϸ� ���� ó��
	{
		DebugOut( "[SMH]���ĺ�ī�� PL üũ ����� ==> [%x]", bPLValue );
		DebugOut( " PL ����� �̻� ��\n" );
		sResult = ErrRet( ERR_CARD_PROC_SCREAD_POSTPAY_CHECK );
	}
	else
	{
		pstTransInfo->bPLUserType = USERTYPE_ADULT;

	}
#endif

	return ( sResult );
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCComTransInfo                                           *
*                                                                              *
*  DESCRIPTION:       ��ī�� ī����������ü ����                               *
*                                                                              *
*  INPUT PARAMETERS:  SC_EF_PURSE_INFO *pstPurseInfo                           *
*                     SC_EF_PURSE *pstPurse                                    *
*                     COMMON_XFER_DATA *pstTrans                               *
*                     TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void SCComTransInfo( SC_EF_PURSE_INFO *pstPurseInfo,
	SC_EF_PURSE *pstPurse,
	SC_EF_PURSE *pstPurseLoad,
	COMMON_XFER_DATA *pstCommonXferData,
	TRANS_INFO *pstTransInfo )
{
	pstTransInfo->bSCAlgoriType			= pstPurseInfo->bAlgoriID;
	pstTransInfo->bSCTransKeyVer		= pstPurseInfo->bEpurseKeysetVer;
	pstTransInfo->bSCEpurseIssuerID		= pstPurseInfo->bEpurseIssuerID;
	memcpy( pstTransInfo->abSCEpurseID, pstPurseInfo->abEpurseID,
		sizeof( pstTransInfo->abSCEpurseID ) );

	pstTransInfo->dwSCTransCnt 			= pstPurse->dwEpurseTransSeqNo;

	// ȯ�� ���� ���� ����
	memcpy( &pstTransInfo->stPrevXferInfo, pstCommonXferData,
			sizeof( COMMON_XFER_DATA ) );

	// ���� ���� ���� ������ ����
	if ( pstPurseLoad->bTransType == TRT_LOAD )
	{
		pstTransInfo->dwBalAfterCharge = pstPurseLoad->dwEpurseBal;
		pstTransInfo->dwChargeTransCnt = pstPurseLoad->dwEpurseTransSeqNo;
		pstTransInfo->dwChargeAmt = pstPurseLoad->dwTransAmt;
		memcpy( pstTransInfo->abLSAMID, pstPurseLoad->abSAMID,
			sizeof( pstTransInfo->abLSAMID ) );
		pstTransInfo->dwLSAMTransCnt = pstPurseLoad->dwSAMTransSeqNo;
		pstTransInfo->bChargeTransType = pstPurseLoad->bTransType;
	}
	else
	{
		pstTransInfo->dwBalAfterCharge = 0UL;
		pstTransInfo->dwChargeTransCnt = 0UL;
		pstTransInfo->dwChargeAmt	   = 0UL;
		memset( pstTransInfo->abLSAMID, 0, sizeof( pstTransInfo->abLSAMID ) );
		pstTransInfo->dwLSAMTransCnt   = 0UL;
		pstTransInfo->bChargeTransType = 0x00;
	}
	pstTransInfo->sErrCode = 0x00;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCWrite                                                  *
*                                                                              *
*  DESCRIPTION:       ��ī�� WRITE                                             *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ???                                                      *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SCWrite( TRANS_INFO *pstTransInfo )
{
	short	sResult = SUCCESS;	// �Լ� ���ϰ�
	byte	bWriteType = 0;		// ������ ���� ������ �̿��Ͽ�
								// �ŷ�/��õ�/��ŷ�/��Ұŷ�/ȯ�´ٽþ��⺯��

	SC_EF_PURSE_INFO	stPurseInfo;
	SC_EF_PURSE			stReadPurse;
	COMMON_XFER_DATA	stCommonXferData;

	memset( &stPurseInfo, 0x00, sizeof( SC_EF_PURSE_INFO ) );
	memset( &stCommonXferData, 0x00, sizeof( COMMON_XFER_DATA ) );

	DebugOut( "[smh] SCWrite ����------------\n" );

	// ������ ���� ������ �̿��Ͽ� �ŷ�/��õ�/��ŷ�/��Ұŷ�/ȯ�´ٽþ��� ����
	switch ( pstTransInfo->sErrCode )
	{

		// ��ī�� ���� �ŷ�
		case 0x00 :
		// ��ī�� �ŷ� Initialize ����
		case ERR_PAY_LIB_SC_PREPAY_PURCHASE_INIT_CARD :
		case ERR_PAY_LIB_SC_PREPAY_PURCHASE_INIT_SAM :
		case ERR_PAY_LIB_SC_POSTPAY_PURCHASE_INIT_CARD :
		case ERR_PAY_LIB_SC_POSTPAY_PURCHASE_INIT_SAM :
			bWriteType = NORMAL;
			break;
			
		// ��ī�� ī��ŷ� ���� �� purchase ������ �ܾ� Ȯ��
		case ERR_PAY_LIB_SC_PREPAY_PURCHASE_CARD:
		case ERR_PAY_LIB_SC_POSTPAY_PURCHASE_GENERATE_TC:
		case ERR_PAY_LIB_VERIFY_PURSE:
		case ERR_PAY_LIB_SC_VERIFY_PURSE_TRANS_TYPE:
		case ERR_PAY_LIB_SC_VERIFY_PURSE_SAM_ID:
			// ī���� �ܾ��� �о
			sResult = SCReadRecordPurse( 1, &stReadPurse );
			if ( sResult != SUCCESS )
			{
				return ERR_CARD_PROC_RETAG_CARD;
			}
			// ���� �ܾװ� ī�� �ܾ��� ������ -> ���Ұŷ�
			if ( stReadPurse.dwEpurseBal == pstTransInfo->stPrevXferInfo.dwBal )
				bWriteType = NORMAL;
			// ���� �ܾװ� ī�� �ܾ��� �ٸ��� -> ��ŷ�
			else
			{
				if ( memcmp( pstTransInfo->abPSAMID,
					gstMyTermInfo.abPSAMID, 16 ) != 0 )
					bWriteType = ABNORMAL;
				else
					bWriteType = REPURCHASE;
			}
			break;
		// �ż��� ��ŷ� ����
		case ERR_PAY_LIB_SC_PREPAY_REPURCHASE_INIT_CARD:
		case ERR_PAY_LIB_SC_PREPAY_REPURCHASE_INIT_SAM:
		case ERR_PAY_LIB_SC_PREPAY_REPURCHASE_CARD:
		case ERR_PAY_LIB_SC_PREPAY_REPURCHASE_CREDIT_SAM:
		case ERR_PAY_LIB_SC_PREPAY_PURCHASE_CREDIT_SAM:
			if ( memcmp( pstTransInfo->abPSAMID,
				gstMyTermInfo.abPSAMID, 16 ) != 0 )
				bWriteType = ABNORMAL;
			else
				bWriteType = REPURCHASE;
			break;
		// ���ĺ� ��ŷ� ����
		case ERR_PAY_LIB_SC_POSTPAY_CANCEL_INIT_CARD:
		case ERR_PAY_LIB_SC_POSTPAY_CANCEL_INIT_SAM:
		case ERR_PAY_LIB_SC_POSTPAY_CANCEL_GENERATE_TC:
		case ERR_PAY_LIB_SC_POSTPAY_CANCEL_VERIFY_TC:
		// ���ĺ� SAM �ŷ� ����
		case ERR_PAY_LIB_SC_POSTPAY_PURCHASE_VERIFY_TC:
			if ( memcmp( pstTransInfo->abPSAMID,
				gstMyTermInfo.abPSAMID, 16 ) != 0 )
				bWriteType = ABNORMAL;
			else
				bWriteType = POSTPAY_CANCEL;
			break;
		// �ż��� ��� ����
		case ERR_PAY_LIB_SC_PREPAY_CANCEL_INIT_CARD:
		case ERR_PAY_LIB_SC_PREPAY_CANCEL_INIT_SAM:
			if ( memcmp( pstTransInfo->abPSAMID,
				gstMyTermInfo.abPSAMID, 16 ) != 0 )
				bWriteType = ABNORMAL;
			else
				bWriteType = PREPAY_CANCEL;
			break;
		// ȯ������ ���� ����
		case ERR_PAY_LIB_GENERATE_MAC:
		case ERR_PAY_LIB_APPEND_TRANS_RECORD:
		case ERR_PAY_LIB_VERIFY_TRANS:
			bWriteType = REWRITE;
			break;
		case ERR_SAM_IF_PSAM_NO_SIGN3:
			bWriteType = ABNORMAL;			
			break;			
		default :
			bWriteType = ABNORMAL;	
			break;
	}

	switch ( bWriteType )
	{
		case NORMAL :
			sResult = SCNormalPayment( pstTransInfo );
			break;
		case REPURCHASE :
		case POSTPAY_CANCEL :
		case PREPAY_CANCEL :
			sResult = SCRePayment( bWriteType, pstTransInfo );
			break;
		case ABNORMAL:
		case REWRITE :
			// ��Ƽ��������� ��� ȯ�������� WRITE���� ����
			if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == FALSE )
			{
				// ȯ������ ����ü ����
				SCDeComTransInfo( pstTransInfo, &stPurseInfo,
					&stCommonXferData );

				// ȯ������ APPEND
				sResult = SCAppendXferInfo( TRANS_REWRITE, &stPurseInfo,
					&stCommonXferData );
			}
			break;
	}

	DebugOut( "[smh] SCWrite ��------------\n" );

	// ��ī�� ����ó�� ����� ���� ����Ʈ�� �߰�
	if ( sResult != SUCCESS )
	{
		AddCardErrLog( sResult, pstTransInfo );
		return ErrRet( ERR_CARD_PROC_RETAG_CARD );
	}

	// ��ó���� ī���� ��� ����LOG�κ��� �ش� ī�������� ���� /////////////////
	else if ( pstTransInfo->sErrCode != SUCCESS )
		DeleteCardErrLog( pstTransInfo->abCardNo );

	return ( sResult );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCNormalPayment                                          *
*                                                                              *
*  DESCRIPTION:       ��ī�� ���Ұŷ�( �������� ��� )                         *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ???                                                      *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCNormalPayment( TRANS_INFO *pstTransInfo )
{
	short sResult = 0;

	SC_EF_PURSE_INFO	stPurseInfo;
	COMMON_XFER_DATA	stCommonXferData;
	PSAM_RES_TRANS		stPSAMResult;
	dword dwTransAmt;

	memset( ( byte * )&stPurseInfo, 		0, sizeof( SC_EF_PURSE_INFO ) );
	memset( ( byte * )&stCommonXferData,	0, sizeof( COMMON_XFER_DATA ) );
	memset( ( byte * )&stPSAMResult,	 	0, sizeof( PSAM_RES_TRANS ) );

	dwTransAmt = pstTransInfo->stNewXferInfo.dwFare;
	// �ż���ī�� ���Ұŷ�
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY )
	{

		sResult = SCPrepayPurchase( dwTransAmt, &stPSAMResult );
		// SAM �ŷ� ���� ����
		SCSavePSAMResult( sResult, &stPSAMResult, pstTransInfo );
		if ( sResult != SUCCESS )
		{
			DebugOut( "SCPrepayPurchase ���� : %x\n", sResult );
			return ( sResult );
		}
	}
	// ���ĺ�ī�� ���Ұŷ�
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_SC_POSTPAY )
	{
		sResult = SCPostpayPurchase( pstTransInfo->boolIsChangeMonth,
			dwTransAmt, &stPSAMResult );
		// SAM �ŷ� ���� ����
		SCSavePSAMResult( sResult, &stPSAMResult, pstTransInfo );
		if ( sResult != SUCCESS )
		{
			DebugOut( "SCPostpayPurchase ���� : %x\n", sResult );
			return ErrRet( sResult );
		}
	}

#ifdef TEST_WRITE_SLEEP
	printf( "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n" );
	sleep( 2 );
#endif

	// ��Ƽ��������� ��� ȯ�������� WRITE���� ����
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == FALSE )
	{
		// ȯ������ ����ü ����
		SCDeComTransInfo( pstTransInfo, &stPurseInfo, &stCommonXferData );

		// ȯ������ APPEND
		sResult = SCAppendXferInfo( TRANS_NORMAL, &stPurseInfo,
			&stCommonXferData );
		if ( sResult != SUCCESS )
		{
			printf( "[SCNormalPayment] SCAppendXferInfo() ����\n" );
			return ErrRet( sResult );
		}
	}
	
	return ( sResult );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCRePayment                                              *
*                                                                              *
*  DESCRIPTION:       ��ī�� �����Ұŷ�                                        *
*                                                                              *
*  INPUT PARAMETERS:  byte bWriteType :REPURCHASE/PREPAY_CANCEL/POSATPAY_CANCEL*
*						TRANS_INFO *pstTransInfo                               *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ???                                                      *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short SCRePayment( byte bWriteType, TRANS_INFO *pstTransInfo )
{
	short sResult = 0;
	SC_EF_PURSE_INFO	stPurseInfo;
	COMMON_XFER_DATA	stCommonXferData;
	PSAM_RES_TRANS		stPSAMResult;
	dword dwTransAmt = 0;

	memset( ( byte * )&stPurseInfo, 		0, sizeof( SC_EF_PURSE_INFO ) );
	memset( ( byte * )&stCommonXferData,	0, sizeof( COMMON_XFER_DATA ) );
	memset( ( byte * )&stPSAMResult,	 	0, sizeof( PSAM_RES_TRANS ) );

	dwTransAmt = pstTransInfo->stNewXferInfo.dwFare;

	// �ż���ī�� ������ŷ�
	if ( bWriteType == REPURCHASE &&
		 pstTransInfo->bCardType == TRANS_CARDTYPE_SC_PREPAY )
	{
		sResult = SCPrepayRepurchase( dwTransAmt, &stPSAMResult );
		// SAM �ŷ� ���� ����
		SCSavePSAMResult( sResult, &stPSAMResult, pstTransInfo );
		if ( ( sResult != SUCCESS ) && ( sResult !=ERR_SAM_IF_PSAM_NO_SIGN3 ) )
		{
			DebugOut( "SCPrepayRepurchase ���� : %x\n", sResult );
			return ErrRet( sResult );
		}
		else 
			sResult = SUCCESS;			
	}
	// ���ĺ�ī�� ������ŷ�( ��� -> �ŷ� )
	else if ( bWriteType == POSTPAY_CANCEL ||
			 ( bWriteType == REPURCHASE &&
			  pstTransInfo->bCardType == TRANS_CARDTYPE_SC_POSTPAY ) )
	{
		sResult = SCPostpayCancel( &stPSAMResult );
		if ( sResult != SUCCESS )
		{
			DebugOut( "SCPostpayCancel ���� : %x\n", sResult );
			return ErrRet( sResult );
		}
		sResult = SCPostpayPurchase( pstTransInfo->boolIsChangeMonth,
			dwTransAmt, &stPSAMResult );
		// SAM �ŷ� ���� ����
		SCSavePSAMResult( sResult, &stPSAMResult, pstTransInfo );
		if ( sResult != SUCCESS )
		{
			DebugOut( "SCPostpayPurchase ���� : %x\n", sResult );
			return ErrRet( sResult );
		}
	}
	// �ż���ī�� ��� �� �ŷ�
	else if ( bWriteType == PREPAY_CANCEL )
	{
		sResult = SCPrepayCancel( dwTransAmt, &stPSAMResult );
		// SAM �ŷ� ���� ����
		SCSavePSAMResult( sResult, &stPSAMResult, pstTransInfo );
		if ( sResult != SUCCESS )
		{
			DebugOut( "SCPrepayCancel ���� : %x\n", sResult );
			return ErrRet( sResult );
		}
		sResult = SCPrepayPurchase( dwTransAmt, &stPSAMResult );
		// SAM �ŷ� ���� ����
		SCSavePSAMResult( sResult, &stPSAMResult, pstTransInfo );
		if ( sResult != SUCCESS )
		{
			DebugOut( "SCPrepayPurchase ���� : %x\n", sResult );
			return ErrRet( sResult );
		}
	}

	// ��Ƽ��������� ��� ȯ�������� WRITE���� ����
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == FALSE )
	{
		// ȯ������ ����ü ����
		SCDeComTransInfo( pstTransInfo, &stPurseInfo, &stCommonXferData );

		// ȯ������ APPEND
		sResult = SCAppendXferInfo( TRANS_NORMAL, &stPurseInfo,
			&stCommonXferData );
		if ( sResult != SUCCESS )
		{
			printf( "[SCRePayment] SCAppendXferInfo() ����\n" );
			return ErrRet( sResult );
		}
	}

	return ( sResult );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCDeComTransInfo                                         *
*                                                                              *
*  DESCRIPTION:       ī����������ü�� ��ī�� ����ü�� ������                  *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                     SC_EF_PURSE_INFO *pstPurseInfo                           *
*                     COMMON_XFER_DATA *pstTrans                               *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void SCDeComTransInfo( TRANS_INFO *pstTransInfo,
	SC_EF_PURSE_INFO *pstPurseInfo,
	COMMON_XFER_DATA *pstCommonXferData )
{
	// ( 1 ) �������� ������ ���� ��ȯ
	pstPurseInfo->bEpurseIssuerID = pstTransInfo->bSCEpurseIssuerID;
	memcpy( pstPurseInfo->abEpurseID, pstTransInfo->abSCEpurseID,
		sizeof( pstPurseInfo->abEpurseID ) );

	// ( 2 ) ȯ�� ���� ���� ��ȯ
	memcpy( pstCommonXferData, &pstTransInfo->stNewXferInfo,
			sizeof( COMMON_XFER_DATA ) );

	// ���� ȯ���������� ������� �ʴ� ����Ʈ��
	pstCommonXferData->wTotalAccEntCnt = 0;
	pstCommonXferData->dwTotalAccUseAmt = 0;

}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SCSavePSAMResult                                         *
*                                                                              *
*  DESCRIPTION:       ��ī�� PSAM������ ī����������ü�� ����                  *
*                                                                              *
*  INPUT PARAMETERS:  PSAM_RES_TRANS *pstPSAMResult                            *
*                     TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-08-24 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void SCSavePSAMResult( short sResult,
	PSAM_RES_TRANS *pstPSAMResult,
	TRANS_INFO *pstTransInfo )
{

	if ( sResult == SUCCESS )
	{
		pstTransInfo->bSCAlgoriType			= pstPSAMResult->bAlgID;
		pstTransInfo->bSCTransKeyVer		= pstPSAMResult->bTransKeyVer;
		pstTransInfo->bSCEpurseIssuerID		= pstPSAMResult->bCenterID;
		memcpy( pstTransInfo->abSCEpurseID, pstPSAMResult->abEpurseID,
			sizeof( pstTransInfo->abSCEpurseID ) );
	}
	pstTransInfo->bSCTransType 			= pstPSAMResult->bTransType;
	pstTransInfo->dwSCTransCnt 			= pstPSAMResult->dwTransCnt;

	memcpy( pstTransInfo->abPSAMID, pstPSAMResult->abSAMID,
		sizeof( pstTransInfo->abPSAMID ) );
	pstTransInfo->dwPSAMTransCnt 		= pstPSAMResult->dwSAMTransCnt;

	pstTransInfo->dwPSAMTotalTransCnt 	= pstPSAMResult->dwSAMTotalTransCnt;
	pstTransInfo->wPSAMIndvdTransCnt 	= pstPSAMResult->wSAMIndvdTransCnt;
	pstTransInfo->dwPSAMAccTransAmt 	= pstPSAMResult->dwSAMAccTransAmt;
	memcpy( pstTransInfo->abPSAMSign, pstPSAMResult->abSAMSign,
		sizeof( pstTransInfo->abPSAMSign ) );
}

