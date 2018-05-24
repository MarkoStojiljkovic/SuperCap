/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : fram_driver.c
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
#include "fram_driver.h"
#include "i2c.h"
#include <xc.h>


#define FRAM_SLAVE_ID                   0x00 // 0000 0000  , address is hardcoded at 0b000
#define FRAM_SLAVE_SELECT               0xA0 // 1010 0000 , from FRAM datasheet
#define FRAM_SLAVE_ADDRESS              (FRAM_SLAVE_ID | FRAM_SLAVE_SELECT)

/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/

/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
static int GetPage(uint32_t address);
/*****************************************************************************
 * Global Functions (Definitions) I2C_WriteArray(uint16_t masterCode, uint16_t address, const void * data, uint16_t size);
 *****************************************************************************/
bool FRAMWriteArray(uint32_t address, const void * data, uint16_t size)
{
  uint16_t page = GetPage(address);
  
  uint16_t address16b = (uint16_t) address; // Address is contained within 16b, 17th is page select
  
  uint16_t masterCode = FRAM_SLAVE_ADDRESS | page;
  
  return I2C_WriteArray(masterCode, address16b, data, size);
}

bool FRAMReadArray(uint32_t address, void * data, uint16_t size)
{
  uint16_t page = GetPage(address);
  
  uint16_t address16b = (uint16_t) address; // Address is contained within 16b, 17th is page select
  
  uint16_t masterCode = FRAM_SLAVE_ADDRESS | page;
  
  return I2C_ReadArray(masterCode, address16b, data, size);
}
/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/
static int GetPage(uint32_t address)
{
  uint16_t page = (uint16_t)((address & 0x10000ul) >> 1);
  if(page != 0)
  {
    page = 1;
  }
  else
  {
    page = 0;
  }
  return page;
}