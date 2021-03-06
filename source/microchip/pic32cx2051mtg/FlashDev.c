/* CMSIS-DAP Interface Firmware
 * Copyright (c) 2009-2017 ARM Limited
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

#define FLASH_DRV_VERS (0x0100+VERS)   // Driver Version, do not modify!

struct FlashDevice const FlashDevice = {
    FLASH_DRV_VERS,               // Driver Version, do not modify!
    "PIC32CXMTG 2MB Flash",       // Device Name
    ONCHIP,                       // Device Type
    0x01000000,                   // Device Start Address
    0x00200000,                   // Device Size
    512,                          // Programming Page Size
    0,                            // Reserved, must be 0
    0xFF,                         // Initial Content of Erased Memory
    300,                          // Program Page Timeout 300 mSec
    3000,                         // Erase Sector Timeout 3000 mSec
    {{0x20000, 0x00000000},       // Sector Size  128kB
		 {0x20000, 0x00020000},       // Sector Size  128kB
		 {0x20000, 0x00040000},       // Sector Size  128kB
		 {0x20000, 0x00060000},       // Sector Size  128kB
		 {0x20000, 0x00080000},       // Sector Size  128kB
		 {0x20000, 0x000A0000},       // Sector Size  128kB
		 {0x20000, 0x000C0000},       // Sector Size  128kB
		 {0x20000, 0x000E0000},       // Sector Size  128kB
		 {0x20000, 0x00100000},       // Sector Size  128kB
		 {0x20000, 0x00120000},       // Sector Size  128kB
		 {0x20000, 0x00140000},       // Sector Size  128kB
		 {0x20000, 0x00160000},       // Sector Size  128kB
		 {0x20000, 0x00180000},       // Sector Size  128kB
		 {0x20000, 0x001A0000},       // Sector Size  128kB
		 {0x20000, 0x001C0000},       // Sector Size  128kB
		 {0x20000, 0x001E0000},       // Sector Size  128kB
     {SECTOR_END}}
};
