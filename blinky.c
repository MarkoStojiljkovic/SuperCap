/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : blinky.c
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
#include "blinky.h"
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/
#define HALF_PERIOD_INTERVAL_IN_MS 1000
#define LED LED_RED


int32_t g_blinkyCounter = HALF_PERIOD_INTERVAL_IN_MS;
/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/

/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/

/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/

// Blink selected LED, with selected period to display that device is working normaly
void BlinkLEDDeviceStatus()
{
    static int state = 0;
    
    switch(state)
    {
        case 0:
        {
            if(g_blinkyCounter == 0)
            {
                state = 1;
                g_blinkyCounter = HALF_PERIOD_INTERVAL_IN_MS;
                LED = 1;
            }
            break;
        }
        case 1:
        {
            if(g_blinkyCounter == 0)
            {
                state = 0;
                g_blinkyCounter = HALF_PERIOD_INTERVAL_IN_MS;
                LED = 0;
            }
            break;
        }
        default: state = 0; // reset SM
    }
}
/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/
