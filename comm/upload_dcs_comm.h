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
*  PROGRAM ID :       upload_dcs_comm.h	                                       *
*                                                                              *
*  DESCRIPTION:       ����ý������κ��� ���� ���ε带 ���� �Լ����� �����Ѵ�.    *
*                                                                              *
*  ENTRY POINT:     short DownVehicleParmFile( void );						   *
*					short DownFromDCS( void );								   *
*                                                                              *
*  INPUT FILES:     reconcileinfo.dat                                          *
*					simxinfo.dat											   *
*					reconcileinfo.dat�� ��ϵ� ����							   *
*                                                                              *
*  OUTPUT FILES:    reconcileinfo.dat                                          *
*					reconcileinfo.dat�� ��ϵ� ����							   *
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
#ifndef _UPLOAD_DCS_COMM_H_
#define _UPLOAD_DCS_COMM_H_

/*******************************************************************************
*  Declaration of Module Functions                                             *
*******************************************************************************/
short Upload2DCS( void );

#endif