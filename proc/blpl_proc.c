/*******************************************************************************
*																			   *
*					   KSCC	- Bus Embeded System							   *
*																			   *
*			 All rights	reserved. No part of this publication may be		   *
*			 reproduced, stored	in a retrieval system or transmitted		   *
*			 in	any	form or	by any means  -	 electronic, mechanical,		   *
*			 photocopying, recording, or otherwise,	without	the	prior		   *
*			 written permission	of LG CNS.									   *
*																			   *
********************************************************************************
*																			   *
*  PROGRAM ID :		  blpl_proc.c											   *
*																			   *
*  DESCRIPTION:		  ī�庰�� bl(blacklist),pl(positive list)�� �Ǻ��ϱ� ���� *
*                     BL/PL ������ �˻��Ͽ� �� ����� �˷��ִ� ����� ����Ѵ�   *
*                     BL/PL ������ ���� ���ϰ� �����̰� �̿� ����              *
*					  ���ο� bl/pl����Ʈ�� �߰��ɰ�� BL�� ���� Merge��      *
*                     PL�� ���� Update�� �Ͽ� ���ŵ� ������ �����Ѵ�.        *														   *
*																			   *
*  ENTRY POINT:		  �����ϴ� ��� Main���� BLPLMerge �Լ��� ȣ��			   *
*					  �˻��ϴ� ��쿡�� ī�峪 ������μ������� ����������     *
*                     SearchBLinBus �Լ��� SearchPLinBus �Լ��� ȣ��           *
*													                           *
*  INPUT PARAMETERS:  void                                                     *
*                                                                              *
*																			   *
*  RETURN/EXIT VALUE: void													   *
*																			   *
*  INPUT FILES:	������ ���� BL/PL���ϵ�� ���� BL/PL ���ϵ� 	               *
*																			   *
*            TEMP_MASTER_BL_FILE "temp.bl" : ������ ���� �ӽ� BL ���Ϸ� ������ *    
*                                         ���������� ������ ���ο� ���� BL�� ��*    
*            MASTER_BL_FILE			  "c_fi_bl.dat"  : Master BL File          *    
*            MASTER_PREPAY_PL_FILE	  "c_fa_pl.dat"  : ���� PL File            *    
*            MASTER_POSTPAY_PL_FILE	  "c_fd_pl.dat"  : �ĺ� PL File            *    
* 			 MASTER_AI_FILE			  "c_fi_ai.dat"  : �ż��� PL File          *    
* 			 CHANGE_BL_FILE			  "c_ch_bl.dat"  : ���� BL File            *    
* 			 UPDATE_PL_FILE			  "c_ch_pl.dat"  : ���� ��/�ĺ� PL File    *    
* 			 UPDATE_AI_FILE			  "c_ch_ai.dat"  : ���� �ż��� PL File     *    
* 			 POSTPAY_ISSUER_INFO_FILE "c_dp_inf.dat" : �ĺҹ���� File         *	
*																			   *
*  OUTPUT FILES: ������ - ���ο� ���� BL/PL ���ϵ�(�����̸��� ����)            *
*                                                                              *
*            MASTER_BL_FILE			  "c_fi_bl.dat"  : New Master BL File      *    
*            MASTER_PREPAY_PL_FILE	  "c_fa_pl.dat"  : New ���� PL File        *    
*            MASTER_POSTPAY_PL_FILE	  "c_fd_pl.dat"  : New �ĺ� PL File        *    
* 			 MASTER_AI_FILE			  "c_fi_ai.dat"  : New �ż��� PL File      *  
*																			   *
*  SPECIAL LOGIC: �������� SearchBL, SeachPL, MergeBL, UpdatePL�� ���� ����  *
*                                                                              *																		   *
********************************************************************************
*						  MODIFICATION LOG									   *
*																			   *
*	 DATE				 SE	NAME					  DESCRIPTION			   *
* ---------- ---------------------------- ------------------------------------ *
* 2005/09/29 Solution Team	Boohyeon Jeon Initial Release(PL Part)			   *
* 2005/09/29 Solution Team	Woolim		  Initial Release(BL Part)			   *
*																			   *
*******************************************************************************/
#include "blpl_proc.h"
#include "main.h"
#include "file_mgt.h"
#include "../system/bus_type.h"
#include "../system/bus_errno.h"
#include "version_mgt.h"
#include "main_process_busTerm.h"

/*******************************************************************************
*  Declaration of Function Prototype    									   *
*******************************************************************************/
short BLMergeMain( void );
short PLMergeMain( void );
short AIMergeMain( void );
short WriteBLPLFileCRC(	int TypeOfCRCFile );
static word GetBLTypePrefixInFile(byte *prefix);
static word GetBLTypePrefix(byte *SixAscPrefix);
/*******************************************************************************
*  Declaration of Defines in BL												   *
*******************************************************************************/
/*
 * ������ ���¸� �˷��ִ� ���Ϸμ� 3byte�� ����
 * 1byte : [0] : B/L,	2byte [1] : �����ĺ�P/L, ���ĺ�	P/L, 3byte[2]:�ż��� P/L
 * �� byte�� ���� 1:���� ��û,	2:������, 0:�������� or �����ʿ����.	
 */
#define	MERGE_FLAG_FILE							"bl_pl_ctrl.flg"
/*
 * ���� FLAG������ �� BYTE�� ���� �ǹ̸� ����
 */	 
#define	MERGE_COMPLETE_OR_NOT_NEED            ( 0 ) // �����Ϸ� OR �ʿ����
#define MERGE_REQUEST                         ( 1 ) // ������û
#define MERGE_WORKING_NOW                     ( 2 ) // ����������
/*
 * FILE ACCESS�� ���� ���翩���Ǵܽ� ����� ��ũ��
 */
#define FILE_EXIST                            ( 0 )
/*
 * CRC�� ��Ͻ� ����� ���� ������ ���� ��ũ��
 */
#define BL_FILE 							  ( 0 )
#define PL_FILE  						      ( 1 )
#define AI_FILE 							  ( 2 ) 
/*
 * CRC�� ��Ͻ� ����� ���� ������ ���� ��ũ��
 */
#define PL_CARD_NO_SIZE                       ( 20 )

#ifndef	__GCC
/*******************************************************************************
*																			   *
*  FUNCTION	ID :	  BLPLMerge										           *
*																			   *
*  DESCRIPTION :	  BL/PL/AI�� �������࿩�θ� üũ�Ͽ� ������ �����Ѵ�  	   *
*					  �����Լ����� ȣ���ϵ��� �Ǿ� �ִ� ���Լ��� �����ÿ�      *
*                     ���μ����� Fork�Ͽ� ������ ���μ����� �����ϰ� �Ǿ��ִ�  *
*																			   *
*  INPUT PARAMETERS:  None													   *
*																			   *
*  RETURN/EXIT VALUE: SUCCESS - ���༺��									   *
*			  BL_PREFIX_ERROR_EVENT	: BL Prefix ����   			               *
*             BL_SERIAL_ERROR_EVENT : BL Prefix�� ������ ������ ��ȣ����       *
*             BL_SIZE_ERROR_EVENT   : BL SIZE ����                             *
*																			   *
*  Author  : Woolim															   *
*																			   *
*  DATE	   : 2005-09-03														   *
*																			   *
*  REMARKS : BL/PL/AI�������࿩�θ� �˱����� bl_pl_ctrl.flg������ üũ�Ͽ�     *
*            ������ �����Լ��� �����ϰ� ���� �� �ش� ������ CRC����            *
*            install.dat���Ͽ� ����Ѵ�.                     				   *
*            ���� ���н� RollBackBLPLVer �Լ��� ȣ���Ͽ� BLPL ������ ������    *
*            �������� �ѹ��Ͽ� ����κ��� �ٽ� ���������� �����Ҽ� �ֵ���      *
*            ó���ϰ� �ȴ�.                                                    *
*                                                        					   *
*******************************************************************************/
short BLPLMerge(void)
{
	int fd, ret = 0;
	int	bl_ret = 0,	pl_ret = 0,	ai_ret = 0;
	pid_t pid;
	short sRetVal =	SUCCESS;

	/*
	 * �� byte�� ��Ÿ���� �ǹ̴� �Ʒ��� ����.
     * 1byte : [0] : B/L,	2byte [1] : �����ĺ�P/L, ���ĺ�	P/L, 3byte[2]:�ż��� P/L
     * �� byte�� ���� �ǹ̴�
     *  MERGE_REQUEST:���� ��û
     *  MERGE_WORKING_NOW:������
     *  MERGE_COMPLETE_OR_NOT_NEED:�������� or �����ʿ����.	
	 */
	byte merge_flag[3];


	/*
     * ���� BL/PL������ ������ �������� ���翩�� Ȯ��
	 */
	bl_ret = access( CHANGE_BL_FILE,F_OK);  // ���� BL
	pl_ret = access( UPDATE_PL_FILE,F_OK);  // ���� PL(������,���ĺ�)
	ai_ret = access( UPDATE_AI_FILE,F_OK);  // ���� PL(AI��� �� : �ż�/�ĺ�)

	if ( bl_ret	== 0 ||	pl_ret == 0	|| ai_ret == 0 )
	{
		printf( "[BLPLMerge] ������ ������ ������\n" );
	}
	else
	{
//		printf( "[BLPLMerge] ������ ������ �������� ����\n" );
		return -1;
	}

	memset(merge_flag, 0, 3);

    /*
     * �������¸� ��Ÿ���� flag���� Open
     */     
	fd = open( MERGE_FLAG_FILE, O_RDWR );
	if (fd < 0)	
	{
		printf( "[BLPLMerge] bl_pl_ctrl.flg ���� OPEN ����\n" );
		return -1;
	}
	
	/*
	 * ���� flag���� Read
	 */
	ret	= read(fd, merge_flag, 3);

	if (ret	== 2)
	{
		printf( "[BLPLMerge] bl_pl_ctrl.flg�� �������̹Ƿ� ��ȯ��\n" );
		merge_flag[2] =	0;
		lseek(fd, 0, SEEK_SET);
		write(fd, merge_flag, 3);
	}
	close(fd);

    /*
     * BL/PL�� ���� ������û���� Ȯ��
     */
	if ( merge_flag[0] == MERGE_REQUEST || merge_flag[1] == MERGE_REQUEST ||
		 merge_flag[2] == MERGE_REQUEST )
	{
	   /*
		* ��������� ���μ��� fork����
		*/
		pid	= fork();
		if (pid	< 0)
		{
			printf( "[BLPLMerge] fork() ����\n" );
		}
		else
		{
			if (pid	== 0)
			{
				/*
				 * �����޸𸮳� �������� ������ �������������� ����
				 */
				gpstSharedInfo->boolIsBLPLMergeNow = TRUE;
				printf( "[BLPLMerge] gpstSharedInfo->boolIsBLPLMergeNow = TRUE ����\n" );

				/*
				 * ���� BL/PL/AI ��û�� ���� ������ Merge Main�Լ� ����
				 */
				if ( merge_flag[0] == MERGE_REQUEST ) // BL ���� ��û
				{
					gpstSharedInfo->boolIsBLMergeNow = TRUE;
					sRetVal = BLMergeMain();		  			
					WriteBLPLFileCRC( BL_FILE );      // CRC�� ���
					
					if ( sRetVal != SUCCESS )
					{
						goto BLPLEXIT;
					}
				}

				if ( merge_flag[1] == MERGE_REQUEST ) // PL ���� ��û
				{
					gpstSharedInfo->boolIsMifPrepayPLMergeNow = TRUE;
					gpstSharedInfo->boolIsPostpayPLMergeNow = TRUE;
					sRetVal = PLMergeMain();
					WriteBLPLFileCRC( PL_FILE );      // CRC�� ���
					if ( sRetVal != SUCCESS )
					{
						goto BLPLEXIT;
					}					
				}

				if ( merge_flag[2] == MERGE_REQUEST ) // AI ���� ��û
				{
					gpstSharedInfo->boolIsSCPrepayPLMergeNow = TRUE;
					sRetVal = AIMergeMain();
					WriteBLPLFileCRC( AI_FILE );      // CRC�� ���
					
					if ( sRetVal != SUCCESS )
					{
						goto BLPLEXIT;
					}				
				}

BLPLEXIT:
				/*
				 * �����޸𸮳� �������� ������ �����Ϸ�� ����
				 */				
				gpstSharedInfo->boolIsBLPLMergeNow = FALSE;

				gpstSharedInfo->boolIsBLMergeNow = FALSE;
				gpstSharedInfo->boolIsMifPrepayPLMergeNow = FALSE;
				gpstSharedInfo->boolIsPostpayPLMergeNow = FALSE;
				gpstSharedInfo->boolIsSCPrepayPLMergeNow = FALSE;
				
				exit(0);
			}
		}
	}
	return ErrRet( sRetVal );
}

#endif

/*******************************************************************************
*																			   *
*  FUNCTION	ID :	  BLMergeMain										       *
*																			   *
*  DESCRIPTION :	  BL�� ������ �����ϴ� MergeBL�Լ���                 	   *
*                     ȣ���ϰ� �������п� ���� ����ó�� �� ��������� ���     *
*																			   *
*  INPUT PARAMETERS:  None													   *
*																			   *
*  RETURN/EXIT VALUE:                                                          *
*             SUCCESS - ���༺���� ������ ���� BL���ϰ� ���� ������            *
*                               �����ϰ� ������ �ӽ� ���������� ���ο� ����BL  *
*                               ���Ϸ� rename�Ѵ�.           				   *
*             ���н� - �������� �ӽð������ϰ� ���������� �����ϰ� ������������*
*                      ���Ϲ����� �ѹ��Ѵ�.                                    *
*                      ���� MergeBL�Լ����� ������ �����ڵ带 event.trn���Ͽ�  *
*                      ����Ѵ�.                                               *
*																			   *
*  Author  : Woolim															   *
*																			   *
*  DATE	   : 2005-09-03														   *
*																			   *
*  REMARKS : MergeBL�� ������ �Լ�										   *
*                                                        					   *
*******************************************************************************/
short BLMergeMain( void )
{
	short retVal = SUCCESS;
	int	fd	= 0;
	byte merge_flag[3];
	
	memset( merge_flag, 0x00, sizeof( merge_flag ) );
	
	printf("BL Merge Start.\n");
	/*
	 * MergeBL ������ �Լ� ȣ��
	 */
	if ((retVal = MergeBL( MASTER_BL_FILE, CHANGE_BL_FILE)) != SUCCESS )
	{
		printf("BL Merge Fail.\n");
		/*
		 * ���н� ���ϻ��� �� �����ѹ�- RollbackBLPLVer(BL�ѹ�, PL�ѹ�, AI�ѹ�)
		 */
		unlink(CHANGE_BL_FILE);               // ���� BL���� ����
		unlink(TEMP_MASTER_BL_FILE);          // �ӽ� BL���� ����
		RollbackBLPLVer( TRUE, FALSE, FALSE); // BL���� �����ѹ�
		/*
		 * ���� FLAG���Ͽ� ���� �Ϸ� ����
		 */		
		fd = open(MERGE_FLAG_FILE, O_RDWR);
		if ( fd	 > 0)
		{
			merge_flag[0] =	0;
			lseek(fd, 0, SEEK_SET);
			write(fd, merge_flag, 1);
			close(fd);
		}
		/*
		 * �����ڵ� ���
		 */			
		if ( retVal	== ERR_BL_PREFIX_ERROR	)  // BL Prefix ����
		{
			ctrl_event_info_write(BL_PREFIX_ERROR_EVENT);
			return ErrRet( retVal );
		}
		if ( retVal	== ERR_BL_SERIAL_ERROR	)  // Prefix�� ������ ��������ȣ����
		{
			ctrl_event_info_write(BL_SERIAL_ERROR_EVENT);
			return ErrRet( retVal );
		}
		if ( retVal	== ERR_BL_SIZE_ERROR )     // BL Size ����
		{
			ctrl_event_info_write(BL_SIZE_ERROR_EVENT);  
			return ErrRet( retVal );
		}
		return ERR_BLPL_PROC_BL_MERGE_FAIL;
	}
	else
	{
		/*
		 * ���� FLAG���Ͽ� ���� �Ϸ� ����
		 */			
		fd = open(MERGE_FLAG_FILE, O_RDWR);
		merge_flag[0] =	0;
		lseek(fd, 0, SEEK_SET);
		write(fd, merge_flag, 1);
		close(fd);
		/*
		 * ������ ���ϻ��� �� �ӽð������� Rename
		 */			
		unlink(CHANGE_BL_FILE);                      // ���� BL ���ϻ���
		unlink(MASTER_BL_FILE);                      // ���� BL ���ϻ���
		rename(TEMP_MASTER_BL_FILE, MASTER_BL_FILE); // ���ο� �������Ϸ� ����
		unlink(UPDATE_BL_BEFORE_VER_FILE);           // �������� BL�������� ����
		printf("BL Merge Complete.\n");
		return SUCCESS;
	}
}
/*******************************************************************************
*																			   *
*  FUNCTION	ID :	  PLMergeMain										       *
*																			   *
*  DESCRIPTION :	  PL�� ������ �����ϴ� UpdatePL�Լ��� PL�� �Ķ���ͷ� �־� *
*                     ȣ���ϰ� �������п� ���� ����ó�� �� ��������� ���     *
*																			   *
*  INPUT PARAMETERS:  None													   *
*																			   *
*  RETURN/EXIT VALUE:                                                          *
*             SUCCESS - ���༺�� - ���� PL������ �����Ѵ�.                     *
*             ���н� - ���� PL ������ �����ϰ� ������������ ���Ϲ����� �ѹ��Ѵ�*
*																			   *
*  Author  : Woolim															   *
*																			   *
*  DATE	   : 2005-09-03														   *
*																			   *
*  REMARKS : UpdatePL�� ������ �Լ�										   *
*                                                        					   *
*******************************************************************************/
short PLMergeMain( void )
{
	short retVal = SUCCESS;
	int	fd	= 0;
	byte merge_flag[3];
	
	memset( merge_flag, 0x00, sizeof( merge_flag ) );	
	printf("PL Merge Start.\n");

	if ((retVal	= UpdatePL(PL))	< 0)
	{
		printf("PL Merge Fail.\n");
		unlink(UPDATE_PL_FILE);
		RollbackBLPLVer( FALSE,	TRUE, FALSE);
		fd = open(MERGE_FLAG_FILE, O_RDWR);
		if ( fd	 > 0)
		{
			merge_flag[1] =	0;
			lseek(fd, 1, SEEK_SET);
			write(fd, merge_flag+1,	1);
			close(fd);
		}
		return ERR_BLPL_PROC_PL_MERGE_FAIL;
	}
	else
	{
		fd = open(MERGE_FLAG_FILE, O_RDWR);
		merge_flag[1] =	0;
		lseek(fd, 1, SEEK_SET);
		write(fd, merge_flag+1,	1);
		close(fd);
		unlink(UPDATE_PL_FILE);
		unlink(UPDATE_PL_BEFORE_VER_FILE);
		printf("PL Merge Complete.\n");
		return SUCCESS;
	}
}
/*******************************************************************************
*																			   *
*  FUNCTION	ID :	  AIMergeMain										       *
*																			   *
*  DESCRIPTION :	  AI�� ������ �����ϴ� UpdatePL�Լ��� AI�� �Ķ���ͷ� �־� *
*                     ȣ���ϰ� �������п� ���� ����ó�� �� ��������� ���     *
*																			   *
*  INPUT PARAMETERS:  None													   *
*																			   *
*  RETURN/EXIT VALUE:                                                          *
*             SUCCESS - ���༺�� - ���� PL������ �����Ѵ�.                     *
*             ���н� - ���� AI ������ �����ϰ� ������������ ���Ϲ����� �ѹ��Ѵ�*
*																			   *
*  Author  : Woolim															   *
*																			   *
*  DATE	   : 2005-09-03														   *
*																			   *
*  REMARKS : UpdatePL�� ������ �Լ�										   *
*                                                        					   *
*******************************************************************************/
short AIMergeMain( void )
{
	short retVal = SUCCESS;
	int	fd	= 0;
	byte merge_flag[3];
	
	memset( merge_flag, 0x00, sizeof( merge_flag ) );	
	printf("AI Merge Start.\n");

	if ((retVal	= UpdatePL(AI))	< 0	)
	{
		printf("AI Merge Fail.\n");
		unlink(UPDATE_AI_FILE);
		RollbackBLPLVer( FALSE,	FALSE, TRUE);
		fd = open(MERGE_FLAG_FILE, O_RDWR);
		if ( fd	 > 0)
		{
			merge_flag[2] =	0;
			lseek(fd, 2, SEEK_SET);
			write(fd, merge_flag+2,	1);
			close(fd);
		}
		return ERR_BLPL_PROC_AI_MERGE_FAIL;
	}
	else
	{
		fd = open(MERGE_FLAG_FILE, O_RDWR);
		merge_flag[2] =	0;
		lseek(fd, 2, SEEK_SET);
		write(fd, merge_flag+2,	1);
		close(fd);
		unlink(UPDATE_AI_FILE);
		unlink(UPDATE_AI_BEFORE_VER_FILE);
		printf("AI Merge Complete.\n");
		return SUCCESS;
	}
}
/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       SearchPLinBus                                            *
*                                                                              *
*  DESCRIPTION:       �Էµ� ������ ���� PL�� �˻��ϰ� �� ����� �����Ѵ�.     *
*                                                                              *
*  INPUT PARAMETERS:  *abCardNo - ī���ȣ(20byte)��Ʈ�� ������                *
*                     dwAliasNo - alias ��ȣ                                   *
*                     *bResult - PLüũ ����� �������� ���� byte ���� ������  *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ����                                      *
*                                                                              *
*  Author:            Boohyeon Jeon                                            *
*                                                                              *
*  DATE:              2006-01-09                                               *
*                                                                              *
*  REMARKS:           �������� SearchPL�Լ��� ȣ���Ѵ�.                      *
*                                                                              *
*******************************************************************************/
short SearchPLinBus(byte *abCardNo, dword dwAliasNo, byte *bResult)
{
	short sResult = SUCCESS;
	/*
	 * �����⿡�� PL��û�� �ʿ��� ���� ����
	 */
	byte pcCmdData[40];        // �����޸𸮿� Ŀ�ǵ�� �����͸� �ֱ����� ����
	byte SharedMemoryCmd;      // �����޸��� Ŀ�ǵ带 ������ Ȯ���ϱ� ���� ����
	word wCmdDataSize;         // �����޸��� Ŀ�ǵ�� �������� ������ ����
	/*
	 * �����⿡�� PL��û �� ����ϱ����� Loop Count����
	 */	
	int check_wait;   
	/*
	 * �����⿡�� PL��û�� �����⿡�� ������ �����ڵ�
	 */		
	short PLCheckErrCode =0;	
	/*
	 * �����⿡�� PL��û ���
	 */	
	*bResult = 0;

#ifndef __GCC

	/*
	 * �����ܸ��⿡�� PL��û�� �� ��� 
	 */
	if ( gboolIsMainTerm != TRUE )
	{
		/*
		 * �����޸𸮿� PL��û�� ���� ������ ����
		 */
		memset( pcCmdData, 0x00, sizeof(pcCmdData) );
		pcCmdData[0] = CMD_REQUEST;							// PL��û(1byte) 
		memcpy( pcCmdData + 1, abCardNo, PL_CARD_NO_SIZE);	// ī���ȣ ����(20byte)
		memcpy( pcCmdData + 21, &dwAliasNo, sizeof(dwAliasNo) );
															// alias ����(4byte)

		while(1)
		{
		   /*
		    * �����޸𸮿� PL��û����(25byte) ���� 
		    */
			sResult = SetSharedCmdnData( CMD_PL_CHECK, pcCmdData, 25 );
			if ( sResult < 0 )
			{
				continue;
			}
			else
			{
				break;
			}
		}
	   /*
	    * �����޸� PL��û�� ���ð� Loop Count ���� 
	    */
		check_wait = 1000000;

	   /*
	    * ���� SearchPL ��û�� ����� ������ ��� LoopCount���� üũ
	    * ��� ���Ͻ� ��ȯ
	    */
		while( check_wait-- )
		{
		   /*
		    * �����޸� ��û�� PL����� �Դ��� ���θ� üũ
		    */			
		    memset( pcCmdData, 0x00, sizeof(pcCmdData) );
			GetSharedCmd(&SharedMemoryCmd, pcCmdData, &wCmdDataSize);
			
	   		/*
		    * �����޸� ��û�� PL��� ������ -'4' : üũ�Ϸ� '8' : üũ�Ұ�
		    */	
			if ( SharedMemoryCmd == CMD_PL_CHECK )
			{
				if (pcCmdData[0] == '4')          // Check complete
				{	
					*bResult = pcCmdData[3];
					break;
				}
				else if (pcCmdData[0] == '8')     // Check fail
				{				
					*bResult = -1;
					/*
					* �����⿡�� �˻��Ұ��� ���� �����ڵ� ���� 
					*/
					memcpy( &PLCheckErrCode, &pcCmdData[1], 2);
					ClearAllSharedCmdnData();
					return PLCheckErrCode;
				}
			}
		}

		//üũ Ÿ�Ӿƿ���
		if	(check_wait == -1)
		{
			ClearAllSharedCmdnData();
			*bResult = -1;
			return ERR_BLPL_PROC_PL_CHECK_TIMEOUT;
		}
		ClearAllSharedCmdnData();
		return ErrRet( sResult );
	}

#endif

		sResult  = SearchPL(dwAliasNo, bResult);

		return sResult;

}

/*******************************************************************************
*																			   *
*  FUNCTION	ID :	  SearchBL												   *
*																			   *
*  DESCRIPTION :	  This program is main of BL Search						   *
*																			   *
*  INPUT PARAMETERS:  byte *ASCPrefix -	6 bytes	Prefix(ASCII): 1byte = 1number *
*					  dword	dwCardNum -	9 bytes	Card Number	 : 1byte = 1number *
*					  byte *bBLResult -	Result of Search BL	File			   *
*																			   *
*																			   *
*  RETURN/EXIT VALUE:  RETURN												   *
*					   1)  0 : ����	- ��� Pass(0),	Not	Pass(1)				   *
*					   2) -1 : "ī�带 �ٽ�	���ּ���"						   *
*																			   *
*  Author  : Woolim															   *
*																			   *
*  DATE	   : 2005-09-03														   *
*																			   *
*  REMARKS : ���� ī��ó�������� RETURN����	������ ��쿡��	bBLResult����	   *
*			 �Ǻ��Ͽ� PASS(0),NOT PASS(1)ó���ϰ� �ȴ�.						   *
*			 RETURN����	����(-1)��쿡�� "ī�带 �ٽ� ���ּ���"ó���� �Ѵ�.	   *
*			   - ����BL	�˻��� Open/Read Error : ���� BL�˻�				   *
*			   - ����BL	�˻��� Open/Read Error : 3ȸ ��õ�	�� - Pass(0)	   *
*																			   *
*******************************************************************************/
short SearchBLinBus(byte *abCardNo, byte *ASCPrefix, dword dwCardNum,
	byte *bBLResult)
{
	int	check_wait = 0;     	    /* ���� BL�˻� �� ���ð�(LoopCnt)*/
	byte pcCmdData[40];		         /* �����޸� Data ����    */
	byte SharedMemoryCmd;           /* �����޸� Command ���� */
	word wCmdDataSize;                  /* �����޸� Data Size    */
	short sRetVal = SUCCESS;           /* Return Variable         */
	word wBLtypePrefix;
   /* 
	* ���������뺯�� - �����⿡�� BL�˻����н�	�����ϴ� �����ڵ带 �����ϴ� ���� 
	*/
	short BLCheckErrCode = 0; 	
	
#ifdef TEST_BLPL_CHECK
	byte debug_pcCmdData[40];
	byte debug_SharedMemoryCmd;
	word debug_wCmdDataSize;
	dword debug_alias;
#endif

#ifdef TEST_BLPL_CHECK
	PrintlnASC(	"[SearchBL]	ASCPrefix :",  ASCPrefix, 6	);
	printf("[SearchBL] CardNum	: %lu\n", dwCardNum);
#endif

	/* �����ܸ�����	��� *//////////////////////////////////////////////////////
	if	(gboolIsMainTerm !=	TRUE )
	{
		memset(	pcCmdData, 0x00, sizeof(pcCmdData));
		pcCmdData[0] = '1';
		memcpy(	pcCmdData + 1, abCardNo, PL_CARD_NO_SIZE );
		memcpy(	pcCmdData + 21, ASCPrefix,	6);
		memcpy(	pcCmdData + 27, &dwCardNum,	sizeof( dwCardNum ) );

		/* �����޸𸮿��� BLüũ�� ��û	: Prefix(6)	6byte��	CardNum(9):	4byte */
#ifdef TEST_BLPL_CHECK
		printf("[BLPLProc]���� SearchBL->���������޸�	��û����////////////\n");
#endif
		while(1)
		{
			sRetVal	= SetSharedCmdnData( 'B', pcCmdData, 31);
			if ( sRetVal < 0 )
			{
#ifdef TEST_BLPL_CHECK
				printf("[2]Error!!SearchBL->���������޸� ��û����/////////\n");
				memset(debug_pcCmdData,	0x00, sizeof(debug_pcCmdData));
				GetSharedCmd(&debug_SharedMemoryCmd, debug_pcCmdData, &debug_wCmdDataSize);
				printf(	"[3]�Ӱ� ����ֳ�?====SharedMemoryCmd =	%c====\n", debug_SharedMemoryCmd );
#endif
				continue;
			}
			else
			{
#ifdef TEST_BLPL_CHECK
				printf("[BLPLProc]���� SearchBL->���������޸�	��û����////\n");
				memset(debug_pcCmdData,	0x00, sizeof(debug_pcCmdData));
				GetSharedCmd(&debug_SharedMemoryCmd, debug_pcCmdData, &debug_wCmdDataSize);
				memcpy(	&debug_alias, debug_pcCmdData + 7, sizeof( debug_alias ) );
				printf(	"\t��û	Cmd=%c,	��û blnum=%lu \n",	debug_SharedMemoryCmd,debug_alias );
#endif
				break;
			}
		}


		check_wait = 1000000;
		while (check_wait--)
		{
			/* �����޸𸮿��� BL����� �Դ��� ���θ�	üũ */
			memset(	pcCmdData, 0x00, sizeof(pcCmdData) );
			GetSharedCmd( &SharedMemoryCmd,	pcCmdData, &wCmdDataSize );
#ifdef TEST_BLPL_CHECK
			if ( check_wait%10000 == 0)
			{
				printf("[BLPLProc�����] Check_Cmd : %c", SharedMemoryCmd);
				PrintlnASC("[BLPLProc�����]Check_pcCmdData	: ",pcCmdData+1, 10);
			}
#endif
			if	(SharedMemoryCmd ==	'B')
			{			
				if ( pcCmdData[0] =='4') /*	check completed	*/
				{							
			
					*bBLResult = pcCmdData[3];
					break;
#ifdef TEST_BLPL_CHECK
					printf("[�������]BL Check Result ==> %d[hex:%x]\n\n\n", pcCmdData[1],pcCmdData[1]);
#endif					
				}
				else if	(pcCmdData[0] == '8')/*	check�Ұ� */
				{
#ifdef TEST_BLPL_CHECK
					printf("[�������]check�Ұ�\n");
#endif				
					*bBLResult = -1;
					memcpy( &BLCheckErrCode, &pcCmdData[1], 2);
					ClearAllSharedCmdnData();
					return BLCheckErrCode;
				}
			}
		}

		/* ������û	BLüũ Ÿ�Ӿƿ���-"ī�带 �ٽ� ���ּ���" *//////////////////
		if	(check_wait	== -1)
		{
#ifdef TEST_BLPL_CHECK
			printf("[�������]üũ Ÿ�Ӿƿ�	: �����ڵ� ����\n");
			printf("ClearAllSharedCmdnData\n");
#endif
			ClearAllSharedCmdnData();
			*bBLResult = -1;
			return ERR_BLPL_PROC_BL_CHECK_TIMEOUT;
		}
#ifdef TEST_BLPL_CHECK
		printf("ClearAllSharedCmdnData\n");
#endif
		ClearAllSharedCmdnData();
		return ErrRet( sRetVal );
	}

	/*
	 *  �������� ��� �������� SearchBL �Լ� ȣ��
	 */
	/* 
	 * ���� BL���� ����� �˻������� ī���ȣ�� Prefix�� ���εǴ� 
	 * �����ڵ�(CompCode) ã��
	 */
	 wBLtypePrefix = GetBLTypePrefix( ASCPrefix );
	
   	 sRetVal = SearchBL( wBLtypePrefix , dwCardNum, bBLResult );

	 return ErrRet( sRetVal );
}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       ReadBLPLFileCRC                            			   *
*                                                                              *
*  DESCRIPTION:       blpl_crc.dat������ CRC���� �о� �Է°��� �ּҿ� �����Ѵ�.*
*                                                                              *
*  INPUT PARAMETERS:   byte* abBLCRC           - BL CRC                        *
*	                   byte* abMifPrepayPLCRC  - ������ PL CRC	               *
*	                   byte* abPostpayPLCRC    - �ĺ� PL CRC                   *
*	                   byte* abAICRC		   - �ż��� PL CRC  	           *							           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                     ERR_BLPL_PROC_OPEN_BLPL_CRC_FILE - blpl_crc.dat open���� *
*                     ERR_BLPL_PROC_READ_BLPL_CRC_FILE - blpl_crc.dat read���� *
*																			   *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:          BLPL_CRC_FILE - blpl_crc.dat����                          *
*                    �� 16byte�� ���Ϸ� ���α����� ������ CRC 4byte�� ����     *
*                    0 ~ 4 byte   - BL CRC                                     *
*                    5 ~ 8 byte   - ������ PL CRC							   *
*                    9 ~ 12 byte  - �ĺ� PL CRC                                *
*                    13 ~ 16 byte - �ż��� PL CRC                              *
*******************************************************************************/
short ReadBLPLFileCRC( byte* abBLCRC, 
	                   byte* abMifPrepayPLCRC, 
	                   byte* abPostpayPLCRC,
	                   byte* abAICRC )
{
	int	fdFile = 0;
	byte abTmpBLPLFileCRC[16];	
	short sRetVal = SUCCESS;
	char chSpace[5] = { 0, };
	/*
	 * blpl_crc.dat���� ����
	 */		 
	fdFile = open( BLPL_CRC_FILE, O_CREAT | O_TRUNC | O_RDWR ,OPENMODE);
	if ( fdFile	< 0	)
	{
		/*
		 * blpl_crc.dat���� ���¿��� �߻��� space(0x20)���� ���� ä���.
		 * 0x20�� �ƴ� 'A'(0x41)�� ä���� ������ �������ڿ� ���� ����
		 */		
		memcpy( abBLCRC, chSpace, 4 );        
		memcpy( abMifPrepayPLCRC, chSpace, 4 ); 
		memcpy( abPostpayPLCRC, chSpace, 4 );   
		memcpy( abAICRC, chSpace, 4 );			 	
		DebugOut("[BLPLPROC]blpl_crc.dat open ���� \n");
		return ErrRet( ERR_BLPL_PROC_OPEN_BLPL_CRC_FILE );
	}
	/*
	 * blpl_crc.dat���� read
	 */	
	sRetVal	= read(	fdFile,	abTmpBLPLFileCRC , sizeof( abTmpBLPLFileCRC ));
	close( fdFile );
	if ( sRetVal < 0 )
	{
		/*
		 * blpl_crc.dat���� Read���� �߻��� space(0x20)���� ���� ä���.
		 * 0x20�� �ƴ� 'A'(0x41)�� ä���� ������ �������ڿ� ���� ����
		 */		
		memcpy( abBLCRC, chSpace, 4 );        
		memcpy( abMifPrepayPLCRC, chSpace, 4 ); 
		memcpy( abPostpayPLCRC, chSpace, 4 );   
		memcpy( abAICRC, chSpace, 4 );		
		DebugOut("[BLPLPROC]blpl_crc.dat read ���� \n");
		return ErrRet( ERR_BLPL_PROC_READ_BLPL_CRC_FILE );
	}
       
	/*
	 * �Է� �Ķ���Ϳ� CRC�� Copy
	 */		
	memcpy( abBLCRC, abTmpBLPLFileCRC, 4 );            /* BL CRC ��� */
	memcpy( abMifPrepayPLCRC, abTmpBLPLFileCRC+4, 4 ); /* ������ PL CRC ��� */
	memcpy( abPostpayPLCRC, abTmpBLPLFileCRC+8, 4 );   /* �ĺ� PL CRC ��� */
	memcpy( abAICRC, abTmpBLPLFileCRC+12, 4 );		   /* �ż��� PL CRC ��� */

	return sRetVal;

}

/*******************************************************************************
*                                                                              *
*  FUNCTION ID:       WriteBLPLFileCRC                            			   *
*                                                                              *
*  DESCRIPTION:       blpl_crc.dat������ CRC���� �о� �Է°��� �ּҿ� �����Ѵ�.*
*                                                                              *
*  INPUT PARAMETERS:   byte* abBLCRC           - BL CRC                        *
*	                   byte* abMifPrepayPLCRC  - ������ PL CRC	               *
*	                   byte* abPostpayPLCRC    - �ĺ� PL CRC                   *
*	                   byte* abAICRC		   - �ż��� PL CRC  	           *							           *
*                                                                              *
*  RETURN/EXIT VALUE: SUCCESS - ���� ���� 	                                   *
*                     ERR_BLPL_PROC_OPEN_BLPL_CRC_FILE                         *
*                      - blpl_crc.dat open����                                 *
*                     ERR_BLPL_PROC_WRITE_BLPL_CRC_FILE						   *
*                      - input parameter�� ���� CRC type ����                *
*                                                                              *
*  Author:            Woolim                                                   *
*                                                                              *
*  DATE:              2005-09-10                                               *
*                                                                              *
*  REMARKS:   blpl_crc.dat���Ͽ� CRC���� ����ϸ� ������� SetUpTerm()�Լ����� *
*             ReadBLPLFileCRC�Լ��� ȣ���Ͽ� install.dat���Ͽ� ����Ͽ� ���Է� *
*             �����ϴ� ������ �Ǿ��ִ�.									       *		
*             abTmpBLPLCRC 16byte�� space�� �ش��ϴ� hex������ �ʱ�ȭ �ϴ�     *
*             ������ MakeCRC32FromFile�Լ� ������ ������ ��� �ش� 4byte��     *
*             space�� ���ܵξ� ���迡 ���۽� �����߻��� ��Ÿ���� �����̴�.     *
*                                                                              *
*******************************************************************************/
short WriteBLPLFileCRC(	int TypeOfCRCFile )
{
	short sRetVal =	0;
	int	fdFile = 0;	
	/*
	 * ���� BL/PL/AI������ CRC���� �����ϱ� ���� ����
	 */			
	dword dwBLCRC =	0;         /* BL CRC             */
	dword dwPrepayPLCRC	= 0;   /* ������ PL	CRC		 */
	dword dwPostpayPLCRC = 0;  /* ���ĺ� PL	CRC		 */
	dword dwAICRC =	0;         /* AI(�ż���/�ĺ� PL) CRC */

	/*
	 * CRC�� 16byte�� write�ϱ����� Buffer
	 */		
	byte abTmpBLPLCRC[16] = {0, };

	/*
	 * space(0x20)���� �ʱ�ȭ
	 */	
	memset( abTmpBLPLCRC, 0x20, sizeof(abTmpBLPLCRC));
	
	/*
	 * blpl_crc.dat���� ����
	 */		 
	fdFile = open( BLPL_CRC_FILE, O_CREAT | O_TRUNC | O_RDWR ,OPENMODE);
	
	if ( fdFile	< 0	)
	{
		DebugOut("[BLPLPROC]blpl_crc.dat open ���� \n");
		return ErrRet( ERR_BLPL_PROC_OPEN_BLPL_CRC_FILE );
	}

	/*
	 * �Լ��� �Ķ����(TypeOfCRCFile)�� �Ǻ��Ͽ� CRC����� ��û�� ���Ͽ� 
	 * Util.c�� MakeCRC32FromFile�Լ��� ȣ���Ͽ� CRC���� ����
	 */		
    switch( TypeOfCRCFile )
    {
    	case BL_FILE :  
			/*
			 * ������BL ������ CRC�� ���
	    	 */		
			dwBLCRC		   = MakeCRC32FromFile(	MASTER_BL_FILE );
			if ( dwBLCRC > 0 )
			{
				memcpy(	abTmpBLPLCRC, &dwBLCRC,	4 );
				DebugOut("[BLPLPROC]MASTER_BL_FILE %lu\n",dwBLCRC );
			}
			else
			{			
				DebugOut("[BLPLPROC]MASTER_BL_FILE ���Ͼ���\n");
			}
			/*
			 * BL CRC�� 4byte ���Ͽ� ��� 
			 */
			lseek( fdFile, 0, SEEK_SET);
			sRetVal	= write( fdFile, abTmpBLPLCRC ,	4);
			if ( sRetVal < 0 )
			{
				DebugOut("[BLPLPROC]BL CRC�� blpl_crc.dat write ����\n");
				close(fdFile);
				return ErrRet( ERR_BLPL_PROC_WRITE_BL_CRC_FILE	);
			}
			
			break;
			
		case PL_FILE : 
			/*
			 * ������ ������PL ������ CRC���� ���
	    	 */	
			dwPrepayPLCRC  = MakeCRC32FromFile(	MASTER_PREPAY_PL_FILE );
			/*
			 * ������ �ĺ�PL ������ CRC���� ���
	    	 */				
			dwPostpayPLCRC = MakeCRC32FromFile(	MASTER_POSTPAY_PL_FILE );
			if ( dwPrepayPLCRC > 0 )
			{
				 memcpy( abTmpBLPLCRC, &dwPrepayPLCRC, 4 );
				 DebugOut("[BLPLPROC]MASTER_PREPAY_PL_FILE %lu\n",dwPrepayPLCRC	);
			}
			else
			{
				DebugOut("[BLPLPROC]MASTER_PREPAY_PL_FILE ���Ͼ���\n");
			}
		
			if ( dwPostpayPLCRC	> 0	)
			{
				 memcpy( abTmpBLPLCRC + 4, &dwPostpayPLCRC,	4 );
				 DebugOut("[BLPLPROC]MASTER_POSTPAY_PL_FILE	%lu\n",dwPostpayPLCRC );
			}
			else
			{
				DebugOut("[BLPLPROC]MASTER_POSTPAY_PL_FILE ���Ͼ���\n");
			}
			/*
			 * ������/�ĺ� PL CRC��(8byte) ���Ͽ� ��� 
			 */
			lseek( fdFile, 4, SEEK_SET);			 
			sRetVal	= write( fdFile, abTmpBLPLCRC ,	8);
			if ( sRetVal < 0 )
			{
				DebugOut("[BLPLPROC]BL CRC�� blpl_crc.dat write ����\n");
				close(fdFile);
				return ErrRet( ERR_BLPL_PROC_WRITE_PL_CRC_FILE	);
			}
		
			break;
			
		case AI_FILE :
			/*
			 * ������ �ż���PL ������ CRC���� ���
	    	 */				
			dwAICRC		   = MakeCRC32FromFile(	MASTER_AI_FILE );
			if ( dwAICRC > 0 )
			{
				memcpy(	abTmpBLPLCRC, &dwAICRC, 4 );
				DebugOut("[BLPLPROC]MASTER_AI_FILE %lu\n",dwAICRC );
			}
			else
			{
				DebugOut("[BLPLPROC]MASTER_AI_FILE ���Ͼ���\n");
			}
			/*
			 * �ż���PL CRC��(4byte) ���Ͽ� ��� 
			 */
			lseek( fdFile, 12, SEEK_SET);
			sRetVal	= write( fdFile, abTmpBLPLCRC ,	4);
			if ( sRetVal < 0 )
			{
				DebugOut("[BLPLPROC]BL CRC�� blpl_crc.dat write ����\n");
				close(fdFile);
				return ErrRet( ERR_BLPL_PROC_WRITE_AI_CRC_FILE	);
			}			
			break;
			
		default : 
			close(fdFile);
			return ErrRet( ERR_BLPL_PROC_CRCTYPE );
	}


	close(fdFile);
	return ErrRet( sRetVal );
}




static word GetBLTypePrefix(byte *SixAscPrefix)
{
	pid_t nProcessId =0;
	int i,nTotPrefixCnt;
	word wBLTypePrefix;

	// �������� ��û�� ���� ������μ���,���������۱⿡�� ��û�� ����
	// ���������۱� ���μ�������  SearchBL�� ȣ���ϹǷ� ���μ��� id�� �Ǻ��Ͽ�
	// File���� ���� compressed prefix code�� �д´�
	// printf( "getpid %d\n", getpid() );
	// printf("gpstSharedInfo->nCommPrinterProcessID %d",gpstSharedInfo->nCommPrinterProcessID);
	nProcessId = getpid();
	if (( nProcessId == gpstSharedInfo->nCommProcProcessID) ||
	    ( nProcessId == gpstSharedInfo->nCommDriverProcessID))
	{
		wBLTypePrefix = GetBLTypePrefixInFile( SixAscPrefix);
		DebugOut( " ������û wBLTypePrefix           : %u\n",
	    wBLTypePrefix );
	}
	else // �������� ��û�� ���� �� �ε�� ����ü���� Compressed prefix code�� �д´�
	{
		nTotPrefixCnt = gstPostpayIssuerInfoHeader.wRecordCnt;
		wBLTypePrefix = -1;
//		printf("������û gstPostpayIssuerInfoHeader.wRecordCnt; %d ",gstPostpayIssuerInfoHeader.wRecordCnt);
		for (i = 0; i < nTotPrefixCnt; i++)
		{
			//PrintlnASC  ( "��Prefix  No.                           : ",
			//   gpstPostpayIssuerInfo[i].abPrefixNo, 6 );
			//PrintlnASC  ( "��SixAscPrefix                         : ",
			//              SixAscPrefix, 6 );
			if (memcmp(gpstPostpayIssuerInfo[i].abPrefixNo, SixAscPrefix, 6) == 0)
			{
			  // PrintlnASC  ( "Prefix  No.                           : ",
			  //            gpstPostpayIssuerInfo[i].abPrefixNo, 6 );
			  // 	printf( "compressed code                   : %u\n",
	         //     gpstPostpayIssuerInfo[i].wCompCode );
				wBLTypePrefix = gpstPostpayIssuerInfo[i].wCompCode;
			//		printf( "wBLTypePrefix           : %u\n",
	         //     wBLTypePrefix );
				break;
			}
		}
	}
	DebugOut("prefix matched[%d]\n", wBLTypePrefix);
	return wBLTypePrefix;
}


static word GetBLTypePrefixInFile(byte *prefix)
{
	byte record[10], bcd_prefix[3+1];
	int	fd, min, max, comp;
	short cnt, int_prefix;
	dword offset;

	if	((fd = open( POSTPAY_ISSUER_INFO_FILE, O_RDWR, OPENMODE)) < 0) {
/**/		printf("File not found.... \n");
 		return (-1);		// File not found
	}

 	if	((read(fd, record, 9)) != 9) {
		close(fd);
/**/		printf ("Record Read Error 1... \n");
 		return (-1);		// record read_error
	}
	ASC2BCD( prefix, bcd_prefix, 6);
	//ascii_to_bcd(bcd_prefix, prefix, 6);

//	printf ("bcd_prefix : [%02x][%02x][%02x] \n", bcd_prefix[0], bcd_prefix[1], bcd_prefix[2]);
	memcpy(&cnt, &record[7], sizeof(cnt));

	min = 1;
	max = cnt;

//    printf ("min = %d   max = %d \n", min, max);

	while (min <= max)
	{
		comp = (min + max) / 2;
   		if	((max < 0) || (min < 0))
   			break;
		offset = ((comp-1) * 5) + 9;
		lseek (fd, offset, 0);
 		if	((read (fd, record, 5)) != 5)
 		{
			close(fd);
/**/		printf("Record Read Error 2... \n");
			return (-1);
		}

//	printf ("comp_prefix : [%02x][%02x][%02x] \n", record[0], record[1], record[2]);

     	if	(memcmp(record, bcd_prefix, 3) == 0)
     	{
			memcpy(&int_prefix, &record[3], sizeof(int_prefix));
			close(fd);
//	printf ("Search Prefix code = %d \n", int_prefix);
		return (int_prefix);
		}
     	if	(memcmp(record, bcd_prefix, 3) < 0)
     	{
		min = comp + 1;
		continue;
		}
		if	(memcpy(record, bcd_prefix, 3) > 0)
		{
			max = comp - 1;
			continue;
		}
	}
/**/	printf("Prefix not found.... \n");

	close (fd);
	return (-1);
}
