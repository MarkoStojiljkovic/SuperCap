

#include "globals.h"
#include "hardware.h"



void UARTset(void)
{
RPINR18bits.U1RXR=10;//RP10 je RX
RPOR8bits.RP17R=3;//RP17 je TX
U1BRG=208;//kada je BRGH=1 -> BR=Fcy/4(U1BRG+1) sada je (208dec) za 19230

U1MODEbits.UARTEN=1;//UART ENABLE
U1MODEbits.RTSMD=1;//pin in simplex mode
U1MODEbits.UEN=0b00;//Rx and TX are enabled and used rts cts controled by port latches
U1MODEbits.BRGH=1;//tip formule za racunanje
U1MODEbits.PDSEL=0b00;//8bit data no parity
U1MODEbits.STSEL=0;//one stop bit

U1STAbits.UTXEN=1;//TX ENABLE

IEC0bits.U1RXIE = 1;//interrupt enable
//while(U1STAbits.TRMT==0);
//U1TXREG=0x40;

}