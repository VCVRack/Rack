/************************************************************************/
/* XMEGA EEPROM Driver                                                  */
/*                                                                      */
/* eeprom.c                                                             */
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

#include "avrlibx/third_party/eeprom_driver/eeprom_driver.h"
#include <string.h>

#ifdef __AVR_XMEGA__

// NVM call
static inline void NVM_EXEC(void)
{
        void *z = (void *)&NVM_CTRLA;
        
        __asm__ volatile("out %[ccp], %[ioreg]"  "\n\t"
        "st z, %[cmdex]"
        :
        : [ccp] "I" (_SFR_IO_ADDR(CCP)),
        [ioreg] "d" (CCP_IOREG_gc),
                     [cmdex] "r" (NVM_CMDEX_bm),
                     [z] "z" (z)
                     );
}

#ifdef USE_AVR1008_EEPROM

// Interrupt handler for the EEPROM write "done" interrupt
ISR(NVM_EE_vect)
{
        // Disable the EEPROM interrupt
        NVM.INTCTRL = (NVM.INTCTRL & ~NVM_EELVL_gm);
}

// AVR1008 fix
static inline void NVM_EXEC_WRAPPER(void)
{
        // Save the Sleep register
        uint8_t sleepCtr = SLEEP.CTRL;
        // Set sleep mode to IDLE
        SLEEP.CTRL = (SLEEP.CTRL & ~SLEEP.CTRL) | SLEEP_SMODE_IDLE_gc;
        // Save the PMIC Status and control registers
        uint8_t statusStore = PMIC.STATUS;
        uint8_t pmicStore = PMIC.CTRL;
        
        // Enable only the highest level of interrupts
        PMIC.CTRL = (PMIC.CTRL & ~(PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm)) | PMIC_HILVLEN_bm;
        // Save SREG for later use
        uint8_t globalInt = SREG;
        // Enable global interrupts
        sei();
        // Set sleep enabled
        SLEEP.CTRL |= SLEEP_SEN_bm;
        // Save eeprom interrupt settings for later
        uint8_t eepromintStore = NVM.INTCTRL;
        NVM_EXEC();
        // Enable EEPROM interrupt
        NVM.INTCTRL =  NVM_EELVL0_bm | NVM_EELVL1_bm;
        // Sleep before 2.5uS has passed
        sleep_cpu();
        // Restore sleep settings
        SLEEP.CTRL = sleepCtr;
        // Restore PMIC status and control registers
        PMIC.STATUS = statusStore;
        PMIC.CTRL = pmicStore;
        // Restore EEPROM interruptsettings
        NVM.INTCTRL = eepromintStore;
        // Restore global interrupt settings
        SREG = globalInt;
}

#else

#define NVM_EXEC_WRAPPER NVM_EXEC

#endif // USE_AVR1008_EEPROM


void wait_for_nvm(void)
{
        while (NVM.STATUS & NVM_NVMBUSY_bm) { };
}

void flush_buffer(void)
{
        wait_for_nvm();
        
        if ((NVM.STATUS & NVM_EELOAD_bm) != 0) {
                NVM.CMD = NVM_CMD_ERASE_EEPROM_BUFFER_gc;
                NVM_EXEC();
        }
}


uint8_t EEPROM_read_byte(uint16_t addr)
{
        wait_for_nvm();
        
        NVM.ADDR0 = addr & 0xFF;
        NVM.ADDR1 = (addr >> 8) & 0x1F;
        NVM.ADDR2 = 0;
        
        NVM.CMD = NVM_CMD_READ_EEPROM_gc;
        NVM_EXEC();
        
        return NVM.DATA0;
}


void EEPROM_write_byte(uint16_t addr, uint8_t byte)
{
        flush_buffer();
        NVM.CMD = NVM_CMD_LOAD_EEPROM_BUFFER_gc;
        
        NVM.ADDR0 = addr & 0xFF;
        NVM.ADDR1 = (addr >> 8) & 0x1F;
        NVM.ADDR2 = 0;
        
        NVM.DATA0 = byte;
        
        NVM.CMD = NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc;
        NVM_EXEC_WRAPPER();
}


uint16_t EEPROM_read_block(uint16_t addr, uint8_t *dest, uint16_t len)
{
        uint16_t cnt = 0;
        
        NVM.ADDR2 = 0;
        
        wait_for_nvm();
        
        while (len > 0)
        {
                NVM.ADDR0 = addr & 0xFF;
                NVM.ADDR1 = (addr >> 8) & 0x1F;
                
                NVM.CMD = NVM_CMD_READ_EEPROM_gc;
                NVM_EXEC();
                
                *(dest++) = NVM.DATA0; addr++;
                
                len--; cnt++;
        }
        return cnt;
}


uint16_t EEPROM_write_block(uint16_t addr, const uint8_t *src, uint16_t len)
{
        uint8_t byte_addr = addr % EEPROM_PAGE_SIZE;
        uint16_t page_addr = addr - byte_addr;
        uint16_t cnt = 0;
        
        flush_buffer();
        wait_for_nvm();
        NVM.CMD = NVM_CMD_LOAD_EEPROM_BUFFER_gc;
        
        NVM.ADDR1 = 0;
        NVM.ADDR2 = 0;
        
        while (len > 0)
        {
                NVM.ADDR0 = byte_addr;
                
                NVM.DATA0 = *(src++);
                
                byte_addr++;
                len--;
                
                if (len == 0 || byte_addr >= EEPROM_PAGE_SIZE)
                {
                        NVM.ADDR0 = page_addr & 0xFF;
                        NVM.ADDR1 = (page_addr >> 8) & 0x1F;
                        
                        NVM.CMD = NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc;
                        NVM_EXEC();
                        
                        page_addr += EEPROM_PAGE_SIZE;
                        byte_addr = 0;
                        
                        wait_for_nvm();
                        
                        NVM.CMD = NVM_CMD_LOAD_EEPROM_BUFFER_gc;
                }
                
                cnt++;
        }
        
        return cnt;
}


void EEPROM_erase_page(uint16_t addr)
{
        NVM.ADDR0 = addr & 0xFF;
        NVM.ADDR1 = (addr >> 8) & 0x1F;
        NVM.ADDR2 = 0;
        
        wait_for_nvm();
        
        NVM.CMD = NVM_CMD_ERASE_EEPROM_PAGE_gc;
        NVM_EXEC_WRAPPER();
}


void EEPROM_erase_all(void)
{
        wait_for_nvm();
        
        NVM.CMD = NVM_CMD_ERASE_EEPROM_gc;
        NVM_EXEC_WRAPPER();
}

#else // __AVR_XMEGA__

void EEPROM_erase_all(void)
{
        for (uint16_t i = 0; i < E2END; i++)
        {
                eeprom_update_byte((uint8_t *)i, 0xff);
        }
}

#endif // __AVR_XMEGA__



