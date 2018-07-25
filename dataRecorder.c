/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : dataRecorder.c
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
#include "dataRecorder.h"
#include "sigmaDeltaADC.h"
#include "UARTBuffer.h"
#include "FRAM_Controller.h"
#include "bsp.h"

#define BUFF_SIZE 100
#define CHANNEL_SWAP_DELAY 3
#define CHANNEL_DATA_PERIOD 2

#define DATA_REC_DEBUG_DIODES 1
#define DEBUG_DIODE LED_GREEN
// Dual channel sequence looks like pppVVpppCC , period is 10ms, p - delay, V - channel0, C - channel 1
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/
// Global function pointer which should be used in 1ms interrupt
void (*DataRecorderTask)(signed int rawData, int channel); // Function ptr which will change depending on which task is selected (continuous mode or fixed points mode), call it in 1ms interval

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/
//typedef enum
//{
//  STATE_INIT_CH, // State init 
//  STATE_CH_CHANGE, // State used to skip corrupted result when changing channels
//  STATE_CH_MEASURE, // Measure whatever channel is selected
//  STATE_MEASURE_CH0, // Measure CH0
//  STATE_MEASURE_CH1 // Measure CH1
//}calc_state_t;


//typedef void (*calcPtr_t)(int); // Typedef for function pointers
//
//// Function pointers
//static calcPtr_t calculate; // This function ptr will change function depending on what mode is selected (single or dual channel mode)

// Data buffers
signed int * dataBuffPtr; // Pointer which points to current active data buffer
signed int * dataBuffReadyPtr; // Pointer which points to current ready to send buffer
int _currentDataBufferIndex = 0; // Buffer usage counter
signed int dataBufferA[BUFF_SIZE]; // First buffer
signed int dataBufferB[BUFF_SIZE]; // Second buffer

static uint32_t _prescalerValue = 0xFFFFFFFF; // Prescaler value used by this module
static uint32_t _currentPrescalerVal = 0; // Current prescaler value used by this module
static int _enabled = 0; // Enable flag used by this modules 
static int _finished = 0; // When this flag is activated, save remaining data and close FRAM data sector.
static uint16_t _recordedPoints = 0; // Number of recorded data points
static uint16_t _targetPoints = 0xFFFF; // Target points to which data will be recorded (only used in targetPoints mode)
static uint16_t selectedChannel = 3; // Invalid starting value

static int forceFirstSampleToBeFromCH0 = 1;

// Initialization of state machines
//static calc_state_t stateCalcu = STATE_INIT_CH;

int dataRecorderReady = 1; // When this module is active ready is 0 (even if paused), otherwise its 1
/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
static void DataRecorderContinuousTask(signed int rawData, int channel);
static void DataRecorderTargetPointsTask(signed int rawData, int channel);
static void DataRecorderDummy(int rawData, int channel);
static void SwitchBufferPtr();
static void InsertInBuffer(signed int data);

/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/

// Call before main loop

void DataRecorderInit()
{
    _prescalerValue = 0xFFFFFFFF;
    _currentPrescalerVal = 0;
    _enabled = 0;
    _recordedPoints = 0;
    _targetPoints = 0;
    _currentDataBufferIndex = 0;

    dataBuffPtr = dataBufferA; // Default buffer
    dataBuffReadyPtr = dataBufferB;
    DataRecorderTask = DataRecorderDummy; // Default function
    //  calculate = CalcSingleCh; // Default function
    //  stateCalcu = STATE_INIT_CH;
    dataRecorderReady = 1;

}

// This function configures dataRecorder and runs it

void DataRecorderConfigAndRun(uint8_t chMode, uint8_t operationMode, uint32_t prescaler, uint32_t targetPoints, uint8_t * ts)
{

    // Set calculate function based on channel mode, and set corresponding ADC channel 
    if (chMode < 0 || chMode > 2)
    {
        // invalid argument
        dataRecorderReady = 1;
        return;
    }
    selectedChannel = chMode;
    
    // If dual channel mode is selected force first sample in buffer to be from CH0
    if(selectedChannel == CH_MODE_DUAL)
    {
        forceFirstSampleToBeFromCH0 = 1;
    }
    else
    {
        forceFirstSampleToBeFromCH0 = 0;
    }
        
    // Chose data recorder task function based on operation mode
    if (operationMode < 1 || operationMode > 2)
    {
        dataRecorderReady = 1;
        return; // Invalid argument
    }

    if (operationMode == 1)
    {
        DataRecorderTask = DataRecorderContinuousTask; //1
    }
    else
    {
        _targetPoints = targetPoints;
        DataRecorderTask = DataRecorderTargetPointsTask; //2
    }

    // Set new prescaler value
    _prescalerValue = prescaler;

    // Reset dataRecorder to starting state
    _currentPrescalerVal = 0;
    _recordedPoints = 0;
    _currentDataBufferIndex = 0;
    _finished = 0;
    _enabled = 1;


    measureHeader_t tempHeader;
    tempHeader.channelMode = chMode;
    tempHeader.operatingMode = operationMode;
    tempHeader.prescaler = prescaler;
    tempHeader.timestamp.year = ts[0];
    tempHeader.timestamp.month = ts[1];
    tempHeader.timestamp.day = ts[2];
    tempHeader.timestamp.hour = ts[3];
    tempHeader.timestamp.min = ts[4];
    tempHeader.timestamp.sec = ts[5];

    FRAMCreateHeader(tempHeader);
    dataRecorderReady = 0;
}

// Resume task if paused

void DataRecorderResume()
{
    _enabled = 1;
}

// Pause dataRecorder task

void DataRecorderStop()
{
    _enabled = 0;
}

// Finish DataRecorder task

void DataRecordFinish()
{
    _finished = 1;
}

/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/

/* Call this from DataRecorderTask global function pointer T1 interrupt */
static void DataRecorderContinuousTask(signed int offsetCorrectedData, int channel)
{
    if (_enabled != 1) return;
    #if (DATA_REC_DEBUG_DIODES == 1)
//    DEBUG_DIODE = 1;
    #endif

    if (_finished == 1) // This flag can be set only by commander
    {
        // Update header
        // flush buffers to FRAM _currentDataBufferIndex
        #if (DATA_REC_DEBUG_DIODES == 1)
        // Reset LEDs to original state
//        LED_RED = 0;
        DEBUG_DIODE = 0;
        #endif
        FRAMControllerPushWrite((uint8_t *) dataBuffPtr, _currentDataBufferIndex * 2); // Flush remaining data to FRAM
        FRAMCloseHeader(_recordedPoints);
        _currentDataBufferIndex = 0;
        SwitchBufferPtr();
        DataRecorderTask = DataRecorderDummy;
        dataRecorderReady = 1;
        return;
    }
    if ((selectedChannel == 2) || (selectedChannel == channel))
    {
        // First sample in buffer must be CH0 (in dual channel mode)
        if(selectedChannel == 2 && forceFirstSampleToBeFromCH0 ==1)
        {
            if(channel == 0)
            {
                forceFirstSampleToBeFromCH0 = 0;
            }
            else
            {
                // Its channel 1, skip
                return;
            }
        }
        
        if (_currentPrescalerVal < _prescalerValue)
        {
            _currentPrescalerVal++;
            return;
        }
        else
        {
            _currentPrescalerVal = 0;
            InsertInBuffer(offsetCorrectedData);
            #if (DATA_REC_DEBUG_DIODES == 1)
            if (DEBUG_DIODE == 1)
            {
                DEBUG_DIODE = 0;
            }
            else
            {
                DEBUG_DIODE = 1;
            }
            #endif
        }
    }
}

/* Call this from DataRecorderTask global function pointer in 1ms interval */
static void DataRecorderTargetPointsTask(signed int offsetCorrectedData, int channel)
{
    if (_enabled != 1) return;

    #if (DATA_REC_DEBUG_DIODES == 1)
//    LED_RED = 1;
    #endif

    if (_recordedPoints == _targetPoints)
    {
        #if (DATA_REC_DEBUG_DIODES == 1)
        // Reset LEDs to original state
//        LED_RED = 0;
        DEBUG_DIODE = 0;
        #endif
        FRAMControllerPushWrite((uint8_t *) dataBuffPtr, _currentDataBufferIndex * 2); // Flush remaining data to FRAM
        FRAMCloseHeader(_recordedPoints);
        _currentDataBufferIndex = 0;
        SwitchBufferPtr();
        DataRecorderTask = DataRecorderDummy;
        dataRecorderReady = 1;
        return;
    }

    if ((selectedChannel == 2) || (selectedChannel == channel))
    {
        // First sample in buffer must be CH0 (in dual channel mode)
        if(selectedChannel == 2 && forceFirstSampleToBeFromCH0 ==1)
        {
            if(channel == 0)
            {
                forceFirstSampleToBeFromCH0 = 0;
            }
            else
            {
                // Its channel 1, skip
                return;
            }
        }
        if (_currentPrescalerVal < _prescalerValue)
        {
            _currentPrescalerVal++;
            return;
        }
        else
        {
            _currentPrescalerVal = 0;
            InsertInBuffer(offsetCorrectedData);
            #if (DATA_REC_DEBUG_DIODES == 1)
            if (DEBUG_DIODE == 1)
            {
                DEBUG_DIODE = 0;
            }
            else
            {
                DEBUG_DIODE = 1;
            }
            #endif
        }
    }

}

static void DataRecorderDummy(int offsetCorrectedData, int channel)
{
    return;
}


// Switch pointer to point to another buffer
// TODO: Test that switching wont occur before written in FRAM

static void SwitchBufferPtr()
{
    if (dataBuffPtr == dataBufferA)
    {
        dataBuffPtr = dataBufferB;
        dataBuffReadyPtr = dataBufferA;
    }
    else
    {
        dataBuffPtr = dataBufferA;
        dataBuffReadyPtr = dataBufferB;
    }
}

static void InsertInBuffer(signed int data)
{
    if (_currentDataBufferIndex < BUFF_SIZE)
    {
        dataBuffPtr[_currentDataBufferIndex++] = data;
    }
    else
    {
        // CREATE FRAM TASK
        _currentDataBufferIndex = 0;
        SwitchBufferPtr();
        FRAMControllerPushWrite((uint8_t *) dataBuffReadyPtr, BUFF_SIZE * 2); // At this point buffer is full, 
        dataBuffPtr[_currentDataBufferIndex++] = data;
    }
    _recordedPoints++;
}