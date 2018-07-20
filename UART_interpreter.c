
/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : UART_interpreter.c
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

#include "UART_interpreter.h"
#include <stdbool.h>
#include "stopwatch.h"
#include "xc.h"
#include "UARTBuffer.h"
#include "bsp.h"
#include "sigmaDeltaADC.h"
#include <stdio.h>
#include "commander.h"
#include "interrupts.h"
#include "debuggerBuffer.h"
#include "checksum.h"
#include "delay.h"

#define DEV_ADDRESS 0x31
#define MESSAGE_INFO_LEN 2
#define CHECKSUM_LEN 2
#define MAX_STRING_LENGTH 80
#define START_SEQ 0xfe
#define ENABLE_DEBUG 1
#define DEBUG_DIODE LED_YELLOW
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/

typedef enum 
{
  STATE_INIT,
  STATE_START_CHECK,
  STATE_ADDR_CHECK,
  STATE_READ_CMD_LENGTH,
  STATE_RECEIVE_MESSAGE
}interpreter_state_t ;

interpreter_state_t stateDR = STATE_INIT;

static bool lock; // Prevent that timeout occurs inside UART task
static int commandsLen = 0;

/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
static bool CompareAddress(char c);
static void ExecuteAction();
static bool EvaluateStart(char c);
static int CalculateMessageLength(char* buff);
/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
  
// This function should be called from UART receive interrupt (when new byte arrived)
void UART_Interpreter_Task(char c)
{
  
  // Reset and stop stopwatch here, it will be started further if needed
  // it will be started in all transition states except for STATE_START_CHECK (this is idle state, and state after timeout)
  StopwatchResetValue();
  StopwatchStop();
  lock = true; // Prevent timeout callback to occur inside this function
  
  switch(stateDR)
  {
    case STATE_INIT: // Initialize timeout mechanism
    {
      StopwatchSetPreset(1000); // stopwatch tick is 1ms
      StopwatchResetValue();
      stateDR = STATE_START_CHECK; // Skip init next time
      // No break, go to next state immediately
    }
    case STATE_START_CHECK:
    { 
      if(EvaluateStart(c)) // Check is it start, ignore otherwise
      {
        stateDR = STATE_ADDR_CHECK; 
        StopwatchStart(); // Start stopwatch
      }
      break;
    }
    case STATE_ADDR_CHECK:
    {
      // Check with own address
      if(CompareAddress(c))
      {
        StopwatchStart(); // Start stopwatch
        stateDR = STATE_READ_CMD_LENGTH;
        // Init checksum calculation here, no need for start seq and address, they will be added at the end if all goes well
        ChecksumResetReceiveMode();
      }
      else
      {
        stateDR = STATE_START_CHECK;
      }
      break;
    }
    case STATE_READ_CMD_LENGTH: // Receive length of commands
    {
      AppendCharToReceiveBuffer(c);
      StopwatchStart(); // Start stopwatch
      
      if(totalCharsRead == MESSAGE_INFO_LEN)
      {
        commandsLen = CalculateMessageLength(receiveBuffer);
        stateDR = STATE_RECEIVE_MESSAGE;
      }
      break;
    }
    case STATE_RECEIVE_MESSAGE: // Receive remaining message and after its fully received process it
    {
      AppendCharToReceiveBuffer(c);
      if(totalCharsRead == commandsLen + MESSAGE_INFO_LEN)
      {
//        DebuggerResetBuff(0);
        
        ChecksumAppendReceiveMode(receiveBuffer, totalCharsRead - 2); // Exclude checksum from calculation
        ChecksumAppenByteReceiveMode(START_SEQ);
        ChecksumAppenByteReceiveMode(DEV_ADDRESS);
        
        if(ChecksumCompareReceiveMode(receiveBuffer, totalCharsRead - 2))
        {
          // If checksum passed execute action
          commandsLen -= 2; // Remove checksum bytes
          ExecuteAction();
        }
//        StartTransmision();
        stateDR = STATE_START_CHECK;
        totalCharsRead = 0; // reset read buffer
      }
      else StopwatchStart(); // Start stopwatch only if not all data are received
      
      break;
    }
    default:
    {
      break;
    }
  }
  lock = false;
}

// This function will be called after transmit finished interrupt to check if there are more pending chars for sending
void SendNextCharUARTInterpreter()
{
  if(currentCharPtrWrite < totalCharsWrite)
  {
    U1TXREG = transmitBuffer[currentCharPtrWrite++];
  }
  else
  {
    // Sending is finished, wait some time and then Set RS485 directional pin to input
    Delay1ms(3);
    RS485_DIR = 0; // Set RS485 directional pin to input
    // Reset write buffer
    currentCharPtrWrite = 0;
    totalCharsWrite = 0;
  }
}

void UART_Interpreter_Timeout()
{
  // Timeout period passed, restart state machine
  if(lock == true)
  {
    // We received something, that means this is false positive (timeout happened while processing received data)
    StopwatchResetValue();
  }
  else
  {
    // Reset UART_Interpreter
    
    // Reset receive buffer
    totalCharsRead = 0;
    
    // Reset write buffer
    currentCharPtrWrite = 0;
    totalCharsWrite = 0;
    
    // Reset state machine
    stateDR = STATE_START_CHECK;
    StopwatchStop(); // We are done with it for now
#if (ENABLE_DEBUG == 1)
    if(DEBUG_DIODE == 1)
    {
        DEBUG_DIODE = 0;
    }
    else
    {
        DEBUG_DIODE = 1;
    }
    DebuggerResetBuff(0);
#endif
  }
}

/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/

static bool CompareAddress(char c)
{
  return (c == (char)DEV_ADDRESS);
}

static void ExecuteAction()
{
  // I will skip first 2 chars which are holding number of instructions
  FillInstructionArray((uint8_t*)receiveBuffer + 2, commandsLen);
  startFlag = 1;
  
}

static bool EvaluateStart(char c)
{
  return( c == (char)START_SEQ);
}

static int CalculateMessageLength(char* buff)
{
  int res = buff[0] << 8;
  res |= buff[1];
  return res;
}

//static char tempFloatBuffer[20];
//static void SendSDValue()
//{
//  sprintf(tempFloatBuffer,"%.4f", (double)SDLastValue);
//  AppendStringToTransmitBuffer("\r\nS/D Last value of ADC is : ");
//  AppendStringToTransmitBuffer(tempFloatBuffer);
//  AppendStringToTransmitBuffer(" mV");
//}