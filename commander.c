/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : commander.c
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

#define INSTRUCTION_BUFFER_SIZE 100

enum commander_state
{
  STATE_IDLE,
  STATE_INIT,
  STATE_ACTION,
  STATE_WAIT,
  STATE_ERROR
};
typedef enum
{
  STATE_WAIT_FOR_VALUE_INIT,
  STATE_WAIT_FOR_VALUE_ACTIVE
} charger_controller_SM_t;

/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include <xc.h>
#include <stdint.h>
#include "bsp.h"
#include "dataRecorder.h"
#include <stdbool.h>
#include "dataProvider.h"
#include "FRAM_Controller.h"
#include "chargerController.h"

typedef void (*instructionPtr_t)(void); // Typedef for instruction pointers




/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/
uint8_t instructionArray[INSTRUCTION_BUFFER_SIZE] = {0};
int currentInstruction = 0;
int startFlag = 0;
int totalInstructions = 0;

static uint32_t delayCounter = 0;



static charger_controller_SM_t chargerControllerInstructionState = STATE_WAIT_FOR_VALUE_INIT; // state machine variable used for rising and falling operation mode
/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
static bool DelayCommander();
static uint8_t Get8bValue();
static uint16_t Get16bValue();
static uint32_t Get32bValue();

static void ResetCommander();

// UART instructions
static void instructionLedON (void); //0
static void instructionLedOFF (void); //1
static void instructionPinSet (void); //2
static void instructionSetDelay(void);//3
static void instructionConfigDataRec(void); //4
static void instructionDataRecFinish(void); //5
static void instructionDataFetch(void); //6
static void instructionEraseMeasurements(void); // 7
static void instructionWaitForDataRecorderToFinish(void); //8
static void instructionWaitForValueRising(void); //9
static void instructionWaitForValueFalling(void); //10
static void instructionSetCriticalLow(void); // 11
static void instructionSetCriticalHigh(void); // 12
static void instructionPinSetHigh(void); // 13
static void instructionPinSetLow(void); //14
static void instructionChargerOn(void); // 15
static void instructionChargerOff(void); // 16
static void instructionDischarger100AOn(void); // 17
static void instructionDischarger100AOffS1(void); //18
static void instructionDischarger100AOffS2(void); // 19
static void instructionDischarger10AOn(void); //20
static void instructionDischarger10AOffS1(void); // 21
static void instructionDischarger10AOffS2(void); // 22
static void instructionFanoxOn(void); // 23
static void instructionFanoxOff(void); //24
static void instructionResOn(void); // 25
static void instructionResOff(void); // 26
static void instructionGetGain(void); //27
static void instructionFastChargingOn(void); //28
static void instructionFastChargingOff(void); // 29
static void instructionSendACK(void); // 30

instructionPtr_t instructionPtr[31] = 
{
  instructionLedON,       //0
  instructionLedOFF,      //1
  instructionPinSet,      //2
  instructionSetDelay,    //3
  instructionConfigDataRec, //4
  instructionDataRecFinish,   //5
  instructionDataFetch,   //6
  instructionEraseMeasurements,   //7
  instructionWaitForDataRecorderToFinish,  //8
  instructionWaitForValueRising, // 9
  instructionWaitForValueFalling, // 10
  instructionSetCriticalLow,        //11
  instructionSetCriticalHigh,       //12
  instructionPinSetHigh,            //13
  instructionPinSetLow,             //14
  instructionChargerOn,             //15
  instructionChargerOff,            //16
  instructionDischarger100AOn,      //17
  instructionDischarger100AOffS1,   //18
  instructionDischarger100AOffS2,   //19
  instructionDischarger10AOn,       //20
  instructionDischarger10AOffS1,    //21
  instructionDischarger10AOffS2,    //22
  instructionFanoxOn,               //23
  instructionFanoxOff,              //24
  instructionResOn,                 //25
  instructionResOff,                //26
  instructionGetGain,               //27
  instructionFastChargingOn,        //28
  instructionFastChargingOff,       //29
  instructionSendACK,               //30
};
/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/

static enum commander_state state = STATE_IDLE;
void CommanderTask(void)
{
  switch (state)
  {
    case STATE_IDLE:
    {
      if(startFlag != 0) state = STATE_INIT;
      break;
    }
    
    case STATE_INIT:
    { // Initial state 
      currentInstruction = 0;
      startFlag = 0;
      state = STATE_ACTION;
      break;
    }
    case STATE_ACTION: 
    {
      if(!DelayCommander()) break; // Skip action if delay is activated
      // Implement wait for function
      if(currentInstruction >= totalInstructions)
      {
        // All instructions executed go back in idle
        state = STATE_IDLE;
        break;
      }
      uint8_t op = instructionArray[currentInstruction]; // Get instruction pointed by PC (program counter)
      // Implement failsafe mechanism
      instructionPtr[op](); //executeSMInstruction
      break;
    }
    case STATE_ERROR:
    {
      break; // Just return for now
    }
    default : break;
  }
  return;
} // End RuleEngineSM




void FillInstructionArray(uint8_t* buff, int len)
{
  ResetCommander(); // this should be done prior to filling
  if(len > INSTRUCTION_BUFFER_SIZE) return;
  
  int i;
  for(i=0; i <len; i++)
  {
    instructionArray[i] = buff[i];
  }
  totalInstructions = len;
}

void DelayTick()
{
  if(delayCounter != 0) delayCounter--;
}

/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/
static bool DelayCommander()
{
  if(delayCounter != 0) return false;
  
  // Its 0, timeout has passed
  return true;
}

static uint8_t Get8bValue()
{
  return instructionArray[currentInstruction++];
}

static uint16_t Get16bValue()
{
  uint16_t temp = ((uint16_t)instructionArray[currentInstruction++]) << 8;
  temp |= (uint16_t)instructionArray[currentInstruction++];
  return temp;
}

static uint32_t Get32bValue()
{
  uint16_t temp = Get16bValue();
  uint32_t temp32 = (uint32_t)temp << 16;
  temp32 |= (uint32_t)Get16bValue();
  return temp32;
}

static void ResetCommander()
{
  delayCounter = 0;
  currentInstruction = 0;
  totalInstructions = 0;
  startFlag = 0;
  chargerControllerInstructionState = STATE_WAIT_FOR_VALUE_INIT; // Reset charger controller internal state
  state = STATE_IDLE;
}




static void instructionLedON (void)//0
{
  currentInstruction++; // point to next location which hold LED ID
  uint8_t ledNum = instructionArray[currentInstruction];
  switch(ledNum)
  {
    case 1:
    {
      LED_RED = 1;
      break;
    }
    case 2:
    {
      LED_GREEN = 1;
      break;
    }
    case 3:
    {
      LED_YELLOW = 1;
      break;
    }
    default: break;
  }
  currentInstruction++;
}

static void instructionLedOFF (void) //1
{
  currentInstruction++; // point to next location which hold LED ID
  uint8_t ledNum = instructionArray[currentInstruction];
  switch(ledNum)
  {
    case 1:
    {
      LED_RED = 0;
      break;
    }
    case 2:
    {
      LED_GREEN = 0;
      break;
    }
    case 3:
    {
      LED_YELLOW = 0;
      break;
    }
    default: break;
  }
  currentInstruction++;
}

static void instructionPinSet (void) //2    Sets P6 pin, DEBUG FUNCTION, IGNORE
{
  currentInstruction++; // point to next loc
  TP6_PIN = 1;
}

static void instructionSetDelay (void) //3   Set delay to the commander, loc1 & loc2 & loc3 & loc4 forms 32bit mseconds value
{
  currentInstruction++; // point to next loc
  uint16_t temp = Get16bValue();
  delayCounter = (uint32_t)temp << 16;
  delayCounter |= (uint32_t)Get16bValue();
}

static void instructionConfigDataRec(void) //4
{
  currentInstruction++; // point to next loc
  uint8_t chMode = Get8bValue();
  uint8_t opMode = Get8bValue();
  uint32_t presc = Get32bValue();
  uint32_t targetPts = Get32bValue();
  uint8_t time[6];
  int i;
  for(i = 0; i < 6; i++)
  {
      time[i] = Get8bValue();
  }
  
  DataRecorderConfigAndRun(chMode, opMode, presc, targetPts, time);
}

static void instructionDataRecFinish(void) //5
{
  currentInstruction++; // point to next loc
  DataRecordFinish();
}

static void instructionDataFetch(void) //6
{
  currentInstruction++; // point to next loc
  uint32_t addr = Get32bValue();
  uint16_t len = Get16bValue();
  DataProviderFetchData(addr, len);
}

static void instructionEraseMeasurements(void) //7
{
  currentInstruction++; // point to next loc
  EraseMeasurements();
}

static void instructionWaitForDataRecorderToFinish(void) //8
{
  // Keep this instruction executed until dataRecorder ready flag is set
  if(dataRecorderReady != 1) return;
  currentInstruction++; // dataRecorder finished its task, point to next location/instruction
}


static void instructionWaitForValueRising(void) // 9
{
  switch(chargerControllerInstructionState)
  {
    case STATE_WAIT_FOR_VALUE_INIT:
    {
      currentInstruction++;
      ChargerControllerSetChannel(Get8bValue());
      ChargerControllerSetLatency(Get16bValue());
      ChargerControllerSetTargetValue(Get16bValue());
      chargerControllerInstructionState = STATE_WAIT_FOR_VALUE_ACTIVE;
      currentInstruction -= 6; // Repeat this instruction until value is reached
      // Go to next state immediately
    }
    case STATE_WAIT_FOR_VALUE_ACTIVE:
    {
      if(ChargerControllerRunToValueUp())
      {
        chargerControllerInstructionState = STATE_WAIT_FOR_VALUE_INIT;
        currentInstruction += 6; // point to location after this instruction
      }
      break;
    }
    default:
    {
      // This shouldn't happen, just go to next instruction
      currentInstruction += 6; // point to next loc
      chargerControllerInstructionState = STATE_WAIT_FOR_VALUE_INIT;
    }
  }
}

static void instructionWaitForValueFalling(void) // 10
{
  switch(chargerControllerInstructionState)
  {
    case STATE_WAIT_FOR_VALUE_INIT:
    {
      currentInstruction++;
      ChargerControllerSetChannel(Get8bValue());
      ChargerControllerSetLatency(Get16bValue());
      ChargerControllerSetTargetValue(Get16bValue());
      chargerControllerInstructionState = STATE_WAIT_FOR_VALUE_ACTIVE;
      currentInstruction -= 6; // Repeat this instruction until value is reached
      // Go to next state immediately
    }
    case STATE_WAIT_FOR_VALUE_ACTIVE:
    {
      if(ChargerControllerRunToValueDown())
      {
        chargerControllerInstructionState = STATE_WAIT_FOR_VALUE_INIT;
        currentInstruction += 6; // point to location after this instruction
      }
      break;
    }
    default:
    {
      // This shouldn't happen, just go to next instruction
      currentInstruction += 6; // point to next loc
      chargerControllerInstructionState = STATE_WAIT_FOR_VALUE_INIT;
    }
  }
}

static void instructionSetCriticalLow() // 11
{
    currentInstruction++;
    uint16_t temp = Get16bValue();
    ChargerControllerSetCritLow(temp, Get8bValue());
}

static void instructionSetCriticalHigh() // 12
{
    currentInstruction++;
    uint16_t temp = Get16bValue();
    ChargerControllerSetCritHigh(temp, Get8bValue());
}

static void instructionPinSetHigh() //13
{
    currentInstruction++;
    uint8_t pinId = Get8bValue();
    switch(pinId)
    {
        case 51: RES_EN_PIN = 1; break;
        case 52: FANOX_EN_PIN = 1; break;
        case 53: SEL_MEASURE_100_10_PIN = 1; break;
        case 55: CHARGER_EN_PIN = 1; break;
        case 60: REF_100_10_PIN = 1; break;
        case 61: SWITCH_10A_PIN = 1; break;
        case 62: SWITCH_100A_PIN = 1; break;
        case 63: DISCH_EN_PIN = 1; break;
        default: break;
    }   
}

static void instructionPinSetLow() //14
{
    currentInstruction++;
    uint8_t pinId = Get8bValue();
    switch(pinId)
    {
        case 51: RES_EN_PIN = 0; break;
        case 52: FANOX_EN_PIN = 0; break;
        case 53: SEL_MEASURE_100_10_PIN = 0; break;
        case 55: CHARGER_EN_PIN = 0; break;
        case 60: REF_100_10_PIN = 0; break;
        case 61: SWITCH_10A_PIN = 0; break;
        case 62: SWITCH_100A_PIN = 0; break;
        case 63: DISCH_EN_PIN = 0; break;
        default: break;
    }   
}

static void instructionChargerOn() // 15
{
    currentInstruction++;
    FANOX_EN_PIN = 0;
    RES_EN_PIN = 0;
    SWITCH_10A_PIN = 0;
    SWITCH_100A_PIN =0;
    DISCH_EN_PIN = 1;  // complementary logic
    ODRZAVANJE = 1;
    CHARGER_EN_PIN = 1;
}

static void instructionChargerOff() // 16
{
    currentInstruction++;
    CHARGER_EN_PIN = 0;
}

static void instructionDischarger100AOn() // 17
{
    currentInstruction++;
    CHARGER_EN_PIN = 0;
    REF_100_10_PIN = 0;
    SEL_MEASURE_100_10_PIN = 1;
    SWITCH_10A_PIN = 0;
    FANOX_EN_PIN = 0;
    RES_EN_PIN = 0;
    DISCH_EN_PIN = 0; // complementary logic
    SWITCH_100A_PIN = 1;
}

// Turning off discharger is composed of 2 stages which should be apart by some time (1s), this is first
static void instructionDischarger100AOffS1() //18
{
    currentInstruction++;
    SWITCH_100A_PIN = 0;
}

// Stage 2 discharger off instruction
static void instructionDischarger100AOffS2() // 19
{
    currentInstruction++;
    DISCH_EN_PIN = 1;
}


static void instructionDischarger10AOn() //20
{
    currentInstruction++;
    CHARGER_EN_PIN = 0;
    REF_100_10_PIN = 1;
    SEL_MEASURE_100_10_PIN = 0;
    SWITCH_100A_PIN = 0;
    FANOX_EN_PIN = 0;
    RES_EN_PIN = 0;
    DISCH_EN_PIN = 0;
    SWITCH_10A_PIN = 1;
}

// Turning off discharger is composed of 2 stages which should be apart by some time (1s), this is first
static void instructionDischarger10AOffS1() // 21
{
    currentInstruction++;
    SWITCH_10A_PIN = 0;
}

// Stage 2 discharger off instruction
static void instructionDischarger10AOffS2() // 22
{
    currentInstruction++;
    DISCH_EN_PIN = 1;
}

static void instructionFanoxOn() // 23
{
    currentInstruction++;
    CHARGER_EN_PIN = 0;
    SWITCH_100A_PIN = 0;
    DISCH_EN_PIN = 1;
    SWITCH_10A_PIN = 0;
    RES_EN_PIN = 0;
    FANOX_EN_PIN = 1;
}

static void instructionFanoxOff() //24
{
    currentInstruction++;
    FANOX_EN_PIN = 0;
}

static void instructionResOn() // 25
{
    currentInstruction++;
    CHARGER_EN_PIN = 0;
    SWITCH_100A_PIN = 0;
    DISCH_EN_PIN = 1;
    SWITCH_10A_PIN = 0;
    FANOX_EN_PIN = 0;
    RES_EN_PIN = 1;
}

static void instructionResOff() // 26
{
    currentInstruction++;
    RES_EN_PIN = 0;
}

static void instructionGetGain() // 27
{
    currentInstruction++;
    DataProviderFetchGain();
}

static void instructionFastChargingOn() //28
{
    currentInstruction++;
    ODRZAVANJE = 1;
}

static void instructionFastChargingOff() //29
{
    currentInstruction++;
    ODRZAVANJE = 0; // Miksa je imenovao pin
}

static void instructionSendACK() // 30
{
    currentInstruction++;
    DataProviderSendACK();
}