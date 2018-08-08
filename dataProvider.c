/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : dataProvider.c
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
#define CHECKSUM_SIZE 2
#define ACK_SIZE 3
/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "dataProvider.h"
#include "FRAM_Controller.h"
#include "UARTBuffer.h"
#include "checksum.h"
#include "sigmaDeltaADC.h"
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/
// Those variables are controlled by FRAMController when read is initiated
static uint8_t dataBuff[MAX_DATA_LENGTH];
static readFlagStatus_t flag;
static uint16_t buffLen;


static int busy = 0;
/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/

/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
// Call this instruction from commander
void DataProviderFetchData(uint32_t addr, uint16_t len)
{
  if(busy == 1) return; // For now return if module is sending data
  // Send total number of data that will be sent , number of bytes + checksum
  uint8_t dataLenBuff[2];
  uint16_t totalLen = len +  CHECKSUM_SIZE;
  dataLenBuff[0] = (uint8_t)(totalLen); // LSB first
  dataLenBuff[1] = (uint8_t)(totalLen >> 8);  //MSB after
  ChecksumResetSendingMode();
  ChecksumAppendSendingMode(dataLenBuff, sizeof(dataLenBuff));
  AppendDataToTransmitBuffer(dataLenBuff, sizeof(dataLenBuff)); 
  FRAMControllerAsyncRead(addr, (uint32_t)len, dataBuff, &flag, &buffLen);
  busy = 1;
}

// Return gain, this function is not checking if transmit buffer is free, this could cause errors on client side but it is very unlikely
void DataProviderFetchGain()
{
    if(busy == 1) return; // For now return if module is already sending data
    busy = 1;
    uint8_t dataLenBuff[2];
    uint16_t totalLen = sizeof(float) +  CHECKSUM_SIZE;
    dataLenBuff[0] = (uint8_t)(totalLen); // LSB first
    dataLenBuff[1] = (uint8_t)(totalLen >> 8);  //MSB after
    
    // Calculate checksum
    ChecksumResetSendingMode();
    ChecksumAppendSendingMode(dataLenBuff, sizeof(dataLenBuff));
    ChecksumAppendSendingMode(&SDGain, sizeof(SDGain));
    uint16_t res = ChecksumGetSendingMode();
    
    // Append data to transmit buffer
    AppendDataToTransmitBuffer(dataLenBuff, sizeof(dataLenBuff));
    AppendDataToTransmitBuffer((uint8_t *)(&SDGain), sizeof(SDGain));
    AppendDataToTransmitBuffer((uint8_t *)&res, sizeof(res));
    busy = 0;
}

void DataProviderSendACK()
{
    const char ack[] = {'A', 'C', 'K'};
    
    if(busy == 1) return; // For now return if module is already sending data
    busy = 1;
    uint8_t dataLenBuff[2];
    uint16_t totalLen = ACK_SIZE +  CHECKSUM_SIZE;
    dataLenBuff[0] = (uint8_t)(totalLen); // LSB first
    dataLenBuff[1] = (uint8_t)(totalLen >> 8);  //MSB after
    
    // Calculate checksum
    ChecksumResetSendingMode();
    ChecksumAppendSendingMode(dataLenBuff, sizeof(dataLenBuff));
    ChecksumAppendSendingMode(&ack, ACK_SIZE);
    uint16_t res = ChecksumGetSendingMode();
    
    // Append data to transmit buffer
    AppendDataToTransmitBuffer(dataLenBuff, sizeof(dataLenBuff));
    AppendDataToTransmitBuffer((uint8_t *)(&ack), ACK_SIZE);
    AppendDataToTransmitBuffer((uint8_t *)&res, sizeof(res));
    busy = 0;
}

void DataProviderTask()
{
  if(busy == 0) return; // If module is idle return
  
  // By far fetching is in progress and variables are initialized
  switch(flag)
  {
    case READ_STARTED:
    {
      break; // We are waiting for buffer to be filled
    }
    case READ_PARTIAL:
    case READ_NOT_COMPLETE:
    {
      // Data arrived but not all of it
      if(AppendDataToTransmitBuffer(dataBuff, buffLen) == true) // If not successfully appended, try again
      {
        flag = READ_STARTED; // reset flag for more data
        ChecksumAppendSendingMode(dataBuff, buffLen);
      }
      else
      {
        flag = READ_NOT_COMPLETE; // Try again later
      }
      
      break;
    }
    case READ_DONE:
    {
      // All data arrived
      if(AppendDataToTransmitBuffer(dataBuff, buffLen) == true) // If not successfully appended, try again
      {
        ChecksumAppendSendingMode(dataBuff, buffLen);
        uint16_t res = ChecksumGetSendingMode();
        if(AppendDataToTransmitBuffer((uint8_t *)&res, sizeof(res)) == true)
        {
          flag = READ_STARTED;
          busy = 0;
        }
        else
        {
          flag = CHECKSUM_NOT_SENT; // Try again later
        } 
      }
      break;
    }
    case CHECKSUM_NOT_SENT:
    {
      uint16_t res = ChecksumGetSendingMode();
      if(AppendDataToTransmitBuffer((uint8_t *)&res, sizeof(res)) == true)
      {
        flag = READ_STARTED;
        busy = 0;
      }
      else
      {
        flag = CHECKSUM_NOT_SENT; // Try again later
      } 
      break;
    }
    default:
    {
      while(1); // I want device locked if this happens, for easy debug
    }
    
  }
}
/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/
