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


/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/
void ChargerControllerInit();
void ChargerControllerSetLatency(uint16_t lat);
void ChargerControllerSetCritLow(signed int newVal, uint8_t ch);
void ChargerControllerSetCritHigh(signed int newVal, uint8_t ch);
void ChargerControllerFailSafeTask(signed int val, uint8_t ch);
int ChargerControllerRunToValueUp();
int ChargerControllerRunToValueDown();
void ChargerControllerSetTargetValue(signed int val);
void ChargerControllerSetChannel(uint8_t channel);
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

