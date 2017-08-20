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
	#include "device_interface.h"

#endif




//���� �˻��� ���� ������
static int s_nVirDetectCnt = 0;
static int s_nCurVirStaOrder = 1;
static int s_nPrevVirStaOrder = 1;

static int s_nRecogFlag = 0;	// ������ ���� ���� �÷���

void ErrorLogCheck(unsigned int Order, unsigned int Index);

/******************************************************************************
*                                                                             *
*    �Լ��� : BusStationDetecting()                                        	  *
*                                                                             *
*      ���� : [������ �˻�]										  			  *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME      TYPE            DESCRIPTION             *
*             ------  ----------      --------        ----------------------- *
*			  IN	  stFoundStaInfo  FOUND_STA_INFO* �˻��� ������ ����      *
*    ���ϰ� : �߰��� ������ ����.				   						      *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
int BusStationDetecting(FOUND_STA_INFO *stFoundStaInfo, double x, double y, 
                                                   double utctime, double speed)
{
	double GPSLatDeg;
	double GPSLonDeg;
	double GPSHgt;
	double EnuPos[3]={0};
	double TempLLH[3];
	double TempXYZ[3];
	double CurrentVelms =0;         //���� GPS �ӵ� ����
	
	unsigned int hour;
	unsigned int min;
	double sec;	
	
	int nCurStaOrder = 1;	// ���� ������ ����
	int nNextStaOrder = 0;
	
	int	nMinOrder = 1;	//���� ��ġ���� ���� ����� ���������.
	double dbDis = 0;	//���� ����� ������ ������ �Ÿ�
	double dbCurStaOrderDis = 0; // �� ��ġ, ���� ������ �� �Ÿ�
	double dbNextStaOrderDis = 0; // �� ��ġ, ���� ������ �� �Ÿ�
	
	int nRecogFlag = 0;	// ���� ���� �÷���
	int nDecidedFlag = 0;	// �ν� ���� �÷���

	char szCurrTime[15];	// ���� �ð�


	//--------------------------------------------------
	// ���� ��ġ�� ��� ����
	hour = (int)(utctime/10000);
	min = (int)((utctime - hour*10000)/100);
	sec = utctime - hour*10000 - min*100;
//	GPSsec = hour*3600 + min*60 + sec;

	// �� �� -> �� 
	GPSLatDeg = (int)(x/100) + ((x - (int)(x/100)*100.)/60) ;
	GPSLonDeg = (int)(y/100) + ((y - (int)(y/100)*100.)/60) ;
	GPSHgt = 30;

	TempLLH[0] = GPSLatDeg*D2R;
	TempLLH[1] = GPSLonDeg*D2R;
	TempLLH[2] = GPSHgt;
	

	//LLH���� XYZ������ 
	LatLonHgtToXYZ_D(TempLLH,TempXYZ);  //���� ��ǥ�� XYZ���� 


	//XYZ���� ���� ���� ��ġ�� ENU������ �ٲپ��� 
	//DB 1�� ���� ENU ���� ���ϰ� �ȴ�. 
	//EnuPos ���� ���� ��ġ X, Y
	XyztoENU_D(TempXYZ,coordinates.OrgXYZ,coordinates.OrgLLH,EnuPos); 

	// ���� �ӵ�
	CurrentVelms = speed *1.852/3.6;  // unit m/s

	if (CurrentVelms > g_dbMaxSpeed)
		g_dbMaxSpeed = CurrentVelms;

	/*
	 * ���ؼӵ� �����̸� ������ �˻����� ����
	 */
	if (CurrentVelms < SPEED_BOUND_SEARCH)
	{
		//printf("\n�˻����ؼӵ� �����Դϴ�. ����ӵ�[%f]\n", CurrentVelms);
		return CANNOT_FIND_NEW_STATION;
	}
	
	// ���� ��ġ ��� ��	
	//--------------------------------------------------	

	// �� ��ġ ��� ���� ����� ������ ������ ��´�.
	GetShortestStaOrder(EnuPos[0], EnuPos[1], &nMinOrder, &dbDis);
	
//	printf("\n���� ����� ������ ����:%d, �Ÿ�:%f \n", nMinOrder, dbDis);

	
	nCurStaOrder = g_nCurStaOrder;
	nNextStaOrder = nCurStaOrder + 1;
	if (nNextStaOrder > g_nTotalStaCnt)
		nNextStaOrder = 1;

	// ���� ������� �� ��ġ ������ �Ÿ� ���
	dbCurStaOrderDis = GetDis(g_stBusStation[nCurStaOrder-1].ENU[0], 
							  g_stBusStation[nCurStaOrder-1].ENU[1], 
							  EnuPos[0], EnuPos[1]);
			
	// ���� ������� �� ��ġ ������ �Ÿ� ���
	dbNextStaOrderDis = GetDis(g_stBusStation[nNextStaOrder-1].ENU[0], 
								g_stBusStation[nNextStaOrder-1].ENU[1], 
								EnuPos[0], EnuPos[1]);
	
#ifdef	_DEBUGGPS_	
	printf("\n ���� ��ġ ��ǥ : x[%f], y[%f] \n", EnuPos[0], EnuPos[1]);
	printf("\n �� ������ ���� : %d \n", nCurStaOrder);
	printf("\n �� ������ ���� ��ǥ : x[%f], y[%f] \n", g_stBusStation[nCurStaOrder-1].ENU[0],
			g_stBusStation[nCurStaOrder-1].ENU[1]);
	printf("\n �� ��������� �Ÿ� : %f \n", dbCurStaOrderDis);
	printf("\n ���� ��������� �Ÿ� : %f \n", dbNextStaOrderDis);
#endif	

	/*
	 * ����Ÿ�����, �ƴ���
	 */
	if (IsOverDistance(nCurStaOrder, dbNextStaOrderDis) == 1)
	{
		g_gpsStatusFlag = DIST_OVER ; // �Ÿ� �̻����� �Ǹ�
	}
	
	// ������ ���� ���θ� �ǵ��Ѵ�.
	nRecogFlag = RecognizeStation(dbCurStaOrderDis, dbNextStaOrderDis, dbDis,
									nMinOrder, nCurStaOrder, nNextStaOrder);

#ifdef	_DEBUGGPS_		
	printf("\n �ǵ��� ������ ���� : %d \n", nRecogFlag);
#endif

	if (nRecogFlag >= 1)	{
		// ������ �������� ���� �ν��ߴ��� �ǵ��Ѵ�.
		nDecidedFlag = DecideStation(nRecogFlag, dbDis);
		
		if (nDecidedFlag > 0)	{
			g_nPrevStaOrder = g_nCurStaOrder;	// �� ������ ������ ���������尪��
			g_nCurStaOrder = nDecidedFlag;
			g_nPassMinDis = (int)dbDis;
			
			//printf("\n g_nPrevStaOrder:%d, g_nCurStaOrder:%d\n",g_nPrevStaOrder, g_nCurStaOrder);
			// �ν��� ������ �������� �α׿� ����Ѵ�.
			WriteLog();

			// ���� �ν��� ������ ������ �����Ѵ�.
			SaveCurrentStation(nDecidedFlag, 1);

			memcpy(stFoundStaInfo->station_id, g_stStaInfo[nDecidedFlag-1].abStationID, 7);
			
			// �ν����� ���� �ð��� �����Ѵ�
			memcpy(g_szStaFindTime, szCurrTime, 14);
		}
	}
	return nDecidedFlag;
}

/******************************************************************************
*                                                                             *
*    �Լ��� : GetShortestStaOrder()                                        	  *
*                                                                             *
*      ���� : [����ġ ��� ���� ����� ������ ���� �˻�]  			  		  *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE          DESCRIPTION             	  *
*             ------  ----------  --------      ----------------------- 	  *
*			  IN	  curX  	  double 		�� ��ġ X��ǥ     			  *
*			  IN	  curY		  double		�� ��ġ Y��ǥ				  *
*			  IN/OUT  nMinOrder	  int *			�ּ� �Ÿ� ������ ����		  *
*			  IN/OUT  dbDis		  double *		�ּ� �Ÿ�					  *
*    ���ϰ� : ����.						                      				  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
void GetShortestStaOrder(double curX, double curY, int *nMinOrder, double *dbDis)
{
	int i=0;
	int nFlag=0;	// X, Y �ݰ� ���ֿ� �ִ� �� Ȯ�� 1:X �ݰ游, 2:Y �ݰ游, 3:X, Y �Ѵ�
	double dbMaxX, dbMinX;
	double dbMaxY, dbMinY;
	double dbTmpX, dbTmpY;
	double dbMinDis = 9999999;


	*dbDis = 0;
	*nMinOrder = 0;
	
	dbMaxX = curX + g_dbRoundX;
	dbMinX = curX - g_dbRoundX;
	dbMaxY = curY + g_dbRoundY;
	dbMinY = curY - g_dbRoundY;
#ifdef	_DEBUGGPS_
	printf("curX:[%f], dbMaxX:[%f], dbMinX:[%f]\n", curX, dbMaxX, dbMinX);
	printf("curY:[%f], dbMaxY:[%f], dbMinY:[%f]\n", curY, dbMaxY, dbMinY);
#endif	

	
	for (i=0; i<g_nTotalStaCnt; i++)	{
		// �ݰ�ȿ� �ִ� ���������� �˻�
#ifdef	_DEBUGGPS_
		printf("g_stSortX[%d].dbX = %f\n", i, g_stSortX[i].dbX);
		printf("g_stSortY[%d].dbY = %f\n", i, g_stSortY[i].dbY);		
#endif
		if (g_stSortX[i].dbX < dbMaxX && g_stSortX[i].dbX > dbMinX)	{

			dbTmpX=g_stBusStation[g_stSortX[i].nOrder-1].ENU[0];
			dbTmpY=g_stBusStation[g_stSortX[i].nOrder-1].ENU[1];

#ifdef	_DEBUGGPS_
		printf("g_stSortX[%d].nOrder = %d\n", i, g_stSortX[i].nOrder);
		printf("dbTmpX:[%f], dbTmpY:[%f]\n", dbTmpX, dbTmpY);		
#endif
			
			// ���־ȿ� �ִ� ������� ���� ��ġ���� �Ÿ��� ���� �ּҰ��� ��
			*dbDis=sqrt((dbTmpX-curX)*(dbTmpX-curX)+(dbTmpY-curY)*(dbTmpY-curY));
			if (*dbDis < dbMinDis)	{
				dbMinDis = *dbDis;
				*nMinOrder = g_stSortX[i].nOrder;
			}
			nFlag = 1;
		}
		if (g_stSortY[i].dbY < dbMaxY && g_stSortY[i].dbY > dbMinY)	{
			dbTmpX=g_stBusStation[g_stSortY[i].nOrder-1].ENU[0];
			dbTmpY=g_stBusStation[g_stSortY[i].nOrder-1].ENU[1];
			
			// ���־ȿ� �ִ� ������� ���� ��ġ���� �Ÿ��� ���� �ּҰ��� ��
			*dbDis=sqrt((dbTmpX-curX)*(dbTmpX-curX)+(dbTmpY-curY)*(dbTmpY-curY));
			if (*dbDis < dbMinDis)	{
				dbMinDis = *dbDis;
				*nMinOrder = g_stSortY[i].nOrder;
			}
			if (nFlag > 0)	nFlag = 3;
			else			nFlag = 2;
		}
		   
	}
	
	*dbDis = dbMinDis;
	
}

int SetInitStaOrder(int nMinOrder, double dbDis)
{
	int nInitStaOrder = 1;
	
	if (g_nContinueFlag == 1)
		nInitStaOrder = g_nPrevStaOrder+1;
	else
		nInitStaOrder = GetStartOrder(nMinOrder, dbDis);
	
	if (nInitStaOrder > g_nTotalStaCnt)
		nInitStaOrder = 1;
		
	return nInitStaOrder;
}


int GetStartOrder(int nMinOrder, double dbDis)
{
	int nStartOrder = 1;	// �⺻�� ����.
	
	if (dbDis <= 50) {
		if (nMinOrder == g_nPrevStaOrder)
			nStartOrder = g_nPrevStaOrder;
		else	{
			if (abs(nMinOrder-g_nPrevStaOrder) <= 3)
				nStartOrder = g_nPrevStaOrder;
			else
				nStartOrder = nMinOrder;
		}
	}
	
	return nStartOrder;
}

/******************************************************************************
*                                                                             *
*    �Լ��� : RecognizeStation()                                         	  *
*                                                                             *
*      ���� : [������ �������� �Ǵ�] 										  *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  		TYPE    DESCRIPTION             	  *
*             ------  ----------  		------  ----------------------- 	  *
*			  IN	  dbCurStaOrderDis  double 	���� ������� ����ġ �Ÿ� 	  *
*			  IN	  dbNextStaOrderDis	double	���� ������� ����ġ �Ÿ�	  *
*			  IN	  dbShortDis		double  ���� ����� ��������� �Ÿ�	  *
*			  IN	  nMinOrder			int		����ġ���� ���� ����� ������ *
*												����						  *
*			  IN	  nCurStaOrder		int		���� ������ ����			  *
*			  IN	  nNextStaOrder		int		���� ������ ����			  *
*    ���ϰ� : ������ ���� ���� .						                      *
*			  0 : ������													  *
*			  1�̻� : ������ ������ ����									  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
int RecognizeStation(double dbCurStaOrderDis, double dbNextStaOrderDis, 
					 double dbShortDis, int nMinOrder, int nCurStaOrder, 
					 int nNextStaOrder)
{
	int nRecogStaOrder = 0;
	int nEntryAngle	= 0;	// ���� ���� ���԰�
	int nEntryNextSta = 0;	// ���� ������ ���԰�
	int nEntryMinOrder = 0;	// ���� ����� �������� ���԰�
	int nVirCurStaOrder = 0;	// ���� ���μ������� �����ϴ� ������ ����

	// ���� ���� ���� ���� ���� ���� (0:������, 1�̻�:����)
	nRecogStaOrder = s_nRecogFlag;
	
	nEntryAngle = (int)s_stRMC.TrackTrue;
	nEntryNextSta = g_stBusStation[nNextStaOrder-1].Angle;
	nEntryMinOrder = g_stBusStation[nMinOrder-1].Angle;
#ifdef	_DEBUGGPS_
	printf("\n nRecogStaOrder:%d, nEntryAngle:%d, nEntryNextSta:%d, nEntryMinOrder:%d \n ",
			nRecogStaOrder, nEntryAngle, nEntryNextSta, nEntryMinOrder);
#endif	

	// ���� ��������� �Ÿ����� ���������� ���� �Ÿ��� �� ª�� ���
	if (dbCurStaOrderDis > dbNextStaOrderDis)	{
		s_nRecogFlag = nNextStaOrder;

	}
	// �� ��ġ��� ���� ����� �������� ���� �� ������������ �ƴ� ���
	if (nMinOrder != nCurStaOrder && nMinOrder != nNextStaOrder)	{
		if (dbShortDis <= SEARCH_ROUND_DIS / 2
			   && HeadingErrorABS_D(nEntryAngle, nEntryMinOrder) <= SEARCH_ROUND_ANGLE)
		{
				// ���� �˻��� ���� ���μ���
				printf("\n---- VirturalMode ----\n ");
				
				nVirCurStaOrder = VirturalDetectProcess(nMinOrder);
				
				printf("s_nVirDetectCnt : %d \n", s_nVirDetectCnt);
				
				// ������ ������ �°� ���� 3���� �������� ���� �Ǿ��ٸ�, 
				// ��������° �������� ���� ������ ������ ����.
				if (nVirCurStaOrder == 0)	{
					nRecogStaOrder = g_nNextStaOrder;
					s_nRecogFlag = nRecogStaOrder;
				}
				else
					return 0;
		}
	}

	nRecogStaOrder = s_nRecogFlag;
	
	return nRecogStaOrder;
}

/******************************************************************************
*                                                                             *
*    �Լ��� : VirturalDetectProcess()                                         *
*                                                                             *
*      ���� : [������ ������������ �ƴ� ���� �������� �ν� ���ǿ� ���� ��� *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  		TYPE    DESCRIPTION             	  *
*             ------  ----------  		------  ----------------------- 	  *
*			  IN	  nDectectStaOrder  int 	�˻��� ������ ���� 		  	  *
*    ���ϰ� : ������ Ȯ�� ���� .						                      *
*			  0 : Ȯ��														  *
*			  1�̻� : ���� ������ Ȯ��										  *
*			  -1 : ��Ȯ��													  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
int VirturalDetectProcess(int nDetectStaOrder)
{
	int ret = -1;
	
	if (s_nVirDetectCnt == 0)	{
		s_nVirDetectCnt = 1;
		s_nCurVirStaOrder = nDetectStaOrder;
		s_nPrevVirStaOrder = s_nCurVirStaOrder;
		
		return s_nCurVirStaOrder;
	}
	
	if (s_nVirDetectCnt <= 2)	{
		s_nCurVirStaOrder = nDetectStaOrder;
		if (abs(s_nPrevVirStaOrder - s_nCurVirStaOrder) == 1)	{
			s_nPrevVirStaOrder = s_nCurVirStaOrder;
			s_nVirDetectCnt = s_nVirDetectCnt + 1;
		}
		else if (s_nPrevVirStaOrder == s_nCurVirStaOrder)
			;
		else
			s_nVirDetectCnt = 0;
	}
	else if (s_nVirDetectCnt == 3)	{
		s_nVirDetectCnt = 0;
		s_nPrevVirStaOrder = 0;
		s_nCurVirStaOrder = nDetectStaOrder;
		g_nNextStaOrder = s_nCurVirStaOrder;
		ret = 0;
	}
	
	return ret;
}

/******************************************************************************
*                                                                             *
*    �Լ��� : DecideStation()                                         	  	  *
*                                                                             *
*      ���� : [������ �νĿ��� �Ǵ�] 										  *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  			TYPE    DESCRIPTION               *
*             ------  ----------  			------  -----------------------   *
*			  IN	  nRecogOrder  			int 	������ ������ ���� 	  	  *
*			  IN	  dbRecogStaOrderDis	double	���� ������� ����ġ �Ÿ� *
*    ���ϰ� : ������ ���� ���� .						                      *
*			  0 : ���ν�													  *
*			  1�̻� : �ν��� ������ ����									  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
int DecideStation(int nRecogOrder, double dbRecogStaOrderDis)
{
	int nDecidedFlag = 0;
	int nEntryAngle = 0;
	int nAngleRecog = 0;
	
	if (nRecogOrder == g_nCurStaOrder)	return nDecidedFlag;
	
	nAngleRecog = g_stBusStation[nRecogOrder-1].Angle;
	nEntryAngle = (int)s_stRMC.TrackTrue;	// ���� ���԰�
	
	if (dbRecogStaOrderDis <= SEARCH_ROUND_DIS 
		&& HeadingErrorABS_D(nEntryAngle,nAngleRecog)  <= SEARCH_ROUND_ANGLE)	{
		nDecidedFlag = nRecogOrder;
	}
	
	return nDecidedFlag;
}

/******************************************************************************
*                                                                             *
*    �Լ��� : SaveCurrentStation()                                         	  *
*                                                                             *
*      ���� : [���� �ν� ������ ���� ����] 									  *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  		TYPE    DESCRIPTION   				  *
*			  IN	  nDecidedOrder  	int 	�ν��� ������ ���� 	  		  *
*			  IN	  nContinueFlag		int		���� 1, �������� �� 0		  *
*    ���ϰ� : ����.					                      					  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
void SaveCurrentStation(int nDecidedOrder, int nContinueFlag)
{
	FILE*	file = NULL;
	int nSaveStaOrder;
//	int nContinueFlag = 1;

	nSaveStaOrder = nDecidedOrder;

	file = fopen(PREV_PASS_FILE, "wb+");
	
	printf("SaveCurStation \n");
	
	if (file != NULL)	
	{
		fwrite(&nSaveStaOrder, sizeof(int), 1, file); // ���� �ν� ������ ����
		fwrite(&nContinueFlag, sizeof(int), 1, file); // ���࿬�� ���� ����
		fwrite(g_szRunDepartTime, 14, 1, file);	// ������߽ð� ����
		fclose(file);
	}
	
	// ���� ����Ÿ�� ī��Ʈ �� ���
	g_nPrevAvailCnt = g_nAvailCnt;
	g_nPrevInvalidCnt = g_nInvalidCnt;
	g_nPrevTimeOutCnt = g_nTimeOutCnt;
	g_nPrevCheckSumCnt = g_nCheckSumCnt;
	
	GpsDataLogWrite();
}

void WriteLog()
{
	int i=0;
	int i_order = 0;
	char buff[11] = {0, };
	char strbuff[11] = {0, };		
	unsigned int write22_flag =0;		// ������ ���۽� 22 Set flag
	FILE *gpsFile2 = NULL;	
	
	unsigned int GPSDataCnt = 0;  //������ �ν��� ������� ���� �ν��� ����������� �� Data count�� ������
	unsigned int InvalidCnt = 0; // ������ �ν��� ������� ���� �ν��� ������ ������ invalid count 
	unsigned int AvailCnt = 0; //������ �ν��� ������� ���� �ν��� ������ ������ Avail count 
	unsigned int TimeOutCnt = 0; //������ �ν��� ������� ���� �ν��� ������ ������ Avail count
	double AvailErrorPerc = 0;   // �� ������ ���� �߿� valid count�� �ۼ�Ʈ
	
//	unsigned int InvalidCntbtST =0; 
//	unsigned int TimeOutCntbtST =0; 


	InvalidCnt = g_nInvalidCnt - g_nPrevInvalidCnt; 
	AvailCnt = g_nAvailCnt - g_nPrevAvailCnt;
	TimeOutCnt =(unsigned int)((g_nTimeOutCnt - g_nPrevTimeOutCnt)*2);
	GPSDataCnt = InvalidCnt + AvailCnt + TimeOutCnt; //���� ��ü ������ ����(check sum error�� ��� ���� ����)
	
	if(GPSDataCnt == 0)
		GPSDataCnt++;
		
	AvailErrorPerc = ((double)(AvailCnt) / (double)(GPSDataCnt)) *100.0;

	// ���� �ν��� ������ ������ �ӽ� ������ ����
	i_order = g_nCurStaOrder;

	memcpy(g_stGPS_INFO_STATION[i_order].szLineNum, g_szBusRouteID, 8);	// �����뼱 ID
	memcpy(g_stGPS_INFO_STATION[i_order].szBusID, g_szVehcID, 9);				// �������� ID
	memcpy(g_stGPS_INFO_STATION[i_order].szStartTime, g_szRunDepartTime,14);
	memcpy(g_stGPS_INFO_STATION[i_order].szOrder, 	g_stStaInfo[i_order-1].bStationOrder, 3);	// ������ ����
	memcpy(g_stGPS_INFO_STATION[i_order].szID, 		g_stStaInfo[i_order-1].abStationID, 7);	// ������ id
	memcpy(g_stGPS_INFO_STATION[i_order].szLongitude, g_szPassMinLongitude, 10);	// ����
	memcpy(g_stGPS_INFO_STATION[i_order].szLatitude,  g_szPassMinLatitude, 9);	// �浵
	sprintf(strbuff, "%3d", g_nPassMinDis);
	memcpy(g_stGPS_INFO_STATION[i_order].szPassDis,   strbuff, 3);		// �νİŸ�
	memcpy(g_stGPS_INFO_STATION[i_order].szPass, 	  "01", 2);					// ������� 
	memcpy(g_stGPS_INFO_STATION[i_order].szPassTime,  g_szGpsCurrTime, 14);		// ����ð�
	memcpy(g_stGPS_INFO_STATION[i_order].szSateCnt,   g_szPassSatelliteCnt, 2);	// ��������
	sprintf(strbuff, "%5d", g_nInvalidCnt);
	memcpy(g_stGPS_INFO_STATION[i_order].szIVcnt, strbuff, 5); // Invalid Cnt
	memset(strbuff, 0x00, 5); 
	sprintf(strbuff, "%5d", g_nAvailCnt);
	memcpy(&g_stGPS_INFO_STATION[i_order].szIVcnt[5], strbuff, 5); // Valid Cnt

	memset(g_stGPS_INFO_STATION[i_order].szErrorLog,  0x30, 19);		  
	memset(g_stGPS_INFO_STATION[i_order].szTemp,  0x20, 4);					
	memset(g_stGPS_INFO_STATION[i_order].szSerialNum,  0x20, 4);
	
/*	
	//�α׸� �������� �ڱ⺸�� ������ ���� ������ ���� �ʱ�ȭ 
	for (i=i_order+1; i<=g_nTotalStaCnt; i++)	
	{
		
		memcpy(g_stGPS_INFO_STATION[i].szLineNum, g_szBusRouteID, 8);	// �����뼱 ID
		memcpy(g_stGPS_INFO_STATION[i].szBusID, g_szVehcID, 9);				// �������� ID
		memset(g_stGPS_INFO_STATION[i].szStartTime,0x20,14);
		memcpy(g_stGPS_INFO_STATION[i].szOrder, 	g_stStaInfo[i].bStationOrder, 3);	// ������ ����
		memcpy(g_stGPS_INFO_STATION[i].szID, 		g_stStaInfo[i].abStationID, 7);	// ������ id
		memset(g_stGPS_INFO_STATION[i].szLongitude, 0x20, 10);						// ����
		memset(g_stGPS_INFO_STATION[i].szLatitude,  0x20, 9);							// �浵
		memset(g_stGPS_INFO_STATION[i].szPassDis,   0x20, 3);							// ����ּҰŸ�
		memset(g_stGPS_INFO_STATION[i].szPass, 	  0x20, 2);							// ������� 
		memset(g_stGPS_INFO_STATION[i].szPassTime,  0x20, 14);						// ����ð�
		memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);							// ��������
		memset(g_stGPS_INFO_STATION[i].szIVcnt,	  0x20, 10);		
		memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);		  
		memset(g_stGPS_INFO_STATION[i].szTemp,  0x20, 4);					
		memset(g_stGPS_INFO_STATION[i].szSerialNum,  0x20, 4);
	}
*/	
/*
	//���� ���� �ð��� �ҷ� �´�. 
	if (i_order != 0)
	{
		gpsFile2 = fopen(GPS_INFO_FILE2, "r");
		
		if(gpsFile2 != NULL)
		{
			fread(buffer,SIMXINFO_SIZE,1,gpsFile2);
			memcpy(szStartTimebuff,&buffer[17],14);
			fclose(gpsFile2);
		}
	}
*/	

	printf("i_order = [%d], g_nPrevStaOrder = [%d]", i_order, g_nPrevStaOrder);
	///////////////////////�߰��� ���� ��Ȳ �߻��� ���� �ڵ� ä��/////////////////////////////
	// �߰��� Jump ���� ����
	if( i_order > g_nPrevStaOrder+1)
	{
		printf("\n�߰��� ������ ���\n");
		//2005-02-18 7:21����
		for(i = g_nPrevStaOrder+1; i <= i_order-1; i++)
		{
			if(AvailErrorPerc < AVAIL_CNT_TH)//Avail count�� 30%�����̸� 88�̳� 99�� ī��Ʈ�� �������� ���´�. 
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
					printf("\n 99 ��� 1 \n");
				}
				
			}
			else//  Avail count�� 30%�̻��̸� 66�̳� �ٸ� ������ ��´�. 
			{
				//memcpy(g_stGPS_INFO_STATION[i].szPass,g_stGPS_INFO_STATION[i_order].szTemp, 2);
				//memcpy(g_stGPS_INFO_STATION[i].szTemp,g_stGPS_INFO_STATION[i_order].szTemp, 2);
				memcpy(g_stGPS_INFO_STATION[i].szPass, "66", 2);
				memcpy(g_stGPS_INFO_STATION[i].szTemp, "66", 2);
			}

			memcpy(g_stGPS_INFO_STATION[i].szStartTime,g_szRunDepartTime,14);
			//������ �ʵ�� ��� �ʱ�ȭ (�����̽� �ٷ�)
			memset(g_stGPS_INFO_STATION[i].szIVcnt,0x20, 10);
			memset(g_stGPS_INFO_STATION[i].szPassTime, 0x20, 14);
			memset(g_stGPS_INFO_STATION[i].szLongitude,0x20, 10);
			memset(g_stGPS_INFO_STATION[i].szLatitude, 0x20, 9);
			memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);
			memset(g_stGPS_INFO_STATION[i].szPassDis,   0x20, 3);
			memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);		
		}

	}
	
	// ���� �αٿ��� Jump ���� ���
	if( i_order < g_nPrevStaOrder)
	{
		printf("\n���� �αٿ��� �����Ѱ��\n");
		//2005-02-18 7:21����
		for(i = g_nPrevStaOrder + 1; i<= g_nTotalStaCnt;i++)
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
					printf("\n 99 ��� 2 \n");
				}
				
			}
			else // 66 �̳� �ٸ� ���� 
			{
	
				// �߰��� �ý����� ���� ȸ�� ���� �������� �ٽ� �ý����� ���� ��쿡 ���
				//if( memcmp(g_stGPS_INFO_STATION[i_order].szTemp, "  ", 2) == 0 )
				//{
					memcpy(g_stGPS_INFO_STATION[i].szPass, "66", 2);
					memcpy(g_stGPS_INFO_STATION[i].szTemp, "66", 2);
				//}
				//else
				//{
				//	memcpy(g_stGPS_INFO_STATION[i].szPass,g_stGPS_INFO_STATION[i_order].szTemp, 2);
				//	memcpy(g_stGPS_INFO_STATION[i].szTemp,g_stGPS_INFO_STATION[i_order].szTemp, 2);
				//}
			}	
			memcpy(g_stGPS_INFO_STATION[i].szStartTime,g_szRunDepartTime,14);
			//������ �ʵ�� ��� �ʱ�ȭ (�����̽� �ٷ�)
			memset(g_stGPS_INFO_STATION[i].szIVcnt,0x20, 10);
			memset(g_stGPS_INFO_STATION[i].szPassTime, 0x20, 14);
			memset(g_stGPS_INFO_STATION[i].szLongitude,0x20, 10);
			memset(g_stGPS_INFO_STATION[i].szLatitude, 0x20, 9);
			memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);
			memset(g_stGPS_INFO_STATION[i].szPassDis,   0x20, 3);
			memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);							
				
		}
		for(i =1; i<= i_order-1;i++)
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
					printf("\n 99 ��� 3 \n");			
				}
				
			}
			else // 66 �̳� �ٸ� ���� 
			{
				
				//memcpy(g_stGPS_INFO_STATION[i].szPass,g_stGPS_INFO_STATION[i_order].szTemp, 2);
				//memcpy(g_stGPS_INFO_STATION[i].szTemp,g_stGPS_INFO_STATION[i_order].szTemp, 2);
				memcpy(g_stGPS_INFO_STATION[i].szPass, "66", 2);
				memcpy(g_stGPS_INFO_STATION[i].szTemp, "66", 2);

			}			
			memcpy(g_stGPS_INFO_STATION[i].szStartTime,g_szRunDepartTime,14);

			//������ �ʵ�� ��� �ʱ�ȭ (�����̽� �ٷ�)
			memset(g_stGPS_INFO_STATION[i].szIVcnt,0x20, 10);
			memset(g_stGPS_INFO_STATION[i].szPassTime, 0x20, 14);
			memset(g_stGPS_INFO_STATION[i].szLongitude,0x20, 10);
			memset(g_stGPS_INFO_STATION[i].szLatitude, 0x20, 9);
			memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);
			memset(g_stGPS_INFO_STATION[i].szPassDis,   0x20, 3);
			memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);								
				
		}		

	}


	// ���������� �������� �ν�������� szPass '01'�� Set
	if(i_order != 0)
	{
		printf("\n ������ %d ��  '01' ���\n", i_order);
		memcpy(g_stGPS_INFO_STATION[i_order].szPass, "01", 2);
		memcpy(g_stGPS_INFO_STATION[i_order].szStartTime,g_szRunDepartTime,14);
	}
	////////////////////////////////////////////////////////////////////////////////////////
	
	memcpy(g_stGPS_INFO_STATION[i_order].szSateCnt, 	g_szPassSatelliteCnt, 2);
//	memcpy(g_stGPS_INFO_STATION[i_order].szPassTime, 	szPassMinTime, 14);	ver03.33 ���� ����
	// �ܸ��� �ð��� �ƴ� gps ��� �ð����� ���
	memcpy(g_stGPS_INFO_STATION[i_order].szPassTime, 	g_szGpsCurrTime, 14);	
																		// 03.34 ���� 2005-06-08 5:59����

	sprintf(buff, "%03d", g_nPassMinDis);
	memcpy(g_stGPS_INFO_STATION[i_order].szPassDis, 	buff, 3);

	memset(g_stGPS_INFO_STATION[0].szLongitude,0x20, 10);	
	memset(g_stGPS_INFO_STATION[0].szLatitude,0x20, 9);	
	memset(g_stGPS_INFO_STATION[0].szPassDis,0x20, 3);
	memcpy(g_stGPS_INFO_STATION[0].szPassTime,g_stGPS_INFO_STATION[0].szStartTime, 14);

	
	
	//���Ͽ� ��� �Ѵ�. 

	//gpsFile = fopen(GPS_INFO_FILE1, "ab+");
	gpsFile2 = fopen(GPS_INFO_FILE2,"ab+");
	
	if(gpsFile2 != NULL)
	{
	//ó���� �ν������� ���� �����忡 ���ؼ� ���Ͽ� ���

		if(i_order != 0)
		{
			// ������ ��߽�
			if(g_nFirstRun == 0)
			{
				printf("\n������� �� ù �ν��� ���\n");
				if(i_order != 1)
				{
					// ó���� ���� �ν��� �������� 2 �ΰ�� ù��° ������ �νķα׸� ����Ѵ�.
					if(i_order == 2)
					{
						memcpy(g_stGPS_INFO_STATION[1].szPass, "01", 2);
						memset(g_stGPS_INFO_STATION[1].szTemp, 0x20, 2);
						memcpy(g_stGPS_INFO_STATION[1].szIVcnt, g_stGPS_INFO_STATION[2].szIVcnt, 10);
						memcpy(g_stGPS_INFO_STATION[1].szPassTime,g_stGPS_INFO_STATION[2].szPassTime, 14);
						memcpy(g_stGPS_INFO_STATION[1].szLongitude,g_stGPS_INFO_STATION[2].szLongitude, 10);
						memcpy(g_stGPS_INFO_STATION[1].szStartTime, g_stGPS_INFO_STATION[2].szStartTime,14);
							memcpy(g_stGPS_INFO_STATION[1].szLatitude, g_stGPS_INFO_STATION[2].szLatitude, 9);
						
						fwrite(&g_stGPS_INFO_STATION[1], SIMXINFO_SIZE, 1, gpsFile2);
						write22_flag =1;

					}
					else	// ������ ��߽ÿ� �������� ���� order���� '22'�� Set
					{
						for(i = 1; i < i_order ; i++) //2005-02-18 7:21����
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
									printf("\n 99 ��� 5 \n");			
								}
								
							}
							else
							{
								memcpy(g_stGPS_INFO_STATION[i].szPass, "22", 2);
								memcpy(g_stGPS_INFO_STATION[i].szTemp, "22", 2);
							}
							memset(g_stGPS_INFO_STATION[i].szIVcnt,0x20, 10);
							memset(g_stGPS_INFO_STATION[i].szPassDis,0x20, 3);
							memset(g_stGPS_INFO_STATION[i].szPassTime, 0x20, 14);
							memset(g_stGPS_INFO_STATION[i].szLongitude,0x20, 10);
							memcpy(g_stGPS_INFO_STATION[i].szStartTime, g_szRunDepartTime,14);	
							memset(g_stGPS_INFO_STATION[i].szLatitude,  0x20, 9);	
							memset(g_stGPS_INFO_STATION[i].szSateCnt,   0x20, 2);	
							fwrite(&g_stGPS_INFO_STATION[i], SIMXINFO_SIZE, 1, gpsFile2);
							memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);


						}
						write22_flag = 1;
					}
				}
				g_nFirstRun = 1;
			}
		}
		
		// 2005-01-18 12:49���� ������ ��ġ�� �������� ���� ��쿡 ���� �α� ó�� 
		// ������ƽ�� ó��.
		///////////////////////�߰��� ���� �������� ����Ѵ�. ////////////////////////////////
		// �ý��� ó�� ������ ��찡 �ƴϸ�
		if(write22_flag != 1)
		{
			// �߰��� Jump ���� ����� �����α� ���� ���
			if( i_order > g_nPrevStaOrder)
			{
				for(i = g_nPrevStaOrder+1; i <= i_order-1; i++)
				{

					fwrite(&g_stGPS_INFO_STATION[i], SIMXINFO_SIZE, 1, gpsFile2);

					memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);
				}
				
			}
			// ���� �αٿ��� Jump ���� ����� �����α� ���� ���
			if( i_order < g_nPrevStaOrder)
			{
				
				for(i = g_nPrevStaOrder + 1; i<= g_nTotalStaCnt;i++)
				{
					fwrite(&g_stGPS_INFO_STATION[i], SIMXINFO_SIZE, 1, gpsFile2);	

					memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);
				}
				for(i =1; i<= i_order-1;i++)
				{

					fwrite(&g_stGPS_INFO_STATION[i], SIMXINFO_SIZE, 1, gpsFile2);

					memset(g_stGPS_INFO_STATION[i].szErrorLog,  0x30, 19);
				}
			}
		}
		///////////////////////////////////////////////////////////////////////////////////


		//���� �ν��� ������ �� ��� �Ѵ�. 
		//printf("\n %d ��° ������ �ν� �α� ��� \n [%s] \n", i_order, &g_stGPS_INFO_STATION[i_order]);

		// ���������� �ν����� ��� �����α� ���Ͽ� ����Ѵ�.
		fwrite(&g_stGPS_INFO_STATION[i_order], SIMXINFO_SIZE, 1, gpsFile2);
		
		memset(g_stGPS_INFO_STATION[i_order].szErrorLog,  0x30, 19);

		fflush(gpsFile2);
		fclose(gpsFile2);
	}


}

int	GetStation(void)
{
	int i_order = 0; //�˻��� ������ ����
	double dis_z1 = 0.; // �˻��� ������ ���� �Ÿ�
	int bPassStation = FALSE;	// �������� �ν�����. ǥ��	
	
	////////////////////////////////////
	//���� ���� ������ ���� ����
	BusStationDetect(&g_stBusStation[0], &BusInfo, &s_stRMC, 
	                 &g_stGGA, &coordinates, g_nPrevStaOrder);

	// �˻��� ������ ������ 1���� �����Ѵ�
 	i_order = BusInfo.CurrentStationOrder;
	dis_z1 = BusInfo.CurrentStationDist;	

 	switch (BusInfo.Status)	{
 		case INIT_STATION_SEARCH : 
 			ErrorLogCheck(BusInfo.CurrentStationOrder, 1);		
			bPassStation = TRUE;				
 			break;
		case INIT_STATION_SEARCH_COMPLETE1 : 
			ErrorLogCheck(BusInfo.CurrentStationOrder, 2);	
			bPassStation = TRUE;				
			break;						
		case INIT_STATION_SEARCH_COMPLETE2 : 
			ErrorLogCheck(BusInfo.CurrentStationOrder, 3);
			bPassStation = TRUE;
			break;
		case NEXT_STATION_SEARCH :
			ErrorLogCheck(BusInfo.CurrentStationOrder, 4);		
			bPassStation = TRUE;		
			break;						
		case STATION_VERIFICATION :
			ErrorLogCheck(BusInfo.CurrentStationOrder, 5);
			bPassStation = TRUE;
			break;
		case NEXT_STATION_UPDATE_COMPLETE1 :
			ErrorLogCheck(BusInfo.CurrentStationOrder, 6);
			memcpy(stGPS_INFO_STATION[i_order].szPass, "01", 2);
			memcpy(stGPS_INFO_STATION[i_order].szTemp, "55", 2);				
			bPassStation = TRUE;
			break;
		case NEXT_STATION_UPDATE_COMPLETE2 :
			ErrorLogCheck(BusInfo.CurrentStationOrder, 7);	
			bPassStation = TRUE;
			break;					
		case NEXT_STATION_UPDATE_COMPLETE3 :
			ErrorLogCheck(BusInfo.CurrentStationOrder, 8);	
			bPassStation = TRUE;
			break;
		case STATION_DETECTION_FAIL : 
			ErrorLogCheck(BusInfo.CurrentStationOrder, 9);		
			memcpy(stGPS_INFO_STATION[i_order].szPass, "01", 2);
			
			if((memcmp(stGPS_INFO_STATION[g_nPrevStaOrder].szTemp, "88", 2)!= 0) && (memcmp(stGPS_INFO_STATION[g_nPrevStaOrder].szTemp, "99", 2)!= 0))
			{
				memcpy(stGPS_INFO_STATION[i_order].szTemp, "66", 2);
			}
				
			bPassStation = TRUE;
			break;
		case GPS_INVALID :					
			ErrorLogCheck(BusInfo.CurrentStationOrder, 10);
			break;					
		case GPS_DOP_ERROR :		
			ErrorLogCheck(BusInfo.CurrentStationOrder, 11);
			break;						
		case GPS_INVALID_TIMEOUT :
			ErrorLogCheck(BusInfo.CurrentStationOrder, 14);	
			break;						
		case INIT_DB_ERROR:
			break;
		default:
			bPassStation = TRUE;
 	}

 	if (i_order < 1)	
	{
		return 0;
	}

	
	// ������ ������ ���� �ν��� �Ͱ� ����, ������ ���̵� ������ ����
	if (g_nPrevStaOrder == i_order)
	{
		return 0;
	}	

	if (bPassStation)  
	{

//		nOldLastSuccessOrder = nLastSuccessOrder;
	 	g_nPrevStaOrder = i_order;

		SaveCurrentStation(i_order, 1);

		printf("\n������ �ν��� ������ %d \n",i_order);

		g_nPassMinDis = dis_z1;

		if(InitStationIDFixFlag == 1)
		{
			g_nCurStaOrder = i_order;
			WriteLog();	// gps �α� ���� ����..

			g_n2PrevInvalidCnt = g_nPrevInvalidCnt;
			g_n2PrevAvailCnt = g_nPrevAvailCnt;
			g_n2PrevTimeOutCnt = g_nPrevTimeOutCnt;
			//�α� ���� �����Ŀ� ���� invalid count���� valid count���� ���� ���� ���� �Ѵ�.
			//�α� ������ �����ٴ� �ǹ̴� ���� �������� ���� �ߴٴ� �ǹ�  
			g_nPrevInvalidCnt = g_nInvalidCnt;	//2005-02-18 4:34����	
			g_nPrevAvailCnt = g_nAvailCnt;  //2005-02-18 4:34����
			g_nPrevTimeOutCnt = g_nTimeOutCnt; // 2005-02-18 6:19����

		}
	
    }
    if (i_order<1) i_order = 0;

//	memcpy( gpstSharedInfo->abNowStationID, gpstStationInfo[i_order-1].abStationID, 7 );
//	memcpy( gpstSharedInfo->abNowStationName, gpstStationInfo[i_order-1].abStationName, 16 );
//	memcpy( gabGPSStationID, gpstStationInfo[i_order-1].abStationID, 7 );
	
	return i_order;
}

void CheckingCurrStation(void)
{
	int i=0;
	int nFoundSta[10] = {0,};
	int nFoundOrder = 0;
	int nGab = 0;
	FOUND_STA_INFO stFoundStaInfo;
	

	/*
	 * ����������ڸ��� ���� ���� ��ġ�� �˻��Ѵ�.
	 * 5�� �ݺ��Ͽ� ���� ���� ������ ������ ������ ���Ѵ�
	 */
	for (i=0; i<5; i++)
	{
		GpsDataProcessing();
		if (g_nTotalStaCnt > 0 && g_gpsStatusFlag == GPS_STATUS_OK)
		{
			nFoundSta[i] = BusStationDetecting(&stFoundStaInfo, s_stRMC.Lat, 
		                                 s_stRMC.Lon, s_stRMC.UTCTime, 
		                                 s_stRMC.SpeedKn);
		}
	}

	nFoundOrder = GetConstantlyNumber(nFoundSta, 5);

	nGab = abs(g_nPrevStaOrder - nFoundOrder);
	
	if (g_nPrevStaOrder == nFoundOrder || nFoundOrder == 0)
		return;

	if (nFoundOrder == 1)	{
		g_nPrevStaOrder = 1;
		g_nCurStaOrder = 1;
	}
	else if (g_nPrevStaOrder < 4 || g_nTotalStaCnt-3 < g_nPrevStaOrder) {
		g_nPrevStaOrder = 1;
		g_nCurStaOrder = 1;
	}
	else if ( nGab < g_nTotalStaCnt-3 && nGab > 3 ) {
		// �������� Ȯ���� �ֽñ� �ٶ��ϴ� ���
		VoiceOut( 45 );	
	}
	else
		;
	
}

// ������� ������ ���̿� �߻��� �ν� �� ���� �Ǵ� GPS ����, �ʱ� DB ������ ���� Flag �����Լ�
void ErrorLogCheck(unsigned int Order, unsigned int Index)
{
//	char strbuff[5];
	memcpy(&stGPS_INFO_STATION[Order].szErrorLog[Index-1], "1", 1);

}