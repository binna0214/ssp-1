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

/*****************************************************
* GPS ���� RAW ����Ÿ �ܰ躰 ī��Ʈ ����             *
******************************************************/
unsigned int g_nSelectFailCnt = 0;	// Select Fail Count
unsigned int g_nTimeOutCnt = 0;		// TimeOut	Count
unsigned int g_nCheckSumCnt = 0;		// CheckSum Error Count
unsigned int g_nInvalidCnt = 0;		// Invalid Count	'V'
unsigned int g_nAvailCnt = 0;		// Avail Count	'A'

unsigned int g_nPrevInvalidCnt =0;	// Previous invalid count
unsigned int g_nPrevAvailCnt =0;     // previous avail count
unsigned int g_nPrevTimeOutCnt =0;   // previous time out count
unsigned int g_nPrevCheckSumCnt = 0;	// previous checksum count

unsigned int g_n2PrevInvalidCnt =1;      // 
unsigned int g_n2PrevAvailCnt =1;        //
unsigned int g_n2PrevTimeOutCnt =0;      //

GPS_STATUS_CNT g_stGpsStatusCnt[MAXSTATIONCNT];
//////////////////////////////////////////////////////

/******************************************************************************
*                                                                             *
*    �Լ��� : GpsPortOpen()                                                   *
*                                                                             *
*      ���� : [gps ����Ÿ ������ ���� �ø��� ��Ʈ�� �ʱ�ȭ �Ѵ�.]             *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.24                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE      DESCRIPTION                       *
*             ------  ----------  --------  --------------------------------- *
*             IN      dev         char*     �ø��� ��Ʈ ��ġ ��               *
*             IN	  baudrate	  int		rs-232c ��żӵ�                  *
*			  IN	  lineread	  int		���δ����� ����(1),		    	  *
								  char ������ ����(�� �ܰ�)					  *
*    ���ϰ� : 0 �̻� �� - ������ ��ġ �ڵ�                                    *
*             -1        - ��Ʈ ���� ���� 								      *
*			  -2		- ��Ʈ �⺻ �Ӽ� Ȯ�� ����							  *
*			  -3		- ��Ʈ �Ӽ��� ���� ����						          *
*			  -4		- �Ķ���� dev �� ����								  *
*			  -5		- �Ķ���� baudrate �� ����							  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
int GpsPortOpen(char* dev, int baudrate, int lineread)
{
	int fd;
	struct termios newtio;
	struct termios oldtio;
	
//	int result = -1;
	speed_t	speed;

	if (dev == NULL || strlen(dev) < 1)	{
		printf("\n gpsPortOpen() ù��° �Ķ���� dev �� ����!!\n");
		return -4;
	}
	
	if (baudrate < 9600 || baudrate > 921600)	{
		printf("\n gpsPortOpen() �ι�° �Ķ���� baudrate �� ����!!\n");
		return -5;
	}
	
	if((fd = open(dev, O_RDONLY | O_NOCTTY)) < 0)
	{
		printf("\n");
		return -1;
	}
	
// CS8 : 8N1 (8bit, no parity, 1 stopbit) 
// CLOCAL : Local connection. �� ��� ���� �ʴ´�. 
// CREAD : ���� ������ �����ϰ� �Ѵ�

	// ��Ʈ �⺻���� �ɼ��� ��´�.
	// -1 : ����
	//  0 : ����
	if (tcgetattr(fd, &oldtio) == -1)	{
		return -2;
	}

	bzero(&newtio, sizeof(newtio));	
/*
	switch(baudrate)
	{
		case 9600 :
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);						
			break;
		case 115200 :
			cfsetispeed(&newtio,B115200);
			cfsetospeed(&newtio,B115200);			
			break;
		case 230400 :
			cfsetispeed(&newtio,B230400);
			cfsetospeed(&newtio,B230400);			
			break;
		case 460800 :
			cfsetispeed(&newtio,B460800);
			cfsetospeed(&newtio,B460800);			
			break;
		case 500000 :
			cfsetispeed(&newtio,B500000);
			cfsetospeed(&newtio,B500000);			
			break;
		case 576000 :
			cfsetispeed(&newtio,B576000);
			cfsetospeed(&newtio,B576000);			
			break;
		case 921600 :
			cfsetispeed(&newtio,B921600);
			cfsetospeed(&newtio,B921600);			
			break;
		default :
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);						
			break;			
	}
*/
	switch(baudrate)	{
		case 9600:
			speed = B9600;	break;
		case 115200:
			speed = B115200;	break;
		default:
			speed = B9600;			
	}	
	
//  newtio.c_cflag &= ~PARENB;
//  newtio.c_cflag &= ~CSTOPB;
//  newtio.c_cflag &= ~CSIZE;
  newtio.c_cflag |= speed | CRTSCTS | CS8 | CLOCAL | CREAD;


	if (lineread == 1)	
		 newtio.c_lflag = ICANON;
	else
		 newtio.c_lflag = 0;
	

//  newtio.c_iflag |= (IGNPAR|IGNBRK);
//  newtio.c_iflag = IGNPAR | ICRNL;
  newtio.c_iflag = IGNPAR;

  newtio.c_oflag = 0;

	newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
	newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	newtio.c_cc[VERASE]   = 0;     /* del */
	newtio.c_cc[VKILL]    = 0;     /* @ */
	newtio.c_cc[VEOF]     = 0;     /* Ctrl-d */
	newtio.c_cc[VTIME]    = 2;     /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	newtio.c_cc[VSWTC]    = 0;     /* '\0' */
	newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
	newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	newtio.c_cc[VEOL]     = 0;     /* '\0' */
	newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	newtio.c_cc[VEOL2]    = 0;     /* '\0' */


	tcflush(fd, TCIFLUSH);		
	if (tcsetattr(fd, TCSANOW , &newtio) == -1)	{
		return -3;		
	}
	
	return fd;
	
}


/******************************************************************************
*                                                                             *
*    �Լ��� : DataStatusSet()                                           	  *
*                                                                             *
*      ���� : [GPS ����Ÿ ���� ���¿� ���� ī��Ʈ �� ����]					  *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.30                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE            DESCRIPTION                 *
*             ------  ----------  --------        --------------------------- *
*			  IN	  param		  int  	          ���� ����Ÿ�� ���а�	      *
*    ���ϰ� : ����.				   						                      *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
void DataStatusSet(int param)
{
	switch (param)
	{
		case 1:
			g_nAvailCnt = g_nAvailCnt + 1;
			break;
		case 0:
			g_nInvalidCnt = g_nInvalidCnt + 1;
			break;
		case -1:
			g_nCheckSumCnt = g_nCheckSumCnt + 1;
			break;
		case -2:
			g_nTimeOutCnt = g_nTimeOutCnt + 1;
			close(g_gps_fd);
			g_gps_fd = 0x00;
			g_gps_fd = GpsPortOpen("/dev/ttyE1", 9600, 1);
			memset(&s_stRMC, 0x00, sizeof(RMC_DATA));
			break;
	}
}


/******************************************************************************
*                                                                             *
*    �Լ��� : GpsDataLogRead()                                                *
*                                                                             *
*      ���� : [GPS ���� ����Ÿ�� �ܰ躰 ī��Ʈ �б�]                          *
*                                                                             *
*    �ۼ��� : �� �� ��                                                        *
*                                                                             *
*    �ۼ��� : 2005.05.24                                                      *
*                                                                             *
*  �Ķ���� : IN/OUT  PARAM NAME  TYPE      DESCRIPTION                       *
*             ------  ----------  --------  --------------------------------- *
*    ���ϰ� : ����                                   						  *
*  ���ǻ��� :                                                                 *
*                                                                             *
******************************************************************************/
void GpsDataLogRead()
{
	FILE* file = NULL;
	
	file = fopen(RAW_DATA_LOG, "rb");
	if (file != NULL)	{
		fread(&g_nSelectFailCnt, sizeof(unsigned int), 1, file);
		fread(&g_nTimeOutCnt, sizeof(unsigned int), 1, file);
		fread(&g_nCheckSumCnt, sizeof(unsigned int), 1, file);
		fread(&g_nInvalidCnt, sizeof(unsigned int), 1, file);
		fread(&g_nAvailCnt, sizeof(unsigned int), 1, file);
		fread(&g_nPrevInvalidCnt, sizeof(unsigned int), 1, file);  //2005-02-18 4:30����
		fread(&g_nPrevAvailCnt, sizeof(unsigned int), 1, file); //2005-02-18 4:30����
		fread(&g_nPrevTimeOutCnt,sizeof(unsigned int), 1, file);  //2005-02-18 4:44����
				
		fclose(file);
 	}
 	else	{
 		g_nSelectFailCnt = 0;
 		g_nTimeOutCnt = 0;
 		g_nCheckSumCnt = 0;
 		g_nInvalidCnt = 0;
 		g_nAvailCnt = 0;
 		g_nPrevInvalidCnt = 0;   //2005-02-18 4:30����
 		g_nPrevAvailCnt =0;  //2005-02-18 4:30����
 		g_nPrevTimeOutCnt = 0; // 2005-02-18 4:45����
 		
 	}
	
}

// GPS ���� ����Ÿ�� �ܰ躰 ī��Ʈ ����
void GpsDataLogWrite()
{
	FILE* file = NULL;
	
	file = fopen(RAW_DATA_LOG, "wb+");
	if (file != NULL)	{
		fwrite(&g_nSelectFailCnt, sizeof(unsigned int), 1, file);
		fwrite(&g_nTimeOutCnt, sizeof(unsigned int), 1, file);
		fwrite(&g_nCheckSumCnt, sizeof(unsigned int), 1, file);
		fwrite(&g_nInvalidCnt, sizeof(unsigned int), 1, file);
		fwrite(&g_nAvailCnt, sizeof(unsigned int), 1, file);
		fwrite(&g_nPrevInvalidCnt,sizeof(unsigned int),1,file);  //2005-02-18 4:30����
		fwrite(&g_nPrevAvailCnt,sizeof(unsigned int),1,file);  //2005-02-18 4:30����
		fwrite(&g_nPrevTimeOutCnt,sizeof(unsigned int),1,file); //2005-02-18 4:43����
		fflush(file);
		fclose(file);
 	}
}

void GlobalParameterInit(void)
{
	// �������� �ʱ�ȭ �̰� ���ϸ�.. ���� �ȵ�.
	g_nFirstRun =0;
	
	g_nSelectFailCnt = 0;	// Select Fail Count
	g_nTimeOutCnt = 0;	// TimeOut	Count
	g_nCheckSumCnt = 0;	// CheckSum Error Count
	g_nInvalidCnt = 0;	// Invalid Count	'V'
	g_nAvailCnt = 0;		// Avail Count	'A'

	g_nPrevInvalidCnt =0;	// Previous invalid count
	g_nPrevAvailCnt =0;         // previous avail count
	g_nPrevTimeOutCnt =0;       // previous time out counter


	nMoveDis = 0;	// ������� ~ ������� �̵��� �Ÿ�
	// ����������� �ּҰŸ� �ܼ��� �α׿� ����ϱ� ���� ����Ѵ�.
	g_nPassMinDis = 0;

	g_nGpsExceptCnt = 0;	// �� ������� ������ �� ��

	// �����庰 GPS ���� ���� ī��Ʈ ���� �ʱ�ȭ
	memset(&g_stGpsStatusCnt, 0X00, sizeof(GPS_STATUS_CNT)*MAXSTATIONCNT);

	// ������ʿ��� ���ǵǸ� ��밡��
	//gpstSharedInfo->boolIsValidGPSSearch = TRUE;
}

