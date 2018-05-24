/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : checksum.c
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
#include "checksum.h"
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/
uint16_t CSCounterSendingMode = 0;
uint16_t CSCounterReceiveMode = 0;
/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/

/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/

// Use this functions when in sending mode
// Always reset counter before starting calculations for new data
void ChecksumResetSendingMode()
{
  CSCounterSendingMode = 0;
}

// Append data byte by byte
void ChecksumAppendSendingMode(void * val, int size)
{
  int i;
  for(i = 0; i < size; i++)
  {
    CSCounterSendingMode += ((uint8_t *)val)[i];
  }
}

uint16_t ChecksumGetSendingMode()
{
  return CSCounterSendingMode;
}


// Use this functions when in receive mode
void ChecksumResetReceiveMode()
{
  CSCounterReceiveMode = 0;
}

// Append data byte by byte
void ChecksumAppendReceiveMode(void * val, int size)
{
  int i;
  for(i = 0; i < size; i++)
  {
    CSCounterReceiveMode += ((uint8_t *)val)[i];
  }
}

void ChecksumAppenByteReceiveMode(uint8_t val)
{
  CSCounterReceiveMode += val;
}

uint16_t ChecksumGetReceiveMode()
{
  return CSCounterReceiveMode;
}

int ChecksumCompareReceiveMode(void * buff, int offset)
{
  uint16_t givenCs = ((uint8_t *)buff)[offset];
  givenCs |= (((uint8_t *)buff)[offset + 1] << 8);
  
  return (givenCs == CSCounterReceiveMode);
}
/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/
