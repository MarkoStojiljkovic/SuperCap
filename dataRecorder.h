/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : dataRecorder.h
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

#ifndef _DATA_RECORDER_h
#define _DATA_RECORDER_h

/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include <stdint.h>

/*****************************************************************************
 * Project Specific Types
 *****************************************************************************/
#define CH_MODE_CH0 0 
#define CH_MODE_CH1 1
#define CH_MODE_DUAL 2

#define OP_MODE_CONTINUOUS 1
#define OP_MODE_FIXED_POINTS 2
/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/
// Call before main loop
void DataRecorderInit();

// This function configures dataRecorder and runs it
void DataRecorderConfigAndRun(uint8_t chMode, uint8_t operationMode, uint32_t prescaler, uint32_t targetPoints, uint8_t * ts);

// Resume task if paused
void DataRecorderResume();

// Pause dataRecorder task
void DataRecorderStop();

// Finish DataRecorder task
void DataRecordFinish();
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/
extern void (*DataRecorderTask)(int rawData, int channel);
extern int dataRecorderReady;
/*****************************************************************************
 * Macros
 *****************************************************************************/

/*****************************************************************************
 * Constants
 *****************************************************************************/

#endif

