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
#define USE_35_BOARD 1
#define USE_FALSE_CHARGING_PROTECTION 0


// DEV ID PINS (INPUTS)
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

#define REF_100_10_PIN           LATBbits.LATB1 // KEEP AT ZERO WHEN 35 CHARGER/DISCHARGER IS USED, pin 15, TP na J3 jumper




// Pinout is different for 100A and 35A board
#if (USE_35_BOARD == 1)
// LED-s
#define LED_RED LATEbits.LATE2 // pin 62, LED1
#define LED_GREEN LATEbits.LATE3 // pin 63 LED2
#define LED_YELLOW LATEbits.LATE4  // pin 64 LED3
// Charger pins
#define EN_CH_SW    LATDbits.LATD8 // pin 42, TP R83, R82
#define EN_CH_SW_READ   PORTDbits.RD8
#define EN_CHARGE   LATEbits.LATE1 // pin 61
#define EN_CHARGE_READ  PORTEbits.RE1 


// Discharger pins
#define DISABLE_DISCHARGER      LATBbits.LATB2 // Komplementarna logika, pin 14, TP na J4 jumper
#define DICH_SW_35A             LATBbits.LATB3 // pin 13, TP J6 jumper i TP7
// Resistor
#define GATE_RES_CONTROL        LATEbits.LATE7 // pin 3, TP9 i J8
// Fanox
#define FANOX_CONTROL           LATEbits.LATE6 // pin 2, TP na otporniku R76 gornji graj
#else
// LED pins
#define LED_RED LATEbits.LATE1 // pin 61, LED1
#define LED_GREEN LATEbits.LATE2 // pin 62 LED2
#define LED_YELLOW LATEbits.LATE3  // pin 63 LED3 

// Charger/discharger control pins
#define CHARGER_EN_PIN          LATEbits.LATE4 // pin 64 , TP15
#define SWITCH_100A_PIN         LATEbits.LATE6 // pin 2, TP na otporniku R76 gornji graj
#define DISCH_EN_PIN            LATBbits.LATB2 // pin 14, TP na J4 jumper
#define SWITCH_10A_PIN          LATEbits.LATE5 // pin 1, TP na R77
#define SEL_MEASURE_100_10_PIN  LATBbits.LATB0 // pin 16, TP ./.
#define FANOX_EN_PIN            LATBbits.LATB3 // pin 13, TP J6 jumper i TP7
#define RES_EN_PIN              LATEbits.LATE7 // pin 3, TP9 i J8
#define FAST_CHARGING           LATDbits.LATD8 // pin 42, TP R83, R82
#endif

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

