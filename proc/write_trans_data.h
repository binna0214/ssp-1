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

/*

�����ŷ���������			YYYYMMDDhhmmss.trn					MAIN_TRANS
�����ŷ���������			alight_term_td.tmp		203B		SUB_TRANS
�������ŷ���������		YYYYMMDDhhmmss.trn					SUB_REMAKE
�ӽô������				temptd.dat							TEMP_REMAKE
��������ŷ���������		aboard_term_td.bak					BACKUP_TRANS
��������ŷ���������		alight_term_td.bak					BACKUP_TRANS
�����ӽô������			YYYYMMDDhhmmss.10					SUB_TEMP_REMAKE

*/

#ifndef _WRITE_TRANS_DATA_H_
#define _WRITE_TRANS_DATA_H_

typedef struct {
	byte bDriveNow;							// ���౸��
	byte abTotalCnt[9] ;					// �ѰǼ�
	byte abTotalAmt[10];					// �ѱݾ�
	byte abTRFileName[18];					// TR���ϸ�
}__attribute__((packed)) CONTROL_TRANS;

short WriteTransHeader( void );
void WriteTransData( TRANS_INFO *pstTransInfo );
short WriteTransDataForCommProcess( byte *abTransTD );
void WriteCashTransData( byte bUserType );
short WriteTransTail( void );
short UpdateTransDCSRecvDtime( byte *abTransFileName );
short RemakeTrans( void );

#endif
