/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : consoleCommands.c
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
#include "consoleCommands.h"
#include <xc.h>
#include <string.h>
#include "sigmaDeltaADC.h"
#include "console.h"
#include <stdio.h>
#include "rtc.h"
#include "UARTBuffer.h"

#define USE_RTC 0

#if USE_RTC == 1
#define RTCCDec2Bin(Dec) (10*(Dec>>4)+(Dec&0x0f))
#define RTCCBin2Dec(Bin) (((Bin/10)<<4)|(Bin%10))
#endif
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/

/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
static void LEDOn(int num);
static void LEDOff(int num);
static void GetSDValue();

#if USE_RTC == 1
static void GetRTC();
static void SetRTC(char * str);
#endif
/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
  
void ParseCommand(char* buff)
{
  if(strncmp(buff, "ledon", 5) == 0) return LEDOn(buff[6] - '0');
  if(strncmp(buff, "ledoff", 6) == 0) return LEDOff(buff[7] - '0');
  if(strncmp(buff, "getadc", 6) == 0) return GetSDValue();
//  if(strncmp(buff, "getrtc", 6) == 0) return GetRTC();
//  if(strncmp(buff, "setrtc", 6) == 0) return SetRTC(buff);
  AppendStringToTransmitBuffer(" Command not recognized");
}



/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/

static void LEDOn(int num)
{
  switch(num)
  {
    case 1:
    {
      LATEbits.LATE4=1;
      break;
    }
    case 2:
    {
      LATEbits.LATE5=1;
      break;
    }
    case 3:
    {
      LATEbits.LATE6=1;
      break;
    }
    default:
      return;
  }
    
}

static void LEDOff(int num)
{
  switch(num)
  {
    case 1:
    {
      LATEbits.LATE4=0;
      break;
    }
    case 2:
    {
      LATEbits.LATE5=0;
      break;
    }
    case 3:
    {
      LATEbits.LATE6=0;
      break;
    }
    default:
      return;
  }
    
}

// i copied this function elsewhere for debug 
static char tempFloatBuffer[20];
static void GetSDValue()
{
  sprintf(tempFloatBuffer,"%.4f", (double)SDLastValue);
  AppendStringToTransmitBuffer("\r\nS/D Last value of ADC is : ");
  AppendStringToTransmitBuffer(tempFloatBuffer);
  AppendStringToTransmitBuffer(" mV");
}
#if USE_RTC == 1
static void GetRTC()
{
  RTCC time = RTCGetTime();
  char year[5];
//  char temp = RTCCDec2Bin(time.yr);
  year[0] = '2';
  year[1] = '0';
  year[2] = (time.yr >> 4) + '0';
  year[3] = (time.yr & 0x0F) + '0';
  year[4] = 0;
  
  AppendStringToTransmitBuffer("\r\nRTC time is: ");
  AppendStringToTransmitBuffer(year);
  AppendCharToTransmitBuffer('-');
  
  char month[3];
  month[0] = (time.mth >> 4) + '0';
  month[1] = (time.mth & 0x0F) + '0';
  month[2] = 0;
  AppendStringToTransmitBuffer(month);
  AppendCharToTransmitBuffer('-');
  
  char day[3];
  day[0] = (time.day >> 4) + '0';
  day[1] = (time.day & 0x0F) + '0';
  day[2] = 0;
  AppendStringToTransmitBuffer(day);
  AppendCharToTransmitBuffer('-');
  
  char hour[3];
  hour[0] = (time.hr >> 4) + '0';
  hour[1] = (time.hr & 0x0F) + '0';
  hour[2] = 0;
  AppendStringToTransmitBuffer(hour);
  AppendCharToTransmitBuffer('-');
  
  char min[3];
  min[0] = (time.min >> 4) + '0';
  min[1] = (time.min & 0x0F) + '0';
  min[2] = 0;
  AppendStringToTransmitBuffer(min);
  AppendCharToTransmitBuffer('-');
 
  char sec[3];
  sec[0] = (time.sec >> 4) + '0';
  sec[1] = (time.sec & 0x0F) + '0';
  sec[2] = 0;
  AppendStringToTransmitBuffer(sec);
}



static void SetRTC(char * str)
{
  // Maybe add some intelligent parsing
  unsigned char year = (str[9] - '0')*10 + (str[10] - '0'); // Years are passed just as 2 last digits, its always 20xx no matter what you put
  unsigned char month = (str[12] - '0')*10 + (str[13] - '0');
  unsigned char day = (str[15] - '0')*10 + (str[16] - '0');
  unsigned char hour = (str[18] - '0')*10 + (str[19] - '0');
  unsigned char min = (str[21] - '0')*10 + (str[22] - '0');
  unsigned char sec = (str[24] - '0')*10 + (str[25] - '0');
  RTCSetTime(year, month, day, hour, min, sec);
}
#endif