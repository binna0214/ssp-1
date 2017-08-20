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
*  PROGRAM ID :       main.c                                                   *
*                                                                              *
*  DESCRIPTION:       Device Open, Use, Close								   *
*                                                                              *
*  ENTRY POINT:       None                  ** mandatory **                    *
*                                                                              *
*  INPUT PARAMETERS:  None                                                     *
*                                                                              *
*  RETURN/EXIT VALUE: None                                                     *
*                                                                              *
*  INPUT FILES:       None                                                     *
*                                                                              *
*  OUTPUT FILES:      None                                                     *
*                                                                              *
*  SPECIAL LOGIC:     None                                                     *
*                                                                              *
********************************************************************************
*                         MODIFICATION LOG                                     *
*                                                                              *
*    DATE                SE NAME              DESCRIPTION                      *
* ---------- ---------------------------- -------------------------------------*
* 2005/08/09 Solution Team Gwan Yul Kim  Initial Release                       *
*                                                                              *
*******************************************************************************/

#ifndef _DEVICE_INTERFACE_H
#define _DEVICE_INTERFACE_H

/*******************************************************************************
*  Inclusion of Header Files                                                 *
*******************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/statfs.h>

#include "../system/bus_type.h"

struct seg_info {
	int disp_no;
	unsigned long value;
	unsigned char *data_str;
} __attribute__((packed)) ;


/*******************************************************************************
*  Macro Defintion                                                             *
*******************************************************************************/
#define F_WRITE							0x01
#define F_READ							0x00

#define VOICE_VOLUME_ADDR				0x88
#define VOICE_IOCTL_TYPE 				'p'
#define VOICE_FLASH_DUMP				_IOR('p',0x82,int)
#define VOICE_FLASH_BLKNUM      		_IOW('p',0x83,long)
#define VOICE_FLASH_ERASE_ALL			_IOW('p',0x84,long)
#define VOICE_FLASH_ID_GET				_IOR('p',0x85,char)
#define VOICE_FLASH_WRITE_ON			_IOW('p',0x86,char)
#define VOICE_BEEP_SET					_IOW('p',0x87,int)
#define VOICE_FREQ_SET					_IOW('p',0x88,int)
#define VOICE_REPEAT_SET				_IOW('p',0x89,int)
#define VOICE_FLASH_BLANK_CK			_IOW('p',0x8a,char)
#define VOICE_FLASH_VERIFY				_IOW('p',0x8b,char)

/*** ioctl command ***/
#define SEG_IOCTL_TYPE 					's'

#define SEG_CONROL						_IOW('s', 0x80, char)
#define SEG_DISP_NUM					_IOW('s', 0x81, char)
#define ALL_LED_ON_OFF					_IOW('s', 0x82, char)
#define SEG_DISP_STR					_IOW('s', 0x83, char)
#define SEG_DISP_CLR					_IOW('s', 0x84, char)

#define I2C_SLAVE	0x0703	/* Change slave address			*/
#define I2C_TENBIT	0x0704	/* 0 for 7 bit addrs, != 0 for 10 bit	*/
#define I2C_RDWR	0x0707	/* Combined R/W transfer (one stop only)*/

#define I2C_M_TEN	0x10	/* we have a ten bit chip address	*/
#define I2C_M_RD						0x01
#define I2C_M_NOSTART					0x4000
#define I2C_M_REV_DIR_ADDR				0x2000

#define VOICE_END_DRIVE					3		// ������ �����մϴ�.
#define VOICE_RETAG_CARD				5		// ī�带 �ٽ� ���ּ���.
#define VOICE_CHECK_TERM				15		// �ܸ��⸦ �������ֽñ� �ٶ��ϴ�.
#define VOICE_PRESS_END_BUTTON			16		// ������ �������ϴ�. ���� ���� ��ư�� ���� �ּ���.
#define VOICE_PASSENGER_CHILD			20		// ����Դϴ�.
#define VOICE_PASSENGER_YOUNG			21		// û�ҳ��Դϴ�.
#define VOICE_INSUFFICIENT_BAL			23		// �ܾ��� �����մϴ�.
#define VOICE_INVALID_CARD				26		// ����� �� ���� ī���Դϴ�.
#define VOICE_XFER						27		// ȯ���Դϴ�.
#define VOICE_ALREADY_PROCESSED			28		// �̹� ó���Ǿ����ϴ�.
#define VOICE_START_DRIVE				29		// ������ �����մϴ�.
#define VOICE_EXPIRED_CARD				30		// ī�� ��ȿ�Ⱓ�� �������ϴ�.
#define VOICE_TAG_ONE_CARD				33		// ī�带 ���常 ���ֽʽÿ�.
#define	VOICE_RECEIVE_LATEST_INFO		35		// �ֽ� ���������� �����Ͽ� �ֽʽÿ�.
#define VOICE_MULTI_ENT					36		// ���ν��Դϴ�.
#define VOICE_NOT_APPROV				37		// �̽��� ī���Դϴ�.
#define VOICE_TEST_CARD					39		// �׽�Ʈ ī���Դϴ�.
#define VOICE_TAG_IN_EXT				40		// ������ ī�带 ���ּ���.
#define VOICE_CHECK_TERM_TIME			42		// �ܸ��� �ð��� Ȯ���� �ּ���.
#define VOICE_CANNOT_MULTI_ENT_CARD		46		// ���ν��� �Ұ����� ī���Դϴ�.
#define VOICE_INPUT_TICKET_INFO			49		// �������� ������ �ּ���.
#define VOICE_THANK_YOU					48		// Thank you.
#define VOICE_CITY_PASS_CARD			43		// ��Ƽ�н��Դϴ�.

/*******************************************************************************
*  Declaration of Function                                                     *
*******************************************************************************/
short OpenVoice( void );
short WriteVoiceFile2Flash( int* fdVoiceFile, int nVoiceFileSize );
void CloseVoice( void );
short VoiceOut( word wVoiceNo );
short OpenRC531( int *fdRC531Device );
void CloseRC531( int *fdRC531Device );
short OpenPrinter( int *fdPrinterDevice, word wBaudrate );
void ClosePrinter( int *fdPrinterDevice );
short DisplayASCInUpFND(byte *abASC);
short DisplayASCInDownFND(byte *abASC);
short DisplayDWORDInUpFND(dword dwNo);
short DisplayDWORDInDownFND(dword dwNo);
short OpenBuzz( void );
void CloseBuzz( void );
short Buzzer( word wCnt, dword dwDelayTime );
short WriteBuzz( char *pcVal, int nVal );
short OpenDipSwitch( void );
void CloseDipSwitch( void );
byte ReadDipSwitch( void );
short OpenEEPROM( void );
void CloseEEPROM( void );
short WriteEEPROM( byte* bWriteData );
short ReadEEPROM( byte* bReadData );
short CheckWriteEEPROM( byte* bWriteData, byte* bReadData );
long MemoryCheck( void );
void OLEDOn(void);
void OLEDOff(void);
void XLEDOn(void);
void XLEDOff(void);
short DisplayCommUpDownMsg( word wUpDown, word wWorkGubunClass );


#endif
