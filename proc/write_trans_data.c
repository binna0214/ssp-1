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
*  PROGRAM ID :       write_trans_data.c                                       *
*                                                                              *
*  DESCRIPTION:       �ŷ����� ����� ���� �Լ����� �����Ѵ�.                  *
*                                                                              *
*  ENTRY POINT:       short WriteTransHeader( void );                          *
*                     void WriteTransData( TRANS_INFO *pstTransInfo );         *
*                     short WriteTransDataForCommProcess( byte *abTransTD );   *
*                     void WriteCashTransData( byte bUserType );               *
*                     short WriteTransTail( void );                            *
*                     short UpdateTransDCSRecvDtime( byte *abTransFileName );  *
*                     short RemakeTrans( void );                               *
*                                                                              *
*  INPUT FILES:       control.trn - ���������������                           *
*                     YYYYMMDDhhmmss.trn - �����ŷ���������                    *
*                     YYYYMMDDhhmmss.10 - �����ӽô������                     *
*                     YYYYMMDDhhmmss.20 - �����ӽô������                     *
*                     YYYYMMDDhhmmss.30 - �����ӽô������                     *
*                                                                              *
*  OUTPUT FILES:      control.trn - ���������������                           *
*                     YYYYMMDDhhmmss.trn - �����ŷ���������                    *
*                     YYYYMMDDhhmmss.trn - �������ŷ���������                *
*                     alight_term_td.tmp - �����ŷ���������                    *
*                     aboard_term_td.bak - ��������ŷ���������                *
*                     alight_term_td.bak - ��������ŷ���������                *
*                     temptd.dat - �ӽô������                                *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2006/01/11 F/W Dev Team Boohyeon Jeon   Initial Release                      *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "main_process_busTerm.h"
#include "file_mgt.h"
#include "card_proc.h"
#include "card_proc_util.h"
#include "trans_proc.h"

#include "write_trans_data.h"
#include "reconcile_file_mgt.h"

#define LENGTH_OF_TRANS_HEADER			"185"		// �ŷ������������ ���ڿ�
#define LENGTH_OF_TRANS_DATA			"202"		// �ŷ����������ͱ��� ���ڿ�
#define LENGTH_OF_TRANS_TAIL			"161"		// �ŷ��������ϱ��� ���ڿ�

#define RECORD_TYPE_TRANS_HEADER		'H'			// �ŷ������������ ����
#define RECORD_TYPE_TRANS_DATA			'D'			// �ŷ��������������� ����
#define RECORD_TYPE_TRANS_TAIL			'T'			// �ŷ������������� ����

#define PENALTY_TYPE_NO_TAG_IN_EXT		"01"		// ���Ƽ���� - �������±�

#define CHECK_SAM_SIGN_FAIL				'1'			// SAM SIGN üũ ����
#define CHECK_SAM_SIGN_PASS				'2'			// SAM SIGN ��üũ

#define SUB_TRANS_DATA_NOT_YET_SEND		'0'			// �����ŷ����� ������
#define SUB_TRANS_DATA_SEND_COMPELETED	'1'			// �����ŷ����� ���ۿϷ�

/*******************************************************************************
*  Declaration of Structure                                                    *
*******************************************************************************/
/*
 * �ŷ����� ��� ����ü
 */
typedef struct {
	byte	abTHRecLen[3];					// Recode ����
	byte	bTHRecType;						// Record ����
	byte	abTranspBizrID[9];				// �������� ID
	byte	abBizOfficeID[2];				// ������ ID
	byte	abRouteID[8];					// ���� �뼱 ID
	byte	abTranspMethodCode[3];			// ��������ڵ�
	byte	abVehicleID[9];					// ���� ID
	byte	abTermID[9];					// �ܸ��� ID
	byte	abDriverID[6];					// ������ ID
	byte	abDCSRecvDtime[14];				// ����ý��� �����Ͻ�
	byte	abStartStationID[7];			// ���������� ID
	byte	abStartDtime[14];				// ���� ���� �Ͻ�
	byte	abTermUseSeqNo[4];				// �ܸ����������Ϸù�ȣ
	byte	abFileName[90];					// File Name
	byte	abRecMAC[4];					// Record Mac
	byte	abCRLF[2];						// ���� ����
}__attribute__( ( packed ) ) TRANS_TH;

/*
 * �ŷ����� ������ ����ü
 */
typedef struct {
	byte	abTDRecLen[3];					// TD Record ����
	byte	bTDRecType;						// TD Record ����
	byte	bCardType;						// ī������
	byte	bEntExtType;					// ����������
	byte	abCardNo[10];					// ī���ȣ/��ȸ��ID
	dword	dwAliasNo;						// alias��ȣ
	byte	abEntExtDtime[7];				// �������Ͻ�
	byte	bUserType;						// ���������
	byte	abDisExtraTypeID1[6];			// �°�1 ������������ID
	word	wUserCnt1;						// �°���1
	byte	abDisExtraTypeID2[6];			// �°�2 ������������ID
	word	wUserCnt2;						// �°���2
	byte	abDisExtraTypeID3[6];			// �°�3 ������������ID
	word	wUserCnt3;						// �°���3
	byte	abPenaltyType[2];				// �г�Ƽ����
	dword	dwPenaltyFare;					// ���Ƽ���
	byte	bSCAlgoriType;					// �˰�������
	byte	bSCTransType;					// �ŷ�����
	byte	bSCTransKeyVer;					// �����ŷ�����Ű����
	byte	bSCEpurseIssuerID;				// ����ȭ���ID
	byte	abSCEpurseID[8];				// ��������ID
	dword	dwSCTransCnt;					// ī��ŷ��Ǽ�
	dword	dwBal;							// ī���ܾ�
	dword	dwFare;							// ���
	byte	abPSAMID[8];					// SAM ID
	dword	dwPSAMTransCnt;					// SAM�ŷ�ī����
	dword	dwPSAMTotalTransCnt;			// SAM�Ѿװŷ�����ī����
	word	wPSAMIndvdTransCnt;				// SAM�����ŷ������Ǽ�
	dword	dwPSAMAccTransAmt;				// SAM�����ŷ��Ѿ�
	byte	abPSAMSign[4];					// SAM����
	byte	abXferSeqNo[3];					// ȯ���Ϸù�ȣ
	dword	dwPrevMaxBaseFare1;				// �°�1 �����ŷ� ���� �ִ�⺻���
	dword	dwPrevMaxBaseFare2;				// �°�2 �����ŷ� ���� �ִ�⺻���
	byte	abPrevMaxBaseFare3[4];			// �°�3 �����ŷ� ���� �ִ�⺻���
	dword	dwTotalBaseFareInXfer;			// ȯ�³��̿���ܱ⺻�������
	word	wXferCnt;						// ȯ�´���Ƚ��
	dword	dwDist;							// ���Ÿ�
	dword	dwAccDistInXfer;				// ȯ�³������̵��Ÿ�
	dword	dwAccUseAmtInXfer;				// ȯ�³������̿�ݾ�
	dword	dwTotalAccAmt;					// �Ѵ������ݾ�
	byte	abStationID[4];					// ������ID
	dword	dwPrevPenaltyFare;				// �������Ƽ���
	dword	dwPrevEntFare;					// �����ý����ݾ�
	byte	abPrevStationID[4];				// ����������ID
	byte	abPrevTranspMethodCode[2];		// ���������������
	byte	abPrevEntExtDtime[7];			// �����������Ͻ�
	dword	dwBalAfterCharge;				// ������ī���ܾ�
	dword	dwChargeTransCnt;				// ������ī��ŷ��Ǽ�
	dword	dwChargeAmt;					// �����ݾ�
	byte	abLSAMID[8];					// ������SAM ID
	dword	dwLSAMTransCnt;					// ������SAM�ŷ��Ϸù�ȣ
	byte	bChargeTransType;				// �����ŷ�����
	byte	abRecMAC[4];					// Record Mac
	byte	abVerifySignVal;				// SIGN���������
	byte	abCRLF[2];						// ���๮��
}__attribute__( ( packed ) ) TRANS_TD;

/*
 * �ŷ����� ���� ����ü
 */
typedef struct {
	byte	abTTRecLen[3];					// Record ����
	byte	bTTRecType;						// Record����
	byte	abFileName[90];					// File Name
	byte	abTotalCnt[9];					// �ѰǼ�
	byte	abTotalAmt[10];					// �ѱݾ�
	byte	abEndDtime[14];					// ���� ���� �Ͻ�
	byte	abReturnStationID[7];			// ȸ���� ID
	byte	abReturnDtime[14];				// ȸ���ð�
	byte	abEndStationID[7];				// ���������� ID
	byte	abRecMAC[4];					// Record MAC
	byte	abCRLF[2];						// ���๮��
}__attribute__( ( packed ) ) TRANS_TT;

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short WriteTransDataInMainTerm( TRANS_TD *pstTransTD );
static short WriteTransDataInSubTerm( TRANS_TD *pstTransTD );
static void WriteRemakeTransDataInSubTerm( TRANS_TD *pstTransTD );
static short WriteBackupTransData( TRANS_TD *pstTransTD );
static void CreateTransFileName( byte *abTransFileName,
	time_t *ptStartDriveDtime );
static void CreateEndedTransFileName( byte *abEndedTransFileName,
	time_t *ptEndDriveDtime );
static short AppendTransFileToReconcileFile( time_t tNowDtime );
static short CopyTransHeaderAndTransData( byte *abSrcTransFileName,
	byte *abDesTransFileName, dword *pdwTotalCnt, dword *pdwTotalAmt );
static short RemakeTransData( byte *abMainTransFileName,
	byte *abSubTempRemakeFileName, byte *abTempRemakeFileName,
	dword *pdwTotalCnt, dword *pdwTotalAmt );
static short CopyTransTail( byte *abSrcTransFileName,
	byte *abDesTransFileName, dword dwTotalCnt, dword dwTotalAmt );
static word GetTermAggrSeqNo( void );
static word GetBackupTransDataSeqNo( void );
static word GetCashTransSeqNo( void );
static void SetControlTrans(CONTROL_TRANS *pstControlTrans,
	byte *abTransFileName);
static void SetTransHeader( TRANS_TH *pstTransTH, time_t tStartDriveDtime );
static void SetTransData( TRANS_INFO *pstTransInfo, TRANS_TD *pstTransTD );
static void SetCashTransData( TRANS_TD *pstTransTD, byte bUserType );
static void SetTransTail( TRANS_TT *pstTransTT, dword dwTotalCnt,
	dword dwTotalAmt, time_t tEndDriveDtime );
static short FileRenameTransFile( byte *abOldTransFileName,
	byte *abNewTransFileName );
static short FileReadControlTrans( CONTROL_TRANS *pstControlTrans );
static short FileReadTransHeader( TRANS_TH *pstTransTH, byte *abTransFileName );
static short FileReadTransDataWithFD( FILE *fdFile, TRANS_TD *pstTransTD,
	dword dwIndex );
static short FileReadTransDataWithoutTransHeaderWithFD( FILE *fdFile,
	TRANS_TD *pstTransTD, dword dwIndex );
static short FileReadTransTail( TRANS_TT *pstTransTT, byte *abTransFileName );
static short FileWriteControlTrans( CONTROL_TRANS *pstControlTrans );
static short FileWriteTransHeader( TRANS_TH *pstTransTH,
	byte *abTransFileName );
static short FileAppendTransData( TRANS_TD *pstTransTD, byte *abTransFileName );
static short FileAppendTransDataWithFD( FILE *fdFile, TRANS_TD *pstTransTD );
static short FileAppendSubTransData( byte *abSubTransTD,
	byte *abTransFileName );
static short FileAppendTransTail( TRANS_TT *pstTransTT, byte *abTransFileName );
static short FileUpdateControlTrans( dword dwFare );
static short FileUpdateTransHeaderAndAppendTransTail( TRANS_TT *pstTransTT,
	byte *abTransFileName );
static bool IsExistTransDataWithFD( FILE *fdFile, TRANS_TD *pstTransTD,
	dword dwTotCnt );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteTransHeader                                         *
*                                                                              *
*  DESCRIPTION:       �����������ϰ� �����ŷ����������� �����ϰ� ������������  *
*                     �� �����ŷ����������� ����� ������ WRITE�Ѵ�.           *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS                    *
*                         - ������������ OPEN ����                             *
*                     ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS                   *
*                         - ������������ WRITE ����                            *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - �����ŷ��������� OPEN ����                         *
*                     ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS                      *
*                         - �����ŷ��������� WRITE ����                        *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short WriteTransHeader( void )
{
	short sResult = SUCCESS;
	time_t tStartDriveDtime = 0;
	byte abTransFileName[19] = {0, };

	CONTROL_TRANS stControlTrans;
	TRANS_TH stTransTH;

	/*
	 * ī�� ���±� ���� Ƚ�� �ʱ�ȭ
	 * - ������۽� �ʱ�ȭ�Ǹ�, ��������� �ŷ����������� ���Ͽ� ��ϵȴ�.
	 */
	gwRetagCardCnt = 0;

	/*
	 * ������ �ŷ��������ϸ� ����
	 * - �ŷ��������ϸ��� ����ð��� ������� "YYYYMMDDhhmmss.trn"�� ��������
	 *	�����Ǹ�, ������ ������ �����ϴ� ��� ���� �ð��� sleep�� ��
	 *	�ٽ� ���ϸ��� �����ϵ��� �Ѵ�.
	 */
	CreateTransFileName( abTransFileName, &tStartDriveDtime );

	// �����޸𸮿� ������ �ŷ��������ϸ� ����
	memcpy( gpstSharedInfo->abTransFileName, abTransFileName,
		sizeof( gpstSharedInfo->abTransFileName ) );

	/*
	 * ����������ϱ���ü ����
	 */
	SetControlTrans( &stControlTrans, abTransFileName );

	/*
	 * �ŷ������������ü ����
	 */
	SetTransHeader( &stTransTH, tStartDriveDtime );

	/*
	 * �ŷ��������� ���� �������� ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	/*
	 * ��������������� WIRTE
	 */
	sResult = FileWriteControlTrans( &stControlTrans );
	if ( sResult != SUCCESS )
	{
		goto FINALLY;
	}

	/*
	 * �ŷ�������� WIRTE
	 * - ��������������� WRITE�� SUCCESS�� ��쿡�� �����Ѵ�.
	 */
	sResult = FileWriteTransHeader( &stTransTH, abTransFileName );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				sResult = ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
			case ERR_CARD_PROC_FILE_WRITE_TRANS:
				sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
		}
		sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
		goto FINALLY;
	}

	FINALLY:

	/*
	 * �� ���� �� ��� �ϳ��� WRITE�� ���������� �� ���� ��θ� �����Ѵ�.
	 */
	if ( sResult != SUCCESS )
	{
		unlink( CONTROL_TRANS_FILE );
		unlink( abTransFileName );
	}

	/*
	 * �ŷ��������� ���� �������� FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );

	if ( sResult != SUCCESS)
	{
		ErrRet( sResult );
		return sResult;
	}

	/*
	 * ������۽ð� �������� ����
	 * - GPS �α׿� ����ϱ� ����
	 */
	gtDriveStartDtime = tStartDriveDtime;

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteTransData                                           *
*                                                                              *
*  DESCRIPTION:       �ŷ����������͸� �ŷ��������� �� ����ŷ��������Ͽ�      *
*                     WRITE�Ѵ�.                                               *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī������ ����ü                           *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
void WriteTransData( TRANS_INFO *pstTransInfo )
{
	short sResult = SUCCESS;
	bool boolIsSuccessWrite = TRUE;
	TRANS_TD stTransTD;

	/*
	 * ī����������ü�� ������ �̿��Ͽ� �ŷ���������ü ����
	 */
	SetTransData( pstTransInfo, &stTransTD );

	/*
	 * �ŷ��������� ���� �������� ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	/*
	 * �����ܸ����� ���
	 */
	if ( gboolIsMainTerm )
	{
		/*
		 * �����ŷ��������Ͽ� ���
		 * - ����������������� UPDATE�� ������ �߻��ϴ��� ������ �߻�����
		 *   �ʴ´�.
		 */
		sResult = WriteTransDataInMainTerm( &stTransTD );
		if ( sResult != SUCCESS )
		{
			ErrRet( sResult );
			DebugOut( "[WriteTransData] �����ŷ��������� ��� ����\n" );
		}
	}
	/*
	 * �����ܸ����� ���
	 */
	else
	{
		/*
		 * �����ŷ��������Ͽ� ���
		 */
		sResult = WriteTransDataInSubTerm( &stTransTD );
		if ( sResult != SUCCESS )
		{
			ErrRet( sResult );
			DebugOut( "[WriteTransData] �����ŷ��������� ��� ����\n" );
			boolIsSuccessWrite = FALSE;
		}

		/*
		 * �������ŷ��������Ͽ� ���
		 */
		WriteRemakeTransDataInSubTerm( &stTransTD );
	}

	/*
	 * ��/�����ܸ��� ���� ����ŷ��������Ͽ� �ŷ����� ���
	 */
	sResult = WriteBackupTransData( &stTransTD );
	if ( sResult != SUCCESS )
	{
		ErrRet( sResult );
		DebugOut( "[WriteTransData] ��� �ŷ����� ��� ����\n" );

		/*
		 * �����ܸ��� �ŷ����� ��� �����̸鼭 ��� �ŷ����� ��� ������
		 * �߻��� ��� -> FND�� 119119 ��� �� �ܸ��� ����
		 */
		if ( boolIsSuccessWrite == FALSE )
		{
			DisplayASCInUpFND( FND_ERR_MSG_WRITE_SUB_TRANS_DATA );
			SendKillAllProcSignal();
		}
	}

	/*
	 * �ŷ��������� ���� �������� FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteTransDataForCommProcess                             *
*                                                                              *
*  DESCRIPTION:       ������ ������μ����� ���� �ܸ���κ��� ���۵�           *
*                     �ŷ������� WRITE�ϱ� ���� ����Ѵ�.                      *
*                                                                              *
*  INPUT PARAMETERS:  abTransTD - 202����Ʈ�� ������ �ŷ�����������(TD)        *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-09-01                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short WriteTransDataForCommProcess( byte *abTransTD )
{
	short sResult = SUCCESS;

	/*
	 * �ŷ��������� ���� �������� ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	/*
	 * �����ܸ��� �ŷ����� ���
	 */
	sResult = WriteTransDataInMainTerm( ( TRANS_TD * )abTransTD );
	if ( sResult != SUCCESS )
	{
		ErrRet( sResult );
	}

	/*
	 * �ŷ��������� ���� �������� FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteTransDataInMainTerm                                 *
*                                                                              *
*  DESCRIPTION:       �����ŷ��������Ͽ� �ŷ����������͸� WRITE�ϰ�            *
*                     �������������� ������Ʈ�Ѵ�.                             *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTD - �ŷ����������� ����ü�� ������              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS                    *
*                         - ������������ OPEN ����                             *
*                     ERR_CARD_PROC_FILE_READ_CONTROL_TRANS                    *
*                         - ������������ READ ����                             *
*                     ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS                   *
*                         - ������������ WRITE ����                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short WriteTransDataInMainTerm( TRANS_TD *pstTransTD )
{
	short sResult = SUCCESS;
	byte i = 0;

	/*
	 * �����ܸ��� �ŷ��������Ͽ� �ŷ������� ���
	 * - �ŷ�������� �� �����ϴ� ��� �ܸ��Ⱑ FND�� �޽��� ��� �� �ߴܵȴ�.
	 */
	for ( i = 0; i < 3; i++ )
	{
		sResult = FileAppendTransData( pstTransTD,
			gpstSharedInfo->abTransFileName );
		if ( sResult == SUCCESS )
			break;
	}
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				sResult = ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
				break;
			case ERR_CARD_PROC_FILE_WRITE_TRANS:
				sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
				break;
			default:
				sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
		}
		ErrRet( sResult );

		ctrl_event_info_write( TR_FILE_OPEN_ERROR_EVENT );
		DisplayASCInUpFND( FND_ERR_MSG_WRITE_MAIN_TRANS_DATA );
		Buzzer( 5, 50000 );
		VoiceOut( VOICE_CHECK_TERM );
		ErrRet( ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS );
		SendKillAllProcSignal();
	}

	/*
	 * ����������������� ������Ʈ
	 */
	sResult = FileUpdateControlTrans( pstTransTD->dwFare );

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteTransDataInSubTerm                                  *
*                                                                              *
*  DESCRIPTION:       �����ŷ��������Ͽ� �ŷ����������͸� WRITE�Ѵ�.           *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTD - �ŷ����������� ����ü�� ������              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_SUB_TRANS                        *
*                         - �����ŷ��������� OPEN ����                         *
*                     ERR_CARD_PROC_FILE_WRITE_SUB_TRANS                       *
*                         - �����ŷ��������� WRITE ����                        *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short WriteTransDataInSubTerm( TRANS_TD *pstTransTD )
{
	short sResult = SUCCESS;
	byte i = 0;
	byte abSubTransData[203] = {0, };

	// TD ����
	memcpy( abSubTransData, ( byte * )pstTransTD, sizeof( TRANS_TD ) );
	// ������ ���·� ����
	abSubTransData[202] = SUB_TRANS_DATA_NOT_YET_SEND;

	for ( i = 0; i < 3; i++ )
	{
		sResult = FileAppendSubTransData( abSubTransData, SUB_TERM_TRANS_FILE );
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
		return sResult;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteRemakeTransDataInSubTerm                            *
*                                                                              *
*  DESCRIPTION:       �������ŷ��������Ͽ� �ŷ����������͸� WRITE�Ѵ�.       *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTD - �ŷ����������� ����ü�� ������              *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void WriteRemakeTransDataInSubTerm( TRANS_TD *pstTransTD )
{
	short sResult = SUCCESS;
	byte i = 0;

	/*
	 * �������ŷ��������Ͽ� �ŷ��������
	 * - �ŷ�������� �� �����ϴ� ��� �ܸ��Ⱑ FND�� �޽��� ��� �� �ߴܵȴ�.
	 */
	for ( i = 0; i < 3; i++ )
	{
		sResult = FileAppendTransData( pstTransTD,
			gpstSharedInfo->abTransFileName );
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				sResult = ERR_CARD_PROC_FILE_OPEN_SUB_REMAKE;
				break;
			case ERR_CARD_PROC_FILE_WRITE_TRANS:
				sResult = ERR_CARD_PROC_FILE_WRITE_SUB_REMAKE;
				break;
			default:
				sResult = ERR_CARD_PROC_FILE_WRITE_SUB_REMAKE;
		}
		ErrRet( sResult );

		ctrl_event_info_write( TR_FILE_OPEN_ERROR_EVENT );
		DisplayASCInUpFND( FND_ERR_MSG_WRITE_SUB_TRANS_DATA );
		Buzzer( 5, 50000 );
		VoiceOut( VOICE_CHECK_TERM );
		SendKillAllProcSignal();
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteBackupTransData                                     *
*                                                                              *
*  DESCRIPTION:       ��/��������ŷ��������Ͽ� �ŷ����������͸� WRITE�Ѵ�.    *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTD - �ŷ����������� ����ü�� ������              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_BACKUP_TRANS                     *
*                         - ����ŷ��������� OPEN ����                         *
*                     ERR_CARD_PROC_FILE_WRITE_BACKUP_TRANS                    *
*                         - ����ŷ��������� WRITE ����                        *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short WriteBackupTransData( TRANS_TD *pstTransTD )
{
	short sResult = SUCCESS;
	FILE *fdFile = NULL;
	byte i = 0;
	int nResult = 0;
	word wSeqNo = 0;
	byte abFileName[256] = {0, };
	bool boolAlreadyExist = FALSE;

	/*
	 * ��/�����ܸ��� ���ο� ���� ����ŷ��������ϸ��� ����
	 */
	if ( gboolIsMainTerm == TRUE )
	{
		memcpy( abFileName, MAIN_TERM_BACKUP_TRANS_FILE,
			strlen( MAIN_TERM_BACKUP_TRANS_FILE ) );
	}
	else
	{
		memcpy( abFileName, SUB_TERM_BACKUP_TRANS_FILE,
			strlen( SUB_TERM_BACKUP_TRANS_FILE ) );
	}

	/*
	 * �ش� ����ŷ����������� ���翩�ο� ���� ���� �ٸ� ���� ������ OPEN
	 */
	boolAlreadyExist = IsExistFile( abFileName );

	for ( i = 0; i < 3; i++ )
	{
		// ���� �̸��� ���� ������
		if ( boolAlreadyExist == TRUE )
		{
			fdFile = fopen( abFileName, "rb+" );
		}
		// ���� �̸��� ���� �������� ����
		else
		{
			fdFile = fopen( abFileName, "wb+" );
		}

		if ( fdFile != NULL )
		{
			break;
		}
	}
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_BACKUP_TRANS;
		goto FINALLY;
	}

	/*
	 * ����ŷ����������� ����ŷ������������Ϸκ��� ������
	 */
	wSeqNo = GetBackupTransDataSeqNo();

	/*
	 * �ŷ������� ����ŷ��������Ͽ� WRITE
	 */
	fseek( fdFile, ( wSeqNo * sizeof( TRANS_TD ) ), SEEK_SET );
	for ( i = 0; i < 3; i++ )
	{
		nResult = fwrite( ( byte * )pstTransTD, sizeof( TRANS_TD ), 1, fdFile );
		if ( nResult == 1 )
			break;
	}
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_BACKUP_TRANS;
		goto FINALLY;
	}

	FINALLY:

	/*
	 * ���� CLOSE
	 */
	if ( fdFile != NULL )
	{
		fflush( fdFile );
		fclose( fdFile );
	}

	if ( sResult != SUCCESS )
	{
		ctrl_event_info_write( TR_FILE_BACKUP_ERROR_EVENT );
		return sResult;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteCashTransData                                       *
*                                                                              *
*  DESCRIPTION:       �����ŷ��������Ͽ� �Էµ� ����������� �ش��ϴ�          *
*                     ���ݰŷ����������͸� �����Ͽ� �����ŷ����� ���� ��       *
*                     ���� ��� �ŷ����� ���Ͽ� WRITE�Ѵ�.                     *
*                                                                              *
*  INPUT PARAMETERS:  bUserType - ���������                                   *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-30                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
void WriteCashTransData( byte bUserType )
{
	short sResult = SUCCESS;
	TRANS_TD stTransTD;

	/*
	 * �ŷ��������� ���� �������� ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	/*
	 * ���ݰŷ����������� ����
	 */
	SetCashTransData( &stTransTD, bUserType );

	/*
	 * �����ŷ��������Ͽ� ���
	 */
	sResult = WriteTransDataInMainTerm( &stTransTD );
	if ( sResult != SUCCESS )
	{
		ErrRet( sResult );
	}

	/*
	 * ����ŷ��������Ͽ� ���
	 */
	sResult = WriteBackupTransData( &stTransTD );
	if ( sResult != SUCCESS )
	{
		ErrRet( sResult );
	}

	/*
	 * �ŷ��������� ���� �������� FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteTransTail                                           *
*                                                                              *
*  DESCRIPTION:       �����ŷ����������� ����(TT)�� �����ϰ� ���ϸ���          *
*                     ������ �� ������� ������ ���� ���ܻ��� ����Ʈ��         *
*                     ����Ѵ�.                                                *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���༺��                                       *
*                     ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS                    *
*                         - ������������ OPEN ����                             *
*                     ERR_CARD_PROC_FILE_READ_CONTROL_TRANS                    *
*                         - ������������ READ ����                             *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - �����ŷ��������� OPEN ����                         *
*                     ERR_CARD_PROC_FILE_READ_MAIN_TRANS                       *
*                         - �����ŷ��������� READ ����                         *
*                     ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS                      *
*                         - �����ŷ��������� WRITE ����                        *
*                     ERR_CARD_PROC_FILE_RENAME_MAIN_TRANS                     *
*                         - �����ŷ��������� RENAME ����                       *
*                     ERR_CARD_PROC_FILE_WRITE_RECONCILE_LIST                  *
*                         - RECONCILE ���� WRITE ����                          *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short WriteTransTail( void )
{
	short sResult = SUCCESS;
	time_t tEndDriveDtime = 0;
	dword dwTotalCnt = 0;
	dword dwTotalAmt = 0;
	byte abEndedTransFileName[15] = {0, };
	CONTROL_TRANS stControlTrans;
	TRANS_TT stTransTT;

	/*
	 * ���������� �ŷ��������ϸ� ����
	 * - ������ ����Ǹ� "YYYYMMDDhhmmss.trn" ������ ������ �ŷ��������ϸ���
	 *   "YYYYMMDDhhmmss" ������ ���������� �ŷ��������ϸ����� RENAME�Ѵ�.
	 *   ���� �ð��� �̿��Ͽ� ���������� �ŷ��������ϸ��� �����ϰ�,
	 *   ������ ������ �����ϴ� ��� ���� �ð��� sleep�� �� �ٽ� ���ϸ���
	 *   �����ϵ��� �Ѵ�.
	 */
	CreateEndedTransFileName( abEndedTransFileName, &tEndDriveDtime );

	// �����޸𸮿� ���������� �ŷ��������ϸ� ����
	memcpy( gpstSharedInfo->abEndedTransFileName, abEndedTransFileName,
		sizeof( gpstSharedInfo->abEndedTransFileName ) );

	/*
	 * �ŷ��������� ���� �������� ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	/*
	 * ��������������� READ
	 */
	sResult = FileReadControlTrans( &stControlTrans );
	if ( sResult != SUCCESS )
	{
		goto FINALLY;
	}

	/*
	 * �̹� ������ �ŷ����� �ѰǼ� �� �ѱݾ� ����
	 */
	dwTotalCnt = GetDWORDFromASC( stControlTrans.abTotalCnt, 9 );
	dwTotalAmt = GetDWORDFromASC( stControlTrans.abTotalAmt, 10 );

	/*
	 * �ŷ��������� ����
	 * - �̹� ������ �ŷ����� �ѰǼ� �� �ѱݾ�, �׸��� ��������ð��� �̿��Ͽ�
	 *   �ŷ��������� ����ü�� �����Ѵ�.
	 */
	SetTransTail( &stTransTT, dwTotalCnt, dwTotalAmt, tEndDriveDtime );

	/*
	 * �ŷ�������� ������Ʈ �� �ŷ��������� APPEND
	 * - �ŷ���������� ���ϸ� �ʵ带 ������Ʈ�� �� �ŷ����������� APPEND�Ѵ�.
	 */
	sResult = FileUpdateTransHeaderAndAppendTransTail( &stTransTT,
		gpstSharedInfo->abTransFileName );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				sResult = ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
				break;
			case ERR_CARD_PROC_FILE_READ_TRANS:
				sResult = ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
				break;
			case ERR_CARD_PROC_FILE_WRITE_TRANS:
				sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
				break;
		}
		sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
		goto FINALLY;
	}

	/*
	 * �ŷ��������ϸ� RENAME
	 * - �������� �ŷ�������� ������Ʈ �� �ŷ��������� APPEND �۾��� ������
	 *   ��쿡�� �����Ѵ�.
	 * - ������ �ŷ��������ϸ��� �ŷ����������� ���������� �ŷ��������ϸ�����
	 *   RENAME�Ѵ�.
	 */
	sResult = FileRenameTransFile( gpstSharedInfo->abTransFileName,
		gpstSharedInfo->abEndedTransFileName );
	if ( sResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_RENAME_MAIN_TRANS;
		goto FINALLY;
	}

	FINALLY:

	/*
	 * �ŷ��������� ���� �������� FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );

	if ( sResult != SUCCESS )
	{
		return ErrRet( sResult );
	}

	/*
	 * RECONCILE ���Ͽ� ���
	 */
	sResult = AppendTransFileToReconcileFile( tEndDriveDtime );
	if ( sResult != SUCCESS )
	{
		return ErrRet ( sResult );
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateTransFileName                                      *
*                                                                              *
*  DESCRIPTION:       "YYYYMMDDhhmmss.trn" ������ ������ �ŷ��������ϸ���      *
*                     �����Ѵ�. ���� �ð��� ������� �����Ǹ�, ������ ���ϸ��� *
*                     �����ϴ� ��쿡�� �����ð��� sleep�� �� ������Ѵ�.      *
*                                                                              *
*  INPUT PARAMETERS:  abTransFileName - ������ �ŷ��������ϸ��� �����ϱ� ����  *
*                         byte �迭�� ������                                   *
*                     ptStartDriveDtime - ������۽ð��� �����ϱ� ���� time_t  *
*                         Ÿ���� ������                                        *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void CreateTransFileName( byte *abTransFileName,
	time_t *ptStartDriveDtime )
{
	GetRTCTime( ptStartDriveDtime );
	while ( TRUE )
	{
		TimeT2ASCDtime( *ptStartDriveDtime, abTransFileName );
		memcpy( &abTransFileName[14], ".trn", 4 );
		abTransFileName[18] = '\0';			// ���Ṯ�� ����

		// ���� �̸��� ���� ������
		if ( IsExistFile( abTransFileName ) )
		{
			ptStartDriveDtime++;
		}
		// ���� �̸��� ���� �������� ����
		else
		{
			break;
		}
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CreateEndedTransFileName                                 *
*                                                                              *
*  DESCRIPTION:       "YYYYMMDDhhmmss" ������ ���������� �ŷ��������ϸ���      *
*                     �����Ѵ�. ���� �ð��� ������� �����Ǹ�, ������ ���ϸ��� *
*                     �����ϴ� ��쿡�� �����ð��� sleep�� �� ������Ѵ�.      *
*                                                                              *
*  INPUT PARAMETERS:  abEndedTransFileName - ���������� �ŷ��������ϸ���       *
*                         �����ϱ� ���� byte �迭�� ������                     *
*                     ptEndDriveDtime - ��������ð��� �����ϱ� ���� time_t    *
*                         Ÿ���� ������                                        *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void CreateEndedTransFileName( byte *abEndedTransFileName,
	time_t *ptEndDriveDtime )
{
	GetRTCTime( ptEndDriveDtime );
	while ( TRUE )
	{
		TimeT2ASCDtime( *ptEndDriveDtime, abEndedTransFileName );
		abEndedTransFileName[14] = '\0';	// ���Ṯ�� ����

		// ���� �̸��� ������ �����ϸ�
		if ( IsExistFile( abEndedTransFileName ) )
		{
			ptEndDriveDtime++;
		}
		// ���� �̸��� ���� �������� ����
		else
		{
			break;
		}
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       AppendTransFileToReconcileFile                           *
*                                                                              *
*  DESCRIPTION:       "YYYYMMDDhhmmss" ������ ���������� �ŷ��������ϸ���      *
*                     ���� �ŷ����������� RECONCILE ���Ͽ� �߰��Ѵ�.           *
*                                                                              *
*  INPUT PARAMETERS:  tNowDtime - RECONCILE ���� ��� �ð�                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_WRITE_RECONCILE_LIST                  *
*                         - RECONCILE ���� WRITE ����                          *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short AppendTransFileToReconcileFile( time_t tNowDtime )
{
	short sResult = SUCCESS;
	byte i = 0;
	RECONCILE_DATA stReconcileData;

	memset( ( byte * )&stReconcileData, 0, sizeof( RECONCILE_DATA ) );

	// ����PC�� ������ ���ϸ� - ���Ṯ�ڱ��� �Բ� ����
	memcpy( stReconcileData.achFileName, gpstSharedInfo->abEndedTransFileName,
		sizeof( gpstSharedInfo->abEndedTransFileName ) );

	// ������ ���ϻ��� - TR���� �۽Ŵ��
	stReconcileData.bSendStatus = RECONCILE_SEND_WAIT;

	// ���ϸ��Write�ð�
	stReconcileData.tWriteDtime = tNowDtime;

	// RECONCILE ���Ͽ� ���
	for ( i = 0; i < 3; i++ )
	{
		sResult = WriteReconcileFileList( &stReconcileData );
		if ( sResult == SUCCESS )
			break;
	}
	if ( sResult != SUCCESS )
	{
		return ERR_CARD_PROC_FILE_WRITE_RECONCILE_LIST;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       UpdateTransDCSRecvDtime                                  *
*                                                                              *
*  DESCRIPTION:       ������Ÿ��κ��� ȣ��Ǿ� �����ŷ�����������           *
*                     ����ý��ۼ����Ͻø� ����ð����� ������Ʈ �Ѵ�.         *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - �����ŷ��������� OPEN ����                         *
*                     ERR_CARD_PROC_FILE_READ_MAIN_TRANS                       *
*                         - �����ŷ��������� READ ����                         *
*                     ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS                      *
*                         - �����ŷ��������� WRITE ����                        *
*                                                                              *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short UpdateTransDCSRecvDtime( byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	dword dwCRC = 0;
	time_t tNowDtime = 0;
	byte i = 0;
	byte abASCNowDtime[14] = {0, };
	FILE *fdFile = NULL;
	TRANS_TH stTransTH;

	memset( ( byte * )&stTransTH, 0, sizeof( TRANS_TH ) );

	// RTC�κ��� ����ð��� ������
	GetRTCTime( &tNowDtime );
	TimeT2ASCDtime( tNowDtime, abASCNowDtime );

	/*
	 * �ŷ��������� ���� �������� ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	fdFile = fopen( abTransFileName, "rb+" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
		goto FINALLY;
	}

	fseek( fdFile, 0 ,SEEK_SET );

	nResult = fread( ( byte * )&stTransTH, sizeof( TRANS_TH ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
		goto FINALLY;
	}

	// �ŷ���������� ����ý��� �����Ͻ�
	memcpy( stTransTH.abDCSRecvDtime, abASCNowDtime,
		sizeof( stTransTH.abDCSRecvDtime ) );

	// �ŷ���������� Record Mac
	dwCRC = MakeCRC32( ( byte * )&stTransTH, sizeof( TRANS_TH ) - 6 );
	memcpy( stTransTH.abRecMAC, ( byte * )&dwCRC,
		sizeof( stTransTH.abRecMAC ) );

	// �ŷ�������� WRITE
	fseek( fdFile, 0 ,SEEK_SET );

	for ( i = 0; i < 3; i++ )
	{
		nResult = fwrite( ( byte * )&stTransTH, sizeof( TRANS_TH ), 1, fdFile );
		if ( nResult == 1 )
			break;
	}
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_MAIN_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fflush( fdFile );
		fclose( fdFile );
	}

	/*
	 * �ŷ��������� ���� �������� FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );

	ErrRet( sResult );

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RemakeTrans                                              *
*                                                                              *
*  DESCRIPTION:       �����κ��� ���۵� �����ӽô�����ϰ� �����ŷ���������    *
*                     ������ ����۾��� �����Ѵ�.                              *
*                     ���� �ӽô������( "temptd.dat" )�� �����ŷ�����������   *
*                     ��� �� �����͸� ������ �� �� �����ӽô�������� TD��    *
*                     �����ŷ��������Ͽ� �����ϴ����� ���θ� üũ�� ��         *
*                     �������� �ʴ´ٸ� �ӽô�����Ͽ� �߰��Ѵ�.               *
*                     �׸��� ���������� �ӽô�����Ͽ� TT�� �߰��� ��          *
*                     �ӽô�������� �����ŷ��������ϸ����� rename�Ѵ�.        *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - �����ŷ��������� OPEN ����                         *
*                     ERR_CARD_PROC_FILE_READ_MAIN_TRANS                       *
*                         - �����ŷ��������� READ ����                         *
*                     ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE                      *
*                         - �ӽô������ OPEN ����                             *
*                     ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE                     *
*                         - �ӽô������ WRITE ����                            *
*                     ERR_CARD_PROC_FILE_OPEN_SUB_TEMP_REMAKE                  *
*                         - �����ӽô������ OPEN ����                         *
*                     ERR_CARD_PROC_FILE_READ_SUB_TEMP_REMAKE                  *
*                         - �����ӽô������ READ ����                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-10                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
short RemakeTrans( void )
{
	short sResult = SUCCESS;
	byte abRemakeTransFileName[3][18];		// ���Ṯ�ڸ� �����ϴ�
											// �� �����ܸ����� ������ϸ�
											// YYYYMMDDhhmmss.#0
	dword i = 0;
	dword dwMainTDCnt = 0;
	int nResultSize = 0;

	dword dwTotalCnt = 0;
	dword dwTotalAmt = 0;

	// �����ܸ��Ⱑ �������� ������ SUCCESS ���� ///////////////////////////////
	if ( gbSubTermCnt == 0 )
		return SUCCESS;

	DebugOut( "[RemakeTrans] �ŷ��������ϸ�		 : %s\n",
		gpstSharedInfo->abTransFileName );
	DebugOut( "[RemakeTrans] �����İŷ��������ϸ� : %s\n",
		gpstSharedInfo->abEndedTransFileName );

	/*
	 * �� �����ܸ���κ��� �����ܸ���� ���۵� ����������ϸ��� ����
	 * - YYYYMMDDhhmmss.10
	 *   YYYYMMDDhhmmss.20
	 *   YYYYMMDDhhmmss.30
	 * - �����ܸ��Ⱑ �����ϰ� �ִ� �ŷ��������ϸ���
	 *   ������ �ŷ��������ϸ��̹Ƿ�, YYYYMMDDhhmmss �����
	 *   ���������İŷ��������ϸ��� �ƴ� ������ �ŷ��������ϸ���
	 *   �����ϵ��� �Ѵ�.
	 */
	memset( abRemakeTransFileName, 0, sizeof( abRemakeTransFileName ) );
	for ( i = 0; i < gbSubTermCnt; i++ )
	{
		memcpy( abRemakeTransFileName[i], gpstSharedInfo->abTransFileName, 14 );
		abRemakeTransFileName[i][14] = '.';
		abRemakeTransFileName[i][15] = ( i + 1 ) + '0';
		abRemakeTransFileName[i][16] = '0';

		DebugOutlnASC( "[RemakeTrans] ����������ϸ� : ",
			abRemakeTransFileName[i], 17 );
	}

	// �����ŷ��������� TD �Ǽ� Ȯ�� ///////////////////////////////////////////
	dwMainTDCnt = ( ( dword )GetFileSize( gpstSharedInfo->abEndedTransFileName )
		- sizeof( TRANS_TH ) - sizeof( TRANS_TT ) ) / sizeof( TRANS_TD );

	DebugOut( "[RemakeTrans] �ŷ��������ϰǼ� : %lu\n", dwMainTDCnt );

	/*
	 * �ŷ��������� ���� �������� ALLOC ****************************************
	 */
	SemaAlloc( SEMA_KEY_TRANS );

	/*
	 * �����ŷ����������� TH �� TD�� �ӽô�����Ϸ� ����
	 */
	sResult = CopyTransHeaderAndTransData( gpstSharedInfo->abEndedTransFileName,
		TEMP_REMAKE_FILE, &dwTotalCnt, &dwTotalAmt );
	if ( sResult != SUCCESS )
	{
		goto FINALLY;
	}

	// TD ���� ( �����ӽô������ -> �ӽô������ ) ////////////////////////////
	for ( i = 0; i < gbSubTermCnt; i++ )
	{
		DebugOutlnASC( "[RemakeTrans] ����������� ó������ : ",
			abRemakeTransFileName[i], 17 );

		// �����ӽô�������� �������� ������ continue /////////////////////////
		if ( !IsExistFile( abRemakeTransFileName[i] ) )
		{
			DebugOutlnASC( "[RemakeTrans] ����������� ������ : ",
				abRemakeTransFileName[i], 17 );
			continue;
		}

		sResult = RemakeTransData( gpstSharedInfo->abEndedTransFileName,
			abRemakeTransFileName[i], TEMP_REMAKE_FILE,
			&dwTotalCnt, &dwTotalAmt );
		if ( sResult != SUCCESS )
		{
			goto FINALLY;
		}

		DebugOutlnASC( "[RemakeTrans] ����������� ó������ : ",
			abRemakeTransFileName[i], 17 );
	}

	sResult = CopyTransTail( gpstSharedInfo->abEndedTransFileName,
		TEMP_REMAKE_FILE, dwTotalCnt, dwTotalAmt );
	if ( sResult != SUCCESS )
	{
		goto FINALLY;
	}

	// �ӽô�������� �����ŷ��������Ϸ� RENAME ////////////////////////////////
	for ( i = 0; i < 3; i++ )
	{
		nResultSize = rename( TEMP_REMAKE_FILE,
			gpstSharedInfo->abEndedTransFileName );
		if ( nResultSize != -1 )
			break;
	}
	if ( nResultSize == -1 )
	{
		ctrl_event_info_write( TR_REMAKE_RENAME_ERROR_EVENT );
		sResult = ERR_CARD_PROC_FILE_RENAME_TEMP_REMAKE;
		goto FINALLY;
	}

	FINALLY:

	// �����ܸ��������� �� �ӽô������ DELETE ///////////////////////////////
	for ( i = 0; i < 3; i++ )
	{
		unlink( abRemakeTransFileName[i] );
	}

	unlink( TEMP_REMAKE_FILE );

	/*
	 * �ŷ��������� ���� �������� FREE *****************************************
	 */
	SemaFree( SEMA_KEY_TRANS );

	ErrRet( sResult );

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CopyTransHeaderAndTransData                              *
*                                                                              *
*  DESCRIPTION:       �����ŷ����������� ��� �� �����͸� �ӽô�����Ϸ�       *
                      �����Ѵ�. �� �������� �ѰǼ� �� �ѱݾ��� �ٽ� ����Ѵ�.  *
*                                                                              *
*  INPUT PARAMETERS:  abMainTransFileName - �����ŷ��������ϸ�                 *
*                     abSubTempRemakeFileName - �����ӽô�����ϸ�             *
*                     abTempRemakeFileName - �ӽô�����ϸ�                    *
*                     pdwTotalCnt - �ѰǼ� ������                              *
*                     pdwTotalAmt - �ѱݾ� ������                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - �����ŷ��������� OPEN ����                         *
*                     ERR_CARD_PROC_FILE_READ_MAIN_TRANS                       *
*                         - �����ŷ��������� READ ����                         *
*                     ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE                      *
*                         - �ӽô������ OPEN ����                             *
*                     ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE                     *
*                         - �ӽô������ WRITE ����                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short CopyTransHeaderAndTransData( byte *abSrcTransFileName,
	byte *abDesTransFileName, dword *pdwTotalCnt, dword *pdwTotalAmt )
{
	short sResult = SUCCESS;
	dword i = 0;
	dword dwSrcTDCnt = 0;
	FILE *fdSrcFile = NULL;
	FILE *fdDesFile = NULL;
	TRANS_TH stTransTH;
	TRANS_TD stTransTD;

	dwSrcTDCnt = ( ( dword )GetFileSize( abSrcTransFileName ) -
		sizeof( TRANS_TH ) - sizeof( TRANS_TT ) ) / sizeof( TRANS_TD );

	DebugOut( "[CopyTransHeaderAndTransData] �����ŷ������Ǽ�	  : %lu\n",
		dwSrcTDCnt );

	/*
	 * �ŷ�������� COPY
	 */
	sResult = FileReadTransHeader( &stTransTH, abSrcTransFileName );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				return ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
			case ERR_CARD_PROC_FILE_READ_TRANS:
				return ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
		}
		return ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
	}
	sResult = FileWriteTransHeader( &stTransTH, abDesTransFileName );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				return ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE;
			case ERR_CARD_PROC_FILE_WRITE_TRANS:
				return ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE;
		}
		return ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE;
	}

	/*
	 * �ŷ����������� COPY
	 */
	fdSrcFile = fopen( abSrcTransFileName, "rb" );
	if ( fdSrcFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
		goto FINALLY;
	}

	fdDesFile = fopen( abDesTransFileName, "ab" );
	if ( fdDesFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE;
		goto FINALLY;
	}

	for ( i = 0; i < dwSrcTDCnt; i++ )
	{
		sResult = FileReadTransDataWithFD( fdSrcFile, &stTransTD, i );
		if ( sResult != SUCCESS )
		{
			sResult = ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
			goto FINALLY;
		}
		sResult = FileAppendTransDataWithFD( fdDesFile, &stTransTD );
		if ( sResult != SUCCESS )
		{
			sResult = ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE;
			goto FINALLY;
		}

		*pdwTotalCnt += 1;
		*pdwTotalAmt += stTransTD.dwFare;
	}

	FINALLY:

	if ( fdSrcFile != NULL )
	{
		fclose( fdSrcFile );
	}
	if ( fdDesFile != NULL )
	{
		fclose( fdDesFile );
	}

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       RemakeTransData                                          *
*                                                                              *
*  DESCRIPTION:       �����ŷ��������ϰ� �����ӽô�����ϰ��� ����۾���       *
*                     �����Ͽ� �ӽô�����Ͽ� WRITE�Ѵ�.                       *
*                     (�����ӽô�����Ͽ� �����ϸ鼭 �����ŷ��������Ͽ�        *
*                      �������� �ʴ� �ŷ����������͸� �ӽô����Ͽ� �߰��Ѵ�.)  *
*                                                                              *
*  INPUT PARAMETERS:  abMainTransFileName - �����ŷ��������ϸ�                 *
*                     abSubTempRemakeFileName - �����ӽô�����ϸ�             *
*                     abTempRemakeFileName - �ӽô�����ϸ�                    *
*                     pdwTotalCnt - �ѰǼ� ������                              *
*                     pdwTotalAmt - �ѱݾ� ������                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - �����ŷ��������� OPEN ����                         *
*                     ERR_CARD_PROC_FILE_OPEN_SUB_TEMP_REMAKE                  *
*                         - �����ӽô������ OPEN ����                         *
*                     ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE                      *
*                         - �ӽô������ OPEN ����                             *
*                     ERR_CARD_PROC_FILE_READ_SUB_TEMP_REMAKE                  *
*                         - �����ӽô������ READ ����                         *
*                     ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE                     *
*                         - �ӽô������ WRITE ����                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short RemakeTransData( byte *abMainTransFileName,
	byte *abSubTempRemakeFileName, byte *abTempRemakeFileName,
	dword *pdwTotalCnt, dword *pdwTotalAmt )
{
	short sResult = SUCCESS;
	dword dwCnt = 0;
	dword dwSubRemakeFileSize = 0;
	dword dwMainTDCnt = 0;
	TRANS_TD stTransTD;

	FILE *fdMainTransFile = NULL;
	FILE *fdSubRemakeFile = NULL;
	FILE *fdTempRemakeFile = NULL;

	// �����ӽô������ SIZE Ȯ�� //////////////////////////////////////////////
	dwSubRemakeFileSize = ( dword )GetFileSize( abSubTempRemakeFileName );

	if ( dwSubRemakeFileSize % sizeof( TRANS_TD ) != 0 )
	{
		DebugOut( "[DEBUG] �������ŷ��������� ũ�� ����\n" );
		return SUCCESS;
	}

	dwMainTDCnt = ( ( dword )GetFileSize( abMainTransFileName ) -
		sizeof( TRANS_TH ) - sizeof( TRANS_TT ) ) / sizeof( TRANS_TD );

	fdMainTransFile = fopen( abMainTransFileName, "rb" );
	if ( fdMainTransFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
		goto FINALLY;
	}

	fdSubRemakeFile = fopen( abSubTempRemakeFileName, "rb" );
	if ( fdSubRemakeFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_SUB_TEMP_REMAKE;
		goto FINALLY;
	}

	fdTempRemakeFile = fopen( abTempRemakeFileName, "ab" );
	if ( fdTempRemakeFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE;
		goto FINALLY;
	}

	// �������ŷ��������� �� TD�� �����ŷ����������� ��� TD�� ///////////////
	// ���� �� �������� ������ �ӽô�����Ͽ� �߰�
	for ( dwCnt = 0; dwCnt < dwSubRemakeFileSize / sizeof( TRANS_TD ); dwCnt++ )
	{
		sResult = FileReadTransDataWithoutTransHeaderWithFD( fdSubRemakeFile,
			&stTransTD, dwCnt );
		if ( sResult != SUCCESS )
		{
			sResult = ERR_CARD_PROC_FILE_READ_SUB_TEMP_REMAKE;
			goto FINALLY;
		}

		DebugOutlnBCD( "[RemakeTrans] \t����������� TD READ : ",
			stTransTD.abEntExtDtime, 7 );

		// �������� �ʴ� TD�� ��� �ӽô�����Ͽ� �߰��Ѵ�.
		if ( IsExistTransDataWithFD( fdMainTransFile, &stTransTD, dwMainTDCnt )
			== FALSE )
		{
			DebugOut( "[RemakeTrans] \t�������� �ʴ� TD�̹Ƿ� �߰���\n" );

			sResult = FileAppendTransDataWithFD( fdTempRemakeFile, &stTransTD );
			if ( sResult != SUCCESS )
			{
				sResult = ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE;
				goto FINALLY;
			}

			*pdwTotalCnt += 1;
			*pdwTotalAmt += stTransTD.dwFare;
		}
		else
		{
			DebugOut( "[RemakeTrans] \t�̹� �����ϴ� TD�̹Ƿ� �߰�����" );
			DebugOut( " ����\n" );
		}
	}

	FINALLY:

	if ( fdMainTransFile != NULL )
	{
		fclose( fdMainTransFile );
	}
	if ( fdSubRemakeFile != NULL )
	{
		fclose( fdSubRemakeFile );
	}
	if ( fdTempRemakeFile != NULL )
	{
		fclose( fdTempRemakeFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       CopyTransTail                                            *
*                                                                              *
*  DESCRIPTION:       �����ŷ����������� �ŷ����������� �ӽô�����Ϸ�         *
*                     �����Ѵ�. ��, �ѰǼ� �� �ѱݾ��� �ٽ� �����Ѵ�.          *
*                                                                              *
*  INPUT PARAMETERS:  abSrcTransFileName - �����ŷ��������ϸ�                  *
*                     abDesTransFileName - �ӽô�����ϸ�                      *
*                     dwTotalCnt - �ѰǼ�                                      *
*                     dwTotalAmt - �ѱݾ�                                      *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS                       *
*                         - �����ŷ��������� OPEN ����                         *
*                     ERR_CARD_PROC_FILE_READ_MAIN_TRANS                       *
*                         - �����ŷ��������� READ ����                         *
*                     ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE                      *
*                         - �ӽô������ OPEN ����                             *
*                     ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE                     *
*                         - �ӽô������ WRITE ����                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short CopyTransTail( byte *abSrcTransFileName,
	byte *abDesTransFileName, dword dwTotalCnt, dword dwTotalAmt )
{
	short sResult = SUCCESS;
	dword dwCRC = 0;
	TRANS_TT stTransTT;

	// TT ���� ( �����ŷ��������� -> �ӽô������ ) //////////////////////////////
	// �����ŷ��������� READ
	sResult = FileReadTransTail( &stTransTT, abSrcTransFileName );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				return ERR_CARD_PROC_FILE_OPEN_MAIN_TRANS;
			case ERR_CARD_PROC_FILE_READ_TRANS:
				return ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
		}
		return ERR_CARD_PROC_FILE_READ_MAIN_TRANS;
	}

	// �ѰǼ� �ٽ� ����
	DWORD2ASCWithFillLeft0( dwTotalCnt, stTransTT.abTotalCnt, 9 );
	// �ѱݾ� �ٽ� ����
	DWORD2ASCWithFillLeft0( dwTotalAmt, stTransTT.abTotalAmt, 10 );
	// Record MAC �ٽ� ����
	dwCRC = MakeCRC32( ( byte * )&stTransTT, sizeof( TRANS_TT ) - 6 );
	memcpy( stTransTT.abRecMAC, ( byte * )&dwCRC,
		sizeof( stTransTT.abRecMAC ) );

	// �ӽô�����Ͽ� WRITE
	sResult = FileAppendTransTail( &stTransTT, abDesTransFileName );
	if ( sResult != SUCCESS )
	{
		switch ( sResult )
		{
			case ERR_CARD_PROC_FILE_OPEN_TRANS:
				return ERR_CARD_PROC_FILE_OPEN_TEMP_REMAKE;
			case ERR_CARD_PROC_FILE_WRITE_TRANS:
				return ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE;
		}
		return ERR_CARD_PROC_FILE_WRITE_TEMP_REMAKE;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetTermAggrSeqNo                                         *
*                                                                              *
*  DESCRIPTION:       �ܸ����������Ϸù�ȣ�� "seqth.tmp" ���Ϸκ���          *
*                     1 �����Ͽ� �����´�.                                     *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: word - �ܸ����������Ϸù�ȣ                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static word GetTermAggrSeqNo( void )
{
	int nResult = 0;
	int fdFile = 0;
	word wSeqNo = 0;
	byte abBuf[4] = {0, };

	/*
	 * ���� �̸��� ������ �̹� �����ϴ� ��� ***********************************
	 * - ���Ϸκ��� �Ϸù�ȣ�� �о� 1 �����Ͽ� WRITE�� �� �� ���� �����Ѵ�.
	 */
	if ( IsExistFile( TERM_AGGR_SEQ_NO_FILE ) )
	{
		fdFile = open( TERM_AGGR_SEQ_NO_FILE, O_RDWR | O_CREAT );
		if ( fdFile < 0 )
		{
			return 0;
		}

		nResult = read( fdFile, abBuf, sizeof( abBuf ) );
		// ���� READ - READ�� size�� 4
		if ( nResult == 4 )
		{
			wSeqNo = GetWORDFromASC( abBuf, 4 );
		}
		// ���� READ - READ�� size�� 4�� �ƴ�
		else
		{
			close( fdFile );
			return 0;
		}

		// 0000 ~ 9999 �����̼�
		wSeqNo = ( wSeqNo + 1 ) % 10000;

		WORD2ASCWithFillLeft0( wSeqNo, abBuf, sizeof( abBuf ) );

		// ������ �������� ������ �̵� - ���ϰ� ����
		lseek( fdFile, 0L, SEEK_SET );

		nResult = write( fdFile, abBuf, sizeof( abBuf ) );

		// ���� WRITE �̸� - WRITE�� size�� 4�� �ƴϸ�
		if ( nResult != 4 )
		{
			close( fdFile );
			return 0;
		}

		close( fdFile );

		return wSeqNo;
	}
	/*
	 * ���� �̸��� ������ �������� �ʴ� ��� ***********************************
	 * - "0000"�� ���� ���Ͽ� WRITE�� �� 0�� �����Ѵ�.
	 */
	else
	{
		fdFile = open( TERM_AGGR_SEQ_NO_FILE, O_RDWR | O_CREAT );
		if ( fdFile < 0 )
		{
			return 0;
		}

		memset( abBuf, '0', sizeof( abBuf ) );

		nResult = write( fdFile, abBuf, sizeof( abBuf ) );

		// ���� WRITE �̸� - WRITE�� size�� 4�� �ƴϸ�
		if ( nResult != 4 )
		{
			close( fdFile );
			return 0;
		}

		close( fdFile );

		return 0;
	}

	return 0;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetTermAggrSeqNo                                         *
*                                                                              *
*  DESCRIPTION:       ����ŷ����������� "backseq.tmp" ���Ϸκ���              *
*                     1 �����Ͽ� �����´�.                                     *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: word - ����ŷ���������                                  *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static word GetBackupTransDataSeqNo( void )
{
	int fdFile = 0;
	word wSeqNo = 0;
	int nResult = 0;
	byte abBuf[4] = {0, };

	// ���� �̸��� ���� ������
	if ( IsExistFile( BACKUP_TRANS_SEQ_NO_FILE ) )
	{
		fdFile = open( BACKUP_TRANS_SEQ_NO_FILE, O_RDWR | O_CREAT );
		if ( fdFile < 0 )
		{
			return 0;
		}

		nResult = read( fdFile, abBuf, sizeof( abBuf ) );
		// ���� READ
		if ( nResult == 4 )
		{
			wSeqNo = GetWORDFromASC( abBuf, 4 );
		}
		// ���� READ
		else
		{
			close( fdFile );
			return 0;
		}

		wSeqNo = ( wSeqNo + 1 ) % 5000;

		WORD2ASCWithFillLeft0( wSeqNo, abBuf, sizeof( abBuf ) );

		// ���ϰ��� �����Ѵ�.
		lseek( fdFile, 0L, SEEK_SET );

		nResult = write( fdFile, abBuf, sizeof( abBuf ) );
		if ( nResult != 4 )
		{
			close( fdFile );
			return 0;
		}

		close( fdFile );

		return wSeqNo;
	}
	// ���� �̸��� ���� �������� ����
	else
	{
		fdFile = open( BACKUP_TRANS_SEQ_NO_FILE, O_RDWR | O_CREAT );
		if ( fdFile < 0 )
		{
			return 0;
		}

		memset( abBuf, '0', sizeof( abBuf ) );

		nResult = write( fdFile, abBuf, sizeof( abBuf ) );
		if ( nResult != 4 )
		{
			close( fdFile );
			return 0;
		}

		close( fdFile );

		return 0;
	}

	return 0;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       GetCashTransSeqNo                                        *
*                                                                              *
*  DESCRIPTION:       ��ȸ��ID�Ϸù�ȣ�� "ticketseq.tmp" ���Ϸκ���            *
*                     1 �����Ͽ� �����´�.                                     *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: word -  ��ȸ��ID�Ϸù�ȣ                                 *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-30                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static word GetCashTransSeqNo( void )
{
	int fdFile = 0;
	word wSeqNo = 0;
	int nResult = 0;
	byte abBuf[5];

	// ���� �̸��� ���� ������
	if ( IsExistFile( CASH_GETON_SEQ_NO_FILE ) )
	{
		fdFile = open( CASH_GETON_SEQ_NO_FILE, O_RDWR | O_CREAT );
		if ( fdFile < 0 )
		{
			return 1;
		}

		nResult = read( fdFile, abBuf, sizeof( abBuf ) );
		// ���� READ
		if ( nResult == 5 )
		{
			wSeqNo = GetWORDFromASC( abBuf, 5 );
		}
		// ���� READ
		else
		{
			close( fdFile );
			return 1;
		}

		wSeqNo = ( wSeqNo + 1 ) % 100000;

		WORD2ASCWithFillLeft0( wSeqNo, abBuf, sizeof( abBuf ) );

		// ���ϰ��� �����Ѵ�.
		lseek( fdFile, 0L, SEEK_SET );

		nResult = write( fdFile, abBuf, sizeof( abBuf ) );
		if ( nResult != 5 )
		{
			close( fdFile );
			return 1;
		}

		close( fdFile );

		return wSeqNo;
	}
	// ���� �̸��� ���� �������� ����
	else
	{
		fdFile = open( CASH_GETON_SEQ_NO_FILE, O_RDWR | O_CREAT );
		if ( fdFile < 0 )
		{
			return 1;
		}

		// memset( abBuf, '0', sizeof( abBuf ) );
		memcpy( abBuf, "00001", sizeof( abBuf ) );

		nResult = write( fdFile, abBuf, sizeof( abBuf ) );
		if ( nResult != 5 )
		{
			close( fdFile );
			return 1;
		}

		close( fdFile );

		return 1;
	}

	return 1;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SetControlTrans                                          *
*                                                                              *
*  DESCRIPTION:       ������������ ����ü�� �ʱ�ȭ�Ѵ�.                        *
*                                                                              *
*  INPUT PARAMETERS:  pstControlTrans - ������������ ����ü ������             *
*                     abTransFileName - �ŷ��������ϸ�                         *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void SetControlTrans(CONTROL_TRANS *pstControlTrans,
	byte *abTransFileName)
{
	memset( ( byte * )pstControlTrans, 0, sizeof( CONTROL_TRANS ) );

	// 1. ���౸��
	pstControlTrans->bDriveNow = '1';

	// 2. �ѰǼ�
	memset( pstControlTrans->abTotalCnt, '0',
		sizeof( pstControlTrans->abTotalCnt ) );

	// 3. �ѱݾ�
	memset( pstControlTrans->abTotalAmt, '0',
		sizeof( pstControlTrans->abTotalAmt ) );

	// 4. TR���ϸ�
	memcpy( pstControlTrans->abTRFileName, abTransFileName,
		sizeof( pstControlTrans->abTRFileName ) );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SetTransHeader                                           *
*                                                                              *
*  DESCRIPTION:       ��������Ͻ� ���� ������ �̿��Ͽ� �ŷ����������         *
*                     �����Ѵ�.                                                *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTH - �����ϰ��� �ϴ� �ŷ�������� ����ü ������  *
*                     tStartDriveDtime - ��������Ͻ�                          *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void SetTransHeader( TRANS_TH *pstTransTH, time_t tStartDriveDtime )
{
	memset( ( byte * )pstTransTH, 0, sizeof( TRANS_TH ) );

	// 1. Recode ���� - 185
	memcpy( pstTransTH->abTHRecLen, LENGTH_OF_TRANS_HEADER,
		sizeof( pstTransTH->abTHRecLen ) );

	// 2. Record ���� - H
	pstTransTH->bTHRecType = RECORD_TYPE_TRANS_HEADER;

	// 3. �������� ID
	memcpy( pstTransTH->abTranspBizrID, gstVehicleParm.abTranspBizrID,
		sizeof( pstTransTH->abTranspBizrID ) );

	// 4. ������ ID
	memcpy( pstTransTH->abBizOfficeID, gstVehicleParm.abBusBizOfficeID,
		sizeof( pstTransTH->abBizOfficeID ) );

	// 5. ���� �뼱 ID
	memcpy( pstTransTH->abRouteID, gstVehicleParm.abRouteID,
		sizeof( pstTransTH->abRouteID ) );

	// 6. ��������ڵ�
	memcpy( pstTransTH->abTranspMethodCode, gstVehicleParm.abTranspMethodCode,
		sizeof( pstTransTH->abTranspMethodCode ) );

	// 7. ���� ID
	memcpy( pstTransTH->abVehicleID, gstVehicleParm.abVehicleID,
		sizeof( pstTransTH->abVehicleID ) );

	// 8. �ܸ��� ID
	memcpy( pstTransTH->abTermID, gpstSharedInfo->abMainTermID,
		sizeof( pstTransTH->abTermID ) );

	// 9. ������ ID
	memcpy( pstTransTH->abDriverID, gstVehicleParm.abDriverID,
		sizeof( pstTransTH->abDriverID ) );

	// 10. ����ý��� �����Ͻ� - ����ý������� ���۽� �ٽ� ������Ʈ��
	memset( pstTransTH->abDCSRecvDtime, '0',
		sizeof( pstTransTH->abDCSRecvDtime ) );

	// 11. ���������� ID
	memcpy( pstTransTH->abStartStationID, gpstSharedInfo->abNowStationID,
		sizeof( pstTransTH->abStartStationID ) );

	// 12. ���� ���� �Ͻ�
	TimeT2ASCDtime( tStartDriveDtime, pstTransTH->abStartDtime );

	// 13. �ܸ����������Ϸù�ȣ
	WORD2ASCWithFillLeft0( GetTermAggrSeqNo(), pstTransTH->abTermUseSeqNo,
		sizeof( pstTransTH->abTermUseSeqNo ) );

	// 14. File Name
	memset( pstTransTH->abFileName, 0, sizeof( pstTransTH->abFileName ) );

	// 15. Record Mac
	memset( pstTransTH->abRecMAC, '0', sizeof( pstTransTH->abRecMAC ) );

	// 16. ���� ����
	pstTransTH->abCRLF[0] = 0x0D;
	pstTransTH->abCRLF[1] = 0x0A;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SetTransData                                             *
*                                                                              *
*  DESCRIPTION:       ī����������ü�κ��� �ŷ����������͸� �����Ѵ�.          *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī������ ����ü�� ������                  *
*                     pstTransTD - �ŷ����������� ����ü�� ������              *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-29                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void SetTransData( TRANS_INFO *pstTransInfo, TRANS_TD *pstTransTD )
{
	byte abBuf[8] = {0, };
	dword dwCRC = 0;

	memset( ( byte * )pstTransTD, 0, sizeof( TRANS_TD ) );

	// 1. TD Record ����
	memcpy( pstTransTD->abTDRecLen, LENGTH_OF_TRANS_DATA,
		sizeof( pstTransTD->abTDRecLen ) );

	// 2. TD Record ����
	pstTransTD->bTDRecType = RECORD_TYPE_TRANS_DATA;

	// 3. ī������
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->bCardType = TRANS_CARDTYPE_SC_PREPAY;
	}
	else
	{
		pstTransTD->bCardType = pstTransInfo->bCardType;
	}

	// 4. ����������
	if ( IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
	{
		pstTransTD->bEntExtType = '0';				// ����
	}
	else
	{
		pstTransTD->bEntExtType = '1';				// ����
	}

	// 5. ī���ȣ/��ȸ��ID
	ASC2BCD( pstTransInfo->abCardNo, pstTransTD->abCardNo,
		sizeof( pstTransInfo->abCardNo ) );

	// 6. alias��ȣ
	pstTransTD->dwAliasNo = pstTransInfo->dwAliasNo;

	// 7. �������Ͻ�
	TimeT2BCDDtime( pstTransInfo->stNewXferInfo.tEntExtDtime,
		pstTransTD->abEntExtDtime );

	LogMain("�ŷ�1 : %lu\n", pstTransInfo->stNewXferInfo.tEntExtDtime);
	LogMain("�ŷ�2 : %02x%02x%02x%02x%02x%02x%02x\n",
		pstTransTD->abEntExtDtime[0],
		pstTransTD->abEntExtDtime[1],
		pstTransTD->abEntExtDtime[2],
		pstTransTD->abEntExtDtime[3],
		pstTransTD->abEntExtDtime[4],
		pstTransTD->abEntExtDtime[5],
		pstTransTD->abEntExtDtime[6]);

	// 8. ��������� - ī�� ����� ������ �־���
	pstTransTD->bUserType = pstTransInfo->bCardUserType;

	// 9. �°�1 ������������ID
	memcpy( pstTransTD->abDisExtraTypeID1, pstTransInfo->abDisExtraTypeID[0],
		sizeof( pstTransTD->abDisExtraTypeID1 ) );

	// 11. �°�2 ������������ID
	memcpy( pstTransTD->abDisExtraTypeID2, pstTransInfo->abDisExtraTypeID[1],
		sizeof( pstTransTD->abDisExtraTypeID2 ) );

	// 13. �°�3 ������������ID
	memcpy( pstTransTD->abDisExtraTypeID3, pstTransInfo->abDisExtraTypeID[2],
		sizeof( pstTransTD->abDisExtraTypeID3 ) );

	if ( IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
	{
		// 10. �°���1
		pstTransTD->wUserCnt1 = pstTransInfo->abUserCnt[0];

		// 12. �°���2
		pstTransTD->wUserCnt2 = pstTransInfo->abUserCnt[1];

		// 14. �°���3
		pstTransTD->wUserCnt3 = pstTransInfo->abUserCnt[2];
	}
	else
	{
		// 10. �°���1
		pstTransTD->wUserCnt1 =
			pstTransInfo->stPrevXferInfo.abMultiEntInfo[0][USER_CNT];

		// 12. �°���2
		pstTransTD->wUserCnt2 =
			pstTransInfo->stPrevXferInfo.abMultiEntInfo[1][USER_CNT];

		// 14. �°���3
		pstTransTD->wUserCnt3 =
			pstTransInfo->stPrevXferInfo.abMultiEntInfo[2][USER_CNT];
	}

	// 15. �г�Ƽ����
	if ( pstTransInfo->stNewXferInfo.wPrevPenaltyFare != 0 )
	{
		memcpy( pstTransTD->abPenaltyType, PENALTY_TYPE_NO_TAG_IN_EXT,
			sizeof( pstTransTD->abPenaltyType ) );
	}
	else
	{
		memset( pstTransTD->abPenaltyType, 0,
			sizeof( pstTransTD->abPenaltyType ) );
	}

	// 16. ���Ƽ���
	pstTransTD->dwPenaltyFare = pstTransInfo->stNewXferInfo.wPrevPenaltyFare;

	// 17. �˰�������
	pstTransTD->bSCAlgoriType = pstTransInfo->bSCAlgoriType;

	// 18. �ŷ�����
	pstTransTD->bSCTransType = pstTransInfo->bSCTransType;

	// 19. �����ŷ�����Ű����
	pstTransTD->bSCTransKeyVer = pstTransInfo->bSCTransKeyVer;

	// 20. ����ȭ���ID
	pstTransTD->bSCEpurseIssuerID = pstTransInfo->bSCEpurseIssuerID;

	// 21. ��������ID ( ī��ĺ��� )
	//     1) ������ī�� - ���ʻ���Ͻ�
	//     2) ������ī��/��ġ��ī�� - ������ī�� �����������ι�ȣ
	//     3) �� �� ī�� - ��������ID (���ĺ�ī��� ��� 0���� ����)
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		if ( pstTransInfo->tMifTourFirstUseDtime == 0 )
		{
			TimeT2BCDDtime( pstTransInfo->stNewXferInfo.tEntExtDtime,
				pstTransTD->abSCEpurseID );
		}
		else
		{
			TimeT2BCDDtime( pstTransInfo->tMifTourFirstUseDtime,
				pstTransTD->abSCEpurseID );
		}
	}
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
			  pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		ASC2BCD( pstTransInfo->abMifPrepayChargeAppvNop,
			pstTransTD->abSCEpurseID,
			sizeof( pstTransInfo->abMifPrepayChargeAppvNop ) );
	}
	else
	{
		ASC2BCD( pstTransInfo->abSCEpurseID, pstTransTD->abSCEpurseID,
			sizeof( pstTransInfo->abSCEpurseID ) );
	}

	// 22. ī��ŷ��Ǽ�
	//     1) ��Ƽ����������� ������ī�� - ����ȯ�������� �Ѵ������Ƚ��
	//     2) ������/���ĺ�/��ġ��/������ī�� - �ű�ȯ�������� �Ѵ������Ƚ��
	//     3) �ż�/�ĺ�ī�� - ī���� NTEP
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == TRUE &&
		 pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->dwSCTransCnt = pstTransInfo->stPrevXferInfo.wTotalAccEntCnt;
	}
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
			  pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT ||
			  pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_POSTPAY ||
			  pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->dwSCTransCnt = pstTransInfo->stNewXferInfo.wTotalAccEntCnt;
	}
	else
	{
		pstTransTD->dwSCTransCnt = pstTransInfo->dwSCTransCnt;
	}

	// 23. ī���ܾ�
	//     1) ������ī�� - �ŷ� �� �Ѵ������Ƚ��
	//     2) ��ġ��ī�� - 0
	//     3) �� �� ī�� - �ŷ� �� �ܾ�
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->dwBal = pstTransInfo->stNewXferInfo.dwMifTourTotalAccUseCnt;
	}
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		pstTransTD->dwBal = 0;
	}
	else
	{
		pstTransTD->dwBal = pstTransInfo->stNewXferInfo.dwBal;
	}

	// 24. ���
	pstTransTD->dwFare = pstTransInfo->stNewXferInfo.dwFare;

	// 25. SAM ID
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
		 pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT ||
		 pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_POSTPAY )
	{
		if ( pstTransInfo->boolIsTCard == TRUE )
		{
			ASC2BCD( gstMyTermInfo.abPSAMID, pstTransTD->abPSAMID,
				sizeof( gstMyTermInfo.abPSAMID ) );
		}
		else
		{
			memset( abBuf, '0', sizeof( abBuf ) );
			memcpy( abBuf, gstMyTermInfo.abISAMID,
				sizeof( gstMyTermInfo.abISAMID ) );
			memset( pstTransTD->abPSAMID, 0, sizeof( pstTransTD->abPSAMID ) );
			ASC2BCD( abBuf, pstTransTD->abPSAMID, 8 );
		}
	}
	else
	{
		ASC2BCD( pstTransInfo->abPSAMID, pstTransTD->abPSAMID,
			sizeof( pstTransInfo->abPSAMID ) );
	}

	// 26. SAM�ŷ�ī����
	//     1) ������ī�� - �̹� ��/������ ���� Ƚ��
	//     2) ������ī��/��ġ��ī�� - TCC �� 4B
	//     3) �� �� ī�� - SAM�ŷ�ī����
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->dwPSAMTransCnt = pstTransInfo->bMifTourUseCnt;
	}
	else if ( ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
				pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT ) &&
			  pstTransInfo->boolIsTCard == FALSE )
	{
		memcpy( ( byte * )&pstTransTD->dwPSAMTransCnt,
			pstTransInfo->abMifPrepayTCC,
			sizeof( pstTransTD->dwPSAMTransCnt ) );
	}
	else
	{
		pstTransTD->dwPSAMTransCnt = pstTransInfo->dwPSAMTransCnt;
	}

	// 27. SAM�Ѿװŷ�����ī����
	//     1) ������ī�� - ������ī������
	//     2) ������ī��/��ġ��ī�� - TCC �� 4B
	//     3) �� �� ī�� - SAM�Ѿװŷ�����ī����
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->dwPSAMTotalTransCnt = pstTransInfo->wMifTourCardType;
	}
	else if ( ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
				pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT ) &&
			  pstTransInfo->boolIsTCard == FALSE )
	{
		memcpy( ( byte * )&pstTransTD->dwPSAMTotalTransCnt,
			&pstTransInfo->abMifPrepayTCC[4],
			sizeof( pstTransTD->dwPSAMTotalTransCnt ) );
	}
	else
	{
		pstTransTD->dwPSAMTotalTransCnt = pstTransInfo->dwPSAMTotalTransCnt;
	}

	// 28. SAM�����ŷ������Ǽ�
	pstTransTD->wPSAMIndvdTransCnt = pstTransInfo->wPSAMIndvdTransCnt;

	// 29. SAM�����ŷ��Ѿ�
	pstTransTD->dwPSAMAccTransAmt = pstTransInfo->dwPSAMAccTransAmt;

	// 30. SAM����
	if ( pstTransInfo->boolIsTCard == TRUE )
	{
		memcpy( pstTransTD->abPSAMSign, pstTransInfo->abMifPrepayTCC,
			sizeof( pstTransTD->abPSAMSign ) );
	}
	else
	{
		pstTransTD->abPSAMSign[0] = pstTransInfo->abPSAMSign[3];
		pstTransTD->abPSAMSign[1] = pstTransInfo->abPSAMSign[2];
		pstTransTD->abPSAMSign[2] = pstTransInfo->abPSAMSign[1];
		pstTransTD->abPSAMSign[3] = pstTransInfo->abPSAMSign[0];
	}

	// 31. ȯ���Ϸù�ȣ
	//     1) ��Ƽ������� - "300"
	//     2) �� �� ������� - ȯ���Ϸù�ȣ
	if ( IsCityTourBus( gstVehicleParm.wTranspMethodCode ) == TRUE )
	{
		memcpy( pstTransTD->abXferSeqNo, "300",
			sizeof( pstTransTD->abXferSeqNo ) );
	}
	else
	{
		WORD2ASCWithFillLeft0( ( word )pstTransInfo->stNewXferInfo.bXferSeqNo,
			pstTransTD->abXferSeqNo, sizeof( pstTransTD->abXferSeqNo ) );
	}

	// 32. �°�1 �����ŷ��� ���ҵ� �ִ�⺻���
	pstTransTD->dwPrevMaxBaseFare1 =
		pstTransInfo->stPrevXferInfo.awMaxBaseFare[0];

	// 33. �ŷ����ܾ� (�°�2 �����ŷ��� ���ҵ� �ִ�⺻���)
	//     1) ������ī�� - �ŷ� �� �Ѵ������Ƚ��
	//     2) ��ġ��ī�� - 0
	//     3) �� �� ī�� - �ŷ� �� �ܾ�
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_TOUR )
	{
		pstTransTD->dwPrevMaxBaseFare2 =
			pstTransInfo->stPrevXferInfo.dwMifTourTotalAccUseCnt;
	}
	else if ( pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		pstTransTD->dwPrevMaxBaseFare2 = 0;
	}
	else
	{
		pstTransTD->dwPrevMaxBaseFare2 =
			pstTransInfo->stPrevXferInfo.dwBal;
	}

	// 34. �°�3 �����ŷ��� ���ҵ� �ִ�⺻���
	//	  - 4����Ʈ�� ������ ������ ������ ����ϵ��� �����ϰ� ����
	//		 [0] : ��õȯ���÷���
	//		 [1] : �����Ϻ� : GPS�����÷���, �����Ϻ� : �߰������÷���
	//		 [2] : ī�庹���α�
	//		 [3] : ȯ�¹�ó���α�

	// [0] : ��õȯ���÷���
	if ( pstTransInfo->stNewXferInfo.bEntExtType == XFER_ENT_AFTER_INCHEON ||
		 pstTransInfo->stNewXferInfo.bEntExtType == XFER_EXT_AFTER_INCHEON )
	{
		pstTransTD->abPrevMaxBaseFare3[0] = 0x01;
	}
	// [1] �����Ϻ� : GPS�����÷���
	//		 - 0x00 : ����
	//		 - 0x01 : �����̻�
	//		 - 0x02 : ���Ŵ���
	//		 - 0x03 : �Ÿ��̻�
	pstTransTD->abPrevMaxBaseFare3[1] = gpstSharedInfo->gbGPSStatusFlag << 4;
	
	// [1] �����Ϻ� : �߰�����/�����庸��/�����ð����� �÷���
	//        7 6 5 4 3 2 1 0
	//        -------   | | |
	//        GPS����   | | �߰����� �÷��� (�����ܸ��⿡���� �߻�)
	//                  | �����庸�� �÷��� (�����ܸ��⿡���� �߻�)
	//                  �����ð����� �÷���

	// �����ð��� ������ ���
	if ( pstTransInfo->boolIsAdjustExtDtime == TRUE )
	{
		pstTransTD->abPrevMaxBaseFare3[1] |= 0x04;		// 0000 0100
	}

	// �����ܸ����� ���
	if ( gboolIsMainTerm == TRUE )
	{
		// �������� ������ ���
		if ( memcmp( gpstSharedInfo->abNowStationID, gabGPSStationID,
				sizeof( gpstSharedInfo->abNowStationID ) ) != 0 )
		{
			pstTransTD->abPrevMaxBaseFare3[1] |= 0x02;	// 0000 0010
		}

		// �߰������� ���
		if ( pstTransInfo->boolIsAddEnt == TRUE )
		{
			pstTransTD->abPrevMaxBaseFare3[1] |= 0x01;	// 0000 0001
		}
	}
	// [2] : ī�庹���α�
	pstTransTD->abPrevMaxBaseFare3[2] = pstTransInfo->bWriteErrCnt;
	// [3] : ȯ�¹�ó���α�
	pstTransTD->abPrevMaxBaseFare3[3] = pstTransInfo->bNonXferCause;

	// 35. ȯ�³��̿���ܱ⺻�������
	pstTransTD->dwTotalBaseFareInXfer =
		pstTransInfo->stNewXferInfo.wTotalBaseFareInXfer;

	// 36. ȯ�´���Ƚ��
	pstTransTD->wXferCnt = pstTransInfo->stNewXferInfo.bAccXferCnt;

	// 37. ���Ÿ�
	pstTransTD->dwDist = pstTransInfo->dwDist;

	// 38. ȯ�³������̵��Ÿ�
	pstTransTD->dwAccDistInXfer = pstTransInfo->stNewXferInfo.dwAccDistInXfer;

	// 39. ȯ�³������̿�ݾ�
	pstTransTD->dwAccUseAmtInXfer = pstTransInfo->stNewXferInfo.dwAccAmtInXfer;

	// 40. �Ѵ������ݾ�
	pstTransTD->dwTotalAccAmt = pstTransInfo->stNewXferInfo.dwTotalAccUseAmt;

	// 41. ������ID
	memset( abBuf, '0', sizeof( abBuf ) );
	memcpy( abBuf, pstTransInfo->stNewXferInfo.abStationID,
		sizeof( pstTransInfo->stNewXferInfo.abStationID ) );
	ASC2BCD( abBuf, pstTransTD->abStationID, 8 );

	// 42. �������Ƽ���
	pstTransTD->dwPrevPenaltyFare =
		pstTransInfo->stPrevXferInfo.wPrevPenaltyFare;

	// 43. �����ý����ݾ�
	if ( IsEnt( pstTransInfo->stNewXferInfo.bEntExtType ) )
	{
		pstTransTD->dwPrevEntFare = 0;
	}
	else
	{
		pstTransTD->dwPrevEntFare = pstTransInfo->stPrevXferInfo.dwFare;
	}

	// 44. ����������ID
	memset( abBuf, '0', sizeof( abBuf ) );
	memcpy( abBuf, pstTransInfo->stPrevXferInfo.abStationID,
		sizeof( pstTransInfo->stPrevXferInfo.abStationID ) );
	ASC2BCD( abBuf, pstTransTD->abPrevStationID, 8 );

	// 45. ���������������
	memset( abBuf, '0', sizeof( abBuf ) );
	// ��õȯ�� ���Ƽ�� �߻��� ���
	if ( !IsSeoulTransp( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) &&
		 pstTransInfo->stPrevXferInfo.wIncheonPenaltyPrevTMCode != 0 &&
		 pstTransInfo->stNewXferInfo.wPrevPenaltyFare != 0 )
	{
		WORD2ASCWithFillLeft0(
			pstTransInfo->stPrevXferInfo.wIncheonPenaltyPrevTMCode, abBuf, 3 );
	}
	// ��õȯ�� ���Ƽ�� �߻����� ���� ���
	else
	{
		WORD2ASCWithFillLeft0( pstTransInfo->stPrevXferInfo.wTranspMethodCode,
			abBuf, 3 );
	}
	ASC2BCD( abBuf, pstTransTD->abPrevTranspMethodCode, 4 );

	// 46. �����������Ͻ�
	// ��õȯ�� ���Ƽ�� �߻��� ���
	if ( !IsSeoulTransp( pstTransInfo->stPrevXferInfo.wTranspMethodCode ) &&
		 pstTransInfo->stPrevXferInfo.wIncheonPenaltyPrevTMCode != 0 &&
		 pstTransInfo->stNewXferInfo.wPrevPenaltyFare != 0 )
	{
		TimeT2BCDDtime( pstTransInfo->stPrevXferInfo.tIncheonPenaltyPrevDtime,
			pstTransTD->abPrevEntExtDtime );
	}
	// ��õȯ�� ���Ƽ�� �߻����� ���� ���
	else
	{
		TimeT2BCDDtime( pstTransInfo->stPrevXferInfo.tEntExtDtime,
			pstTransTD->abPrevEntExtDtime );
	}

	// 47. ������ī���ܾ�
	pstTransTD->dwBalAfterCharge = pstTransInfo->dwBalAfterCharge;

	// 48. ������ī��ŷ��Ǽ�
	pstTransTD->dwChargeTransCnt = pstTransInfo->dwChargeTransCnt;

	// 49. �����ݾ�
	pstTransTD->dwChargeAmt = pstTransInfo->dwChargeAmt;

	// 50. ������SAM ID
	ASC2BCD( pstTransInfo->abLSAMID, pstTransTD->abLSAMID,
		sizeof( pstTransInfo->abLSAMID ) );

	// 51. ������SAM�ŷ��Ϸù�ȣ
	pstTransTD->dwLSAMTransCnt = pstTransInfo->dwLSAMTransCnt;

	// 52. �����ŷ�����
	pstTransTD->bChargeTransType = pstTransInfo->bChargeTransType;

	// 53. Record Mac
	dwCRC = MakeCRC32( ( byte * )pstTransTD, sizeof( TRANS_TD ) - 7 );
	memcpy( pstTransTD->abRecMAC, ( byte * )&dwCRC,
		sizeof( pstTransTD->abRecMAC ) );

	// 54. SIGN���������
	switch ( pstTransInfo->bCardType )
	{
		case TRANS_CARDTYPE_MIF_PREPAY:
		case TRANS_CARDTYPE_MIF_POSTPAY:
		case TRANS_CARDTYPE_DEPOSIT:
			pstTransTD->abVerifySignVal = CHECK_SAM_SIGN_PASS;
			break;
		default:
			pstTransTD->abVerifySignVal = CHECK_SAM_SIGN_FAIL;
	}

	// 55. ���๮��
	pstTransTD->abCRLF[0] = 0x0D;
	pstTransTD->abCRLF[1] = 0x0A;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SetCashTransData                                         *
*                                                                              *
*  DESCRIPTION:       ��������� ������ �̿��Ͽ� ���� �ŷ�������               *
*                     �ŷ���������ü�� �����Ѵ�.                               *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTD - �����ϰ��� �ϴ� �ŷ����������� ����ü ������*
*                     bUserType - ���� ����� ����                             *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void SetCashTransData( TRANS_TD *pstTransTD, byte bUserType )
{
	word wSeqNo = 0;
	dword dwCRC = 0;
	time_t tNowDtime = 0;
	byte abBuf[8] = {0, };
	byte abASCNowDtime[14] = {0, };
	byte abTempASCCardNo[20] = {0, };

	GetRTCTime( &tNowDtime );

	memset( ( byte * )pstTransTD, 0, sizeof( TRANS_TD ) );

	// 1. TD Record ����
	memcpy( pstTransTD->abTDRecLen, LENGTH_OF_TRANS_DATA,
		sizeof( pstTransTD->abTDRecLen ) );

	// 2. TD Record ����
	pstTransTD->bTDRecType = RECORD_TYPE_TRANS_DATA;

	// 3. ī������
	pstTransTD->bCardType = TRANS_CARDTYPE_CASH;

	// 4. ����������
	pstTransTD->bEntExtType = '0';				// ����

	// 5. ī���ȣ/��ȸ��ID
	//     - 9B : �����ܸ���ID
	//     - 6B : ����ð� (hhmmss)
	//     - 5B : ��ȸ��ID �Ϸù�ȣ
	memset( abTempASCCardNo, 0, sizeof( abTempASCCardNo ) );
	wSeqNo = GetCashTransSeqNo();
	TimeT2ASCDtime( tNowDtime, abASCNowDtime );
	memcpy( abTempASCCardNo, gpstSharedInfo->abMainTermID, 9 );
	memcpy( &abTempASCCardNo[9], &abASCNowDtime[8], 6 );
	WORD2ASCWithFillLeft0( wSeqNo, &abTempASCCardNo[15], 5 );
	ASC2BCD( abTempASCCardNo, pstTransTD->abCardNo, 20 );

	// 6. alias��ȣ

	// 7. �������Ͻ�
	TimeT2BCDDtime( tNowDtime, pstTransTD->abEntExtDtime );

	// 8. ���������
	pstTransTD->bUserType = bUserType;

	// 9. �°�1 ������������ID
	CreateDisExtraTypeID( pstTransTD->bCardType, bUserType, tNowDtime, FALSE,
		pstTransTD->abDisExtraTypeID1 );

	// 10. �°���1
	pstTransTD->wUserCnt1 = 1;

	// 11. �°�2 ������������ID

	// 12. �°���2

	// 13. �°�3 ������������ID

	// 14. �°���3

	// 15. �г�Ƽ����

	// 16. ���Ƽ���

	// 17. �˰�������

	// 18. �ŷ�����

	// 19. �����ŷ�����Ű����

	// 20. ����ȭ���ID

	// 21. ��������ID

	// 22. ī��ŷ��Ǽ�

	// 23. ī���ܾ�

	// 24. ���
	pstTransTD->dwFare = GetBaseFare( pstTransTD->bCardType, bUserType,
		gstVehicleParm.wTranspMethodCode );

	// 25. SAM ID

	// 26. SAM�ŷ�ī����

	// 27. SAM�Ѿװŷ�����ī����

	// 28. SAM�����ŷ������Ǽ�

	// 29. SAM�����ŷ��Ѿ�

	// 30. SAM����

	// 31. ȯ���Ϸù�ȣ
	memcpy( pstTransTD->abXferSeqNo, "001", 3 );

	// 32. �°�1 �����ŷ��� ���ҵ� �ִ�⺻���

	// 33. �°�2 �����ŷ��� ���ҵ� �ִ�⺻��� - RFU

	// 34. �°�3 �����ŷ��� ���ҵ� �ִ�⺻��� - �ŷ��α�

	// 35. ȯ�³��̿���ܱ⺻�������

	// 36. ȯ�´���Ƚ��

	// 37. ���Ÿ�

	// 38. ȯ�³������̵��Ÿ�

	// 39. ȯ�³������̿�ݾ�

	// 40. �Ѵ������ݾ�

	// 41. ������ID
	memset( abBuf, '0', sizeof( abBuf ) );
	memcpy( abBuf, gpstSharedInfo->abNowStationID,
		sizeof( gpstSharedInfo->abNowStationID ) );
	ASC2BCD( abBuf, pstTransTD->abStationID, 8 );

	// 42. �������Ƽ���

	// 43. �����ý����ݾ�

	// 44. ����������ID

	// 45. ���������������

	// 46. �����������Ͻ�

	// 47. ������ī���ܾ�

	// 48. ������ī��ŷ��Ǽ�

	// 49. �����ݾ�

	// 50. ������SAM ID

	// 51. ������SAM�ŷ��Ϸù�ȣ

	// 52. �����ŷ�����

	// 53. Record Mac
	dwCRC = MakeCRC32( ( byte * )pstTransTD, sizeof( TRANS_TD ) - 7 );
	memcpy( pstTransTD->abRecMAC, ( byte * )&dwCRC,
		sizeof( pstTransTD->abRecMAC ) );

	// 54. SIGN���������
	switch ( pstTransTD->bCardType )
	{
		case TRANS_CARDTYPE_MIF_PREPAY:
		case TRANS_CARDTYPE_MIF_POSTPAY:
		case TRANS_CARDTYPE_DEPOSIT:
		case TRANS_CARDTYPE_CASH:
		case TRANS_CARDTYPE_STUDENT_TOKEN:
			pstTransTD->abVerifySignVal = CHECK_SAM_SIGN_PASS;
			break;
		default:
			pstTransTD->abVerifySignVal = CHECK_SAM_SIGN_FAIL;
	}

	// 55. ���๮��
	pstTransTD->abCRLF[0] = 0x0D;
	pstTransTD->abCRLF[1] = 0x0A;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SetTransTail                                             *
*                                                                              *
*  DESCRIPTION:       �ѰǼ� �� �ѱݾ�, ���������Ͻ� ���� ������ �̿��Ͽ�      *
*                     �ŷ����������� �����Ѵ�.                                 *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTT - �����ϰ��� �ϴ� �ŷ��������� ����ü ������  *
*                     dwTotalCnt - �ѰǼ�                                      *
*                     dwTotalAmt - �ѱݾ�                                      *
*                     tEndDriveDtime - ���������Ͻ�                            *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static void SetTransTail( TRANS_TT *pstTransTT, dword dwTotalCnt,
	dword dwTotalAmt, time_t tEndDriveDtime )
{
	dword dwCRC = 0;
	byte abBuf[7] = {0, };
	byte abASCDtime[14] = {0, };

	memset( ( byte * )pstTransTT, 0, sizeof( TRANS_TT ) );

	TimeT2ASCDtime( tEndDriveDtime, abASCDtime );

	// 1. Record ����
	memcpy( pstTransTT->abTTRecLen, LENGTH_OF_TRANS_TAIL,
		sizeof( pstTransTT->abTTRecLen ) );

	// 2. Record����
	pstTransTT->bTTRecType = RECORD_TYPE_TRANS_TAIL;

	// 3. File Name
	memcpy( &pstTransTT->abFileName[0], "B001.0", 6 );		// �����׸��
	memcpy( &pstTransTT->abFileName[6], ".", 1 );
	memcpy( &pstTransTT->abFileName[7], gstVehicleParm.abTranspBizrID,
		sizeof( gstVehicleParm.abTranspBizrID ) );
															// ��������ID
	memcpy( &pstTransTT->abFileName[16], ".", 1 );
	memcpy( &pstTransTT->abFileName[17], gpstSharedInfo->abMainTermID,
		sizeof( gpstSharedInfo->abMainTermID ) );
															// �ܸ���ID
	memcpy( &pstTransTT->abFileName[26], ".", 1 );
	memcpy( &pstTransTT->abFileName[27], gpstSharedInfo->abTransFileName, 14 );
															// �����Ͻ�
	memcpy( &pstTransTT->abFileName[41], ".", 1 );
	memcpy( &pstTransTT->abFileName[42], "C", 1 );			// ������
	memset( &pstTransTT->abFileName[43], 0, 48 );

	// 4. �ѰǼ�
	DWORD2ASCWithFillLeft0( dwTotalCnt, pstTransTT->abTotalCnt, 9 );

	// 5. �ѱݾ�
	DWORD2ASCWithFillLeft0( dwTotalAmt, pstTransTT->abTotalAmt, 10 );

	// 6. ���� ���� �Ͻ�
	memcpy( pstTransTT->abEndDtime, abASCDtime,
		sizeof( pstTransTT->abEndDtime ) );

	// 7. ȸ���� ID
	// �������νķ� 3�ڸ�
	WORD2ASCWithFillLeft0( gwGPSRecvRate, abBuf, 3 );
	memcpy( &pstTransTT->abReturnStationID[0], abBuf, 3 );
	// 'ī�带 �ٽ� ���ּ���' �߻� Ƚ�� 4�ڸ�
	WORD2ASCWithFillLeft0( gwRetagCardCnt, abBuf, 4 );
	memcpy( &pstTransTT->abReturnStationID[3], abBuf, 4 );

	// 8. ȸ���ð�
	memset( pstTransTT->abReturnDtime, '0',
		sizeof( pstTransTT->abEndStationID ) );
	// ����Ƚ�� 3�ڸ�
	WORD2ASCWithFillLeft0( gwDriveCnt, abBuf, 3 );
	memcpy( &pstTransTT->abReturnDtime[0], abBuf, 3 );
	// ����Ÿ� 7�ڸ�
	DWORD2ASCWithFillLeft0( gdwDistInDrive, abBuf, 7 );
	memcpy( &pstTransTT->abReturnDtime[3], abBuf, 7 );
	// ���α׷� ���� 4�ڸ�
	memcpy( &pstTransTT->abReturnDtime[10], MAIN_RELEASE_VER, 4 );

	// 9. ���������� ID
	memcpy( pstTransTT->abEndStationID, gpstSharedInfo->abNowStationID,
		sizeof( pstTransTT->abEndStationID ) );

	// 10. Record MAC
	dwCRC = MakeCRC32( ( byte * )pstTransTT, sizeof( TRANS_TT ) - 6 );
	memcpy( pstTransTT->abRecMAC, ( byte * )&dwCRC,
		sizeof( pstTransTT->abRecMAC ) );

	// 11. ���๮��
	pstTransTT->abCRLF[0] = 0x0D;
	pstTransTT->abCRLF[1] = 0x0A;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileRenameTransFile                                      *
*                                                                              *
*  DESCRIPTION:       �ŷ��������ϸ��� RENAME�Ѵ�.                             *
*                                                                              *
*  INPUT PARAMETERS:  abOldTransFileName - RENAME�ϰ��� �ϴ� �ŷ��������ϸ�    *
*                     abNewTransFileName - RENAME ��� �ŷ��������ϸ�          *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_RENAME_TRANS                          *
*                         - �ŷ��������ϸ� RENAME ����                         *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileRenameTransFile( byte *abOldTransFileName,
	byte *abNewTransFileName )
{
	byte i = 0;
	int nResult = 0;

	for ( i = 0; i < 3; i++ )
	{
		/*
		 * rename�� ���ϰ� -> 0: ����, -1: ����
		 */
		nResult = rename( abOldTransFileName, abNewTransFileName );
		if ( nResult == 0 )
			break;
	}
	if ( nResult != 0 )
	{
		return ERR_CARD_PROC_FILE_RENAME_TRANS;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileReadControlTrans                                     *
*                                                                              *
*  DESCRIPTION:       �������������� �о� �� ����� CONTROL_TRANS ����ü       *
*                     �������� �����Ѵ�.                                       *
*                                                                              *
*  INPUT PARAMETERS:  pstControlTrans - �������������� ������ ��� ������      *
*                         CONTROL_TRANS ����ü�� ������                        *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS                    *
*                         - ������������ OPEN ����                             *
*                     ERR_CARD_PROC_FILE_READ_CONTROL_TRANS                    *
*                         - ������������ READ ����                             *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileReadControlTrans( CONTROL_TRANS *pstControlTrans )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	memset( ( byte * )pstControlTrans, 0, sizeof( CONTROL_TRANS ) );

	fdFile = fopen( CONTROL_TRANS_FILE, "rb+" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS;
		goto FINALLY;
	}

	nResult = fread( ( byte * )pstControlTrans, sizeof( CONTROL_TRANS ), 1,
		fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_READ_CONTROL_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileReadTransHeader                                      *
*                                                                              *
*  DESCRIPTION:       �ŷ��������Ϸκ��� �ŷ���������� �о� TRANS_TH ����ü
                      �������� �����Ѵ�.                                       *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTH - �ŷ��������Ϸκ��� READ�� TRNAS_TH ����ü   *
*                     abTransFileName - �ŷ��������ϸ�                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_TRANS                            *
*                         - �ŷ��������� OPEN ����                             *
*                     ERR_CARD_PROC_FILE_READ_TRANS                            *
*                         - �ŷ��������� READ ����                             *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileReadTransHeader( TRANS_TH *pstTransTH, byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( abTransFileName, "rb" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TRANS;
		goto FINALLY;
	}

	nResult = fread( ( byte * )pstTransTH, sizeof( TRANS_TH ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_READ_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileReadTransDataWithFD                                  *
*                                                                              *
*  DESCRIPTION:       �ŷ��������Ϸκ��� �ε����� �ش��ϴ�                     *
*                     �ŷ����������͸� �о� TRANS_TD ����ü �������� �����Ѵ�. *
*                                                                              *
*  INPUT PARAMETERS:  fdFile - �ŷ����������� ���ϵ�ũ����                   *
*                     pstTransTD - �ŷ��������Ϸκ��� READ�� TRNAS_TD ����ü   *
*                     dwIndex - READ�ϰ��� �ϴ� �ŷ������������� �ε���        *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_READ_TRANS                            *
*                         - �ŷ��������� READ ����                             *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileReadTransDataWithFD( FILE *fdFile, TRANS_TD *pstTransTD,
	dword dwIndex )
{
	int nResult = 0;

	fseek( fdFile, ( sizeof( TRANS_TH ) + sizeof( TRANS_TD ) * dwIndex ),
		SEEK_SET );

	nResult = fread( ( byte * )pstTransTD, sizeof( TRANS_TD ), 1, fdFile );
	if ( nResult != 1 )
	{
		return ERR_CARD_PROC_FILE_READ_TRANS;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileReadTransDataWithoutTransHeaderWithFD                *
*                                                                              *
*  DESCRIPTION:       �ŷ���������� ���� �ŷ��������Ϸκ��� �ε����� �ش��ϴ� *
*                     �ŷ����������͸� �о� TRANS_TD ����ü �������� �����Ѵ�. *
*                                                                              *
*  INPUT PARAMETERS:  fdFile                                                   *
*                         - �ŷ���������� ���� �ŷ����������� ���ϵ�ũ����  *
*                     pstTransTD - �ŷ��������Ϸκ��� READ�� TRNAS_TD ����ü   *
*                     dwIndex - READ�ϰ��� �ϴ� �ŷ������������� �ε���        *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_READ_TRANS                            *
*                         - �ŷ��������� READ ����                             *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileReadTransDataWithoutTransHeaderWithFD( FILE *fdFile,
	TRANS_TD *pstTransTD, dword dwIndex )
{
	int nResult = 0;

	fseek( fdFile, ( sizeof( TRANS_TD ) * dwIndex ), SEEK_SET );

	nResult = fread( ( byte * )pstTransTD, sizeof( TRANS_TD ), 1, fdFile );
	if ( nResult != 1 )
	{
		return ERR_CARD_PROC_FILE_READ_TRANS;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileReadTransTail                                        *
*                                                                              *
*  DESCRIPTION:       �ŷ��������Ͽ��� �ŷ����������� �о� TRANS_TT ����ü     *
*                     �������� �����Ѵ�.                                       *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTT - �ŷ��������Ϸκ��� READ�� TRANS_TT ����ü   *
*                     abTransFileName - �ŷ��������ϸ�                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_TRANS                            *
*                         - �ŷ��������� OPEN ����                             *
*                     ERR_CARD_PROC_FILE_READ_TRANS                            *
*                         - �ŷ��������� READ ����                             *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileReadTransTail( TRANS_TT *pstTransTT, byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( abTransFileName, "rb" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TRANS;
		goto FINALLY;
	}

	fseek( fdFile, ( -1 * sizeof( TRANS_TT ) ), SEEK_END );

	nResult = fread( ( byte * )pstTransTT, sizeof( TRANS_TT ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_READ_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileWriteControlTrans                                    *
*                                                                              *
*  DESCRIPTION:       �����������Ͽ� CONTROL_TRANS ����ü�� ������ �űԷ�      *
*                     WRITE�Ѵ�. (�����Ѵٸ�) ������ �������������� �����ȴ�.  *
*                                                                              *
*  INPUT PARAMETERS:  pstControlTrans - �����������Ͽ� WRITE�� CONTROL_TRANS   *
*                         ����ü                                               *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS                    *
*                         - ������������ OPEN ����                             *
*                     ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS                   *
*                         - ������������ WRITE ����                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileWriteControlTrans( CONTROL_TRANS *pstControlTrans )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( CONTROL_TRANS_FILE, "wb" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS;
		goto FINALLY;
	}

	nResult = fwrite( ( byte * )pstControlTrans, sizeof( CONTROL_TRANS ), 1,
		fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileWriteTransHeader                                     *
*                                                                              *
*  DESCRIPTION:       �ŷ���������� �־��� �̸��� �ŷ��������Ͽ� ����Ѵ�.    *
*                     ��, ������ �űԷ� �����ȴ�.                              *
*                     append�Ѵ�.                                              *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTH - �ŷ��������                                *
*                     abTransFileName - �ŷ��������ϸ�                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_TRANS                            *
*                         - �ŷ��������� OPEN ����                             *
*                     ERR_CARD_PROC_FILE_WRITE_TRANS                           *
*                         - �ŷ��������� WRITE ����                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileWriteTransHeader( TRANS_TH *pstTransTH, byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( abTransFileName, "wb" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TRANS;
		goto FINALLY;
	}

	nResult = fwrite( ( byte * )pstTransTH, sizeof( TRANS_TH ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileAppendTransData                                      *
*                                                                              *
*  DESCRIPTION:       �ŷ����������͸� �־��� �̸��� �ŷ��������Ͽ�            *
*                     append�Ѵ�.                                              *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTD - �ŷ�����������                              *
*                     abTransFileName - �ŷ��������ϸ�                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_TRANS                            *
*                         - �ŷ��������� OPEN ����                             *
*                     ERR_CARD_PROC_FILE_WRITE_TRANS                           *
*                         - �ŷ��������� WRITE ����                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileAppendTransData( TRANS_TD *pstTransTD, byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( abTransFileName, "ab" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TRANS;
		goto FINALLY;
	}

	nResult = fwrite( ( byte * )pstTransTD, sizeof( TRANS_TD ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileAppendTransDataWithFD                                *
*                                                                              *
*  DESCRIPTION:       �ŷ����������͸� �־��� ���ϵ�ũ���Ϳ� ����Ѵ�. ��,   *
*                     ���� �������� �̵��� ���� �����Ƿ� �Լ���ó�� append��   *
*                     �����ϱ� ���ؼ��� ���������Ͱ� ������ ���� ��������      *
*                     �ִٴ� ������ �����Ǿ�� �Ѵ�.                           *
*                                                                              *
*  INPUT PARAMETERS:  fdFile - �ŷ����������� ���ϵ�ũ����                   *
*                     pstTransTD - �ŷ�����������                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_WRITE_TRANS                           *
*                         - �ŷ��������� WRITE ����                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileAppendTransDataWithFD( FILE *fdFile, TRANS_TD *pstTransTD )
{
	int nResult = 0;

	nResult = fwrite( ( byte * )pstTransTD, sizeof( TRANS_TD ), 1, fdFile );
	if ( nResult != 1 )
	{
		return ERR_CARD_PROC_FILE_WRITE_TRANS;
	}

	return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileAppendSubTransData                                   *
*                                                                              *
*  DESCRIPTION:       203B�� ������ �ŷ����������� (�ŷ����������� 202B +      *
*                     ���ۿ��� 1B)�� �ŷ��������Ͽ� APPEND�Ѵ�.                *
*                                                                              *
*  INPUT PARAMETERS:  abSubTransTD - �ŷ��������Ͽ� APPEND��                   *
*                         �ŷ����������� (203B ����Ʈ �迭)                    *
*                     abTransFileName - �ŷ��������ϸ�                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_SUB_TRANS                        *
*                         - �����ŷ��������� OPEN ����                         *
*                     ERR_CARD_PROC_FILE_WRITE_SUB_TRANS                       *
*                         - �����ŷ��������� WRITE ����                        *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileAppendSubTransData( byte *abSubTransTD, byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( abTransFileName, "ab" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_SUB_TRANS;
		goto FINALLY;
	}

	nResult = fwrite( abSubTransTD, sizeof( TRANS_TD ) + 1, 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_SUB_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_SUB_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileAppendTransTail                                      *
*                                                                              *
*  DESCRIPTION:       �ŷ��������Ͽ� �ŷ����������� APPEND�Ѵ�.                *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTT - �ŷ��������Ͽ� APPEND�� �ŷ��������ϱ���ü  *
*                     abTransFileName - �ŷ��������ϸ�                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_TRANS                            *
*                         - �ŷ��������� OPEN ����                             *
*                     ERR_CARD_PROC_FILE_WRITE_TRANS                           *
*                         - �ŷ��������� WRITE ����                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileAppendTransTail( TRANS_TT *pstTransTT, byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	FILE *fdFile = NULL;

	fdFile = fopen( abTransFileName, "ab" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TRANS;
		goto FINALLY;
	}

	nResult = fwrite( ( byte * )pstTransTT, sizeof( TRANS_TT ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileUpdateControlTrans                                   *
*                                                                              *
*  DESCRIPTION:       �������������� �о� �ѰǼ� �� �ѱݾ��� ������Ʈ�� ��     *
*                     �ٽ� WRITE�Ѵ�.                                          *
*                                                                              *
*  INPUT PARAMETERS:  dwFare - �ѱݾ׿� ���� �ݾ�                              *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS                    *
*                         - ������������ OPEN ����                             *
*                     ERR_CARD_PROC_FILE_READ_CONTROL_TRANS                    *
*                         - ������������ READ ����                             *
*                     ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS                   *
*                         - ������������ WRITE ����                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileUpdateControlTrans( dword dwFare )
{
	short sResult = SUCCESS;
	int nResult = 0;
	dword dwTotalCnt = 0;
	dword dwTotalAmt = 0;
	FILE *fdFile = NULL;
	CONTROL_TRANS stControlTrans;

	fdFile = fopen( CONTROL_TRANS_FILE, "rb+" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_CONTROL_TRANS;
		goto FINALLY;
	}

	nResult = fread( ( byte * )&stControlTrans, sizeof( CONTROL_TRANS ), 1,
		fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_READ_CONTROL_TRANS;
		goto FINALLY;
	}

	dwTotalCnt = GetDWORDFromASC( stControlTrans.abTotalCnt,
		sizeof( stControlTrans.abTotalCnt ) );
	dwTotalAmt = GetDWORDFromASC( stControlTrans.abTotalAmt,
		sizeof( stControlTrans.abTotalAmt ) );

	dwTotalCnt++;
	dwTotalAmt += dwFare;

	DWORD2ASCWithFillLeft0( dwTotalCnt, stControlTrans.abTotalCnt,
		sizeof( stControlTrans.abTotalCnt ) );
	DWORD2ASCWithFillLeft0( dwTotalAmt, stControlTrans.abTotalAmt,
		sizeof( stControlTrans.abTotalAmt ) );

	fseek( fdFile, 0 ,SEEK_SET );

	nResult = fwrite( ( byte * )&stControlTrans, sizeof( CONTROL_TRANS ), 1,
		fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_CONTROL_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       FileUpdateTransHeaderAndAppendTransTail                  *
*                                                                              *
*  DESCRIPTION:       �ŷ����������� ����� �ŷ��������ϸ��� ������Ʈ�� ��,    *
*                     �ŷ��������Ͽ� ������ ������ APPEND�Ѵ�.                 *
*                                                                              *
*  INPUT PARAMETERS:  pstTransTT - �ŷ������� APPEND�� �ŷ����� ������ ������  *
*                         ����ִ� TRANS_TT ����ü                             *
*                     abTransFileName - �ŷ��������ϸ�                         *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                     ERR_CARD_PROC_FILE_OPEN_TRANS                            *
*                         - �ŷ��������� OPEN ����                             *
*                     ERR_CARD_PROC_FILE_READ_TRANS                            *
*                         - �ŷ��������� READ ����                             *
*                     ERR_CARD_PROC_FILE_WRITE_TRANS                           *
*                         - �ŷ��������� WRITE ����                            *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-03-14                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short FileUpdateTransHeaderAndAppendTransTail( TRANS_TT *pstTransTT,
	byte *abTransFileName )
{
	short sResult = SUCCESS;
	int nResult = 0;
	byte i = 0;
	dword dwCRC = 0;
	FILE *fdFile = NULL;
	TRANS_TH stTransTH;

	/*
	 * �ŷ��������� OPEN
	 */
	fdFile = fopen( abTransFileName, "rb+" );
	if ( fdFile == NULL )
	{
		sResult = ERR_CARD_PROC_FILE_OPEN_TRANS;
		goto FINALLY;
	}

	fseek( fdFile, 0 ,SEEK_SET );

	nResult = fread( ( byte * )&stTransTH, sizeof( TRANS_TH ), 1, fdFile );
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_READ_TRANS;
		goto FINALLY;
	}

	/*
	 * �ŷ���������� �ŷ��������ϸ� ���� �� Record Mac ����
	 */
	memcpy( stTransTH.abFileName, pstTransTT->abFileName,
		sizeof( stTransTH.abFileName ) );
	dwCRC = MakeCRC32( ( byte * )&stTransTH, sizeof( TRANS_TH ) - 6 );
	memcpy( stTransTH.abRecMAC, ( byte * )&dwCRC,
		sizeof( stTransTH.abRecMAC ) );

	/*
	 * �ŷ�������� WRITE
	 */
	fseek( fdFile, 0 ,SEEK_SET );

	for ( i = 0; i < 3; i++ )
	{
		nResult = fwrite( ( byte * )&stTransTH, sizeof( TRANS_TH ), 1, fdFile );
		if ( nResult == 1 )
			break;
	}
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	/*
	 * �ŷ��������� APPEND
	 */
	fseek( fdFile, 0 ,SEEK_END );

	for ( i = 0; i < 3; i++ )
	{
		nResult = fwrite( ( byte * )pstTransTT, sizeof( TRANS_TT ), 1, fdFile );
		if ( nResult == 1 )
			break;
	}
	if ( nResult != 1 )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	nResult = fflush( fdFile );
	if ( nResult != SUCCESS )
	{
		sResult = ERR_CARD_PROC_FILE_WRITE_TRANS;
		goto FINALLY;
	}

	FINALLY:

	if ( fdFile != NULL )
	{
		fclose( fdFile );
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsExistTransDataWithFD                                   *
*                                                                              *
*  DESCRIPTION:       �ŷ���������� �����ϴ� �ŷ��������Ͽ� �Է���            *
*                     �ŷ����������Ͱ� �����ϴ����� ���θ� TRUE/FALSE��        *
*                     �����Ѵ�. ��, �Էµ� ���ϵ�ũ������ ���������� ��ġ��  *
*                     �ʱ�ȭ���� �����Ƿ� �����Ѵ�.                            *
*                                                                              *
*  INPUT PARAMETERS:  fdFile - �ŷ���������� �����ϴ� �ŷ�����������          *
*                         ���ϵ�ũ����                                       *
*                     pstTransTD - �ŷ��������Ͽ� �����ϴ����� ���θ� Ȯ����   *
*                         �ŷ����������� ����ü�� ������                       *
*                     dwTotCnt - �ŷ����������� �ŷ����������� ����            *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - �ŷ��������Ͽ� �ŷ����������Ͱ� ����              *
*                     FALSE - �ŷ��������Ͽ� �ŷ����������Ͱ� ������           *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-03-15                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static bool IsExistTransDataWithFD( FILE *fdFile, TRANS_TD *pstTransTD,
	dword dwTotCnt )
{
	int nResult = 0;
	dword i = 0;
	TRANS_TD stTransTD;

	/*
	 * �ŷ����������� �ŷ����������� ������ 0�̸� ������ FALSE ����
	 */
	if ( dwTotCnt == 0 )
	{
		return FALSE;
	}

	/*
	 * �ŷ����������� ������ŭ �ݺ�
	 */
	for ( i = 0; i < dwTotCnt; i++ )
	{
		fseek( fdFile, ( sizeof( TRANS_TH ) + sizeof( TRANS_TD ) * i ),
			SEEK_SET );

		nResult = fread( ( byte * )&stTransTD, sizeof( TRANS_TD ), 1, fdFile );
		if ( nResult != 1 )
		{
			return FALSE;
		}

		/*
		 * �ŷ��������Ϸκ��� READ�� �ŷ����������Ϳ� �Ķ���ͷ� �Էµ�
		 * �ŷ����������Ͱ� �����ϸ� TRUE ����
		 */
		if ( memcmp( ( byte * )&stTransTD, ( byte * )pstTransTD,
				sizeof( TRANS_TD ) ) == 0 )
		{
			return TRUE;
		}
	}

	/*
	 * ������ �ŷ����������͸� �߰����� ���� ��� FALSE ����
	 */
	return FALSE;
}

