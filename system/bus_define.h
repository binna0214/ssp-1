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
*  PROGRAM ID :       bys_define.h                                             *
*                                                                              *
*  DESCRIPTION:       Definition of Bus defines and macros 			           *
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

#ifndef _BUS_DEFINE_H
#define _BUS_DEFINE_H

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>

#ifndef __GCC
#include <wait.h>
#endif

/*******************************************************************************
*  Definition of Image Version                                                 *
*******************************************************************************/
#define MAIN_RELEASE_VER				"0409"

/*******************************************************************************
*  Macro Definition of File Names                                              *
*******************************************************************************/
#define CONTROL_TRANS_FILE				"control.trn"		// ���������������
#define CONTROL_TRANS_BACKUP_FILE		"control.bak"		// ����������Ϲ������

#define MAIN_TERM_BACKUP_TRANS_FILE		"aboard_term_td.bak"// ��������ŷ���������
#define SUB_TERM_BACKUP_TRANS_FILE		"alight_term_td.bak"// ��������ŷ���������
#define SUB_TERM_TRANS_FILE				"alight_term_td.tmp"// �����ŷ���������
#define TEMP_REMAKE_FILE				"temptd.dat"		// �ӽô������

#define TERM_AGGR_SEQ_NO_FILE			"seqth.tmp"			// �ܸ����������Ϸù�ȣ����
#define BACKUP_TRANS_SEQ_NO_FILE		"backseq.tmp"		// ����ŷ������Ϸù�ȣ����
#define CASH_GETON_SEQ_NO_FILE			"ticketseq.tmp"		// ��ȸ��ID�Ϸù�ȣ����

#define PG_LOADER_VER_FILE				"pgver.dat"			// PG�δ� ���� ����
#define VOICE0_FILE						"c_v0.dat"			// ��������
#define VOICEAPPLY_FLAGFILE				"voiceapply.dat"	// ������������
#define VOICEAPPLY_VERSION				"voicever.dat"		// ���������������
#define VEHICLE_PARM_FILE				"c_op_par.dat"
#define ROUTE_PARM_FILE					"c_li_par.dat"
#define PREPAY_ISSUER_INFO_FILE			"c_ap_inf.dat"
#define POSTPAY_ISSUER_INFO_FILE		"c_dp_inf.dat"

#define BUS_STATION_INFO_FILE			"c_st_inf.dat"
#define XFER_APPLY_INFO_FILE			"c_tr_inf.dat"
#define DIS_EXTRA_INFO_FILE				"c_de_inf.dat"
#define HOLIDAY_INFO_FILE				"c_ho_inf.dat"
#define XFER_TERM_INFO_FILE				"c_tc_inf.dat"
#define NEW_FARE_INFO_FILE				"c_n_far.dat"

#define MASTER_BL_FILE					"c_fi_bl.dat"
#define MASTER_PREPAY_PL_FILE			"c_fa_pl.dat"
#define MASTER_POSTPAY_PL_FILE			"c_fd_pl.dat"
#define MASTER_AI_FILE					"c_fi_ai.dat"
#define CHANGE_BL_FILE					"c_ch_bl.dat"
#define UPDATE_PL_FILE					"c_ch_pl.dat"
#define UPDATE_AI_FILE					"c_ch_ai.dat"

#define DOWN_ING_MASTER_BL_FILE			"tmp_c_fi_bl.dat"
#define DOWN_ING_MASTER_PREPAY_PL_FILE	"tmp_c_fa_pl.dat"
#define DOWN_ING_MASTER_POSTPAY_PL_FILE	"tmp_c_fd_pl.dat"
#define DOWN_ING_MASTER_AI_FILE			"tmp_c_fi_ai.dat"

#define DOWN_ING_UPDATE_BL_FILE			"tmp_c_ch_bl.dat"
#define DOWN_ING_UPDATE_PL_FILE			"tmp_c_ch_pl.dat"
#define DOWN_ING_UPDATE_AI_FILE			"tmp_c_ch_ai.dat"

#define BUS_MAIN_IMAGE_FILE				"c_en_pro.dat"
#define BUS_SUB_IMAGE_FILE				"c_ex_pro.dat"
#define KPD_IMAGE_FILE					"c_dr_pro.dat"
#define KPDAPPLY_FLAGFILE				"driverdn.cfg"

#define SUB_TRANS_SEND_SUCC				"check.trn"
#define SUBTERM_ID_FILENAME				"subid.dat"
#define PSAMID_FILE						"simid.flg"
#define STATUS_FLAG_FILE				"statusFlag.dat"

#define COMMDCS_SUCCDATE_FILE			"connSucc.dat"

#define RECONCILE_FILE					"reconcileinfo.dat"
#define DOWN_FILE						"downloadinfo.dat"
#define RELAY_DOWN_INFO_FILE			"downfilelink.dat"
#define INSTALL_INFO_FILE				"install.dat"
#define BLPL_CRC_FILE        			"blpl_crc.dat"
#define SETUP_FILE						"setup.dat"
#define SETUP_BACKUP_FILE				"setup.backup"
#define TC_LEAP_FILE					"tc_leap.dat"
#define TC_LEAP_BACKUP_FILE				"tc_leap.backup"

#define GPS_INFO_FILE					"gps_info.dat"
#define GPS_LOG_FILE					"simxinfo.dat"
#define GPS_LOG_FILE2					"simxlog.dat"

#define ERR_LOG_MODE_FLAG_FILE			"c_ho_bl.dat"

#define KERNEL_VERSION_FILE				"/proc/version"
#define RAMDISK_VERSION_FILE			"/root/config.ini"

#define FLASH_DATA_OVER_ONE				"diskone.dat"
#define FLASH_DATA_OVER_TWO				"disktwo.dat"

#define BUS_BACKUP_DIR					"/mnt/mtd7"
#define BUS_EXECUTE_DIR					"/mnt/mtd8/bus"
#define BUS_EXECUTE_FILE				"bus100"
#define BUS_EXECUTE_BACKUP_FILE			"bus200"
#define BUS_EXECUTE_BEFORE_BACKUP_FILE	"bus300"

#define SEMA_KEY_TRANS					1000
#define SEMA_KEY_ERR_LOG				1001
#define SEMA_KEY_LOG					1002
#define SEMA_KEY_SHARED_CMD				1003

#define SUBTERM_TRANS_RECORD_SIZE		202
#define MSGQUEUE_MAX_DATA				1024		// Max Data Size of MsgQueue

#define SHARED_MEMORY_KEY				((key_t)5678)
#define MESSAGE_QUEUE_PRINTER_KEY		(900000 + 1)

/*******************************************************************************
*  Macro of Cmd between Processes											   *
*******************************************************************************/
#define CMD_SETUP						'A'	// MainProc �� CommProc
#define CMD_RESETUP						'R'	// MainProc �� CommProc
#define CMD_SUBID_SET					'I'	// KeypadProc �� CommProc
#define CMD_NEW_CONF_IMG				'L'	// CommProc �� MainProc/ KeypadProc
#define CMD_MULTI_GETON					'M'	// KeypadProc �� MainProc
#define CMD_CANCEL_MULTI_GETON			'C'	// KeypadProc �� MainProc
#define CMD_START_STOP					'D'	// KeypadProc �� MainProc
#define CMD_STATION_CORRECT				'S'	// KeypadProc �� MainProc
#define CMD_PRINT						'Q'	// KeypadProc �� MainProc �� PrinterProc
#define CMD_BL_CHECK					'B'	// MainProc �� CommProc
#define CMD_PL_CHECK					'U'	// MainProc �� CommProc
#define CMD_PARMS_RESET					'E'	// KeypadProc �� CommProc
#define CMD_CONFIRM_CANCEL_STATION_CORRECT \
										'O'	// KeypadProc �� MainProc
#define CMD_KEYSET_IDCENTER_WRITE		'K'	// CommProc �� MainProc
#define CMD_INPUT_CITY_TOUR_BUS_TICKET	'T' // KeypadProc �� MainProc
#define CMD_CANCEL_CITY_TOUR_BUS_TICKET	'V'	// KeypadProc �� MainProc

/*******************************************************************************
*  Declaration of Date/ Version Size                                           *
*******************************************************************************/
#define FILE_DATE_SIZE					14
#define FILE_VERSION_SIZE				4

/*******************************************************************************
*  Declaration of Shared Memory Command Meaning                                *
*******************************************************************************/
#define CMD_REQUEST						'1'	//
#define CMD_SUCCESS_RES					'0'	//
#define CMD_FAIL_RES					'9'	//

/*******************************************************************************
*  MessageQueue Permission                                                     *
*******************************************************************************/
#define IPC_PERM ( 0660 )

/*******************************************************************************
*  Reconcile Send status                                                       *
*******************************************************************************/
#define RECONCILE_SEND_WAIT			'0'   	// TR���� �۽Ŵ��
#define RECONCILE_SEND_JUST			'1'   	//
#define RECONCILE_SEND_COMP			'2'   	// �۽ſϷ� reconcile ���
#define RECONCILE_RESEND_REQ		'3'   	// ��۽ſ�û
#define RECONCILE_RESEND_JUST		'4'   	//
#define RECONCILE_RESEND_COMP		'5'   	// ��۽ſϷ� reconcile ���
#define RECONCILE_SEND_SETUP		'7'   	// �¾����� �۽Ŵ��
#define RECONCILE_SEND_ERR_LOG		'8'   	// �����α� �۽Ŵ�� (�̺�Ʈ)
#define RECONCILE_SEND_VERSION		'9'   	// �������� �۽Ŵ��
#define RECONCILE_SEND_GPS		    'a'   	// GPS���� �۽Ŵ��
#define RECONCILE_SEND_GPSLOG	    'b'   	// GPSLOG �۽Ŵ��
#define RECONCILE_SEND_GPSLOG2	    'c'   	// GPSLOG2 �۽Ŵ��
#define RECONCILE_SEND_TERM_LOG		'd'		// �ܸ��� �α�
#define RECONCILE_SEND_STATUS_LOG	'e'		// ���� �α�


/*******************************************************************************
*  Reconcile result status                                                 *
*******************************************************************************/
#define RECONCILE_RESULT_DEL		'0'		// ����
#define RECONCILE_RESULT_RESEND		'1'		// ��۽�
#define RECONCILE_RESULT_NONE		'2'		// ���
#define RECONCILE_RESULT_ERR_DEL	'3'		// ������ ���� ����

/*******************************************************************************
*  download status                                                 *
*******************************************************************************/
#define UNDER_DOWN					1		// �ٿ�ε� ��
#define DOWN_COMPL					2		// �ٿ�ε� �Ϸ�

/*******************************************************************************
*  Printer Message Queue Type                                                  *
*******************************************************************************/
#define PRINT_RECEIPT				1
#define PRINT_TERM_INFO				2

/*******************************************************************************
*  GPS SATELLITE DATA VALID                                                    *
*******************************************************************************/
#define GPS_DATA_VALID				'A'

#define PSAMID_SIZE					16

#define CURR						0
#define NEXT						1
#define MAX_FILE_READ_SIZE_OLD		1024
#define MAX_PKT_SIZE 				1024
#define MAX_PKT_SIZE_OLD			1034
#define MULTI_GET_ON_TIME			10		// ���ν��Է�ó���ð�

#define SYSTEM_RESET_REQ			(short)0x0001
#define OPENMODE 	S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

/*******************************************************************************
*  Declaration of Comm Macros                                                  *
*******************************************************************************/
#define SOH								0x01
#define STX								0x02
#define	ETX								0x03
#define ACK								0x06
#define NAK								0x15

/*******************************************************************************
*  Declaration of Keypad Cmd Macros                                            *
*******************************************************************************/
#define KPDCMD_SET_TERMID_LEAPPWD		0x11
#define KPDCMD_SETUP					0x12
#define KPDCMD_SET_TIME					0x13
#define KPDCMD_START_STOP				0x14
#define KPDCMD_EXTRA					0x15
#define KPDCMD_CANCEL_MULTI_GETON		0x17
#define KPDCMD_BL_CHECK					0x18
#define KPDCMD_PL_CHECK					0x19
#define KPDCMD_MULTI_GETON				0x20
#define KPDCMD_STATION_CORRECT			0x21
#define KPDCMD_CASHENT_RECEIPT_PRINT	0x22
#define KPDCMD_SUBID_SET				0x23
#define KPDCMD_SERVERIP_SET				0x24
#define KPDCMD_TERMINFO_PRINT			0x25
#define KPDCMD_CONFIRM_STATION_CORRECT	0x26
#define KPDCMD_CANCEL_STATION_CORRECT	0x27

#define KPDCMD_SET_TERMID_LEAPPWD_0118	0x31
#define KPDCMD_MAINID_SET				0x32
#define KPDCMD_CITY_TOUR_BUS_TICKET		0x33

#define KPDCMD_RECV(bCmd) \
	((KPDCMD_SET_TERMID_LEAPPWD <= bCmd) && \
	(bCmd <=KPDCMD_CITY_TOUR_BUS_TICKET) )	
#define IS_KPDIMAGE_SEND(bCmd, bSendReq) \
	((bCmd == CMD_NEW_CONF_IMG) && \
	 (bSendReq == CMD_REQUEST) )

/*******************************************************************************
*  �����⿡�� ������ID�� ������ ����ϱ����� ����ũ��                          *
*******************************************************************************/
#define SERVER_IP_SIZE				12
#define MAINTERM_ID_SIZE			9
#define SUBTERM_ID_SIZE				9
#define SUBTERM_ID_COUNT_SIZE		(1 + SUBTERM_ID_SIZE*3)
#define END_MSG_COND( bTermCnt, nLoopCnt ) \
	(((bTermCnt <= 1)&&(nLoopCnt >= 320)) || ((bTermCnt > 1)&&(nLoopCnt >= 55)))

#define RECEIPT_ADULTCOUNT_SIZE		3
#define RECEIPT_YOUNGCOUNT_SIZE		3
#define RECEIPT_CHILDCOUNT_SIZE		3
#define RECEIPT_ALLCOUNT_SIZE \
	(RECEIPT_ADULTCOUNT_SIZE + RECEIPT_YOUNGCOUNT_SIZE + RECEIPT_CHILDCOUNT_SIZE)
#define STATION_ID_CORRECT_SIZE		1
#define CANCEL_MULTI_GETON_SIZE		1
#define DRIVER_ID_SIZE				7
				 
/*******************************************************************************
*  DEVICE CLASS                                          					   *
*******************************************************************************/
#define DEVICE_CLASS_MAIN					"14"		// �����ܸ���
#define DEVICE_CLASS_SUB					"15"		// �����ܸ���

#define SPACE   0x20	// ' '
#define ZERO    0x30	// '0'
#define CR		0x0D
#define LF		0x0A

#define FND_VOICEFILE_WRITE_TO_FLASH		"000999"	// FLASH�� �������� �����

#define FND_ERR_MSG_WRITE_MAIN_TRANS_DATA	"999111"	// �����ܸ��� �ŷ�������� ����
#define FND_ERR_MSG_WRITE_SUB_TRANS_DATA	"119119"	// �����ܸ��� �ŷ�������� ����

#define FND_ERR_CRITERIA_INFOFILE_NO_EXIST	"999000"	// ����������� �Ѱ����� ������
#define FND_ERR_GET_PSAM_ID					"111111"	// PSAM IDȹ�� ����
#define FND_ERR_RF_INIT						"222222"	// RF�ʱ�ȭ �����߻�
#define FND_ERR_ISAM						"333333"	// ISAM �����߻�
#define FND_ERR_PSAM						"444444"	// PSAM �����߻�
#define FND_ERR_RECEIVE_LATEST_INFO			"777777"	// �ֽſ����������� ��û		
#define FND_ERR_SYSTEM_MEMORY				"888888"	// �����޸� �ʱ�ȭ����
#define FND_ERR_SEMA_CREATE					"999222"	// SEMAPHORE Create Failed
#define FND_ERR_MAIN_SUB_COMM_PORT			"666666"	// ���PORT �ʱ�ȭ �����߻�

#define FND_ERR_MAIN_SUB_COMM_POLLING		"999999"	// ���� �������� ����
#define FND_ERR_SUBTERM_TRN_EXIST			"114114"	// �����ܸ��⿡ ������ ����Ÿ����

#define DOWNLOAD_START_FROM_DCS				"111111"	// ����κ��� �ٿ�ε� ����
#define DOWNLOAD_SUBTERM_PARM				"122222"	// �����ܸ���� �������������
#define FND_DOWNLOAD_START_SUBTERM_VOICE	"133333"	// �����ܸ���� ������������
#define FND_DOWNLOAD_DRIVER_IMG				"911111"	// ���������۱� �ݿ�
#define FND_DOWNLOAD_SUBTERM_IMG			"922222"	// �����ܸ���� F/W��������
#define FND_DOWNLOAD_END_SUBTERM_VOICE		"933333"	// �����ܸ���� �����������ۿϷ�

#define FND_INIT_MSG 						"000000"
#define FND_READY_MSG						"0"

#define MAX_RETRY_COUNT         			3
#define MIN_RETRY_COUNT         			1

#define EEPROM_DEVICE						"/dev/i2c"
#define DIPSWTICH_DEVICE					"/dev/dipsw"
#define BUZZ_DEVICE							"/dev/buz"
#define FND_DEVICE							"/dev/seg"
#define PRINTER_DEVICE						"/dev/ttyE3"
#define RC531_DEVICE						"/dev/rc531"
#define VOICE_DEVICE						"/dev/voice"

#define LANCARD_SIGNAL_FILE					"/proc/driver/aironet/eth0/Status"

#define TEST_PRINT_CARD_PROC_INFO				// �� ��ũ�ΰ� define�Ǹ� ���带 ���� ����ȯ������/�ű�ȯ�������� ��µ�
//#define TEST_PRINT_CARD_PROC_TIME				// �� ��ũ�ΰ� define�Ǹ� ī���Ǻ��ð�/ī��ó���ð��� ��µ�
//#define TEST_NOT_LOG							// �� ��ũ�ΰ� define�Ǹ� log.c�� �̿��� �αװ� ���� ����
//#define TEST_CARDTYPE_DEPOSIT				"10008001000114115409"
												// �� ��ũ�ΰ� define�Ǹ� �ش� ī���ȣ�� ���� ������ī�带 ��ġ ��ġ��ī��ó�� ó����
//#define TEST_NOT_CHECK_BLPL					// �� ��ũ�ΰ� define�Ǹ� BL/PLüũ�� �������� ����
//#define TEST_NOT_CHECK_ISSUER					// �� ��ũ�ΰ� define�Ǹ� �����üũ�� �������� ����
//#define TEST_NOT_CHECK_EXPIRY_DATE			// �� ��ũ�ΰ� define�Ǹ� �ĺ�ī���� ��ȿ�Ⱓ üũ�� �������� ����
//#define TEST_NOT_CHECK_TEST_CARD				// �� ��ũ�ΰ� define�Ǹ� �׽�Ʈī�嵵 ����� �� ����
//#define TEST_NOT_SLEEP_DURING_DISPLAY			// �� ��ũ�ΰ� define�Ǹ� ī��ó����� ���÷��̽� SLEEP�� ���� ����
//#define TEST_NO_VOICE							// �� ��ũ�ΰ� define�Ǹ� ������ ��µ��� ����
//#define TEST_NO_BUZZER						// �� ��ũ�ΰ� define�Ǹ� ������ ��µ��� ����
//#define TEST_ALWAYS_ENT						// �� ��ũ�ΰ� define�Ǹ� ī��ó���� �׻� ������ ó����
//#define TEST_TRANS_0_WON						// �� ��ũ�ΰ� define�Ǹ� ����� ������ 0������ ó����
//#define TEST_TRANS_10_WON						// �� ��ũ�ΰ� define�Ǹ� ����� ������ 10������ ó����
//#define TEST_NOT_CHECK_MIN_BAL				// �� ��ũ�ΰ� define�Ǹ� ����ī�� ó���� �ּ��ܾ� üũ�� ó������ ����
//#define TEST_WRITE_SLEEP
//#define TEST_MIF_POSTPAY_CANNOT_READ_SECTOR5
//#define TEST_MIF_POSTPAY_BLOCK20_ALL_ZERO
//#define TEST_MIF_POSTPAY_INVALID_ALIAS
//#define TEST_MIF_PREPAY_ALIAS_0
//#define TEST_NOT_SEND_SUBTERM_TRN_ON_POLLING	// �� ��ũ�ΰ� define�Ǹ� ������ ������ �ŷ��������� ������
//#define TEST_UPDATE_ONE_DAY_BL_PL_AI
//#define TEST_PRINT_TIME_CARD_PROC
//#define TEST_BLPL_CHECK
//#define TEST_DOWN_AND_ROLLBACK
//#define TEST_IDCENTER_KEYSET_REGIST
//#define TEST_DAESA
//#define TEST_SYNC_FW
//#define TEST_SENDING_NOT_SEND_SUBTERM_TRN_ON_POLLING
												// �� ��ũ�ΰ� define�Ǹ� ������ ������ �ŷ��������� ���� �� Ÿ��������� �׽�Ʈ

#endif

