// 20060222_main.log
// 20060222_gps.log
// 20060222_term.log
// 20060222_dcs.log
//
// 1. 5��ġ�� �α����ϸ� �����Ѵ�
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "../../common/type.h"
#include "../../common/define.h"
#include "../../common/errno.h"
#include "../../common/util.h"
#include "../../common/rtc.h"

static const dword dwMaxFileSize = 1024 * 1024 * 1;		// 1MB

static byte abMainLogFileName[64] = {0, };
static byte abGPSLogFileName[64] = {0, };
static byte abTermLogFileName[64] = {0, };
static byte abDCSLogFileName[64] = {0, };

static short ArrangeLogFile(void);
static word GetFileCountInDir(byte *abDirPath);
static short DeleteOldestLogFiles(void);


/*******************************************************************************
*                                                                              *
*  FUNCTION ID :      WriteError                                               *
*                                                                              *
*  DESCRIPTION :      Writes error logs to defined error file                  *
*                                                                              *
*  INPUT PARAMETERS:  fmt - format string                                      *
*                     ... - variable argument                                  *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*******************************************************************************/
void LogMain(const char *fmt, ...)
{
#ifndef TEST_NOT_LOG
	FILE *fdLogFile;
	time_t tNowDtime = 0;
	byte abBuf[512] = {0, };
	va_list ap;

	if (GetFileSize(abMainLogFileName) >= dwMaxFileSize)
	{
		return;
	}

	fdLogFile = fopen(abMainLogFileName, "a+");
	if (fdLogFile == NULL)
	{
		return;
	}

	// ����ð��� ������
	GetRTCTime(&tNowDtime);
	TimeT2ASCDtime(tNowDtime, &abBuf[1]);
	abBuf[0] = '[';
	abBuf[15] = ']';
	abBuf[16] = ' ';
	
	va_start(ap, fmt);
	vsprintf(&abBuf[17], fmt, ap);
	va_end(ap);
	fwrite(abBuf, strlen(abBuf), 1, fdLogFile);

	fclose(fdLogFile);
#endif
}

void LogGPS(const char *fmt, ...)
{
#ifndef TEST_NOT_LOG
	FILE *fdLogFile;
	time_t tNowDtime = 0;
	byte abBuf[512] = {0, };
	va_list ap;

	if (GetFileSize(abGPSLogFileName) >= dwMaxFileSize)
	{
		return;
	}

	fdLogFile = fopen(abGPSLogFileName, "a+");
	if (fdLogFile == NULL)
	{
		return;
	}

	// ����ð��� ������
	GetRTCTime(&tNowDtime);
	TimeT2ASCDtime(tNowDtime, &abBuf[1]);
	abBuf[0] = '[';
	abBuf[15] = ']';
	abBuf[16] = ' ';
	
	va_start(ap, fmt);
	vsprintf(&abBuf[17], fmt, ap);
	va_end(ap);
	fwrite(abBuf, strlen(abBuf), 1, fdLogFile);

	fclose(fdLogFile);
#endif
}

void LogTerm(const char *fmt, ...)
{
#ifndef TEST_NOT_LOG
	FILE *fdLogFile;
	time_t tNowDtime = 0;
	byte abBuf[512] = {0, };
	va_list ap;

	if (GetFileSize(abTermLogFileName) >= dwMaxFileSize)
	{
		return;
	}

	fdLogFile = fopen(abTermLogFileName, "a+");
	if (fdLogFile == NULL)
	{
		return;
	}

	// ����ð��� ������
	GetRTCTime(&tNowDtime);
	TimeT2ASCDtime(tNowDtime, &abBuf[1]);
	abBuf[0] = '[';
	abBuf[15] = ']';
	abBuf[16] = ' ';
	
	va_start(ap, fmt);
	vsprintf(&abBuf[17], fmt, ap);
	va_end(ap);
	fwrite(abBuf, strlen(abBuf), 1, fdLogFile);

	fclose(fdLogFile);
#endif
}

void LogDCS(const char *fmt, ...)
{
#ifndef TEST_NOT_LOG
	FILE *fdLogFile;
	time_t tNowDtime = 0;
	byte abBuf[512] = {0, };
	va_list ap;

	if (GetFileSize(abDCSLogFileName) >= dwMaxFileSize)
	{
		return;
	}

	fdLogFile = fopen(abDCSLogFileName, "a+");
	if (fdLogFile == NULL)
	{
		return;
	}

	// ����ð��� ������
	GetRTCTime(&tNowDtime);
	TimeT2ASCDtime(tNowDtime, &abBuf[1]);
	abBuf[0] = '[';
	abBuf[15] = ']';
	abBuf[16] = ' ';
	
	va_start(ap, fmt);
	vsprintf(&abBuf[17], fmt, ap);
	va_end(ap);
	fwrite(abBuf, strlen(abBuf), 1, fdLogFile);

	fclose(fdLogFile);
#endif
}

short InitLog(void)
{
	byte abASCDate[8] = {0, };
	time_t tNowDtime = 0;

	// ����ð��� ������
	GetRTCTime(&tNowDtime);

	// LOG ������ ������ ������ 16���� ����
	ArrangeLogFile();

	// 4���� �α����ϸ��� �����Ѵ�.
	// YYYYMMDD_main.log
	// YYYYMMDD_gps.log
	// YYYYMMDD_term.log
	// YYYYMMDD_dcs.log
	TimeT2ASCDate(tNowDtime, abASCDate);

	memset(abMainLogFileName, 0x00, sizeof(abMainLogFileName));
	memcpy(abMainLogFileName, "/mnt/mtd8/bus/log/YYYYMMDD_main.log", 35);
	memcpy(&abMainLogFileName[18], abASCDate, sizeof(abASCDate));

	memset(abGPSLogFileName, 0x00, sizeof(abGPSLogFileName));
	memcpy(abGPSLogFileName, "/mnt/mtd8/bus/log/YYYYMMDD_gps.log", 34);
	memcpy(&abGPSLogFileName[18], abASCDate, sizeof(abASCDate));

	memset(abTermLogFileName, 0x00, sizeof(abTermLogFileName));
	memcpy(abTermLogFileName, "/mnt/mtd8/bus/log/YYYYMMDD_term.log", 35);
	memcpy(&abTermLogFileName[18], abASCDate, sizeof(abASCDate));

	memset(abDCSLogFileName, 0x00, sizeof(abDCSLogFileName));
	memcpy(abDCSLogFileName, "/mnt/mtd8/bus/log/YYYYMMDD_dcs.log", 34);
	memcpy(&abDCSLogFileName[18], abASCDate, sizeof(abASCDate));

	printf("[InitLog] LOG ���ϸ� : %s\n", abMainLogFileName);
	printf("[InitLog] LOG ���ϸ� : %s\n", abGPSLogFileName);
	printf("[InitLog] LOG ���ϸ� : %s\n", abTermLogFileName);
	printf("[InitLog] LOG ���ϸ� : %s\n", abDCSLogFileName);

	if (!IsExistFile(abMainLogFileName))
	{
		byte abTouchMain[] = "touch /mnt/mtd8/bus/log/YYYYMMDD_main.log";
		byte abTouchGPS[] = "touch /mnt/mtd8/bus/log/YYYYMMDD_gps.log";
		byte abTouchTerm[] = "touch /mnt/mtd8/bus/log/YYYYMMDD_term.log";
		byte abTouchDCS[] = "touch /mnt/mtd8/bus/log/YYYYMMDD_dcs.log";
		
		memcpy(&abTouchMain[24], abASCDate, sizeof(abASCDate));
		memcpy(&abTouchGPS[24], abASCDate, sizeof(abASCDate));
		memcpy(&abTouchTerm[24], abASCDate, sizeof(abASCDate));
		memcpy(&abTouchDCS[24], abASCDate, sizeof(abASCDate));
		
		system(abTouchMain);
		system(abTouchGPS);
		system(abTouchTerm);
		system(abTouchDCS);
	}
	
	return SUCCESS;
}

short ArrangeLogFile(void)
{
	DIR *dp;
	byte bDeleteCnt = 0;

	// LOG ���丮�� ���翩�θ� Ȯ���ϰ� ������� ���� ////////////////////////
	// LOG ���丮�� �����ϴ��� �˻�
	dp = opendir("/mnt/mtd8/bus/log");
	if (dp == NULL)
	{
		printf("[ArrangeLogFile] LOG ���丮 ������\n");
		// �������� �ʴ´ٸ� ���� ����
		if (mkdir("/mnt/mtd8/bus/log", S_IRWXO) != 0)
			printf("[ArrangeLogFile] LOG ���丮 ���� ����\n");

		// LOG ���丮 ���� ���� ����
		return -1;
	}
	closedir(dp);
	
	// LOG ���丮���� ���� ������ 16���� �ʰ��ϸ� ���� ������ �α����� ���� //
	while (GetFileCountInDir("/mnt/mtd8/bus/log") > 16)
	{
		DeleteOldestLogFiles();
		if ( bDeleteCnt++ > 100 )
		{
			printf("[ArrangeLogFile] 100ȸ �̻� DeleteOldestLogFiles() ȣ��\n");
			break;
		}
	}

	return SUCCESS;
}

word GetFileCountInDir(byte *abDirPath)
{
	DIR *dp;
	struct dirent *dirp;
	word wCount = 0;

	dp = opendir(abDirPath);
	if (dp == NULL)
	{
		return wCount;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
		{
			continue;
		}
		wCount++;
	}
	closedir(dp);

	return wCount;
}

short DeleteOldestLogFiles(void)
{
	DIR *dp;
	struct dirent *dirp;
	byte abASCDate[9] = {0, };
	byte abCommand[64] = {0, };

	dp = opendir("/mnt/mtd8/bus/log");
	if (dp == NULL)
	{
		printf("[DeleteOldestLogFiles] LOG ���丮 OPEN ����\n");
		// LOG ���丮 OPEN ���� ����
		return -1;
	}
	
	memset(abASCDate, 0xff, sizeof(abASCDate));
	
	while ((dirp = readdir(dp)) != NULL)
	{
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
		{
			continue;
		}

		if (memcmp(dirp->d_name, abASCDate, 8) < 0)
		{
			memcpy(abASCDate, dirp->d_name, 8);
			PrintlnASC("[DeleteOldestLogFiles] ���ο� ��¥ : ", abASCDate, 8);
		}
	}
	closedir(dp);

	memset(abCommand, 0x00, sizeof(abCommand));
	abASCDate[8] = 0;
	sprintf(abCommand, "rm -f /mnt/mtd8/bus/log/%s*", abASCDate);
	system(abCommand);

	PrintlnASC("[DeleteOldestLogFiles] ", abCommand, strlen(abCommand));
	
	return SUCCESS;
}

