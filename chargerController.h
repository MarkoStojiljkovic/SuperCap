/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : chargerController.h
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

#ifndef _CHARGER_CONTROLLER_h
#define _CHARGER_CONTROLLER_h

/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include <stdint.h>

/*****************************************************************************
 * Project Specific Types
 *****************************************************************************/
typedef enum
{
    RESPONSE_NOT_READY,
    RESPONSE_READY,
    RESPONSE_ERROR
} SHUTDOWN_RESPONSE_t;

/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/
void ChargerControllerInit();
void ChargerControllerSetLatency(uint16_t lat);
void ChargerControllerSetCritLow(signed int newVal, uint8_t ch);
void ChargerControllerSetCritHigh(signed int newVal, uint8_t ch);
void ChargerControllerSetCutoffValue(signed int newVal);
void ChargerControllerFailSafeTask(signed int val, uint8_t ch);
int ChargerControllerRunToValueUp();
int ChargerControllerRunToValueDown();
void ChargerControllerSetTargetValue(signed int val);
void ChargerControllerSetChannel(uint8_t channel);
SHUTDOWN_RESPONSE_t ChargerControllerDisableAllActions();
#if (USE_35_BOARD == 1 && USE_FALSE_CHARGING_PROTECTION == 1)
void FalseChargingFailSafeTask();
#endif
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

