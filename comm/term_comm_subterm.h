
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
*  PROGRAM ID :       sc_read.c                                                *
*                                                                              *
*  DESCRIPTION:       This program reads Smart Card, checks validation of card *
*                     and saves Common Structure.                              *
*                                                                              *
*  ENTRY POINT:       SCRead()               ** optional **                    *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: 0 - Success                                              *
*                     (-) - Error                                              *
*                                                                              *
*  INPUT FILES:       Issure Info Files                                        *
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
* 2005/09/27 Solution Team  woolim        Initial Release                      *
* 2006/04/17 F/W Dev. Team  wangura       ���Ϻи� �� �߰� ����ȭ              *
*                                                                              *
*******************************************************************************/
#ifndef _TERM_COMM_SUBTERM_H
#define _TERM_COMM_SUBTERM_H


typedef struct {
    byte    bEncryptAlgCd;  // 1 ��ȣȭ �˰���
    byte    bIDCenter;      // 1 Ű�¹��� �����id
    byte    abSAMID[8];     // 8 SAM ID
    byte    bSortKey;       // sort key (Ű����)
    byte    abVK[4];        // vk (Ű����)
    byte    abEKV[64];      // ekv ����ȭ��纰 Ű��
    byte    abSign[4];      // sign
} __attribute__((packed))OFFLINE_KEYSET_DATA;


typedef struct { // tot 35
    byte    bEncrytAlgCd;   // �˰���id
    byte    bIDCenter;      // Ű�¹��� �����id
    byte    abSAMID[8];     // sam id
    byte    abEKV[16];      // EKV
    byte    abSign[4];      // Sign
} __attribute__((packed)) OFFLINE_IDCENTER_DATA;



/*******************************************************************************
*  Declaration of Header Files                                                 *
*******************************************************************************/
void SubTermProc( void );
short RegistOfflineID2PSAMbyEpurseIssuer(void);
short RegistOfflineKeyset2PSAMbyEpurseIssuer(void);

#endif
