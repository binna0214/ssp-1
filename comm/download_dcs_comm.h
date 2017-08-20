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
*  PROGRAM ID :       download_dcs_comm.h	                                   *
*                                                                              *
*  DESCRIPTION:       ����ý������κ��� ���� �ٿ�ε带 ���� �Լ����� �����Ѵ�.  *
*                                                                              *
*  ENTRY POINT:     short DownVehicleParmFile( void );						   *
*					short DownFromDCS( void );								   *
*                                                                              *
*  INPUT FILES:     c_ve_inf.dat                                               *
*                                                                              *
*  OUTPUT FILES:    achDCSCommFile�� ��ϵ� ���ϵ�                      		   *
*					c_ve_inf.dat			- ��������						   *
*					downloadinfo.dat		- �ٿ�ε�� ���� ��� ����		   *
*					downfilelink.dat		- �̾�޴� ������ ����			   *
*					connSucc.dat			- ������� �����ð�				   *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME                      DESCRIPTION              *
* ---------- ---------------------------- ------------------------------------ *
* 2006/03/27 F/W Dev Team Mi Hyun Noh  Initial Release                         *
*                                                                              *
*******************************************************************************/
#ifndef _DOWNLOAD_DCS_COMM_H_
#define _DOWNLOAD_DCS_COMM_H_

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
short DownVehicleParmFile( void );
short DownFromDCS( void );
short GetRelayRecvFileSize( char chFileNo, long *plFileSize );

#endif