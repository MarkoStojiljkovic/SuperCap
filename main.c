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
#include "blinky.h"
#include "chargerController.h"
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/



/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
static void Clear_WDT (void);
static void FailSafeTask();
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
  
  int debugSizeof = sizeof(measureInfo_t);
  
  Init();
    while(1)
    {
      FailSafeTask();
      CommanderTask();
      FRAMControllerWriteTask();
      FRAMControllerReadTask();
      DataProviderTask();
      BlinkLEDDeviceStatus();
      #if (USE_35_BOARD == 1 && USE_FALSE_CHARGING_PROTECTION == 1)
      FalseChargingFailSafeTask();
      #endif
      Clear_WDT();
    }
}

// Separate function because optimization "issue" (according to forum )
static void Clear_WDT (void) 
{
     ClrWdt();
}

#define TIME_BETWEEN_STAGES_IN_MS 200
// SM for disabling charging/discharging 
static int disableDischargingSM = 0;
static int disableChargingSM = 0;
int g_failSafeChargeDelay = TIME_BETWEEN_STAGES_IN_MS;
int g_failSafeDischargeDelay = TIME_BETWEEN_STAGES_IN_MS;

static void FailSafeTask()
{
    // Discharger state machine
    switch(disableDischargingSM)
    {
        // Disable all discharging related stuff, some modules can't be shut down instantly so SM is needed
        case 0:
        {
            if(g_DisableDischarger != 0)
            {
                #if (USE_35_BOARD == 1)
                DICH_SW_35A = 0;
                GATE_RES_CONTROL = 0;
                #else
                // Disable S1
                SWITCH_100A_PIN = 0;
                SWITCH_10A_PIN = 0;
                // Disable RES discharging
                RES_EN_PIN = 0;
                #endif
                
                disableDischargingSM = 1;
                g_failSafeDischargeDelay = TIME_BETWEEN_STAGES_IN_MS;
            }
            break;
        }
        case 1:
        {
            // When timeout go to next state
            if(g_failSafeDischargeDelay != 0) break;
            disableDischargingSM = 2;
            break;
        }
        case 2:
        {
            #if (USE_35_BOARD == 1)
            DISABLE_DISCHARGER = 1; // Komplementarna logika
            #else
            // Disable S2
            DISCH_EN_PIN = 1; // Complementary logic
            #endif
            
            disableDischargingSM = 0;
            break;
        }
    }
    
    // Charger state machine
    switch(disableChargingSM)
    {
        // Disable all charging related stuff, some modules can't be shut down instantly so SM is needed
        case 0:
        {
            if(g_DisableCharger != 0)
            {
                #if (USE_35_BOARD == 1)
                EN_CH_SW = 0;
                EN_CHARGE = 1; // Komplementarna logika
                #else
                // Disable S1
                CHARGER_EN_PIN = 0;
                #endif
                disableChargingSM = 1;
                // Check if fanox needs to be disabled
                g_failSafeChargeDelay = TIME_BETWEEN_STAGES_IN_MS;
                
                
            }
            break;
        }
        case 1:
        {
            // When timeout go to next state
            if(g_failSafeChargeDelay != 0) break;
            disableChargingSM = 2;
            break;
        }
        case 2:
        {
            #if (USE_35_BOARD == 1)
            #else
            #endif
            disableChargingSM = 0;
            break;
        }
    }
}
