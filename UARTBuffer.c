/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : UARTBuffer.c
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

#define MAX_STRING_LENGTH 100


/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "UARTBuffer.h"
#include "xc.h"
#include "bsp.h"
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/
char transmitBuffer[TRANSMIT_BUFFER_SIZE]; // Buffer which will hold chars that need to be sent
int currentCharPtrWrite = 0; // UART interrupt will use this index
int totalCharsWrite = 0; // Total writes to transmit buffer

char receiveBuffer[RECEIVE_BUFFER_SIZE]; // Buffer which will hold chars received by UART
int totalCharsRead = 0; // Total number of chars
/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/

/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/

/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
// This buffer is filled with data received via UART
void AppendCharToReceiveBuffer(char c)
{
  if(totalCharsRead >= RECEIVE_BUFFER_SIZE)
  {
    // Not enough space for char, buffer is full, just ignore
    return;
  }
  receiveBuffer[totalCharsRead++] = c;
}

// THIS FUNCTION IS USED ONLY WITH CONSOLE AND SHOULD BE REPLACED !!!
void AppendStringToTransmitBuffer(char * c)
{
  int len;
  
  // Find string length
  for(len = 0; len < MAX_STRING_LENGTH; len++)
  {
    if(c[len] == 0) break;
  }
  
  if(totalCharsWrite >= TRANSMIT_BUFFER_SIZE - len)
  {
    // Not enough space for string, just ignore it for now
    return;
  }
  int i;
  for(i = 0; i <= len; i++)
  {
    transmitBuffer[totalCharsWrite++] = c[i];
  }
  
  if(totalCharsWrite == len + 1)
  {
    // if this is first char it needs to start sending manually
    RS485_DIR = 1; // Set RS485 directional pin to output
    while(U1STAbits.TRMT==0);
    U1TXREG=transmitBuffer[0];
    currentCharPtrWrite++;
  }
}

int AppendDataToTransmitBuffer(uint8_t * buff, uint16_t len)
{
  if(totalCharsWrite >= TRANSMIT_BUFFER_SIZE - len)
  {
    // Not enough space for data, abort
    return 0;
  }
  int i;
  for(i = 0; i < len; i++)
  {
    transmitBuffer[totalCharsWrite++] = buff[i];
  }
  
  if(totalCharsWrite == len)
  {
    RS485_DIR = 1; // Set RS485 directional pin to output
    // if this is first char it needs to start sending manually
    while(U1STAbits.TRMT==0);
    U1TXREG=transmitBuffer[0];
    currentCharPtrWrite++;
  }
  
  return 1;
}

// THIS FUNCTION IS USED ONLY WITH CONSOLE AND SHOULD BE REPLACED !!!
// Append char to transmitBuffer and initialize sending if needed (which will be automated via interrupt after initialization, until all chars are sent)
void AppendCharToTransmitBuffer(char c)
{
  if(totalCharsWrite >= TRANSMIT_BUFFER_SIZE)
  {
    // Not enough space for char, buffer is full, just ignore
    return;
  }
  transmitBuffer[totalCharsWrite++] = c;
  if(totalCharsWrite == 1 )
  {
    RS485_DIR = 1; // Set RS485 directional pin to output
    // if this is first char it needs to start sending manually
    while(U1STAbits.TRMT==0);
    U1TXREG=transmitBuffer[0];
    currentCharPtrWrite++;
  }
}
/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/

