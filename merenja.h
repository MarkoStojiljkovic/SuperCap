/* 
 * File:   merenja.h
 * Author: User
 *
 * Created on 04.02.2015., 19.38
 */

#ifndef MERENJA_H
#define	MERENJA_H

#include <stdint.h>

void startujMerenja(void);
void izvrsiMerenja(void);


extern volatile uint16_t current[600];
extern volatile uint16_t voltage[600];

extern volatile uint16_t* currentfill;
extern volatile uint16_t* voltagefill;

extern volatile uint8_t transmitc;
extern volatile uint8_t transmitv;




#endif	/* MERENJA_H */

