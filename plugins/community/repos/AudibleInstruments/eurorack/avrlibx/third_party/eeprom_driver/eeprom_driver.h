/************************************************************************/
/* XMEGA EEPROM Driver                                                  */
/*                                                                      */
/* eeprom.h                                                             */
/*                                                                      */
/* Alex Forencich <alex@alexforencich.com>                              */
/*                                                                      */
/* Copyright (c) 2011 Alex Forencich                                    */
/*                                                                      */
/* Permission is hereby granted, free of charge, to any person          */
/* obtaining a copy of this software and associated documentation       */
/* files(the "Software"), to deal in the Software without restriction,  */
/* including without limitation the rights to use, copy, modify, merge, */
/* publish, distribute, sublicense, and/or sell copies of the Software, */
/* and to permit persons to whom the Software is furnished to do so,    */
/* subject to the following conditions:                                 */
/*                                                                      */
/* The above copyright notice and this permission notice shall be       */
/* included in all copies or substantial portions of the Software.      */
/*                                                                      */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,      */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF   */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                */
/* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS  */
/* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN   */
/* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN    */
/* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE     */
/* SOFTWARE.                                                            */
/*                                                                      */
/************************************************************************/

#ifndef __EEPROM_DRIVER_H
#define __EEPROM_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef __AVR_XMEGA__
#include <avr/eeprom.h>
#endif // __AVR_XMEGA__

#ifndef EEPROM_PAGE_SIZE
#define EEPROM_PAGE_SIZE E2PAGESIZE
#endif

#ifdef __AVR_XMEGA__
// AVR1008 fix
//#define USE_AVR1008_EEPROM
#endif // __AVR_XMEGA__

// Prototypes

#ifdef __AVR_XMEGA__

uint8_t EEPROM_read_byte(uint16_t addr);
void EEPROM_write_byte(uint16_t addr, uint8_t byte);
uint16_t EEPROM_read_block(uint16_t addr, uint8_t *dest, uint16_t len);
uint16_t EEPROM_write_block(uint16_t addr, const uint8_t *src, uint16_t len);

void EEPROM_erase_page(uint16_t addr);

#else // __AVR_XMEGA__

#define EEPROM_read_byte(addr) eeprom_read_byte((const uint8_t *)((uint16_t)(addr)))
#define EEPROM_write_byte(addr, value) eeprom_write_byte((uint8_t *)((uint16_t)(addr)), (value))
#define EEPROM_read_block(addr, dest, len) eeprom_read_block((dest), (void *)((uint16_t)(addr)), (len))
#define EEPROM_write_block(addr, src, len) eeprom_write_block((src), (void *)((uint16_t)(addr)), (len))

#endif // __AVR_XMEGA__

void EEPROM_erase_all(void);

#ifdef __cplusplus
}
#endif


#endif // __EEPROM_DRIVER_H
