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
*  PROGRAM ID :       bys_type.h                                               *
*                                                                              *
*  DESCRIPTION:       Definition of Bus Common Function & Variables.           *
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
*    DATE                SE NAME              DESCRIPTION                      *
* ---------- ---------------------------- -------------------------------------*
* 2005/08/09 Solution Team Gwan Yul Kim  Initial Release                       *
*                                                                              *
*******************************************************************************/

#ifndef _BUS_TYPE_H
#define _BUS_TYPE_H

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/io.h>
#include <malloc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/param.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/wait.h>
#include <termio.h>
#include <termios.h>
#include <unistd.h>

#ifndef __GCC
#include <asm/io.h>
#include <err.h>
#include <linux/if_arp.h>       /* For ARPHRD_ETHER */
#include <linux/ioctl.h>
#include <linux/rtc.h>
#include <linux/socket.h>       /* For AF_INET & struct sockaddr */
#include <linux/soundcard.h>
#include <linux/wireless.h>
#endif

/*******************************************************************************
*  Inclusion of Common Header Files                                            *
*******************************************************************************/
#include "../../common/type.h"
#include "../../common/define.h"
#include "../../common/errno.h"
#include "../../common/util.h"
#include "../../common/transp_util.h"
#include "../../common/rtc.h"
#include "../../common/comm.h"
#include "../../common/rf_comm.h"
#include "../../common/sam.h"
#include "../../common/card_if.h"
#include "../../common/bl.h" 
#include "../../common/pl.h"
#include "../../common/card_if_mif.h"
#include "../../common/card_if_sc.h"
#include "../../common/mif_key_mgt.h"
#include "../../common/sam_if_csam.h"
#include "../../common/sam_if_isam.h"
#include "../../common/sam_if_psam.h"
#include "../../common/sam_if_lsam.h"
#include "../../common/sam_if_olsam.h"
#include "../../common/sam_if_passsam.h"
#include "../../common/pay_lib_mif.h"
#include "../../common/pay_lib_sc.h"
#include "bus_define.h"
#include "bus_errno.h"
#include "log.h"							// ���� �����Ѵ�.

#define MAX_ERROR_LOG				10		// ����LOG �ִ� ����
#define RECONCILE_MAX				1024

typedef struct {
	dword dwChipSerialNo;					// Ĩ �ø��� ��ȣ
	byte bPLUserType;						// PL�� ��ϵ� ���������
	byte bCardUserType;						// ī�忡 ��ϵ� ����� ����
	byte bCardType;							// ī������
	byte abCardNo[20];						// ī���ȣ
	dword dwAliasNo;						// alias ��ȣ
	bool boolIsTCard;						// T������ī�� ����

	byte bSCAlgoriType;						// ��ī�� �˰�������
	byte bSCTransType;						// ��ī�� �ŷ�����
	byte bSCTransKeyVer;					// ��ī�� �����ŷ�����Ű����
	byte bSCEpurseIssuerID;					// ��ī�� ����ȭ���ID
	byte abSCEpurseID[16];					// ��ī�� ��������ID
	dword dwSCTransCnt;						// ��ī�� �ŷ��Ǽ�

	byte abPSAMID[16];						// PSAM ID
	dword dwPSAMTransCnt;					// PSAM �ŷ�ī����
	dword dwPSAMTotalTransCnt;				// PSAM �Ѿװŷ�����ī����
	word wPSAMIndvdTransCnt;				// PSAM �����ŷ������Ǽ�
	dword dwPSAMAccTransAmt;				// PSAM �����ŷ��Ѿ�
	byte abPSAMSign[4];						// PSAM ����

	dword dwBalAfterCharge;					// ����������ī���ܾ�
	dword dwChargeTransCnt;					// ����������ī��ŷ��Ǽ�
	dword dwChargeAmt;						// ���������ݾ�
	byte abLSAMID[16];						// ����������SAMID
	dword dwLSAMTransCnt;					// ����������SAM�ŷ��Ϸù�ȣ
	byte bChargeTransType;					// ���������ŷ�����
	byte abMifPrepayChargeAppvNop[14];		// ������ī�� �����������ι�ȣ
	byte abMifPrepayTCC[8];					// ������ī�� TCC

	byte bEntExtType;						// ���������� (ENT / EXT)
	byte bMifTermGroupCode;					// ��ī�� �ܸ���׷��ڵ� (��ȯ�¿������)
	time_t tMifEntExtDtime;					// ��ī�� �̿�ð� (��ȯ�¿������)
	bool boolIsMultiEnt;					// ���ν¿���
	bool boolIsAddEnt;						// �߰���������
	byte abUserType[3];						// ���ν»��������
	byte abUserCnt[3];						// ���ν»���ڼ�
	byte abDisExtraTypeID[3][6];			// ���ν�������������ID
	dword dwDist;							// ���Ÿ�
	byte bNonXferCause;						// ��ȯ�� ����
	bool boolIsChangeMonth;					// �ĺ�ī���̸鼭 �����濩��
	time_t tMifTourFirstUseDtime;			// ������ī�� ���� ��� �Ͻ�
	byte abMifTourExpiryDate[8];			// ������ī�� ������
	word wMifTourCardType;					// ������ī�� ����
	byte bMifTourUseCnt;					// ������ī�� �̹� ��/������ ���� Ƚ��
	bool boolIsAdjustExtDtime;				// �����ð���������

	COMMON_XFER_DATA stPrevXferInfo;		// ����ȯ������
	COMMON_XFER_DATA stNewXferInfo;			// �ű�ȯ������
	MIF_PREPAY_BLOCK18 stMifPrepayBlock18;	// ������ī�� BLOCK18
	MIF_POSTPAY_OLD_XFER_DATA stOldXferInfo;// ���ĺ�ī�� ��ȯ������
	short sErrCode;							// �����ڵ�
	byte bWriteErrCnt;						// WRITE���� �߻� Ƚ��
}__attribute__((packed)) TRANS_INFO;

typedef struct
{
	bool boolIsDeleted;
	TRANS_INFO stTransInfo;
}__attribute__((packed)) TRANS_ERR_LOG;

/*******************************************************************************
*  Declaration of Process Shared Memory                                        *
*******************************************************************************/
typedef struct
{
	pid_t  	nCommProcProcessID;		// Polling & DCS Comm Process ID
	pid_t	nCommPrinterProcessID;	// Printing Process ID
	pid_t	nCommDriverProcessID;	// Keypad Comm Process ID
	pid_t	nCommGPSProcessID;		// GPS Thread ID
	byte    abMainTermID[9];        // TerminalID( MainTerm ID)
	byte    abSubTermID[3][9];      // SubTerm ID
	byte    abDriverOperatorID[9];	// ���������۱�ID
	byte    abMainVer[4];           // MainTerm Execution File Version
	byte    abSubVer[3][4];         // SubTerm Execution File Version
	byte    abMainVoiceVer[4];      // MainTerm Voice File Version
	byte    abSubVoiceVer[3][4];    // SubTerm Voice File Version
	byte    abMainPSAMID[16];		// MainTerm PSAMID
	byte    abSubPSAMID[3][16];		// SubTerm PSAMID
	byte    bMainPSAMPort;     		// MainTerm PSAM PORT #
	byte    abSubPSAMPort[3];   	// SubTerm PSAMID Comm PORT
	byte    abMainISAMID[7];        // MainTerm ISAMID
	byte    abSubISAMID[3][7];      // SubTerm ISAMID
	byte    abMainCSAMID[8];        // MainTerm CSAMID
	byte    abSubCSAMID[3][8];      // SubTerm CSAMID
	byte	bMainPSAMVer;			// �����ܸ��� PSAM ����
	byte	abSubPSAMVer[3];		// �����ܸ��� PSAM ����
	byte	abKpdVer[4];			// Keypad Execution File Version

	bool	boolIsReadyToDrive;		// Ready to StartAlarm
	bool    boolIsKpdLock;          // Execution /Voice File Transfer Status
	byte    bVoiceApplyStatus;      // Voice File Apply Status

	bool    boolIsBLPLMergeNow;			// BL/PL ������ ����
	bool    boolIsBLMergeNow;			// BL ������ ����
	bool    boolIsMifPrepayPLMergeNow;	// ������PL ������ ����
	bool    boolIsPostpayPLMergeNow;	// �ĺ�PL ������ ����
	bool    boolIsSCPrepayPLMergeNow;	// �ż���PL ������ ����

	byte    bCommCmd;               // Command Between Process
	char    abCommData[40];         // Command Data Between Process
	byte	bCmdDataSize;			// Command Data Size
	byte    bPollingResCnt;         // Polling Response Status
	bool    boolIsDriveNow;         // Driving Status
	byte    abTransFileName[19];    // TR Data FileName
	byte    abEndedTransFileName[15];
	time_t  tTermTime;              // Term Time
	byte    abNowStationID[7];      // Station ID
	byte	abNowStationName[16];	// Station Name
	bool    boolIsCardProc;         // Card Process Status
	bool	boolIsTransFileUpdateEnable;	// TR File Write Enable
	
	bool	boolIsKpdImgRecv;		// �ű� ���������۱� ���� ����
	byte	bCardUserType;			// ī�屸��('1':�Ϲ� '2':û�ҳ� '3':���)
	byte	abTotalFare[6];			// �ѿ��
	bool	boolIsXfer;				// ȯ�¿���
	bool	boolIsGpsValid;			// GPS DATA�� ��ȿ������
	bool	boolIsSetStationfromKpd;// ������ 0:�������, 1:���������۱⺯����
	byte	abKeyStationID[7];		//
	byte	abKeyStationName[16];	//
	byte	abLanCardStrength[3];
	byte	abLanCardQuality[3];
	dword	dwDriveChangeCnt;

	byte gbGPSStatusFlag;			// �ŷ������� ��ϵǴ� GPS ���� �÷���

	byte bTransErrLogPtr;			// �ŷ��̿Ϸ�α� ������
	TRANS_ERR_LOG astTransErrLog[MAX_ERROR_LOG];
									// �ŷ��̿Ϸ�α�
} __attribute__((packed)) PROC_SHARED_INFO;

/*******************************************************************************
*  Declaration of Terminal Info                                                *
*******************************************************************************/
typedef struct
{
    byte    abVoiceVer[4];      // MainTerm Voice File Version
    byte    abPSAMID[16];		// MainTerm PSAMID
	byte    bPSAMPort;     		// MainTerm PSAM PORT #
	byte    abISAMID[7];        // MainTerm ISAMID
	byte    abCSAMID[8];        // MainTerm CSAMID
} __attribute__((packed)) MYTERM_INFO;


/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/
typedef struct
{
	long lMsgType;
	char achMsgData[MSGQUEUE_MAX_DATA + 1];
} __attribute__((packed)) MSGQUEUE_DATA;

/*******************************************************************************
*  Declaration of Rebooting qualification                                                *
*******************************************************************************/
typedef struct
{
	bool boolIsCriteriaRecv;	// �űԿ���� ����
	bool boolIsImgRecv;			// �ű� ���� IMAGE����
	bool boolIsVoiceFileRecv;	// �ű� �������ϼ���
	bool boolIsKpdImgRecv;		// �ű� ���������۱� ���� ����
	bool boolIsReset;			// �ܸ��� RESET �ʿ�
}__attribute__((packed)) NEW_IMGNVOICE_RECV;

/*******************************************************************************
*  Declaration of MULTI_ENT_INFO                        					   *
*******************************************************************************/
typedef struct
{
	bool boolIsMultiEnt;				// ���ν��Է¿���
	time_t tInputDatetime;				// ���ν��Է��Ͻ�
	bool boolIsAddEnt;					// �߰���������
	byte abUserType[3];					// ����� ����
	byte abUserCnt[3];					// ����� ���
} __attribute__((packed)) MULTI_ENT_INFO;

/*******************************************************************************
* ��Ƽ������� �������Է�����                                                  *
*******************************************************************************/
typedef struct
{
	bool boolIsTicketInput;				// �����������Է¿���
	time_t tInputDtime;					// �����������Է��Ͻ�
	bool boolIsOneTimeTicket;			// 1ȸ�ǿ��� (TRUE:1ȸ��, FALSE:���ϱ�)
	byte abUserType[2];					// ����� ���� (��� ������)
	byte abUserCnt[2];					// ����� ��� (��� ������)
	dword dwTicketAmt;					// ���� ������ �ݾ�
} __attribute__((packed)) CITY_TOUR_BUS_TICKET_INFO;


/*******************************************************************************
*  structure of reconcileinfo.dat                                              *
*******************************************************************************/
typedef struct
{
	char 	achFileName[30];	// ����PC�� ������ ���ϸ�
	byte 	bSendStatus;		// ������ ���ϻ���
	int		nSendSeqNo;			// ����Ƚ��
	time_t 	tWriteDtime;		// ���ϸ��Write�ð�
} __attribute__((packed)) RECONCILE_DATA;


/*******************************************************************************
*  Declaration of MSGQUEUE_RECEIPT_DATA                                        *
*******************************************************************************/
typedef struct
{
	byte abBizNo[10];
	byte abDateTime[14];
	byte abVehicleNo[20];
	byte abTranspMethodCodeName[8];
	byte abUserTypeName[12];
	byte achBusStationName[16] ;
	byte abFare[6] ;
} __attribute__((packed)) MSGQUEUE_RECEIPT_DATA;

/*******************************************************************************
* �����ҹ�������� - ap_inf.dat (B0670)                                        *
*******************************************************************************/
typedef struct
{
	time_t tApplyDtime;					// �����Ͻ�
	byte bApplySeqNo;					// �����Ϸù�ȣ
	dword dwRecordCnt;					// ���ڵ� �Ǽ�
} __attribute__((packed)) PREPAY_ISSUER_INFO_HEADER;

typedef struct
{
	byte abPrepayIssuerID[7];			// ���� ����� ID
	byte bAssocCode;					// �ܸ��� �Ҽ� ���� �����ڵ�
	byte bXferDisYoungCardApply;		// ȯ������/�л�ī�� ��������
} __attribute__((packed)) PREPAY_ISSUER_INFO;

/*******************************************************************************
* �ĺҹ�������� - dp_inf.dat (B0690)                                          *
*******************************************************************************/
typedef struct
{
	time_t tApplyDtime;					// �����Ͻ�
	word wRecordCnt;					// ���ڵ� �Ǽ�
} __attribute__((packed)) POSTPAY_ISSUER_INFO_HEADER;

typedef struct
{
	byte abPrefixNo[6];					// Prefix ��ȣ
	word wCompCode;						// �����ڵ�
} __attribute__((packed)) POSTPAY_ISSUER_INFO;

/*******************************************************************************
* �ĺҹ������ȿ�Ⱓ���� - cp_inf.dat                                          *
*******************************************************************************/
typedef struct
{
	time_t tApplyDtime;					// �����Ͻ�
	word wRecordCnt;					// ���ڵ� �Ǽ�
} __attribute__((packed)) ISSUER_VALID_PERIOD_INFO_HEADER;

typedef struct
{
	byte abPrefixNo[6];					// Prefix ��ȣ
	byte abIssuerID[7];					// �����ID
	byte abExpiryDate[6];				// ī����ȿ�Ⱓ (YYYYMM)
} __attribute__((packed)) ISSUER_VALID_PERIOD_INFO;

/*******************************************************************************
* ȯ���������� - tr_inf.dat (B1700)                                            *
*******************************************************************************/
typedef struct
{
	dword dwRecordCnt;					// ���ڵ� �Ǽ�
} __attribute__((packed)) XFER_APPLY_INFO_HEADER;

typedef struct
{
	time_t tApplyDtime;					// �����Ͻ�
	word wXferApplyStartTime;			// ȯ��������۽ð�
	word wXferApplyEndTime;				// ȯ����������ð�
	byte bHolidayClassCode;				// ���ϱ����ڵ�
	dword dwXferEnableTime;				// ȯ�°��ɽð�
	word wXferEnableCnt;				// ȯ�°���Ƚ��
} __attribute__((packed)) XFER_APPLY_INFO;

/*******************************************************************************
* ������������ - de_inf.dat (B0640)                                            *
*******************************************************************************/
typedef struct
{
	time_t tApplyDtime;					// �����Ͻ�
	byte bApplySeqNo;					// �����Ϸù�ȣ
	dword dwRecordCnt;					// ���ڵ� �Ǽ�
} __attribute__((packed)) DIS_EXTRA_INFO_HEADER;

typedef struct
{
	byte bTranspCardClassCode;			// ����ī�屸���ڵ�
	byte abDisExtraTypeID[6];			// ������������ID
	byte bDisExtraApplyCode;			// ����������������ڵ�
	float fDisExtraRate;				// ����/������
	int nDisExtraAmt;					// ����/���� �ݾ�
} __attribute__((packed)) DIS_EXTRA_INFO;

/*******************************************************************************
* �������� - ho_inf.dat (B0630)                                                *
*******************************************************************************/
typedef struct
{
	time_t tApplyDtime;					// �����Ͻ�
	dword dwRecordCnt;					// ���ڵ� �Ǽ�
} __attribute__((packed)) HOLIDAY_INFO_HEADER;

typedef struct
{
	byte abHolidayDate[8];				// ��������ID
	byte bHolidayClassCode;				// ���ϱ����ڵ�
} __attribute__((packed)) HOLIDAY_INFO;

/*******************************************************************************
* �ſ������ - n_far.dat (B0070)                                               *
*******************************************************************************/
typedef struct
{
	time_t tApplyDatetime;				// �����Ͻ�
	byte bApplySeqNo;					// �����Ϸù�ȣ
	word wTranspMethodCode;				// ��������ڵ�
	dword dwSingleFare;					// ���Ͽ�� - �̻��
	dword dwBaseFare;					// �⺻����
	dword dwBaseDist;					// �⺻�Ÿ�
	dword dwAddedFare;					// �ΰ�����
	dword dwAddedDist;					// �ΰ��Ÿ�
	dword dwOuterCityAddedDist;			// �ÿܺΰ��Ÿ� - �̻��
	dword dwOuterCityAddedDistFare;		// �ÿܺΰ��Ÿ����� - �̻��
	dword dwAdultCashEntFare;			// ���� ���ݽ������
	dword dwChildCashEntFare;			// ��� ���ݽ������
	dword dwYoungCashEntFare;			// û�ҳ� ���ݽ������
	dword dwPermitErrDist;				// �������Ÿ� - �̻��
} __attribute__((packed)) NEW_FARE_INFO;

/*******************************************************************************
*  structure of STATION_INFO - c_st_inf.dat                              					   *
*******************************************************************************/
typedef struct
{
	time_t tApplyDtime;					// �����Ͻ�
	byte bApplySeqNo;					// �����Ϸù�ȣ
	byte abRouteID[8];					// �����뼱 ID
	byte abTranspMethodName[16];		// �����뼱��
	dword dwRecordCnt;					// ���ڵ� �Ǽ�
} __attribute__((packed)) STATION_INFO_HEADER;

typedef struct
{
	byte abStationID[7];				// ���������� ID
	byte bCityInOutClassCode;			// �ð賻�ܱ��� �ڵ�
	word wStationOrder;					// ���������
	byte abStationName[16];				// �����������
	double dStationLongitude;			// ���������� �浵
	double dStationLatitude;			// ���������� ����
	word wOffset;						// offset
	dword dwDistFromFirstStation;		// ù�����忡���� �Ÿ�
	word wStationApproachAngle;			// ������ ���԰�
} __attribute__((packed)) STATION_INFO;


/*******************************************************************************
*  structure of VEHICLE_PARM - op_par.dat                 					   *
*******************************************************************************/
typedef struct
{
	byte	abApplyDtime[14];      			// ���� �Ͻ�
	byte    abTranspBizrID[9];            	// �������� ID
	byte    abBusBizOfficeID[2];          	// ���������� ID
	byte    abVehicleID[9];              	// ���� ID
	byte    abVehicleNo[20];             	// ������ȣ
	byte    abRouteID[8];                	// �뼱 ID
	byte    abDriverID[6];               	// ������ ID
	byte    abTranspMethodCode[3];       	// ��������ڵ� - byte �迭
	word	wTranspMethodCode;				// ��������ڵ� - word
	byte    abBizNo[10];                  	// ����ڹ�ȣ
	byte    abTranspBizrNm[20];           	// �������ڸ�
	byte    abAddr[103];                  	// �ּ�
	byte    abRepreNm[10];                	// ��ǥ�ڸ�
} __attribute__((packed)) VEHICLE_PARM;


typedef struct
{
	byte	abApplyDtime[14];				// �����Ͻ�
	byte	abRouteID[8];					// �뼱ID
	byte	abTranspMethodCode[3];			// ��������ڵ� - byte �迭
	word	wTranspMethodCode;				// ��������ڵ� - word
	byte	abXferDisApplyFreq[2];			// ȯ����������Ƚ�� (�̻��)
	byte	abXferApplyTime[3];				// ȯ������ð� (�̻��)
	byte	abTermGroupCode[2];				// �ܸ���׷��ڵ� (�̻��)
	byte	abSubTermCommIntv[3];			// �����ܸ������Ű��� (�̻��)
	byte	abCardProcTimeIntv[3];			// ī��ó���ð����� (�̻��)
	byte	abUpdateBLValidPeriod[2];		// ����BL��ȿ�Ⱓ (�̻��)
	byte	abUpdatePLValidPeriod[2];		// ����PL��ȿ�Ⱓ (�̻��)
	byte	abUpdateAIValidPeriod[2];		// ����AI��ȿ�Ⱓ (�̻��)
	byte	bDriverCardUseYN;				// ������ī�������� (�̻��)
	byte	abChargeInfoSendFreq[2];		// ������������Ƚ�� (�̻��)
	byte	bRunKindCode;					// ���������ڵ� - ���������۱�����
	byte	bGyeonggiIncheonRangeFareInputWay;
											// �����õ��������Է¹�� - ���������۱�����
	byte	abLEAPPasswd[20];				// LEAP ��ȣ (�̻��)
	byte	bECardUseYN;					// �ĺ�ecard ������� (�̻��)

} __attribute__((packed)) ROUTE_PARM;

/*******************************************************************************
*  Declaration of Extern Global Variables                                      *
*******************************************************************************/
extern bool gboolIsMainTerm;			// Div of MainTerm & SubTerm
extern byte gabStationName[17];			// StationName
extern byte gbSubTermNo;				// # of SubTerm
//bool gboolIsDriveNow;            		// �����޸𸮿� ������.
extern bool gboolIsWhileNow;            // WhileLoop Status
extern bool boolIsAllLogMode;			// Whether LogMode is AllLOG Mode or Not
extern byte gbSubTermCnt;				// �����ܸ��ⰹ��

// GPS Thread�� ���ÿ� ����ϴº���
extern bool boolIsEndStation;			// Bus�� ������ �ִ��� ����
extern char gbchStationID[8];

// DCS Thread�� ���ÿ� ����ϴº���
extern bool boolIsDCSThreadComplete;    /* DCS thread complete flag */
extern bool boolIsDCSThreadStartEnable; /* DCS thread start enable/disable */
extern int nDCSCommSuccCnt;				// DCS comm success count
extern time_t gtDCSCommStart;		// DCS comm ���� �ð� 

extern PROC_SHARED_INFO *gpstSharedInfo;	// Shared Memory Pointer
extern MYTERM_INFO		gstMyTermInfo;

extern byte gabKernelVer[3];

// Decla. of File Desc.
extern int gfdRC531Device;				// RC531 Device File Descriptor

extern MULTI_ENT_INFO gstMultiEntInfo;	// ���ν�����
extern CITY_TOUR_BUS_TICKET_INFO gstCityTourBusTicketInfo;
										// ��Ƽ������� �������Է�����

// Message Queue for Printing
extern int gnMsgQueue;

extern  byte 	gabLEAPPasswd[21];
extern	byte	gabDCSIPAddr[16];			// DCS server IP

// ���ó������ ���Ǵ� �⺻����
extern VEHICLE_PARM 					gstVehicleParm;
extern ROUTE_PARM						gstRouteParm;
extern PREPAY_ISSUER_INFO_HEADER 		gstPrepayIssuerInfoHeader;
extern PREPAY_ISSUER_INFO 				*gpstPrepayIssuerInfo;
extern POSTPAY_ISSUER_INFO_HEADER 		gstPostpayIssuerInfoHeader;
extern POSTPAY_ISSUER_INFO 				*gpstPostpayIssuerInfo;
extern ISSUER_VALID_PERIOD_INFO_HEADER
										gstIssuerValidPeriodInfoHeader;
extern ISSUER_VALID_PERIOD_INFO 		*gpstIssuerValidPeriodInfo;
extern XFER_APPLY_INFO_HEADER 			gstXferApplyInfoHeader;
extern XFER_APPLY_INFO 					*gpstXferApplyInfo;
extern DIS_EXTRA_INFO_HEADER 			gstDisExtraInfoHeader;
extern DIS_EXTRA_INFO 					*gpstDisExtraInfo;
extern HOLIDAY_INFO_HEADER 				gstHolidayInfoHeader;
extern HOLIDAY_INFO 					*gpstHolidayInfo;
extern NEW_FARE_INFO 					gstNewFareInfo;
extern STATION_INFO_HEADER				gstStationInfoHeader;
extern STATION_INFO 					*gpstStationInfo;

extern word gwGPSRecvRate;				// GPS ������
extern word gwDriveCnt;					// ����Ƚ��
extern time_t gtDriveStartDtime;		// ������۽ð�
extern dword gdwDistInDrive;			// ������ �̵��� �Ÿ�
extern bool gboolIsRunningGPSThread;	// GPS������ ���� ���� (GPS�α� �ۼ� ����)
extern int gnGPSStatus;					// GPS Status
extern bool gboolIsEndStation;
extern byte gabEndStationID[7];			// ����������ID
extern dword gdwDelayTimeatEndStation;	// ������ �����ѵ� delay time
extern byte gabGPSStationID[7];			// GPS������ ������Ʈ�ϴ� ������ID
extern bool boolIsRegistMainTermPSAM;   // thread exit enabl for DCS Comm
extern word gwRetagCardCnt;				// 'ī�带 �ٽ� ���ּ���' ���� Ƚ��

#endif

