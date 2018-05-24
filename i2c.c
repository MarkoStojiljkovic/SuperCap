/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : i2c.c
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
#include "i2c.h"
#include <xc.h>

#define WRITE_MASK                      0x1u


#define I2C_RCV                         I2C1RCV // I2Cx Receive Register (default value 0x0000)
#define I2C_TRN                         I2C1TRN // I2Cx Transmit Register (default value 0x00FF)
#define I2C_BRG                         I2C1BRG // I2Cx Baud Rate Generator Register (default value 0x0000)
#define I2C_CON                         I2C1CON // I2Cx Configuration Register (default value 0x1000)
#define I2C_STAT                        I2C1STAT // I2Cx Status Register (default value 0x0000)
#define I2C_ADD                         I2C1ADD // I2Cx Address Register (default value 0x0000)
#define I2C_MSK                         I2C1MSK // I2Cx Address Mask (default value 0x0000)


// I2Cx Enable bit
// 1 = Enables the I2Cx module and configures the SDAx and SCLx pins as serial port pins
// 0 = Disables the I2Cx module; all the I2C? pins are controlled by port functions
#define I2C_CON_I2CEN                   (0x1u << 15)  

//I2CSIDL: I2Cx Stop in Idle Mode bit
//1 = Discontinues the module operation when a device enters the Idle mode
//0 = Continues the module operation in the Idle mode
#define I2C_CON_I2CSIDL                 (0x1u << 13)

//SCLREL: SCLx Release Control bit (when operating as I2C slave)
//1 = Releases the SCLx clock
//0 = Holds SCLx clock low (clock stretch)
//If STREN = 1:
//User software may write ?0? to initiate a clock stretch and write ?1? to release the clock. Hardware clears
//at the beginning of every slave data byte transmission. Hardware clears at the end of every slave
//address byte reception. Hardware clears at the end of every slave data byte reception.
//If STREN = 0:
//User software may only write ?1? to release the clock. Hardware clears at the beginning of every slave
//data byte transmission. Hardware clears at the end of every slave address byte reception.
#define I2C_CON_SCLREL                  (0x1u << 12)

//IPMIEN: IPMI Enable bit
//1 = IPMI Support mode is enabled, all addresses are Acknowledged
//0 = IPMI Support mode is disabled
#define I2C_CON_IPMIEN                  (0x1u << 11)

//A10M: 10-Bit Slave Address bit
//1 = I2CxADD register is a 10-bit slave address
//0 = I2CxADD register is a 7-bit slave address
#define I2C_CON_A10M                    (0x1u << 10)

//DISSLW: Disable Slew Rate Control bit
//1 = Slew rate control is disabled
//0 = Slew rate control is enabled
#define I2C_CON_DISSLW                  (0x1u <<  9)

//SMEN: SMBus Input Levels bit
//1 = Enables the I/O pin thresholds compliant with the SMBus specification
//0 = Disables the SMBus input thresholds
#define I2C_CON_SMEN                    (0x1u <<  8)

//GCEN: General Call Enable bit (when operating as I2C slave)
//1 = Enables the interrupt when a general call address is received in the I2CxRSR register (module is
//enabled for reception)
//0 = Disables the general call address
#define I2C_CON_GCEN                    (0x1u <<  7)

//STREN: SCLx Clock Stretch Enable bit (I2C Slave mode only; used in conjunction with the SCLREL bit)
//1 = Enables the user software or the receive clock stretching
//0 = Disables the user software or the receive clock stretching
#define I2C_CON_STREN                   (0x1u <<  6)

//ACKDT: Acknowledge Data bit (I2C Master mode; receive operation only)
//Value that will be transmitted when the user software initiates an Acknowledge sequence.
//1 = Sends a NACK during an Acknowledge
//0 = Sends an ACK during an Acknowledge
#define I2C_CON_ACKDT                   (0x1u <<  5)

//ACKEN: Acknowledge Sequence Enable bit (I2C Master mode receive operation)
//1 = Initiates the Acknowledge sequence on the SDAx and SCLx pins and transmits the ACKDT data bit
//(hardware clears at the end of the master Acknowledge sequence)
//0 = Acknowledge sequence is not in progress
#define I2C_CON_ACKEN                   (0x1u <<  4)

//RCEN: Receive Enable bit (I2C Master mode)
//1 = Enables Receive mode for I2C (hardware clears at the end of eighth bit of master receive data byte)
//0 = Receive sequence is not in progress
#define I2C_CON_RCEN                    (0x1u <<  3)

//PEN: Stop Condition Enable bit (I2C Master mode)
//1 = Initiates the Stop condition on the SDAx and SCLx pins (hardware clears at the end of master Stop
//sequence)
//0 = Stop condition is not in progress
#define I2C_CON_PEN                     (0x1u <<  2)

//RSEN: Repeated Start Condition Enable bit (I2C Master mode)
//1 = Initiates the Repeated Start condition on the SDAx and SCLx pins (hardware clears at the end of
//master Repeated Start sequence)
//0 = Repeated Start condition is not in progress
#define I2C_CON_RSEN                    (0x1u <<  1)

//SEN: Start Condition Enable bit (I2C Master mode)
//1 = Initiates the Start condition on the SDAx and SCLx pins (hardware clears at the end of master Start
//sequence)
//0 = Start condition is not in progress
#define I2C_CON_SEN                     (0x1u <<  0)




#define I2C_STAT_ACKSTAT                (0x1u << 15) // Acknowledge Status bit 1 = NACK received from slave, 0 = ACK received from slave
#define I2C_STAT_TRSTAT                 (0x1u << 14) // TRSTAT: Transmit Status bit (I2C? Master mode transmit operation) 1 = Master transmit is in progress (8 bits + ACK) ,0 = Master transmit is not in progress
#define I2C_STAT_ACKTIM                 (0x1u << 13) // Not available on all devices, Acknowledge Time Status bit (valid in I2C Slave mode only)
#define I2C_STAT_BCL                    (0x1u << 10) // Bus Collision Detect bit (Master and Slave modes)
#define I2C_STAT_GCSTAT                 (0x1u <<  9) // GCSTAT: General Call Status bit
#define I2C_STAT_ADD10                  (0x1u <<  8) // ADD10: 10-Bit Address Status bit
#define I2C_STAT_IWCOL                  (0x1u <<  7) // IWCOL: I2Cx Write Collision Detect bit 1 = An attempt to write to the I2CxTRN register failed because the I2C module is busy , 0 = No collision
#define I2C_STAT_I2COV                  (0x1u <<  6) // I2COV: I2Cx Receive Overflow Flag bit
#define I2C_STAT_DA                     (0x1u <<  5) // D/A: Data/Address bit (I2C Slave mode) 
#define I2C_STAT_P                      (0x1u <<  4) // P: Stop bit
#define I2C_STAT_S                      (0x1u <<  3) // S: Start bit
#define I2C_STAT_RW                     (0x1u <<  2) // R/W: Read/Write Information bit (when operating as I2C slave)
#define I2C_STAT_RBF                    (0x1u <<  1) // RBF: Receive Buffer Full Status bit
#define I2C_STAT_TBF                    (0x1u <<  0) // TBF: Transmit Buffer Full Status bit


/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/

/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
static void i2cWaitForIdle(void);
static void i2cStart(void);
static void i2cStop(void);
static void i2cRestart(void);
static uint16_t i2cWrite(uint8_t data);
static uint8_t i2cRead(void);
static void i2cAck(void);
static void i2cNack(void);
/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
  
void I2cInit()
{
  I2C1CONbits.I2CEN = 0;   // disable i2c
  I2C1CONbits.I2CSIDL = 1; // continue operation in idle mode
  I2C1CONbits.IPMIEN = 0;  // we are master, so disable
  I2C1CONbits.A10M = 0;    // 7 bit slave address
  I2C1CONbits.DISSLW = 1;  // slew rate control disabled
  I2C1CONbits.SMEN = 0;    // SMBus input thresholds disabled
  I2C1CONbits.ACKDT = 1;   // master sends a nack

//  I2C1BRG = 0x27;          // 400 kHz clock
//  I2C1BRG = 0xA0;  // 16MHz / 0xA0 = 100kHz
  I2C1BRG = 0x28;  // 16MHz / 0x28 = 400kHz
  I2C1CONbits.I2CEN = 1;   // enable the module
  
  i2cStop();
}


bool I2C_WriteArray(uint16_t masterCode, uint16_t address, const void * data, uint16_t size)
{
  uint16_t              position;

  i2cStart(); 
  if (i2cWrite(masterCode & ~WRITE_MASK) != 0) goto failure;   
  
  if (i2cWrite((address >> 8) & 0xffu) != 0)  goto failure;                     /* Write address MSB byte                                   */
  
  if (i2cWrite((address >> 0) & 0xffu) != 0)  goto failure;                     /* Write address LSB byte                                   */

  for (position = 0; position < size; position++)
  {
    if (i2cWrite(((const uint8_t *)data)[position]) != 0) goto failure;
  }
  i2cStop();

  return (true);
failure:
  i2cStop();
  return (false);
};



bool I2C_ReadArray(uint16_t masterCode, uint16_t address, void * data, uint16_t size)
{
  uint16_t  position;
  i2cStart();
  if (i2cWrite(masterCode & ~WRITE_MASK) != 0)  goto failure;                   /* Write device address with WR = 1                         */

  if (i2cWrite((address >> 8) & 0xffu) != 0)  goto failure;                     /* Write address MSB byte                                   */

  if (i2cWrite((address >> 0) & 0xffu) != 0)  goto failure;                     /* Write address LSB byte                                   */
  
  i2cRestart();                                                                 /* Start reading bytes: see FRAM datasheet                  */

  if (i2cWrite(masterCode | WRITE_MASK) != 0) goto failure;                     /* Read device address with WR = 0                          */

  for (position = 0; position < size - 1; position++)
  {
    ((uint8_t *)data)[position] = i2cRead();
    i2cAck();
  }
  ((uint8_t *)data)[position] = i2cRead();
  i2cNack();                                                                    /* NACK to FRAM: stops reading procedure                    */
  i2cStop();

  return (true);
failure:
  i2cStop();
  return (false);
}

/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/

static void i2cWaitForIdle(void)
{
  /* Wait until I2C is inactive */
  while ((I2C_CON & (I2C_CON_ACKEN | I2C_CON_RCEN | I2C_CON_PEN | I2C_CON_RSEN | I2C_CON_SEN)) ||
         (I2C_STAT & I2C_STAT_TRSTAT));
}

static void i2cStart(void)
{
  i2cWaitForIdle();
  I2C_CON |= I2C_CON_SEN;

  while (I2C_CON & I2C_CON_SEN);
}

static void i2cStop(void)
{
  i2cWaitForIdle();
  I2C_CON |= I2C_CON_PEN;

  while (I2C_CON & I2C_CON_PEN);
}

static void i2cRestart(void)
{
  i2cWaitForIdle();
  I2C_CON |= I2C_CON_RSEN;

  while (I2C_CON & I2C_CON_RSEN);
}

static uint16_t i2cWrite(uint8_t data)
{
  i2cWaitForIdle();
  while (I2C_STAT & I2C_STAT_TBF);
  I2C_TRN = data;

  while (I2C_STAT & I2C_STAT_TRSTAT);

  if (I2C_STAT & I2C_STAT_ACKSTAT) {

    return (-1);
  } else {

    return (0);
  }
}

static uint8_t i2cRead(void)
{
  i2cWaitForIdle();
  I2C_CON |= I2C_CON_RCEN;

  while(I2C_CON & I2C_CON_RCEN);
  I2C_STAT &= ~I2C_STAT_I2COV;

  return(I2C1RCV);
}

static void i2cAck(void)
{
  i2cWaitForIdle();
  I2C_CON &= ~I2C_CON_ACKDT;
  I2C_CON |=  I2C_CON_ACKEN;

  while (I2C_CON & I2C_CON_ACKEN);
}

static void i2cNack(void)
{
  i2cWaitForIdle();
  I2C_CON |= I2C_CON_ACKDT;
  I2C_CON |= I2C_CON_ACKEN;

  while (I2C_CON & I2C_CON_ACKEN);
}