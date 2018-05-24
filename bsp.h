/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : bsp.h
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

#ifndef _BSP_h
#define _BSP_h

/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "xc.h"

/*****************************************************************************
 * Project Specific Types
 *****************************************************************************/
#define OUTPUT 0
#define INPUT 1


#define LED_RED LATEbits.LATE4
#define LED_GREEN LATEbits.LATE5
#define LED_BLUE LATEbits.LATE6

#define TP6_PIN LATBbits.LATB15

// Charger control pins
#define CHARGER_EN_PIN          LATDbits.LATD7 // pin 55, TP15
#define SWITCH_100A_PIN         LATEbits.LATE2 // pin 62, TP18
#define DISCH_EN_PIN            LATEbits.LATE3 // pin 63, TP19
#define SWITCH_10A_PIN          LATEbits.LATE1 // pin 61, TP17
#define REF_100_10_PIN          LATEbits.LATE0 // pin 60, TP16
#define SEL_MEASURE_100_10_PIN  LATDbits.LATD5 // pin 53, TP14
#define FANOX_EN_PIN            LATDbits.LATD4 // pin 52, TP13
#define RES_EN_PIN              LATDbits.LATD3 // pin 51, TP11

/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/

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

