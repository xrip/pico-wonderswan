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

#ifndef __MEMORY_H__
#define __MEMORY_H__

#if 0
extern uint8	*ws_staticRam;
extern uint8	*internalRam;
extern uint8	*externalEeprom;
#else
//extern uint8	ws_staticRam[64 << 10];
extern uint8	internalRam[64 << 10];
//extern uint8	externalEeprom[128 << 10];
#endif

void	ws_memory_init(uint8 *rom, uint32 romSize);
void	ws_memory_reset(void);
uint8	*memory_getRom(void);
uint32	memory_getRomSize(void);
uint16	memory_getRomCrc(void);
void	ws_memory_done(void);
void memory_load(int fp);
void memory_save(int fp);

#endif
