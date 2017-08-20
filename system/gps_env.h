#ifndef _GPS_ENV_
#define _GPS_ENV_




#define LOG_MAX_SIZE	2097152L	// �α����� �ִ� ũ��

#define recvData_size 	1024
#define GPS_BPS_SPEED	9600

#define LINE_MAX_BUFFER		255		// ���۶��� �ִ밪
#define GPSTIMEOUT			2		// GPS Ÿ�Ӿƿ� �ð� ��
#define MAXSTATIONCNT		256		// �ִ� �ѳ뼱�� ������ ��
#define SIMXINFO_SIZE		118		// SIMXINFO �α� ���� ���ڵ� ������
#define MaxFileSize (1024*1024)  	// simx log file size
#define AVAIL_CNT_TH		30.0   	// avail count threshold
#define SEARCH_ROUND_VALUE	5		// ������ �˻� �ݰ� ���� ���ϱ� ���� �����
#define TIME_CHECK_GAP		10		// �ð� ���� ����

#define SEARCH_ROUND_DIS	50
#define SEARCH_ROUND_ANGLE	40

#define GPS_STATUS_OK		0
#define GPS_INVALID_DATA	1
#define GPS_CONN_OUT		2
#define DIST_OVER			3

#define MAX_BOUND_HDOP		4.5		// ���� ����Ÿ �ŷڼ� Ȯ���� ����

#define TIME_BOUND_SEARCH	100	// ���� �������� �˻��ϱ���� ��ٸ��� �ִ�ð�
								// ���� ��������� �Ÿ��� �������� �̵��ð��� �� �ӵ���� ����
								
#define SPEED_BOUND_SEARCH	0	// �ʼ� 5 m/s �̻�Ǿ�� ������ �˻��� ��
#define DEFAULT_MAX_SPEED		5	// �ʼ� 5 m/s ���ؼӵ���
#define GPS_STATUS_VALID_BOUND	40	// GPS ���� ������ �Ǵ� ���ذ�
#define GPS_STA_CNT_REF_BOUND	3	// �ֱ� 3���� �������� �ν����� ���� ���� ���ź� ���� ��

#define DISTANCE_MAX_TIMES	3	// ���ذŸ� ���� ��� �̻� ��������?

#define GPS_DEV_NAME		"/dev/ttyE1"
#define BAUD_RATE			9600

#define CANNOT_FIND_NEW_STATION	0

/////////////////////////////////////////////////////////////////////////
// 2004.09.25
// ������ �߰�
// ���� ����Ÿ� ������
// ���ϸ� : drivedis.tmp
typedef struct _PASSDIST
{
	int sta_order;	// ������ ��� ������ȣ
	int dist;		// ���� ����� ����������� ����Ÿ�
}__attribute__((packed)) PASSDIST, *LPPASSDIST;

/////////////////////////////////////////////////////////////////////////
// 2004.09.25
// ������ �߰�
// ���� ����Ÿ� ���� ��������� 
// ���ϸ� : drivedis.dat
typedef struct _DRIVEDIST
{
	char date[14];	// ��������Ͻ�
	int  dist;		// ������ۺ��� ����ñ��� ����� ����Ÿ�
}__attribute__((packed)) DRIVEDIST, *LPDRIVEDIST;

/////////////////////////////////////////////////////////////////////////
// 2004.08.17
// ������ �߰�
// ���� ���� Ƚ�� ī��Ʈ��
typedef struct _TURN_CNT_STATION
{
	char szDate[8];
	int	nTotalCnt;	// �Ϸ� ��ü ī��Ʈ
	int	nCnt;		// ��������� �������� ī��Ʈ
}__attribute__((packed)) TURN_CNT_STATION, *LPTURN_CNT_STATION;


/////////////////////////////////////////////////////////////////////////
// 2004.07.10 
// ������ �߰�
// ���������� ��� ���� ���ϻ���
/////////////////////////////////////////////////////////////////////////
// 2004.12.28
// ������ ����
// ���� ������ ��� ���� �� ���� �α� ���� ����
/////////////////////////////////////////////////////////////////////////
typedef struct  _GPS_INFO_STATION
{
	char szLineNum[8];		// �����뼱ID
	char szBusID[9];		// ��������ID
	char szStartTime[14];
	char szOrder[3];		// ���������
	char szID[7];			// ������ ID
	char szSerialNum[4];            // Serial number
	char szLongitude[10];	// ����
	char szLatitude[9];		// �浵
	char szPassDis[3];		// ����� �ּҰŸ�
	char szPass[2];			// ������� 1: ���
	char szPassTime[14];	// �����ýð�
	char szSateCnt[2];		// ������
	char szIVcnt[10];		// Invalid cnt : 5����Ʈ, Valid cnt : 5����Ʈ, 12�� 28�� Jeff �߰� 
	char szErrorLog[19];		// �������Ī	16����Ʈ: ��Ī,  ���� 16����Ʈ:������۽ð�,�� ����Ÿ ���Žð�, ��������ð�	
	char szTemp[4];


} __attribute__((packed)) GPS_INFO_STATION, *LPGPS_INFO_STATION;
/////////////////////////////////////////////////////////////////////////



typedef struct
{
	unsigned char order;
	unsigned char pass;
}__attribute__((packed)) ACCUM_DIST_DATA;

typedef struct
{
	byte station_id[7];
	byte station_nm[16];
}__attribute__((packed)) FOUND_STA_INFO;

// X��ǥ�� �ش��ϴ� ������ �� ���� ���� ����� ������ ���Ķ� ����ϱ� ����.
typedef struct
{
	double 	dbX;
	int		nOrder;
}__attribute__((packed)) SORT_X;

// Y��ǥ�� �ش��ϴ� ������ �� ���� ���� ����� ������ ���Ķ� ����ϱ� ����.
typedef struct
{
	double 	dbY;
	int		nOrder;
}__attribute__((packed)) SORT_Y;


typedef struct
{
	byte abStationID[7];				// ���������� ID
	byte bCityInOutClassCode;			// �ð賻�ܱ��� �ڵ�
	byte bStationOrder[3];				// ���������
	byte abStationName[16];				// �����������
	double dLongitude;			// ���������� �浵
	double dLatitude;			// ���������� ����
	word wOffset;						// offset
	dword dwDist;		// ù�����忡���� �Ÿ�
	word wAngle;			// ������ ���԰�
} __attribute__((packed)) COMM_STA_INFO;

typedef struct
{
	int nValidCnt;		// ���� ��ġ ī��Ʈ
	int nInvalidCnt;	// ���� ���� ��ġ ī��Ʈ
	int nTimeoutCnt;	// Ÿ�Ӿƿ� ī��Ʈ
} __attribute__((packed)) GPS_STATUS_CNT;

int GpsPortOpen(char* dev, int baudrate, int lineread);
void DataStatusSet(int param);
void GpsDataLogRead(void);
void GpsDataLogWrite(void);
void GlobalParameterInit(void);

#endif