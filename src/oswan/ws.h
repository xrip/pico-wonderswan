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

#ifndef __WS_H__
#define __WS_H__

#define WS_SYSTEM_MONO			0
#define WS_SYSTEM_COLOR			1
#define WS_SYSTEM_AUTODETECT    2
#include "types.h"

// � supprimer !
extern uint32	ws_cyclesByLine;

int ws_init(uint8  *rom, size_t romSize);
int		ws_rotated(void);
void	ws_set_colour_scheme(int scheme);
void	ws_set_system(int system);
void	ws_reset(void);
int		ws_executeLine(int16 *framebuffer, int renderLine);
void	ws_patchRom(void);
#if 0
int		ws_loadState(char *statepath);
int		ws_saveState(char *statepath);
#else
int		ws_loadState(void);
int		ws_saveState(void);
int		ws_loadSRAM(char *sram_path);
int		ws_saveSRAM(char *sram_path);
#endif
void	ws_done(void);

#endif
