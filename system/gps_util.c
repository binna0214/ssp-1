#include "gps_define.h"

#ifdef	_GPS_27_
	#include "../include/bus100.h"
#else
	#include <stdio.h>
	#include <stdlib.h>
	#include "../../common/type.h"
	#include "../system/bus_type.h"
	#include "gps.h"		
//	#include "gps_env.h"
	#include "gps_search.h"
	#include "gps_util.h"

#endif

long getFileSize(char* f)
{
	struct stat s;
	s.st_size = 0;
	if (stat(f, &s) != 0)	s.st_size = 0;
	return (long)s.st_size;	
}

/****************************************************************************
* Function: void LatLonHgtToXYZ_D(llhstruc *llh, cartstruc *xyz)
*
* Converts from WGS-84 latitude, longitude and height and X, Y and Z
* rectangular co-ordinates. For more information: Simo H. Laurila,
* "Electronic Surveying and Navigation", John Wiley & Sons (1976).
*
* Input: llh - the (lat,lon,hgt) vector.
*
* Output: xyz - (x,y,z) vector.
*         
* Return Value: None.
****************************************************************************/
void LatLonHgtToXYZ_D(double llh[3],double  xyz[3])
{
    double n;         /* WGS-84 Radius of curvature in the prime vertical. */
    double a;                                    /* WGS-84 semimajor axis. */
    double e;                                /* WGS-84 first eccentricity. */
    double ome2;                                   /* WGS-84 (1.0E0 - e2). */
    double clat;                                              /* cos(lat). */
    double slat;                                              /* sin(lat). */
    double clon;                                              /* cos(lon). */
    double slon;                                              /* sin(lon). */
    double d,nph;
    double tmp;

    a = 6378137.0E0;
    e = 0.0818191908426E0;
    ome2 = 0.99330562000987;

    clat = cos(llh[0]);
    slat = sin(llh[0]);  //sin���� 0-1������ ���� ���� 
    clon = cos(llh[1]);
    slon = sin(llh[1]);
    d = e*slat;   //d ���� 1���� ũ�ų� ���� ���� ���÷��� 12.222���� Ŀ�� �ȴ�. �׷��� slat���� 0-1������ ���ۿ� ���� 

    n = a/sqrt(1.0E0-d*d);  //d���� �ִ밪�� 0.0818�̹Ƿ� ����� 0���� ���� ���� ����. 
    nph = n + llh[2];

    tmp = nph*clat;
    xyz[0] = tmp*clon;
    xyz[1] = tmp*slon;
    xyz[2] = (ome2*n + llh[2])*slat;
    
}

/**************************************************************************************
*  Convert from WGS-84 ECEF cartesian coordinates to  rectangular local-level-tangent 
*  ('East'-'North'-Up) coordinates.
*
*    INPUTS  xyz = ECEF x-coordinate in meters
*            orgxyz = ECEF x-coordinate of local origin in meters
*            LLA = orgxyz's lla value
*            ENU = ENU result value
*
*    OUTPUTS
*            ENU = ENU coordinate relative to local origin (meters)
***************************************************************************************/
int XyztoENU_D(double xyz[3], double orgxyz[3],double LLA[3],double ENU[3])
{
	double tmpxyz[3], tmporg[3], difxyz[3], phi, lam, sinphi, cosphi, sinlam, coslam, R[3][3];
	short i;

	memset(ENU,0,sizeof(ENU));


	for(i=0;i<3;i++){
		tmpxyz[i] = xyz[i];
		tmporg[i] = orgxyz[i];
		difxyz[i] = tmpxyz[i] - tmporg[i];
	}
	//f_tolla(orgxyz);
	//phi = LLA[3];
	phi = LLA[0];
	lam = LLA[1];	
	sinphi = sin(phi);
	cosphi = cos(phi);
	sinlam = sin(lam);
	coslam = cos(lam);
	R[0][0] = -sinlam;             
    	R[0][1] = coslam;
	R[0][2] = 0 ;
	R[1][0] = -sinphi*coslam;
	R[1][1] = -sinphi*sinlam; 
	R[1][2] = cosphi;
	R[2][0] = cosphi*coslam;
	R[2][1] = cosphi*sinlam;
	R[2][2] = sinphi;
	for (i=0;i<3;i++) 
		ENU[i] = R[i][0]*difxyz[0]+R[i][1]*difxyz[1]+R[i][2]*difxyz[2];	
		
	
	return  1 ;	
}

/******************************************************************************
*                                                                             *
*    �Լ��� : BubbleSortStaInfo()                                         	  *
*                                                                             *
*      ���� : [���� ��� ����� �Ÿ������� ����]					  		  *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                 *
*             ------  ----------  --------        --------------------------- *
*			  ����.	      													  *
*    ���ϰ� : ����.				   						                      *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
void BubbleSortStaInfo()
{
	int ck = 1;	// ���� ��Ʈ�� ����Ȱ� �������� �ʱⰪ �ο�.
	double dbM;
	int    nN;
	int	i;
	
	// X��ǥ ���� ������ ����� ������ ������ �Ѵ�.
	while (ck != 2)	{
		ck = 2;
		for (i=1; i<g_nTotalStaCnt-1; i++)	{
			if (abs(g_stSortX[i].dbX) > abs(g_stSortX[i+1].dbX)) {
				dbM = g_stSortX[i].dbX;
				nN = g_stSortX[i].nOrder;
				g_stSortX[i].dbX = g_stSortX[i+1].dbX;
				g_stSortX[i].nOrder = g_stSortX[i+1].nOrder;
				g_stSortX[i+1].dbX = dbM;
				g_stSortX[i+1].nOrder = nN;
				ck = 1;
			}
		}
	}

	// Y��ǥ ���� ������ ����� ������ ������ �Ѵ�.
	ck = 1;
	while (ck != 2)	{
		ck = 2;
		for (i=1; i<g_nTotalStaCnt-1; i++)	{
			if (abs(g_stSortY[i].dbY) > abs(g_stSortY[i+1].dbY)) {
				dbM = g_stSortY[i].dbY;
				nN = g_stSortY[i].nOrder;
				g_stSortY[i].dbY = g_stSortY[i+1].dbY;
				g_stSortY[i].nOrder = g_stSortY[i+1].nOrder;
				g_stSortY[i+1].dbY = dbM;
				g_stSortY[i+1].nOrder = nN;
				ck = 1;
			}
		}
	}
	
}

double GetDis(double x1, double y1, double x2, double y2)
{
	double ret = 0;
	
	ret = sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
	
	return ret;
}

// ���Ͽ� ����ϴ� �Լ��� ���� ������ ���� �߻��� 3�������� ��õ��ϰ�,
// �� �̻� ������ �߻��ϸ�, ����
// GPS 2.7 ����
size_t UsrFwrite(const void *data, size_t size, size_t count, FILE *stream)
{
	size_t	ret=0;
	unsigned int i = 0;
	unsigned int trycnt = 3;
	
	while (i < trycnt)	{
		ret = fwrite(data, size, count, stream);
		if (ret < 0)	i = i+1;
		else			break;
	}
	
	return ret;
}

double AtoDouble(char* str, int len)
{
	char	temp[255];

	if (len < 1)	return -1.;
		
	memcpy(temp, str, len);
	temp[len] = 0x00;
	
	return (double)(atof(temp));
}
void gpstime_convert (char *gps_time)
{

	int month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	int gps_yyyy, gps_mm, gps_dd, gps_hh, gps_mn, gps_ss;
	char temp[20];

	gps_yyyy = atointeger (gps_time, 4);
	gps_mm = atointeger (&gps_time[4], 2);
	gps_dd = atointeger (&gps_time[6], 2);
	gps_hh = atointeger (&gps_time[8], 2);
	gps_mn = atointeger (&gps_time[10], 2);
	gps_ss = atointeger (&gps_time[12], 2);


	if	((gps_hh + 9) < 24) {			// �ð����� 9�ð� ���� ���ں��� ������
		sprintf (temp, "%02d", gps_hh + 9);
		memcpy (&gps_time[8], temp, 2);
		return;
	}

	if	((gps_yyyy % 4) == 0) {			// ���� 2�� �ϼ� �Ǵ� 
		month[1] = 29;
		if	((gps_yyyy % 100) == 0) {
			month[1] = 28;
			if	((gps_yyyy % 400) == 0) 
				month[1] = 29;
		}
	}

	gps_hh = (9 + gps_hh) - 24; 		// �ð����� 9�ð� ����  ���� ���� ������ 
	if	((gps_dd + 1) <= month[gps_mm-1]) {	// ���� ������ �� ���� ������ 
		gps_dd++;
		sprintf (temp, "%02d%02d", gps_dd, gps_hh);
		memcpy (&gps_time[6], temp, 4);
		return;
	}

	if	((gps_mm + 1) <= 12) {				// �� ����� �� ���� ������ 
		gps_dd = 1;
		gps_mm++;
		sprintf (temp, "%02d%02d%02d", gps_mm, gps_dd, gps_hh);
		memcpy (&gps_time[4], temp, 6);
	}
	else {								// �� �����   �⵵ ���� ������ 
		gps_dd = 1;			 
		gps_mm = 1;
		gps_yyyy++;
		sprintf (temp, "%04d%02d%02d%02d", gps_yyyy, gps_mm, gps_dd, gps_hh);
		memcpy (gps_time, temp, 10);
	}

	return;
}

/*****************************************************************************
 *		
 *	function: HeadingErrorABS_D(int Ang, int RefAng)
 *	Param: Ang : angle 1 , RefAng : angle2
 *  	Return: Heading Error abs result
 *	discription: �� ������ ������ ���밪�� �ش�. 
 *	note: 
 ****************************************************************************/
int HeadingErrorABS_D(int Ang, int RefAng)
{
	int DiffAng;

	DiffAng = Ang - RefAng;
	
	if (DiffAng > 180)
	{
		DiffAng = DiffAng -360;
	}

	if(DiffAng <= -180)
	{
		DiffAng = DiffAng + 360;
	}
	return abs(DiffAng);
}

int gpstime_check(void)
{
	char gps_time[15];
	char gps_yyyy[5];
	char gps_mt[3];
	char gps_dd[3];
	char gps_hh[3];
	char gps_mm[3];
	char gps_ss[3];	

	char gps_rtime[15];      // 2005.01.26 : 03.28 ����	
	
	time_t tReadDtime;
	time_t tSetDtime;
	short ret = 0;

	memset(gps_time,0x00,sizeof(gps_time));
	memset(gps_yyyy,0x00,sizeof(gps_yyyy));
	memset(gps_mt,0x00,sizeof(gps_mt));
	memset(gps_dd,0x00,sizeof(gps_dd));
	memset(gps_hh,0x00,sizeof(gps_hh));
	memset(gps_mm,0x00,sizeof(gps_mm));	
	memset(gps_ss,0x00,sizeof(gps_ss));	


	memcpy(gps_time,"20",2);
	memcpy(&gps_time[2],&GPS.date[4],2);
	memcpy(&gps_time[4],&GPS.date[2],2);
	memcpy(&gps_time[6],&GPS.date[0],2);
	memcpy(&gps_time[8],GPS.utc_world_time,6);

	memcpy(gps_yyyy,gps_time,4);
	memcpy(gps_mt,&gps_time[4],2);
	memcpy(gps_dd,&gps_time[6],2);
	memcpy(gps_hh,&gps_time[8],2);
	memcpy(gps_mm,&gps_time[10],2);	
	memcpy(gps_ss,&gps_time[12],2);

	
	if (atointeger(gps_time,4) < 2000 || atointeger(gps_time,4) > 2030) return -1;
	
	if (atointeger(&gps_time[4],2) < 1 || atointeger(&gps_time[4],2) > 12) return -1;	

	if (atointeger(&gps_time[6],2) < 1 || atointeger(&gps_time[6],2) > 31) return -1;	

	if (atointeger(&gps_time[8],2) < 0 || atointeger(&gps_time[8],2) > 23) return -1;
	
	if (atointeger(&gps_time[10],2) < 0 || atointeger(&gps_time[10],2) > 59) return -1;	
	
	if (atointeger(&gps_time[12],2) < 0 || atointeger(&gps_time[12],2) > 59) return -1;	

	gpstime_convert(gps_time);

#ifdef	_GPS_27_
	// SetRTCTime(timet);
	// GetRTCTime(&timet);
	printf("\n g_stRMC.PosStatus = %c \n", g_stRMC.PosStatus);
    if ( rtc_gettime( gps_rtime ) > 2 )              // 2005.5.9 ���� <=  // 2005.01.26 : 03.28 ����	Start
    {
        rtc_settime(&gps_time[2]);
    }
    else if ( strncmp( gps_rtime, "200401", 6 ) == 0 )
    {
        rtc_settime(&gps_time[2]);
    }
	else if ( g_stRMC.PosStatus == 'A' )
	{
	    rtc_settime(&gps_time[2]);
    }                                                          // 2005.01.26 : 03.28 ����	End
#else
	ret = GetRTCTime( &tReadDtime );
	memset(gps_rtime, 0x00, 15);
	TimeT2ASCDtime( tReadDtime, gps_rtime );
   	tSetDtime = GetTimeTFromASCDtime(gps_time);
    if ( ret >= 2 )              // 2005.5.9 ���� <=  // 2005.01.26 : 03.28 ����	Start
    {
        SetRTCTime(tSetDtime);
    }
    else if ( strncmp( gps_rtime, "200401", 6 ) == 0 )
    {
        SetRTCTime(tSetDtime);
    }
	else if ( s_stRMC.PosStatus == 'A' )
	{
	    SetRTCTime(tSetDtime);
    }                                                          // 2005.01.26 : 03.28 ����	End

#endif
	
	return 0;
}



// ���� ���� 3.xx �߿�� �ݿ��ߴ� ������ ������� �ʰ� ����.
int GpsTimeCheck(void)
{
	int gps_fd = 0;
	int retVal = 0;
	int success_yn = 0;	
	unsigned char szSentence[1024];	
	unsigned char szCopy[1024];
	char strGGAdata[LINE_MAX_BUFFER];
	char strRMCdata[LINE_MAX_BUFFER];	

	memset(szSentence,0x00,sizeof(szSentence));
	memset(szCopy,0x00,sizeof(szCopy));
	
	gps_fd = GpsPortOpen("/dev/ttyE1", GPS_BPS_SPEED, 1);
	if (gps_fd < 0) {
		printf("\r\n gpsOpen fail \r\n");
		return -1;
	}
	

	retVal = GetGpsData(gps_fd, strGGAdata, strRMCdata);

	if (retVal == -2)	{	// gps ���� �Ҵ�, Timeout
		printf("\r\n gpsRecv fail \r\n");	
		close(g_gps_fd);
		return retVal;
	}
	
	success_yn = ParseRMC(&s_stRMC, szCopy, strlen(szCopy));
	retVal = gpstime_check();
	if (retVal < 0) {
		printf("\r\n gpstime_check fail \r\n");
	}
	else	{
		printf("\n gpstime_check success \n");
	}
	close(gps_fd);	
	return retVal;
	
}

int IsOverDistance(int n, float dis)
{
	int result = 0;
	int gap = 0;
	int L1, L2;
	
	L1 = (int)g_stStaInfo[n].dwDist;

	if (n-1 >= 0)
		L2 = (int)g_stStaInfo[n-1].dwDist;
	else
		L2 = 0;

	gap = L1 - L2;

	if (gap * DISTANCE_MAX_TIMES < dis)
		result = 1;
	else
		result = 0;

	return result;
}

int GetMaxTimeforNextStaSearch(void)
{
	double dbDistToNext = 0;
	int nNextOrder = 0;
	int s = 0;
	/* 
	 * ���� ������� ���������尣 �Ÿ�
	 */
	nNextOrder = g_nCurStaOrder;
	if (nNextOrder < 1)	nNextOrder = 1;
	dbDistToNext = g_stBusStation[nNextOrder].DiffDist2;

	if (g_dbMaxSpeed > 0)	// �ӵ����� �ִ� ���
	{
		s = dbDistToNext / (g_dbMaxSpeed / 2);
	}
	else					// �ӵ����� ���� ���
	{
		s = dbDistToNext / DEFAULT_MAX_SPEED;
	}

	return s;
}

double GetGpsActStatus()
{
	double r = 0;
	int nValidCnt = 0;
	int nInvalidCnt = 0;
	int nTimeoutCnt = 0;
	int nIndex = 0;
	int nRoof = 0;

	nIndex = g_nCurStaOrder-1;

	if (nIndex < 0)	nIndex = 0;

	nRoof = GPS_STA_CNT_REF_BOUND;

	while (nRoof > 0)	{
		nRoof--;
		nValidCnt += g_stGpsStatusCnt[nIndex].nValidCnt;
		nInvalidCnt += g_stGpsStatusCnt[nIndex].nInvalidCnt;
		nTimeoutCnt += g_stGpsStatusCnt[nIndex].nTimeoutCnt;
		nIndex--;
		if(nIndex < 0)	nIndex = g_nTotalStaCnt-1;
	}

	r = nValidCnt / (nValidCnt+nInvalidCnt+nTimeoutCnt) * 100.;

	return r;
}

void GetStationInfoForNewLogic(void)
{
	byte buff[11];
	int i=0;
	
	TimeT2ASCDtime(gstStationInfoHeader.tApplyDtime, STA_IF_LOAD[0].appltn_dtime);
	STA_IF_LOAD[0].appltn_seq[0] = gstStationInfoHeader.bApplySeqNo;
	memcpy(STA_IF_LOAD[0].bus_route_id, g_szBusRouteID, sizeof(g_szBusRouteID));			  // �����뼱 ID                  
	memcpy(STA_IF_LOAD[0].transp_method_cd, gstStationInfoHeader.abTranspMethodName, sizeof(gstStationInfoHeader.abTranspMethodName));		  // �����뼱��                   
	INT2ASC(gstStationInfoHeader.dwRecordCnt, buff);
	memcpy(STA_IF_LOAD[0].bus_sta_cnt, buff, sizeof(STA_IF_LOAD[0].bus_sta_cnt));				  // ���������尹��         


	for (i=0; i<g_nTotalStaCnt;	i++)	{
		STA_IF_LOAD[i].appltn_seq[0] = STA_IF_LOAD[0].appltn_seq[0];
		memcpy(STA_IF_LOAD[i].bus_route_id, STA_IF_LOAD[0].bus_route_id, sizeof(STA_IF_LOAD[0].bus_route_id));			  // �����뼱 ID                  
		memcpy(STA_IF_LOAD[i].transp_method_cd, STA_IF_LOAD[0].transp_method_cd, sizeof(STA_IF_LOAD[0].transp_method_cd));		  // �����뼱��                   
		memcpy(STA_IF_LOAD[i].bus_sta_cnt, STA_IF_LOAD[0].bus_sta_cnt, sizeof(STA_IF_LOAD[0].bus_sta_cnt));				  // ���������尹��         

		memcpy(STA_IF_LOAD[i].bus_sta_id, gpstStationInfo[i].abStationID, sizeof(gpstStationInfo[i].abStationID));				  // ���������� ID                
		
		memcpy(&STA_IF_LOAD[i].city_bnd_in_out_class_cd, &gpstStationInfo[i].bCityInOutClassCode, sizeof(gpstStationInfo[i].bCityInOutClassCode));  // �ð賻�ܱ��� �ڵ�            

		WORD2ASC(gpstStationInfo[i].wStationOrder, buff);
		memcpy(STA_IF_LOAD[i].bus_sta_order, buff, sizeof(STA_IF_LOAD[i].bus_sta_order));        // ���������                   
		
		memcpy(STA_IF_LOAD[i].bus_sta_pos_nm, gpstStationInfo[i].abStationName, sizeof(gpstStationInfo[i].abStationName));		  // ���������� ��                
		
		DoubleToASC(gpstStationInfo[i].dStationLongitude, buff);
		memcpy(STA_IF_LOAD[i].bus_sta_pos_x, buff,sizeof(STA_IF_LOAD[i].bus_sta_pos_x));	  // ������������ǥ X             
		DoubleToASC(gpstStationInfo[i].dStationLatitude, buff);
		memcpy(STA_IF_LOAD[i].bus_sta_pos_y, buff,sizeof(STA_IF_LOAD[i].bus_sta_pos_y));			  // ������������ǥ Y             
		
		WORD2ASC(gpstStationInfo[i].wOffset, buff);
		memcpy(STA_IF_LOAD[i].dist_6, buff, sizeof(STA_IF_LOAD[i].dist_6));						         // Off Set                      
		
		DWORD2ASC(gpstStationInfo[i].dwDistFromFirstStation, buff);
		memcpy(STA_IF_LOAD[i].bus_sta_cum_dist, buff, sizeof(STA_IF_LOAD[i].bus_sta_cum_dist));	  // ù�����忡���� �Ÿ�(BCD)     	      
		
		WORD2ASC(gpstStationInfo[i].wStationApproachAngle, buff);
		memcpy(STA_IF_LOAD[i].heading, buff, sizeof(STA_IF_LOAD[i].heading));						         // ���                  
	
	}
}

int GetConstantlyNumber(int nFoundSta[], int cnt)
{
	int nTemp[10] = {0,};
	int nCheck[10] = {0, };
	int i=0, j=0;
	int ret = 0;
	int nMax = 0;
	
	for (i=0; i<cnt; i++) {
		for (j=0; j<cnt; j++) {
			if (nFoundSta[i] == nTemp[j])	{
				nCheck[j] += 1;
				break;
			}
		}
	}

	for (i=0; i<cnt; i++)	{
		if (nMax <= nCheck[i])	{
			nMax = nCheck[i];
			ret = nFoundSta[i];
		}
	}

	return ret;
}

/******************************************************************************
*                                                                             *
*    �Լ��� : GpsDataProcessing()                                             *
*                                                                             *
*      ���� : [gps ����Ÿ ���μ���]		   								      *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE      DESCRIPTION                       *
*             ------  ----------  --------  --------------------------------- *
*             ����.             											  *
*    ���ϰ� : 0		- ����												      *
*			  0����	- gps ����Ÿ ������ ���� �߻�							  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
int GpsDataProcessing(void)
{

	int ret = 0;
	int result = 0;
	char strGGAdata[LINE_MAX_BUFFER];
	char strRMCdata[LINE_MAX_BUFFER];
		
	result = GetGpsData(g_gps_fd, strGGAdata, strRMCdata);
#ifdef	_DEBUGGPS_
	printf("\n GetGpsData returned %d, \n GPRMC DATA : %s \n", result, strRMCdata);
#endif
	// �о�� ����Ÿ�� ���� : 1�� ���	
	if (result == 1)	{
		ret = ParseRMC(&s_stRMC, strRMCdata, strlen(strRMCdata));
		if (ret == 1)
			ParseGGA(&g_stGGA, strGGAdata, strlen(strGGAdata));
		
		// HDOP ��ġ�� 3���� ũ�� ���ŵ� ����Ÿ�� ����
		if (g_stGGA.HDOP > MAX_BOUND_HDOP)	
		{
			printf("\nHDOP ��ġ ������ : [%f]\n", g_stGGA.HDOP);
			ret = 0;
		}			
	}

	// ������� ����
	WriteTrace(&s_stRMC);
	
	if (result == 1 && ret == 1) {
		ret = 1; // Valid ����Ÿ(����) ó��
		g_nAvailCnt++;// = g_nAvailCnt + 1;
		g_stGpsStatusCnt[g_nCurStaOrder-1].nValidCnt++;
		g_gpsStatusFlag = GPS_STATUS_OK; 	// GPS ����Ÿ ���Ŵ���		
	}
	else if (result == 1 && ret == 0) {
		ret = 0; // Invalid ����Ÿ(����) ó��
		g_nInvalidCnt++; // = g_nInvalidCnt + 1;
		g_stGpsStatusCnt[g_nCurStaOrder-1].nInvalidCnt++;
		g_gpsStatusFlag = GPS_INVALID_DATA;	// GPS ����Ÿ �����̻�
	}
	else if (result == 1 && ret == -1) {
		printf("\nChecksum Err....\n");
		ret = -1;// Checksum Err
		g_nCheckSumCnt++;// = g_nCheckSumCnt + 1;
		g_gpsStatusFlag = GPS_INVALID_DATA;	// GPS ����Ÿ �����̻�
	}
	else if (result == -2) {
		//printf("\nTimeout �߻�....\n");
		ret = -2;// Timeout �߻�
		g_nTimeOutCnt++;// = g_nTimeOutCnt + 1;
		g_stGpsStatusCnt[g_nCurStaOrder-1].nTimeoutCnt++;
		g_gpsStatusFlag = GPS_CONN_OUT; 	// GPS ����Ÿ ���Ŵ���
	}
	else
		ret = -3;// ��Ÿ �ý��� �Լ� ����
	
	return ret;
}


#ifndef	_GPS_27_

int atointeger (byte *s, byte length)
{
	byte temp[12];

	memcpy(temp,s,length);
	temp[length] = 0;
	return (atoi(temp));
}

void EndStationAdviceMsg( void )
{

	if ( memcmp(gabEndStationID, gpstSharedInfo->abNowStationID, 7) == 0 )
	{
		gboolIsEndStation = TRUE;			
	}
	else
	{ 
		gboolIsEndStation = FALSE;
	}
	
	gdwDelayTimeatEndStation = 0;		
}	

#endif	

void DoubleToASC(double src, byte* desc)
{
	sprintf(desc, "%5.4f",  src);
}


