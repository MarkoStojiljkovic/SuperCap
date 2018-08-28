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


#define LED_RED LATEbits.LATE1 // pin 61, LED1
#define LED_GREEN LATEbits.LATE2 // pin 62 LED2
#define LED_YELLOW LATEbits.LATE3  // pin 63 LED3 

#define TP6_PIN LATBbits.LATB15

// Charger control pins
#define CHARGER_EN_PIN          LATEbits.LATE4 // pin 64 , TP15
#define SWITCH_100A_PIN         LATEbits.LATE6 // pin 2, TP na otporniku R76 gornji graj
#define DISCH_EN_PIN            LATBbits.LATB2 // pin 14, TP na J4 jumper
#define SWITCH_10A_PIN          LATEbits.LATE5 // pin 1, TP na R77
#define REF_100_10_PIN          LATBbits.LATB1 // pin 15, TP na J3 jumper
#define SEL_MEASURE_100_10_PIN  LATBbits.LATB0 // pin 16, TP ./.
#define FANOX_EN_PIN            LATBbits.LATB3 // pin 13, TP J6 jumper i TP7
#define RES_EN_PIN              LATEbits.LATE7 // pin 3, TP9 i J8

#define ODRZAVANJE              LATDbits.LATD8 // pin 42, TP R83, R82


// DEV ID PINS
#define ADDR0                   PORTDbits.RD0 // pin 46
#define ADDR1                   PORTDbits.RD2 // pin 50
#define ADDR2                   PORTDbits.RD3 // pin 51
#define ADDR3                   PORTDbits.RD4 // pin 52
#define ADDR4                   PORTDbits.RD5 // pin 53
#define ADDR5                   PORTDbits.RD6 // pin 54
#define ADDR6                   PORTDbits.RD7 // pin 55
#define ADDR7                   PORTEbits.RE0 // pin 60

// UART
#define RS485_DIR               LATFbits.LATF3 // pin 33
/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/
void Toggle_LED(int num); // Toggle LED, 1-red, 2-green, 3-yellow
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

