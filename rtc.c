/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : rtc.c
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
 * $LastChangedRevision:  $
 * $LastChangedBy:  $
 * $HeadURL:  $
 *****************************************************************************/

/*****************************************************************************
 * Include Files
 *****************************************************************************/

#include <xc.h>
#include "rtc.h"




RTCC _time;
RTCC _time_chk;
RTCC _alarm;

// macro
#define mRTCCDec2Bin(Dec) (10*(Dec>>4)+(Dec&0x0f))
#define mRTCCBin2Dec(Bin) (((Bin/10)<<4)|(Bin%10))
#define DISABLE_UNUSED_FUNCTIONS 1


/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/

/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
static void RTCCUnlock();
#if (DISABLE_UNUSED_FUNCTIONS == 0)
static void RTCCCalculateWeekDay();
#endif
static void RTCCSet(void);
static void RTCCgrab(void);
#if (DISABLE_UNUSED_FUNCTIONS == 0)
static void RTCCALMSet(void); // I made it static because its not used
#endif
static void RTCCSetBinSec(unsigned char );
static void RTCCSetBinMin(unsigned char );
static void RTCCSetBinHour(unsigned char );
static void RTCCSetBinDay(unsigned char );
static void RTCCSetBinMonth(unsigned char );
static void RTCCSetBinYear(unsigned char );
/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
  


/*********************************************************************
 * Function: RTCCProcessEvents
 *
 * Preconditions: RTCCInit must be called before.
 *
 * Overview: The function grabs the current time from the RTCC and
 * translate it into strings.
 *
 * Input: None.
 *
 * Output: It update time and date strings  _time_str, _date_str,
 * and _time, _time_chk structures.
 *
 ********************************************************************/
static void RTCCgrab(void)
{
	// Process time object only if time is not being set

		// Grab the time
  while(1)
  {
    RCFGCALbits.RTCPTR = 3;			
		_time.prt11 = RTCVAL;
		_time.prt10 = RTCVAL;
		_time.prt01 = RTCVAL;
		_time.prt00 = RTCVAL;

		// Grab the time again 
		RCFGCALbits.RTCPTR = 3;			
		_time_chk.prt11 = RTCVAL;
		_time_chk.prt10 = RTCVAL;
		_time_chk.prt01 = RTCVAL;
		_time_chk.prt00 = RTCVAL;

		// Verify there is no roll-over
		if ((_time.prt00 == _time_chk.prt00) &&
			(_time.prt01 == _time_chk.prt01) &&
			(_time.prt10 == _time_chk.prt10) &&
			(_time.prt11 == _time_chk.prt11))
		{
			// This means valid time was captured
      return;
		}
  }
		
	
}

/*********************************************************************
 * Function: RTCCInit
 *
 * Preconditions: RTCCInit must be called before.
 *
 * Overview: Enable the oscillator for the RTCC
 *
 * Input: None.
 *
 * Output: None.
 ********************************************************************/
void RTCCInit(void)
{
    // Enables the LP OSC for RTCC operation
	asm("mov #OSCCON,W1");	// move address of OSCCON to W1
	asm("mov.b #0x02, W0");	// move 8-bit literal to W0, 16-bit.
	asm("mov.b #0x46, W2");	// unlock byte 1 for OSCCONL(low byte)
	asm("mov.b #0x57, W3");	// unlock byte 2 for OSCCONL(low byte)
							// move 8-bit of Wn to OSCCON register
	asm("mov.b W2, [W1]");	// write unlock byte 1
	asm("mov.b W3, [W1]");	// write unlock byte 2
	asm("mov.b W0, [W1]");	// enable SOSCEN

  RTCPWCbits.PWCEN = 0; // Disable RTC
  RTCPWCbits.RTCLK = 1; // Internal LPRC
  
    // Unlock sequence must take place for RTCEN to be written
	RCFGCAL	= 0x0000;			
    RTCCUnlock();
    RCFGCALbits.RTCEN = 1;	// bit15
    
    //RTCC pin pad conected to RTCC second clock
//	PADCFG1bits.RTSECSEL = 1;	
	RCFGCALbits.RTCOE = 0;		//RTCC output disable

	/* Enable the RTCC interrupt*/
	IFS3bits.RTCIF = 0;		//clear the RTCC interrupt flag
	IEC3bits.RTCIE = 1;		//enable the RTCC interrupt

    // TO DO: Write the time and date to RTCC as follow. 
	_time_chk.sec = 0x00;
	_time_chk.min = 0x00;
	_time_chk.hr = 0x0;
	_time_chk.wkd = 0x0; 
	_time_chk.day = 0x0;
	_time_chk.mth = 0x0;
	_time_chk.yr = 0x0;
//	RTCCCalculateWeekDay();	// To calculate and confirm the weekday

	// Set it after you change the time and date. 
	RTCCSet();
	RTCPWCbits.PWCEN = 1; // Disable RTC
	// Set alarm
//	_alarm.sec	= 0x01;
//	_alarm.min	= 0x01;
//	_alarm.hr	= 0x01;
//	_alarm.wkd	= 0x01;
//	_alarm.day	= 0x01;
//	_alarm.mth	= 0x01;
//	RTCCALMSet();
}

/*********************************************************************
 * Function: RTCCSet
 *
 * Preconditions: None.
 *
 * Overview: 
 * The function upload time and date from _time_chk into clock.
 *
 * Input: _time_chk - structure containing time and date.
 *
 * Output: None.
 *
 ********************************************************************/
static void RTCCSet(void)
{
	RTCCUnlock();				// Unlock the RTCC
	
	RCFGCALbits.RTCPTR = 3;		// Set the time
	RTCVAL = _time_chk.prt11;	// set year
	RTCVAL = _time_chk.prt10;	// set month:day
	RTCVAL = _time_chk.prt01;	// set week:hour
	RTCVAL = _time_chk.prt00;	// set min:sec

	RCFGCALbits.RTCWREN = 0;	// Lock the RTCC
}

void RTCSetDate(int year, int month, int day, int hour, int sec)
{
  _time_chk.yr = year;
  
}

#if (DISABLE_UNUSED_FUNCTIONS == 0)
/*********************************************************************
 * Function: RTCCALMSet
 *
 * Preconditions: None.
 *
 * Overview: 
 * The function upload time and date from _alarm into RTCC alarm.
 *
 * Input: _alarm - structure containing time and date.
 *
 * Output: None.
 *
 ********************************************************************/
static void RTCCALMSet(void)
{
	RTCCUnlock();				// Unlock the RTCC
	while(RCFGCALbits.RTCSYNC==1);		//wait for RTCSYNC bit to become 0
	
	ALCFGRPTbits.ALRMEN		= 0;		//disable alarm to update it
	ALCFGRPTbits.ALRMPTR	= 2;  	 	//Point to Month/Day register		
	ALRMVAL = _alarm.prt10;				//load month & day	
	ALRMVAL = _alarm.prt01;				//load weekday & hour	
	ALRMVAL = _alarm.prt00;				//load minute & seconds

	ALCFGRPTbits.AMASK		= 2;		//alarm every 10 seconds
	ALCFGRPTbits.ARPT		= 0xff;		//alarm 255 times
	ALCFGRPTbits.CHIME		= 1;		//enable chime
    ALCFGRPTbits.ALRMEN		= 1;  	 	//enable the alarm

	RCFGCALbits.RTCWREN = 0;	// Lock the RTCC
}
#endif

/*********************************************************************
 * Function: RTCCUnlock
 *
 * Preconditions: None.
 *
 * Overview: The function allows a writing into the clock registers.
 *
 * Input: None.
 *
 * Output: None.
 *
 ********************************************************************/
static void RTCCUnlock()
{
	asm volatile("disi	#5");
	asm volatile("mov #0x55, w7");		// write 0x55 and 0xAA to
	asm volatile("mov w7, _NVMKEY"); 	//  NVMKEY to disable
	asm volatile("mov #0xAA, w8");		// 	write protection
	asm volatile("mov w8, _NVMKEY");
  asm volatile("bset _RCFGCAL, #13");	// set the RTCWREN bit
	asm volatile("nop");
	asm volatile("nop");
}

RTCC RTCGetTime()
{
  RTCCgrab();
  return _time;
}

void RTCSetTime(unsigned char year, unsigned char month, unsigned char day, unsigned char hour, unsigned char min, unsigned char sec)
{
//  RTCPWCbits.PWCEN = 0; // Disable RTC
  RTCCSetBinSec(sec );
  RTCCSetBinMin(min );
  RTCCSetBinHour(hour );
  RTCCSetBinDay(day);
  RTCCSetBinMonth(month );
  RTCCSetBinYear(year);
  RTCCSet(); // Apply changes to RTC registers
//  RTCPWCbits.PWCEN = 1; // Enable RTC
}

/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/


/*********************************************************************
 * Function: RTCCSetBinSec
 *
 * Preconditions: None.
 *
 * Overview: The function verifies setting seconds range, translates
 * it into BCD format and writes into _time_chk structure. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: Seconds binary value.
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
static void RTCCSetBinSec(unsigned char Sec)
{
    if(Sec >= 60)  Sec = 0;
    _time_chk.sec = mRTCCBin2Dec(Sec);
}

/*********************************************************************
 * Function: RTCCSetBinMin
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting minutes range, translates
 * it into BCD format and writes into _time_chk structure. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: Minutes binary value.
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
static void RTCCSetBinMin(unsigned char Min)
{
    if(Min >= 60)  Min = 0;
    _time_chk.min = mRTCCBin2Dec(Min);
}

/*********************************************************************
 * Function: RTCCSetBinHour
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting hours range, translates
 * it into BCD format and writes into _time_chk structure. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: Hours binary value.
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
static void RTCCSetBinHour(unsigned char Hour)
{
    if(Hour >= 24) Hour = 0;
    _time_chk.hr = mRTCCBin2Dec(Hour);
}
#if (DISABLE_UNUSED_FUNCTIONS == 0)
/*********************************************************************
 * Function: RTCCCalculateWeekDay
 *
 * Preconditions: 
 * Valid values of day, month and year must be presented in 
 * _time_chk structure.
 *
 * Overview: The function reads day, month and year from _time_chk and 
 * calculates week day. Than It writes result into _time_chk. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: _time_chk with valid values of day, month and year.
 *
 * Output: Zero based week day in _time_chk structure.
 *
 ********************************************************************/
static void RTCCCalculateWeekDay()
{
	const char MonthOffset[] =
	//jan feb mar apr may jun jul aug sep oct nov dec
	{   0,  3,  3,  6,  1,  4,  6,  2,  5,  0,  3,  5 };
	unsigned Year;
	unsigned Month;
	unsigned Day;
	unsigned Offset;
    // calculate week day 
    Year  = mRTCCDec2Bin(_time_chk.yr);
    Month = mRTCCDec2Bin(_time_chk.mth);
    Day  = mRTCCDec2Bin(_time_chk.day);
    
    // 2000s century offset = 6 +
    // every year 365%7 = 1 day shift +
    // every leap year adds 1 day
    Offset = 6 + Year + Year/4;
    // Add month offset from table
    Offset += MonthOffset[Month-1];
    // Add day
    Offset += Day;

    // If it's a leap year and before March there's no additional day yet
    if((Year%4) == 0)
        if(Month < 3)
            Offset -= 1;
    
    // Week day is
    Offset %= 7;

    _time_chk.wkd = Offset;
}
#endif
/*********************************************************************
 * Function: RTCCSetBinDay
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting day range, translates it
 * into BCD format and writes into _time_chk structure. To write the
 * structure into clock RTCCSet must be called.
 *
 * Input: Day binary value.
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
static void RTCCSetBinDay(unsigned char Day)
{
    const char MonthDaymax[] =
    //jan feb mar apr may jun jul aug sep oct nov dec
    {  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    unsigned Daymax;
    unsigned Month;
    unsigned Year;

    Month = mRTCCDec2Bin(_time_chk.mth);
    Year = mRTCCDec2Bin(_time_chk.yr);

    Daymax = MonthDaymax[Month-1];

    // February has one day more for a leap year
    if(Month == 2)
    if( (Year%4) == 0)
        Daymax++;

    if(Day == 0) Day = Daymax;
    if(Day > Daymax) Day = 1;
    _time_chk.day = mRTCCBin2Dec(Day);
}

/*********************************************************************
 * Function: RTCCSetBinMonth
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting month range, translates
 * it into BCD format and writes into _time_chk structure. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: Month binary value.
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
static void RTCCSetBinMonth(unsigned char Month)
{
    if(Month < 1) Month = 12;
    if(Month > 12) Month = 1;
    _time_chk.mth = mRTCCBin2Dec(Month);
}

/*********************************************************************
 * Function: RTCCSetBinYear
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting year range, translates it
 * into BCD format and writes into _time_chk structure. To write the 
 * structure into clock RTCCSet must be called.
 *
 * Input: Year binary value.
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
static void RTCCSetBinYear(unsigned char Year)
{
   if(Year >= 100)  Year = 0;
    _time_chk.yr = mRTCCBin2Dec(Year);
    // Recheck day. Leap year influences to Feb 28/29.
    RTCCSetBinDay(mRTCCDec2Bin(_time_chk.day));
}