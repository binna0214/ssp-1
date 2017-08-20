
#ifndef _SIMX_
#define _SIMX_

//#include "gps_env.h"


//////////////////////////////////////////////
//GPS Station Detection Status
//
#define INIT_STATION_SEARCH 1  //�ʱ� ������ Ž�� ��
#define INIT_STATION_SEARCH_COMPLETE1 2  //�ʱ� ������ Ž�� �Ϸ�(������)
#define INIT_STATION_SEARCH_COMPLETE2 3  //�ʱ� ������ Ž�� �Ϸ�(1�������� �˻�)
#define NEXT_STATION_SEARCH 4  //���� ������ �˻���
#define STATION_VERIFICATION 5  //���� ������ ���� ����
#define NEXT_STATION_UPDATE_COMPLETE1 6  //���� ���������� 
#define NEXT_STATION_UPDATE_COMPLETE2 7 //
#define NEXT_STATION_UPDATE_COMPLETE3 8
#define STATION_DETECTION_FAIL 9

#define GPS_INVALID 10
#define GPS_DOP_ERROR 11
#define GPS_UN_USEDATA 12
#define GPS_DB_HEADING_ERROR 13

#define GPS_INVALID_TIMEOUT 14
#define INIT_DB_ERROR 15
//////////////////////////////////////////////

//Threshold value
#define GPS_TIMEOUT 15  //GPS timeout value threshold
#define DOP_THRESHOLD 10  //dop error threshold
//////////////////////////////////////////////

// global parameter
#define D2R 0.01745329251994   
#define R2D 57.29577951308232


#define DIFF_DIST_P_FIRST	40  //�ʱ� db ������ ���� �Ÿ�
#define DIFF_DIST_P			0.7  //db percentage


/////////////////////////////////////////////////////////////////////////
//rmc data
typedef struct {

	double	UTCTime;
	char	PosStatus;
	double	Lat;
	char 	LatDir;
	double	Lon;
	char 	LonDir;
	double	SpeedKn;
	double	TrackTrue;
	double	date;
} __attribute__((packed)) RMC_DATA;

typedef struct
{
       byte start_st;
	byte gps_st[2];
	byte data_kind[3];
	byte utc_world_time[11];
	byte use_kind[2];
	byte lattitude[10];
	byte north_south[2];
	byte longitude[11];
	byte east_west[2];
	byte knot_speed[7];
	byte heading[7];	
	byte date[7];
	byte adjust_dec[5];
	byte west_dod[2];
	byte schecksum[2];
	byte dchecksum[3];	
	byte crlf[3];
} __attribute__((packed)) GPS_DATA;

//���� ������ ����ü 
typedef struct {
	
	unsigned int StationID; //ID
	unsigned int StationOrder; //Station order
	unsigned int DBLen;  //Station number
	double LonDeg;  //Station longitude
	double LatDeg;  //Station latitude
	double Angle;  //Station Angle
	double AccumDist; //Station Accumulated Distance
	double DiffDist;  //70% Distance between current station and previous station 
	double DiffDist2; // Distance between current station and previous station 
	double ENU[3];  //ENU value
} __attribute__((packed)) BUS_STATION;  


typedef struct{
	
	double		AllStationMinDist[256];	//��ü Station�� ���� ����� �ּ� �Ÿ� ���� 
	unsigned int	DBLen;  				//DB�� �� ���� 
	unsigned int	CurrentStationID;                       //���� ������ ID
	unsigned int	CurrentStationOrder;                    //���� ������ ����
	unsigned int	NextStationID;                          //���� ������ ID
	unsigned int	NextStationOrder;                       //���� ������ ����
	unsigned int	Status;  				//���� �ڵ� 
	double		CurrentStationDist; 			//���� Station����  �Ÿ� 
	double		CurrentVelms;                           //���� ���� �ӵ�(m/s)
	double		CurrentPosLat;                          //���� ���� ����
	double		CurrentPosLon;                          //���� ���� �浵
	double		CurrentPosENU[3];                       //���� ���� ENU �� 
	double		CurrentHeading;                         //���� ���� ���
	double		CurrentHeadingError;                    //���� ������� ���� ������� ���̰�
	unsigned int	GPSValidFlag;				// GPS Valid Flag
	double		MinStationUTCTime;			//�ּ� �Ÿ������� UTC time 


} __attribute__((packed)) BUSINFO;

//cordinate value
typedef struct {
	
	double OrgLLH[3];
	double OrgXYZ[3];  
} __attribute__((packed)) COORDINATES;



//gga data
typedef struct {
	double UTCTime;			//UTC of Position
	double Lat;	   		//Latitude  (DDmm.mm)
	char   LatDir;			//N or S
	double Lon;			//Longitude (DDmm.mm)
	char   LonDir;			//E or W
	int    GPSQual;			//GPS quality indicator (0=invalid; 1=GPS fix; 2=Diff. GPS fix)
	int    SatNum;			//Number of satellites in use [not those in view]
	double HDOP;			//Horizontal dilution of position
	double Altitude;		//Antenna altitude above/below mean sea level (geoid)
} __attribute__((packed)) GGA_DATA;



typedef struct				
{
        byte  appltn_dtime[14];          // �����Ͻ�(BCD)
        byte  appltn_seq[2];               // �����Ϸù�ȣ(BCD)
        byte  bus_route_id[8];          // �����뼱 ID
        byte  transp_method_cd[16];     // �����뼱��
        byte  bus_sta_cnt[3];           // ���������尹��
        byte  bus_sta_id[7];            // ���������� ID
        byte  city_bnd_in_out_class_cd; // �ð賻�ܱ��� �ڵ�
        byte  bus_sta_order[3];            // ���������
        byte  bus_sta_pos_nm[16];       // ���������� ��
        byte  bus_sta_pos_x[10];        // ������������ǥ X
        byte  bus_sta_pos_y[9];        // ������������ǥ Y
        byte  dist_6[3];                   // Off Set
        byte  bus_sta_cum_dist[6];      // ù�����忡���� �Ÿ�(BCD)
        byte  heading[3];
} __attribute__((packed)) STA_INFO_LOAD;              // ���������� ���� �ε�

extern unsigned int InitStationIDFixFlag;  // �ʱ� �������� ��� ���� 1�� ����

/*************************************************************************************
 *		
 *	function: PreProcess
 *	Param: sBus_station : ���� ������ ������ ������ ����ü 
 *		nSTA_IF_LOAD: ���� ������ ���� ���� ����ü(�ƽ�Ű ����),
 *		coordinates : ��ǥ��
 *      Return: zero
 *	discription: ���������� �ν� �˰����� �����ϱ����� ���� ���� �ʱ�ȭ�� DB�����͸� �о��  
 *	note: 
 **************************************************************************************/
extern unsigned int PreProcess(BUS_STATION *sBus_station, STA_INFO_LOAD* nSTA_IF_LOAD, COORDINATES* coordinates);



/*************************************************************************************
 *		
 *	function: DBManage
 *	Param: sBus_station : ���� ������ ������ ������ ����ü 
 *      Return: zero
 *	discription: �ν� �˰��򿡼� ����� DB�����͸� ���� �Ѵ�.  
 *	note: 
 **************************************************************************************/
extern unsigned int DBManage(BUS_STATION *sBus_station);




/*************************************************************************************
 *		
 *	function: GPSCorr
 *	Param: sBus_station : gps : gps data(�ƽ�Ű) , rmc : rmc����Ÿ 
 *      Return: zero
 *	discription: �ν� �˰��򿡼� ����� GPS�����͸� ���� �Ѵ�. 
 *      ���ִ� gga�� ����Ѵ�. 
 *	note: 
 **************************************************************************************/
extern unsigned int GPSCorr(GPS_DATA *gps, RMC_DATA *rmc,GGA_DATA* gga);




/*************************************************************************************
 *		
 *	function: BusStationDetect
 *	Param: sBus_station : ���� ������ ������ ������ ����ü
 *              spBusInfo : ���� ���¹� ������ ��ȣ ���� ����ü 
 *              rmc : rmc ����Ÿ 
 *              gga : gga ����Ÿ 
 *              coordinates : ��ǥ�� 
 *              InitStationOrder : �ʱ� ������ ���ð� 
 *      Return: zero
 *	discription: ���� ������ �ν� �˰��� 
 *	note: 
 **************************************************************************************/
extern unsigned int BusStationDetect(BUS_STATION* sBus_station, BUSINFO *spBusInfo, RMC_DATA* rmc, GGA_DATA* gga, COORDINATES* coordinates, unsigned int InitStationOrder);



/*************************************************************************************
 *		
 *	function: WriteLogSimx
 *	Param: rmc : rmc ������
 *             spBusInfo : ���� ������ ����ü ������  
 *      Return: zero
 *	discription: ���ű⿡�� ������ Raw�����͸� ����. �� �Լ��� ������ ���߿�
 *                   ���� ������ �׸��ų� �˰��� �׽�Ʈ�뵵�̱� ������ �������� �ʴ� 
 *                   �����͸� ����.   
 *	note: 
 **************************************************************************************/
extern unsigned int WriteLogSimx(RMC_DATA* rmc,BUSINFO *spBusInfo);  //500k simxlog�� ���� �Լ� 



extern GPS_DATA GPS;

#endif
