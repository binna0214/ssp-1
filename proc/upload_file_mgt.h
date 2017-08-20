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
*  PROGRAM ID :       upload_file_mgt.h                                       *
*                                                                              *
*  DESCRIPTION:       �� ���α׷��� �ܸ��� ��ġ������ ���������� ���� ����� 	   *
*						�����Ѵ�.       										   *
*                                                                              *
*  ENTRY POINT:       CreateInstallInfoFile                                    *
*                     SetUploadVerInfo                                         *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       None			                                           *
*                                                                              *
*  OUTPUT FILES:      install.dat                                              *
*                     version.trn                                              *
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
#ifndef _UPLOAD_FILE_MGT_H_
#define _UPLOAD_FILE_MGT_H_

/*******************************************************************************
*  Inclusion of System Header Files                                            *
*******************************************************************************/
#include "file_mgt.h"

/*******************************************************************************
*  structure of UPLOAD_VER_INFO - version.trn                                  *
*******************************************************************************/
typedef struct				       
{
	byte	abMainTermApplVerName[5];		// �����ܸ��� F/W ����
	byte	abVerInfoCreateDtime[7];		// �������������Ͻ�
	byte	abSubTerm1ApplVerName[5];		// �����ܸ���1 F/W ����
	byte	abSubTerm2ApplVerName[5];		// �����ܸ���2 F/W ����
	byte	abSubTerm3ApplVerName[5];		// �����ܸ���3 F/W ����
	byte	abDriverOperatorApplVerName[5];	// �����������۱� F/W ����
	byte	abMainTermKernelVerName[2];		// �����ܸ��� Ŀ�� ����
	byte	abTranspBizrID[9];				// �������� ID
	byte	abBusBizOfficeID[2];			// ������ ID
	byte	abRouteID[4];					// �뼱 ID
	byte	abVehicleID[9];					// ���� ID
	byte	abMainTermID[9];				// �����ܸ��� ID
	byte	bSubTermCnt;					// �����ܸ��� ����
	byte	abSubTerm1ID[9];				// �����ܸ���1 ID
	byte	abSubTerm2ID[9];				// �����ܸ���2 ID
	byte	abSubTerm3ID[9];				// �����ܸ���3 ID
	byte	abDriverOperatorID[9];			// ���������۱� ID
	byte	abMainTermSAMID[16];			// �����ܸ��� PSAM ID
	byte	abSubTerm1SAMID[16];			// �����ܸ���1 PSAM ID
	byte	abSubTerm2SAMID[16];			// �����ܸ���2 PSAM ID
	byte	abSubTerm3SAMID[16];			// �����ܸ���3 PSAM ID
	byte	abDCSIPAddr[12];				// ���輭�� IP
	byte	abTranspMethodCode[3];			// ��������ڵ�            
	byte	abMasterBLSize[5];				// ����BL ũ��
	byte	abMasterPrepayPLSize[5];		// �����Ұ���PL ũ��
	byte	abMasterPostpayPLSize[5];		// �ĺҰ���PL ũ��
	byte	abMasterAISize[5];				// �ż��Ұ���PL ũ��
	byte	abMasterBLVer[7];				// ����BL �Ͻù���
	byte	abUpdateBLVer[7];				// ����BL �Ͻù���
	byte	abHotBLVer[7];					// ��BL �Ͻù���
	byte	abMasterPrepayPLVer[7];			// �����Ұ���PL �Ͻù���
	byte    abMasterPostpayPLVer[7];          
		// fixed  p/l version (postpay)           
	byte    abUpdatePLVer[7];                 
		// update  pl version                   
	byte    abHotPLVer[7];                    
		// hot p/l version                   
	byte    abMasterAIVer[7];                 
		// fixed  ai version  (new transportation )         
	byte    abUpdateAIVer[7];                 
		// update  ai version (new transportation )          
	byte    abHotAIVer[7];                    
		// hot ai version  i(new transportation )         
	byte    abVehicleParmVer[7];              
		// run vehicle parameter  version         
	byte    abRouteParmVer[7];                
		// bus terminalroute  parameter  version  
	byte    abBasicFareInfoVer[7];            
		// basic fareinformation  version              
	byte    abBusStationInfoVer[7];           
		// bus station interval info  version          
	byte    abPrepayIssuerInfoVer[7];         
		// prepay issuer info  version          
	byte    abPostpayIssuerInfoVer[7];        
		// postpay issuer  info  version          
	byte    abDisExtraInfoVer[7];             
		// discount or extra info  version              
	byte    abHolidayInfoVer[7];              
		// holiday info  version                  
	byte    abXferApplyRegulationVer[7];      
		// transfer apply  regulation version              
	byte    abXferApplyVer[7];      	  
		// transfer apply  version                  
	byte    abMainTermApplVer[7];             
		// enter  S/W version                   
	byte    abSubTerm1ApplVer[7];             
		// sub  S/W version   1               
	byte    abSubTerm2ApplVer [7];            
		// sub  S/W version   2               
	byte    abSubTerm3ApplVer [7];            
		// sub  S/W version   3               
	byte    abDriverOperatorApplVer[7];       
		// driver operator  S/W version            
	byte    abEpurseIssuerRegistInfoVer[7];   
		// purse issuer registration info  version    
	byte    abPSAMKeysetVer[7];               
		// Payment  sam ketset version           
	byte    abAutoChargeKeysetVer[7];         
		// auto- charge  sam ketset          
	byte    abAutoChargeParmVer[7];           
		// auto- charge  parameter version        
	byte    abPostpayIssuerChkVer[7];         
		// postpay issuer  check version          
	byte    abCrLf[2];                        
		// special character                      
}__attribute__((packed))  UPLOAD_VER_INFO;   

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
short CreateInstallInfoFile( void );
short SetUploadVerInfo( void );

#endif