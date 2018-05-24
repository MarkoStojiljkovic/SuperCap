/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : rtc.h
 *
 * Created By  : Marko
 *
 * Based On    :
 *
 * Hardware    :
 *
 * Description : 
 *
 *****************************************************************************
 * $LastChangedDate:  $
 * $LastChangedRevision: $
 * $LastChangedBy: $
 * $HeadURL:  $
 *****************************************************************************/

#ifndef _RTC_h
#define _RTC_h

/*****************************************************************************
 * Include Files
 *****************************************************************************/


/*****************************************************************************
 * Project Specific Types
 *****************************************************************************/
// Union to access rtcc registers
typedef union tagRTCC {
	struct {
		unsigned char sec;
		unsigned char min;
		unsigned char hr;
		unsigned char wkd;
		unsigned char day;
		unsigned char mth;
		unsigned char yr;
	};
	struct {
		unsigned int prt00;
		unsigned int prt01;
		unsigned int prt10;
		unsigned int prt11;
	};
} RTCC;

/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/

void RTCCInit(void);
RTCC RTCGetTime();
void RTCSetTime(unsigned char year, unsigned char month, unsigned char day, unsigned char hour, unsigned char min, unsigned char sec);
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 * Macros
 *****************************************************************************/

/*****************************************************************************
 * Constants
 *****************************************************************************/

#endif

