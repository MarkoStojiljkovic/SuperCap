/*****************************************************************************
 * Copyright 2018 Netico
 *****************************************************************************
 *
 * Filename    : init.c
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
#include "init.h"
#include <xc.h>
#include "delay.h"
#include "merenja.h"
#include "sigmaDeltaADC.h"
#include "i2c.h"
#include "rtc.h"
#include "bsp.h"
#include "dataRecorder.h"
#include "FRAM_Controller.h"
#include "chargerController.h"
#include "UART_interpreter.h"

//TODO: Maybe move all priority selection in same file (ADC is not here)
#define CPU_PRIO    0
#define TIMER1_PRIO 5
//#define SDA_PRIO 4 // Its in another file, no need for defining here
#define UART_RX_PRIO 3
#define UART_TX_PRIO 3


/*****************************************************************************
 * Declaration of Global Variables
 *****************************************************************************/

/*****************************************************************************
 Declaration of File Scope Variables
 *****************************************************************************/
// WDT Prescaler Ratio Select:
// FWPSA_PR32           1:32
//
// Watchdog Timer Postscaler Select:
// WDTPS_PS2048         1:2,048
// Watchdog is configured to timeout when device is stuck for 2 seconds, and is software controlled via RCONbits.SWDTEN bit


_CONFIG1(FWDTEN_WDT_SW & FWPSA_PR32 & WDTPS_PS2048 & ICS_PGx2 & LPCFG_ON & JTAGEN_OFF)
_CONFIG2(POSCMD_HS & OSCIOFCN_OFF & FNOSC_PRI & IESO_OFF)

//#pragma config POSCMOD = HS   // Primary Oscillator Select (HS = crystal, EC = external clock input)
//#pragma config OSCIOFNC = OFF // OSCO Pin Configuration (OSCO/CLKO/RC15 functions as CLKO (FOSC/2))
//#pragma config FCKSM = CSDCMD // Clock Switching and Fail-Safe Clock Monitor Configuration bits (Disabled)
//#pragma config FNOSC = PRI    // Initial Oscillator Select (Primary Oscillator (XT, HS, EC))
//#pragma config IESO = OFF     // Internal External Switchover (Disabled)
//#pragma config FWDTEN = OFF   // watchdog timer (off)


/*****************************************************************************
 Local Function Prototypes - Same order as defined
 *****************************************************************************/
static void UartInit();
static void InitDeviceAddress();
/*****************************************************************************
 * Global Functions (Definitions)
 *****************************************************************************/
void Init()
{
    RCONbits.SWDTEN = 1; // Enable WDT

    // OSCILATOR INIT
    OSCCONbits.COSC = 0b010; //Primary Oscillator (XT, HS, EC)
    CLKDIVbits.CPDIV = 0b01; //32 MHz (divide-by-1)

    // CPU INIT
    //  SRbits.IPL=0b010;//cpu prior is 2
    SRbits.IPL = CPU_PRIO; //cpu prior is 0

    INTCON1bits.NSTDIS = 0; //nesting is enabled

    /* FROM DATASHEET
      TIECS<1:0>: Timerx Extended Clock Source Select bits (selected when TCS = 1)
      When TCS = 1:
      11 = Generic Timer (TMRCK) external input
      10 = LPRC Oscillator
      01 = TxCK external clock input
      00 = SOSC
      When TCS = 0:
      These bits are ignored; the timer is clocked from the internal system clock (FOSC/2).
     */

    // Timer 1 init
    IPC0bits.T1IP = TIMER1_PRIO; //T1 interrupt priority is 5   
    T1CONbits.TCS = 0; //T1 clock Fosc/2 16MHz
    // TCKPS<1:0>: Timerx Input Clock Prescale Select bits, 11-1:256, 10-1:64, 01-1:8, 00-1:1
    T1CONbits.TCKPS = 0b00; //clock prescale 1:1

    T1CONbits.TON = 1; //start T1
    //  PR1=15875;//T1 period reg
    PR1 = 16000; //T1 period reg
    IFS0bits.T1IF = 0; //reset interrupt flag
    IEC0bits.T1IE = 1; //enable T1 interrupt

    // PORT INIT ---------------------------------------

    // Diods for testing
    TRISEbits.TRISE1 = OUTPUT; //LED
    //  LATEbits.LATE4=1;

    TRISEbits.TRISE2 = OUTPUT; //LED
    //  LATEbits.LATE5=1;

    TRISEbits.TRISE3 = OUTPUT; //LED
    //  LATEbits.LATE6=1;

    // Debug, TP6 pin
    TRISBbits.TRISB15 = OUTPUT;
    TP6_PIN = 0;

    // Charger control pins init
    
    TRISBbits.TRISB0 = OUTPUT; // SEL_MEASURE_100_10_PIN
    TRISBbits.TRISB1 = OUTPUT; // SEL_MEASURE_100_10_PIN
    TRISBbits.TRISB2 = OUTPUT; // DISCH_EN_PIN
    TRISBbits.TRISB3 = OUTPUT; // FANOX_EN_PIN
    
    TRISEbits.TRISE4 = OUTPUT; // CHARGER_EN_PIN
    TRISEbits.TRISE5 = OUTPUT; // SWITCH_10A_PIN
    TRISEbits.TRISE6 = OUTPUT; // SWITCH_100A_PIN
    TRISEbits.TRISE7 = OUTPUT; // RES_EN_PIN

    TRISDbits.TRISD8 = OUTPUT; // ODRZAVANJE

    // DEV ID PINS
    TRISDbits.TRISD0 = INPUT;       // pin 46
    TRISDbits.TRISD2 = INPUT;       // pin 50
    TRISDbits.TRISD3 = INPUT;       // pin 51
    TRISDbits.TRISD4 = INPUT;       // pin 52
    TRISDbits.TRISD5 = INPUT;       // pin 53
    TRISDbits.TRISD6 = INPUT;       // pin 54
    TRISDbits.TRISD7 = INPUT;       // pin 55
    TRISEbits.TRISE0 = INPUT;       // pin 60

            // End port init -----------------------------------
    ANSB = 0b0000000000000000; //B0,B1 nisu analogni ulazi
    ANSC = 0b0000000000000000;
    ANSD = 0b0000000000000000;
    ANSF = 0b0000000000000000;
    ANSE = 0b0000000000000000;
    ANSG = 0b0000000000000000;


    DataRecorderInit(); // WARNING: If 1ms tick appears before this function MCU will go in invalid state :`(
    SD_ADCInit();
    //  DEBUG_INIT_ADC();
    UartInit();
    I2cInit();
    FRAMControllerInit();
    ChargerControllerInit();

    InitDeviceAddress();
    
}

/*****************************************************************************
 * Local Functions (Definitions)
 *****************************************************************************/


static void UartInit()
{
    // ------------------------- UART
    // UART SEND/RECEIVE PIN
    TRISFbits.TRISF3 = OUTPUT;
    RS485_DIR = 0;

    RPINR18bits.U1RXR = 10; //RP10 je RX
    RPOR8bits.RP17R = 3; //RP17 je TX
    U1BRG = 107; //kada je BRGH=1 -> BR=Fcy/16(U1BRG+1) sada je (51dec) za 19230


    U1MODEbits.RTSMD = 1; //pin in simplex mode
    U1MODEbits.UEN = 0b00; //Rx and TX are enabled and used rts cts controled by port latches
    U1MODEbits.BRGH = 0; //   1 = BRG generates 4 clocks per bit period (4x baud clock, High-Speed mode), 0 = BRG generates 16 clocks per bit period (16x baud clock, Standard Speed mode)
    U1MODEbits.PDSEL = 0b00; //8bit data no parity
    U1MODEbits.STSEL = 0; //one stop bit

    // 00 = Interrupt is generated when any character is transferred to the Transmit Shift Register and the transmit buffer is empty (which implies at least one location is empty in the transmit buffer)
    U1STAbits.UTXISEL1 = 0;
    U1STAbits.UTXISEL0 = 0;
    IEC0bits.U1TXIE = 1; // Enable UART TX interrupt

    // 0x = Interrupt flag bit is set when a character is received
    U1STAbits.URXISEL = 0;


    U1MODEbits.UARTEN = 1; //UART ENABLE
    U1STAbits.UTXEN = 1; //TX ENABLE

    // Set interrupt levels
    IPC2bits.U1RXIP = UART_RX_PRIO;
    IPC3bits.U1TXIP = UART_TX_PRIO;


    Delay1ms(100);
    IEC0bits.U1RXIE = 1; //interrupt enable
}

static void InitDeviceAddress()
{
    unsigned char adr;
    
    adr = ADDR0;
    adr |= ADDR1 << 1;
    adr |= ADDR2 << 2;
    adr |= ADDR3 << 3;
    adr |= ADDR4 << 4;
    adr |= ADDR5 << 5;
    adr |= ADDR6 << 6;
    adr |= ADDR7 << 7;
    g_dev_address = adr;
}