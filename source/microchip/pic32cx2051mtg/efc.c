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

/** \addtogroup efc_module Working with EEFC
 * \ingroup peripherals_module
 * The EEFC driver provides the interface to configure and use the EEFC
 * peripheral.
 *
 * The user needs to set the number of wait states depending on the frequency used.\n
 * Configure number of cycles for flash read/write operations in the FWS field of EEFC_FMR.
 *
 * It offers a function to send flash command to EEFC and waits for the
 * flash to be ready.
 *
 * To send flash command, the user could do in either of following way:
 * <ul>
 * <li>Write a correct key, command and argument in EEFC_FCR. </li>
 * <li>Or, Use IAP (In Application Programming) function which is executed from
 * ROM directly, this allows flash programming to be done by code running in flash.</li>
 * <li>Once the command is achieved, it can be detected even by polling EEFC_FSR or interrupt.
 * </ul>
 *
 * The command argument could be a page number,GPNVM number or nothing, it depends on
 * the command itself. Some useful functions in this driver could help user tranlate physical
 * flash address into a page number and vice verse.
 *
 * For more accurate information, please look at the EEFC section of the
 * Datasheet.
 *
 * Related files :\n
 * \ref sefc.c\n
 * \ref sefc.h.\n
*/
/*@{*/
/*@}*/


/**
 * \file
 *
 * Implementation of Enhanced Embedded Flash Controller (EEFC).
 *
 */


/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include "chip.h"

#include <assert.h>

#define EEFC_FCR_FCMD(value) ((EEFC_FCR_FCMD_Msk & ((value) << EEFC_FCR_FCMD_Pos)))


/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

/**
 * \brief Enables the flash ready interrupt source on the EEFC peripheral.
 *
 * \param sefc  Pointer to a Efc instance
 */
extern void SEFC_EnableFrdyIt( Sefc* sefc )
{
    sefc->EEFC_FMR |= EEFC_FMR_FRDY;
}

/**
 * \brief Disables the flash ready interrupt source on the EEFC peripheral.
 *
 * \param sefc  Pointer to a Efc instance
 */
extern void SEFC_DisableFrdyIt( Sefc* sefc )
{
    sefc->EEFC_FMR &= (~EEFC_FMR_FRDY);
}


/**
 * \brief Set read/write wait state on the EEFC perpherial.
 *
 * \param sefc  Pointer to a Efc instance
 * \param cycles  the number of wait states in cycle.
 */
extern void SEFC_SetWaitState( Sefc* sefc, uint8_t ucCycles )
{
    uint32_t dwFmr ;

    dwFmr = sefc->EEFC_FMR ;
    dwFmr &= ~((uint32_t)EEFC_FMR_FWS_Msk) ;
    dwFmr |= EEFC_FMR_FWS(ucCycles);
    sefc->EEFC_FMR = dwFmr;
}

/**
 * \brief Returns the current status of the EEFC.
 *
 * \note Keep in mind that this function clears the value of some status bits (LOCKE, PROGE).
 *
 * \param sefc  Pointer to a Efc instance
 */
extern uint32_t SEFC_GetStatus( Sefc* sefc )
{
    return sefc->EEFC_FSR ;
}

/**
 * \brief Returns the result of the last executed command.
 *
 * \param sefc  Pointer to a Efc instance
 */
extern uint32_t SEFC_GetResult( Sefc* sefc )
{
    return sefc->EEFC_FRR ;
}

/**
 * \brief Translates the given address page and offset values.
 * \note The resulting values are stored in the provided variables if they are not null.
 *
 * \param sefc  Pointer to a Efc instance
 * \param address  Address to translate.
 * \param pPage  First page accessed.
 * \param pOffset  Byte offset in first page.
 */
extern void SEFC_TranslateAddress( Sefc** ppSefc, uint32_t dwAddress, uint16_t* pwPage, uint16_t* pwOffset )
{
    assert( dwAddress >= IFLASH0_CNC_ADDR ) ;
    assert( dwAddress <= (IFLASH0_CNC_ADDR + IFLASH_SIZE) ) ;

#if defined (EFC1)

    /* Store values */
    if ( ppSefc )
    {
        *ppSefc = (dwAddress < (IFLASH0_CNC_ADDR + IFLASH_SIZE/2)) ? SEFC0 : EFC1;
    }

    if ( pwPage )
    {
        *pwPage = (dwAddress < (IFLASH0_CNC_ADDR + IFLASH_SIZE/2)) ?
          (dwAddress - IFLASH0_CNC_ADDR) / IFLASH_PAGE_SIZE:
          (dwAddress - (IFLASH0_CNC_ADDR + IFLASH_SIZE/2)) / IFLASH_PAGE_SIZE;
    }

    if ( pwOffset )
    {
         *pwOffset = (dwAddress < (IFLASH0_CNC_ADDR + IFLASH_SIZE/2)) ?
          ((dwAddress - IFLASH0_CNC_ADDR) % IFLASH_PAGE_SIZE) :
          (dwAddress - (IFLASH0_CNC_ADDR + IFLASH_SIZE/2)) % IFLASH_PAGE_SIZE;
    }
#else

    /* Store values */
    if ( ppSefc )
    {
        *ppSefc = SEFC0 ;
    }

    if ( pwPage )
    {
        *pwPage = (dwAddress - IFLASH0_CNC_ADDR) / IFLASH_PAGE_SIZE ;
    }

    if ( pwOffset )
    {
        *pwOffset = (dwAddress - IFLASH0_CNC_ADDR) % IFLASH_PAGE_SIZE; ;
    }
#endif
}

/**
 * \brief Computes the address of a flash access given the page and offset.
 *
 * \param sefc  Pointer to a Efc instance
 * \param page  Page number.
 * \param offset  Byte offset inside page.
 * \param pAddress  Computed address (optional).
 */
extern void SEFC_ComputeAddress( Sefc* sefc, uint16_t wPage, uint16_t wOffset, uint32_t *pdwAddress )
{
    uint32_t dwAddress ;

    /* Stop warning */
    sefc = sefc;

    assert( sefc ) ;
    assert( wPage <= IFLASH_NB_OF_PAGES ) ;
    assert( wOffset < IFLASH_PAGE_SIZE ) ;
#if defined (EFC1)
/* Compute address */
    dwAddress = (sefc == SEFC0) ?
        IFLASH0_CNC_ADDR + wPage * IFLASH_PAGE_SIZE + wOffset :
        IFLASH0_CNC_ADDR + IFLASH_SIZE/2 + wPage * IFLASH_PAGE_SIZE + wOffset;
#else
    dwAddress = IFLASH0_CNC_ADDR + wPage * IFLASH_PAGE_SIZE + wOffset ;
#endif
    /* Store result */
    //if ( pdwAddress != NULL )
    if ( pdwAddress != 0 )
    {
        *pdwAddress = dwAddress ;
    }
}

/**
 * \brief Performs the given command and wait until its completion (or an error).
 *
 * \param sefc  Pointer to a Efc instance
 * \param command  Command to perform.
 * \param argument  Optional command argument.
 *
 * \return 0 if successful, otherwise returns an error code.
 */

extern uint32_t SEFC_PerformCommand( Sefc* sefc, uint32_t dwCommand, uint32_t dwArgument, uint32_t dwUseIAP )
{
    if ( dwUseIAP != 0 )
    {
        /* Pointer on IAP function in ROM */
        static uint32_t (*IAP_PerformCommand)( uint32_t, uint32_t ) ;

        IAP_PerformCommand = (uint32_t (*)( uint32_t, uint32_t )) *((uint32_t*)CHIP_FLASH_IAP_ADDRESS ) ;
        if (sefc == SEFC0) {
            IAP_PerformCommand( 0, EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FARG(dwArgument) | EEFC_FCR_FCMD(dwCommand) ) ;
        }
#if defined (EFC1)
        if (sefc == EFC1) {
            IAP_PerformCommand( 1, EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FARG(dwArgument) | EEFC_FCR_FCMD(dwCommand) ) ;
        }
#endif
        return (sefc->EEFC_FSR & (EEFC_FSR_FLOCKE | EEFC_FSR_FCMDE | EEFC_FSR_FLERR)) ;
    }
    else
    {
        uint32_t dwStatus ;

        sefc->EEFC_FCR = EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FARG(dwArgument) | EEFC_FCR_FCMD(dwCommand) ;
        do
        {
            dwStatus = sefc->EEFC_FSR ;
        }
        while ( (dwStatus & EEFC_FSR_FRDY) != EEFC_FSR_FRDY ) ;

        return ( dwStatus & (EEFC_FSR_FLOCKE | EEFC_FSR_FCMDE | EEFC_FSR_FLERR) ) ;
    }
}

