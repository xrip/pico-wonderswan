//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __ROM_H__
#define __ROM_H__

#include "types.h"

#define WS_SYSTEM_MONO			0
#define WS_SYSTEM_COLOR			1

#define WS_ROM_SIZE_2MBIT		1
#define WS_ROM_SIZE_4MBIT		2
#define WS_ROM_SIZE_8MBIT		3
#define WS_ROM_SIZE_16MBIT		4
#define WS_ROM_SIZE_24MBIT		5
#define WS_ROM_SIZE_32MBIT		6
#define WS_ROM_SIZE_48MBIT		7
#define WS_ROM_SIZE_64MBIT		8
#define WS_ROM_SIZE_128MBIT		9

#define WS_EEPROM_SIZE_NONE		0
#define WS_SRAM_SIZE_NONE		0
#define WS_EEPROM_SIZE_64k		1
#define WS_EEPROM_SIZE_256k		2
#define WS_SRAM_SIZE_1k			10
#define WS_SRAM_SIZE_16k		20
#define WS_SRAM_SIZE_8k			50


typedef struct ws_romHeaderStruct
{
	uint8	developperId;
	uint8	minimumSupportSystem;
	uint8	cartId;
	uint8	romSize;
	uint8	eepromSize;
	uint8	additionnalCapabilities;
	uint8	realtimeClock;
	uint16	checksum;
} ws_romHeaderStruct;


uint8	*ws_rom_load(uint8 *rom, uint32 *romSize);
void				ws_rom_dumpInfo(uint8 *wsrom, uint32 wsromSize);
ws_romHeaderStruct	*ws_rom_getHeader(uint8 *wsrom, uint32 wsromSize);
uint32				ws_rom_sramSize(uint8 *wsrom, uint32 wsromSize);
uint32				ws_rom_eepromSize(uint8 *wsrom, uint32 wsromSize);


#endif
