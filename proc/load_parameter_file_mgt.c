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
*  PROGRAM ID :       load_parameter_file_mgt.c                                *
*                                                                              *
*  DESCRIPTION:       �� ���α׷��� �Ķ���� ���ϰ� ����� ����ϱ�����  	   *
*                       ������ �ε��Ѵ�.                                       *
*                                                                              *
*  ENTRY POINT:     void InitOperParmInfo( void );							   *
*					short LoadOperParmInfo( void );							   *
*					short LoadVehicleParm( void );							   *
*					short LoadRouteParm( ROUTE_PARM *pstRouteParmInfo );	   *
*					short LoadInstallInfo( void );							   *
*					short GetLEAPPasswd( void );                               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:         c_op_par.dat                                           *
*                       c_li_par.dat                                           *
*                       c_ap_inf.dat                                           *
*                       c_dp_inf.dat                                           *
*                       c_cd_inf.dat                                           *
*                       c_tr_inf.dat                                           *
*                       c_de_inf.dat                                           *
*                       c_ho_inf.dat                                           *
*                       c_n_far.dat                                            *
*                       c_st_inf.dat                                           *
*                       setup.dat                                              *
*                       setup.backup                                           *
*                       tc_leap.dat                                            *
*                       tc_leap.backup                                         *
*                                                                              *
*  OUTPUT FILES:                                                               *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2005/09/03 Solution Team Mi Hyun Noh  Initial Release                        *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "load_parameter_file_mgt.h"
#include "trans_proc.h"

/*******************************************************************************
*  Declaration of variables                                                    *
*******************************************************************************/
COMM_INFO                       gstCommInfo;

#define     NEGATIVE_CLASS      'M'     // ������������ -
#define     IP_CLASS_LENGTH     3       // �� Ŭ������ �����ϴ� ����

int  nTermIDLength = sizeof( gpstSharedInfo->abMainTermID );


/*******************************************************************************
*  Declaration of Extern variables                                             *
*******************************************************************************/
ROUTE_PARM                      gstRouteParm;	// �뼱���Ķ���� ����ü
VEHICLE_PARM                    gstVehicleParm;	// �������� �Ķ���� ����ü
PREPAY_ISSUER_INFO_HEADER		gstPrepayIssuerInfoHeader;
PREPAY_ISSUER_INFO          	*gpstPrepayIssuerInfo = NULL;
POSTPAY_ISSUER_INFO_HEADER      gstPostpayIssuerInfoHeader;
POSTPAY_ISSUER_INFO             *gpstPostpayIssuerInfo = NULL;
ISSUER_VALID_PERIOD_INFO_HEADER	gstIssuerValidPeriodInfoHeader;
ISSUER_VALID_PERIOD_INFO		*gpstIssuerValidPeriodInfo = NULL;
XFER_APPLY_INFO_HEADER          gstXferApplyInfoHeader;
XFER_APPLY_INFO                 *gpstXferApplyInfo = NULL;
DIS_EXTRA_INFO_HEADER           gstDisExtraInfoHeader;
DIS_EXTRA_INFO                  *gpstDisExtraInfo = NULL;
HOLIDAY_INFO_HEADER             gstHolidayInfoHeader;
HOLIDAY_INFO                    *gpstHolidayInfo = NULL;
NEW_FARE_INFO                   gstNewFareInfo;
STATION_INFO_HEADER             gstStationInfoHeader;
STATION_INFO                    *gpstStationInfo = NULL;

byte    gabDCSIPAddr[16] 	= { 0, };     	// DCS server IP (xxx.xxx.xxx.xxx)
byte    gabLEAPPasswd[21]	= { 0, };		// LEAP password
byte    gabEndStationID[7] 	= { 0, };    	// ����������ID

TEMP_STATION_INFO_HEADER        stTmpStationInfoHeader;
TEMP_STATION_INFO               stTmpStationInfo;

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
static short LoadPrepayIssuerInfo( void );
static short LoadPostpayIssuerInfo( void );
static short LoadIssuerValidPeriodInfo( void );
static short LoadXferApplyInfo( void );
static short LoadDisExtraInfo( void );
static short LoadHolidayInfo( void );
static short LoadNewFareInfo( void );
static short LoadHardCodedNewFareInfo( void );

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      InitOperParmInfo                                         *
*                                                                              *
*  DESCRIPTION :      ���� �Ķ���� ������ �ʱ�ȭ �Ѵ�.					       *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     void                                                 *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-11-09                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
void InitOperParmInfo( void )
{

    memset( &gstPrepayIssuerInfoHeader,
            0,
            sizeof(gstPrepayIssuerInfoHeader));
    memset( &gstPostpayIssuerInfoHeader,
            0,
            sizeof(gstPostpayIssuerInfoHeader));
    memset( &gstIssuerValidPeriodInfoHeader,
            0,
            sizeof(gstIssuerValidPeriodInfoHeader));
    memset( &gstXferApplyInfoHeader,
            0,
            sizeof(gstXferApplyInfoHeader));
    memset( &gstDisExtraInfoHeader,
            0,
            sizeof(gstDisExtraInfoHeader));
    memset( &gstHolidayInfoHeader,
            0,
            sizeof(gstHolidayInfoHeader));
    memset( &gstNewFareInfo,
            0,
            sizeof(gstNewFareInfo));
    memset( &gstStationInfoHeader,
            0,
            sizeof(gstStationInfoHeader));
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadOperParmInfo                                         *
*                                                                              *
*  DESCRIPTION :      �� �Ķ���� ������ �ε� �Ѵ�.							   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:     SUCCESS                                              *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadOperParmInfo( void )
{
    short sResult = SUCCESS;
    short sRetVal = SUCCESS;
	byte i = 0;

    /*
     * �������� �ε�
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadVehicleParm();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  �������� �ε� ���� �̺�Ʈ ����
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_VEHICLE_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

    /*
     * �뼱���� �ε�
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadRouteParm();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  �뼱���� �ε� ���� �̺�Ʈ ����
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_ROUTE_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

    /*
     * �ſ������ �ε�
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadNewFareInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
    if ( sResult != SUCCESS )
    {
        /*
         *  �ſ������ �ε� ���� �̺�Ʈ ����
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_NEW_FARE_INFO );

		/*
		 * �ϵ��ڵ��� �ſ�������� �ε�
		 */
		LoadHardCodedNewFareInfo();

        sRetVal = ERR_LOAD_PARM;
    }

    /*
     * ���������� �ε�
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadStationInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  ���������� �ε� ���� �̺�Ʈ ����
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_STATION_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

    /*
     * ���ҹ�������� �ε�
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadPrepayIssuerInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  ���ҹ�������� �ε� ���� �̺�Ʈ ����
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_PREPAY_ISSUER_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

    /*
     * �ĺҹ�������� �ε�
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadPostpayIssuerInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  �ĺҹ�������� �ε� ���� �̺�Ʈ ����
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_POSTPAY_ISSUER_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

    /*
     * �������ȿ�Ⱓ���� �ε�
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadIssuerValidPeriodInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  �������ȿ�Ⱓ���� �ε� ���� �̺�Ʈ ����
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_ISSUER_VALID_PERIOD_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

    /*
     * ������������ �ε�
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadDisExtraInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
    if ( sResult != SUCCESS )
    {
        /*
         *  ������������ �ε� ���� �̺�Ʈ ����
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_DIS_EXTRA_INFO );

        sRetVal = ERR_LOAD_PARM;
    }

    /*
     * ȯ������ �ε�
     */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadHolidayInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
    if ( sResult != SUCCESS )
	{
		/*
		 *  �������� �ε� ���� �̺�Ʈ ����
		 */
	    ctrl_event_info_write( EVENT_LOAD_ERR_HOLIDAY_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

	/*
	 * ȯ���������� �ε�
	 */
	for ( i = 0; i < 3; i++ )
	{
	    sResult  = LoadXferApplyInfo();
		if ( sResult == SUCCESS )
		{
			break;
		}
	}
	if ( sResult != SUCCESS )
	{
        /*
         *  ȯ���������� �ε� ���� �̺�Ʈ ����
         */
        ctrl_event_info_write( EVENT_LOAD_ERR_XFER_APPLY_INFO );

		sRetVal = ERR_LOAD_PARM;
	}

	printf( "[LoadInfo] ��� ��������� �ε� �Ϸ� ----------------------------------------\n" );

	return sRetVal;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadVehicleParm                                          *
*                                                                              *
*  DESCRIPTION :      �������� �Ķ���� ������ �ε� �Ѵ�.			               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( VEHICLE_PARM_FILE )         *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadVehicleParm( void )
{
	int nReadSize = 0;
	FILE *fdVehicleParm = NULL;
	TEMP_VEHICLE_PARM stTmpVehicleParm;

	memset( &stTmpVehicleParm, 0x00, sizeof( stTmpVehicleParm ) );
	memset( &gstVehicleParm, 0x00, sizeof( gstVehicleParm ) );

	/*
	 * ������������ OPEN
	 */
	fdVehicleParm = fopen( VEHICLE_PARM_FILE, "rb" );
	if ( fdVehicleParm == NULL )
	{
		printf( "[LoadInfo] �������� �ε� ���� (OPEN)\n" );
		return ErrRet( ERR_FILE_OPEN | GetFileNo( VEHICLE_PARM_FILE ) );
	}

	/*
	 * ������������ READ
	 */
	nReadSize = fread( &stTmpVehicleParm,
		sizeof( stTmpVehicleParm ),
		1,
		fdVehicleParm );

    fclose( fdVehicleParm );

	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] �������� �ε� ���� (���ڵ� READ)\n" );
		return ErrRet( ERR_FILE_READ | GetFileNo( VEHICLE_PARM_FILE ) );
	}

	/*
	 * �������� ����
	 */
	// 1. �����Ͻ�
	memcpy( gstVehicleParm.abApplyDtime, stTmpVehicleParm.abApplyDtime,
		sizeof( gstVehicleParm.abApplyDtime ) );

	// 2. �������� ID
	memcpy( gstVehicleParm.abTranspBizrID, stTmpVehicleParm.abTranspBizrID,
		sizeof( gstVehicleParm.abTranspBizrID ) );

	// 3. ���������� ID
	memcpy( gstVehicleParm.abBusBizOfficeID, stTmpVehicleParm.abBusBizOfficeID,
		sizeof( gstVehicleParm.abBusBizOfficeID ) );

	// 4. ���� ID
	memcpy( gstVehicleParm.abVehicleID, stTmpVehicleParm.abVehicleID,
		sizeof( gstVehicleParm.abVehicleID ) );

	// 5. ������ȣ
	memcpy( gstVehicleParm.abVehicleNo, stTmpVehicleParm.abVehicleNo,
		sizeof( gstVehicleParm.abVehicleNo ) );

	// 6. �뼱 ID
	memcpy( gstVehicleParm.abRouteID, stTmpVehicleParm.abRouteID,
		sizeof( gstVehicleParm.abRouteID ) );

	// 7. ������ ID
	memcpy( gstVehicleParm.abDriverID, stTmpVehicleParm.abDriverID,
		sizeof( gstVehicleParm.abDriverID ) );

	// 8. ��������ڵ� - byte �迭
	memcpy( gstVehicleParm.abTranspMethodCode,
		stTmpVehicleParm.abTranspMethodCode,
		sizeof( gstVehicleParm.abTranspMethodCode ) );

	// 9. ��������ڵ� - word
	gstVehicleParm.wTranspMethodCode =
		GetWORDFromASC( gstVehicleParm.abTranspMethodCode,
			sizeof( gstVehicleParm.abTranspMethodCode ) );

	// 10. ����ڹ�ȣ
	memcpy( gstVehicleParm.abBizNo, stTmpVehicleParm.abBizNo,
		sizeof( gstVehicleParm.abBizNo ) );

	// 11. �������ڸ�
	memcpy( gstVehicleParm.abTranspBizrNm, stTmpVehicleParm.abTranspBizrNm,
		sizeof( gstVehicleParm.abTranspBizrNm ) );

	// 12. �ּ�
	memcpy( gstVehicleParm.abAddr, stTmpVehicleParm.abAddr,
		sizeof( gstVehicleParm.abAddr ) );

	// 13. ��ǥ�ڸ�
	memcpy( gstVehicleParm.abRepreNm, stTmpVehicleParm.abRepreNm,
		sizeof( gstVehicleParm.abRepreNm ) );

	DebugOutlnASC	( "�����Ͻ�                              : ",
		gstVehicleParm.abApplyDtime, sizeof( stTmpVehicleParm.abApplyDtime ) );
	DebugOutlnASC	( "�������� ID                         : ",
		gstVehicleParm.abTranspBizrID,
		sizeof( gstVehicleParm.abTranspBizrID ) );
	DebugOutlnASC	( "���������� ID                         : ",
		gstVehicleParm.abBusBizOfficeID,
		sizeof( gstVehicleParm.abBusBizOfficeID ) );
	DebugOutlnASC	( "���� ID                               : ",
		gstVehicleParm.abVehicleID, sizeof( gstVehicleParm.abVehicleID ) );
	DebugOutlnASC	( "������ȣ                              : ",
		gstVehicleParm.abVehicleNo, sizeof( gstVehicleParm.abVehicleNo ) );
	DebugOutlnASC	( "�뼱 ID                               : ",
		gstVehicleParm.abRouteID, sizeof( gstVehicleParm.abRouteID ) );
	DebugOutlnASC	( "������ ID                             : ",
		gstVehicleParm.abDriverID, sizeof( gstVehicleParm.abDriverID ) );
	DebugOutlnASC	( "��������ڵ� - byte �迭              : ",
		gstVehicleParm.abTranspMethodCode,
		sizeof( gstVehicleParm.abTranspMethodCode ) );
	DebugOut		( "��������ڵ� - word                   : %u\n",
		gstVehicleParm.wTranspMethodCode );
	DebugOutlnASC	( "����ڹ�ȣ                            : ",
		gstVehicleParm.abBizNo, sizeof( gstVehicleParm.abBizNo ) );
	DebugOutlnASC	( "�������ڸ�                          : ",
		gstVehicleParm.abTranspBizrNm,
		sizeof( gstVehicleParm.abTranspBizrNm ) );
	DebugOutlnASC	( "�ּ�                                  : ",
		gstVehicleParm.abAddr, sizeof( gstVehicleParm.abAddr ) );
	DebugOutlnASC	( "��ǥ�ڸ�                              : ",
		gstVehicleParm.abRepreNm, sizeof( gstVehicleParm.abRepreNm ) );

	printf( "[LoadInfo] ��������           [%s] �ε� ����\n", VEHICLE_PARM_FILE );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadRouteParm                                            *
*                                                                              *
*  DESCRIPTION :      �뼱�� �Ķ���� ������ �ε��Ѵ�.		                   *
*                                                                              *
*  INPUT PARAMETERS:  ROUTE_PARM *pstRouteParmInfo                             *
*                       - ������ ���۱⿡���� ���Ǵ� ����                      *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( ROUTE_PARM_FILE )           *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadRouteParm( void )
{
	int i = 0;								// LEAP �н����� ������
	int nReadSize = 0;
	FILE *fdRouteParm = NULL;				// ���� ��ũ����
	TEMP_ROUTE_PARM	stTmpRouteParm;			// �뼱���Ķ���� ����ü( read�� )

	memset( &stTmpRouteParm, 0x00, sizeof( TEMP_ROUTE_PARM ) );
	memset( &gstRouteParm, 0x00, sizeof( ROUTE_PARM ) );

	/*
	 * �뼱�������� OPEN
	 */
	fdRouteParm = fopen( ROUTE_PARM_FILE, "rb" );
	if ( fdRouteParm == NULL )
	{
//		printf( "[LoadInfo] �뼱���� �ε� ���� (OPEN)\n" );
		return ErrRet( ERR_FILE_OPEN | GetFileNo( ROUTE_PARM_FILE ) );
	}

	/*
	 * �뼱�������� READ
	 */
	nReadSize = fread( &stTmpRouteParm,
		sizeof( stTmpRouteParm ),
		1,
		fdRouteParm );

    fclose( fdRouteParm );

	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] �뼱���� �ε� ���� (���ڵ� READ)\n" );
		return ErrRet( ERR_FILE_READ | GetFileNo( ROUTE_PARM_FILE ) );
	}

	/*
	 * �뼱���� ����
	 */
	// 1. �����Ͻ�
	memcpy( gstRouteParm.abApplyDtime, stTmpRouteParm.abApplyDtime,
		sizeof( gstRouteParm.abApplyDtime ) );

	// 2. �뼱ID
	memcpy( gstRouteParm.abRouteID, stTmpRouteParm.abRouteID,
		sizeof( gstRouteParm.abRouteID ) );

	// 3. ��������ڵ� - byte �迭
	memcpy( gstRouteParm.abTranspMethodCode, stTmpRouteParm.abTranspMethodCode,
		sizeof( gstRouteParm.abTranspMethodCode ) );

	// 4. ��������ڵ� - word
	gstRouteParm.wTranspMethodCode =
		GetWORDFromASC( stTmpRouteParm.abTranspMethodCode,
			sizeof( stTmpRouteParm.abTranspMethodCode ) );

	// 5. ȯ����������Ƚ�� (�̻��)
	memcpy( gstRouteParm.abXferDisApplyFreq, stTmpRouteParm.abXferDisApplyFreq,
		sizeof( gstRouteParm.abXferDisApplyFreq ) );

	// 6. ȯ������ð� (�̻��)
	memcpy( gstRouteParm.abXferApplyTime, stTmpRouteParm.abXferApplyTime,
		sizeof( gstRouteParm.abXferApplyTime ) );

	// 7. �ܸ���׷��ڵ� (�̻��)
	memcpy( gstRouteParm.abTermGroupCode, stTmpRouteParm.abTermGroupCode,
		sizeof( gstRouteParm.abTermGroupCode ) );

	// 8. �����ܸ������Ű��� (�̻��)
	memcpy( gstRouteParm.abSubTermCommIntv, stTmpRouteParm.abSubTermCommIntv,
		sizeof( gstRouteParm.abSubTermCommIntv ) );

	// 9. ī��ó���ð����� (�̻��)
	memcpy( gstRouteParm.abCardProcTimeIntv, stTmpRouteParm.abCardProcTimeIntv,
		sizeof( gstRouteParm.abCardProcTimeIntv ) );

	// 10. ����BL��ȿ�Ⱓ (�̻��)
	memcpy( gstRouteParm.abUpdateBLValidPeriod,
		stTmpRouteParm.abUpdateBLValidPeriod,
		sizeof( gstRouteParm.abUpdateBLValidPeriod ) );

	// 11. ����PL��ȿ�Ⱓ (�̻��)
	memcpy( gstRouteParm.abUpdatePLValidPeriod,
		stTmpRouteParm.abUpdatePLValidPeriod,
		sizeof( gstRouteParm.abUpdatePLValidPeriod ) );

	// 12. ����AI��ȿ�Ⱓ (�̻��)
	memcpy( gstRouteParm.abUpdateAIValidPeriod,
		stTmpRouteParm.abUpdateAIValidPeriod,
		sizeof( stTmpRouteParm.abUpdateAIValidPeriod ) );

	// 13. ������ī�������� (�̻��)
	gstRouteParm.bDriverCardUseYN = stTmpRouteParm.bDriverCardUseYN;

	// 14. ������������Ƚ�� (�̻��)
	memcpy( gstRouteParm.abChargeInfoSendFreq,
		stTmpRouteParm.abChargeInfoSendFreq,
		sizeof( gstRouteParm.abChargeInfoSendFreq ) );

	// 15. ���������ڵ� - ���������۱�����
	gstRouteParm.bRunKindCode = stTmpRouteParm.bRunKindCode;

	// 16. �����õ��������Է¹�� - ���������۱�����
	gstRouteParm.bGyeonggiIncheonRangeFareInputWay =
		stTmpRouteParm.bGyeonggiIncheonRangeFareInputWay;

	// 17. LEAP ��ȣ (�̻��)
	//     - gpaLEAPPasswd���� �������� ����
	memset( gstRouteParm.abLEAPPasswd, 0x00,
		sizeof( gstRouteParm.abLEAPPasswd ) );

	for ( i = 0 ; i < sizeof( stTmpRouteParm.abLEAPPasswd ) ; i++ )
	{
		if ( stTmpRouteParm.abLEAPPasswd[i] == 0xff )
		{
			break;
		}
	}

	BCD2ASC( stTmpRouteParm.abLEAPPasswd, gstRouteParm.abLEAPPasswd, i );

	// 18. �ĺ�ecard ������� (�̻��)
	gstRouteParm.bECardUseYN = stTmpRouteParm.bECardUseYN;

	DebugOutlnASC	( "�����Ͻ�                              : ",
		gstRouteParm.abApplyDtime, sizeof( gstRouteParm.abApplyDtime ) );
	DebugOutlnASC	( "�뼱ID                                : ",
		gstRouteParm.abRouteID, sizeof( gstRouteParm.abRouteID ) );
	DebugOutlnASC	( "��������ڵ� - byte �迭              : ",
		gstRouteParm.abTranspMethodCode,
		sizeof( gstRouteParm.abTranspMethodCode ) );
	DebugOut     	( "��������ڵ� - word                   : %u\n",
		gstRouteParm.wTranspMethodCode );
	DebugOutlnASC	( "ȯ����������Ƚ�� (�̻��)             : ",
		gstRouteParm.abXferDisApplyFreq,
		sizeof( gstRouteParm.abXferDisApplyFreq ) );
	DebugOutlnASC	( "ȯ������ð� (�̻��)                 : ",
		gstRouteParm.abXferApplyTime,
		sizeof( gstRouteParm.abXferApplyTime ) );
	DebugOutlnASC	( "�ܸ���׷��ڵ� (�̻��)               : ",
		gstRouteParm.abTermGroupCode, sizeof( gstRouteParm.abTermGroupCode ) );
	DebugOutlnASC	( "�����ܸ������Ű��� (�̻��)         : ",
		gstRouteParm.abSubTermCommIntv,
		sizeof( gstRouteParm.abSubTermCommIntv ) );
    DebugOutlnASC	( "ī��ó���ð����� (�̻��)             : ",
		gstRouteParm.abCardProcTimeIntv,
		sizeof( gstRouteParm.abCardProcTimeIntv ) );
	DebugOutlnASC	( "����BL��ȿ�Ⱓ (�̻��)               : ",
		gstRouteParm.abUpdateBLValidPeriod,
		sizeof( gstRouteParm.abUpdateBLValidPeriod ) );
	DebugOutlnASC	( "����PL��ȿ�Ⱓ (�̻��)               : ",
		gstRouteParm.abUpdatePLValidPeriod,
		sizeof( gstRouteParm.abUpdatePLValidPeriod ) );
	DebugOutlnASC	( "����AI��ȿ�Ⱓ (�̻��)               : ",
		gstRouteParm.abUpdateAIValidPeriod,
		sizeof( gstRouteParm.abUpdateAIValidPeriod ) );
	DebugOut		( "������ī�������� (�̻��)           : %u\n",
		gstRouteParm.bDriverCardUseYN );
	DebugOutlnASC	( "������������Ƚ�� (�̻��)             : ",
		gstRouteParm.abChargeInfoSendFreq,
		sizeof( gstRouteParm.abChargeInfoSendFreq ) );
	DebugOut		( "���������ڵ� - ���������۱�����       : %u\n",
		gstRouteParm.bRunKindCode );
	DebugOut		( "�����õ��������Է¹�� - ��������   : %u\n",
		gstRouteParm.bGyeonggiIncheonRangeFareInputWay );
    DebugOutlnASC	( "LEAP ��ȣ (�̻��)                    : ",
		gstRouteParm.abLEAPPasswd, sizeof( gstRouteParm.abLEAPPasswd ) );
    DebugOut		( "�ĺ�ecard ������� (�̻��)           : %u\n",
		gstRouteParm.bECardUseYN );

    printf( "[LoadInfo] �뼱����           [%s] �ε� ����\n", ROUTE_PARM_FILE );

    return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadPrepayIssuerInfo                                     *
*                                                                              *
*  DESCRIPTION :      �����ҹ���� ���� ������ �ε��Ѵ�.					   *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS,                                               *
*                       ERR_FILE_OPEN | GetFileNo( PREPAY_ISSUER_INFO_FILE )   *
*                       ERR_FILE_READ | GetFileNo( PREPAY_ISSUER_INFO_FILE )   *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short LoadPrepayIssuerInfo( void )
{
	short	sResult		= SUCCESS;
	FILE	*fdInfo		= NULL;
	dword	i			= 0;
	int		nReadSize	= 0;

	PREPAY_ISSUER_INFO					*pstPrepayIssuerInfo = NULL;
	TEMP_PREPAY_ISSUER_INFO_HEADER		stInfoHeader;
	TEMP_PREPAY_ISSUER_INFO				stInfo;

	/*
	 * ���ҹ������������ OPEN
	 */
	fdInfo = fopen( PREPAY_ISSUER_INFO_FILE, "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] ���ҹ�������� �ε� ���� (OPEN)\n" );
		sResult = ERR_FILE_OPEN | GetFileNo( PREPAY_ISSUER_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * ���ҹ������������ ��� READ
	 */
	nReadSize = fread( &stInfoHeader,
		sizeof( stInfoHeader ),
		1,
		fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] ���ҹ�������� �ε� ���� (��� READ)\n" );
		sResult = ERR_FILE_READ | GetFileNo( PREPAY_ISSUER_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * ���ҹ�������� ��� ����
	 */
	gstPrepayIssuerInfoHeader.tApplyDtime =
		GetTimeTFromBCDDtime( stInfoHeader.abApplyDtime );
	gstPrepayIssuerInfoHeader.bApplySeqNo = stInfoHeader.bApplySeqNo;
	gstPrepayIssuerInfoHeader.dwRecordCnt =
		GetDWORDFromASC( stInfoHeader.abRecordCnt,
			sizeof( stInfoHeader.abRecordCnt ) );

	DebugOutlnTimeT ( "�����Ͻ�                              : ",
		gstPrepayIssuerInfoHeader.tApplyDtime );
	DebugOut        ( "�����Ϸù�ȣ                          : %u\n",
		gstPrepayIssuerInfoHeader.bApplySeqNo );
	DebugOut        ( "���ڵ� �Ǽ�                           : %lu\n",
		gstPrepayIssuerInfoHeader.dwRecordCnt );

	/*
	 * ���ҹ�������� �޸� ���� ALLOC
	 */
	pstPrepayIssuerInfo =
		( PREPAY_ISSUER_INFO * )malloc( sizeof( PREPAY_ISSUER_INFO ) *
			gstPrepayIssuerInfoHeader.dwRecordCnt );
	if ( pstPrepayIssuerInfo == NULL )
	{
		printf( "[LoadInfo] ���ҹ�������� �ε� ���� (�޸𸮰��� ALLOC)\n" );
		sResult = ERR_FILE_READ | GetFileNo( PREPAY_ISSUER_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * ���ҹ�������� ����
	 */
	for ( i = 0; i < gstPrepayIssuerInfoHeader.dwRecordCnt; i++ )
	{
		nReadSize = fread( &stInfo, sizeof( stInfo ), 1, fdInfo );
		if ( nReadSize != 1 )
		{
			printf( "[LoadInfo] ���ҹ�������� �ε� ���� (���ڵ� READ)\n" );
			sResult = ERR_FILE_READ | GetFileNo( PREPAY_ISSUER_INFO_FILE );
			goto FINALLY;
		}

		memcpy( pstPrepayIssuerInfo[i].abPrepayIssuerID,
			stInfo.abPrepayIssuerID,
			sizeof( pstPrepayIssuerInfo[i].abPrepayIssuerID ) );
		pstPrepayIssuerInfo[i].bAssocCode = stInfo.bAssocCode;
		pstPrepayIssuerInfo[i].bXferDisYoungCardApply =
			stInfo.bXferDisYoungCardApply;

		DebugOutlnASC   ( "���� ����� ID                        : ",
			pstPrepayIssuerInfo[i].abPrepayIssuerID,
			sizeof( pstPrepayIssuerInfo[i].abPrepayIssuerID ));
		DebugOut        ( "�ܸ��� �Ҽ� ���� �����ڵ�             : '%c'\n",
			pstPrepayIssuerInfo[i].bAssocCode );
		DebugOut        ( "ȯ������/�л�ī�� ��������            : '%c'\n",
			pstPrepayIssuerInfo[i].bXferDisYoungCardApply );
    }

	FINALLY:

	if ( fdInfo != NULL )
	{
		fclose( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * ���� �ε�� ���ҹ���������� FREE
		 */
	    free( gpstPrepayIssuerInfo );

		/*
		 * ���� �ε�� ������ �����Ϳ� ����
		 */
	    gpstPrepayIssuerInfo = pstPrepayIssuerInfo;

		printf( "[LoadInfo] ���ҹ��������     [%s] �ε� ����\n", PREPAY_ISSUER_INFO_FILE );
	}
	else
	{
		free( gpstPrepayIssuerInfo );
		free( pstPrepayIssuerInfo );
		memset( &gstPrepayIssuerInfoHeader, 0x00,
			sizeof( PREPAY_ISSUER_INFO_HEADER ) );
		gpstPrepayIssuerInfo = NULL;
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadPostpayIssuerInfo                                    *
*                                                                              *
*  DESCRIPTION :      �ĺҹ���� ���� ������ �ε��Ѵ�.					       *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS,                                               *
*                       ERR_FILE_OPEN | GetFileNo( POSTPAY_ISSUER_INFO_FILE )  *
*                       ERR_FILE_READ | GetFileNo( POSTPAY_ISSUER_INFO_FILE )  *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short LoadPostpayIssuerInfo( void )
{
	short	sResult		= SUCCESS;
	FILE	*fdInfo		= NULL;
	dword	i			= 0;
	int		nReadSize	= 0;

	POSTPAY_ISSUER_INFO				*pstPostpayIssuerInfo = NULL;
	TEMP_POSTPAY_ISSUER_INFO_HEADER	stInfoHeader;
	TEMP_POSTPAY_ISSUER_INFO		stInfo;

	/*
	 * �ĺҹ������������ OPEN
	 */
	fdInfo = fopen( POSTPAY_ISSUER_INFO_FILE, "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] �ĺҹ�������� �ε� ���� (OPEN)\n" );
		sResult = ERR_FILE_OPEN | GetFileNo( POSTPAY_ISSUER_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * �ĺҹ������������ ��� READ
	 */
	nReadSize = fread( &stInfoHeader,
		sizeof( stInfoHeader ),
		1,
		fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] �ĺҹ�������� �ε� ���� (��� READ)\n" );
		sResult = ERR_FILE_READ | GetFileNo( POSTPAY_ISSUER_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * �ĺҹ�������� ��� ����
	 */
	gstPostpayIssuerInfoHeader.tApplyDtime =
		GetTimeTFromBCDDtime( stInfoHeader.abApplyDtime );
	gstPostpayIssuerInfoHeader.wRecordCnt = stInfoHeader.wRecordCnt;

	DebugOutlnTimeT ( "�����Ͻ�                              : ",
		gstPostpayIssuerInfoHeader.tApplyDtime );
	DebugOut        ( "���ڵ� �Ǽ�                           : %u\n",
		gstPostpayIssuerInfoHeader.wRecordCnt );

	/*
	 * �ĺҹ�������� �޸� ���� ALLOC
	 */
	pstPostpayIssuerInfo =
		( POSTPAY_ISSUER_INFO *)malloc( sizeof( POSTPAY_ISSUER_INFO ) *
			gstPostpayIssuerInfoHeader.wRecordCnt );
	if ( pstPostpayIssuerInfo == NULL )
	{
		printf( "[LoadInfo] �ĺҹ�������� �ε� ���� (�޸𸮰��� ALLOC)\n" );
		sResult = ERR_FILE_READ | GetFileNo( POSTPAY_ISSUER_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * �ĺҹ�������� ����
	 */
	for ( i = 0; i < gstPostpayIssuerInfoHeader.wRecordCnt; i++ )
	{
		nReadSize = fread( &stInfo, sizeof( stInfo ), 1, fdInfo );
		if ( nReadSize != 1 )
		{
			printf( "[LoadInfo] �ĺҹ�������� �ε� ���� (���ڵ� READ)\n" );
			sResult = ERR_FILE_READ | GetFileNo( POSTPAY_ISSUER_INFO_FILE );
			goto FINALLY;
		}

		BCD2ASC( stInfo.abPrefixNo, pstPostpayIssuerInfo[i].abPrefixNo,
			sizeof( stInfo.abPrefixNo ) );
		pstPostpayIssuerInfo[i].wCompCode = stInfo.wCompCode;

		DebugOutlnASC   ( "Prefix ��ȣ                           : ",
			pstPostpayIssuerInfo[i].abPrefixNo,
			sizeof ( pstPostpayIssuerInfo[i].abPrefixNo ) );
		DebugOut		( "�����ڵ�                              : %u\n",
			pstPostpayIssuerInfo[i].wCompCode );
	}

	FINALLY:

	if ( fdInfo != NULL )
	{
		fclose( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * ���� �ε�� �ĺҹ���������� FREE
		 */
	    free( gpstPostpayIssuerInfo );

		/*
		 * ���� �ε�� ������ �����Ϳ� ����
		 */
	    gpstPostpayIssuerInfo = pstPostpayIssuerInfo;

		printf( "[LoadInfo] �ĺҹ��������     [%s] �ε� ����\n", POSTPAY_ISSUER_INFO_FILE );
	}
	else
	{
		free( gpstPostpayIssuerInfo );
		free( pstPostpayIssuerInfo );
		memset( &gstPostpayIssuerInfoHeader, 0x00,
			sizeof( POSTPAY_ISSUER_INFO_HEADER ) );
		gpstPostpayIssuerInfo = NULL;
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadIssuerValidPeriodInfo                                *
*                                                                              *
*  DESCRIPTION :      �ĺҹ��༭ ��ȿ�Ⱓ ���� ������ �ε��Ѵ�.                *
*                                                                              *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS,                                               *
*          ERR_FILE_OPEN | GetFileNo( ISSUER_VALID_PERIOD_INFO_FILE )          *
*          ERR_FILE_READ | GetFileNo( ISSUER_VALID_PERIOD_INFO_FILE )          *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short LoadIssuerValidPeriodInfo( void )
{
	short	sResult		= SUCCESS;
	FILE	*fdInfo		= NULL;
	dword	i			= 0;
	int		nReadSize	= 0;

	ISSUER_VALID_PERIOD_INFO		*pstIssuerValidPeriodInfo = NULL;
	TEMP_POSTPAY_ISSUER_VALID_PERIOD_INFO_HEADER	stInfoHeader;
	TEMP_POSTPAY_ISSUER_VALID_PERIOD_INFO			stInfo;

	fdInfo = fopen( ISSUER_VALID_PERIOD_INFO_FILE, "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] �������ȿ�Ⱓ���� �ε� ���� (OPEN)\n" );
		sResult =
			ERR_FILE_OPEN | GetFileNo( ISSUER_VALID_PERIOD_INFO_FILE );
		goto FINALLY;
	}

	nReadSize = fread( &stInfoHeader,
		sizeof( stInfoHeader ),
		1,
		fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] �������ȿ�Ⱓ���� �ε� ���� (��� READ)\n" );
		sResult =
			ERR_FILE_READ | GetFileNo( ISSUER_VALID_PERIOD_INFO_FILE );
		goto FINALLY;
	}

	gstIssuerValidPeriodInfoHeader.tApplyDtime =
		GetTimeTFromASCDtime( stInfoHeader.abApplyDtime );
	gstIssuerValidPeriodInfoHeader.wRecordCnt =
		GetWORDFromASC(stInfoHeader.abRecordCnt,
			sizeof( stInfoHeader.abRecordCnt ) );

	DebugOutlnTimeT ( "�����Ͻ�                              : ",
		gstIssuerValidPeriodInfoHeader.tApplyDtime );
	DebugOut        ( "���ڵ� �Ǽ�                           : %u\n",
		gstIssuerValidPeriodInfoHeader.wRecordCnt );

	pstIssuerValidPeriodInfo =
		( ISSUER_VALID_PERIOD_INFO * )malloc(sizeof( ISSUER_VALID_PERIOD_INFO ) *
			gstIssuerValidPeriodInfoHeader.wRecordCnt );
	if ( pstIssuerValidPeriodInfo == NULL )
	{
		printf( "[LoadInfo] �������ȿ�Ⱓ���� �ε� ���� (�޸𸮰��� ALLOC)\n" );
		sResult =
			ERR_FILE_READ | GetFileNo( ISSUER_VALID_PERIOD_INFO_FILE );
		goto FINALLY;
	}

	for ( i = 0; i < gstIssuerValidPeriodInfoHeader.wRecordCnt; i++ )
	{
		nReadSize = fread( &stInfo, sizeof( stInfo ), 1, fdInfo );
		if ( nReadSize != 1 )
		{
			printf( "[LoadInfo] �������ȿ�Ⱓ���� �ε� ���� (���ڵ� READ)\n" );
			sResult =
				ERR_FILE_READ | GetFileNo( ISSUER_VALID_PERIOD_INFO_FILE );
			goto FINALLY;
		}

		memcpy( pstIssuerValidPeriodInfo[i].abPrefixNo,
			stInfo.abPrefixNo,
			sizeof( pstIssuerValidPeriodInfo[i].abPrefixNo ) );
		memcpy( pstIssuerValidPeriodInfo[i].abIssuerID,
			stInfo.abIssuerID,
			sizeof( pstIssuerValidPeriodInfo[i].abIssuerID ) );
		memcpy( pstIssuerValidPeriodInfo[i].abExpiryDate,
			stInfo.abExpiryDate,
			sizeof( pstIssuerValidPeriodInfo[i].abExpiryDate ) );

		DebugOutlnASC	( "Prefix  ��ȣ                          : ",
			pstIssuerValidPeriodInfo[i].abPrefixNo,
			sizeof( pstIssuerValidPeriodInfo[i].abPrefixNo ) );
        DebugOutlnASC	( "�����ID                              : ",
			pstIssuerValidPeriodInfo[i].abIssuerID,
			sizeof( pstIssuerValidPeriodInfo[i].abIssuerID ) );
        DebugOutlnASC	( "ī����ȿ�Ⱓ (YYYYMM)                 : ",
			pstIssuerValidPeriodInfo[i].abExpiryDate,
			sizeof( pstIssuerValidPeriodInfo[i].abExpiryDate ) );
    }

	FINALLY:

	if ( fdInfo != NULL )
	{
		fclose( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * ���� �ε�� �������ȿ�Ⱓ������ FREE
		 */
	    free( gpstIssuerValidPeriodInfo );

		/*
		 * ���� �ε�� ������ �����Ϳ� ����
		 */
	    gpstIssuerValidPeriodInfo = pstIssuerValidPeriodInfo;

		printf( "[LoadInfo] �������ȿ�Ⱓ���� [%s] �ε� ����\n",
			ISSUER_VALID_PERIOD_INFO_FILE );
	}
	else
	{
		free( gpstIssuerValidPeriodInfo );
		free( pstIssuerValidPeriodInfo );
		memset( &gstIssuerValidPeriodInfoHeader, 0x00,
			sizeof( ISSUER_VALID_PERIOD_INFO_HEADER ) );
		gpstIssuerValidPeriodInfo = NULL;
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       LoadXferApplyInfo                                        *
*                                                                              *
*  DESCRIPTION:       ȯ���������� ������ �ε��Ѵ�. ��, �ε忡 �����ϴ� ���   *
*                     ������ ȯ�����������Ǽ��� 0���� ����� �����͸�          *
*                     �ʱ�ȭ�Ѵ�.                                              *
*                                                                              *
*  INPUT PARAMETERS:  ����                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_FILE_OPEN | GetFileNo( XFER_APPLY_INFO_FILE )        *
*                     ERR_FILE_READ | GetFileNo( XFER_APPLY_INFO_FILE )        *
*                                                                              *
*  Author:            Mi Hyun Noh                                              *
*                                                                              *
*  DATE:              2005-09-03                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short LoadXferApplyInfo( void )
{
	short	sResult		= SUCCESS;
	int		fdInfo		= 0;
    dword	i			= 0;
    int		nReadSize	= 0;

	struct stat				fileStatus;			// File Status ����ü
	XFER_APPLY_INFO			*pstXferApplyInfo = NULL;
	TEMP_XFER_APPLY_INFO	stInfo;

	/*
	 * ȯ�������������� OPEN
	 */
	fdInfo = open( XFER_APPLY_INFO_FILE, O_RDWR, OPENMODE );
	if ( fdInfo < 0 )
	{
	    printf( "[LoadInfo] ȯ���������� �ε� ���� (OPEN)\n" );
		sResult = ERR_FILE_OPEN | GetFileNo( XFER_APPLY_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * ȯ�������������� fstat
	 */
    if ( fstat( fdInfo, &fileStatus ) != SUCCESS )
	{
	    printf( "[LoadInfo] ȯ���������� �ε� ���� (fstat)\n" );
		sResult = ERR_FILE_READ | GetFileNo( XFER_APPLY_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * ȯ�������������� ���ڵ� ���� ����
	 */
    gstXferApplyInfoHeader.dwRecordCnt = fileStatus.st_size / sizeof( stInfo );
    if ( gstXferApplyInfoHeader.dwRecordCnt == 0 )
	{
	    printf( "[LoadInfo] ȯ���������� �ε� ���� (�Ǽ� 0)\n" );
		sResult = ERR_FILE_READ | GetFileNo( XFER_APPLY_INFO_FILE );
		goto FINALLY;
	}

	DebugOut		( "ȯ���������� �Ǽ�                     : %lu\n",
		gstXferApplyInfoHeader.dwRecordCnt );

	/*
	 * ȯ���������� �޸� ���� ALLOC
	 */
    pstXferApplyInfo =
    	( XFER_APPLY_INFO *)malloc( sizeof( XFER_APPLY_INFO ) *
    		gstXferApplyInfoHeader.dwRecordCnt );
	if ( pstXferApplyInfo == NULL )
	{
	    printf( "[LoadInfo] ȯ���������� �ε� ���� (�޸𸮰��� ALLOC)\n" );
		sResult = ERR_FILE_READ | GetFileNo( XFER_APPLY_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * ȯ���������� ����
	 */
    for ( i = 0; i < gstXferApplyInfoHeader.dwRecordCnt; i++ )
	{
	    nReadSize = read( fdInfo, &stInfo, sizeof( stInfo ) );
	    if ( nReadSize < sizeof( stInfo ) )
		{
		    printf( "[LoadInfo] ȯ���������� �ε� ���� (���ڵ� READ)\n" );
			sResult = ERR_FILE_READ | GetFileNo( XFER_APPLY_INFO_FILE );
			goto FINALLY;
		}

	    pstXferApplyInfo[i].tApplyDtime =
			GetTimeTFromBCDDtime( stInfo.abApplyDtime );
	    pstXferApplyInfo[i].wXferApplyStartTime =
			GetWORDFromASC( stInfo.abXferApplyStartTime,
				sizeof( stInfo.abXferApplyStartTime ) );
	    pstXferApplyInfo[i].wXferApplyEndTime =
			GetWORDFromASC( stInfo.abXferApplyEndTime,
				sizeof( stInfo.abXferApplyEndTime ) );
	    pstXferApplyInfo[i].bHolidayClassCode = stInfo.bHolidayClassCode;
	    pstXferApplyInfo[i].dwXferEnableTime =
			GetDWORDFromASC( stInfo.abXferEnableTime,
				sizeof( stInfo.abXferEnableTime ) );
	    pstXferApplyInfo[i].wXferEnableCnt =
			GetWORDFromASC( stInfo.abXferEnableCnt,
				sizeof( stInfo.abXferEnableCnt ) );

	    DebugOutlnTimeT ( "�����Ͻ�                              : ",
			pstXferApplyInfo[i].tApplyDtime );
	    DebugOut		( "ȯ��������۽ð�                      : %u\n",
			pstXferApplyInfo[i].wXferApplyStartTime );
        DebugOut		( "ȯ����������ð�                      : %u\n",
			pstXferApplyInfo[i].wXferApplyEndTime );
        DebugOut		( "���ϱ����ڵ�                          : '%c'\n",
			pstXferApplyInfo[i].bHolidayClassCode );
        DebugOut		( "ȯ�°��ɽð�                          : %lu\n",
			pstXferApplyInfo[i].dwXferEnableTime );
        DebugOut		( "ȯ�°���Ƚ��                          : %u\n",
			pstXferApplyInfo[i].wXferEnableCnt );
    }

	FINALLY:

	if ( fdInfo >= 0 )
	{
		close( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * ���� �ε�� ȯ������������ FREE
		 */
	    free( gpstXferApplyInfo );

		/*
		 * ���� �ε�� ������ �����Ϳ� ����
		 */
	    gpstXferApplyInfo = pstXferApplyInfo;

		printf( "[LoadInfo] ȯ����������       [%s] �ε� ����\n", XFER_APPLY_INFO_FILE );
	}
	else
	{
		free( gpstXferApplyInfo );
		free( pstXferApplyInfo );
		memset( &gstXferApplyInfoHeader, 0x00,
			sizeof( XFER_APPLY_INFO_HEADER ) );
		gpstXferApplyInfo = NULL;
	}

    return SUCCESS;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadDisExtraInfo                                         *
*                                                                              *
*  DESCRIPTION :      �������� ���������� �ε��Ѵ�.							   *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( DIS_EXTRA_INFO_FILE )       *
*                       ERR_FILE_READ | GetFileNo( DIS_EXTRA_INFO_FILE )       *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short LoadDisExtraInfo( void )
{
	short	sResult		= SUCCESS;
	FILE	*fdInfo		= NULL;
	dword	i			= 0;
	int		nReadSize	= 0;

	DIS_EXTRA_INFO*				pstDisExtraInfo = NULL;
	TEMP_DIS_EXTRA_INFO_HEADER	stInfoHeader;
	TEMP_DIS_EXTRA_INFO			stInfo;

	fdInfo = fopen( DIS_EXTRA_INFO_FILE, "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] ������������ �ε� ���� (OPEN)\n" );
		sResult = ERR_FILE_OPEN | GetFileNo( DIS_EXTRA_INFO_FILE );
		goto FINALLY;
	}

	nReadSize = fread( &stInfoHeader,
		sizeof( stInfoHeader ),
		1,
		fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] ������������ �ε� ���� (��� READ)\n" );
		sResult = ERR_FILE_READ | GetFileNo( DIS_EXTRA_INFO_FILE );
		goto FINALLY;
	}

	gstDisExtraInfoHeader.tApplyDtime =
		GetTimeTFromBCDDtime( stInfoHeader.abApplyDtime );
	gstDisExtraInfoHeader.bApplySeqNo = stInfoHeader.bApplySeqNo;
	gstDisExtraInfoHeader.dwRecordCnt =
		GetDWORDFromBCD( stInfoHeader.abRecordCnt,
			sizeof( stInfoHeader.abRecordCnt ) );

	DebugOutlnTimeT ( "�����Ͻ�                              : ",
		gstDisExtraInfoHeader.tApplyDtime );
	DebugOut        ( "�����Ϸù�ȣ                          : %u\n",
		gstDisExtraInfoHeader.bApplySeqNo );
	DebugOut        ( "���ڵ� �Ǽ�                           : %lu\n",
		gstDisExtraInfoHeader.dwRecordCnt );

	pstDisExtraInfo = ( DIS_EXTRA_INFO * )malloc( sizeof( DIS_EXTRA_INFO ) *
		gstDisExtraInfoHeader.dwRecordCnt );
	if ( pstDisExtraInfo == NULL )
	{
		printf( "[LoadInfo] ������������ �ε� ���� (�޸𸮰��� ALLOC)\n" );
		sResult = ERR_FILE_READ | GetFileNo( DIS_EXTRA_INFO_FILE );
		goto FINALLY;
	}

	for ( i = 0; i < gstDisExtraInfoHeader.dwRecordCnt; i++ )
	{
		nReadSize = fread( &stInfo, sizeof( stInfo ), 1, fdInfo );
		if ( nReadSize != 1 )
		{
			printf( "[LoadInfo] ������������ �ε� ���� (���ڵ� READ)\n" );
			sResult = ERR_FILE_READ | GetFileNo( DIS_EXTRA_INFO_FILE );
			goto FINALLY;
		}

		pstDisExtraInfo[i].bTranspCardClassCode = stInfo.bTranspCardClassCode;
		memcpy( pstDisExtraInfo[i].abDisExtraTypeID, stInfo.abDisExtraTypeID,
			sizeof( pstDisExtraInfo[i].abDisExtraTypeID ) );
		pstDisExtraInfo[i].bDisExtraApplyCode =
			stInfo.bDisExtraApplyStandardCode;
		pstDisExtraInfo[i].fDisExtraRate =
			(float)GetINTFromASC( &stInfo.abDisExtraRate[0], 3 );
		pstDisExtraInfo[i].fDisExtraRate +=
			( stInfo.abDisExtraRate[4] - ZERO ) * 0.1 +
			( stInfo.abDisExtraRate[5] - ZERO ) * 0.01;
		pstDisExtraInfo[i].nDisExtraAmt =
			GetDWORDFromBCD( stInfo.abDisExtraAmt,
				sizeof( stInfo.abDisExtraAmt ) );

		if ( stInfo.bPositiveNegativeClass == NEGATIVE_CLASS )
		{
			pstDisExtraInfo[i].fDisExtraRate *= -1;
			pstDisExtraInfo[i].nDisExtraAmt *= -1;
		}

		DebugOut		( "����ī�屸���ڵ�                      : '%c'\n",
			pstDisExtraInfo[i].bTranspCardClassCode );
		DebugOutlnASC   ( "������������ID                        : ",
			pstDisExtraInfo[i].abDisExtraTypeID,
			sizeof( pstDisExtraInfo[i].abDisExtraTypeID ) );
		DebugOut		( "����������������ڵ�                  : '%c'\n",
			pstDisExtraInfo[i].bDisExtraApplyCode );
		DebugOut		( "����/������                           : %f\n",
			pstDisExtraInfo[i].fDisExtraRate );
		DebugOut		( "����/���� �ݾ�                        : %d\n",
			pstDisExtraInfo[i].nDisExtraAmt );
	}

	FINALLY:

	if ( fdInfo != NULL )
	{
		fclose( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * ���� �ε�� �������������� FREE
		 */
		free( gpstDisExtraInfo );

		/*
		 * ���� �ε�� ������ �����Ϳ� ����
		 */
		gpstDisExtraInfo = pstDisExtraInfo;

		printf( "[LoadInfo] ������������       [%s] �ε� ����\n", DIS_EXTRA_INFO_FILE );
	}
	else
	{
		free( gpstDisExtraInfo );
		free( pstDisExtraInfo );
		memset( &gstDisExtraInfoHeader, 0x00, sizeof( DIS_EXTRA_INFO_HEADER ) );
		gpstDisExtraInfo = NULL;
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       LoadHolidayInfo                                          *
*                                                                              *
*  DESCRIPTION:       �������� ������ �ε��Ѵ�. ��, �ε忡 �����ϴ� ���       *
*                     ������ ���������Ǽ��� 0���� ����� �����͸� �ʱ�ȭ�Ѵ�.  *
*                                                                              *
*  INPUT PARAMETERS:  ����                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS                                                  *
*                     ERR_FILE_OPEN | GetFileNo( HOLIDAY_INFO_FILE )           *
*                     ERR_FILE_READ | GetFileNo( HOLIDAY_INFO_FILE )           *
*                                                                              *
*  Author:            Mi Hyun Noh                                              *
*                                                                              *
*  DATE:              2005-09-03                                               *
*                                                                              *
*  REMARKS:                                                                    *
*                                                                              *
*******************************************************************************/
static short LoadHolidayInfo( void )
{
	short	sResult		= SUCCESS;
	FILE	*fdInfo		= NULL;
	dword	i 			= 0;
	int		nReadSize   = 0;

	HOLIDAY_INFO 				*pstHolidayInfo = NULL;
	TEMP_HOLIDAY_INFO_HEADER	stInfoHeader;

	/*
	 * ������������ OPEN
	 */
	fdInfo = fopen( HOLIDAY_INFO_FILE, "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] �������� �ε� ���� (OPEN)\n" );
		sResult = ERR_FILE_OPEN | GetFileNo( HOLIDAY_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * ������������ ��� READ
	 */
	nReadSize = fread( &stInfoHeader,
		sizeof( stInfoHeader ),
		1,
		fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] �������� �ε� ���� (��� READ)\n" );
		sResult = ERR_FILE_READ | GetFileNo( HOLIDAY_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * �������� �������ü ����
	 */
	gstHolidayInfoHeader.tApplyDtime =
		GetTimeTFromBCDDtime( stInfoHeader.abApplyDtime );
	gstHolidayInfoHeader.dwRecordCnt =
		GetDWORDFromASC( stInfoHeader.abRecordCnt,
			sizeof( stInfoHeader.abRecordCnt ) );

	DebugOutlnTimeT	( "�������� �����Ͻ�                     : ",
		gstHolidayInfoHeader.tApplyDtime );
	DebugOut		( "�������� �Ǽ�                         : %lu\n",
		gstHolidayInfoHeader.dwRecordCnt );

	/*
	 * �������� �޸� ���� ALLOC
	 */
	pstHolidayInfo = ( HOLIDAY_INFO * )malloc( sizeof( HOLIDAY_INFO )
		* gstHolidayInfoHeader.dwRecordCnt );
	if ( pstHolidayInfo == NULL )
	{
		printf( "[LoadInfo] �������� �ε� ���� (�޸𸮰��� ALLOC)\n" );
		sResult = ERR_FILE_READ | GetFileNo( HOLIDAY_INFO_FILE );
		goto FINALLY;
	}

	/*
	 * �������� ����
	 */
	for ( i = 0; i < gstHolidayInfoHeader.dwRecordCnt; i++ )
	{
		nReadSize = fread( &pstHolidayInfo[i],
			sizeof( HOLIDAY_INFO ),
			1,
			fdInfo );
		if ( nReadSize != 1 )
		{
			printf( "[LoadInfo] �������� �ε� ���� (���ڵ� READ)\n" );
			sResult = ERR_FILE_READ | GetFileNo( HOLIDAY_INFO_FILE );
			goto FINALLY;
		}

		DebugOutlnASC	( "��������ID                            : ",
			pstHolidayInfo[i].abHolidayDate,
			sizeof( pstHolidayInfo[i].abHolidayDate ) );
		DebugOut		( "���ϱ����ڵ�                          : '%c'\n",
			pstHolidayInfo[i].bHolidayClassCode );
	}

	FINALLY:

	if ( fdInfo != NULL )
	{
		fclose( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * ���� �ε�� ���������� FREE
		 */
		free( gpstHolidayInfo );

		/*
		 * ���� �ε�� ������ �����Ϳ� ����
		 */
		gpstHolidayInfo = pstHolidayInfo;

		printf( "[LoadInfo] ��������           [%s] �ε� ����\n", HOLIDAY_INFO_FILE );
	}
	else
	{
		free( gpstHolidayInfo );
		free( pstHolidayInfo );
		memset( &gstHolidayInfoHeader, 0x00, sizeof( HOLIDAY_INFO_HEADER ) );
		gpstHolidayInfo = NULL;
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadNewFareInfo                                          *
*                                                                              *
*  DESCRIPTION :      ������� ������ �ε��Ѵ�.					               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( NEW_FARE_INFO_FILE )        *
*                       ERR_FILE_READ | GetFileNo( NEW_FARE_INFO_FILE )        *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
static short LoadNewFareInfo( void )
{
	FILE	*fdInfo		= NULL;
	int		nReadSize	= 0;

	TEMP_NEW_FARE_INFO	stInfo;

	/*
	 * �ſ���������� OPEN
	 */
	fdInfo = fopen( NEW_FARE_INFO_FILE, "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] �ſ������ �ε� ���� (OPEN)\n" );
		return ErrRet( ERR_FILE_OPEN | GetFileNo( NEW_FARE_INFO_FILE ) );
	}

	/*
	 * �ſ������ ���ڵ� READ
	 */
	nReadSize = fread( &stInfo, sizeof( stInfo ), 1, fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] �ſ������ �ε� ���� (���ڵ� READ)\n" );
		fclose( fdInfo );
		return ErrRet( ERR_FILE_READ | GetFileNo( NEW_FARE_INFO_FILE ) );
	}

	fclose( fdInfo );

	gstNewFareInfo.tApplyDatetime = GetTimeTFromBCDDtime( stInfo.abApplyDtime );
	gstNewFareInfo.bApplySeqNo = stInfo.bApplySeqNo;
	gstNewFareInfo.wTranspMethodCode =
		GetWORDFromASC( stInfo.abTranspMethodCode,
			sizeof( stInfo.abTranspMethodCode ) );
	gstNewFareInfo.dwSingleFare =
		GetDWORDFromBCD( stInfo.abSingleFare, sizeof( stInfo.abSingleFare ) );
	gstNewFareInfo.dwBaseFare =
		GetDWORDFromBCD( stInfo.abBaseFare, sizeof( stInfo.abBaseFare ) );
	gstNewFareInfo.dwBaseDist =
		GetDWORDFromBCD( stInfo.abBaseDist, sizeof( stInfo.abBaseDist ) );
	gstNewFareInfo.dwAddedFare =
		GetDWORDFromBCD( stInfo.abAddedFare, sizeof( stInfo.abAddedFare ) );
	gstNewFareInfo.dwAddedDist =
		GetDWORDFromBCD( stInfo.abAddedDist, sizeof( stInfo.abAddedDist ) );
	gstNewFareInfo.dwOuterCityAddedDist =
		GetDWORDFromBCD( stInfo.abOuterCityAddedDist,
			sizeof( stInfo.abOuterCityAddedDist ) );
	gstNewFareInfo.dwOuterCityAddedDistFare =
		GetDWORDFromBCD( stInfo.abOuterCityAddedDistFare,
			sizeof( stInfo.abOuterCityAddedDistFare ) );
	gstNewFareInfo.dwAdultCashEntFare =
		GetDWORDFromBCD( stInfo.abAdultCashEntFare,
			sizeof( stInfo.abAdultCashEntFare ) );
	gstNewFareInfo.dwChildCashEntFare =
		GetDWORDFromBCD( stInfo.abChildCashEntFare,
			sizeof( stInfo.abChildCashEntFare ) );
	gstNewFareInfo.dwYoungCashEntFare =
		GetDWORDFromBCD( stInfo.abYoungCashEntFare,
			sizeof( stInfo.abYoungCashEntFare ) );
	gstNewFareInfo.dwPermitErrDist =
		GetDWORDFromBCD( stInfo.abPermitErrDist,
			sizeof( stInfo.abPermitErrDist ) );

	DebugOutlnTimeT	( "���� �Ͻ�                             : ",
		gstNewFareInfo.tApplyDatetime );
	DebugOut		( "���� �Ϸù�ȣ                         : %u\n",
		gstNewFareInfo.bApplySeqNo );
	DebugOut		( "��������ڵ�                          : %u\n",
		gstNewFareInfo.wTranspMethodCode );
    DebugOut		( "���Ͽ�� - �̻��                     : %lu\n",
		gstNewFareInfo.dwSingleFare );
    DebugOut		( "�⺻����                              : %lu\n",
		gstNewFareInfo.dwBaseFare );
    DebugOut		( "�⺻�Ÿ�                              : %lu\n",
		gstNewFareInfo.dwBaseDist );
    DebugOut		( "�ΰ�����                              : %lu\n",
		gstNewFareInfo.dwAddedFare );
    DebugOut		( "�ΰ��Ÿ�                              : %lu\n",
		gstNewFareInfo.dwAddedDist );
    DebugOut		( "�ÿܺΰ��Ÿ�                          : %lu\n",
		gstNewFareInfo.dwOuterCityAddedDist );
    DebugOut		( "�ÿܺΰ��Ÿ�����                      : %lu\n",
		gstNewFareInfo.dwOuterCityAddedDistFare );
    DebugOut		( "���� ���ݽ������                     : %lu\n",
		gstNewFareInfo.dwAdultCashEntFare );
    DebugOut		( "��� ���ݽ������                   : %lu\n",
		gstNewFareInfo.dwChildCashEntFare );
    DebugOut		( "û�ҳ� ���ݽ������                   : %lu\n",
		gstNewFareInfo.dwYoungCashEntFare );
    DebugOut		( "�������Ÿ� - �̻��                 : %lu\n",
		gstNewFareInfo.dwPermitErrDist );

	printf( "[LoadInfo] �ſ������         [%s]  �ε� ����\n", NEW_FARE_INFO_FILE );

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadStationInfo                                          *
*                                                                              *
*  DESCRIPTION :      ���� ������ ���� ������ �ε��Ѵ�.			               *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_READ | GetFileNo( BUS_STATION_INFO_FILE ) )   *
*                       ERR_FILE_READ | GetFileNo( BUS_STATION_INFO_FILE )     *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadStationInfo( void )
{
	short	sResult			= SUCCESS;
	FILE*	fdInfo			= NULL;
	int		nRecordCntLoop	= 0;
	int		nReadSize		= 0;
	byte	abTempBuf[11]	= { 0, };

	STATION_INFO				*pstStationInfo = NULL;
	TEMP_STATION_INFO_HEADER	stTmpStationInfoHeader;
	TEMP_STATION_INFO			stTmpStationInfo;

	memset( &gstStationInfoHeader, 0x00, sizeof( gstStationInfoHeader ) );
	memset( &stTmpStationInfoHeader, 0x00, sizeof( stTmpStationInfoHeader ) );

	fdInfo = fopen( BUS_STATION_INFO_FILE,  "rb" );
	if ( fdInfo == NULL )
	{
		printf( "[LoadInfo] ���������� �ε� ���� (OPEN)\n" );
		sResult = ERR_FILE_OPEN | GetFileNo( BUS_STATION_INFO_FILE );
		goto FINALLY;
	}

	nReadSize = fread( &stTmpStationInfoHeader,
		sizeof( stTmpStationInfoHeader ),
		1,
		fdInfo );
	if ( nReadSize != 1 )
	{
		printf( "[LoadInfo] ���������� �ε� ���� (��� READ)\n" );
		sResult = ERR_FILE_READ | GetFileNo( BUS_STATION_INFO_FILE );
		goto FINALLY;
	}

	gstStationInfoHeader.tApplyDtime =
		GetTimeTFromBCDDtime( stTmpStationInfoHeader.abApplyDtime );
	gstStationInfoHeader.bApplySeqNo = stTmpStationInfoHeader.bApplySeqNo;
	memcpy( gstStationInfoHeader.abRouteID, stTmpStationInfoHeader.abRouteID,
		sizeof( gstStationInfoHeader.abRouteID ) );
	memcpy( gstStationInfoHeader.abTranspMethodName,
		stTmpStationInfoHeader.abTranspMethodName,
		sizeof( gstStationInfoHeader.abTranspMethodName ) );
	gstStationInfoHeader.dwRecordCnt =
		GetDWORDFromASC( stTmpStationInfoHeader.abRecordCnt,
			sizeof( stTmpStationInfoHeader.abRecordCnt ) );

	DebugOutlnTimeT ( "�����Ͻ�                              : ",
		gstStationInfoHeader.tApplyDtime );
	DebugOut        ( "�����Ϸù�ȣ                          : %u\n",
		gstStationInfoHeader.bApplySeqNo );
	DebugOutlnASC   ( "�����뼱 ID                           : ",
		gstStationInfoHeader.abRouteID,
		sizeof( gstStationInfoHeader.abRouteID ) );
	DebugOutlnASC   ( "�����뼱��                            : ",
		gstStationInfoHeader.abTranspMethodName,
		sizeof( gstStationInfoHeader.abTranspMethodName ) );
	DebugOut        ( "���ڵ� �Ǽ�                           : %lu\n",
		gstStationInfoHeader.dwRecordCnt );
	DebugOut		( "\n" );

	pstStationInfo = ( STATION_INFO * )malloc( sizeof( STATION_INFO ) *
		gstStationInfoHeader.dwRecordCnt );
	if ( pstStationInfo == NULL )
	{
		printf( "[LoadInfo] ���������� �ε� ���� (�޸𸮰��� ALLOC)\n" );
		sResult = ERR_FILE_READ | GetFileNo( BUS_STATION_INFO_FILE );
		goto FINALLY;
	}

	for ( nRecordCntLoop = 0;
		nRecordCntLoop < gstStationInfoHeader.dwRecordCnt; nRecordCntLoop++ )
	{
		nReadSize = fread( &stTmpStationInfo,
			sizeof( stTmpStationInfo ),
			1,
			fdInfo );
		if ( nReadSize != 1 )
		{
			printf( "[LoadInfo] ���������� �ε� ���� (���ڵ� READ)\n" );
			sResult = ERR_FILE_READ | GetFileNo( BUS_STATION_INFO_FILE );
			goto FINALLY;
		}

		// ������ �������� ���
		if ( nRecordCntLoop == gstStationInfoHeader.dwRecordCnt - 1  )
		{
			memcpy( gabEndStationID, stTmpStationInfo.abStationID,
				sizeof( gabEndStationID ) );
		}

		// ���������� ID
		memcpy( pstStationInfo[nRecordCntLoop].abStationID,
			stTmpStationInfo.abStationID,
			sizeof( pstStationInfo[nRecordCntLoop].abStationID ) );

		// �ð賻�ܱ��� �ڵ�
		pstStationInfo[nRecordCntLoop].bCityInOutClassCode =
			stTmpStationInfo.bCityInOutClassCode;

		// ���������
		pstStationInfo[nRecordCntLoop].wStationOrder =
			GetWORDFromASC( stTmpStationInfo.abStationOrder,
				sizeof( stTmpStationInfo.abStationOrder ) );

		// �����������
		memcpy( pstStationInfo[nRecordCntLoop].abStationName,
			stTmpStationInfo.abStationName,
			sizeof( pstStationInfo[nRecordCntLoop].abStationName ) );

		// ���������� �浵
		memset( abTempBuf, 0, sizeof( abTempBuf ) );
		memcpy( abTempBuf,
			stTmpStationInfo.abStationLongitude,
			sizeof( stTmpStationInfo.abStationLongitude ) );
		pstStationInfo[nRecordCntLoop].dStationLongitude = atof( abTempBuf );

		// ���������� ����
		memset( abTempBuf, 0, sizeof( abTempBuf ) );
		memcpy( abTempBuf,
			stTmpStationInfo.abStationLatitude,
			sizeof( stTmpStationInfo.abStationLatitude ) );
		pstStationInfo[nRecordCntLoop].dStationLatitude = atof( abTempBuf );

		// offset
		pstStationInfo[nRecordCntLoop].wOffset =
		   GetWORDFromASC( stTmpStationInfo.abOffset,
			   sizeof( stTmpStationInfo.abOffset ) );

		// ù�����忡���� �Ÿ�
		pstStationInfo[nRecordCntLoop].dwDistFromFirstStation =
			GetDWORDFromBCD( stTmpStationInfo.abDistFromFirstStation,
				 sizeof( stTmpStationInfo.abDistFromFirstStation ) );

		// ������ ���԰�
		pstStationInfo[nRecordCntLoop].wStationApproachAngle =
		   GetWORDFromASC( stTmpStationInfo.abStationApproachAngle,
			   sizeof( stTmpStationInfo.abStationApproachAngle ) );

		DebugOutlnASC	( "���������� ID                         : ",
			pstStationInfo[nRecordCntLoop].abStationID,
			sizeof( pstStationInfo[nRecordCntLoop].abStationID ) );
		DebugOut		( "�ð賻�ܱ��� �ڵ�                     : '%c'\n",
			pstStationInfo[nRecordCntLoop].bCityInOutClassCode );
		DebugOut		( "���������                            : %u\n",
			pstStationInfo[nRecordCntLoop].wStationOrder );
		DebugOutlnASC	( "�����������                          : ",
			pstStationInfo[nRecordCntLoop].abStationName,
			sizeof( pstStationInfo[nRecordCntLoop].abStationName ) );
		DebugOut		( "���������� �浵                       : %f\n",
			pstStationInfo[nRecordCntLoop].dStationLongitude );
		DebugOut		( "���������� ����                       : %f\n",
			pstStationInfo[nRecordCntLoop].dStationLatitude );
		DebugOut		( "offset                                : %u\n",
			pstStationInfo[nRecordCntLoop].wOffset );
		DebugOut		( "ù�����忡���� �Ÿ�                   : %lu\n",
			pstStationInfo[nRecordCntLoop].dwDistFromFirstStation );
		DebugOut		( "������ ���԰�                         : %u\n",
			pstStationInfo[nRecordCntLoop].wStationApproachAngle );
	}

	FINALLY:

	if ( fdInfo != NULL )
	{
		fclose( fdInfo );
	}

	if ( sResult == SUCCESS )
	{
		/*
		 * ���� �ε�� ������������ FREE
		 */
		free( gpstStationInfo );

		/*
		 * ���� �ε�� ������ �����Ϳ� ����
		 */
		gpstStationInfo = pstStationInfo;

		printf( "[LoadInfo] ����������         [%s] �ε� ����\n", BUS_STATION_INFO_FILE );
	}
	else
	{
		free( gpstStationInfo );
		free( pstStationInfo );
		memset( &gstStationInfoHeader, 0x00, sizeof( STATION_INFO_HEADER ) );
		gpstStationInfo = NULL;
	}

	return sResult;
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      LoadInstallInfo                                          *
*                                                                              *
*  DESCRIPTION :      setup.dat�� ��ϵ� ����ý��� IP�� ���� ID�� �ε��Ѵ�.   *
*                                                                              *
*  INPUT PARAMETERS:  none                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( SETUP_BACKUP_FILE )         *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short LoadInstallInfo( void )
{
	FILE* fdSetup = NULL;
	int nReadSize = 0;
	int nIP1 = 0;		// IP first group
	int nIP2 = 0;		// IP second group
	int nIP3 = 0;		// IP third group
	int nIP4 = 0;		// IP fourth group

	fdSetup = fopen( SETUP_FILE, "rb" );
	if ( fdSetup == NULL )
	{
		fdSetup = fopen( SETUP_BACKUP_FILE, "rb" );
        if ( fdSetup == NULL )
        {
            return ErrRet( ERR_FILE_OPEN | GetFileNo( SETUP_BACKUP_FILE ) );
        }
	}

	nReadSize = fread( &gstCommInfo, sizeof( gstCommInfo ), 1, fdSetup );

	fclose(fdSetup);

	if ( nReadSize != 1 )
	{
		return ErrRet( ERR_FILE_READ | GetFileNo( SETUP_BACKUP_FILE ) );
	}

	nIP1 = GetINTFromASC( &gstCommInfo.abDCSIPAddr[0], IP_CLASS_LENGTH );
	nIP2 = GetINTFromASC( &gstCommInfo.abDCSIPAddr[3], IP_CLASS_LENGTH );
	nIP3 = GetINTFromASC( &gstCommInfo.abDCSIPAddr[6], IP_CLASS_LENGTH );
	nIP4 = GetINTFromASC( &gstCommInfo.abDCSIPAddr[9], IP_CLASS_LENGTH );

	memset( gabDCSIPAddr, 0x00, sizeof( gabDCSIPAddr ) );
	sprintf( gabDCSIPAddr, "%d.%d.%d.%d", nIP1, nIP2, nIP3, nIP4 );

	memcpy( gstVehicleParm.abVehicleID, gstCommInfo.abVehicleID,
		sizeof( gstVehicleParm.abVehicleID ) );

	DebugOutlnASC	( "����ID                                : ",
		gstCommInfo.abVehicleID,
		sizeof( gstCommInfo.abVehicleID ) );
	DebugOutlnASC	( "���輭��IP                            : ",
		gstCommInfo.abDCSIPAddr,
		sizeof( gstCommInfo.abDCSIPAddr ) );
	DebugOutlnASC	( "���輭��IP                            : ",
		gabDCSIPAddr, sizeof( gabDCSIPAddr ) );

	printf( "[LoadInfo] ��ġ����           [%s]    �ε� ����\n", SETUP_FILE );

	return SUCCESS;
}


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      GetLEAPPasswd                                            *
*                                                                              *
*  DESCRIPTION :      This program gets LEAP password.                         *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE:   SUCCESS                                                *
*                       ERR_FILE_OPEN | GetFileNo( TC_LEAP_FILE )              *
*                                                                              *
*  Author  : Mi Hyun Noh                                                       *
*                                                                              *
*  DATE    : 2005-09-03                                                        *
*                                                                              *
*  REMARKS :                                                                   *
*                                                                              *
*******************************************************************************/
short GetLEAPPasswd( void )
{

    byte    abFileName[30] 	= { 0,};
    int     nLoopCnt    	= 0;
    int     nTmpLoop    	= 0;

    FILE    	*fdSetup;
	LEAP_PASSWD	stLeapPasswd;

    strcpy( abFileName, TC_LEAP_FILE );
    memset( &stLeapPasswd, 0x00, sizeof( stLeapPasswd ) );

    /*
     *  tc_leap.dat�� �дµ� �����ϸ� tc_leap.backup�� �д´�
     */
    do
    {
        fdSetup = fopen( abFileName, "rb" );

        if ( fdSetup != NULL )
        {

            if ( fread( &stLeapPasswd,
                        sizeof( stLeapPasswd ),
                        1,
                        fdSetup
                      ) == 1 )
            {
                for ( nTmpLoop = 0 ;
                      nTmpLoop < sizeof( stLeapPasswd.abLEAPPasswd ) ;
                      nTmpLoop++ )
                {
                    if ( stLeapPasswd.abLEAPPasswd[nTmpLoop] == 0xff )
                    {
                        break;
                    }
                }

                memset( gabLEAPPasswd, 0x00, sizeof( gabLEAPPasswd ) );
                memcpy( gabLEAPPasswd, stLeapPasswd.abLEAPPasswd, nTmpLoop );

                fclose( fdSetup );

                return SUCCESS;
            }

            fclose( fdSetup );

        }

        /*
         *  tc_leap.dat�� open ���� ��
         */
        memset( abFileName, 0x00, sizeof( abFileName ) );
        strncpy( abFileName,
                 TC_LEAP_BACKUP_FILE,
                 sizeof( TC_LEAP_BACKUP_FILE ) );
        nLoopCnt++;

    } while ( nLoopCnt < 2 );

    return ErrRet( ERR_FILE_OPEN | GetFileNo( TC_LEAP_FILE ) );
}

static short LoadHardCodedNewFareInfo( void )
{
	memset( &gstNewFareInfo, 0x00, sizeof( NEW_FARE_INFO ) );

	/*
	 *  ��������ڵ�
	 */
	gstNewFareInfo.wTranspMethodCode = gstVehicleParm.wTranspMethodCode;

	/*
	 *  ���Ͽ�� (�̻��)
	 */
	gstNewFareInfo.dwSingleFare =
		GetHardCodedBaseFare( gstVehicleParm.wTranspMethodCode );

	/*
	 *  �⺻����
	 */
	gstNewFareInfo.dwBaseFare =
		GetHardCodedBaseFare( gstVehicleParm.wTranspMethodCode );

	/*
	 *  �⺻�Ÿ�
	 */
	gstNewFareInfo.dwBaseDist = 10000;

	/*
	 *  �ΰ�����
	 */
	gstNewFareInfo.dwAddedFare = 100;

	/*
	 *  �ΰ��Ÿ�
	 */
	gstNewFareInfo.dwAddedDist = 5000;

	/*
	 *  �ÿܺΰ��Ÿ� (�̻��)
	 */
	gstNewFareInfo.dwOuterCityAddedDist = 0;

	/*
	 *  �ÿܺΰ��Ÿ����� (�̻��)
	 */
	gstNewFareInfo.dwOuterCityAddedDistFare = 0;

	/*
	 *  ���� ���ݽ������
	 */
	gstNewFareInfo.dwAdultCashEntFare = 0;

	/*
	 *  ��� ���ݽ������
	 */
	gstNewFareInfo.dwChildCashEntFare = 0;

	/*
	 *  û�ҳ� ���ݽ������
	 */
	gstNewFareInfo.dwYoungCashEntFare = 0;

	/*
	 *  �������Ÿ� (�̻��)
	 */
	gstNewFareInfo.dwPermitErrDist = 0;

	PrintlnTimeT	( "���� �Ͻ�                             : ",
		gstNewFareInfo.tApplyDatetime );
	printf			( "���� �Ϸù�ȣ                         : %u\n",
		gstNewFareInfo.bApplySeqNo );
	printf			( "��������ڵ�                          : %u\n",
		gstNewFareInfo.wTranspMethodCode );
    printf			( "���Ͽ�� - �̻��                     : %lu\n",
		gstNewFareInfo.dwSingleFare );
    printf			( "�⺻����                              : %lu\n",
		gstNewFareInfo.dwBaseFare );
    printf			( "�⺻�Ÿ�                              : %lu\n",
		gstNewFareInfo.dwBaseDist );
    printf			( "�ΰ�����                              : %lu\n",
		gstNewFareInfo.dwAddedFare );
    printf			( "�ΰ��Ÿ�                              : %lu\n",
		gstNewFareInfo.dwAddedDist );
    printf			( "�ÿܺΰ��Ÿ�                          : %lu\n",
		gstNewFareInfo.dwOuterCityAddedDist );
    printf			( "�ÿܺΰ��Ÿ�����                      : %lu\n",
		gstNewFareInfo.dwOuterCityAddedDistFare );
    printf			( "���� ���ݽ������                     : %lu\n",
		gstNewFareInfo.dwAdultCashEntFare );
    printf			( "��� ���ݽ������                   : %lu\n",
		gstNewFareInfo.dwChildCashEntFare );
    printf			( "û�ҳ� ���ݽ������                   : %lu\n",
		gstNewFareInfo.dwYoungCashEntFare );
    printf			( "�������Ÿ� - �̻��                 : %lu\n",
		gstNewFareInfo.dwPermitErrDist );

	printf( "[LoadInfo] �ϵ��ڵ��� �ſ������ �ε�\n" );

	return ErrRet( ERR_FILE_OPEN | GetFileNo( NEW_FARE_INFO_FILE ) );
}
