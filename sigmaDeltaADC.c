/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : sigmaDeltaADC.c
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
#include <stdint.h>
#include <xc.h>

#include "sigmaDeltaADC.h"


#define SDA_PRIO 4
/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/
signed int SDOffset; // Calculated offset during initialization IF I USED offset for name everything i cant assign it value for some reason
signed int maxMeasuredValue; // Max value (SVDD) measured during initialization

const static signed int expectedMaxValue=32767;
float SDGain;
float SDLastValue;



/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
static void CalculateOffset();
static void ADCFlushCache();
/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
void SD_ADCInit()
{
  //  SADC INIT
  IPC26bits.SDA1IP = SDA_PRIO;//Interrupt priority 4 , timer is IP5
  SD1CON1bits.PWRLVL=1;//2x bandwidth
  SD1CON1bits.SDREFP=0;//Positive Reference SVDD
  SD1CON1bits.SDREFN=0;//Negative Reference SVss
  SD1CON1bits.VOSCAL=1;//Internal offset measurement enable
  SD1CON1bits.DITHER=0b01;//Low dither
  SD1CON1bits.SDGAIN=0b000;//gain=1
  SD1CON1bits.SDRST = 0; // Release from Reset

  SD1CON2bits.RNDRES=2;//Round result to 16 bits
  SD1CON2bits.SDWM=1;//SDxRESH/SDxRESL updated on every Interrupt
  SD1CON2bits.SDINT=3;//Interrupt on every data output
  SD1CON2bits.CHOP=3;//Chopping is enabled

  SD1CON3bits.SDCH= CHANNEL1;//Differential Channel 1
  SD1CON3bits.SDCS=0;//System clock (FOSC/2) 16MHz // Original not working
//  SD1CON3bits.SDCS=1;//System clock FRC(8MHz)
//    SD1CON3bits.SDDIV=2;//4  16/4 = 4MHZ
  SD1CON3bits.SDDIV=1;//4  16/4 = 4MHZ
  //    SD1CON3bits.SDDIV=0;//16MhZ
  SD1CON3bits.SDOSR=0;//1024 OVERSAMPLING RATIO, freq is 4kHz (1kHz for some reason)
  IEC6bits.SDA1IE=0; // SDA1IE: Sigma-Delta A/D Converter Interrupt Enable bit, DISABLE INTERRUPT

  // GET OFFSET     -----------------------------------------------------------------------------
  SD1CON1bits.SDON=1;//module is enable
  CalculateOffset();
    // Switch off offset measurement mode, now that we have the value
  SD1CON1bits.SDON = 0; // Turn off ADC
  SD1CON1bits.VOSCAL = 0; // TURN OFF CALIBRATION
    
    //  CALCULATE GAIN ------------------------------------------------------------------------
    // Now reconfigure ADC so as to measure the SVDD through channel 1,
    // and compare it against the expected result (after offset correction).
    // Use the comparison to compute the optimum gain calibration factor, and
    // save the value, so that it may be used later.
  SD1CON3bits.SDCH = CHANNEL_REFERENCE; // point to the input voltage
  SD1CON3bits.SDCH = CHANNEL1; // point to the input voltage
  SD1CON1bits.SDON = 1; // SD_ADC back on to make next measurement
  // Wait for a minimum of five interrupts to be generated.
  ADCFlushCache();
  // Save the maximum measured value to calculate the gain.
  IFS6bits.SDA1IF = 0; //Clear interrupt flag
  while(IFS6bits.SDA1IF == 0); //Wait until hardware says we have a result ready.
  IFS6bits.SDA1IF = 0; //Clear interrupt flag
  maxMeasuredValue = SD1RESH;

  // we have establish gain/offset calibration values for the ADC).
  SD1CON1bits.SDON = 0; 
  SD1CON3bits.SDCH = CHANNEL1; //Setup Module for normal reads on CH1
  SD1CON1bits.SDON = 1;
  // Wait for SYNC filter to flush, so the next result the application tries
  //to read will be valid
  ADCFlushCache();
  signed long int temp = (signed long int)(maxMeasuredValue-SDOffset);
  SDGain = (float)expectedMaxValue/temp;
    // The ADC is now ready
    // ENABLE INTERRUPT
  IEC6bits.SDA1IE=1; // SDA1IE: Sigma-Delta A/D Converter Interrupt Enable bit
    
}

// Set channel while preserving running state
void SetChannel(int ch)
{
  int state = SD1CON1bits.SDON;
  SD1CON1bits.SDON = 0; 
  SD1CON3bits.SDCH = ch;
  SD1CON1bits.SDON = state; 
}

// Swap ADC channel while preserving running state
void SwapChannel()
{
  int state = SD1CON1bits.SDON;
  SD1CON1bits.SDON = 0;
  
  if(SD1CON3bits.SDCH == CHANNEL1)
    SD1CON3bits.SDCH = CHANNEL0;
  else
    SD1CON3bits.SDCH = CHANNEL1;
  
  SD1CON1bits.SDON = state; 
}

/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/

static void CalculateOffset()
{
  // FUCKING HARDWARE PROBLEM NEEDS TO BE DONE THIS WAY
  int i;
  for(i = 0; i < 100; i++)
  {
    ADCFlushCache();
    while(IFS6bits.SDA1IF == 0); //Wait until hardware says we have a result ready.
    IFS6bits.SDA1IF = 0; //Clear interrupt flag
    SDOffset = SD1RESH; // THIS IS 0 BECAUSE ITS BROKEN
    if(SDOffset != 0) break;
  }
}

// Flash internal cache for (should be done everytime chanel or some configuration is changed )
static void ADCFlushCache()
{   
  if(SD1CON1bits.SDON == 0) return; // it would be stupid to wait for ADC to finish if it is off
  int i;
  for (i = 0; i<6; i++) // (value must be >= 5)
  {
    IFS6bits.SDA1IF = 0; //Clear interrupt flag
    while(IFS6bits.SDA1IF == 0); //Wait until hardware says we have a result ready.
    IFS6bits.SDA1IF = 0; //Clear interrupt flag
  }
}


