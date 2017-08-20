/*****************************************************************************
* �� ����ī�� ������Ʈ
*
* Copyright (c) 2003-2005 by LG CNS, Inc.
*
*     All rights reserved.
*
*****************************************************************************
*  ���뼳��   : ������ �ν� ���α׷� �Դϴ�.
*  INPUT      : c_st_inf.dat
*  OUTPUT     : simxlog.dat, simxinfo.dat
*  $Id: gps.c,v 1.48 2006/07/11 00:57:53 jaeekim Exp $
*  $Log: gps.c,v $
*  Revision 1.48  2006/07/11 00:57:53  jaeekim
*  ������� ������ 1M��,
*  �ܸ��� ���� ���� ��쿡 �̾�������
*
*  Revision 1.47  2006/06/16 10:41:39  jeonbh
*  memcpy() ���� size ����
*
*  Revision 1.46  2006/06/15 01:05:49  jeonbh
*  sprintf() ���� ���� ����
*
*  Revision 1.45  2006/06/12 00:31:41  jeonbh
*  ����ð� ��� ���� ����
*
*  Revision 1.44  2006/05/19 02:47:41  jaeekim
*  *** empty log message ***
*
*  Revision 1.43  2006/05/16 09:58:14  jaeekim
*  *** empty log message ***
*
*  Revision 1.42  2006/05/16 07:45:35  jaeekim
*  *** empty log message ***
*
*  Revision 1.41  2006/02/28 05:25:19  jaeekim
*  *** empty log message ***
*
*  Revision 1.40  2006/02/24 01:55:20  jeonbh
*  �����Ͻ� warning �� �߻���Ű�� �ڵ� ����
*
*  Revision 1.39  2006/01/24 09:49:39  jaeekim
*  *** empty log message ***
*
*  Revision 1.38  2006/01/24 09:40:15  jaeekim
*  *** empty log message ***
*
*  Revision 1.37  2005/11/24 07:31:59  jaeekim
*  *** empty log message ***
*
*  Revision 1.36  2005/11/24 07:29:52  jaeekim
*  *** empty log message ***
*
*  Revision 1.35  2005/11/23 07:43:19  jaeekim
*  *** empty log message ***
*
*  Revision 1.34  2005/11/19 11:26:48  jaeekim
*  *** empty log message ***
*
*  Revision 1.16  2005/09/22 01:13:08  jaeekim
*  *** empty log message ***
*
*  Revision 1.15  2005/09/21 01:09:13  mhson
*  �����Ͽ���
*
*  Revision 1.14  2005/09/15 08:37:05  jaeekim
*  *** empty log message ***
*
*  Revision 1.9  2005/09/09 06:32:31  gykim
*  *** empty log message ***
*
*  Revision 1.8  2005/09/09 06:02:10  gykim
*  *** empty log message ***
*
*  Revision 1.7  2005/09/08 04:58:34  jeonbh
*  �űԹ����ܸ��� ���α׷����� �����ϵǵ��� ����
*
*  Revision 1.6  2005/09/08 04:22:42  jaeekim
*  *** empty log message ***
*
*****************************************************************************/
#define VERSION		"GPS 3.0"

#include "gps_define.h"

#ifdef	_GPS_27_
	#include "../include/bus100.h"
	#include "../include/simx.h"
#else
	#include <stdio.h>
	#include <stdlib.h>
	#include "../../common/type.h"
	#include "../system/bus_type.h"
	#include "../proc/reconcile_file_mgt.h"
	#include "gps.h"	
	#include "gps_env.h"
	#include "gps_search.h"
	#include "gps_util.h"

#endif

GPS_DATA GPS;						// RMC ����Ÿ ���� ����ü
GPS_INFO_STATION st_GPS_FINAL_INFO; // ���� �α������� ������ �ٿ� ���� ����ü
									// (�νķ�, ����Ÿ�, ����Ƚ��)
									
GPS_INFO_STATION stGPS_INFO_STATION[MAXSTATIONCNT];

int g_nTotalStaCnt = 0;		// ������ �� ��
int g_nContinueFlag = 0;		// ���� Ų ��� ���࿬���ΰ�(1), �ƴѰ�(0)?

int g_nNextStaOrder = 0;
int g_nPrevStaOrder = 0;
int g_nCurStaOrder = 0;

int g_nPassMinDis = 0;				// ����������� �ּҰŸ�
char g_szPassMinLongitude[10];	
char g_szPassMinLatitude[9];	

char g_szVehcID[10];	// ���� id
char g_szBusRouteID[9];	// �뼱 id
	
char g_szPassMinTime[15];			
char g_szPassSatelliteCnt[3];	

char szGpsLogStartTime[15];	
char szGpsLogEndTime[15];	
char g_szRunDepartTime[15];	// ������۽ð�

char g_szStaFindTime[15];	// ���� ������ �νĽð�

int g_gps_fd = -1;				// gps ��Ʈ �ڵ�
int g_bInc = FALSE;
int g_nFirstRun = 0;	// ��������� ������ �˻��� ÷����?

int gpsloopcheck = 4;

//GPS 2.7 ����. ��⿡�� ������ �ð� (�ܸ��� �ð� �ƴ�.)
char g_szGpsCurrTime[15];

int g_nGpsExceptCnt = 0;	// ������ ����� ������ �� ��

byte g_gpsStatusFlag = 0;	// ���� gps ���Ż��� �÷��� ����
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// ���� ����Ÿ� ������ ���� ������..
//static DRIVEDIST stDriveDist;

int	nMoveDis = 0;	// ������ۺ��� ��������ñ��� �̵��� �Ÿ�

byte old_station_id[8];

////////////////////////////
// SIMX Format...
BUSINFO			BusInfo;				// ������ �ν� ���� ���� ����ü(BusStationDetect�Լ��� ���ϰ�)
COORDINATES		coordinates;			// ��ǥ��ȯ ����ü
RMC_DATA		s_stRMC;				// RMC ������ ���� ����ü
GGA_DATA		g_stGGA;				// GGA ������ ���� ����ü
////////////////////////////

#ifdef	_SIMMODE_
static FILE*	s_SimFile = NULL;	// �ùķ��̼ǿ� RAW ����
#endif

//static unsigned int FirstRun =0;
//static unsigned int FirstWriteFlag =0;

GPS_INFO_STATION g_stGPS_INFO_STATION[MAXSTATIONCNT];


STA_INFO_LOAD	STA_IF_LOAD[256];       // ������ ���� �ε�

// ������ �������� �Ͽ� ���� ����� ������� ������ ����Ÿ ���� ������ 
// �ִ� �迭.
// ���� ��ġ ��� ���� �ݰ泻 ������ �������� �˻��ϱ� ����.
SORT_X	g_stSortX[MAXSTATIONCNT];
SORT_Y	g_stSortY[MAXSTATIONCNT];

double g_dbRoundX;	// ������ �˻� �ݰ� X'
double g_dbRoundY;	// ������ �˻� �ݰ� Y'


static	struct timeval stTime1, stTime2;
static 	int bTimeCheck = 0;

// ����, 2.7 ��� �����ϱ� ���� �߰� ����
COMM_STA_INFO g_stStaInfo[MAXSTATIONCNT];

// ���� ������ DB ���� ���� ����ü
BUS_STATION	g_stBusStation[MAXSTATIONCNT];

double g_dbMaxSpeed = 0;	// ������� �� ������ �ִ� �ӵ���
//===========================================================================//
// �Լ� ���� �κ� ����
void ThreadStartCheck(void);

int BusStationInfoLoading(int current_new);
unsigned int ReadyStaInfo(int nStation_cnt);
int ContinueDriveCheck(void);
int ReadyWriteLogFile(void);
void SyncDeparttimeWithMainLogic(char* szStartLogTime);
int GpsDataProcessing(void);

int GpsRecv(int fd, unsigned char* buffer, int timeout);

int ParseGGA(GGA_DATA* gga, unsigned char *Data, int DataNum);
int CheckSum(unsigned char *Data);
void clean_up(void *arg);
void GpsErrorProcess(int param);
void WriteTrace(RMC_DATA* rmc);
void* gpsMain(void *arg);


void GpsExitProc(void);
//void GpsDataProc(void);
short GpsPortProc(void);
void BusStationInfoSet(void);
static void GpsDataProc(void);

// �Լ� ���� �κ� ��
//=============================================================================

void* gpsMain(void *arg)
{
	
	GlobalParameterInit();	// static ���� �ʱ�ȭ

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_cleanup_push(clean_up, (void *)NULL);

	ThreadStartCheck();		// ������ ���� �⵿�ߴ��� �˻�
	
	BusStationInfoSet();	// ���� ������ �⺻���� ����

	if (GpsPortProc() < 0)	// GPS ��Ʈ ����
	{
		GpsExitProc();// Gps ������ ���� ���μ���
		return (void*)NULL;	// gps ��Ʈ ���� ���� �߻��Ͽ� ������ ����
	}
	
	ReadyWriteLogFile();	// �α� ��� �غ�	

	strcpy(g_szStaFindTime, g_szRunDepartTime);	// ó�� ������ �ν��� �ð��� ������۽ð�


	/*
	 * ������� ���� ���� �˻��� ������� ���Ͽ��� �о�� ���� ������ �ν� ������
	 * �� ���Ͽ� ������ ���� �������� ���̰� �߻��ϸ�
	 * [������ ��ġ�� Ȯ���� �ֽñ� �ٶ��ϴ�] ��� �������
	 */
	CheckingCurrStation();
	 
	/*
	 *	GPS ����Ÿ�� ���Ź޾� ������ �˻� �� �α� ���
	*/
	while (1)	
	{
		GpsDataProc();	// GPS ����Ÿ ���μ���(������ �˻� ����)
	}
	
	GpsExitProc();	// Gps ������ ���� ���μ���

	pthread_cleanup_pop(0);	
	
}

/*******************************************************************************
 *		
 *	function: ReadyStaInfo
 *	Param: g_stBusStation :���� ������ ������ �����ϴ� ����ü�迭 ������ 
 * 	       nSTA_IF_LOAD : ���� ������ ������ �����ϴ� �ƽ�Ű ����ü 
 *             coordinates : ��ǥ���� �����ϴ� ����ü 
 * 	Return: 
 *		0 : no error
 *		1 : g_stBusStation is null pointer
 *		2 : nStation_cnt is overflow data number, check the nStation_cnt 
 *													or MAXSTATIONCNT
 *		3 : nStation_cnt is overflow data number, check the nStation_cnt 
 *		4 : g_stBusStation[i].StationOrder is wrong value
 *		5 : g_stBusStation[i].DBLen is zero
 *		6 : nStation_Cnt is zero
 *		7 : g_stBusStation[i].LonDeg is wrong value
 *		8 : g_stBusStation[i].LatDeg  is wrong value
 *		9 : g_stBusStation[i].Angle is wrong value
 *	discription : ���� ������ ó���� ���� ��ó���� �Ѵ�. 
 *	note: �� �Լ��� ȣ���ϱ� ���� g_stBusStation����ü�� �ݵ�� ���� DB 
 * 									�����Ͱ� ���ڵ��Ȱ��� ����Ǿ� �־�� �Ѵ�.
 *	        g_stBusStation����ü ����� 	
 * 		unsigned int StationID; 	//ID
 *		unsigned int StationOrder; 	//Station order
 *		unsigned int DBLen;  		//Station number
 *		double LonDeg;  		//Station longitude Degree, DD.ddddd
 * 		double LatDeg;  		//Station latitude  Degree, DD.ddddd
 *		double Angle;  			//Station Angle
 *	      ������ ���� ä���� �־�� �Ѵ�. 	
 *            �� �Լ��� �ݵ�� BusStationDetect()�� ȣ���ϱ� ���� �ݵ�� ���� 
 * 																�Ǿ����� �Ѵ�. 
 *	      �Ϲ������� GPS main �Լ����� �ʱ�ȭ ��ƾ���� ������ �ָ� �ȴ�. 
 ******************************************************************************/
unsigned int ReadyStaInfo(int nStation_cnt)
{
	int i=0;
	double tempXYZ[3];
	double tempLLH[3];
	double pos_x, pos_y;
	
	memset(&coordinates,0x00,sizeof(COORDINATES));
	///////////////////////////////////////////////////////////////////////////////
	
	for (i = 0; i < nStation_cnt; i++)	{
		g_stBusStation[i].StationID = atointeger(g_stStaInfo[i].abStationID, 7);
		g_stBusStation[i].StationOrder = atointeger(g_stStaInfo[i].bStationOrder, 3);
	  	g_stBusStation[i].DBLen = nStation_cnt;  //Station number

		pos_x = g_stStaInfo[i].dLongitude;
		pos_y = g_stStaInfo[i].dLatitude;
		g_stBusStation[i].LatDeg = (int)(pos_y/100) + ((pos_y - (int)(pos_y/100)*100.0)/60);
		g_stBusStation[i].LonDeg = (int)(pos_x/100) + ((pos_x - (int)(pos_x/100)*100.0)/60);
		
	 	g_stBusStation[i].Angle  = g_stStaInfo[i].wAngle;  //Station Angle
	 	g_stBusStation[i].AccumDist = g_stStaInfo[i].dwDist; //Station Accumulated Distance
	 	g_stBusStation[i].DiffDist = 0;  //70% Distance between current station and previous station 
	 	g_stBusStation[i].DiffDist2 = 0; // Distance between current station and previous station 
	 	g_stBusStation[i].ENU[0] = 0;  //ENU value
	 	g_stBusStation[i].ENU[1] = 0;  //ENU value
	 	g_stBusStation[i].ENU[2] = 0;  //ENU value
	}
	
	
	/*********************************Excetion Process ****************************************/
	
	//null pointer test
	if(g_stBusStation == 0x00)
	{
		return 1;
	}
	
	//nStation_cnt value test
	if(nStation_cnt > MAXSTATIONCNT)
	{
		return 2;
	}
	for(i =0; i < nStation_cnt; i++)
	{
		if(nStation_cnt == 0)
		{
			return 5;
		}
		if(g_stBusStation[i].StationOrder > nStation_cnt)
		{
			return 6;
		}
		if(g_stBusStation[i].LonDeg > 180.0 || g_stBusStation[i].LonDeg < 0)
		{
			return 7;
		}
		if(g_stBusStation[i].LatDeg > 90.0 || g_stBusStation[i].LatDeg < 0)
		{
			return 8;
		}
		if(g_stBusStation[i].Angle > 360.0 || g_stBusStation[i].Angle < 0)
		{
			return 9;
		}
	}
	
	/************************Exception Precess End*********************************************/
	

	//1�� DB�� ���� ���� ��ǥ LLH�� ���� 
	coordinates.OrgLLH[0] = g_stBusStation[0].LatDeg*D2R;
	coordinates.OrgLLH[1] = g_stBusStation[0].LonDeg*D2R;
	coordinates.OrgLLH[2] = 30;  //heigth 30m fix

	LatLonHgtToXYZ_D(coordinates.OrgLLH,coordinates.OrgXYZ);


    //�ʱ� DB�� ENU���� ��� 0�̹Ƿ� ������� �ʰ� �ٷ� 0���� 
	g_stBusStation[0].ENU[0] =0;
	g_stBusStation[0].ENU[1] =0;
	g_stBusStation[0].ENU[2] =0;


	//1�� �����忡 ���ؼ� ��� DB�� ���� ENU ���� ���Ѵ�. 
	for(i = 1 ; i < nStation_cnt ; i++)
	{
		tempLLH[0] = g_stBusStation[i].LatDeg*D2R;
		tempLLH[1] = g_stBusStation[i].LonDeg*D2R;
		tempLLH[2] = 30;  //heigth 30m fix 
	  	LatLonHgtToXYZ_D(tempLLH,tempXYZ);
		XyztoENU_D(tempXYZ,coordinates.OrgXYZ,coordinates.OrgLLH,g_stBusStation[i].ENU);
#ifdef	_DEBUGGPS_		
		printf("g_stBusStation[%d].ENU[0] = %f\n", i, g_stBusStation[i].ENU[0]);
		printf("g_stBusStation[%d].ENU[1] = %f\n", i, g_stBusStation[i].ENU[1]);
#endif
	}

	//���� �Ÿ��� ����Ͽ� ���� �Ѵ�. 
	g_stBusStation[0].DiffDist = DIFF_DIST_P_FIRST;
	g_stBusStation[0].DiffDist2 = 0;	
	for(i =1; i < nStation_cnt; i++)
	{
		g_stBusStation[i].DiffDist = (g_stBusStation[i].AccumDist - g_stBusStation[i-1].AccumDist) * DIFF_DIST_P;
		g_stBusStation[i].DiffDist2 = (g_stBusStation[i].AccumDist - g_stBusStation[i-1].AccumDist);	
#ifdef	_DEBUGGPS_		
		printf("g_stBusStation[%d].DiffDist = %f\n", i, g_stBusStation[i].DiffDist);
		printf("g_stBusStation[%d].DiffDist2 = %f\n", i, g_stBusStation[i].DiffDist2);
#endif		
	}	


	// ������ ��ǥ ������ ���� �迭�� ���� ������. 
    for (i=0; i<g_nTotalStaCnt; i++)	{
    	g_stSortX[i].dbX = g_stBusStation[i].ENU[0];
    	g_stSortX[i].nOrder = i+1;
    	g_stSortY[i].dbY = g_stBusStation[i].ENU[1];
    	g_stSortY[i].nOrder = i+1;
    }
	
    // ������ ��ǥ�� X, Y �������� ���� �����Ͽ� �迭�� ����.
	BubbleSortStaInfo();
	
#ifdef	_DEBUGGPS_		
		printf("g_stSortX[g_nTotalStaCnt-1].dbX = %f\n", g_stSortX[g_nTotalStaCnt-1].dbX);
		printf("g_stSortY[g_nTotalStaCnt-1].dbY = %f\n", g_stSortY[g_nTotalStaCnt-1].dbY);
#endif	 
	// ������ �˻� �ݰ� ���ϱ�.
	g_dbRoundX = g_stSortX[g_nTotalStaCnt-1].dbX / g_nTotalStaCnt * SEARCH_ROUND_VALUE;
	g_dbRoundY = g_stSortY[g_nTotalStaCnt-1].dbY / g_nTotalStaCnt * SEARCH_ROUND_VALUE;

	g_dbRoundX = abs(g_dbRoundX);
	g_dbRoundY = abs(g_dbRoundY);
	
#ifdef	_DEBUGGPS_		
		printf("g_dbRoundX = %f\n", g_dbRoundX);
		printf("g_dbRoundY = %f\n", g_dbRoundY);
#endif
	return 0;

}

void ThreadStartCheck(void)
{
		
	// pid �� ���������� �� ����
#ifdef _GPS_27_
	gpspid = getpid();

	// ������ ������ Main Process���� ���Ƿ� �ߴٸ�, 
	// �������� gpsstatus�� 44�� ���õǾ� �ִ�.
	// ����, �� gpsstatus ���� 44 �̸�, ������ ����α��� 0��° 
	// pass_yn �� ���� '44'�� �����.
	if (gpsstatus == 44)	{
		gpsstatus = 0;	// �ٽ� 0 ���� ����� �ش�.
		// 0��° ������ ��� �α׿� '44'�� ����Ѵ�.
		memcpy(g_stGPS_INFO_STATION[0].szPass, 	  "44", 2);
	}
#else
	gpstSharedInfo->nCommGPSProcessID= getpid();

	// ������ ������ Main Process���� ���Ƿ� �ߴٸ�, 
	// �������� gpsstatus�� 44�� ���õǾ� �ִ�.
	// ����, �� gpsstatus ���� 44 �̸�, ������ ����α��� 0��° 
	// pass_yn �� ���� '44'�� �����.
	if (gnGPSStatus == 44)	{
		gnGPSStatus = 0;	// �ٽ� 0 ���� ����� �ش�.
		// 0��° ������ ��� �α׿� '44'�� ����Ѵ�.
		memcpy(g_stGPS_INFO_STATION[0].szPass, 	  "44", 2);
	}
#endif
	
}

/******************************************************************************
*                                                                             *
*    �Լ��� : ContinueDriveCheck()                                         	  *
*                                                                             *
*      ���� : [���� �ν� ������ ���� �ε�]									  *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.08.01                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                 *
*             ------  ----------  --------        --------------------------- *
*			  ����.	  														  *
*    ���ϰ� : ����	- ���� �ν� ������ ����	                                  *
*			  ����	- -1		   							              	  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
int ContinueDriveCheck()
{
	int nTmpOrder = -1;
	int nContinueFlag = 0;
	FILE*	file = NULL;

//	g_bInc = FALSE;	// �⺻�� 1 ���� ���� ���� ����
	
	file = fopen(PREV_PASS_FILE, "rb");
	
	if (file != NULL)	{
		fread(&nTmpOrder, sizeof(int), 1, file);
		fread(&nContinueFlag, sizeof(int), 1, file);
		fread(g_szRunDepartTime, 14, 1, file);
		g_szRunDepartTime[14] = 0x00;
		fclose(file);
	}
	
	g_nContinueFlag = nContinueFlag;
	
	if ( nContinueFlag == 1)	{
		printf("\n�߰��� �̾ ��� �ϴ� ���\n");
		GpsDataLogRead();
	}
		
	if ( nTmpOrder < 1 || nTmpOrder > g_nTotalStaCnt)	{

		nTmpOrder = 1;
	}
	
	if(nTmpOrder == g_nTotalStaCnt)//2005-02-18 8:02����
	{
		nTmpOrder = 1;
	}
	
	return nTmpOrder;
}

/*******************************************************************************
*                                                                              *
*    �Լ��� : ReadyWriteLogFile()                                         	   *
*                                                                              *
*      ���� : [�� �����ϴ� �α� ���� ���� Ȯ�� �� GPS ����ü �ʱ�ȭ]           *
*    �ۼ��� : �� �� ��                                                         *
*                                                                              *
*    �ۼ��� : 2005.08.01                                                       *
*                                                                              *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                  *
*             ------  ----------  --------        ---------------------------  *
*			  ����.	  														   *
*    ���ϰ� : ����	- 0           INT             �α� ���� ����               *
*			  ����	- -1		  INT			  SIMXINFO2.TMP ���� ������    *
*			          -2          INT			  SIMXINFO.DAT ���� ������     *
*  ���ǻ��� : �ݵ�� ������� �ð��� ������ �Ŀ� ����Ǿ�� �Ѵ�.              *
*                                                                              *
*******************************************************************************/
int ReadyWriteLogFile()
{
	FILE *gpsFile2 = NULL;	
	int n44 = 0;
	int filesize = 0;
	int i=0;

	// 2005.01.17 ������ ����
	/////////////////////////////////////////////////////////////////////////////////// 
	// 05.01.13
	// TMP File ����� üũ�ؼ� 118 Byte �� �����谡 ���� ������ ������ �����.
	filesize = getFileSize(GPS_INFO_FILE2);
	// tmp���Ͽ� ������ �߻��Ͽ��ٸ�, tmp ������ �����ϰ�, dat ���Ͽ� '44'�α׸� ����Ѵ�.			
	if(filesize % SIMXINFO_SIZE != 0 || filesize >LOG_MAX_SIZE)
	{
		system("rm simxinfo2.tmp");
		usleep(100);
		// tmp�� ����� dat ���Ͽ� 44�� ����Ѵ�.
		n44 = -1;
	}

	///////////////////////////////////////////////////////////
	// 05.01.17				// ������ �߰�
	// dat ���ϰ˻�
	filesize = 0;
	filesize = getFileSize(GPS_INFO_FILE1);
	if (filesize % SIMXINFO_SIZE != 0 || filesize > LOG_MAX_SIZE)	{
		printf("\n���� �������� ���� �߻�.. ���� ������...simxinfo.dat\n");
		system ("rm simxinfo.dat");
		usleep(100);
//		system ("rm simxinfo2.tmp");

		n44 = -2;
	}
	///////////////////////////////////////////////////////////
	
	//GPS ����ü �ʱ�ȭ, �ܸ��� ������۽� �ѹ��� �ʱ�ȭ
	memset(&GPS,0x00,sizeof(GPS_DATA));


	// ����α� ��� ���� ���ۺκ��� ���� ����Ѵ�.
	if (g_nContinueFlag == 0)	{
		// �����뼱 ID
		memcpy(g_stGPS_INFO_STATION[0].szLineNum, g_szBusRouteID,
			sizeof(g_stGPS_INFO_STATION[0].szLineNum));
		// �������� ID
		memcpy(g_stGPS_INFO_STATION[0].szBusID, g_szVehcID,
			sizeof(g_stGPS_INFO_STATION[0].szBusID));
		memcpy(g_stGPS_INFO_STATION[0].szStartTime, g_szRunDepartTime,
			sizeof(g_stGPS_INFO_STATION[0].szStartTime));
		memset(g_stGPS_INFO_STATION[0].szOrder, 	0x30, 3);	// ������ ����
		memset(g_stGPS_INFO_STATION[0].szID, 		0x30, 7);	// ������ id
		memset(g_stGPS_INFO_STATION[0].szLongitude, 0x20, 10);				// ����
		memset(g_stGPS_INFO_STATION[0].szLatitude,  0x20, 9);					// �浵
		memset(g_stGPS_INFO_STATION[0].szPassDis,   0x20, 3);					// ����ּҰŸ�
		memset(g_stGPS_INFO_STATION[0].szPass, 	  0x20, 2);					// ������� 
		// ����ð�
		memcpy(g_stGPS_INFO_STATION[0].szPassTime, g_szRunDepartTime,
			sizeof(g_stGPS_INFO_STATION[0].szPassTime));
		memset(g_stGPS_INFO_STATION[0].szSateCnt,   0x20, 2);					// ��������
		memset(g_stGPS_INFO_STATION[0].szIVcnt,	  0x20, 10);		
		memset(g_stGPS_INFO_STATION[0].szErrorLog,  0x30, 19);		  
		memset(g_stGPS_INFO_STATION[0].szTemp,  0x20, 4);					
		memset(g_stGPS_INFO_STATION[0].szSerialNum,  0x20, 4);
		
				
		// �ܸ��� ���ο��� GPS �����带 ���Ƿ� �����ߴ��� ���� ���	
		if (memcmp(g_stGPS_INFO_STATION[0].szPass, 	  "44", 2) != 0)	
			memcpy(g_stGPS_INFO_STATION[0].szPass, 	  "00", 2);
	
		memset(g_stGPS_INFO_STATION[0].szIVcnt,0x20, 5);	// Invalid Cnt
		memset(&g_stGPS_INFO_STATION[0].szIVcnt[5], 0x20, 5);	// Valid Cnt	
		
		gpsFile2 = fopen(GPS_INFO_FILE2,"ab+");
		if(gpsFile2 != NULL)
		{
			fwrite(&g_stGPS_INFO_STATION[0],SIMXINFO_SIZE, 1, gpsFile2);
			fflush(gpsFile2);
			fclose(gpsFile2);
		}
	}
	
	// 0��°�� �ٸ� ������ �����ϱ� ����..�������, 1��°���� ������ �ִ´�.
	for (i=1; i<=g_nTotalStaCnt; i++)	
	{
		// �����뼱 ID
		memcpy(g_stGPS_INFO_STATION[i].szLineNum, g_szBusRouteID,
			sizeof(g_stGPS_INFO_STATION[i].szLineNum));	
		// �������� ID
		memcpy(g_stGPS_INFO_STATION[i].szBusID, g_szVehcID,
			sizeof(g_stGPS_INFO_STATION[i].szBusID));
		memcpy(g_stGPS_INFO_STATION[i].szStartTime, g_szRunDepartTime,
			sizeof(g_stGPS_INFO_STATION[i].szStartTime));
		// ������ ����
		memcpy(g_stGPS_INFO_STATION[i].szOrder, g_stStaInfo[i-1].bStationOrder,
			sizeof(g_stGPS_INFO_STATION[i].szOrder));
		// ������ id
		memcpy(g_stGPS_INFO_STATION[i].szID, g_stStaInfo[i-1].abStationID,
			sizeof(g_stGPS_INFO_STATION[i].szID));
		memset(g_stGPS_INFO_STATION[i].szLongitude, 0x20, 10);			// ����	
		memset(g_stGPS_INFO_STATION[i].szLatitude,  0x20, 9);				// �浵	
		memset(g_stGPS_INFO_STATION[i].szPassDis,   0x20, 3);				// ����ּҰŸ�
		memset(g_stGPS_INFO_STATION[i].szPass, 	  0x20, 2);				// �������
		memset(g_stGPS_INFO_STATION[i].szPassTime,  0x20, 14);	// ����ð�
		memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);				// ��������
		memset(g_stGPS_INFO_STATION[i].szIVcnt,	  0x20, 10);		
		memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);		   
		memset(g_stGPS_INFO_STATION[i].szTemp,  0x20, 4);					
		memset(g_stGPS_INFO_STATION[i].szSerialNum,  0x20, 4);
	}
		
	return n44;
}


/******************************************************************************
*                                                                             *
*    �Լ��� : SyncDeparttimeWithMainLogic()                                   *
*                                                                             *
*      ���� : [�ܸ��� ������ ������۽ð��� ����α��� ������۽ð���         *
               ����ȭ ��Ŵ]                                                   *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.08.01                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  		TYPE      DESCRIPTION                 *
*             ------  ----------     	--------  --------------------------- *
*             IN      szStartLogTime    char*     ����α� ������� �ð�      *
*    ���ϰ� : ����.						                                      *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
void SyncDeparttimeWithMainLogic(char* szStartLogTime)
{
	int nLoopCnt = 0;
	time_t tNowDtime = 0;

	
#ifdef _GPS_27_
	rtc_gettime(szStartLogTime);
#else
	GetRTCTime(&tNowDtime);
	TimeT2ASCDtime(tNowDtime, szStartLogTime);
#endif	
	szStartLogTime[14] = 0x00;
	
	nLoopCnt = 0;
	while(1)// ���� ���� �ð� �����Ͱ� ���ö����� 
	{
#ifdef _GPS_27_
		//driver flag�� �ܸ����� ���� ���� , ���� 1�̸� ���� �ð��� ���»��� 
		if(driveFlag == '0')  
		{
			strncpy(szStartLogTime,TRAN_TH.run_start_date_time, 14);
#else
		// TODO : if���� ������ '�������̸�'�� �´��� Ȯ�� �ʿ�
		if (gpstSharedInfo->boolIsDriveNow)  
		{
			TimeT2ASCDtime(gtDriveStartDtime, szStartLogTime);
#endif		
			szStartLogTime[14] = 0x00;
			printf("[2] TRAN_TH.run_start_date_time = %s \n", szStartLogTime);
			nLoopCnt = 0;
			break;
		}
		else
		{
			usleep(100);
		}
		nLoopCnt++;
		if (nLoopCnt == 3)	{
#ifdef _GPS_27_			
			rtc_gettime(szStartLogTime);
#else
			GetRTCTime(&tNowDtime);
			TimeT2ASCDtime(tNowDtime, szStartLogTime);
#endif
			szStartLogTime[14] = 0x00;
			
			nLoopCnt = 0;
			break;
		}
	}

}


/******************************************************************************
*                                                                             *
*    �Լ��� : GetGpsData()                                                    *
*                                                                             *
*      ���� : [gps �ø��� ��Ʈ �ݱ�]										  *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE      DESCRIPTION                       *
*             ------  ----------  --------  --------------------------------- *
*             IN      fd          int       �ø��� ��Ʈ �ڵ��ȣ              *
*			  IN/OUT  strGGA	  char*		GPGGA ����Ÿ�� ����				  *
*			  IN/OUT  strRMC	  char*		GPRMC ����Ÿ�� ����				  *
*    ���ϰ� : 0		- ����												      *
*			  0����	- gps ����Ÿ ������ ���� �߻�							  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
int GetGpsData(int fd, char* strGGA, char* strRMC)
{
	int ret = 0;
	int	bFindGGA = 0;
	int	bFindRMC = 0;
	char strGpsdata[LINE_MAX_BUFFER];
		
	while (1)	{
#ifdef	_SIMMODE_
  	    char* recv = NULL;
		if (s_SimFile != NULL)	{
			recv = fgets(strGpsdata, LINE_MAX_BUFFER, s_SimFile);
		}
		if (recv == NULL)	ret = -1;
		else				ret = 1;
#else		
		ret = GpsRecv(fd, strGpsdata, 2);
		if (ret < 0)	{
			//printf("\nGpsRecv func error!!\n");
			break;
		}
#endif

		//printf("%s",strGpsdata);
		if (strncmp(strGpsdata, "$GPGGA", 6) == 0)	{
			bFindGGA = 1;
			strcpy(strGGA, strGpsdata);
		}
		if (strncmp(strGpsdata, "$GPRMC", 6) == 0)	{
			bFindRMC = 1;
			strcpy(strRMC, strGpsdata);
		}
		
		if (bFindGGA && bFindRMC)
			break;
	}
	
	return ret;
}

/******************************************************************************
*                                                                             *
*    �Լ��� : WriteTrace()                                                    *
*                                                                             *
*      ���� : [GPS ���� ���� ���]										      *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE      DESCRIPTION                       *
*             ------  ----------  --------  --------------------------------- *
*             IN      rmc         RMC_DATA* RMC ����ü ������                 *
*    ���ϰ� : ����.							 								  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
void WriteTrace(RMC_DATA* rmc)
{
	unsigned int temp1 =0;
	double temp2=0;
	float temp3=0;
	unsigned char temp4 =0;
	
	FILE* file = NULL;
	char temp[LINE_MAX_BUFFER];
	
	long filesize = 0L;

	
//	static unsigned int init =0;
	
	memset(temp, 0x00, LINE_MAX_BUFFER);
/** // ���� ���� ��쿡�� ������ �˻��ؼ� �αױ�� ��
	
	//�ܸ��⸦ ���� ���� ��� �� ���� 0, �� �ܸ��� ���� ������� �ٽ� ó������ ������ ������ �� 
	if(init == 0)
	{
		file = fopen(LOG_INFO_FILE, "w+");
		memcpy(temp, g_szVehcID, 9);		// GPS 2.7 ���� ������� ���Ͽ� ���� id  �߰� 2005-06-09 5:24����
		UsrFwrite(temp, 9, 1, file);
		init = 1;
	}
	else 
	{
**/		
		filesize = getFileSize(LOG_INFO_FILE);
		if (filesize == 0L || filesize > MaxFileSize)
		{
			file = fopen(LOG_INFO_FILE, "w+");
			memcpy(temp, g_szVehcID, 9);	// GPS 2.7 ���� ������� ���Ͽ� ���� id  �߰� 2005-06-09 5:24����
			UsrFwrite(temp, 9, 1, file);
		}
		else
		{
			file = fopen(LOG_INFO_FILE, "a+");
		}
/**		
	}// if.. end.. 
**/		
	if( file != NULL )		
	{
		//utc time store
		temp1 = (unsigned int)(rmc->UTCTime);	
		UsrFwrite(&temp1,sizeof(temp1),1,file);
		
		//lat lon
		temp2 = rmc->Lat;
		UsrFwrite(&temp2,sizeof(temp2),1,file);
		temp2 = rmc->Lon;
		UsrFwrite(&temp2,sizeof(temp2),1,file);
		
		//angle
		temp3 = (float)(rmc->TrackTrue);
		UsrFwrite(&temp3,sizeof(temp3),1,file);
		//velocity
		temp3 = (float)(rmc->SpeedKn);
		UsrFwrite(&temp3,sizeof(temp3),1,file);
		
		//valid
		temp4 = (unsigned char)(rmc->PosStatus);
		UsrFwrite(&temp4,sizeof(temp4),1,file);

		
		fflush(file);
		fclose(file);	
	}
		
}

/******************************************************************************
*                                                                             *
*    �Լ��� : GpsRecv()                                                       *
*                                                                             *
*      ���� : [gps ����Ÿ ����]									              *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.24                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE      DESCRIPTION                       *
*             ------  ----------  --------  --------------------------------- *
*             IN      fd          int       �ø��� ��Ʈ �ڵ��ȣ              *
*             IN	  buffer   	  char*		gps ���� ����Ÿ                   *
*		      IN	  timeout	  int		����Ÿ ���� timeout �ð�(��)  	  *
*    ���ϰ� : 0,1	- ����									                  *
*            -1  	- select �Լ� timeout 			  				          *
* 			 -2		- select �Լ� ����              	  					  *
*			 -3		- read �Լ� ����										  *
*			 -4	    - ù��° �Ķ���� �� ����								  *
*			 -5		- �ι�° �Ķ���� �� ����								  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
int GpsRecv(int fd, unsigned char* buffer, int timeout)
{
	fd_set sockSet;
	struct timeval timeoutVal;
	int nbyte = 0;
	
	if (fd < 0)	{
		printf("\n GpsRecv() ù��° �Ķ���� ��Ʈ �ڵ� �� ���� !!\n");
		return -4;
	}
	
	if (buffer == NULL)	{
		printf("\n GpsRecv() �ι�° �Ķ���� ���� null �� !!\n");
		return -5;
	}
	FD_ZERO(&sockSet);
	FD_SET(fd, &sockSet);
	
	timeoutVal.tv_sec = timeout;
	timeoutVal.tv_usec = 0;	

	switch (select(fd + 1, &sockSet, NULL, NULL,  &timeoutVal  ))
	{
		case -1 :	// Select �Լ� ����
			printf("\nSelect �Լ� ���� ....\n");
			return -1;
		case 0 :	// Timeout �߻�
			return -2;
	}	
	
	nbyte = read(fd, buffer, LINE_MAX_BUFFER);
	
	if (nbyte == 0)
	{
		printf("\n��Ʈ���� ���� ����Ÿ ����.....\n");
		return 0;
	}
	
	if (nbyte < 0)
	{
		printf("\n��Ʈ �б� ����.....\n");
		return -5;
	}
	
	buffer[nbyte] = 0x00;	
	
	return 1;
}

/******************************************************************************
*                                                                             *
*    �Լ��� : ParseRMC()              	                                      *
*                                                                             *
*      ���� : [GPRMC ����Ÿ �Ľ�]                                             *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                 *
*             ------  ----------  --------        --------------------------- *
*             IN/OUT  rmc         RMC_DATA*       GPRMC ����Ÿ ����ü�� ����  *
*			  IN	  Data  	  unsigned char*  GPRMC ����Ÿ ��ü			  *
*			  IN	  DataNum	  int			  GPRMC ����Ÿ ����			  *
*    ���ϰ� : 0	: ��Ȱ�� ����Ÿ						                          *
*			  1 : Ȱ�� ����Ÿ												  *
*			 -1 : CheckSum ����												  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
int ParseRMC(RMC_DATA* rmc, unsigned char *Data, int DataNum)
{
	char buffer[LINE_MAX_BUFFER] ={0};
	int i=0,j=0;
	int temp[100];
	int ret = 0;
	
	memcpy(buffer,Data,DataNum);  //�����͸� ���� �Ѵ�. 

	if (CheckSum(Data) == 1)	// CheckSum Err
		return -1;
	
	//�޸� ��ġ�� ���� �Ѵ�. 
	while(buffer[j]!='*')
	{
		if(buffer[j]==0x2c){
			temp[i]=j;
			i++;
		}
		j++;		
	}

	for (j=0; j<i; j++)	{	
		switch (j)	{
			case 0:
				sscanf((buffer+temp[0]+1),"%lf",&rmc->UTCTime);
				memcpy(GPS.utc_world_time,(buffer+temp[0]+1),10);
				break;
			case 1:
				sscanf((buffer+temp[1]+1),"%c",&rmc->PosStatus);
				memcpy(GPS.use_kind,(buffer+temp[1]+1),1);
				GPS.use_kind[1] = 0x00;
				break;
			case 2:
				sscanf((buffer+temp[2]+1),"%lf",&rmc->Lat);
				memcpy(g_szPassMinLatitude, (buffer + temp[2] + 1),
					sizeof(g_szPassMinLatitude));
				memcpy(GPS.lattitude,(buffer+temp[2]+1),9);
				break;
			case 3:
				sscanf((buffer+temp[3]+1),"%c",&rmc->LatDir);
				memcpy(GPS.north_south,(buffer+temp[3]+1),1);
				break;
			case 4:
				sscanf((buffer+temp[4]+1),"%lf",&rmc->Lon);
				memcpy(g_szPassMinLongitude, (buffer + temp[4] + 1),
					sizeof(g_szPassMinLongitude));
				memcpy(GPS.longitude,(buffer+temp[4]+1),10);
				break;
			case 5:
				sscanf((buffer+temp[5]+1),"%c",&rmc->LonDir);
				memcpy(GPS.east_west,(buffer+temp[5]+1),1);	
				break;
			case 6:
				sscanf((buffer+temp[6]+1),"%lf",&rmc->SpeedKn);
				break;
			case 7:
				sscanf((buffer+temp[7]+1),"%lf",&rmc->TrackTrue);
				break;
			case 8:
				sscanf((buffer+temp[8]+1),"%lf",&rmc->date);
				memcpy(GPS.date,(buffer+temp[8]+1),6);
				break;
		}
	}
	
	// GPS 2.7 ����. ���� GPS ��⿡�� ���� ���� �ð��� �����Ѵ�. 2005-06-08 5:07����
	memcpy(g_szGpsCurrTime,"20",2);
	memcpy(&g_szGpsCurrTime[2],&GPS.date[4],2);
	memcpy(&g_szGpsCurrTime[4],&GPS.date[2],2);
	memcpy(&g_szGpsCurrTime[6],&GPS.date[0],2);
	memcpy(&g_szGpsCurrTime[8], GPS.utc_world_time, 6);
	gpstime_convert(g_szGpsCurrTime);
	g_szGpsCurrTime[14] = 0x00;
	
	if (strcmp(GPS.use_kind, "A") == 0)	ret = 1;
    
    return ret;
}

/******************************************************************************
*                                                                             *
*    �Լ��� : ParseGGA()              	                                      *
*                                                                             *
*      ���� : [GPGGA ����Ÿ �Ľ�]											  *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                 *
*             ------  ----------  --------        --------------------------- *
*             IN/OUT  gga         GGA_DATA*       GPGGA ����Ÿ ����ü�� ����  *
*			  IN	  Data  	  unsigned char*  GPGGA ����Ÿ ��ü			  *
*			  IN	  DataNum	  int			  GPGGA ����Ÿ ����			  *
*    ���ϰ� : 0	: ����� �ǹ� ����.				                  			  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
int ParseGGA(GGA_DATA* gga, unsigned char *Data, int DataNum)
{
	char buffer[LINE_MAX_BUFFER] ={0};
	int i=0;
	int j=0;
	int temp[100];
	
	
	memcpy(buffer,Data,DataNum);  //�����͸� ���� �Ѵ�. 

	//�޸� ��ġ�� ���� �Ѵ�. 
	while(buffer[j]!='*')
	{
		if(buffer[j]==0x2c){
			temp[i]=j;
			i++;
		}
		j++;		
	}

	//���� ���α׷������� ,�� �������� ������ ��츦 ����Ͽ� ó���Ͽ���.
	//�׷��� sscanf��ü���� ���ڰ� �ƴ� ��� �ڵ����� ó���� ���ϱ� ������ ������ �Ȼ��� 
	sscanf((buffer+temp[0]+1),"%lf",&gga->UTCTime);

	sscanf((buffer+temp[1]+1),"%lf",&gga->Lat);

	sscanf((buffer+temp[2]+1),"%c",&gga->LatDir);

	sscanf((buffer+temp[3]+1),"%lf",&gga->Lon);

	sscanf((buffer+temp[4]+1),"%c",&gga->LonDir);

	sscanf((buffer+temp[5]+1),"%d",&gga->GPSQual);

	sscanf((buffer+temp[6]+1),"%d",&gga->SatNum);
	
	memcpy(g_szPassSatelliteCnt,(buffer+temp[6]+1),2);

	sscanf((buffer+temp[7]+1),"%lf",&gga->HDOP);

	sscanf((buffer+temp[8]+1),"%lf",&gga->Altitude);

    return 0;	
}

/******************************************************************************
*                                                                             *
*    �Լ��� : CheckSum()                                                  	  *
*                                                                             *
*      ���� : [gps �ø��� ��Ʈ �ݱ�]										  *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                 *
*             ------  ----------  --------        --------------------------- *
*			  IN	  Data		  unsigned char*	GPRMC ����Ÿ ��ü		  *
*    ���ϰ� : ����	- 0				   						                  *
*			  ����	- 1											              *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
int CheckSum(unsigned char *Data)
{
	char* lpEnd;
	char tmp[3];
	int i;
	DWORD dwCheckSum = 0, dwSentenceCheckSum;

	for (i=1; i<strlen(Data) && Data[i] != '*'; i++)
		dwCheckSum ^= Data[i];
	
	memcpy(tmp, &Data[i+1], 2);	tmp[2] = 0x00;
	dwSentenceCheckSum = strtoul(tmp, &lpEnd, 16);
	if (dwCheckSum != dwSentenceCheckSum)	{
		return 1;
	}
	
	return 0;
}

// ������ ����� ȣ��� �Լ� 
void clean_up(void *arg)
{
	FILE *gpsFile = NULL;
	FILE *gpsFile2 = NULL;
	unsigned int i=0;
	unsigned char strbuff[32]={0};
	double successRate = 0;				// �νķ�
	unsigned int total_cnt=0;			// '01'�� �ν��������� + '88' + '99' + '55' + '11' ����
	unsigned int tmpTotal=0;			// �����α� ���� ���μ� ī��Ʈ
	unsigned int cnt_01=0;				// '01'�� �ν��������� ��
	unsigned int cnt_22=0;				// ������ ����� ������
	unsigned int cnt_33=0;				// �������� ������ ������
	unsigned int cnt_00=0;				// '00' ���� .. �����α� ����ü�� ù��° �迭
	unsigned int cnt_66=0;				// �뼱��Ż�� ������
	unsigned int cnt_space=0;			// �˼����� ����� ����
	unsigned int cur_order=0;			// ���� ������ order
	unsigned char Order_buff[3]={0};
	unsigned int Total_Dist=0;			// ����Ÿ�
	unsigned char buffer[SIMXINFO_SIZE]={0};
	unsigned int filesize =0;
	unsigned int loopCnt =0;			// �����α� ������ ���μ� ī��Ʈ
	unsigned int turn_cnt =0;			// ����Ƚ��
	unsigned int Order_check =0;		// �ø��� ��ȣ ������ ���� ������� ������ ������ order�� ��
	unsigned int SN_num =0;				// �ø��� ��ȣ
//	unsigned int clean_ck_flag =0;		// �ø��� ��ȣ ������ ������ ������ order�� ���� flag
//	unsigned int reset_flag =0;			// �ø��� ��ȣ reset flag
//	unsigned char szStartTimebuff[14] ={0};
	unsigned int remainder=0;			// ���� �������� 118 byte(�Ѱ��� ���ڵ� ����) ������ �˻�
	
	//2005.02.17 ������ �߰�
	unsigned char pass = 0;	// �Ÿ����� ����

	//
	unsigned int GPSDataCnt = 0;  //������ �ν��� ������� ���� �ν��� ����������� �� Data count�� ������ 2005-02-18 4:55����
	unsigned int InvalidCnt =0; // ������ �ν��� ������� ���� �ν��� ������ ������ invalid count 2005-02-18 4:55����
	unsigned int AvailCnt =0; //������ �ν��� ������� ���� �ν��� ������ ������ Avail count 2005-02-18 4:55����
	unsigned int TimeOutCnt =0; //������ �ν��� ������� ���� �ν��� ������ ������ Timeout count 2005-02-18 4:55����
	double AvailErrorPerc =0;   // �� ������ ���� �߿� valid count�� �ۼ�Ʈ 2005-02-18 4:55����
	
	int i_order = 0;
	time_t tNowDtime = 0;
	
	InvalidCnt = g_nInvalidCnt - g_nPrevInvalidCnt; 
	AvailCnt = g_nAvailCnt - g_nPrevAvailCnt;
	TimeOutCnt =(unsigned int)((g_nTimeOutCnt - g_nPrevTimeOutCnt)*2);
	GPSDataCnt = InvalidCnt + AvailCnt +TimeOutCnt; //���� ��ü ������ ����(check sum error�� ��� ���� ����)
	
	if(GPSDataCnt == 0)
		GPSDataCnt++;
		
	AvailErrorPerc = ((double)(AvailCnt) / (double)(GPSDataCnt)) *100.0;

	if (AvailErrorPerc < 1)	{
		// �̵����� �ƴѰ�� 30%�� ���� ������ �����Ѵ�.
		// �Ʒ� if(AvailErrorPerc < AvailCntTh) �� �κ��� �ҽ��� ����
		// 30% ���� ������ �⺻���� �α׸� '33'���� ����ϰ� �Ǿ� ����.
		// ���� InvalidCnt , TimeOutCnt ���� �ʿ䰡 ����.
		AvailErrorPerc = AVAIL_CNT_TH;
	}
	
	printf("\n InvalidCnt:%d, AvailCnt:%d, TimeOutCnt:%d, GPSDataCnt:%d \n",
			InvalidCnt, AvailCnt, TimeOutCnt, GPSDataCnt);
	
	// ������ ���� AvailErrorPerc ���� ���ذ��� ��ü ī��Ʈ ��� 30%�� �ȵǸ�,
	// Invaild ������ TimeOut ������ ���Ͽ� �α׸� ����Ѵ�.

	
	memset(strbuff, 0x00, 32);

#ifdef _GPS_27_
	if (gpslogwritecheck == 1) {
#else
	if (gboolIsRunningGPSThread) {
#endif
		printf("\r\n GPS log write ���� ����\r\n");
		close(g_gps_fd);
		return;
	}
	
	// �α� �� �������� ����ϴ� �� ���ڵ带 ���� ������ �ʱ�ȭ �ϴ� �� ����.	
	memset(&st_GPS_FINAL_INFO,0x20,SIMXINFO_SIZE);
	/////////////////////////////////����� ������ �ʵ� �� ///////////////////////////////////////
	memset(strbuff, 0x00, 32);	
	sprintf(strbuff, "%5d", g_nInvalidCnt);
	memcpy(st_GPS_FINAL_INFO.szIVcnt, strbuff, strlen(strbuff));
	
	memset(strbuff, 0x00, 32); 
	sprintf(strbuff, "%5d", g_nAvailCnt);
	memcpy(&(st_GPS_FINAL_INFO.szIVcnt[5]), strbuff, 5);
	// �����뼱 ID
	memcpy(st_GPS_FINAL_INFO.szLineNum, g_szBusRouteID, sizeof(st_GPS_FINAL_INFO.szLineNum));
	memcpy(st_GPS_FINAL_INFO.szBusID, g_szVehcID, 9);				// �������� ID
	memset(strbuff, 0x00, 32);
	sprintf(strbuff, "%03d",g_nTotalStaCnt+1);								// ������ �Ѽ� ���� �ϳ� ū���� ��
	memcpy(st_GPS_FINAL_INFO.szOrder, 	strbuff, 3);						// ������ ����
	memcpy(st_GPS_FINAL_INFO.szID, 		"0000000", 7);						// ������ id
	memset(strbuff,0x00,32);
	sprintf(strbuff,"%05d%05d",g_nTimeOutCnt,g_nCheckSumCnt);
	memcpy(st_GPS_FINAL_INFO.szLongitude,strbuff,10);
	memset(st_GPS_FINAL_INFO.szLatitude,  0x20, 9);							// �浵	
	memset(st_GPS_FINAL_INFO.szPassDis,	  0x20, 3);							// ����ּҰŸ� 
	memset(st_GPS_FINAL_INFO.szErrorLog,  0x20, 19);	
	memset(st_GPS_FINAL_INFO.szSateCnt,   0x20, 2);					
	memset(st_GPS_FINAL_INFO.szTemp,  0x20, 4);						
	memset(st_GPS_FINAL_INFO.szSerialNum,  0x20, 4);	
	//������ �ð��� �ִ´�
	memset(strbuff, 0x00, 32); 	
#ifdef _GPS_27_			
	rtc_gettime(strbuff);
#else
	GetRTCTime(&tNowDtime);
	TimeT2ASCDtime(tNowDtime, strbuff);
#endif
	memcpy(st_GPS_FINAL_INFO.szPassTime,strbuff, 14);  		//������ ���� �ð��� �ִ´�. 
	
	// 2005.01.14 ������ ����
	/////////////////////////////////////////////////////////////////////////////////// 
	// 05.01.13
	// File ����� üũ�ؼ� 118 Byte �� �����谡 ���� ������ ������ �����.
	filesize = 0;
	filesize = getFileSize(GPS_INFO_FILE2);
	remainder = filesize % SIMXINFO_SIZE;

	// tmp���Ͽ� ������ �߻��Ͽ��ٸ�, tmp ������ �����ϰ�, dat ���Ͽ� '44'�α׸� ����Ѵ�.			
	if(remainder != 0 || filesize >LOG_MAX_SIZE)
	{
		remainder =0;
		system("rm simxinfo2.tmp");
		usleep(100);
		
		// tmp�� ����� dat ���Ͽ� 44�� ����Ѵ�.
		gpsFile = fopen(GPS_INFO_FILE1, "ab+");
		if(gpsFile != NULL)
		{
			memcpy(&g_stGPS_INFO_STATION[0].szTemp[2], "44", 2);
			memcpy(&st_GPS_FINAL_INFO.szTemp[2], "44", 2);
			fwrite(&g_stGPS_INFO_STATION[0], SIMXINFO_SIZE, 1, gpsFile);
			fwrite(&st_GPS_FINAL_INFO, SIMXINFO_SIZE, 1, gpsFile);
			fflush(gpsFile);
			fclose(gpsFile);
		}
		close(g_gps_fd);
		return;
	}

	filesize = 0;
	filesize = getFileSize(GPS_INFO_FILE1);
	remainder = filesize % SIMXINFO_SIZE;
	// 2005.01.14 ������ �߰�
	// �� tmp ������ �̻��� ���µ�, dat ���Ͽ� ������ ������, ���� dat ������ �����ϰ�, ���ο� dat ���Ͽ� '44'�α׸� ����Ѵ�.
	if(remainder != 0 || filesize > LOG_MAX_SIZE)
	{
		remainder =0;
		system("rm simxinfo.dat");
		usleep(100);
		gpsFile = fopen(GPS_INFO_FILE1, "ab+");
		if(gpsFile != NULL)
		{
			memcpy(&g_stGPS_INFO_STATION[0].szTemp[2], "44", 2);
			memcpy(&st_GPS_FINAL_INFO.szTemp[2], "44", 2);
			fwrite(&g_stGPS_INFO_STATION[0], SIMXINFO_SIZE, 1, gpsFile);
			fwrite(&st_GPS_FINAL_INFO, SIMXINFO_SIZE, 1, gpsFile);
			fflush(gpsFile);
			fclose(gpsFile);
		}
	}
	
	
	//���� �Ÿ� ����� ���� ����ü �ʱ�ȭ 
//	memset(g_accum_dist,0x00,sizeof(ACCUM_DIST_DATA)*1000);

	// GPS ���� ����Ÿ �ܰ躰 ī���� �α����Ͽ� ���	
	sprintf(strbuff, "%d", g_nInvalidCnt);
	memcpy(g_stGPS_INFO_STATION[0].szIVcnt, strbuff, 5);	
	
	memset(strbuff, 0x00, 32); 
	sprintf(strbuff, "%d", g_nAvailCnt);
	memcpy(&(g_stGPS_INFO_STATION[0].szIVcnt[5]), strbuff, 5);	

	memset(strbuff, 0x00, 32); 
	sprintf(strbuff, "%d", g_nTimeOutCnt);
	memcpy((g_stGPS_INFO_STATION[0].szErrorLog), strbuff, 19);

	SaveCurrentStation(g_nCurStaOrder, 0);
	printf("������ ����� ������ ���\n");
	
	// �νķ� ��������	
#ifdef _GPS_27_
	gpsrate = 0;	// �켱 0%���� �ʱ�ȭ
#else
	gwGPSRecvRate = 0;
#endif


	//////////////////////////���� ����� ���� ���� ���Ͽ� ��///////////////////////////////////////
	//tmp ������ ù��° ���ڵ忡 ����� ������۽ð��� �о�´�. 
	filesize = 0;
//	gpsFile2 = NULL;
	
//	gpsFile2 = fopen(GPS_INFO_FILE2, "rb");
//	if(gpsFile2 != NULL)
//	{
//		fread(buffer,SIMXINFO_SIZE,1,gpsFile2);
//		memcpy(szStartTimebuff,&buffer[17],14);
//		memcpy(st_GPS_FINAL_INFO.szStartTime, 	szStartTimebuff, 14);  //������ �ʵ� ���� �ð����� ���� �Ѵ�.
//		fclose(gpsFile2);
//	}

	memcpy(st_GPS_FINAL_INFO.szStartTime, 	g_szRunDepartTime, 14);  //������ �ʵ� ���� �ð����� ���� �Ѵ�.
	
	
	i_order = g_nCurStaOrder;
		
	gpsFile = gpsFile2 = NULL;
	gpsFile2 = fopen(GPS_INFO_FILE2, "ab+");
	
	if (gpsFile2 != NULL) 
	{
		/////////////////////////�������� �������� �������� 3���� ä���� ���Ͽ� ����. 
		
//		if(InitStationIDFixFlag == 1)
//		{
			if(i_order != 1 && i_order != 2)
			{
				// ���� order ������ ���� �α׸� 33 ���� Set�Ѵ�.
				for(i = i_order + 1 ; i <= g_nTotalStaCnt; i++)
				{
					if(AvailErrorPerc < AVAIL_CNT_TH)
					{
						if(InvalidCnt > TimeOutCnt)
						{
							memcpy(g_stGPS_INFO_STATION[i].szPass, "88", 2);
							memcpy(g_stGPS_INFO_STATION[i].szTemp, "88", 2);								
						}
						else						
						{
							memcpy(g_stGPS_INFO_STATION[i].szPass, "99", 2);
							memcpy(g_stGPS_INFO_STATION[i].szTemp, "99", 2);								
						}
						
					}
					else
					{
						memcpy(g_stGPS_INFO_STATION[i].szPass, "33", 2);
						memcpy(g_stGPS_INFO_STATION[i].szTemp, "33", 2);						
					}
					
					memset(g_stGPS_INFO_STATION[i].szLongitude,0x20,10);
					memset(g_stGPS_INFO_STATION[i].szLatitude,  0x20, 9);	
					memset(g_stGPS_INFO_STATION[i].szPassTime, 0x20, 14);
					memset(g_stGPS_INFO_STATION[i].szPassDis, 0x20, 3);						
					memset(g_stGPS_INFO_STATION[i].szIVcnt, 0x20, 10);
					memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);
					memcpy(g_stGPS_INFO_STATION[i].szStartTime, g_szRunDepartTime, 14);
					
					if(i_order !=0)						
					{
						fwrite(&g_stGPS_INFO_STATION[i], SIMXINFO_SIZE, 1, gpsFile2);
						
					}// if.. end...
				}// for.. end...
			}
//		}

		fflush(gpsFile2);
		fclose(gpsFile2);

	}
	else
	{
		
	}


	/*�νķ� , ���� �Ÿ��� ��� �Ѵ�.                 */
	//�α� tmp ���� ����� ��� �Ѵ�. 
	filesize = 0;
	filesize = getFileSize(GPS_INFO_FILE2);
	loopCnt = filesize / SIMXINFO_SIZE; 				//��ü �α� ���� ī��Ʈ ��� ..�α����� ���μ� ���

	gpsFile = gpsFile2 = NULL;
	gpsFile2 = fopen(GPS_INFO_FILE2, "rb");
	//gpsFile = fopen(GPS_INFO_FILE1, "rb");

	
	if(gpsFile2 != NULL)
	{
		fread(buffer, SIMXINFO_SIZE, 1, gpsFile2);  	//ù��° �ʵ尪�� ���۰��̹Ƿ� ���� ��Ų��.
		
			
		//�ι�° �ʵ���� �νķ��� �����Ÿ��� ��� �Ѵ�. 
		for(i =1; i < loopCnt ; i++)
		{
												// �α����� ���μ�
			fread(buffer, SIMXINFO_SIZE, 1, gpsFile2);
			memcpy(Order_buff, &buffer[31], 3);			// order �� ����
			cur_order = atointeger(Order_buff, 3);		// order �� ����ȭ

			if(cur_order < 1 || cur_order > g_nTotalStaCnt)	// ���� order�� 1���� �۰ų� ��ü ������� ���� Ŭ��
			{
				continue;
			}
			
			tmpTotal++;
//			g_accum_dist[tmpTotal-1].order = cur_order;		// ����� ������ order�� ����
			
			// szPass �˻�
			if( memcmp(&buffer[67], "01", 2) == 0 )		
			{
				cnt_01++;
//				g_accum_dist[tmpTotal-1].pass = 1;
				  pass = 1;
			}
			else if( memcmp(&buffer[67], "22", 2) == 0 )
			{
				cnt_22++;
//				g_accum_dist[tmpTotal-1].pass = 0;
				  pass = 0;
			}
			else if( memcmp(&buffer[67], "00", 2) == 0 )
			{
				cnt_00++;
//				g_accum_dist[tmpTotal-1].pass = 0;	
				  pass = 0;		
			}
			else if( memcmp(&buffer[67], "33", 2) == 0 )
			{
				cnt_33++;
//				g_accum_dist[tmpTotal-1].pass = 0;
				  pass = 0;
			}
			else if( memcmp(&buffer[67], "66", 2) == 0 )
			{
				cnt_66++;
//				g_accum_dist[tmpTotal-1].pass = 0;
				  pass = 0;
			}
			else if( memcmp(&buffer[67], "55", 2) == 0 )
			{
//				g_accum_dist[tmpTotal-1].pass = 1;
				  pass = 1;
			}
			else if( memcmp(&buffer[67], "88", 2) == 0 )
			{
//				g_accum_dist[tmpTotal-1].pass = 1;
				  pass = 1;
			}
			else if( memcmp(&buffer[67], "99", 2) == 0 )
			{
//				g_accum_dist[tmpTotal-1].pass = 1;
				  pass = 1;
			}
			else if( memcmp(&buffer[67], "11", 2) == 0 )
			{
//				g_accum_dist[tmpTotal-1].pass = 1;
				  pass = 1;
			}
			else if( memcmp(&buffer[67], "  ", 2) == 0 )
			{
				cnt_space++;
//				g_accum_dist[tmpTotal-1].pass = 0;
				  pass = 0;
			}
			else
			{
				pass =0;  //2005-02-18 4:20����
			}
						
//			Total_Dist += (int)((sBus_station[cur_order-1].DiffDist2)*(g_accum_dist[tmpTotal-1].pass));
			Total_Dist += (int)((g_stBusStation[cur_order-1].DiffDist2)*(pass));
			pass =0;  //2005-02-18 4:20����
				
		}// for..end...
		
		printf("\ntmpTotal = %d, cnt_00 = %d, cnt_33 = %d, cnt_22 = %d, cnt_66 = %d, cnt_space = %d\n"
		        ,tmpTotal,cnt_00,cnt_33,cnt_22,cnt_66,cnt_space);
		printf("cnt_01 = %d \n", cnt_01);
		total_cnt = tmpTotal-cnt_00-cnt_33-cnt_22-cnt_66-cnt_space; 
				
		// �� ���� ���� ������ �� �� ��������
		g_nGpsExceptCnt = cnt_33+cnt_22+cnt_66;		
		if(total_cnt !=0)
		{
			// ����Ƚ�� ���
			successRate = (double)(cnt_01*100) / (double)(total_cnt); // 
//			turn_cnt =(int)( ((double)(loopCnt-cnt_33-cnt_00-cnt_space)/(double)nStation_cnt ) + 0.4);
			turn_cnt =(int)( ((double)(loopCnt-cnt_33-cnt_22-cnt_66-cnt_00-cnt_space)/(double)g_nTotalStaCnt ) + 0.4);
		}
		else
		{
			successRate =0;
			turn_cnt =0;
		}
		gwDriveCnt = turn_cnt;

		gwGPSRecvRate = (word)(successRate);

		sprintf(&st_GPS_FINAL_INFO.szErrorLog[0], "%03d", gwGPSRecvRate);

		// ������������� �̵��Ÿ��� ���Ϸ� �����Ѵ�.
		// ���������Ͽ� ���Ϸ� ���
		gdwDistInDrive = Total_Dist;

		sprintf(&st_GPS_FINAL_INFO.szErrorLog[3], "%06d", Total_Dist);
		sprintf(&st_GPS_FINAL_INFO.szErrorLog[9], "%02d", turn_cnt);
		sprintf(&st_GPS_FINAL_INFO.szErrorLog[11], "%04d", g_nGpsExceptCnt);
		memset(strbuff,0x00,32);
		
		
		fclose(gpsFile2);
		
	}// if.. end...

	
	
	// 01.21 �߰� ������ �ø��� ��ȣ �ΰ��ϴ� ��ƾ������.
	
	//33���� ������ ��ü �α� ���� ����� ��� �Ѵ�. 
	filesize = 0;
	filesize = getFileSize(GPS_INFO_FILE2);

	loopCnt = filesize / SIMXINFO_SIZE; //��ü �α� ���� ī��Ʈ ��� 
	
	
	gpsFile = gpsFile2 = NULL;

	
	gpsFile2 = fopen(GPS_INFO_FILE1,"ab");   //�α׿� ������ ����
	gpsFile = fopen(GPS_INFO_FILE2,"rb");  //�νķ� ���� ���� ������ ����. 
	SN_num =0;
	if(gpsFile != NULL && gpsFile2 != NULL)
	{
		//�α� ���Ͽ� �ø��� ��ȣ�� ���´�. 
		for(i = 0; i <loopCnt ; i++)
		{
	
			fread(buffer,31,1,gpsFile);		//�����뼱ID, ����ID, ������߽ð�
			fwrite(buffer,31,1,gpsFile2);
				
			fread(buffer,3,1,gpsFile);		//���������(i_order)
			fwrite(buffer,3,1,gpsFile2);
			
			Order_check = atointeger(buffer,3);
			
			fread(buffer,7,1,gpsFile);		//���� ������ ID
			fwrite(buffer,7,1,gpsFile2);
			
			///////////////////////////////////
			sprintf(strbuff,"%04d",SN_num);		//�ø��� ��ȣ
			fwrite(strbuff,4,1,gpsFile2);
			///////////////////////////////////
			fread(buffer,4,1,gpsFile); 			// serial skip...
			
			
			
			//����, �浵 , �ν� �Ÿ� 
			fread(buffer,22,1,gpsFile);
			fwrite(buffer,22,1,gpsFile2);

			
			memset(strbuff,0x00,sizeof(strbuff));
			
			
			fread(buffer,2,1,gpsFile);
			fwrite(buffer,2,1,gpsFile2);
			
			fread(buffer,49,1,gpsFile);
			fwrite(buffer,49,1,gpsFile2);

			//���� �ø��� ��ȣ���� ��� ����				
			SN_num++;


		}
		
		if(g_nTimeOutCnt >=1 && successRate==0)
		{
			memcpy(st_GPS_FINAL_INFO.szPass, "99", 2);
		}
			
		sprintf(strbuff,"%04d",SN_num);	
		memcpy(st_GPS_FINAL_INFO.szSerialNum,  strbuff, 4);	
		
		fwrite(&st_GPS_FINAL_INFO, SIMXINFO_SIZE, 1, gpsFile2);
			
		fflush(gpsFile);
		fclose(gpsFile);
		fflush(gpsFile2);
		fclose(gpsFile2);
	}
	else
	{
		//������ ����� ������ �ʾ����� 	
	}	

	
	system("rm simxinfo2.tmp");
	usleep(100);
	
	printf("�νķ�: %d\n",(int)successRate);
	printf("���� �Ÿ�: %lu\n",gdwDistInDrive);


	close(g_gps_fd);
}

void BusStationInfoSet()
{
	int i=0;
	char strbuff[6] = {0, };
		
    /*
     * ������ �� ���� �����Ѵ�.
     */
	g_nTotalStaCnt = gstStationInfoHeader.dwRecordCnt;
	memcpy(g_szVehcID, gstVehicleParm.abVehicleID, 9);
	memcpy(g_szBusRouteID, gstStationInfoHeader.abRouteID, 8);

	GetStationInfoForNewLogic();

	PreProcess(&g_stBusStation[0], &STA_IF_LOAD[0], &coordinates);

	/*
	 * ������ ���������� ����ü �迭�� �����Ѵ�.
	 */
	for (i=0; i<g_nTotalStaCnt; i++)	{
		memcpy(g_stStaInfo[i].abStationID, gpstStationInfo[i].abStationID, 7);
		sprintf(strbuff, "%03d", gpstStationInfo[i].wStationOrder);
		memcpy(g_stStaInfo[i].bStationOrder, strbuff, sizeof(g_stStaInfo[i].bStationOrder));
	 	g_stStaInfo[i].dLongitude = gpstStationInfo[i].dStationLongitude;  //Station longitude
	 	g_stStaInfo[i].dLatitude = gpstStationInfo[i].dStationLatitude;  //Station latitude
	 	g_stStaInfo[i].wAngle  = gpstStationInfo[i].wStationApproachAngle;  //Station Angle
	 	g_stStaInfo[i].dwDist = gpstStationInfo[i].dwDistFromFirstStation; //Station Accumulated Distance

#ifdef	_DEBUGGPS_
		printf("STA_IF_LOAD[%d].bus_sta_pos_x = %f \n", i+1, g_stStaInfo[i].dLongitude);
#endif
	}

	/*
	 * GPS �α� ����� ���� �ּҽð� ���� �ִ�ð� ��	
	 */
	strcpy(szGpsLogStartTime, "20040101010101");
	strcpy(szGpsLogEndTime, "99990101010101");
		
	printf("\n ������ �� �� : %d \n", g_nTotalStaCnt);
	
	ReadyStaInfo(g_nTotalStaCnt);	// ������ ��ǥ ��ȯ�� �˻��� ���� �۾�
	
	g_nPrevStaOrder = ContinueDriveCheck();// ���� �ν� ������ ���� �ε�

	g_nCurStaOrder = g_nPrevStaOrder; // �����ν� �������� ���� ����������
	
	printf("\n �ʱ�ġ ������ ���� : %d \n", g_nCurStaOrder);

	/*
	 * ������ ���� 1�� �̻� ��, �������� �ε��� �Ǿ��ٸ�?
	 * ó�� �������� ������ ������ �������������� �����޸𸮿� ����
	 * station_id : ������ id
	 * station_nm : ������ ��Ī
	 */
	if (g_nTotalStaCnt > 0)
	{
		memcpy( gpstSharedInfo->abNowStationID, gpstStationInfo[g_nCurStaOrder-1].abStationID, 7 );
		memcpy( gpstSharedInfo->abNowStationName, gpstStationInfo[g_nCurStaOrder-1].abStationName, 16 );
	}
	else	
	{
		memcpy(gpstSharedInfo->abNowStationID, "9999999", 7);
		memcpy(gpstSharedInfo->abNowStationName, "                ", 16);
	}
	
}

short GpsPortProc()
{
	char szStartLogTime[15];
		
#ifdef _SIMMODE_
	printf("\r\n s_SimFile = fopen(GPS_RAW_FILE OPEN... \r\n");
	s_SimFile = fopen(GPS_RAW_FILE, "r");
	if (s_SimFile != NULL)	printf("\n open Success........\n");
	printf("\r\n s_SimFile = fopen(GPS_RAW_FILE OPEN SUCCESS... \r\n");
#endif
	// GPS ��Ʈ ����
	g_gps_fd = GpsPortOpen(GPS_DEV_NAME, BAUD_RATE, 1);
	if (g_gps_fd > 0)
		printf("\n GPS ��Ʈ ���� : %d", g_gps_fd);
	else
		return g_gps_fd;

	// ������ ���� Ų���� �ƴϰ�, ���� ��������ϴ� ���
	if (g_nContinueFlag == 0) {
		SyncDeparttimeWithMainLogic(szStartLogTime);
		strcpy(g_szRunDepartTime, szStartLogTime);
		g_szRunDepartTime[14] = 0x00;
	}
	
	return g_gps_fd;
}

static void GpsDataProc()
{
//	struct timeval stTime1, stTime2;
	char szCurrTime[15];
//	int bTimeCheck = 0;
	int	 nParam = 0;	
	int	 nFoundSta = 0;
//	FOUND_STA_INFO stFoundStaInfo;
	time_t tTime1, tTime2;
	int nMaxSearchTime = 0;
	double dbGpsActStatus = 0;
	time_t tReadDtime;

	gpsloopcheck = 4;


	nParam = GpsDataProcessing();
//		GpsErrorProcess(nParam);

	gpstSharedInfo->gbGPSStatusFlag = g_gpsStatusFlag;


	GetRTCTime( &tReadDtime );
	TimeT2ASCDtime( tReadDtime, szCurrTime );

	/*
	 * ���� �������� �ν��� �ð��� ���� �ð��� ���Ͽ� ���ؽð� �̻� 
	 * �������� �ν����� ���ϰ�, GPS ���׳� ���°� ���� �ܼ� �Ǵ� ���� �ҷ��ΰ��
	 * �߰��Ÿ��� '0'�� �ϴ� �÷��׸� �����Ѵ�
	 */
	tTime1 = GetTimeTFromASCDtime(szCurrTime);	// ����ð�
	tTime2 = GetTimeTFromASCDtime(g_szStaFindTime);	// ���� ������ �ν����� �� �ð�

	nMaxSearchTime = GetMaxTimeforNextStaSearch(); // ���ؽð� ���ϱ�
	dbGpsActStatus = GetGpsActStatus();
	
	if ( dbGpsActStatus < GPS_STATUS_VALID_BOUND && abs(tTime1-tTime2) > nMaxSearchTime) 
	{
//		printf("\n������ �˻��� ���� ���ϰ� �ִ� �����Դϴ�.\n");
		/*
		 * boolIsValidGPSSearch ���ǵǾ� �־�� ��
		 * EVENT_GPS_SEARCH_ERROR ���ǵǾ� �־�� ��
		 */
		//gpstSharedInfo->boolIsValidGPSSearch = FALSE;
		//ctrl_event_info_write(EVENT_GPS_SEARCH_ERROR);
	}
	else
	{
		;//gpstSharedInfo->boolIsValidGPSSearch = TRUE;
	}
	
	
	/*
	 * ������ ������ �ְ�, gps ���� ����Ÿ�� ������ ��츸 ������
	 * �˻��ϵ��� ��
	 */
	if (g_nTotalStaCnt > 0 && g_gpsStatusFlag == GPS_STATUS_OK)	{
		GPSCorr(&GPS, &s_stRMC, &g_stGGA);
		DBManage(&g_stBusStation[0]);
		nFoundSta = GetStation();
//		nFoundSta = BusStationDetecting(&stFoundStaInfo, s_stRMC.Lat, 
//		                                 s_stRMC.Lon, s_stRMC.UTCTime, 
//		                                 s_stRMC.SpeedKn);
	}
	else
		nFoundSta = 0;
		
	if (nFoundSta > 0)	{
		// ���տ����� ?
		memcpy( gpstSharedInfo->abNowStationID, gpstStationInfo[nFoundSta-1].abStationID, 7 );
		memcpy( gpstSharedInfo->abNowStationName, gpstStationInfo[nFoundSta-1].abStationName, 16 );
		memcpy( gabGPSStationID, gpstStationInfo[nFoundSta-1].abStationID, 7 );
	}

	// �ð� ������ƾ
	if(bTimeCheck == 0)	
	{
		if(gettimeofday ( &stTime1, NULL ) != -1) 
		{
/**		
#ifdef	_GPS_27_				
			rtc_gettime(szCurrTime);
#else
			GetRTCTime( &tReadDtime );
			TimeT2ASCDtime( tReadDtime, szCurrTime );
#endif
**/
//			printf("[GpsDataProc] ����ð� : %s\n", szCurrTime); //0330 2005.2.28
			bTimeCheck = 1;
		}
		else
		{
			close(g_gps_fd);
			printf("Thread terminate 1 \n");
			return;
		}
	}
	
	if(gettimeofday ( &stTime2, NULL ) == -1 )	{
		
		close(g_gps_fd);			
		printf("Thread terminate 3 \n");
		return;
	}	
	if (stTime2.tv_sec - stTime1.tv_sec >= TIME_CHECK_GAP)	{
		bTimeCheck = 0;
		gpstime_check();
	}
			
	
	EndStationAdviceMsg();		
	
}

void GpsExitProc(void)
{
#ifdef _SIMMODE_
	fclose(s_SimFile);
#endif	
	close(g_gps_fd);  
	
}
