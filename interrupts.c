/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : xxx.c
 *
 * Created By  : Marko
 *
 * Based On    : yyy.c
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
#define USE_CONSOLE 0
#define CHANNEL_SWAP_DELAY 4

typedef enum
{
  STATE_INIT_CH, // State init 
  STATE_CH_DELAY, // State used to skip corrupted result when changing channels
  STATE_CH0, // Measure Channel 0
  STATE_CH1  // Measure Channel 1
}calcu_state_t;
/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "globals.h"
#include "interrupts.h"
#include "delay.h"
#include "sigmaDeltaADC.h"
#include "console.h"
#include "UART_interpreter.h"
#include "stopwatch.h"
#include "bsp.h"
#include "commander.h"
#include "dataRecorder.h"
#include "debuggerBuffer.h"
#include "chargerController.h"
#include "blinky.h"
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/
static signed int LastSyncAdcResult = 0;
/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/
static calcu_state_t calcuState = STATE_INIT_CH;
static int currentSwapDelay = 0;
static int currentChannel = 0;
/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/

/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
  
/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/

//static int pres = 100;
//static long int counter = 0L;
void __attribute__((__interrupt__, __no_auto_psv__)) _T1Interrupt(void)//interupt on 1ms
{
  IFS0bits.T1IF = 0;
  
// Testing frequency drift
//  if(counter < 600000L)
//  {
//    counter++;
//  }
//  else
//  {
//    counter = 0;
//    InsertNewCharConsole('y');
//  }
  DelayTick();
  if(g_DelayCounter != 0) g_DelayCounter--;
  if(g_blinkyCounter != 0) g_blinkyCounter--;
  if(g_failSafeChargeDelay != 0) g_failSafeChargeDelay--;
  if(g_failSafeDischargeDelay != 0) g_failSafeDischargeDelay--;
  
  LastSyncAdcResult = (signed int)SD1RESH - SDOffset;
  // ChargerControllTask(LastSyncAdcResult)
  StopwatchTick();
  
  switch(calcuState)
  {
    case STATE_INIT_CH:
    {
      SetChannel(CHANNEL0);
      currentChannel = CHANNEL0;
      currentSwapDelay = 0;
      calcuState = STATE_CH_DELAY; // Delay until buffer has no more garbage data
      break;
    }
    case STATE_CH_DELAY:
    {
      if(currentSwapDelay++ == CHANNEL_SWAP_DELAY - 1) // -1 because there is 1 more delay because state machine
      {
        currentSwapDelay = 0;
        if(currentChannel == CHANNEL0)
        {
          calcuState = STATE_CH0;
        }
        else
        {
          calcuState = STATE_CH1;
        }
      }
      break;
    }
    case STATE_CH0:
    {
      DataRecorderTask(LastSyncAdcResult, CHANNEL0);
      ChargerControllerFailSafeTask(LastSyncAdcResult, CHANNEL0);
      calcuState = STATE_CH_DELAY;
      // Set new channel
      SetChannel(CHANNEL1);
      currentChannel = CHANNEL1;
      break;
    }
    case STATE_CH1:
    {
      DataRecorderTask(LastSyncAdcResult, CHANNEL1);
      ChargerControllerFailSafeTask(LastSyncAdcResult, CHANNEL1);
      calcuState = STATE_CH_DELAY;
      // Set new channel
      SetChannel(CHANNEL0);
      currentChannel = CHANNEL0;
      break;
    }
    default:
    {
        // This shouldn't happen but if it does, restart state machine
        calcuState = STATE_INIT_CH;
        break;
    }
  }
  
  
}



void __attribute__((__interrupt__, __no_auto_psv__)) _U1TXInterrupt(void)
{  
  IFS0bits.U1TXIF = 0; // Clear TX Interrupt flag
  
#if USE_CONSOLE == 1
  SendNextCharConsole();
#else
  SendNextCharUARTInterpreter();
#endif
}
void __attribute__((__interrupt__, __no_auto_psv__)) _U1RXInterrupt(void)
{ 
  IFS0bits.U1RXIF = 0; // Clear TX Interrupt flag
  char temp = (char)U1RXREG;
//  DebuggerInsertInBuff(temp);
  
#if USE_CONSOLE == 1
  InsertNewCharConsole(U1RXREG);
#else
  UART_Interpreter_Task(temp);
#endif 
}

// SDA interrupts are about 2kHz 
void __attribute__((__interrupt__,__no_auto_psv__)) _SDA1Interrupt(void) // TODO: Maybe not needed
{
  // TODO: remove this LED toggling 
//      static int x = 0;
//      if(x == 0)
//      {
//        LED_GREEN = 1;
//        x = 1;
//      }
//      else
//      {
//        LED_GREEN = 0;
//        x = 0;
//      }
  
    IFS6bits.SDA1IF=0;
}