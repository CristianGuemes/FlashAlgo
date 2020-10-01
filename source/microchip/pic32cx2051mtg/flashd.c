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

/** \addtogroup flashd_module Flash Memory Interface
 * The flash driver manages the programming, erasing, locking and unlocking sequences
 * with dedicated commands.
 *
 * To implement flash programing operation, the user has to follow these few steps :
 * <ul>
 * <li>Configue flash wait states to initializes the flash. </li>
 * <li>Checks whether a region to be programmed is locked. </li>
 * <li>Unlocks the user region to be programmed if the region have locked before.</li>
 * <li>Erases the user page before program (optional).</li>
 * <li>Writes the user page from the page buffer.</li>
 * <li>Locks the region of programmed area if any.</li>
 * </ul>
 *
 * Writing 8-bit and 16-bit data is not allowed and may lead to unpredictable data corruption.
 * A check of this validity and padding for 32-bit alignment should be done in write algorithm.

 * Lock/unlock range associated with the user address range is automatically translated.
 *
 * This security bit can be enabled through the command "Set General Purpose NVM Bit 0".
 *
 * A 128-bit factory programmed unique ID could be read to serve several purposes.
 *
 * The driver accesses the flash memory by calling the lowlevel module provided in \ref efc_module.
 * For more accurate information, please look at the EEFC section of the Datasheet.
 *
 * Related files :\n
 * \ref flashd.c\n
 * \ref flashd.h.\n
 * \ref efc.c\n
 * \ref efc.h.\n
*/
/*@{*/
/*@}*/


/**
 * \file
 *
 * The flash driver provides the unified interface for flash program operations.
 *
 */

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include "pic32cx2051mtg64.h"
#include "efc.h"
#include "flashd.h"

#include <string.h>
#include <assert.h>

/*----------------------------------------------------------------------------
 *        Definitions
 *----------------------------------------------------------------------------*/
#define GPNVM_NUM_MAX    9

#if defined(EFC) && !defined(SEFC0)
    #define SEFC0     EFC
#endif

/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/

static uint32_t _pdwPageBuffer[IFLASH_PAGE_SIZE/sizeof(uint32_t)] ;
static uint32_t _dwUseIAP = 1; /* Not Use IAP interface by default. */

/*----------------------------------------------------------------------------
 *        Local macros
 *----------------------------------------------------------------------------*/

#define min( a, b ) (((a) < (b)) ? (a) : (b))

/*----------------------------------------------------------------------------
 *        Local functions
 *----------------------------------------------------------------------------*/


/**
 * \brief Computes the lock range associated with the given address range.
 *
 * \param dwStart  Start address of lock range.
 * \param dwEnd  End address of lock range.
 * \param pdwActualStart  Actual start address of lock range.
 * \param pdwActualEnd  Actual end address of lock range.
 */
static void ComputeLockRange( uint32_t dwStart, uint32_t dwEnd, uint32_t *pdwActualStart, uint32_t *pdwActualEnd )
{
    Sefc* pStartEfc ;
    Sefc* pEndEfc ;
    uint16_t wStartPage ;
    uint16_t wEndPage ;
    uint16_t wNumPagesInRegion ;
    uint16_t wActualStartPage ;
    uint16_t wActualEndPage ;

    /* Convert start and end address in page numbers */
    SEFC_TranslateAddress( &pStartEfc, dwStart, &wStartPage, 0 ) ;
    SEFC_TranslateAddress( &pEndEfc, dwEnd, &wEndPage, 0 ) ;

    /* Find out the first page of the first region to lock */
    wNumPagesInRegion = IFLASH_LOCK_REGION_SIZE / IFLASH_PAGE_SIZE ;
    wActualStartPage = wStartPage - (wStartPage % wNumPagesInRegion) ;
    wActualEndPage = wEndPage ;

    if ( (wEndPage % wNumPagesInRegion) != 0 )
    {
        wActualEndPage += wNumPagesInRegion - (wEndPage % wNumPagesInRegion) ;
    }
    /* Store actual page numbers */
    SEFC_ComputeAddress( pStartEfc, wActualStartPage, 0, pdwActualStart ) ;
    SEFC_ComputeAddress( pEndEfc, wActualEndPage, 0, pdwActualEnd ) ;
}


/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

/**
 * \brief Initializes the flash driver.
 *
 * \param dwMCk     Master clock frequency in Hz.
 * \param dwUseIAP  0: use EEFC controller interface, 1: use IAP interface.
 *                  dwUseIAP should be set to 1 when running out of flash.
 */

extern void FLASHD_Initialize( uint32_t dwMCk, uint32_t dwUseIAP )
{
    dwMCk = dwMCk; /* avoid warnings */
    
    SEFC_DisableFrdyIt( SEFC0 ) ;
#if defined (EFC1)
    SEFC_DisableFrdyIt( EFC1 ) ;
#endif
    _dwUseIAP = dwUseIAP ;
}

/**
 * \brief Erases the entire flash.
 *
 * \param dwAddress  Flash start address.
 * \return 0 if successful; otherwise returns an error code.
 */
extern uint32_t FLASHD_Erase( uint32_t dwAddress )
{
    Sefc* pSefc ;
    uint16_t wPage ;
    uint16_t wOffset ;
    uint32_t dwError ;

    assert( (dwAddress >=IFLASH0_CNC_ADDR) || (dwAddress <= (IFLASH0_CNC_ADDR + IFLASH_SIZE)) ) ;

    /* Translate write address */
    SEFC_TranslateAddress( &pSefc, dwAddress, &wPage, &wOffset ) ;
    dwError = SEFC_PerformCommand( pSefc, SEFC_FCMD_EA, 0, _dwUseIAP ) ;

    return dwError ;
}

/**
 * \brief Erases flash by sector.
 *
 * \param dwAddress  Start address of be erased sector.
 *
 * \return 0 if successful; otherwise returns an error code.
 */
extern uint32_t FLASHD_EraseSector( uint32_t dwAddress )
{
    Sefc* pSefc ;
    uint16_t wPage ;
    uint16_t wOffset ;
    uint32_t dwError ;

    assert( (dwAddress >=IFLASH0_CNC_ADDR) || (dwAddress <= (IFLASH0_CNC_ADDR + IFLASH_SIZE)) ) ;

    /* Translate write address */
    SEFC_TranslateAddress( &pSefc, dwAddress, &wPage, &wOffset ) ;
    dwError = SEFC_PerformCommand( pSefc, SEFC_FCMD_ES, wPage, _dwUseIAP ) ;

    return dwError ;
}

/**
 * \brief Erases flash by pages.
 *
 * \param dwAddress  Start address of be erased pages.
 * \param dwPageNum  Number of pages to be erased with EPA command (4, 8, 16, 32)
 *
 * \return 0 if successful; otherwise returns an error code.
 */
//extern uint32_t FLASHD_ErasePages( uint32_t dwAddress, uint32_t dwPageNum )
//{
//    Sefc* pSefc ;
//    uint16_t wPage ;
//    uint16_t wOffset ;
//    uint32_t dwError ;
//    static uint32_t dwFarg ;
//
//    assert( (dwAddress >=IFLASH0_CNC_ADDR) || (dwAddress <= (IFLASH0_CNC_ADDR + IFLASH_SIZE)) ) ;
//
//    /* Translate write address */
//    SEFC_TranslateAddress( &pSefc, dwAddress, &wPage, &wOffset ) ;
//
//    /* Get FARG field for EPA command:
//     * The first page to be erased is specified in the FARG[15:2] field of
//     * the MC_FCR register. The first page number must be modulo 4, 8,16 or 32
//     * according to the number of pages to erase at the same time.
//     *
//     * The 2 lowest bits of the FARG field define the number of pages to
//     * be erased (FARG[1:0]).
//     */
//    if (dwPageNum == 32)
//    {
//        wPage &= ~(32u - 1u);
//        dwFarg = (wPage << 2) | 3; /* 32 pages */
//    }
//    else if (dwPageNum == 16)
//    {
//        wPage &= ~(16u - 1u);
//        dwFarg = (wPage << 2) | 2; /* 16 pages */
//    }
//    else if (dwPageNum == 8)
//    {
//        wPage &= ~(8u - 1u);
//        dwFarg = (wPage << 2) | 1; /* 8 pages */
//    }
//    else
//    {
//        wPage &= ~(4u - 1u);
//        dwFarg = (wPage << 2) | 0; /* 4 pages */
//    }
//
//    dwError = SEFC_PerformCommand( pSefc, SEFC_FCMD_EPA, dwFarg, _dwUseIAP ) ;
//
//    return dwError ;
//}


/**
 * \brief Writes a data buffer in the internal flash
 *
 * \note This function works in polling mode, and thus only returns when the
 * data has been effectively written.
 * \param address  Write address.
 * \param pBuffer  Data buffer.
 * \param size  Size of data buffer in bytes.
 * \return 0 if successful, otherwise returns an error code.
 */
extern uint32_t FLASHD_Write( uint32_t dwAddress, const void *pvBuffer, uint32_t dwSize )
{
    Sefc* pSefc ;
    uint16_t page ;
    uint16_t offset ;
    uint32_t writeSize ;
    uint32_t pageAddress ;
    uint16_t padding ;
    uint32_t dwError ;
    uint32_t dwIdx ;
    uint32_t *pAlignedDestination ;
    uint8_t  *pucPageBuffer = (uint8_t *)_pdwPageBuffer;

    assert( pvBuffer ) ;
    assert( dwAddress >=IFLASH0_CNC_ADDR ) ;
    assert( (dwAddress + dwSize) <= (IFLASH0_CNC_ADDR + IFLASH_SIZE) ) ;

    /* Translate write address */
    SEFC_TranslateAddress( &pSefc, dwAddress, &page, &offset ) ;

    /* Write all pages */
    while ( dwSize > 0 )
    {
        /* Copy data in temporary buffer to avoid alignment problems */
        writeSize = min((uint32_t)IFLASH_PAGE_SIZE - offset, dwSize ) ;
        SEFC_ComputeAddress(pSefc, page, 0, &pageAddress ) ;
        /* Use Internal Flash Write address */
        pageAddress |= 0xA0000000;
        
        /* Get padding */
        padding = IFLASH_PAGE_SIZE - offset - writeSize ;

        /* Pre-buffer data */
        memcpy( pucPageBuffer, (void *) pageAddress, offset);

        /* Buffer data */
        memcpy( pucPageBuffer + offset, pvBuffer, writeSize);

        /* Post-buffer data */
        memcpy( pucPageBuffer + offset + writeSize, (void *) (pageAddress + offset + writeSize), padding);

        /* Write page
         * Writing 8-bit and 16-bit data is not allowed and may lead to unpredictable data corruption
         */
        pAlignedDestination = (uint32_t*)pageAddress ;
        for (dwIdx = 0; dwIdx < (IFLASH_PAGE_SIZE / sizeof(uint32_t)); ++ dwIdx) {
            *pAlignedDestination++ = _pdwPageBuffer[dwIdx];
        }

        /* Note for sam3s16 and sam4s:
         * It is not possible to use Erase and write Command (EWP) on all Flash (this
         * command is available on the First 2 Small Sector, 16K Bytes). For the next
         * block, Erase them first then use Write page command.
         */
        /* Send writing command */
        dwError = SEFC_PerformCommand( pSefc, SEFC_FCMD_WP, page, _dwUseIAP ) ;
        if ( dwError )
        {
            return dwError ;
        }

        /* Progression */
        pvBuffer = (void *)((uint32_t) pvBuffer + writeSize) ;
        dwSize -= writeSize ;
        page++;
        offset = 0;
    }

    return 0 ;
}
/**
 * \brief Locks all the regions in the given address range. The actual lock range is
 * reported through two output parameters.
 *
 * \param start  Start address of lock range.
 * \param end    End address of lock range.
 * \param pActualStart  Start address of the actual lock range (optional).
 * \param pActualEnd  End address of the actual lock range (optional).
 * \return 0 if successful, otherwise returns an error code.
 */
extern uint32_t FLASHD_Lock( uint32_t start, uint32_t end, uint32_t *pActualStart, uint32_t *pActualEnd )
{
    Sefc *pSefc ;
    uint32_t actualStart, actualEnd ;
    uint16_t startPage, endPage ;
    uint32_t dwError ;
    uint16_t numPagesInRegion = IFLASH_LOCK_REGION_SIZE / IFLASH_PAGE_SIZE;

    /* Compute actual lock range and store it */
    ComputeLockRange( start, end, &actualStart, &actualEnd ) ;
    if ( pActualStart != NULL )
    {
        *pActualStart = actualStart ;
    }
    if ( pActualEnd != NULL )
    {
        *pActualEnd = actualEnd;
    }

    /* Compute page numbers */
    SEFC_TranslateAddress( &pSefc, actualStart, &startPage, 0 ) ;
    SEFC_TranslateAddress( 0, actualEnd, &endPage, 0 ) ;

    /* Lock all pages */
    while ( startPage < endPage )
    {
        dwError = SEFC_PerformCommand( pSefc, SEFC_FCMD_SLB, startPage, _dwUseIAP ) ;
        if ( dwError )
        {
            return dwError ;
        }
        startPage += numPagesInRegion;
    }

    return 0 ;
}

/**
 * \brief Unlocks all the regions in the given address range. The actual unlock range is
 * reported through two output parameters.
 * \param start  Start address of unlock range.
 * \param end  End address of unlock range.
 * \param pActualStart  Start address of the actual unlock range (optional).
 * \param pActualEnd  End address of the actual unlock range (optional).
 * \return 0 if successful, otherwise returns an error code.
 */
extern uint32_t FLASHD_Unlock( uint32_t start, uint32_t end, uint32_t *pActualStart, uint32_t *pActualEnd )
{
    Sefc* pSefc ;
    uint32_t actualStart, actualEnd ;
    uint16_t startPage, endPage ;
    uint32_t dwError ;
    uint16_t numPagesInRegion = IFLASH_LOCK_REGION_SIZE / IFLASH_PAGE_SIZE;

    /* Compute actual unlock range and store it */
    ComputeLockRange(start, end, &actualStart, &actualEnd);
    if ( pActualStart != NULL )
    {
        *pActualStart = actualStart ;
    }
    if ( pActualEnd != NULL )
    {
        *pActualEnd = actualEnd ;
    }

    /* Compute page numbers */
    SEFC_TranslateAddress( &pSefc, actualStart, &startPage, 0 ) ;
    SEFC_TranslateAddress( 0, actualEnd, &endPage, 0 ) ;

    /* Unlock all pages */
    while ( startPage < endPage )
    {
        dwError = SEFC_PerformCommand( pSefc, SEFC_FCMD_CLB, startPage, _dwUseIAP ) ;
        if ( dwError )
        {
            return dwError ;
        }
        startPage += numPagesInRegion ;
    }
    return 0 ;
}

/**
 * \brief Returns the number of locked regions inside the given address range.
 *
 * \param start  Start address of range
 * \param end    End address of range.
 */
extern uint32_t FLASHD_IsLocked( uint32_t start, uint32_t end )
{
    uint32_t i, j;
    Sefc *pSefc ;
    uint16_t startPage, endPage ;
    uint8_t startRegion, endRegion ;
    uint32_t numPagesInRegion ;
    uint32_t status[IFLASH_NB_OF_LOCK_BITS / 32u] ;
    uint32_t numLockedRegions = 0 ;

    assert( end >= start ) ;
    assert( (start >=IFLASH0_CNC_ADDR) && (end <= IFLASH0_CNC_ADDR + IFLASH_SIZE) ) ;

    /* Compute page numbers */
    SEFC_TranslateAddress( &pSefc, start, &startPage, 0 ) ;
    SEFC_TranslateAddress( 0, end, &endPage, 0 ) ;

    /* Compute region numbers */
    numPagesInRegion = IFLASH_LOCK_REGION_SIZE / IFLASH_PAGE_SIZE ;
    startRegion = startPage / numPagesInRegion ;
    endRegion = endPage / numPagesInRegion ;
    if ((endPage % numPagesInRegion) != 0)
    {
        endRegion++ ;
    }

    /* Retrieve lock status */
    SEFC_PerformCommand( pSefc, SEFC_FCMD_GLB, 0, _dwUseIAP ) ;
    for (i = 0; i < (IFLASH_NB_OF_LOCK_BITS / 32u); i++)
    {
        status[i] = SEFC_GetResult( pSefc ) ;
    }

    /* Check status of each involved region */
    while ( startRegion < endRegion )
    {
        i = startRegion / 32u;
        j = startRegion % 32u;
        if ( (status[i] & (1 << j)) != 0 )
        {
            numLockedRegions++ ;
        }
        startRegion++ ;
    }

    return numLockedRegions ;
}

/**
 * \brief Check if the given GPNVM bit is set or not.
 *
 * \param gpnvm  GPNVM mask.
 * \returns 1 if the given GPNVM bit is currently set; otherwise returns 0.
 */
extern uint32_t FLASHD_IsGPNVMSet( uint16_t usGPVM )
{
    uint32_t dwStatus ;
    
    assert(usGPVM < GPNVM_NUM_MAX);

    /* Get GPNVMs status */
    SEFC_PerformCommand( SEFC0, SEFC_FCMD_GGPB, 0, _dwUseIAP ) ;
    dwStatus = SEFC_GetResult( SEFC0 ) ;

    /* Check if GPNVM is set */
    if ( (dwStatus & (1 << usGPVM)) != 0 )
    {
        return 1 ;
    }
    else
    {
        return 0 ;
    }
}

/**
 * \brief Sets the selected GPNVM bit.
 *
 * \param gpnvm  GPNVM bit mask.
 * \returns 0 if successful; otherwise returns an error code.
 */
extern uint32_t FLASHD_SetGPNVM( uint16_t usGPVM )
{
    assert(usGPVM < GPNVM_NUM_MAX);
    
    if ( !FLASHD_IsGPNVMSet( usGPVM ) )
    {
        return SEFC_PerformCommand( SEFC0, SEFC_FCMD_SGPB, usGPVM, _dwUseIAP ) ;
    }
    else
    {
        return 0 ;
    }
}

/**
 * \brief Clears the selected GPNVM bit.
 *
 * \param gpnvm  GPNVM bit mask.
 * \returns 0 if successful; otherwise returns an error code.
 */
extern uint32_t FLASHD_ClearGPNVM( uint16_t usGPVM )
{
    assert(usGPVM < GPNVM_NUM_MAX);
    
    if ( FLASHD_IsGPNVMSet( usGPVM ) )
    {
        return SEFC_PerformCommand( SEFC0, SEFC_FCMD_CGPB, usGPVM, _dwUseIAP ) ;
    }
    else
    {
        return 0 ;
    }
}

/**
 * \brief Read the unique ID.
 *
 * \param pdwUniqueID pointer on a 4bytes char containing the unique ID value.
 * \returns 0 if successful; otherwise returns an error code.
 */
#ifdef __ICCARM__
extern __ramfunc uint32_t FLASHD_ReadUniqueID( uint32_t* pdwUniqueID )
#else
__attribute__ ((section (".ramfunc")))
uint32_t FLASHD_ReadUniqueID( uint32_t* pdwUniqueID )
#endif
{
    uint32_t status ;
    if (pdwUniqueID == NULL) {
        return 1;
    }

    pdwUniqueID[0] = 0 ;
    pdwUniqueID[1] = 0 ;
    pdwUniqueID[2] = 0 ;
    pdwUniqueID[3] = 0 ;

    /* Send the Start Read unique Identifier command (STUI) by writing the Flash
       Command Register with the STUI command.*/
    SEFC0->EEFC_FCR = EEFC_FCR_FKEY_PASSWD | SEFC_FCMD_STUI;

    /* When the Unique Identifier is ready to be read, the FRDY bit in the Flash
       Programming Status Register (EEFC_FSR) falls. */
//    do
//    {
//        status = SEFC0->EEFC_FSR ;
//    } while ( (status & EEFC_FSR_FRDY) == EEFC_FSR_FRDY ) ;

    /* The Unique Identifier is located in the first 128 bits of the Flash
       memory mapping. So, at the address 0x400000-0x40000F. */
    pdwUniqueID[0] = *(uint32_t *)IFLASH0_CNC_ADDR;
    pdwUniqueID[1] = *(uint32_t *)(IFLASH0_CNC_ADDR + 4);
    pdwUniqueID[2] = *(uint32_t *)(IFLASH0_CNC_ADDR + 8);
    pdwUniqueID[3] = *(uint32_t *)(IFLASH0_CNC_ADDR + 12);

    /* To stop the Unique Identifier mode, the user needs to send the Stop Read
       unique Identifier command (SPUI) by writing the Flash Command Register
       with the SPUI command. */
    SEFC0->EEFC_FCR = EEFC_FCR_FKEY_PASSWD | SEFC_FCMD_SPUI ;

    /* When the Stop read Unique Unique Identifier command (SPUI) has been
       performed, the FRDY bit in the Flash Programming Status Register (EEFC_FSR)
       rises. */
    do
    {
        status = SEFC0->EEFC_FSR ;
    } while ( (status & EEFC_FSR_FRDY) != EEFC_FSR_FRDY ) ;

    return 0;
}

uint32_t FLASHD_GetDescriptor( uint32_t* pdwDescriptor )
{
	while (!(SEFC0->EEFC_FSR & EEFC_FSR_FRDY));  //Flash Ready Status
	SEFC0->EEFC_FCR = EEFC_FCR_FCMD(EEFC_FCR_FCMD_GETD) | EEFC_FCR_FARG(0) | EEFC_FCR_FKEY_PASSWD;
	while (!(SEFC0->EEFC_FSR & EEFC_FSR_FRDY));  //Flash Ready Status

	for (unsigned int i = 0; i < 4; i++)
	{
		pdwDescriptor[i] = SEFC0->EEFC_FRR;
	}

	return 0;
}

