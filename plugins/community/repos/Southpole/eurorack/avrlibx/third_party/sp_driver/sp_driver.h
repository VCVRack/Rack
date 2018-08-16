/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief  XMEGA Self-programming driver header file.
 *
 *      This file contains the function prototypes for the
 *      XMEGA Self-programming driver.
 *      If any SPM instructions are used, the linker file must define
 *      a segment named BOOT which must be located in the device boot section.
 *
 *
 *      None of these functions clean up the NVM Command Register after use.
 *      It is therefore important to write NVMCMD_NO_OPERATION (0x00) to this
 *      register when you are finished using any of the functions in this driver.
 *
 *      For all functions, it is important that no interrupt handlers
 *      perform any NVM operations. The user must implement a scheme for mutually
 *      exclusive access to the NVM. However, the 4-cycle timeout will work fine,
 *      since writing to the Configuration Change Protection register (CCP)
 *      automatically disables interrupts for 4 instruction cycles.
 *
 * \par Application note:
 *      AVR1316: XMEGA Self-programming
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Revision: 1691 $
 * $Date: 2008-07-29 13:25:40 +0200 (ti, 29 jul 2008) $  \n
 *
 * Copyright (c) 2008, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#ifndef SP_DRIVER_H
#define SP_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "avr_compiler.h"
//#include "Flash_Defines.h"



/* Define the size of the flash page if not defined in the header files. */
#ifndef APP_SECTION_PAGE_SIZE
        #error  APP_SECTION_PAGE_SIZE must be defined if not defined in header files.
        //#define APP_SECTION_PAGE_SIZE 512
#endif /*FLASH_PAGE_SIZE*/

/* Define the Start of the application table if not defined in the header files. */
#ifndef APPTABLE_SECTION_START
	#error  APPTABLE_SECTION_START must be defined if not defined in header files.
	//#define APPTABLE_SECTION_START 0x01E000 //APPTABLE address for ATxmega128A1
#endif /*APPTABLE_SECTION_START*/

/*! \brief Read a byte from flash.
 *
 *  This function reads one byte from the flash.
 *
 *  \note Both IAR and GCC have functions to do this, but
 *        we include the fucntions for easier use.
 *
 *  \param address Address to the location of the byte to read.
 *
 *  \retval Byte read from flash.
 */
uint8_t SP_ReadByte(uint32_t address);

/*! \brief Read a word from flash.
 *
 *  This function reads one word from the flash.
 *
 *  \note Both IAR and GCC have functions to do this automatically, but
 *        we include the fucntions for easier use.
 *
 *  \param address Address to the location of the word to read.
 *
 *  \retval word read from flash.
 */
uint16_t SP_ReadWord(uint32_t address);

/*! \brief Read calibration byte at given index.
 *
 *  This function reads one calibration byte from the Calibration signature row.
 *
 *  \param index  Index of the byte in the calibration signature row.
 *
 *  \retval Calibration byte
 */
uint8_t SP_ReadCalibrationByte(uint8_t index);

/*! \brief Read fuse byte from given index.
 *
 *  This function reads the fuse byte at the given index.
 *
 *  \param index  Index of the fuse byte.
 *
 *  \retval Fuse byte
 */
uint8_t SP_ReadFuseByte(uint8_t index);

/*! \brief Write lock bits.
 *
 *  This function changes the lock bits.
 *
 *  \note It is only possible to change the lock bits to a higher security level.
 *
 *  \param data  The new value of the lock bits.
 */
void SP_WriteLockBits(uint8_t data);

/*! \brief Read lock bits.
 *
 *  This function reads the lock bits.
 *
 *  \retval Lock bits
 */
uint8_t SP_ReadLockBits(void);

/*! \brief Read user signature at given index.
 *
 *  This function reads one byte from the user signature row.
 *
 *  \param index  Index of the byte in the user signature row.
 *
 *  \retval User signature byte
 */
uint8_t SP_ReadUserSignatureByte(uint16_t index);

/*! \brief Erase user signature row.
 *
 *  This function erase the entire user signature row.
 */
void SP_EraseUserSignatureRow(void);

/*! \brief Write user signature row.
 *
 *  This function write the flash buffer in the user signature row.
 */
void SP_WriteUserSignatureRow(void);

/*! \brief Erase entire application section.
 *
 *  This function erases the entire application and application table section
 *
 *  \note If the lock bits is set to not allow spm in the application or
 *        application table section the erase is not done.
 */
void SP_EraseApplicationSection(void);

/*! \brief Erase page at byte address in application or application table section.
 *
 *  This function erase one page given by the byte address.
 *
 *  \param address Byte address for flash page.
 */
void SP_EraseApplicationPage(uint32_t address);

/*! \brief Erase and write page buffer to application or application table section at byte address.
 *
 *  This function does a combined erase and write to a flash page in the application
 *  or application table section.
 *
 *  \param address Byte address for flash page.
 */
void SP_EraseWriteApplicationPage(uint32_t address);

/*! \brief Write page buffer to application or application table section at byte address.
 *
 *  This function writes the Flash page buffer to a page in the application or
 *  application table section given by the byte address.
 *
 *  \note The page that is written to must be erased before it is written to.
 *
 *  \param address Byte address for flash page.
 */
void SP_WriteApplicationPage(uint32_t address);

/*! \brief Load one word into Flash page buffer.
 *
 *  This function Loads one word into the Flash page buffer.
 *
 *  \param  address   Position in inside the flash page buffer.
 *  \param  data      Value to be put into the buffer.
 */
void SP_LoadFlashWord(uint16_t address, uint16_t data);

/*! \brief Load entire page from SRAM buffer into Flash page buffer.
 *
 *  This function load an entire page from SRAM.
 *
 *	\param data   Pointer to the data to put in buffer.
 *
 *	\note The __near keyword limits the pointer to two bytes which means that
 *        only data up to 64K (internal SRAM) can be used.
 */
void SP_LoadFlashPage(const uint8_t * data);

/*! \brief Read entire Flash page into SRAM buffer.
 *
 *  This function reads an entire flash page and puts it to SRAM.
 *
 *	\param data      Pointer to where to store the data.
 *	\param address   Address to page to read from flash.
 */
void SP_ReadFlashPage(const uint8_t * data, uint32_t address);

/*! \brief Flush Flash page buffer.
 *
 *  This function flush the Flash page buffer.
 */
void SP_EraseFlashBuffer(void);

/*! \brief Erase page at byte address in boot section.
 *
 *  This function erase one page given by the byte address.
 *
 *  \param address Byte address for flash page.
 */
void SP_EraseBootPage(uint32_t address);

/*! \brief Erase and write page buffer to boot section at byte address.
 *
 *  This function does a combined erase and write to a flash page in the boot
 *  section.
 *
 *  \param address Byte address for flash page.
 */
void SP_EraseWriteBootPage(uint32_t address);

/*! \brief Write page buffer to boot section at byte address.
 *
 *  This function writes the Flash page buffer to a page in the boot section
 *  given by the byte address.
 *
 *  \note The page that is written to must be erased before it is written to.
 *
 *  \param address Byte address for flash page.
 */
void SP_WriteBootPage(uint32_t address);

/*! \brief Generate CRC from application section.
 *
 *  \retval 24-bit CRC value
 */
uint32_t SP_ApplicationCRC(void);

/*! \brief Generate CRC from boot section.
 *
 *  \retval 24-bit CRC value
 */
uint32_t SP_BootCRC(void);

/*! \brief Lock SPM instruction.
 *
 *   This function locks the SPM instruction, and will disable the use of
 *   SPM until the next reset occurs.
 */
void SP_LockSPM(void);

/*! \brief Wait for SPM to finish.
 *
 *   This routine waits for the SPM to finish and clears the command register.
 */
void SP_WaitForSPM(void);

#ifdef __cplusplus
}
#endif

#endif /* SP_DRIVER_H */
