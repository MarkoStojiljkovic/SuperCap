/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : UARTBuffer.h
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

#ifndef _UARTBUFFER_h
#define _UARTBUFFER_h


#define RECEIVE_BUFFER_SIZE 100 // This dictates max chars that are accepted in console
#define TRANSMIT_BUFFER_SIZE 400 // This should be bigger than RECEIVE_BUFFER_SIZE, because somethimes whole receiveBuffer needs to be sent


/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include <stdint.h>

/*****************************************************************************
 * Project Specific Types
 *****************************************************************************/


/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/
void AppendCharToReceiveBuffer(char c);
void AppendStringToTransmitBuffer(char * c);
int AppendDataToTransmitBuffer(uint8_t * buff, uint16_t len);
void AppendCharToTransmitBuffer(char c);
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/
extern char transmitBuffer[TRANSMIT_BUFFER_SIZE]; // Buffer which will hold chars that need to be sent
extern int currentCharPtrWrite; // UART interrupt will use this index
extern int totalCharsWrite; // Total writes to transmit buffer
/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/
extern char receiveBuffer[RECEIVE_BUFFER_SIZE]; // Buffer which will hold chars received by UART
extern int totalCharsRead; // Total number of chars
/*****************************************************************************
 * Macros
 *****************************************************************************/

/*****************************************************************************
 * Constants
 *****************************************************************************/

#endif

