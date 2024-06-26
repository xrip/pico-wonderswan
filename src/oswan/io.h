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

#ifndef __IO_H__
#define __IO_H__

#include "types.h"

extern	uint8	ws_ioRam[0x100];

extern	uint8	ws_key_start;
extern	uint8	ws_key_left;
extern	uint8	ws_key_right;
extern	uint8	ws_key_up;
extern	uint8	ws_key_down;
extern	uint8	ws_key_button_1;
extern	uint8	ws_key_button_2;

void ws_io_init(void);
void ws_io_reset(void);
void ws_io_flipControls(void);
void ws_io_done(void);

#endif
