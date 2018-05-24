/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : debuggerBuffer.c
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
#define BUFF_SIZE 100
/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "debuggerBuffer.h"
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/
char debugBuff[BUFF_SIZE] = {0};
static int buffIndex = 0;
/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/

/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
void DebuggerInsertInBuff(char c)
{
  if(buffIndex == BUFF_SIZE) return;
  
  debugBuff[buffIndex++] = c;
}

void DebuggerResetBuff(int flush)
{
  // Reset index
  buffIndex = 0;
  
  // If flush is enabled rewrite whole buffer with zeroes
  if(flush == 0) return;
  
  int i;
  for(i = 0; i < BUFF_SIZE; i++)
  {
    debugBuff[i] = 0;
  }
}

char * DebuggerGetBuffPointer()
{
  return debugBuff;
}
/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/
