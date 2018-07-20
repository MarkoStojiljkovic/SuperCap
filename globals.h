/* 
 * File:   globals.h
 * Author: User
 *
 * Created on 29.01.2013., 12.16
 */

#ifndef GLOBALS_H
#define	GLOBALS_H


#include <xc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//#include <i2c.h>
//#define i2c_v1_1

#define FCY 16000000UL
#include <libpic30.h>










//RS485---------------------------------------------

// TODO: REMOVE
#define TASTON()          LATFbits.LATF3=1
#define TASTOFF()         LATFbits.LATF3=0
#define INT16_MAX_VALUE 32767
#define INT16_MIN_VALUE -32768

extern int g_DisableDischarger;
extern int g_DisableCharger;

extern int g_failSafeChargeDelay;
extern int g_failSafeDischargeDelay;


#endif	/* GLOBALS_H */

