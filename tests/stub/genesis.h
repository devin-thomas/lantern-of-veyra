#ifndef GENESIS_H
#define GENESIS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int16_t s16;
typedef int32_t s32;

typedef int VDPPlane;
typedef int TransferMethod;

#define BG_A 0
#define BG_B 1
#define TRUE true
#define FALSE false
#define DMA 1
#define JOY_1 0
#define BUTTON_UP 0x0001
#define BUTTON_DOWN 0x0002
#define BUTTON_LEFT 0x0004
#define BUTTON_RIGHT 0x0008
#define BUTTON_A 0x0010
#define BUTTON_B 0x0020
#define BUTTON_C 0x0040
#define BUTTON_START 0x0080
#define TILE_USER_INDEX 16

#define TILE_ATTR_FULL(pal, priority, vertical_flip, horizontal_flip, tile) ((u16)(tile))

extern u16 test_joy_state;
extern s16 test_horizontal_scroll;
extern s16 test_vertical_scroll;
extern size_t test_tile_write_count;
extern u16 test_max_tile_y;
u16 JOY_readJoypad(u16 joy);
void SYS_doVBlankProcess(void);
void VDP_setTileMapXY(VDPPlane plane, u16 tile, u16 x, u16 y);
void VDP_setHorizontalScroll(VDPPlane plane, s16 value);
void VDP_setVerticalScroll(VDPPlane plane, s16 value);
void VDP_drawText(const char *text, u16 x, u16 y);
void VDP_clearPlane(VDPPlane plane, bool wait);
void PSG_write(u8 value);
void PSG_reset(void);
void PSG_setEnvelope(u8 channel, u8 value);
void PSG_setTone(u8 channel, u16 value);
void PSG_setNoise(u8 type, u8 frequency);
void SRAM_enable(void);
void SRAM_enableRO(void);
void SRAM_disable(void);
u8 SRAM_readByte(u32 offset);
void SRAM_writeByte(u32 offset, u8 value);

#define PSG_ENVELOPE_MIN 15
#define PSG_NOISE_TYPE_WHITE 1
#define PSG_NOISE_FREQ_CLOCK8 2

extern u8 test_psg_log[256];
extern size_t test_psg_log_length;

#define random() rand()

#endif
