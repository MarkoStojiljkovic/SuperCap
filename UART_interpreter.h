/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : UART_interpreter.h
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

#ifndef _UART_INTERPRETER_h
#define _UART_INTERPRETER_h

/*****************************************************************************
 * Include Files
 *****************************************************************************/


/*****************************************************************************
 * Project Specific Types
 *****************************************************************************/


/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/
void SendNextCharUARTInterpreter(); // This function will be called after transmit finished interrupt to check if there are more pending chars for sending
void UART_Interpreter_Task(char c); // // This function should be called from UART receive interrupt (when byte arrived)
void UART_Interpreter_Timeout(); // Call this from stopwatch module (after timeout period)
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

