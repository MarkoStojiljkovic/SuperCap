/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : main.c
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
#include "globals.h"//global change
#include "main.h"
#include "komunikacija.h"
#include "merenja.h"
#include "init.h"
#include "fram_driver.h"
#include "rtc.h"
#include "commander.h"
#include "FRAM_Controller.h"
#include "dataProvider.h"
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/
uint16_t drzanje=100;
volatile uint16_t* currentfilltr;
volatile uint16_t* voltagefilltr;





/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/

/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
  
/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/
int main(void) 
{
  //save reset flags for later examination and clear them in hw...
  int RCONOnStartup = RCON;
  RCONbits.TRAPR = 0;
  RCONbits.IOPUWR = 0;
  RCONbits.CM = 0;
  RCONbits.EXTR = 0;
  RCONbits.SWR = 0;
  RCONbits.WDTO = 0;
  RCONbits.SLEEP = 0;
  RCONbits.IDLE = 0;
  RCONbits.BOR = 0;
  RCONbits.POR = 0;
  RCONOnStartup = RCONOnStartup; // Dummy instruction to remove warning
  
  
  Init();
    while(1)
    {
      CommanderTask();
      FRAMControllerWriteTask();
      FRAMControllerReadTask();
      DataProviderTask();
    }
}

