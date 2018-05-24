/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : checksum.h
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

#ifndef _CHECKSUM_h
#define _CHECKSUM_h

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
// Functions for calculating checksum in sending mode
void ChecksumResetSendingMode();

void ChecksumAppendSendingMode(void * val, int size);

uint16_t ChecksumGetSendingMode();


// Functions for calculating checksum in receive mode
void ChecksumResetReceiveMode();
void ChecksumAppendReceiveMode(void * val, int size);
void ChecksumAppenByteReceiveMode(uint8_t val);
uint16_t ChecksumGetReceiveMode();
int ChecksumCompareReceiveMode(void * buff, int offset);
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

