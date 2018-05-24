/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : stopwatch.c
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
#include "stopwatch.h"
#include <stdbool.h>
#include "UART_interpreter.h"
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/

static bool running = false;
static int currentValue = 0;
static int presetValue = 0;

//static int prescalerValue = 1; // Never
/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
static void StopwatchTimeout();
/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
void StopwatchTick()
{
  if(running != true) return;
  
  
  if(currentValue != 0)
  {
    currentValue--;
  }
  else
  {
    currentValue = presetValue;
    StopwatchTimeout();
  }
  

}

void StopwatchStart()
{
  running = true;
}


void StopwatchStop()
{
  running = false;
}

void StopwatchResetValue()
{
  currentValue = presetValue;
}


void StopwatchSetPreset(int p)
{
  if(presetValue < 0) return;
  presetValue = p;
}
/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/

// Use this function to do action when timeout occurs
static void StopwatchTimeout()
{
  UART_Interpreter_Timeout();
}