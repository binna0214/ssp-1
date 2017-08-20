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
*  PROGRAM ID :       card_proc_util.c                                         *
*                                                                              *
*  DESCRIPTION:       ī��ó���� �ʿ��� ��ƿ�� ��ɵ��� �����Ѵ�.              *
*                                                                              *
*  ENTRY POINT:       bool IsValidPrepayIssuer( byte *abCardNo );              *
*                     bool IsValidPostpayCardNo( byte *abCardNo,               *
*                         byte bIssuerCode );                                  *
*                     bool IsSamsungAmexCard( byte *abCardNo );                *
*                     bool IsBCCardInvalidBIN( byte *abCardNo );               *
*                     bool IsValidPostpayIssuer( byte *abCardNo );             *
*                     bool IsValidIssuerValidPeriod( byte *abCardNo,           *
*                         byte *abExpiryDate );                                *
*                     bool IsValidMifPrepayIssueDate( byte *abCardNo,          *
*                         byte *abIssueDate );                                 *
*                     bool IsValidSCPrepayIssueDate( byte *abCardNo,           *
*                         time_t tIssueDate );                                 *
*                     short SearchCardErrLog( TRANS_INFO *pstTransInfo );      *
*                     void InitCardErrLog( void );                             *
*                     void AddCardErrLog( short sResult,                       *
*                         TRANS_INFO *pstTransInfo );                          *
*                     void DeleteCardErrLog( byte *abCardNo );                 *
*                     void PrintTransInfo( TRANS_INFO *pstTransInfo );         *
*                     void InitCardNoLog( void );                              *
*                     void AddCardNoLog( byte *abCardNo );                     *
*                     bool IsExistCardNoLog( byte *abCardNo );                 *
*                     bool IsValidSCPurseInfo(                                 *
*                         SC_EF_PURSE_INFO *pstPurseInfo );                    *
*                     void InitWatch( void );                                  *
*                     double StopWatch( void );                                *
*                     void PrintXferInfo(                                      *
*                         COMMON_XFER_DATA *pstCommonXferData );               *
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
* 2006/01/11 F/W Dev Team Boohyeon Jeon   Initial Release                      *
*                                                                              *
*******************************************************************************/

#include "../system/bus_type.h"
#include "card_proc.h"

#include "card_proc_util.h"

// ����LOG �� ī���ȣLOG �ִ� ���� ���� ///////////////////////////////////////
#define MAX_CARD_NO_LOG				10		// ī���ȣLOG �ִ� ����

// ī���ȣLOG ���� �������� ///////////////////////////////////////////////////
static byte gabCardNoLog[MAX_CARD_NO_LOG][20];
static int gnCardNoLogCount = -1;

//------------------------------------------------------------------------------
// �ð� ������ ���� ���� ����
static struct timeval stTime = {0L, 0L};
static int nWatchCount = 0;
static double dTotDuration = 0.0;

static short SearchErrLog( byte *abCardNo );
static void AddErrLog( TRANS_INFO *pstTransInfo );
static void UpdateErrLog( byte bIndex, TRANS_INFO *pstTransInfo );
static void PrintErrLog( void );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidPrepayIssuer                                      *
*                                                                              *
*  DESCRIPTION:       ���ҹ���������� ī���� prefix�� �����ϴ����� ���θ�     *
*                     �����Ѵ�.                                                *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - ī���ȣ                                      *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - ����� ����                                       *
*                     FALSE - ����� ������                                    *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsValidPrepayIssuer( byte *abCardNo )
{
	dword i = 0;					// �ݺ������� ���Ǵ� �ӽ� ����

	// ���ҹ���������� �ε忡 ������ ��� ������ TRUE ����
	if ( gstPrepayIssuerInfoHeader.dwRecordCnt == 0 )
	{
		return TRUE;
	}

	for ( i = 0; i < gstPrepayIssuerInfoHeader.dwRecordCnt; i++ )
	{
		if ( memcmp( abCardNo, gpstPrepayIssuerInfo[i].abPrepayIssuerID, 7 )
			== 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidPostpayCardNo                                     *
*                                                                              *
*  DESCRIPTION:       ��ȿ�� ī���ȣ������ ���θ� �����Ѵ�.                   *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - ī���ȣ                                      *
*                     bIssuerCode - ������ڵ�                                 *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - ��ȿ�� ī���ȣ                                   *
*                     FALSE - ī���ȣ ����                                    *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsValidPostpayCardNo( byte *abCardNo, byte bIssuerCode )
{
	bool boolResult = FALSE;

	if ( strlen( abCardNo ) < 15 )
		return FALSE;

	switch ( abCardNo[0] )
	{
		case '9':
			switch ( bIssuerCode )
			{
				case 0x00:
				case 0x02:
				case ISS_SS:
					if ( memcmp( abCardNo, "941009", 6 ) == 0 ||
						 memcmp( abCardNo, "941010", 6 ) == 0 )
					{
						boolResult = IsValidISOCardNo( abCardNo );
					}
					else
					{
						boolResult = IsValidSamsungLocalCardNo( abCardNo );
					}
					break;
				case ISS_LG:
					boolResult = IsValidLGLocalCardNo( abCardNo );
					break;
				default:
					boolResult = IsValidISOCardNo( abCardNo );
					break;
			}
			break;
		case '3':
			if ( IsSamsungAmexCard( abCardNo ) )
			{
				boolResult = IsValidAmexCardNo( abCardNo );
			}
			else
			{
				boolResult = IsValidISOCardNo( abCardNo );
			}
			break;
		default:
			boolResult = IsValidISOCardNo( abCardNo );
			break;
	}

	return boolResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsSamsungAmexCard                                        *
*                                                                              *
*  DESCRIPTION:       BIN�ڵ带 �̿��Ͽ� �ＺAMEXī�� ���θ� �Ǻ��Ѵ�.         *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - ī���ȣ                                      *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - �ＺAMEXī��                                      *
*                     FALSE - �ＺAMEXī�� �ƴ�                                *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsSamsungAmexCard( byte *abCardNo )
{
	byte i = 0;
	const static byte bSamsungAmexCardBINCnt = 3;
	const static byte abSamsungAmexCardBIN[3][7] =
		{"376293", "379183", "379184"};

	for ( i = 0; i < bSamsungAmexCardBINCnt; i++ )
	{
		if ( memcmp( abSamsungAmexCardBIN[i], abCardNo, 6 ) == 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsBCCardInvalidBIN                                       *
*                                                                              *
*  DESCRIPTION:       BC �׽�Ʈī�� BIN�ڵ� ���θ� �Ǻ��Ѵ�.                   *
*                     (�ش� BIN�ڵ��̸鼭 alias��ȣ�� ������ �ִ� ī���       *
*                      ���Ұ� ó���Ѵ�.)                                     *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - ī���ȣ                                      *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - BC �׽�Ʈī�� BIN�ڵ�                             *
*                     FALSE - BC �׽�Ʈī�� BIN�ڵ� �ƴ�                       *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsBCCardInvalidBIN( byte *abCardNo )
{
	byte i = 0;
	const static byte bBCCardInvalidBINCnt = 11;
	const static byte abBCCardInvalidBIN[11][7] =
		{"455323", "490623", "537620", "537703", "941025", "942021", "942023",
		 "942025", "942031", "942032", "942033"};

	for ( i = 0; i < bBCCardInvalidBINCnt; i++ )
	{
		if ( memcmp( abBCCardInvalidBIN[i], abCardNo, 6 ) == 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidPostpayIssuer                                     *
*                                                                              *
*  DESCRIPTION:       �ĺҹ���������� �ش� ī���� prefix�� �����ϴ����� ���θ�*
*                     �����Ѵ�.                                                *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - ī���ȣ                                      *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - �ĺҹ���������� prefix�� ������                  *
*                     FALSE - �ĺҹ���������� prefix�� �������� ����          *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsValidPostpayIssuer( byte *abCardNo )
{
	dword i = 0;

	// �ĺҹ���������� �ε忡 ������ ��� ������ TRUE ����
	if ( gstPostpayIssuerInfoHeader.wRecordCnt == 0 )
	{
		return TRUE;
	}

	for ( i = 0; i < gstPostpayIssuerInfoHeader.wRecordCnt; i++ )
	{
		if ( memcmp( gpstPostpayIssuerInfo[i].abPrefixNo, abCardNo, 6 ) == 0 )
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidIssuerValidPeriod                                 *
*                                                                              *
*  DESCRIPTION:       �ĺҹ���� ��ȿ�Ⱓ üũ�� �����Ѵ�.                     *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - ī���ȣ                                      *
*                     abExpiryDate - ��ȿ�Ⱓ (YYYYMM)                         *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - ī���� ��ȿ�Ⱓ�� ������� ��ȿ�Ⱓ���� ����    *
*                     FALSE - ī���� ��ȿ�Ⱓ�� ������� ��ȿ�Ⱓ���� ũ�ų� *
*                         ����                                                 *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-23                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsValidIssuerValidPeriod( byte *abCardNo, byte *abExpiryDate )
{
	dword i = 0;

	// �������ȿ�Ⱓ������ �ε忡 ������ ��� ������ TRUE ����
	if ( gstIssuerValidPeriodInfoHeader.wRecordCnt == 0 )
	{
		return TRUE;
	}

	for ( i = 0; i < gstIssuerValidPeriodInfoHeader.wRecordCnt; i++ )
	{
		if ( memcmp( gpstIssuerValidPeriodInfo[i].abPrefixNo, abCardNo,
				sizeof( gpstIssuerValidPeriodInfo[i].abPrefixNo ) ) == 0 &&
			 memcmp( "000000", gpstIssuerValidPeriodInfo[i].abExpiryDate,
				sizeof( gpstIssuerValidPeriodInfo[i].abExpiryDate ) ) != 0 &&
			 memcmp( abExpiryDate,
			 	gpstIssuerValidPeriodInfo[i].abExpiryDate,
			 	sizeof( gpstIssuerValidPeriodInfo[i].abExpiryDate ) ) >= 0 )
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidMifPrepayIssueDate                                *
*                                                                              *
*  DESCRIPTION:       ������ī�� �߱��� üũ�� �����Ѵ�.                       *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - ī���ȣ                                      *
*                     abIssueDate - �߱��� (YYYYMMDD)                          *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - ������ī�� �߱��� üũ ����                       *
*                     FALSE - ������ī�� �߱��� üũ ����                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-05-03                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsValidMifPrepayIssueDate( byte *abCardNo, byte *abIssueDate )
{
	dword i = 0;
	byte abASCPrefix[7] = {0, };
	byte abCheckIssueDate[8] = {0, };

	// �������ȿ�Ⱓ������ �ε忡 ������ ��� ������ TRUE ����
	if ( gstIssuerValidPeriodInfoHeader.wRecordCnt == 0 )
	{
		return TRUE;
	}

	if ( memcmp( abIssueDate, "00000000", 8 ) == 0 )
	{
		printf( "[IsValidMifPrepayIssueDate] �߱����� '0'���� ����\n" );
		return TRUE;
	}

	memcpy( abASCPrefix, abCardNo, sizeof( abASCPrefix ) );
	if ( abASCPrefix[0] == '0' )
	{
		abASCPrefix[0] = '1';
	}

	for ( i = 0; i < gstIssuerValidPeriodInfoHeader.wRecordCnt; i++ )
	{
		memcpy( &abCheckIssueDate[2], gpstIssuerValidPeriodInfo[i].abExpiryDate,
			sizeof( gpstIssuerValidPeriodInfo[i].abExpiryDate ) );
		if ( memcmp( gpstIssuerValidPeriodInfo[i].abExpiryDate, "70", 2 ) >= 0 )
		{
			abCheckIssueDate[0] = '1';
			abCheckIssueDate[1] = '9';
		}
		else
		{
			abCheckIssueDate[0] = '2';
			abCheckIssueDate[1] = '0';
		}

		if ( memcmp( "000000", gpstIssuerValidPeriodInfo[i].abPrefixNo,
				sizeof( gpstIssuerValidPeriodInfo[i].abPrefixNo ) ) == 0 &&
			 memcmp( abASCPrefix,
			 	gpstIssuerValidPeriodInfo[i].abIssuerID, 
			 	sizeof( gpstIssuerValidPeriodInfo[i].abIssuerID ) ) == 0 &&
			 memcmp( "000000", gpstIssuerValidPeriodInfo[i].abExpiryDate,
			 	sizeof( gpstIssuerValidPeriodInfo[i].abExpiryDate ) ) != 0 &&
			 memcmp( abIssueDate, abCheckIssueDate, sizeof( abCheckIssueDate ) ) >= 0 )
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsValidSCPrepayIssueDate                                 *
*                                                                              *
*  DESCRIPTION:       �ż���ī�� �߱��� üũ�� �����Ѵ�.                       *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - ī���ȣ                                      *
*                     tIssueDate - �߱���                                      *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - �ż���ī�� �߱��� üũ ����                       *
*                     FALSE - �ż���ī�� �߱��� üũ ����                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-05-03                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsValidSCPrepayIssueDate( byte *abCardNo, time_t tIssueDate )
{
	dword i = 0;
	byte abASCPrefix[7] = {0, };
	byte abCardIssueDate[8] = {0, };
	byte abCheckIssueDate[8] = {0, };

	// �������ȿ�Ⱓ������ �ε忡 ������ ��� ������ TRUE ����
	if ( gstIssuerValidPeriodInfoHeader.wRecordCnt == 0 )
	{
		return TRUE;
	}

	TimeT2ASCDate( tIssueDate, abCardIssueDate );

	abASCPrefix[0] = '3';
	memcpy( &abASCPrefix[1], abCardNo, 6 );

	for ( i = 0; i < gstIssuerValidPeriodInfoHeader.wRecordCnt; i++ )
	{
		memcpy( &abCheckIssueDate[2], gpstIssuerValidPeriodInfo[i].abExpiryDate,
			sizeof( gpstIssuerValidPeriodInfo[i].abExpiryDate ) );
		if ( memcmp( gpstIssuerValidPeriodInfo[i].abExpiryDate, "70", 2 ) >= 0 )
		{
			abCheckIssueDate[0] = '1';
			abCheckIssueDate[1] = '9';
		}
		else
		{
			abCheckIssueDate[0] = '2';
			abCheckIssueDate[1] = '0';
		}

		if ( memcmp( "000000", gpstIssuerValidPeriodInfo[i].abPrefixNo,
				sizeof( gpstIssuerValidPeriodInfo[i].abPrefixNo ) ) == 0 &&
			 memcmp( abASCPrefix,
			 	gpstIssuerValidPeriodInfo[i].abIssuerID,
			 	sizeof( gpstIssuerValidPeriodInfo[i].abIssuerID ) ) == 0 &&
			 memcmp( "000000", gpstIssuerValidPeriodInfo[i].abExpiryDate,
			 	sizeof( gpstIssuerValidPeriodInfo[i].abExpiryDate ) ) != 0 &&
			 memcmp( abCardIssueDate, abCheckIssueDate, sizeof( abCardIssueDate ) ) >= 0 )
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      SearchCardErrLog                                         *
*                                                                              *
*  DESCRIPTION:       Error List[��ũ�帮��Ʈ]�� �˻��� �� ����� ��ȯ�Ѵ�.    *
*		              Error List�� �ִ� ī���� ��� ī������ ����ü�� �ش�	   *
*          			  Error Log�� ����ü�� �ٲ��ش�.                           *
*					  Error List�� �ش� Log ��ȣ�� �����Ѵ�. (-> nCurrentLog)  *
*					  Log ��ȣ�� 1������ �����Ѵ�.                             *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - Error List�� ���� (���� ī��)                  *
*                     != 0 - Error Code (Error Log ��� ����� ���� �ڵ�)      *
*                                                                              *
*  Author : Kyoungryun Bae													   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short SearchCardErrLog( TRANS_INFO *pstTransInfo )
{
	short sIndex = 0;

	sIndex = SearchErrLog( pstTransInfo->abCardNo );

	// �����α׿� �������� ���� ////////////////////////////////////////////////
	if ( sIndex == -1 )
	{
		return -1;
	}

	// �����α׿� ������ ///////////////////////////////////////////////////////
	memcpy( pstTransInfo,
		&gpstSharedInfo->astTransErrLog[( byte )sIndex].stTransInfo,
		sizeof( TRANS_INFO ) );

	return SUCCESS;
}

void InitCardErrLog( void )
{
	gpstSharedInfo->bTransErrLogPtr = 0;
	memset( gpstSharedInfo->astTransErrLog, 0x00,
		sizeof( gpstSharedInfo->astTransErrLog ) );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      AddCardErrLog                                            *
*                                                                              *
*  DESCRIPTION:       ī��ŷ�ó�� �� ����ó��                                 *
*                                                                              *
*  INPUT PARAMETERS:  short sResult, TRANS_INFO *pstTransInfo                  *
*                                                                              *
*  RETURN/EXIT VALUE: N/A                                                      *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-09-06 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void AddCardErrLog( short sResult, TRANS_INFO *pstTransInfo )
{
	short sIndex = 0;

	if ( sResult != SUCCESS )
	{
		//ī�� ������� ī��Ʈ ����
		pstTransInfo->bWriteErrCnt++;
		pstTransInfo->sErrCode = sResult;
	}

	if ( ( sIndex = SearchErrLog( pstTransInfo->abCardNo ) ) == -1 )
	{
		AddErrLog( pstTransInfo );
	}
	else
	{
		UpdateErrLog( ( byte )sIndex, pstTransInfo );
	}

	PrintErrLog();
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      DeleteCardErrLog                                         *
*                                                                              *
*  DESCRIPTION:       Error ��ũ�帮��Ʈ�� Ư�� Node(->nCurrentLog) ����       *
*			          SearchErrLog()�� ����Ǽ� nCurrentLog ����               *
*			          �����Ǿ� ���� ������ ���� ��ȯ                           *
*                                                                              *
*  INPUT PARAMETERS:  N/A                                                      *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - ���������� ������                                    *
*				      1 - �����Ϸ��� Log�� ����Ʈ�� ����                       *
*                                                                              *
*  Author : Kyoungryun Bae													   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void DeleteCardErrLog( byte *abCardNo )
{
	short sIndex = 0;

	sIndex = SearchErrLog( abCardNo );
	if ( sIndex != -1 )
	{
		gpstSharedInfo->astTransErrLog[( byte )sIndex].boolIsDeleted = TRUE;
	}

	PrintErrLog();
}

static short SearchErrLog( byte *abCardNo )
{
	byte i = 0;

	for ( i = 0; i < MAX_ERROR_LOG; i++ )
	{
		if ( memcmp( gpstSharedInfo->astTransErrLog[i].stTransInfo.abCardNo,
			abCardNo, 20 ) == 0 &&
			 gpstSharedInfo->astTransErrLog[i].boolIsDeleted == FALSE )
		{
			DebugOut( "[SearchErrLog] �˻� BINGO!\n" );
			return i;
		}
	}

	DebugOut( "[SearchErrLog] �˻� ����!\n" );

	return -1;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      AddErrLog                                                *
*                                                                              *
*  DESCRIPTION:       Error List�� Log �߰�                                    *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*  RETURN/EXIT VALUE: N/A                                                      *
*                                                                              *
*  Author : Kyoungryun Bae													   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void AddErrLog( TRANS_INFO *pstTransInfo )
{
	DebugOutlnASC( "[AddErrLog] ī���ȣ : ", pstTransInfo->abCardNo, 20 );
	DebugOut	 ( "[AddErrLog] �ε���   : %u\n",
		gpstSharedInfo->bTransErrLogPtr );

	gpstSharedInfo->astTransErrLog[gpstSharedInfo->bTransErrLogPtr].boolIsDeleted = FALSE;
	memcpy(
		&gpstSharedInfo->astTransErrLog[gpstSharedInfo->bTransErrLogPtr].stTransInfo,
		pstTransInfo, sizeof( TRANS_INFO ) );
	gpstSharedInfo->bTransErrLogPtr = ( gpstSharedInfo->bTransErrLogPtr + 1 )
		% MAX_ERROR_LOG;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      UpdateErrLog                                             *
*                                                                              *
*  DESCRIPTION:       Error ��ũ�帮��Ʈ�� Ư�� Node( ->nCurrentLog ) �� ����    *
*                                                                              *
*  INPUT PARAMETERS:  TRANS_INFO *pstTransInfo                                 *
*                                                                              *
*			          SearchErrLog()�� ����Ǽ� nCurrentLog ����               *
*			          �����Ǿ� ���� ������ ���� ��ȯ                           *
*                                                                              *
*  INPUT PARAMETERS:  N/A                                                      *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - ���������� ������                                    *
*				      1 - �����Ϸ��� Log�� ����Ʈ�� ����                       *
*                                                                              *
*  Author : Kyoungryun Bae													   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static void UpdateErrLog( byte bIndex, TRANS_INFO *pstTransInfo )
{
	DebugOutlnASC( "[UpdateErrLog] ī���ȣ : ", pstTransInfo->abCardNo, 20 );
	DebugOut	 ( "[UpdateErrLog] �ε���   : %u\n", bIndex );

	memcpy( &gpstSharedInfo->astTransErrLog[bIndex].stTransInfo, pstTransInfo,
		sizeof( TRANS_INFO ) );
}

static void PrintErrLog( void )
{
	byte i = 0;

	for ( i = 0; i < MAX_ERROR_LOG; i++ )
	{
		if ( gpstSharedInfo->astTransErrLog[i].boolIsDeleted )
		{
			DebugOut( "[%2u] ���� ", i );
		}
		else
		{
			DebugOut( "[%2u]      ", i );
		}
		DebugOutlnASC( "",
			gpstSharedInfo->astTransErrLog[i].stTransInfo.abCardNo, 20 );
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       PrintTransInfo                                           *
*                                                                              *
*  DESCRIPTION:       ī����������ü�� ������ ȭ�鿡 ����Ѵ�.                 *
*                                                                              *
*  INPUT PARAMETERS:  pstTransInfo - ī������ ����ü                           *
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
void PrintTransInfo( TRANS_INFO *pstTransInfo )
{
	DebugOut		( "\n// ī����������ü /////////////////////////////////" );
	DebugOut		( "////////////////////////////\n" );
	DebugOut		( "Ĩ �ø��� ��ȣ                        : %lu\n",
		pstTransInfo->dwChipSerialNo );
	DebugOut		( "ī����������                        : %u\n",
		pstTransInfo->bPLUserType );
	DebugOut		( "ī������                              : %u\n",
		pstTransInfo->bCardType );
	DebugOutlnASC	( "ī���ȣ                              : ",
		pstTransInfo->abCardNo, 20 );
	DebugOut		( "alias ��ȣ                            : %lu\n",
		pstTransInfo->dwAliasNo );

	DebugOut		( "��ī�� �˰�������                   : %u\n",
		pstTransInfo->bSCAlgoriType );
	DebugOut		( "��ī�� �ŷ�����                       : %u\n",
		pstTransInfo->bSCTransType );
	DebugOut		( "��ī�� �����ŷ�����Ű����             : %u\n",
		pstTransInfo->bSCTransKeyVer );
	DebugOut		( "��ī�� ����ȭ���ID                   : %u\n",
		pstTransInfo->bSCEpurseIssuerID );
	DebugOutlnASC	( "��ī�� ��������ID                     : ",
		pstTransInfo->abSCEpurseID, 16 );
	DebugOut		( "��ī�� �ŷ��Ǽ�                       : %lu\n",
		pstTransInfo->dwSCTransCnt );

	DebugOutlnASC	( "PSAM ID                               : ",
		pstTransInfo->abPSAMID, 16 );
	DebugOut		( "PSAM �ŷ�ī����                       : %lu\n",
		pstTransInfo->dwPSAMTransCnt );
	DebugOut		( "PSAM �Ѿװŷ�����ī����               : %lu\n",
		pstTransInfo->dwPSAMTotalTransCnt );
	DebugOut		( "PSAM �����ŷ������Ǽ�                 : %u\n",
		pstTransInfo->wPSAMIndvdTransCnt );
	DebugOut		( "PSAM �����ŷ��Ѿ�                     : %lu\n",
		pstTransInfo->dwPSAMAccTransAmt );
	DebugOutlnBCD	( "PSAM ����                             : ",
		pstTransInfo->abPSAMSign, 4 );

	DebugOut		( "����������ī���ܾ�                    : %lu\n",
		pstTransInfo->dwBalAfterCharge );
	DebugOut		( "����������ī��ŷ��Ǽ�                : %lu\n",
		pstTransInfo->dwChargeTransCnt );
	DebugOut		( "���������ݾ�                          : %lu\n",
		pstTransInfo->dwChargeAmt );
	DebugOutlnASC	( "����������LSAMID                      : ",
		pstTransInfo->abLSAMID, 16 );
	DebugOut		( "����������LSAM�ŷ��Ϸù�ȣ            : %lu\n",
		pstTransInfo->dwLSAMTransCnt );
	DebugOut		( "���������ŷ�����                      : %u\n",
		pstTransInfo->bChargeTransType );
	DebugOutlnASC	( "������ī�� �����������ι�ȣ           : ",
		pstTransInfo->abMifPrepayChargeAppvNop, 14 );
	DebugOutlnBCD	( "������ī�� TCC                        : ",
		pstTransInfo->abMifPrepayTCC, 8 );

	DebugOut		( "����������                            : %u\n",
		pstTransInfo->bEntExtType );
	DebugOut		( "��ī�� �ܸ���׷��ڵ�                 : %u\n",
		pstTransInfo->bMifTermGroupCode );
	DebugOutlnTimeT	( "��ī�� �̿�ð�                       : ",
		pstTransInfo->tMifEntExtDtime );
	DebugOut		( "���ν¿���                            : %s\n",
		GetBoolString( pstTransInfo->boolIsMultiEnt ) );
	DebugOut		( "�߰���������                          : %s\n",
		GetBoolString( pstTransInfo->boolIsAddEnt ) );
	DebugOutlnBCD	( "���ν»��������                      : ",
		( byte * )pstTransInfo->abUserType, 3 );
	DebugOutlnBCD	( "���ν»���ڼ�                        : ",
		( byte * )pstTransInfo->abUserCnt, 3 );
	DebugOutlnASC	( "���ν�������������ID1                 : ",
		pstTransInfo->abDisExtraTypeID[0], 6 );
	DebugOutlnASC	( "���ν�������������ID2                 : ",
		pstTransInfo->abDisExtraTypeID[1], 6 );
	DebugOutlnASC	( "���ν�������������ID3                 : ",
		pstTransInfo->abDisExtraTypeID[2], 6 );
	DebugOut		( "���Ÿ�                              : %lu\n",
		pstTransInfo->dwDist );
	DebugOut		( "��ȯ�� ����                           : %u\n",
		pstTransInfo->bNonXferCause );
	DebugOut		( "�ĺ�ī���̸鼭 �����濩��             : %s\n",
		GetBoolString( pstTransInfo->boolIsChangeMonth ) );
	DebugOutlnTimeT	( "������ī�� ���� ��� �Ͻ�             : ",
		pstTransInfo->tMifTourFirstUseDtime );
	DebugOut		( "������ī�� ����                       : %u\n",
		pstTransInfo->wMifTourCardType );
	DebugOut		( "������ī�� �̹� ��/������ ���� Ƚ��   : %u\n",
		pstTransInfo->bMifTourUseCnt );

	DebugOut		( "\n-- ����ȯ������ -----------------------------------" );
	DebugOut		( "----------------------------\n" );
	PrintCommonXferInfo( 0, &pstTransInfo->stPrevXferInfo );
	DebugOut		( "\n-- �ű�ȯ������ -----------------------------------" );
	DebugOut		( "----------------------------\n" );
	PrintCommonXferInfo( 0, &pstTransInfo->stNewXferInfo );

	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_PREPAY ||
		pstTransInfo->bCardType == TRANS_CARDTYPE_DEPOSIT )
	{
		DebugOut		( "\n-- ������ī�� BLOCK18 -------------------------" );
		DebugOut		( "--------------------------------\n" );
		PrintMifPrepayBlock18( &pstTransInfo->stMifPrepayBlock18 );
	}
	if ( pstTransInfo->bCardType == TRANS_CARDTYPE_MIF_POSTPAY )
	{
		DebugOut		( "\n-- ���ĺ�ī�� ��ȯ������ ----------------------" );
		DebugOut		( "--------------------------------\n" );
		PrintMifPostpayOldXferData( &pstTransInfo->stOldXferInfo );
	}

	DebugOut		( "�����ڵ�                              : %x\n",
		pstTransInfo->sErrCode );
	DebugOut		( "WRITE���� �߻� Ƚ��                   : %u\n",
		pstTransInfo->bWriteErrCnt );
	DebugOut		( "/////////////////////////////////////////////////////" );
	DebugOut		( "//////////////////////////\n" );
	DebugOut		( "\n" );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       InitCardNoLog                                            *
*                                                                              *
*  DESCRIPTION:       ī���ȣLOG�� �ʱ�ȭ�Ѵ�.                                *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
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
void InitCardNoLog( void )
{
	gnCardNoLogCount = -1;
	memset( gabCardNoLog, 0, sizeof( gabCardNoLog ) );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       AddCardNoLog                                             *
*                                                                              *
*  DESCRIPTION:       ī���ȣLOG�� ī���ȣ�� �߰��Ѵ�.                       *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - ī���ȣLOG�� �߰��ϰ��� �ϴ� ī���ȣ        *
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
void AddCardNoLog( byte *abCardNo )
{
	byte i = 0;

	if ( IsExistCardNoLog( abCardNo ) )
	{
		for ( i = 0; i < MAX_CARD_NO_LOG; i++ )
		{
			DebugOutlnASC( "[CARDNO_LOG] ī���ȣ : ", gabCardNoLog[i], 20 );
		}
		return;
	}

	if ( gnCardNoLogCount == -1 ) {
		memset( gabCardNoLog, 0, sizeof( gabCardNoLog ) );
		gnCardNoLogCount = 0;
	}

	memcpy( gabCardNoLog[gnCardNoLogCount], abCardNo, 20 );
	gnCardNoLogCount = ( gnCardNoLogCount + 1 ) % MAX_CARD_NO_LOG;

	for ( i = 0; i < MAX_CARD_NO_LOG; i++ )
	{
		DebugOutlnASC( "[CARDNO_LOG] ī���ȣ : ", gabCardNoLog[i], 20 );
	}
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       IsExistCardNoLog                                         *
*                                                                              *
*  DESCRIPTION:       ī���ȣLOG�� ī���ȣ�� �߰��Ѵ�.                       *
*                                                                              *
*  INPUT PARAMETERS:  abCardNo - ī���ȣLOG�� �����ϴ��� Ȯ���ϰ��� �ϴ�      *
*                         ī���ȣ                                             *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE - �Էµ� ī���ȣ�� ī���ȣLOG�� ������            *
*                     FALSE - �Էµ� ī���ȣ�� ī���ȣLOG�� �������� ����    *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-08-25                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
bool IsExistCardNoLog( byte *abCardNo )
{

	int i = 0;

	if ( gnCardNoLogCount == -1 )
		return FALSE;

	for ( i = 0; i < MAX_CARD_NO_LOG; i++ )
		if ( memcmp( gabCardNoLog[i], abCardNo, 20 ) == 0 )
			return TRUE;

	return FALSE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      IsValidSCPurseInfo                                       *
*                                                                              *
*  DESCRIPTION:       ��ī�� �������� �������� üũ                            *
*                                                                              *
*  INPUT PARAMETERS:  SC_EF_PURSE_INFO *pstPurseInfo                           *
*                                                                              *
*  RETURN/EXIT VALUE: TRUE/FALSE                                               *
*                                                                              *
*  Author : MeeHyang Son													   *
*                                                                              *
*  DATE   : 2005-11-17 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
bool IsValidSCPurseInfo( SC_EF_PURSE_INFO *pstPurseInfo )
{

	// ī���ȣ Ȯ��
	if ( IsDigitASC( pstPurseInfo->abEpurseID, 16 ) != TRUE )
		return FALSE;

	// ī������� �����ڵ� Ȯ�� ( 0x00 ~ 0x0F )
	if ( pstPurseInfo->bUserTypeCode > 0x0F )
		return FALSE;

	return TRUE;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InitWatch                                                *
*                                                                              *
*  DESCRIPTION:       �ð� ������ ���� ������ �ʱ�ȭ                           *
*                                                                              *
*  INPUT PARAMETERS:  N/A                                                      *
*                                                                              *
*  RETURN/EXIT VALUE: N/A                                                      *
*                                                                              *
*  Author : Kyoungryun Bae													   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void InitWatch( void )
{
	stTime.tv_sec = 0;
	stTime.tv_usec = 0;
	nWatchCount =0;
	dTotDuration = 0.0;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      StopWatch                                                *
*                                                                              *
*  DESCRIPTION:       InitWatch() �Լ��� ȣ���� �������κ��� ����� �ð� ���  *
*                                                                              *
*  INPUT PARAMETERS:  N/A                                                      *
*                                                                              *
*  RETURN/EXIT VALUE: double ����ð�                                          *
*                                                                              *
*  Author : Kyoungryun Bae													   *
*                                                                              *
*  DATE   : 2005-08-11 														   *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
double StopWatch( void )
{
	struct timeval tv;
	double dur = 0.0;

	gettimeofday( &tv, NULL );
	if ( stTime.tv_sec == 0 && stTime.tv_usec==0 ) {  /* first stop */
		DebugOut( "Start watch....\n" );
	}
	else {
		nWatchCount++;

		dur = ( tv.tv_sec - stTime.tv_sec ) +
			( ( tv.tv_usec - stTime.tv_usec ) / 1000000.0 );
		DebugOut( "[%d]....Time duration %f sec\n", nWatchCount, dur );

		dTotDuration += dur;
		DebugOut( "[%d]....Total Time duration %f sec\n", nWatchCount,
			dTotDuration );
	}
	stTime.tv_sec  = tv.tv_sec;
	stTime.tv_usec = tv.tv_usec;

	return dur;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       PrintXferInfo                                            *
*                                                                              *
*  DESCRIPTION:       �Էµ� ȯ�������� �ֿܼ� ������ ���·� ����Ѵ�.         *
*                                                                              *
*  INPUT PARAMETERS:  pstCommonXferData - �Է� ����ȯ������                    *
*                                                                              *
*  RETURN/EXIT VALUE: void                                                     *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2005-11-17                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
void PrintXferInfo( COMMON_XFER_DATA *pstCommonXferData )
{
	printf( "������ ����\t\t: %9x", pstCommonXferData->bEntExtType );
	printf( "\tȯ�� ���� Ƚ��\t\t: %9u", pstCommonXferData->bAccXferCnt );

	printf( "\nȯ�� �Ϸù�ȣ\t\t: %9u", pstCommonXferData->bXferSeqNo );
	printf( "\t������(��) �ڵ�\t\t:   " );
	PrintASC( pstCommonXferData->abStationID, 7 );

	printf( "\n���� ���� �ڵ�\t\t: %9u", pstCommonXferData->wTranspMethodCode );
	printf( "\t�̿� �ð�\t: " );
	PrintTimeT( pstCommonXferData->tEntExtDtime );

	printf( "\nȯ�³� ���� �̵� �Ÿ�\t: %9lu",
		pstCommonXferData->dwAccDistInXfer );
	printf( "\tȯ�³� ���� �̿� �ݾ�\t: %9lu",
		pstCommonXferData->dwAccAmtInXfer );

	printf( "\n�ܸ���ID\t\t: %09lu", pstCommonXferData->dwTermID );
	printf( "\t���ν� �ŷ� ����\t: " );
	PrintBCD( ( byte * )pstCommonXferData->abMultiEntInfo, 6 );

	printf( "\n�� ���� ���� Ƚ��\t: %9u", pstCommonXferData->wTotalAccEntCnt );
	printf( "\t�� ���� ��� �ݾ�\t: %9lu",
		pstCommonXferData->dwTotalAccUseAmt );

	printf( "\n������1 �ִ� �⺻���\t: %9u",
		pstCommonXferData->awMaxBaseFare[0] );
	printf( "\t������2 �ִ� �⺻���\t: %9u",
		pstCommonXferData->awMaxBaseFare[1] );

	printf( "\n������3 �ִ� �⺻���\t: %9u",
		pstCommonXferData->awMaxBaseFare[2] );
	printf( "\tȯ�³� �⺻����� ��\t: %9u",
		pstCommonXferData->wTotalBaseFareInXfer );

	printf( "\n���� ���Ƽ ���\t: %9u", pstCommonXferData->wPrevPenaltyFare );
	printf( "\t���� ��¡�� �ݾ�\t: %9u",
		pstCommonXferData->wPrevUnchargedFare );

	printf( "\n���\t\t\t: %9lu", pstCommonXferData->dwFare );
	printf( "\t�ܾ�\t\t\t: %9lu", pstCommonXferData->dwBal );

	printf( "\n" );
}
