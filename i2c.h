/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : i2c.h
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

#ifndef _I2C_h
#define _I2C_h

/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/*****************************************************************************
 * Project Specific Types
 *****************************************************************************/


/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/
void I2cInit();
bool I2C_WriteArray(uint16_t masterCode, uint16_t address, const void * data, uint16_t size);
bool I2C_ReadArray(uint16_t masterCode, uint16_t address, void * data, uint16_t size);
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

