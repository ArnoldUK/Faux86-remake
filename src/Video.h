/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
  Copyright (C)2010-2013 Mike Chambers
  
  Contributions and Updates (c)2023 Curtis aka ArnoldUK

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#pragma once

#include "Types.h"
#include "Ports.h"

#define VGA_DAC_MODE_READ	0x00
#define VGA_DAC_MODE_WRITE	0x03

#define VGA_REG_DATA_CURSOR_BEGIN			0x0A
#define VGA_REG_DATA_CURSOR_END				0x0B

#define VGA_MODE_TEXT									0
#define VGA_MODE_GRAPHICS_8BPP				1
#define VGA_MODE_GRAPHICS_4BPP				2
#define VGA_MODE_GRAPHICS_2BPP				3
#define VGA_MODE_GRAPHICS_1BPP				4

#define RGB565(r,g,b) (((r)>>3)<<11 | ((g)>>2)<<5 | (b)>>3)
#define ARGB(a,r,g,b) ((uint32_t)((uint8_t)(a)<<24 | (uint8_t)(r)<<16 | (uint8_t)(g)<<8 | (uint8_t)(b)))

typedef rgb_t PALETTE[256];

typedef struct {
	uint8_t state;
	uint8_t index;
	uint8_t step;
	uint8_t pal[256][3];
} VGADAC_t;

extern volatile double vga_lockFPS;

namespace Faux86
{
	class VM;

	class Palette
	{
	public:
		struct Entry
		{
			inline void set(uint8_t inR, uint8_t inG, uint8_t inB)
			{
				r = inR; g = inG; b = inB;
			}
			uint8_t r, g, b, a;
		};

		Palette();

		inline void set(int index, uint8_t r, uint8_t g, uint8_t b)
		{
			colours[index].r = r;
			colours[index].g = g;
			colours[index].b = b;
			colours[index].a = 0xff;
		}

		inline void set(int index, const Entry& colour)
		{
			colours[index].r = colour.r;
			colours[index].g = colour.g;
			colours[index].b = colour.b;
			colours[index].a = colour.a;
		}

		Entry colours[256];
	};

	class Video : public PortInterface
	{
	public:
		Video(VM& inVM);
		~Video(void);
		
		uint8_t init(void);
	
		//static constexpr int VRAMSize = 0x40000;
		//uint8_t VRAM[VRAMSize];
		uint8_t port3da;
		uint8_t updatedscreen;
		uint8_t	vidmode;
		uint16_t cursorposition;
		uint16_t cursorvisible;
		
		void handleInterrupt();
		
		void vga_drawCallback(void* dummy);		
		void vga_blinkCallback(void* dummy);
		void vga_hblankCallback(void* dummy);
		void vga_hblankEndCallback(void* dummy);

		void vga_renderThread(void* dummy);
		void vga_update(uint32_t start_x, uint32_t start_y, uint32_t end_x, uint32_t end_y);
			
		uint8_t vga_readcrtci();
		uint8_t vga_readcrtcd();
		void vga_writecrtci(uint8_t value);
		void vga_writecrtcd(uint8_t value);
		
		void vga_calcscreensize();
		void vga_calcmemorymap();
		void vga_updateScanlineTiming();
		uint8_t vga_dologic(uint8_t value, uint8_t latch);
		
		void writeVGA(uint32_t addr32, uint8_t value);
		void vga_writememory(void* dummy, uint32_t addr, uint8_t value);
		
		uint8_t readVGA(uint32_t addr32);
		uint8_t vga_readmemory(void* dummy, uint32_t addr);

		virtual bool portWriteHandler(uint16_t portnum, uint8_t value) override;
		virtual bool portReadHandler(uint16_t portnum, uint8_t& outValue) override;

		Palette* getCurrentPalette() { return currentPalette; }
		
		static constexpr int FontSize = 32768;
		uint8_t* fontcga;
		
		volatile uint64_t vga_hblankstart, vga_hblankend, vga_hblanklen, vga_dispinterval, vga_hblankinterval, vga_htotal;
		volatile uint64_t vga_vblankstart, vga_vblankend, vga_vblanklen, vga_vblankinterval, vga_frameinterval;
		volatile uint32_t vga_hblankTimer, vga_hblankEndTimer, vga_drawTimer;

	private:
		uint32_t vga_color(uint32_t c);
		uint8_t vga_dorotate(uint8_t v);
		uint32_t rgb(uint32_t r, uint32_t g, uint32_t b);

		VM& vm;

		Palette::Entry tempRGB;

		Palette* currentPalette;

		Palette paletteVGA;
		Palette paletteCGA;
		
		//VGADAC_t vga_DAC;
		
		const uint32_t vga_fontbases[8] = { 0x0000, 0x4000, 0x8000, 0xC000, 0x2000, 0x6000, 0xA000, 0xE000 };
		
		uint32_t vga_dots = 8;
		uint32_t vga_membase, vga_memmask = 0;
		uint16_t vga_cursorloc = 0;
		uint8_t vga_dbl = 0;
		uint8_t vga_crtci = 0;
		uint8_t vga_crtcd[0x19] = {0};
		uint8_t vga_attri = 0;
		uint8_t vga_attrd[0x15] = {0};
		uint8_t vga_attrflipflop = 0;
		uint8_t vga_attrpal = 0x20;
		uint8_t vga_gfxi = 0;
		uint8_t vga_gfxd[0x09] = {0};
		uint8_t vga_seqi = 0;
		uint8_t vga_seqd[0x05] = {0};
		//uint8_t vga_misc, vga_status0, vga_status1;
		uint8_t vga_misc = 0x40;
		uint8_t vga_status0, vga_status1 = 0; //UPDATED
		uint8_t vga_cursor_blink_state = 0;
		
		volatile uint8_t vga_rotate = 0;
		volatile uint8_t vga_wmode, vga_rmode, vga_shiftmode, vga_logicop, vga_enableplane, vga_readmap, vga_scandbl, vga_hdbl, vga_bpp = 0;
		volatile uint8_t vga_latch[4] = {0};
		
		//volatile uint64_t vga_hblankstart, vga_hblankend, vga_hblanklen, vga_dispinterval, vga_hblankinterval, vga_htotal;
		//volatile uint64_t vga_vblankstart, vga_vblankend, vga_vblanklen, vga_vblankinterval, vga_frameinterval;
		//volatile uint32_t vga_hblankTimer, vga_hblankEndTimer, vga_drawTimer;
		volatile uint8_t vga_doRender = 0;
		volatile uint8_t vga_doBlit = 0;
		//volatile uint32_t vga_w = 640, vga_h = 400;
		volatile uint32_t vga_w = 640, vga_h = 350;
		volatile double vga_targetFPS = 30;
		volatile double vga_lockFPS = 0;
		volatile uint16_t vga_curScanline = 0;
	};
}
