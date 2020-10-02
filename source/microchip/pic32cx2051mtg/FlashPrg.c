/* CMSIS-DAP Interface Firmware
 * Copyright (c) 2009-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FlashOS.H"        // FlashOS Structures
#include "flashd.h"
#include "string.h"

/* Flash Boot mode bits for GPNMV : 0x60 */
#define GPNVM_BOOT_MODE_BIT0     5
#define GPNVM_BOOT_MODE_BIT1     6

/* Bank selection bit for GPNMV */
#define GPNVM_BANK_SELECTION_BIT 1

static uint32_t dev_base_adr = 0;

uint32_t Init(uint32_t adr, uint32_t clk, uint32_t fnc)
{
	uint32_t ret;
	
	FLASHD_Initialize(0, 0); // do not use IAP, Mistral
	
	ret = FLASHD_SetGPNVM(GPNVM_BOOT_MODE_BIT0);
	ret = FLASHD_SetGPNVM(GPNVM_BOOT_MODE_BIT1);
	
	if (ret != 0) {
		return (1);
	}
	
	dev_base_adr = adr;
	
	return (0);
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */
uint32_t UnInit(uint32_t fnc)
{
	return (0);
}

/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */
uint32_t EraseChip(void)
{
	return FLASHD_Erase(dev_base_adr);
}

/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */
uint32_t EraseSector(uint32_t adr)
{
	uint32_t startAddr;
	uint32_t endAddr;
        
	startAddr = adr & 0x01FFFFFF;
	endAddr = startAddr + 0x20000 - 1;

	if (FLASHD_Unlock(startAddr, endAddr, 0, 0) != 0) {
		return (1);
	}
	
	if (FLASHD_EraseSector(adr) != 0) {
		return (1);
	}
	
	return (0);
}

/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */
uint32_t ProgramPage(uint32_t adr, uint32_t sz, uint32_t *buf)
{
	uint32_t startAddr;

	startAddr = adr & 0x01FFFFFF;

	// Write data
	if (FLASHD_Write((unsigned int)startAddr, buf, sz) != 0) {
		return (1);
	}

	return (0);
}

/*
 *  Verify Flash Contents
 *    Parameter:      adr:  Start Address
 *                    sz:   Size (in bytes)
 *                    buf:  Data
 *    Return Value:   (adr+sz) - OK, Failed Address
 */
unsigned long Verify (unsigned long adr, unsigned long sz, unsigned char *buf)
{
	uint8_t *puc_flash_data;
	
	puc_flash_data = (uint8_t *)adr;
	
	if (memcmp(puc_flash_data, buf, sz) == 0) {
		return (adr + sz);
	} else {
		return (adr);
	}
}
