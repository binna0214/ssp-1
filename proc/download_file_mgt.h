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
*  PROGRAM ID :       download_file_mgt.h                                     *
*                                                                              *
*  DESCRIPTION:       �� ���α׷��� ���迡�� �ٿ�ε� ���� ������ ����� �����ϴ� *
*						downloadinfo.dat�� CRD�� �̾�ޱ� ������ �����ϴ� 	   *
*						downfilelink.dat�� CU ���, BLPL�� �ٿ�ε� ���ο�	   *
*						SAM ������ �ٿ�ε� ���θ� üũ�ϴ� ����� �����Ѵ�.	   *
*                                                                              *
*  ENTRY POINT:     short WriteDownFileList( DOWN_FILE_INFO* pstDownFileInfo );*
*					short WriteDownFileInfo( char* pachFileName,  			   *
*											 byte* pabFileVer, 				   *
*											 char chDownStatus, 			   *
*											 int nRecvdFileSize );			   *
*					short ReadDownFileList( char* pchFileName,				   *
*											DOWN_FILE_INFO* stDownFileInfo );  *
*					short DelDownFileList( char* pchFileName,				   *
*										   DOWN_FILE_INFO* stDownFileInfo );   *
*					short WriteRelayRecvFile( FILE_RELAY_RECV_INFO* 		   *
*													pstFileRelayRecvInfo );    *
*					short UpdateRelayRecvFile(  FILE* fdRelayRecv,			   *
*                            	FILE_RELAY_RECV_INFO* 	pstFileRelayRecvInfo );*
*					void WriteBLPLDownInfo( void );							   *
*				    static short CheckSAMInfoDown( void );                     *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       downloadinfo.dat  - ���迡�� �ٿ�ε� ���� ������ ���     *
*                     tmp_c_ch_bl.dat   - �ٿ���� ���� BL���ϸ�( ������Ʈ ���� )*
*                     tmp_c_ch_pl.dat   - �ٿ���� ���� PL���ϸ�( ������Ʈ ���� )*
*                     tmp_c_ch_ai.dat   - �ٿ���� ���� AI���ϸ�( ������Ʈ ���� )*
*                                                                              *
*  OUTPUT FILES:      downloadinfo.dat  - ���迡�� �ٿ�ε���� ������ ���      *
*                     downfilelink.dat  - �̾�ޱ� ������ ����                  *
*                     v_bl_be.dat       - ���� BL�� �ٿ�ޱ� ������ ��������     *
*                     v_pl_be.dat       - ���� PL�� �ٿ�ޱ� ������ ��������     *
*                     v_ai_be.dat       - ���� AI�� �ٿ�ޱ� ������ ��������     *
*                     bl_pl_ctrl.flg    - BLPL�� �ٿ�θ� �������ִ� ����      *
*										  blpl_proc.c���� ������				   *
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
#ifndef _DOWNLOAD_FILE_MGT_H_
#define _DOWNLOAD_FILE_MGT_H_

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "file_mgt.h"


/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
short WriteDownFileList( DOWN_FILE_INFO* pstDownFileInfo );
short WriteDownFileInfo( char* pachFileName,  
						 byte* pabFileVer, 
						 char chDownStatus, 
						 int nRecvdFileSize );
short ReadDownFileList( char* pchFileName, DOWN_FILE_INFO* stDownFileInfo );
short DelDownFileList( char* pchFileName, DOWN_FILE_INFO* stDownFileInfo );
short WriteRelayRecvFile( FILE_RELAY_RECV_INFO* pstFileRelayRecvInfo );
short UpdateRelayRecvFile(  FILE*					fdRelayRecv,
                            FILE_RELAY_RECV_INFO* 	pstFileRelayRecvInfo );
void WriteBLPLDownInfo( void );
short CheckSAMInfoDown( void );
short WriteOperParmFile( byte* pbBuff, long lBuffSize );

#endif