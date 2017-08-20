#ifndef _GPS_
#define _GPS_

#include "simx.h"
#include "gps_env.h"


// ������ �߰� : ������ ��� ���� ����
#define GPS_INFO_FILE	"gps_info.dat"
#define GPS_INFO_FILE1	"simxinfo.dat"
#define GPS_INFO_FILE2  "simxinfo2.tmp"
#define GPS_ENVS		"gps_envs.dat"

// ���� ���� ȸ�� ������ ���ϸ��
#define TURN_CNT_FILE		"turn_cnt.dat"	// �Ϸ� �������� ������ ������ Ƚ��.
#define LAST_PASS_FILE		"lastpass.dat"	// ������ ����� ������ ���� ����.
#define PREV_PASS_FILE		"prevpass.dat"	// ���� �ν� ������ ���� -- GPS 3.0
#define DRIVE_DIST_TMP		"drivedis.tmp"
#define DRIVE_DIST_FILE		"drivedis.dat"
#define GPS_INFO_BACKUP		"gps_info.bak"
#define LOG_INFO_FILE		"simxlog.dat"	// GPS �����忡�� �߻��ϴ� ������ ���� ��� ����� ��.
#define RAW_DATA_LOG		"raw_data.log"	// GPS ���� ����Ÿ�� �ܰ躰 ī��Ʈ ����
#define GPS_INFO_TMP		"gps_info.tmp"	// ���� ���ϴ¿� ���Ϲ���
#define GPS_RAW_FILE		"gps_raw.txt"	// �ùķ��̼ǿ� ���ϸ�
#define DRIVE_INFO	        "drive_info.txt"


int GetGpsData(int fd, char* strGGA, char* strRMC);
int ParseRMC(RMC_DATA* rmc, unsigned char *Data, int DataNum);
void WriteTrace(RMC_DATA* rmc);
//extern driveFlag; //���� ���� �ð� ���� �Ϸ� �÷��� '1' : ���� ���� �ð��� ���� , '0' : ���� ���� �ð��� ��� ���� 

extern int gpsRecv(int fd, unsigned char* buffer, int size, int timeout);
extern int gpsPktRecv(int fd, int size, int timeout);
extern int gpsOpen(int baudrate);
extern void *gpsMain(void *arg);
extern int gpstime_check(void);
extern void gpstime_convert (char *gps_time);
extern int GpsTimeCheck(void);
int ParseGGA(GGA_DATA* gga, unsigned char *Data, int DataNum);


extern GPS_DATA GPS;
extern GPS_INFO_STATION g_stGPS_INFO_STATION[MAXSTATIONCNT];

extern STA_INFO_LOAD	STA_IF_LOAD[256];

extern unsigned int g_nSelectFailCnt;	// Select Fail Count
extern unsigned int g_nTimeOutCnt;		// TimeOut	Count
extern unsigned int g_nCheckSumCnt;		// CheckSum Error Count
extern unsigned int g_nInvalidCnt;		// Invalid Count	'V'
extern unsigned int g_nAvailCnt;		// Avail Count	'A'

extern unsigned int g_nPrevInvalidCnt;	// Previous invalid count
extern unsigned int g_nPrevAvailCnt;     // previous avail count
extern unsigned int g_nPrevTimeOutCnt;   // previous time out count
extern unsigned int g_nPrevCheckSumCnt;	// previous checksum count

extern unsigned int g_n2PrevInvalidCnt;      // 
extern unsigned int g_n2PrevAvailCnt;        //
extern unsigned int g_n2PrevTimeOutCnt;

extern GPS_STATUS_CNT g_stGpsStatusCnt[MAXSTATIONCNT];

extern int g_nTotalStaCnt;		// ������ �� ��
extern int g_nContinueFlag;		// ���� Ų ��� ���࿬���ΰ�(1), �ƴѰ�(0)?


extern int g_nPassMinDis;				// ����������� �ּҰŸ�
extern char g_szPassMinLongitude[10];	
extern char g_szPassMinLatitude[9];	

extern char g_szVehcID[10];	// ���� id
extern char g_szBusRouteID[9];	// �뼱 id
	
extern char g_szPassMinTime[15];			
extern char g_szPassSatelliteCnt[3];	

extern int g_nNextStaOrder;
extern int g_nPrevStaOrder;
extern int g_nCurStaOrder;


extern char szGpsLogStartTime[15];	
extern char szGpsLogEndTime[15];	
extern char g_szRunDepartTime[15];	// ������۽ð�
extern int g_gps_fd;				// gps ��Ʈ �ڵ�
extern int g_bInc;
extern int g_nFirstRun;	// ��������� ������ �˻��� ÷����?

//GPS 2.7 ����. ��⿡�� ������ �ð� (�ܸ��� �ð� �ƴ�.)
extern char g_szGpsCurrTime[15];

extern int g_nGpsExceptCnt;	// ������ ����� ������ �� ��

extern byte g_gpsStatusFlag; // ���� gps ���Ż��� �÷��� ����

extern double g_dbMaxSpeed;	// ������� �� ������ �ִ� �ӵ���

extern int	nMoveDis;

// ����, 2.7 ��� �����ϱ� ���� �߰� ����
extern COMM_STA_INFO g_stStaInfo[MAXSTATIONCNT];

extern BUS_STATION	g_stBusStation[MAXSTATIONCNT];
extern double g_dbRoundX;	// ������ �˻� �ݰ� X'
extern double g_dbRoundY;	// ������ �˻� �ݰ� Y'

extern SORT_X	g_stSortX[MAXSTATIONCNT];
extern SORT_Y	g_stSortY[MAXSTATIONCNT];

extern RMC_DATA	s_stRMC;				// RMC ������ ���� ����ü

extern BUSINFO			BusInfo;				// ������ �ν� ���� ���� ����ü(BusStationDetect�Լ��� ���ϰ�)

extern RMC_DATA		s_stRMC;				// RMC ������ ���� ����ü
extern GGA_DATA		g_stGGA;				// GGA ������ ���� ����ü
extern COORDINATES		coordinates;			// ��ǥ��ȯ ����ü
extern GPS_INFO_STATION stGPS_INFO_STATION[MAXSTATIONCNT];

extern char g_szStaFindTime[15];	// ���� ������ �ν� �ð�

#endif
