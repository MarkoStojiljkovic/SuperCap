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
#define CRITICAL_LATENCY    4 // Number of samples needed to be above/below threshold to trigger shut down
#define TIME_BETWEEN_STAGES_IN_MS 200

/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "chargerController.h"
#include "sigmaDeltaADC.h"
#include "fram_driver.h"
#include "framLayout.h"
#include "bsp.h"
#include "globals.h"
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/
int g_DisableDischarger = 0;
int g_DisableCharger = 0;
int g_chargerControllerDelayBetweenStages = 0;
int g_chargerFailSafeTaskDelayVariable = 0;
/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/

// Cutoff values for ADC, if surpassed charger/discharger is immediately disabled 
static signed int valueLowCH0 = INT16_MIN_VALUE;
static signed int valueHighCH0 = INT16_MAX_VALUE; // Max int16 value (positive)
static signed int valueLowCH1 = INT16_MIN_VALUE;
static signed int valueHighCH1 = INT16_MAX_VALUE; // Max int16 value (positive)
static signed int cutoffValueCH1 = INT16_MAX_VALUE; // Value below this point is unsafe and all charger actions need to be stopped


static uint16_t latencyTarget = 0;
static uint16_t currentLatency = 0;
static uint16_t currentCritLowLatency = 0;
static uint16_t currentCritHighLatency = 0;
static uint16_t cutoffLatency = 0;
static signed int lastSampledValue = 0;
static signed int targetValue = 0; // Value to which runTo task will go
static uint8_t selectedChannel = 0;
static uint8_t currentSampleChannel = 0;

static int syncADCResult = 0; // Set this flag when new sample arrives so wait for value functions don't decide actions on repeated sample
#if (USE_35_BOARD == 1)
static int syncChargerFailsafe = 0; // Set this flag when CH1 sample is set to new value
#endif
/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/

/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/

void ChargerControllerInit() // Call this function after ADC is initialized
{
    FRAMReadArray(FRAM_CH0_LOW_LIMIT, &valueLowCH0, sizeof (valueLowCH0));
    FRAMReadArray(FRAM_CH0_HIGH_LIMIT, &valueHighCH0, sizeof (valueHighCH0));

    FRAMReadArray(FRAM_CH1_LOW_LIMIT, &valueLowCH1, sizeof (valueLowCH1));
    FRAMReadArray(FRAM_CH1_HIGH_LIMIT, &valueHighCH1, sizeof (valueHighCH1));
    FRAMReadArray(FRAM_CUTOFF_CH1, &cutoffValueCH1, sizeof (cutoffValueCH1));
}

/* Skip first N samples that are above/below target value  */
void ChargerControllerSetLatency(uint16_t lat)
{
    latencyTarget = lat;
    currentLatency = 0;
}

void ChargerControllerSetCritLow(signed int newVal, uint8_t ch)
{
    if (ch == 0)
    {
        valueLowCH0 = newVal;
        // Update FRAM, this is allowed because this task is called from commander
        FRAMWriteArray(FRAM_CH0_LOW_LIMIT, &newVal, sizeof (newVal)); // Update offset uncorrected value
    }
    else
    {
        // For now every other value is CH1
        valueLowCH1 = newVal;
        // Update FRAM, this is allowed because this task is called from commander
        FRAMWriteArray(FRAM_CH1_LOW_LIMIT, &newVal, sizeof (newVal)); // Update offset uncorrected value
    }
}

void ChargerControllerSetCritHigh(signed int newVal, uint8_t ch)
{
    if (ch == 0)
    {
        valueHighCH0 = newVal;
        // Update FRAM, this is allowed because this task is called from commander
        FRAMWriteArray(FRAM_CH0_HIGH_LIMIT, &valueHighCH0, sizeof (valueHighCH0)); // Update offset uncorrected value
    }
    else
    {
        // For now every other value is CH1
        valueHighCH1 = newVal;
        // Update FRAM, this is allowed because this task is called from commander
        FRAMWriteArray(FRAM_CH1_HIGH_LIMIT, &valueHighCH1, sizeof (valueHighCH1)); // Update offset uncorrected value
    }
}

void ChargerControllerSetCutoffValue(signed int newVal)
{
    cutoffValueCH1 = newVal;
    FRAMWriteArray(FRAM_CUTOFF_CH1, &cutoffValueCH1, sizeof (cutoffValueCH1));
}

/* Call this from interrupt */
void ChargerControllerFailSafeTask(signed int val, uint8_t ch)
{
    // Update current ADC state, new sample and which channel for other tasks
    currentSampleChannel = ch;
    lastSampledValue = val;
    syncADCResult = 1; // Announce that new sample arrived
    // Check fail safe conditions
    if (ch == 1)
    {
        // Sample is from CH1, this occurs every 10ms 
        #if (USE_35_BOARD == 1)
        syncChargerFailsafe = 1;
        #endif
        // Discharger fail safe
        if (lastSampledValue < valueLowCH1 && valueLowCH1 != INT16_MIN_VALUE)
        {
            if(currentCritLowLatency >= CRITICAL_LATENCY)
            {
                g_DisableDischarger = 1;
            }
            else
                currentCritLowLatency++;
        }
        else
        {
            currentCritLowLatency = 0;
            g_DisableDischarger = 0;
        }

        // Charger fail safe
        if (lastSampledValue > valueHighCH1 && valueHighCH1 != INT16_MIN_VALUE)
        {
            if(currentCritHighLatency >= CRITICAL_LATENCY)
            {
                g_DisableCharger = 1;
            }
            else
                currentCritHighLatency++;
        }
        else
        {
            currentCritHighLatency = 0;
            g_DisableCharger = 0;
        }
        
        // Cutoff failsafe
        if(cutoffValueCH1 != INT16_MIN_VALUE)
        {
            if(lastSampledValue < cutoffValueCH1)
            {
                if(cutoffLatency >= CRITICAL_LATENCY)
                {
                    // Request shutdown for both modules
                    g_DisableCharger = 1;
                    g_DisableDischarger = 1;
                }
                else
                    cutoffLatency++;
            }
            else
            {
//                g_DisableCharger = 0;
//                g_DisableDischarger = 0;
                cutoffLatency = 0;
            }
        }
        
    }
    // Ignore samples from CH0
}

int ChargerControllerRunToValueUp()
{
    if (syncADCResult == 1) // Use every sample once for decision
    {
        syncADCResult = 0;
        // First check is it selected channel
        if (selectedChannel == currentSampleChannel)
        {
            // Channel is matched, now check values
            if (lastSampledValue >= targetValue)
            {
                currentLatency++;
                if (currentLatency >= latencyTarget)
                {
                    return 1;
                }
            }
            else
            {
                currentLatency = 0; // Value is below threshold, it was probably noise
            }
        }
    }
    return 0;
}

int ChargerControllerRunToValueDown()
{
    if (syncADCResult == 1) // Use every sample once for decision
    {
        syncADCResult = 0;
        // First check is it selected channel
        if (selectedChannel == currentSampleChannel)
        {
            // Channel is matched, now check values
            if (lastSampledValue <= targetValue)
            {
                currentLatency++;
                if (currentLatency >= latencyTarget)
                {
                    return 1;
                }
            }
            else
            {
                currentLatency = 0; // Value is below threshold, it was probably noise
            }
        }
    }
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

SHUTDOWN_RESPONSE_t ChargerControllerDisableAllActions()
{
    static int state = 0;
    switch(state)
    {
        case 0:
        {
            #if (USE_35_BOARD == 1)
            // Disable all charging logic
            EN_CH_SW = 0;
            EN_CHARGE = 1; // Komplementarna logika
            DICH_SW_35A = 0;
//            FANOX_CONTROL = 0;
            GATE_RES_CONTROL = 0;
            #else
            // Disable charger
            CHARGER_EN_PIN = 0;
            // Disable S1 for discharger
            SWITCH_100A_PIN = 0;
            SWITCH_10A_PIN = 0;
            // Disable discharging thru resistor
            RES_EN_PIN = 0;
            #endif
            g_chargerControllerDelayBetweenStages = TIME_BETWEEN_STAGES_IN_MS;
            state = 1;
            break;
        }
        case 1:
        {
            // When timeout occurs, execute STAGE2 shutdown
            if(g_chargerControllerDelayBetweenStages != 0) break;
            #if (USE_35_BOARD == 1)
            DISABLE_DISCHARGER = 1; // Komplementarna logika
            #else
            // Disable S2
            DISCH_EN_PIN = 1; // Complementary logic
            #endif
            state = 0;
            return RESPONSE_READY;
        }
    }
    return RESPONSE_NOT_READY;
}

#if (USE_35_BOARD == 1 && USE_FALSE_CHARGING_PROTECTION == 1)
void FalseChargingFailSafeTask()
{
    static signed int lastCH1Value = 0;
    static float lastCH1ValueFloat = 0;
    const float fTarget = 1500.0;
    const float gain = 7.8;
    static int state = 0;
    
    if(syncChargerFailsafe == 0) return; // Ignore if there isn't new sample
    syncChargerFailsafe = 0;
    // Just to be sure check if sample is from CH1
    if(currentSampleChannel != 1) return;
    lastCH1Value = lastSampledValue;
    // Check if charging is active
    if(EN_CHARGE_READ == 0 && EN_CH_SW_READ == 1)
    {
        // So far, charging is active and we have newest sample from CH1
        // Convert to milivolts
        lastCH1ValueFloat =  (lastCH1Value * 3300.0) / 32768.0 * gain;
        switch(state)
        {
            case 0:
            {
                if(lastCH1ValueFloat < fTarget)
                {
                    g_chargerFailSafeTaskDelayVariable = 1000; // Wait 1 second
                    state = 1;
                }
                break;
            }
            case 1:
            {
                if(lastCH1ValueFloat > fTarget)
                {
                    // This is normal behavior, go to initial state
                    state = 0;
                }
                else
                {
                    if(g_chargerFailSafeTaskDelayVariable == 0)
                    {
                        // Timeout, voltage didnt reached given threshold, turn off charger and return to initial state
                        EN_CH_SW = 0;
                        EN_CHARGE = 1; // Komplementarna logika
                        state = 0;
                    }
                }
                break;
            }
        }
    }
    else
    {
        // Reset SM, charging is disabled
        state = 0;
    }
    
}
#endif
/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/