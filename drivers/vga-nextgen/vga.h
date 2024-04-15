#pragma once
#include "stdbool.h"

#define PIO_VGA (pio0)
#ifndef VGA_BASE_PIN
#define VGA_BASE_PIN (6)
#endif
#define VGA_DMA_IRQ (DMA_IRQ_0)

#define TEXTMODE_COLS 80
#define TEXTMODE_ROWS 30

#define RGB888(r, g, b) (((int)r<<16) | ((int)g << 8 ) | (int)b )
