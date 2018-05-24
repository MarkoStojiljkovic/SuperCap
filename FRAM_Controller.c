/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : FRAM_Controller.c
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
#define QUEUE_SIZE 10
#define MEASUREMENT_INFO_SIZE sizeof(measureHeader_t)

/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "FRAM_Controller.h"
#include "fram_driver.h"
#include "framLayout.h"
#include "bsp.h"

/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/

typedef struct framWriteSocket
{
  uint16_t size;
  uint8_t * data;
} framWriteSocket_t;

typedef struct framReadSocket
{
  uint32_t address;
  uint8_t * buffer;
  readFlagStatus_t * flag; // When reading is done, change this flag
  uint32_t length;
  uint32_t readCount; // How much is read so far
  uint16_t * bufferCount; // How much is buffer filled with data
} framReadSocket_t;
static framReadSocket_t currentReader;

static int readerBusy = 0; // For now reader can serve just one caller

// Queue which hold write tasks
static framWriteSocket_t queueWrite[QUEUE_SIZE]; 
static int qHeadWrite = 0;
static int qLenWrite = 0;


measureInfo_t info;
static const measureInfo_t emptyInfoStruct; // This is used for measurement reset command (struct with all zeroes)
static uint32_t currentWriteAddress;
static uint32_t currentHeaderAddress;
static int headerFlag = 0; // 0- header is closed, 1- header is opened, 2- header should be uppdated
static measureHeader_t currentMeasurementHeader;
static int eraseMeasurementFlag = 0; // Signal that erases measures, can only be activated if no writing is in process

/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
static bool FRAMControllerPopWrite(framWriteSocket_t * socket);
static void GetHeader(measureHeader_t * header, int index);
/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
void FRAMControllerInit()
{
  // Debug fill values
//  uint32_t tempAddress = 100000;
//  uint16_t val = 0;
//  uint16_t huge[1000];
//  int iter;
//  
//  for(iter = 0; iter < 1000; iter++)
//  {
//    huge[iter] = iter;
//  }
//  FRAMWriteArray(tempAddress, huge, 1000);
  
  
  
  FRAMReadArray(FRAM_MEASUREMENT_INFO_BASE, &info, FRAM_MEASUREMENT_INFO_SIZE);
  int offset = info.numOfSavedMeasurements;
  if(offset == 0)
  {
    currentHeaderAddress = FRAM_DATA_BASE; // No measurements means start from the beginning of data section
  }
  else
  {
    // Addresses need to be downloaded from FRAM
    measureHeader_t lastHeader;
    GetHeader(&lastHeader, offset - 1); // Get last header
    // Current header is last header address start + sizeof() header structure + length of measurements
    currentHeaderAddress = info.startAddr[offset - 1] + sizeof(measureHeader_t) + lastHeader.numOfPoints * 2; // data point is 16b
    
  }
  // Set current write address to next free space
  currentWriteAddress = currentHeaderAddress + sizeof(measureHeader_t);
  headerFlag = 0; // Clocse header
}

/* Call this function by giving pointer to buffer that will be written in FRAM and its length, make sure buffer does not get overwritten */
void FRAMControllerPushWrite(uint8_t * data, uint16_t len)
{
  if(len > MAX_DATA_LENGTH) return; // Limit data length
  int indexTail = (qHeadWrite + qLenWrite) % QUEUE_SIZE;
  queueWrite[indexTail].data = data;
  queueWrite[indexTail].size = len;
  qLenWrite++;
  if(qLenWrite > QUEUE_SIZE)
  {
    qHeadWrite = (qLenWrite + 1) % QUEUE_SIZE;
    qLenWrite = QUEUE_SIZE;
  }
}

/* Create "task" for reading FRAM, provide address, length and pointer to buffer in which data will be written
 * When read task is finished flag will be set
 *  */ 
void FRAMControllerAsyncRead(uint32_t addr, uint32_t len, uint8_t * buff, readFlagStatus_t * flag, uint16_t * buffLen) // TODO: maybe send whole structure instead of all arguments
{
  if(readerBusy == 1) return; // For now reader can serve only one caller and task
  
  currentReader.address = addr;
  currentReader.length = len;
  currentReader.buffer = buff;
  currentReader.flag = flag; // points to same flag which caller will check
  *(currentReader.flag) = READ_STARTED; // Just to be sure reset flag status
  
  currentReader.readCount = 0; // How much is read so far
  currentReader.bufferCount = buffLen; // this value will indicate how much current buffer is filled with data
  
  readerBusy = 1;
}


/* Check pending FRAM write tasks
 * only write if all conditions are met (header is open and there is enough space in FRAM) */
void FRAMControllerWriteTask()
{
  
  switch(headerFlag)
  {
    case 0: // header is closed, writings when header is not open are not allowed, except for erase
    {
      if(eraseMeasurementFlag == 0) return;
      // Erase is activated
      FRAMWriteArray(FRAM_MEASUREMENT_INFO_BASE, &emptyInfoStruct, FRAM_MEASUREMENT_INFO_SIZE);
      FRAMControllerInit();
      eraseMeasurementFlag = 0;
      break;
    }
    case 1: // header is open
    {
      framWriteSocket_t action;
      if(!FRAMControllerPopWrite(&action)) return; // No tasks

      // There is at least 1 task/action pending, check if there is free space
      uint32_t len = action.size;
      if((len + currentWriteAddress) >= FRAM_SIZE) return; // Ignore if not enough space
      FRAMWriteArray(currentWriteAddress, action.data, len);
      currentWriteAddress += len;
      break;
    }
    case 2: // Header should be updated
    {
      // First check if there are more pending tasks, header should only be updated if there are no other write tasks
      framWriteSocket_t action;
      if(!FRAMControllerPopWrite(&action))
      {
        // No pending tasks, update header
        FRAMWriteArray(currentHeaderAddress, &currentMeasurementHeader, sizeof(measureHeader_t));
        // Update info struct and write it to FRAM
        info.startAddr[info.numOfSavedMeasurements++] = currentHeaderAddress;
        FRAMWriteArray(FRAM_MEASUREMENT_INFO_BASE, &info, FRAM_MEASUREMENT_INFO_SIZE);
        // Set new header and write addresses 
        currentHeaderAddress = currentWriteAddress;
        currentWriteAddress = currentHeaderAddress + sizeof(measureHeader_t);
        
        headerFlag = 0; // Close header
        break;
      }

      // There is at least 1 task/action pending, check if there is free space
      uint32_t len = action.size;
      if((len + currentWriteAddress) >= FRAM_SIZE) return; // Ignore if not enough space
      FRAMWriteArray(currentWriteAddress, action.data, len);
      currentWriteAddress += len;
      break;
    }
  }
}
        
void FRAMControllerReadTask()
{
  if(readerBusy == 0) return; // If no reading is in process return
  
  if(*(currentReader.flag) == READ_NOT_COMPLETE) return; // Data is not yet processed (sent via UART), wait
  
  uint32_t readsRemaining = currentReader.length - currentReader.readCount;
  if(readsRemaining <= MAX_DATA_LENGTH)
  {
    // Reading can be done all at once
    FRAMReadArray(currentReader.address, currentReader.buffer, readsRemaining);
    *(currentReader.flag) = READ_DONE;
    *(currentReader.bufferCount) = readsRemaining; // Let other module know how much buffer is filled
    readerBusy = 0;
  }
  else
  {
    // Reading needs to be fragmented
    FRAMReadArray(currentReader.address, currentReader.buffer, MAX_DATA_LENGTH);
    currentReader.address += MAX_DATA_LENGTH;
    currentReader.readCount += MAX_DATA_LENGTH;
    *(currentReader.flag) = READ_PARTIAL;
    *(currentReader.bufferCount) = MAX_DATA_LENGTH;  // Let other module know how much buffer is filled
  }
  
}


/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/

static bool FRAMControllerPopWrite(framWriteSocket_t * socket)
{
   if(qLenWrite == 0) return false;
   
   socket->data = queueWrite[qHeadWrite].data;
   socket->size = queueWrite[qHeadWrite].size;
   qHeadWrite = (qHeadWrite + 1) % QUEUE_SIZE;
   qLenWrite--;
   return true;
}


void FRAMCreateHeader(measureHeader_t meas)
{
  if(headerFlag == 1) return; // Prevent stupid things if someone tries to open header > 1 times
  if(info.numOfSavedMeasurements >= MAX_HEADERS) return; // Return if there are reached maximum measurement blocks
  headerFlag = 1;
  // Copy whole structure, don't write it yet to FRAM, it will be written to FRAM as soon as it is closed (when we saved all data)
  currentMeasurementHeader = meas; 
}

void FRAMCloseHeader(uint16_t totalMeasuredPoints)
{
  if(headerFlag == 0) return; // Prevent stupid things if someone tries to close header > 1 times
  
  currentMeasurementHeader.numOfPoints = totalMeasuredPoints;
  headerFlag = 2; // Initiate header update
}


static void GetHeader(measureHeader_t * header, int index)
{
  if(index > info.numOfSavedMeasurements) return; // Cant fetch header that doesn't exist
  
  uint32_t headAddr = info.startAddr[index];
  FRAMReadArray(headAddr, header, sizeof(measureHeader_t));
}

// Erase measurements from FRAM, only can be erased if there is no pending data recording
void EraseMeasurements(void)
{
  eraseMeasurementFlag = 1;
}