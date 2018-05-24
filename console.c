/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : console.c
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
#include <stdbool.h>
#include "console.h"
#include "consoleCommands.h"
#include "p24FJ128GC006.h"
#include "UARTBuffer.h"

/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/


/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
//static void AppendCharToTransmitBuffer(char c);
//static void AppendStringToTransmitBuffer(char * c);
static void RemoveLastChar(void);
/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
  
/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/

// Removes last char in receivedBuffer and sends command for visual representation of char removal to console
static void RemoveLastChar()
{
  // Remove it from receive buffer and send command for backspace
  if(totalCharsRead != 0)
  {
    totalCharsRead--;
    AppendCharToTransmitBuffer(0x7f); // Use delete
  }
}

/*****************************************************************************
 * Function Implementation
 ****************************************************************************/
// This should be called from UART receive callback
void InsertNewCharConsole(char c)
{
  switch(c)
  {
    case 0xD: // Carriage return, new line, user pressed enter
    {
      if(totalCharsRead != 0) // if buffer is empty just go to new line
      {
        // Buffer is not empty
        ParseCommand(receiveBuffer);
        AppendCharToTransmitBuffer('\r');
        AppendCharToTransmitBuffer('\n');
        AppendStringToTransmitBuffer(">>> ");
//        DumpReadBuffer();
        totalCharsRead = 0;
      }
      AppendCharToTransmitBuffer('\r');
      AppendCharToTransmitBuffer('\n');
      totalCharsRead = 0;
      break;
    }
    case 0x20:          // Space
    case 0x30 ... 0x39: // Numbers
    case 0x41 ... 0x5a: // Big letters
    case 0x61 ... 0x7a: // Small letters
    {
      AppendCharToReceiveBuffer(c);
      AppendCharToTransmitBuffer(c);
      break;
    }
    case 0x7f: // Backspace
    {
      RemoveLastChar();
      break;
    }
    default:
      break; // Ignore request
  }
}


// This function will be called after transmit finished interrupt to check if there are more pending chars for sending
void SendNextCharConsole()
{
  if(currentCharPtrWrite < totalCharsWrite)
  {
    U1TXREG = transmitBuffer[currentCharPtrWrite++];
  }
  else
  {
    // Reset write buffer
    currentCharPtrWrite = 0;
    totalCharsWrite = 0;
  }
}



