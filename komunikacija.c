

#include "globals.h"//global change
#include "komunikacija.h"
#include "merenja.h"



volatile unsigned int mojcrc;
volatile unsigned int msgcrc;
volatile unsigned int msgcrcpr;




uint8_t highnibble;
uint8_t lownibble;






void CRC16(unsigned char a)
{
int i;
    mojcrc ^= (unsigned int) a;
    for (i = 8; i != 0; i--)
    {
        if ((mojcrc & 0x0001) != 0)
        {
            mojcrc >>= 1;
            mojcrc ^= 0xA001;
        } else
        {
            mojcrc >>= 1;
        }
    }
}



void trans10sc(volatile uint16_t *p)
{
    int8_t i;
    
    __delay_ms(10);
    TASTON();
    __delay_ms(10);
    mojcrc=0xFFFF;
    transbyte(0xAA);
    CRC16(0xAA);
    transbyte(0xAA);
    CRC16(0xAA);
    transbyte(0xAA);
    CRC16(0xAA);
    
    transbyte(0x01);//salje struju
    CRC16(0x01);
    
    for(i=0;i<100;i++)
    {
        highnibble=(uint8_t)(*p>>8);       
        transbyte(highnibble);
        CRC16(highnibble);
        lownibble=(uint8_t)(*p&0xFF);       
        transbyte(lownibble);
        CRC16(lownibble);
        
        p++;
    }
    
    
    transbyte((unsigned char)mojcrc);
    transbyte((unsigned char)(mojcrc>>8));
    __delay_ms(15);
    TASTOFF();
    
    
}

void trans10sv(volatile uint16_t *p)
{
  int8_t i;
    
    __delay_ms(10);
    TASTON();
    __delay_ms(10);
    mojcrc=0xFFFF;
    transbyte(0xAA);
    CRC16(0xAA);
    transbyte(0xAA);
    CRC16(0xAA);
    transbyte(0xAA);
    CRC16(0xAA);
    
    transbyte(0x02);
    CRC16(0x02);
    
    for(i=0;i<100;i++)
    {
        highnibble=(uint8_t)(*p>>8);       
        transbyte(highnibble);
        CRC16(highnibble);
        lownibble=(uint8_t)(*p&0xFF);       
        transbyte(lownibble);
        CRC16(lownibble);
        
        p++;
    }
    
    
    transbyte((unsigned char)mojcrc);
    transbyte((unsigned char)(mojcrc>>8));
    __delay_ms(15);
    TASTOFF();
    
    
}


void transbyte(unsigned char a)
{
    while(U1STAbits.TRMT==0);
    U1TXREG=a;
}

