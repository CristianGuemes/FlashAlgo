/* ----------------------------------------------------------------------------
 *         SAM Software Package License
 * ----------------------------------------------------------------------------
 * Copyright (c) 2020, Microchip Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Microchip's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY MICROCHIP "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL MICROCHIP BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/**
 * \file
 *
 * \section Purpose
 *
 * Interface for configuration the Enhanced Embedded Flash Controller (EEFC) peripheral.
 *
 * \section Usage
 *
 * -# Enable/disable %flash ready interrupt sources using SEFC_EnableFrdyIt()
 *    and SEFC_DisableFrdyIt().
 * -# Translates the given address into which EEFC, page and offset values
 *    for difference density %flash memory using SEFC_TranslateAddress().
 * -# Computes the address of a %flash access given the EFC, page and offset
 *    for difference density %flash memory using SEFC_ComputeAddress().
 * -# Start the executing command with SEFC_PerformCommand()
 * -# Retrieve the current status of the EFC using SEFC_GetStatus().
 * -# Retrieve the result of the last executed command with SEFC_GetResult().
 */

#ifndef _EEFC_
#define _EEFC_

#include "pic32cx2051mtg64.h"

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include <stdint.h>

/*----------------------------------------------------------------------------
 *        Definitions
 *----------------------------------------------------------------------------*/
///* TODO: Temporary definition for missing symbol in header file */
#if ((defined CPU_PIC32CX0212MTG) || (defined CPU_PIC32CX0212MTSH) )
  #define IFLASH_PAGE_SIZE        512u
  #define IFLASH_SECTOR_SIZE      131072u
  #define IFLASH_LOCK_REGION_SIZE 8192u
  #define IFLASH_NB_OF_PAGES      512u
  #define IFLASH_NB_OF_LOCK_BITS  32u
#elif ((defined CPU_PIC32CX2051MTG) || (defined CPU_PIC32CX2051MTSH) )
  #define IFLASH_PAGE_SIZE        512u
  #define IFLASH_SECTOR_SIZE      131072u
  #define IFLASH_LOCK_REGION_SIZE 8192u
  #define IFLASH_NB_OF_PAGES      4096u
  #define IFLASH_NB_OF_LOCK_BITS  256u
#endif

/* EFC command */
#define SEFC_FCMD_GETD    0x00 /* Get Flash Descriptor */
#define SEFC_FCMD_WP      0x01 /* Write page */
#define SEFC_FCMD_WPL     0x02 /* Write page and lock */
#define SEFC_FCMD_EA      0x05 /* Erase all */
#define SEFC_FCMD_EPA     0x07 /* Erase pages */
#define SEFC_FCMD_SLB     0x08 /* Set Lock Bit */
#define SEFC_FCMD_CLB     0x09 /* Clear Lock Bit */
#define SEFC_FCMD_GLB     0x0A /* Get Lock Bit */
#define SEFC_FCMD_SGPB    0x0B /* Set GPNVM Bit */
#define SEFC_FCMD_CGPB    0x0C /* Clear GPNVM Bit */
#define SEFC_FCMD_GGPB    0x0D /* Get GPNVM Bit */
#define SEFC_FCMD_STUI    0x0E /* Start unique ID */
#define SEFC_FCMD_SPUI    0x0F /* Stop unique ID */
#define SEFC_FCMD_GCALB   0x10 /* Get CALIB Bit */
#define SEFC_FCMD_ES      0x11 /* Erase Sector */
#define SEFC_FCMD_WUS     0x12 /* Write User Signature */
#define SEFC_FCMD_EUS     0x13 /* Erase User Signature */
#define SEFC_FCMD_STUS    0x14 /* Start Read User Signature */
#define SEFC_FCMD_SPUS    0x15 /* Stop Read User Signature */

/* The IAP function entry addreass */
#define CHIP_FLASH_IAP_ADDRESS  (0x02000008)

#ifdef __cplusplus
 extern "C" {
#endif
   
/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

extern void SEFC_EnableFrdyIt( Sefc* sefc ) ;

extern void SEFC_DisableFrdyIt( Sefc* sefc ) ;

extern void SEFC_SetWaitState( Sefc* sefc, uint8_t cycles ) ;

extern void SEFC_TranslateAddress(Sefc** ppSefc, uint32_t dwAddress, uint16_t *pwPage, uint16_t *pwOffset ) ;

extern void SEFC_ComputeAddress( Sefc* sefc, uint16_t wPage, uint16_t wOffset, uint32_t *pdwAddress ) ;

extern uint32_t SEFC_PerformCommand( Sefc* sefc, uint32_t dwCommand, uint32_t dwArgument, uint32_t dwUseIAP ) ;

extern uint32_t SEFC_GetStatus( Sefc* sefc ) ;

extern uint32_t SEFC_GetResult( Sefc* sefc ) ;

//extern void SEFC_SetFlashAccessMode(Sefc* sefc, uint32_t dwMode) ;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _EEFC_ */

