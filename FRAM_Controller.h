/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : FRAM_Controller.h
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

#ifndef _FRAM_CONTROLLER_h
#define _FRAM_CONTROLLER_h

#define MAX_HEADERS 8
#define MAX_DATA_LENGTH 200
/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/*****************************************************************************
 * Project Specific Types
 *****************************************************************************/

typedef struct measureInfo //34 bytes
{
  uint16_t numOfSavedMeasurements; //16bit value used for padding
  uint32_t startAddr[MAX_HEADERS]; // Header starting address
}measureInfo_t;

typedef struct measureHeader //14 bytes
{
  struct _timestamp
  {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
  } timestamp;
  uint32_t prescaler;
  uint16_t numOfPoints; // Number of saved points
  uint8_t operatingMode;
  uint8_t channelMode;
} measureHeader_t;


typedef enum readFlagStatus {READ_STARTED, READ_PARTIAL, READ_DONE, READ_NOT_COMPLETE, CHECKSUM_NOT_SENT} readFlagStatus_t;
/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/
void FRAMControllerInit();
void FRAMControllerPushWrite(uint8_t * data, uint16_t len);
void FRAMControllerWriteTask();
void FRAMControllerReadTask();
void FRAMCreateHeader(measureHeader_t meas);
void FRAMCloseHeader(uint16_t totalMeasuredPoints);
void FRAMControllerAsyncRead(uint32_t addr, uint32_t len, uint8_t * buff, readFlagStatus_t * flag, uint16_t * buffLen);
void EraseMeasurements(void);



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

