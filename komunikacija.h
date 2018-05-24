/* 
 * File:   other_func.h
 * Author: User
 *
 * Created on 22.05.2014., 09.12
 */

#ifndef KOMUNIKACIJA_H
#define	KOMUNIKACIJA_H


void CRC16(unsigned char a);
void transbyte(unsigned char a);

void trans10sc(volatile uint16_t *p);
void trans10sv(volatile uint16_t *p);





#endif	/* KOMUNIKACIJA_H */

