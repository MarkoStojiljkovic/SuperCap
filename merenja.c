#include "globals.h"
#include "komunikacija.h"
#include "main.h"

static int16_t offset;


int16_t offsetop=0;
const float expectedmaxvalue=32767;
int16_t maxvalue;
float gain;
int16_t channelvalue;
volatile float result;
volatile uint16_t averageresultcurrent;
volatile uint16_t averageresultvoltage;
volatile float acc;
uint8_t accbrojac=0;
int16_t proba;
volatile int8_t proba1;
volatile int8_t proba2;


float proba3;


volatile uint16_t current[200];
volatile uint16_t voltage[200];

volatile uint16_t* currentfill;
volatile uint16_t* voltagefill;

volatile uint8_t currentcount=0;
volatile uint8_t voltagecount=0;

volatile int8_t mux=1;
volatile uint8_t transmitc;
volatile uint8_t transmitv;


void __attribute__((__interrupt__,__no_auto_psv__)) _T3Interrupt(void)
{
    IFS0bits.T3IF = 0;
    
    if(drzanje>0)
        drzanje--;






}
void __attribute__((__interrupt__,__no_auto_psv__)) _T2Interrupt(void)//interupt on 1s
{

    IFS0bits.T2IF = 0;

}

//void __attribute__((__interrupt__,__no_auto_psv__)) _SDA1Interrupt(void)
//{
//
//    IFS6bits.SDA1IF=0;
//    
//    channelvalue=SD1RESH;
//    result=gain*((float)(channelvalue-offset+offsetop));
//    
//    switch (mux)
//    {
//        case 1://struja
//            acc+=result;
//            accbrojac++;
//            if(accbrojac==40)
//            {
//                *currentfill++=(uint16_t)(acc/40);
//                currentcount++;
//                
//                 if (currentcount==100)
//                {
//                     transmitc=1;
//                                    
//                }
//                
//                if (currentcount==200)
//                {
//                    transmitc=2;
//                    currentcount=0;
//                    currentfill=current;
//                }
//                
//                accbrojac=0;
//                acc=0;
//                mux=2;
//                SD1CON3bits.SDCH=0b001;////Differential Channel 1
//            }
//            
//            break;
//            
//        case 2:
//            
//            if(accbrojac==9)
//            {
//                accbrojac=0;
//                acc=0;
//                mux=3;
//            }
//            break;
//            
//        case 3:
//            
//            acc+=result;
//            accbrojac++;
//            if(accbrojac==40)
//            {
//                *voltagefill=(uint16_t)(acc/40);
//                voltagecount++;
//                
//                if (voltagecount==100)
//                {
//                     transmitv=1;
//                    
//                }
//                
//                if (voltagecount==200)
//                {
//                    transmitv=2;
//                    voltagecount=0;
//                    voltagefill=voltage;
//                }
//                accbrojac=0;
//                acc=0;
//                mux=4;
//                SD1CON3bits.SDCH=0b000;////Differential Channel 0
//            }
//            break;
//            
//        case 4:
//            
//            if(accbrojac==9)
//            {
//                accbrojac=0;
//                acc=0;
//                mux=1;
//            }
//            break;
//  
//    }
//   
//   
//   
//}



void startujMerenja(void)//offset and gain error
{
    uint8_t count;

    for(count=0;count<6;count++)
    {
        IFS6bits.SDA1IF=0;
        while(IFS6bits.SDA1IF==0);
    }

    offset=SD1RESH;
    SD1CON1bits.VOSCAL=0;//Internal offset measurement disable
    SD1CON3bits.SDCH=0b011;//reference measurement
    
    for(count=0;count<20;count++)
    {
        IFS6bits.SDA1IF=0;
        while(IFS6bits.SDA1IF==0);    
    }
    maxvalue=SD1RESH;
    gain=expectedmaxvalue/((float)(maxvalue-offset));

    SD1CON3bits.SDCH=0b000;////Differential Channel 0
    for(count=0;count<20;count++)
    {
        IFS6bits.SDA1IF=0;
        while(IFS6bits.SDA1IF==0);
    }


    IFS6bits.SDA1IF=0;
    IPC26bits.SDA1IP=0b110;//
    IEC6bits.SDA1IE=1;
}
