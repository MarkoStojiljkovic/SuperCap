/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : chargerController.c
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

#define CHARGER_CONTROLLER_DEBUG 0

/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "chargerController.h"
#include "sigmaDeltaADC.h"
#include "fram_driver.h"
#include "framLayout.h"
#include "bsp.h"
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/

// Cutoff values for ADC, if surpassed charger/discharger is immediately disabled 
static signed int valueLowCH0 = 0;
static signed int valueHighCH0 = 0x7FFF; // Max int16 value (positive)
static signed int valueLowCH1 = 0;
static signed int valueHighCH1 = 0x7FFF; // Max int16 value (positive)

static uint16_t latency = 0;
static uint16_t currentLatency = 0;
static signed int lastSampledValue = 0;
static signed int targetValue = 0; // Value to which runTo task will go
static uint8_t selectedChannel = 0;
static uint8_t currentSampleChannel = 0;
/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/

/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/

void ChargerControllerInit() // Call this function after ADC is initialized
{
    FRAMReadArray(FRAM_CH0_LOW_LIMIT, &valueLowCH0, sizeof (valueLowCH0));
    valueLowCH0 -= SDOffset; // Take offset into consideration
    FRAMReadArray(FRAM_CH0_HIGH_LIMIT, &valueHighCH0, sizeof (valueHighCH0));
    valueHighCH0 -= SDOffset; // Take offset into consideration

    FRAMReadArray(FRAM_CH1_LOW_LIMIT, &valueLowCH1, sizeof (valueLowCH1));
    valueLowCH1 -= SDOffset; // Take offset into consideration
    FRAMReadArray(FRAM_CH1_HIGH_LIMIT, &valueHighCH1, sizeof (valueHighCH1));
    valueHighCH1 -= SDOffset; // Take offset into consideration

}

/* Skip first N samples that are above/below target value  */
void ChargerControllerSetLatency(uint16_t lat)
{
    // NOT IMPLEMENTED YET
    latency = lat;
    currentLatency = 0;
}

void ChargerControllerSetCritLow(signed int newVal, uint8_t ch)
{
    if (ch == 0)
    {
        valueLowCH0 = newVal - SDOffset;
        // Update FRAM, this is allowed because this task is called from commander
        FRAMWriteArray(FRAM_CH0_LOW_LIMIT, &newVal, sizeof (newVal)); // Update offset uncorrected value
    }
    else
    {
        // For now every other value is CH1
        valueLowCH1 = newVal - SDOffset;
        // Update FRAM, this is allowed because this task is called from commander
        FRAMWriteArray(FRAM_CH1_LOW_LIMIT, &newVal, sizeof (newVal)); // Update offset uncorrected value
    }
}

void ChargerControllerSetCritHigh(signed int newVal, uint8_t ch)
{
    if (ch == 0)
    {
        valueHighCH0 = newVal - SDOffset;
        // Update FRAM, this is allowed because this task is called from commander
        FRAMWriteArray(FRAM_CH0_HIGH_LIMIT, &newVal, sizeof (newVal)); // Update offset uncorrected value
    }
    else
    {
        // For now every other value is CH1
        valueHighCH1 = newVal - SDOffset;
        // Update FRAM, this is allowed because this task is called from commander
        FRAMWriteArray(FRAM_CH1_HIGH_LIMIT, &newVal, sizeof (newVal)); // Update offset uncorrected value
    }
}

/* Call this from interrupt */
void ChargerControllerFailSafeTask(signed int val, uint8_t ch)
{
    // Update current ADC state, new sample and which channel
    currentSampleChannel = ch;
    lastSampledValue = val;

    // Check fail safe conditions

    if (ch == 0)
    {
        // Current sample is from CH0
        if ((lastSampledValue < valueLowCH0) || (lastSampledValue > valueHighCH0))
        {
            // Turn off charger
            #if (CHARGER_CONTROLLER_DEBUG == 1)
            LED_RED = 1;
            #endif
        }
        else
        {
            #if (CHARGER_CONTROLLER_DEBUG == 1)
            LED_RED = 0;
            #endif
        }
    }
    else
    {
        // Current sample is from CH1
        if ((lastSampledValue < valueLowCH1) || (lastSampledValue > valueHighCH1))
        {
            // Turn off charger
            #if (CHARGER_CONTROLLER_DEBUG == 1)
            LED_BLUE = 1;
            #endif
        }
        else
        {
            #if (CHARGER_CONTROLLER_DEBUG == 1)
            LED_BLUE = 0;
            #endif
        }
    }
}

int ChargerControllerRunToValueUp()
{
    // First check is it selected channel
    if (selectedChannel != currentSampleChannel) return 0;

    // Channel is matched, now check values
    if (lastSampledValue >= targetValue)
    {
        return 1;
    }
    else
        return 0;
}

int ChargerControllerRunToValueDown()
{
    // First check is it selected channel
    if (selectedChannel != currentSampleChannel) return 0;

    // Channel is matched, now check values
    if (lastSampledValue <= targetValue)
    {
        return 1;
    }
    else
        return 0;
}

void ChargerControllerSetTargetValue(signed int val)
{
    targetValue = val;
}

void ChargerControllerSetChannel(uint8_t channel)
{
    selectedChannel = channel;
}
/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/
