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
*  PROGRAM ID :		keypad_proc.h                                              *
*                                                                              *
*  DESCRIPTION:		This program comm. with Driver & Main Proc./ Comm Proc.    *
*                                                                              *
*  ENTRY POINT:		void CommKpd( void )                                       *
* 					int ASC2HEX(byte *abSrcASC,byte *abDesHEX,byte bLengthSrc) *
* 					short KpdCommProcPolling( byte* bCmd, byte* pbRecvBuf )    *
* 					short ReceiveImgVerfromKpd( byte* pbKpdVer )               *
* 					short SendKpd( byte* pbSendData, word wSendSize )          *
* 					short SendKpdPkt( KPDCMD_COMM_PKT* pstSendData )           *
* 					short SendNewImgProcess( char* pchKpdImgVer )              *
* 					short SendNewImgAfterVerCheck( void )                      *
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
* 2005/08/13 Solution Team  Gwan Yul Kim  Initial Release                      *
*                                                                              *
*******************************************************************************/

#ifndef _KEYPAD_PROC_H
#define _KEYPAD_PROC_H

/*******************************************************************************
*  Inclusion of Header Files                                                   *
*******************************************************************************/
#include <termios.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "../system/bus_type.h"
#include "../system/device_interface.h"
#include "file_mgt.h" 

/*******************************************************************************
*  Structure of Comm Pool Data                                                 *
*******************************************************************************/
typedef struct
{
	byte bCommType;																// 0
	byte abDriverDisplayData[2];												// 1
	byte abLanCardStrength[3];													// 3
	byte bRunKindCode;				// ���� ���� �ڵ�							// 6
	byte bGyeonggiIncheonRangeFareInputWay;
									// �����õŰ�е����Է¹��(������)		// 7
	byte bDCSComm;					// DCS����߿���							// 8
	byte abLanCardQuality[3];													// 9
	byte abTotalFare[3];														// 12
	byte abNowStationID[8];														// 15
	byte abNowStationName[16];													// 23
	byte abRTCTime[14];															// 39
	byte bDrivenEnterType;														// 53
	byte bIsCityTourBus;			// ��Ƽ�����������
									// '0' : ��Ƽ������� �ƴ�
									// '1' : ��Ƽ�������						// 54
} __attribute__((packed)) KPDCOMM_POOL_DATA;

/*******************************************************************************
*  Structure of Comm Packet Data                                               *
*******************************************************************************/
typedef struct
{
	byte bCmd;
	byte abData[1024];
	int nDataSize;
}__attribute__((packed))  KPDCMD_COMM_PKT;

//extern KPDCOMM_POOL_DATA gstKpdPool;
	
/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/	
void CommKpd( void );

short KpdCommProcPolling( byte* bCmd, byte* pbRecvBuf );
short ReceiveImgVerfromKpd( byte* pbKpdVer, byte *abDriverOperatorID );
short SendKpd( byte* pbSendData, word wSendSize );
short SendKpdPkt( KPDCMD_COMM_PKT* pstSendData );
short SendNewImgProcess( char* pchKpdImgVer );
short SendNewImgAfterVerCheck( void );

#endif
