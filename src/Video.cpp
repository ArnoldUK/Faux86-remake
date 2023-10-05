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

//#include <stdio.h>
//#include <stdint.h>
//#include <inttypes.h>
//#include <stdlib.h>

#include "VM.h"
#include "MemUtils.h"
#include "Video.h"

/*
#ifdef _WIN32
#else
#include <circle/new.h>
#include <circle/alloc.h>
#endif
*/

using namespace Faux86;

#define shiftVGA(value) {\
	for (cnt=0; cnt<(VGA_GC[3] & 7); cnt++) {\
		value = (value >> 1) | ((value & 1) << 7);\
	}\
}

#define logicVGA(curval, latchval) {\
	switch ((VGA_GC[3]>>3) & 3) {\
		   case 1: curval &= latchval; break;\
		   case 2: curval |= latchval; break;\
		   case 3: curval ^= latchval; break;\
	}\
}

#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
static uint16_t vga_color_rgb(uint16_t c)
#else
static uint32_t vga_color_rgb(uint32_t c)
#endif
{
	return (c |	(c << 8) | (c << 16));
}

//palettes for 320x200 graphics mode 2bpp
static const uint8_t vga_gfxpal[2][2][4] = {
	{
		{ 0, 2, 4, 6 }, //normal palettes
		{ 0, 3, 5, 7 }
	},
	{
		{ 0, 10, 12, 14 }, //intense palettes
		{ 0, 11, 13, 15 }
	}
};

#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
uint16_t *vga_framebuffer;
#else
//static uint32_t vga_framebuffer[1024][1024];
uint32_t vga_framebuffer[VGA_FRAMEBUFFER_HEIGHT][VGA_FRAMEBUFFER_WIDTH] = {0};
#endif

//4 banks of 64KB (It's actually 64K addresses on a 32-bit data bus on real VGA hardware)
static uint8_t* vga_RAM[4] = {}; //4 planes
//static uint8_t vga_RAM[4][VGA_RAMBANK_SIZE]; // = {0};

static VGADAC_t vga_DAC;
#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
static uint16_t vgaColor[256];
#endif

Palette::Palette()
{
	log(Log,"[Palette] Constructed");
	//MemUtils::memset(&colours, 0, sizeof(Palette::Entry) * 256);
}

Video::Video(VM& inVM)
	: vm(inVM)
{
	log(Log,"[VIDEO] Constructed");
	if (vm.config.asciiFile && vm.config.asciiFile->isValid())
	{
		//fontcga = new uint8_t[FontSize];
		//vm.config.asciiFile->read(fontcga, FontSize);
	}
	else
	{
		// Error!
	}

#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
	vga_framebuffer = (uint16_t *)calloc(VGA_FRAMEBUFFER_WIDTH * VGA_FRAMEBUFFER_HEIGHT, sizeof(uint16_t));
	if (!vga_framebuffer)
	{
		log(Log,"[VIDEO] Failed to allocate vga_framebuffer");
	}
#endif

	//vm.ports.setPortRedirector(0x3B0, 0x3DA, this);

	//currentPalette = &paletteCGA;
	currentPalette = &paletteVGA;

	// Default CGA 16-color Palette 
	/*
	paletteCGA.set(0, 0x00, 0x00, 0x00); 	//black
	paletteCGA.set(1, 0x00, 0x00, 0xAA); 	//blue
	paletteCGA.set(2, 0x00, 0xAA, 0x00); 	//green
	paletteCGA.set(3, 0x00, 0xAA, 0xAA); 	//cyan
	paletteCGA.set(4, 0xAA, 0x00, 0x00); 	//red
	paletteCGA.set(5, 0xAA, 0x00, 0xAA); 	//magenta
	paletteCGA.set(6, 0xAA, 0x55, 0x00); 	//brown
	paletteCGA.set(7, 0xAA, 0xAA, 0xAA); 	//gray white (light)
	paletteCGA.set(8, 0x55, 0x55, 0x55); 	//gray (dark)
	paletteCGA.set(9, 0x55, 0x55, 0xFF); 	//blue (bright)
	paletteCGA.set(10, 0x55, 0xFF, 0x55); //green (bright)
	paletteCGA.set(11, 0x55, 0xFF, 0xFF); //cyan (bright)
	paletteCGA.set(12, 0xFF, 0x55, 0x55); //red (bright)
	paletteCGA.set(13, 0xFF, 0x55, 0xFF); //magenta (bright)
	paletteCGA.set(14, 0xFF, 0xFF, 0x55); //yellow
	paletteCGA.set(15, 0xFF, 0xFF, 0xFF); //white (bright)
	*/
	
	// 1 - CGA, 16-color Improved Contrast
	paletteCGA.set(0, 0x01,0x01,0x01);
	paletteCGA.set(1, 0x03,0x1B,0x75);
	paletteCGA.set(2, 0x10,0x8C,0x00);
	paletteCGA.set(3, 0x17,0xBB,0xD3);
	paletteCGA.set(4, 0x72,0x0C,0x0A);
	paletteCGA.set(5, 0x6C,0x1C,0x9E);
	paletteCGA.set(6, 0xB2,0x51,0x16);
	paletteCGA.set(7, 0xB8,0xB0,0xA8);
	paletteCGA.set(8, 0x4A,0x48,0x42);
	paletteCGA.set(9, 0x0B,0x63,0xC4);
	paletteCGA.set(10, 0x9B,0xCE,0x00);
	paletteCGA.set(11, 0x73,0xF5,0xD5);
	paletteCGA.set(12, 0xE8,0x9E,0x00);
	paletteCGA.set(13, 0xFF,0x7B,0xDB);
	paletteCGA.set(14, 0xFE,0xF2,0x55);
	paletteCGA.set(15, 0xFF,0xFF,0xFE);
	
	
	// 2 - green, 16-color-optimized contrast.
	/*
	paletteCGA.set(0, 0x00,0x00,0x00);
	paletteCGA.set(1, 0x00,0x0d,0x03);
	paletteCGA.set(2, 0x01,0x15,0x05);
	paletteCGA.set(3, 0x01,0x17,0x05);
	paletteCGA.set(4, 0x01,0x21,0x08);
	paletteCGA.set(5, 0x01,0x24,0x08);
	paletteCGA.set(6, 0x02,0x2e,0x0b);
	paletteCGA.set(7, 0x02,0x31,0x0b);
	paletteCGA.set(8, 0x01,0x22,0x08);
	paletteCGA.set(9, 0x02,0x28,0x09);
	paletteCGA.set(10, 0x02,0x30,0x0b);
	paletteCGA.set(11, 0x02,0x32,0x0c);
	paletteCGA.set(12, 0x03,0x39,0x0d);
	paletteCGA.set(13, 0x03,0x3b,0x0e);
	paletteCGA.set(14, 0x09,0x3f,0x14);
	paletteCGA.set(15, 0x0d,0x3f,0x17);
	*/
	
	paletteVGA.set(0, 0, 0, 0);
	paletteVGA.set(1, 0, 0, 169);
	paletteVGA.set(2, 0, 169, 0);
	paletteVGA.set(3, 0, 169, 169);
	paletteVGA.set(4, 169, 0, 0);
	paletteVGA.set(5, 169, 0, 169);
	paletteVGA.set(6, 169, 169, 0);
	paletteVGA.set(7, 169, 169, 169);
	paletteVGA.set(8, 0, 0, 84);
	paletteVGA.set(9, 0, 0, 255);
	paletteVGA.set(10, 0, 169, 84);
	paletteVGA.set(11, 0, 169, 255);
	paletteVGA.set(12, 169, 0, 84);
	paletteVGA.set(13, 169, 0, 255);
	paletteVGA.set(14, 169, 169, 84);
	paletteVGA.set(15, 169, 169, 255);
	paletteVGA.set(16, 0, 84, 0);
	paletteVGA.set(17, 0, 84, 169);
	paletteVGA.set(18, 0, 255, 0);
	paletteVGA.set(19, 0, 255, 169);
	paletteVGA.set(20, 169, 84, 0);
	paletteVGA.set(21, 169, 84, 169);
	paletteVGA.set(22, 169, 255, 0);
	paletteVGA.set(23, 169, 255, 169);
	paletteVGA.set(24, 0, 84, 84);
	paletteVGA.set(25, 0, 84, 255);
	paletteVGA.set(26, 0, 255, 84);
	paletteVGA.set(27, 0, 255, 255);
	paletteVGA.set(28, 169, 84, 84);
	paletteVGA.set(29, 169, 84, 255);
	paletteVGA.set(30, 169, 255, 84);
	paletteVGA.set(31, 169, 255, 255);
	paletteVGA.set(32, 84, 0, 0);
	paletteVGA.set(33, 84, 0, 169);
	paletteVGA.set(34, 84, 169, 0);
	paletteVGA.set(35, 84, 169, 169);
	paletteVGA.set(36, 255, 0, 0);
	paletteVGA.set(37, 255, 0, 169);
	paletteVGA.set(38, 255, 169, 0);
	paletteVGA.set(39, 255, 169, 169);
	paletteVGA.set(40, 84, 0, 84);
	paletteVGA.set(41, 84, 0, 255);
	paletteVGA.set(42, 84, 169, 84);
	paletteVGA.set(43, 84, 169, 255);
	paletteVGA.set(44, 255, 0, 84);
	paletteVGA.set(45, 255, 0, 255);
	paletteVGA.set(46, 255, 169, 84);
	paletteVGA.set(47, 255, 169, 255);
	paletteVGA.set(48, 84, 84, 0);
	paletteVGA.set(49, 84, 84, 169);
	paletteVGA.set(50, 84, 255, 0);
	paletteVGA.set(51, 84, 255, 169);
	paletteVGA.set(52, 255, 84, 0);
	paletteVGA.set(53, 255, 84, 169);
	paletteVGA.set(54, 255, 255, 0);
	paletteVGA.set(55, 255, 255, 169);
	paletteVGA.set(56, 84, 84, 84);
	paletteVGA.set(57, 84, 84, 255);
	paletteVGA.set(58, 84, 255, 84);
	paletteVGA.set(59, 84, 255, 255);
	paletteVGA.set(60, 255, 84, 84);
	paletteVGA.set(61, 255, 84, 255);
	paletteVGA.set(62, 255, 255, 84);
	paletteVGA.set(63, 255, 255, 255);
	paletteVGA.set(64, 255, 125, 125);
	paletteVGA.set(65, 255, 157, 125);
	paletteVGA.set(66, 255, 190, 125);
	paletteVGA.set(67, 255, 222, 125);
	paletteVGA.set(68, 255, 255, 125);
	paletteVGA.set(69, 222, 255, 125);
	paletteVGA.set(70, 190, 255, 125);
	paletteVGA.set(71, 157, 255, 125);
	paletteVGA.set(72, 125, 255, 125);
	paletteVGA.set(73, 125, 255, 157);
	paletteVGA.set(74, 125, 255, 190);
	paletteVGA.set(75, 125, 255, 222);
	paletteVGA.set(76, 125, 255, 255);
	paletteVGA.set(77, 125, 222, 255);
	paletteVGA.set(78, 125, 190, 255);
	paletteVGA.set(79, 125, 157, 255);
	paletteVGA.set(80, 182, 182, 255);
	paletteVGA.set(81, 198, 182, 255);
	paletteVGA.set(82, 218, 182, 255);
	paletteVGA.set(83, 234, 182, 255);
	paletteVGA.set(84, 255, 182, 255);
	paletteVGA.set(85, 255, 182, 234);
	paletteVGA.set(86, 255, 182, 218);
	paletteVGA.set(87, 255, 182, 198);
	paletteVGA.set(88, 255, 182, 182);
	paletteVGA.set(89, 255, 198, 182);
	paletteVGA.set(90, 255, 218, 182);
	paletteVGA.set(91, 255, 234, 182);
	paletteVGA.set(92, 255, 255, 182);
	paletteVGA.set(93, 234, 255, 182);
	paletteVGA.set(94, 218, 255, 182);
	paletteVGA.set(95, 198, 255, 182);
	paletteVGA.set(96, 182, 255, 182);
	paletteVGA.set(97, 182, 255, 198);
	paletteVGA.set(98, 182, 255, 218);
	paletteVGA.set(99, 182, 255, 234);
	paletteVGA.set(100, 182, 255, 255);
	paletteVGA.set(101, 182, 234, 255);
	paletteVGA.set(102, 182, 218, 255);
	paletteVGA.set(103, 182, 198, 255);
	paletteVGA.set(104, 0, 0, 113);
	paletteVGA.set(105, 28, 0, 113);
	paletteVGA.set(106, 56, 0, 113);
	paletteVGA.set(107, 84, 0, 113);
	paletteVGA.set(108, 113, 0, 113);
	paletteVGA.set(109, 113, 0, 84);
	paletteVGA.set(110, 113, 0, 56);
	paletteVGA.set(111, 113, 0, 28);
	paletteVGA.set(112, 113, 0, 0);
	paletteVGA.set(113, 113, 28, 0);
	paletteVGA.set(114, 113, 56, 0);
	paletteVGA.set(115, 113, 84, 0);
	paletteVGA.set(116, 113, 113, 0);
	paletteVGA.set(117, 84, 113, 0);
	paletteVGA.set(118, 56, 113, 0);
	paletteVGA.set(119, 28, 113, 0);
	paletteVGA.set(120, 0, 113, 0);
	paletteVGA.set(121, 0, 113, 28);
	paletteVGA.set(122, 0, 113, 56);
	paletteVGA.set(123, 0, 113, 84);
	paletteVGA.set(124, 0, 113, 113);
	paletteVGA.set(125, 0, 84, 113);
	paletteVGA.set(126, 0, 56, 113);
	paletteVGA.set(127, 0, 28, 113);
	paletteVGA.set(128, 56, 56, 113);
	paletteVGA.set(129, 68, 56, 113);
	paletteVGA.set(130, 84, 56, 113);
	paletteVGA.set(131, 97, 56, 113);
	paletteVGA.set(132, 113, 56, 113);
	paletteVGA.set(133, 113, 56, 97);
	paletteVGA.set(134, 113, 56, 84);
	paletteVGA.set(135, 113, 56, 68);
	paletteVGA.set(136, 113, 56, 56);
	paletteVGA.set(137, 113, 68, 56);
	paletteVGA.set(138, 113, 84, 56);
	paletteVGA.set(139, 113, 97, 56);
	paletteVGA.set(140, 113, 113, 56);
	paletteVGA.set(141, 97, 113, 56);
	paletteVGA.set(142, 84, 113, 56);
	paletteVGA.set(143, 68, 113, 56);
	paletteVGA.set(144, 56, 113, 56);
	paletteVGA.set(145, 56, 113, 68);
	paletteVGA.set(146, 56, 113, 84);
	paletteVGA.set(147, 56, 113, 97);
	paletteVGA.set(148, 56, 113, 113);
	paletteVGA.set(149, 56, 97, 113);
	paletteVGA.set(150, 56, 84, 113);
	paletteVGA.set(151, 56, 68, 113);
	paletteVGA.set(152, 80, 80, 113);
	paletteVGA.set(153, 89, 80, 113);
	paletteVGA.set(154, 97, 80, 113);
	paletteVGA.set(155, 105, 80, 113);
	paletteVGA.set(156, 113, 80, 113);
	paletteVGA.set(157, 113, 80, 105);
	paletteVGA.set(158, 113, 80, 97);
	paletteVGA.set(159, 113, 80, 89);
	paletteVGA.set(160, 113, 80, 80);
	paletteVGA.set(161, 113, 89, 80);
	paletteVGA.set(162, 113, 97, 80);
	paletteVGA.set(163, 113, 105, 80);
	paletteVGA.set(164, 113, 113, 80);
	paletteVGA.set(165, 105, 113, 80);
	paletteVGA.set(166, 97, 113, 80);
	paletteVGA.set(167, 89, 113, 80);
	paletteVGA.set(168, 80, 113, 80);
	paletteVGA.set(169, 80, 113, 89);
	paletteVGA.set(170, 80, 113, 97);
	paletteVGA.set(171, 80, 113, 105);
	paletteVGA.set(172, 80, 113, 113);
	paletteVGA.set(173, 80, 105, 113);
	paletteVGA.set(174, 80, 97, 113);
	paletteVGA.set(175, 80, 89, 113);
	paletteVGA.set(176, 0, 0, 64);
	paletteVGA.set(177, 16, 0, 64);
	paletteVGA.set(178, 32, 0, 64);
	paletteVGA.set(179, 48, 0, 64);
	paletteVGA.set(180, 64, 0, 64);
	paletteVGA.set(181, 64, 0, 48);
	paletteVGA.set(182, 64, 0, 32);
	paletteVGA.set(183, 64, 0, 16);
	paletteVGA.set(184, 64, 0, 0);
	paletteVGA.set(185, 64, 16, 0);
	paletteVGA.set(186, 64, 32, 0);
	paletteVGA.set(187, 64, 48, 0);
	paletteVGA.set(188, 64, 64, 0);
	paletteVGA.set(189, 48, 64, 0);
	paletteVGA.set(190, 32, 64, 0);
	paletteVGA.set(191, 16, 64, 0);
	paletteVGA.set(192, 0, 64, 0);
	paletteVGA.set(193, 0, 64, 16);
	paletteVGA.set(194, 0, 64, 32);
	paletteVGA.set(195, 0, 64, 48);
	paletteVGA.set(196, 0, 64, 64);
	paletteVGA.set(197, 0, 48, 64);
	paletteVGA.set(198, 0, 32, 64);
	paletteVGA.set(199, 0, 16, 64);
	paletteVGA.set(200, 32, 32, 64);
	paletteVGA.set(201, 40, 32, 64);
	paletteVGA.set(202, 48, 32, 64);
	paletteVGA.set(203, 56, 32, 64);
	paletteVGA.set(204, 64, 32, 64);
	paletteVGA.set(205, 64, 32, 56);
	paletteVGA.set(206, 64, 32, 48);
	paletteVGA.set(207, 64, 32, 40);
	paletteVGA.set(208, 64, 32, 32);
	paletteVGA.set(209, 64, 40, 32);
	paletteVGA.set(210, 64, 48, 32);
	paletteVGA.set(211, 64, 56, 32);
	paletteVGA.set(212, 64, 64, 32);
	paletteVGA.set(213, 56, 64, 32);
	paletteVGA.set(214, 48, 64, 32);
	paletteVGA.set(215, 40, 64, 32);
	paletteVGA.set(216, 32, 64, 32);
	paletteVGA.set(217, 32, 64, 40);
	paletteVGA.set(218, 32, 64, 48);
	paletteVGA.set(219, 32, 64, 56);
	paletteVGA.set(220, 32, 64, 64);
	paletteVGA.set(221, 32, 56, 64);
	paletteVGA.set(222, 32, 48, 64);
	paletteVGA.set(223, 32, 40, 64);
	paletteVGA.set(224, 44, 44, 64);
	paletteVGA.set(225, 48, 44, 64);
	paletteVGA.set(226, 52, 44, 64);
	paletteVGA.set(227, 60, 44, 64);
	paletteVGA.set(228, 64, 44, 64);
	paletteVGA.set(229, 64, 44, 60);
	paletteVGA.set(230, 64, 44, 52);
	paletteVGA.set(231, 64, 44, 48);
	paletteVGA.set(232, 64, 44, 44);
	paletteVGA.set(233, 64, 48, 44);
	paletteVGA.set(234, 64, 52, 44);
	paletteVGA.set(235, 64, 60, 44);
	paletteVGA.set(236, 64, 64, 44);
	paletteVGA.set(237, 60, 64, 44);
	paletteVGA.set(238, 52, 64, 44);
	paletteVGA.set(239, 48, 64, 44);
	paletteVGA.set(240, 44, 64, 44);
	paletteVGA.set(241, 44, 64, 48);
	paletteVGA.set(242, 44, 64, 52);
	paletteVGA.set(243, 44, 64, 60);
	paletteVGA.set(244, 44, 64, 64);
	paletteVGA.set(245, 44, 60, 64);
	paletteVGA.set(246, 44, 52, 64);
	paletteVGA.set(247, 44, 48, 64);
	paletteVGA.set(248, 0, 0, 0);
	paletteVGA.set(249, 0, 0, 0);
	paletteVGA.set(250, 0, 0, 0);
	paletteVGA.set(251, 0, 0, 0);
	paletteVGA.set(252, 0, 0, 0);
	paletteVGA.set(253, 0, 0, 0);
	paletteVGA.set(254, 0, 0, 0);
	paletteVGA.set(255, 0, 0, 0);
}

Video::~Video(void)
{
#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
	if (vga_framebuffer)
	{
		free(vga_framebuffer);
	}
#endif

	delete[] fontcga;
	fontcga = nullptr;
	
	/*
	uint8_t i;
	//4 banks of 64 KB (64K addresses on a 32-bit data bus on real VGA hardware)
	for (i = 0; i < 4; i++) {
		if (vga_RAM[i] != nullptr) {
			#ifdef _WIN32
			free((void*)vga_RAM[i]);
			#else
			delete[] vga_RAM[i];
			#endif
		}
		vga_RAM[i] = nullptr;
	}
	*/
}

#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
uint16_t Video::vga_color(uint16_t c)
#else
uint32_t Video::vga_color(uint32_t c)
#endif
{
	//log(Log, "[VIDEO] vga_color %d",c);
	//return c + 255;
	//return rgb(paletteVGA.colours[c].r , paletteVGA.colours[c].g , paletteVGA.colours[c].b);
	
	#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
	uint16_t result;
	#else
	uint32_t result;
	#endif
	//RGB565(r,g,b) (((r)>>3)<<11 | ((g)>>2)<<5 | (b)>>3)
	
	if (vm.config.monitorDisplay) {
		switch (vm.config.monitorDisplay) {
	  	case MONITOR_DISPLAY_AMBER:
				result = (vga_DAC.pal[c][0] << 16) | 8; //Red channel only
				break;
			case MONITOR_DISPLAY_GREEN:
				result = (vga_DAC.pal[c][1] << 8) | 10; //Green channel only
				break;
			case MONITOR_DISPLAY_BLUE:
				result = vga_DAC.pal[c][2] | 20; //Blue channel only
				break;
			default:
				result = (vga_DAC.pal[c][2] |	(vga_DAC.pal[c][1] << 8) | (vga_DAC.pal[c][0] << 16));
				break;
		}
	} else {
		result = (vga_DAC.pal[c][2] |	(vga_DAC.pal[c][1] << 8) | (vga_DAC.pal[c][0] << 16));
	}
	
	return result;
		
	/*
	return (uint32_t)vga_DAC.pal[c][2] |
		(uint32_t)vga_DAC.pal[c][1] << 8 |
		(uint32_t)vga_DAC.pal[c][0] << 16;
	*/
}

uint8_t Video::vga_dorotate(uint8_t v) {
	return (uint8_t)((v >> vga_rotate) | (v << (8 - vga_rotate)));
}

#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
uint16_t Video::rgb(uint16_t r, uint16_t g, uint16_t b)
#else
uint32_t Video::rgb(uint32_t r, uint32_t g, uint32_t b)
#endif
{
#ifdef __BIG_ENDIAN__
	return ( (r << 24) | (g << 16) | (b << 8) );
#else
	return (r | (g << 8) | (b << 16) );
#endif
}

uint8_t Video::init(void)
{
	uint32_t x, y, i;

	log(Log, "[VIDEO] Initializing Video Device");

#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
	// uint32_t color = vga_color(0);
	uint16_t color = vgaColor[0];
	uint16_t *p = vga_framebuffer;
	uint32_t l = vm.config.framebuffer.width * vm.config.framebuffer.height;
	while (l--)
	{
		*p++ = color;
	}
#else
	for (y = 0; y < vm.config.framebuffer.width; y++) { //400
		for (x = 0; x < vm.config.framebuffer.height; x++) { //640
			vga_framebuffer[y][x] = vga_color(0);
		}
	}
#endif

	//vm.renderer.draw((uint32_t*)vga_framebuffer,
	//	vm.config.framebuffer.width, vm.config.framebuffer.height, VGA_FRAMEBUFFER_STRIDE * sizeof(uint32_t));
	//vm.renderer.draw((uint32_t*)vga_framebuffer,
	//	vm.config.framebuffer.width, vm.config.framebuffer.height, VGA_FRAMEBUFFER_STRIDE);

	/*
	//if (vga_lockFPS >= 1) vga_targetFPS = vga_lockFPS;
	//timing_addTimer(vga_blinkCallback, NULL, 3.75, TIMING_ENABLED, "[VGA] vga_blinkCallback");
	//vga_drawTimer = timing_addTimer(vga_drawCallback, NULL, vga_targetFPS, TIMING_ENABLED, "[VGA] vga_drawCallback");
	//vga_hblankTimer = timing_addTimer(vga_hblankCallback, NULL, 10000, TIMING_ENABLED, "[VGA] vga_hblankCallback"); //nonsense frequency values to begin with is fine
	//vga_hblankEndTimer = timing_addTimer(vga_hblankEndCallback, NULL, 100, TIMING_ENABLED, "[VGA] vga_hblankEndCallback"); //same here
	*/
	
	vga_curScanline = 0;

	/*
	//4 banks of 64KB (64K addresses on a 32-bit data bus on real VGA hardware)
	for (i = 0; i < 4; i++) { 
		#ifdef _WIN32
		//vga_RAM[i] = (uint8_t*)malloc(VGA_RAMBANK_SIZE); //new uint8_t[65536]
		vga_RAM[i] = (uint8_t*)calloc(VGA_RAMBANK_SIZE, sizeof(uint8_t));
		if (vga_RAM[i] == nullptr) { //NULL
			log(Log,"[VIDEO] init vga_RAM[%lu] = nullptr", i);
			break;
		}
		#else
		//vga_RAM[i] = new uint8_t[65536];
		vga_RAM[i] = (uint8_t*)malloc(VGA_RAMBANK_SIZE);
		if (vga_RAM[i] == nullptr) {
			log(Log,"[VIDEO] init vga_RAM[%lu] = nullptr", i);
			break;
		}
		#endif
	}
	*/
	
	
	//vga_RAM[0] = (uint8_t*)malloc(sizeof(uint8_t) * VGA_RAMBANK_SIZE);
	//vga_RAM[1] = (uint8_t*)malloc(sizeof(uint8_t) * VGA_RAMBANK_SIZE);
	//vga_RAM[2] = (uint8_t*)malloc(sizeof(uint8_t) * VGA_RAMBANK_SIZE);
	//vga_RAM[3] = (uint8_t*)malloc(sizeof(uint8_t) * VGA_RAMBANK_SIZE);
	
	//vga_RAM[0] = (uint8_t*)malloc(VGA_RAMBANK_SIZE);
	//vga_RAM[1] = (uint8_t*)malloc(VGA_RAMBANK_SIZE);
	//vga_RAM[2] = (uint8_t*)malloc(VGA_RAMBANK_SIZE);
	//vga_RAM[3] = (uint8_t*)malloc(VGA_RAMBANK_SIZE);
	
	vga_RAM[0] = new uint8_t [VGA_RAMBANK_SIZE];
	vga_RAM[1] = new uint8_t [VGA_RAMBANK_SIZE];
	vga_RAM[2] = new uint8_t [VGA_RAMBANK_SIZE];
	vga_RAM[3] = new uint8_t [VGA_RAMBANK_SIZE];
	
	if (vga_RAM[0] == nullptr) log(Log, "[VIDEO] Error Allocating Video Memory for Bank 1");
	if (vga_RAM[1] == nullptr) log(Log, "[VIDEO] Error Allocating Video Memory for Bank 2");
	if (vga_RAM[2] == nullptr) log(Log, "[VIDEO] Error Allocating Video Memory for Bank 3");
	if (vga_RAM[3] == nullptr) log(Log, "[VIDEO] Error Allocating Video Memory for Bank 4");
	
	for (i=0;i<VGA_RAMBANK_SIZE;i++) {
		vga_RAM[0][i] = 0;
		vga_RAM[1][i] = 0;
		vga_RAM[2][i] = 0;
		vga_RAM[3][i] = 0;
	}
	log(Log, "[VIDEO] Video Memory Allocation Completed");
	
	
	log(Log, "[VIDEO] Initializing DAC Start");
	//uint32_t c;
	//uint16_t c;
	//for (i = 0; i < 256; i++) {
	//	c = vga_color(i);
	//}

	log(Log, "[VIDEO] Initializing DAC End");
	//free(pMemBuffer);
	
	/*
	//check for memory allocation error
	if (i < 4) {
		//step back through any successfully allocated chunks 
		for (; i >= 0; i--) { 
		#ifdef _WIN32
			free((void*)vga_RAM[i]);
		#else
			delete[] vga_RAM[i];
		#endif
		}
		return -1;
	}
	*/

	//vm.ports.setPortRedirector(0x3B4, 0x3B4 + 39, this); //0x3B4 to 0x3DB
	vm.ports.setPortRedirector(0x3B0, 0x3DA, this); //UPDATED
	
	/*
	memory_mapCallbackRegister(start,count,read,write,data)
	memory_mapCallbackRegister(0xA0000, 0x20000, (void*)vga_readmemory, (void*)vga_writememory, NULL);
	LOGDEBUG("[VIDEO] Loading ROM BIOS %lu\n", vbiossize);
	if (utility_loadFile(VBIOS, vbiossize, "roms/video/videorom.bin")) {
		LOGERR("[VIDEO] Failed to load ROM BIOS\n");
		return -1;
	}
	memory_mapRegister(0xC0000, vbiossize, VBIOS, NULL);
	*/
	return 0;
}

void Video::handleInterrupt()
{
	updatedscreen = true;
	#ifdef DEBUG_VIDEO
	log(Log, "[VIDEO] handleInterrupt");
	#endif
}

void Video::vga_drawCallback(void* dummy) {
	//if (!updatedscreen) return;
	vga_doRender = 1;
	vga_doBlit = 1;
	vga_renderThread(0);
}

void Video::vga_blinkCallback(void* dummy) {
	vga_cursor_blink_state ^= 1;
}

void Video::vga_hblankCallback(void* dummy) {
	//timing_timerEnable(vga_hblankEndTimer);
	vga_status1 |= 0x01;
	vga_curScanline++;
	if (vga_curScanline == vga_vblankstart) {
		vga_status1 |= 0x08;
	}
	else if (vga_curScanline == vga_vblankend) {
		vga_curScanline = 0;
		vga_status1 &= 0xF7;
	}
}

void Video::vga_hblankEndCallback(void* dummy) {
	//timing_timerDisable(vga_hblankEndTimer);
	vga_status1 &= 0xFE;
}

void Video::vga_renderThread(void* dummy) {
		if (vga_doRender == 1) {
			vga_update(0, 0, vga_w - 1, vga_h - 1);
			vga_doRender = 0;
		}

		if (vga_doBlit == 1) {
		#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
			vm.renderer.draw(vga_framebuffer, (int)vga_w, (int)vga_h, VGA_FRAMEBUFFER_STRIDE);// * sizeof(uint32_t));
		#else
			vm.renderer.draw((uint32_t*)vga_framebuffer, (int)vga_w, (int)vga_h, VGA_FRAMEBUFFER_STRIDE);// * sizeof(uint32_t));
		#endif
			vga_doBlit = 0;
			//updatedscreen = false;
		}
}

void Video::vga_update(uint32_t start_x, uint32_t start_y, uint32_t end_x, uint32_t end_y) {
	#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
	uint32_t color16 = 0;
	#else
	uint32_t color32 = 0;
	#endif
	uint32_t addr, startaddr, cursorloc, cursor_x, cursor_y, fontbase = 0;
	uint32_t scx, scy, x, y, hchars, divx, yscanpixels, xscanpixels, xstride, bpp, pixelsperbyte, shift = 0;
	uint8_t cc, attr, fontdata, blink, mode, colorset, intensity, blinkenable, cursorenable, dup9 = 0;

	//log(Log, "[VIDEO] vga_update Width: %u", vga_crtcd[0x01] - ((vga_crtcd[0x05] & 0x60) >> 5));
	
	// Graphics mode enable
	//if (vga_attrd[0x10] & 1) { //ORIGINAL
	if (vga_gfxd[0x06] & 1) { //UPDATED
		if (vga_shiftmode & 0x02) {
			xscanpixels = 2;
			yscanpixels = (vga_crtcd[0x09] & 0x1F) + 1;
		} else {
			xscanpixels = (vga_seqd[0x01] & 0x08) ? 2 : 1;
			yscanpixels = (vga_crtcd[0x09] & 0x80) ? 2 : 1;
		}

		/*
		// Graphics Mode (GR05)
		 Shift Register Mode
		 65          --- Shift Direction -->
		 00  M0D0 M0D1 M0D2 M0D3 M0D4 M0D5 M0D6 M0D7  Bit0
		     M1D0 M1D1 M1D2 M1D3 M1D4 M1D5 M1D6 M1D7  Bit1
		     M2D0 M2D1 M2D2 M2D3 M2D4 M2D5 M2D6 M2D7  Bit2
		     M3D0 M3D1 M3D2 M3D3 M3D4 M3D5 M3D6 M3D7  Bit3
		 01  M1D0 M1D2 M1D4 M1D6 M0D0 M0D2 M0D4 M0D6  Bit0
		     M1D1 M1D3 M1D5 M1D7 M0D1 M0D3 M0D5 M0D7  Bit1
		     M3D0 M3D2 M3D4 M3D6 M2D0 M2D2 M2D4 M2D6  Bit2
		     M3D1 M3D3 M3D5 M3D7 M2D1 M2D3 M2D5 M2D7  Bit3
		 1x  M3D0 M3D4 M2D0 M2D4 M1D0 M1D4 M0D0 M0D4  Bit0
		     M3D1 M3D5 M2D1 M2D5 M1D1 M1D5 M0D1 M0D5  Bit1
		     M3D2 M3D6 M2D2 M2D6 M1D2 M1D6 M0D2 M0D6  Bit2
		     M3D3 M3D7 M2D3 M2D7 M1D3 M1D7 M0D3 M0D7  Bit3
		
		 Note: If the Shift Register is not loaded every character clock
		       then the four 8-bit shift registers are effectively "chained"
		       with the output of shift register 1 becoming the input to
		       shift register 0 and so on. This allows one to have a large
		       monochrome(or 4 color) bit map and display one portion thereof.
		       See SR01[2,4].
		*/
		
		// Normal shift mode
		switch (vga_shiftmode) {
		case 0x00:
			//TODO: is this the right way to detect 1bpp mode?
			if ((vga_attrd[0x12] & 0x0F) == 0x01) {
				bpp = 1;
				pixelsperbyte = 8;
				mode = VGA_MODE_GRAPHICS_1BPP;
			} else {
				bpp = 4;
				mode = VGA_MODE_GRAPHICS_4BPP;
				pixelsperbyte = 8;
			}
			break;
		// Packet shift mode
		case 0x01:
			bpp = 2;
			pixelsperbyte = 4;
			mode = VGA_MODE_GRAPHICS_2BPP;
			break;
		// 256 color shift mode
		case 0x02:
		case 0x03:
			bpp = 8;
			pixelsperbyte = 1;
			mode = VGA_MODE_GRAPHICS_8BPP;
			break;
		}
		xstride = (vga_w / xscanpixels) / pixelsperbyte;
		#ifdef DEBUG_VIDEO
		log(Log, "[VIDEO] vga_update Graphics Mode Resolution: %lu x %lu %lu bpp (X stride: %lu, V lpp: %lu, H lpp = %lu)",
			vga_w, vga_h, bpp, xstride, yscanpixels, xscanpixels);
		#endif
	} else { //text mode enable
		mode = VGA_MODE_TEXT;
		hchars = vga_dbl ? 40 : 80;
		divx = vga_dbl ? vga_dots * 2 : vga_dots;
		cursorenable = (vga_crtcd[0x0A] & 0x20) ? 0 : 1; //TODO: fix this
		blinkenable = 0;
		//log(Log, "[VIDEO] VGA_MODE_TEXT vga_seqd[0x03] = %u", vga_seqd[0x03]);
		fontbase = vga_fontbases[vga_seqd[0x03]];
		dup9 = (vga_attrd[0x10] & 0x04) ? 0 : 1;
		vga_scandbl = 0;
		#ifdef DEBUG_VIDEO
		log(Log, "[VIDEO] vga_update Text Mode Resolution: %lu x %lu (text mode)",	vga_w, vga_h);
		#endif
	}
	intensity = 0;
	colorset = 0;
	startaddr = ((uint32_t)vga_crtcd[0xC] << 8) | (uint32_t)vga_crtcd[0xD];
	cursorloc = ((uint32_t)vga_crtcd[0xE] << 8) | (uint32_t)vga_crtcd[0xF];

#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
	uint32_t y_offset;
	switch (mode) {
	case VGA_MODE_TEXT:
		// log(Log, "[VIDEO] vga_update VGA_MODE_TEXT");
		{
			dup9 = 1; //TODO: fix this hack
			cursor_x = cursorloc % hchars;
			cursor_y = cursorloc / hchars;
			//cursor_y = (cursorloc / hchars) + 1;
			uint32_t maxscan = ((vga_crtcd[0x09] & 0x1F) + 1);
			uint32_t console_x, console_y;
			uint32_t console_cols = (end_x / divx) + 1;
			uint32_t console_rows = (end_y / maxscan) + 1;
			uint8_t cursor_indicator_begin = vga_crtcd[VGA_REG_DATA_CURSOR_BEGIN] & 31;
			uint8_t cursor_indicator_end = vga_crtcd[VGA_REG_DATA_CURSOR_END] & 31;
			uint8_t cursor_blink;

			uint8_t char_x, char_y;
			addr = startaddr + start_x;
			y_offset = (VGA_FRAMEBUFFER_WIDTH * start_y) + start_x;
			uint16_t vga_attrd_x10_80 = vga_attrd[0x10] & 0x80;
			uint16_t vga_attrd_x14_3s4 = (vga_attrd[0x14] & 3) << 4;
			uint16_t vga_attrd_x14_Cs4 = (vga_attrd[0x14] & 0xC) << 4;
			uint8_t *font_p;
			for (console_y = 0; console_y < console_rows; ++console_y) {
				for (console_x = 0; console_x < console_cols; ++console_x) {
					cc = vga_RAM[0][addr];
					attr = vga_RAM[1][addr];
					blink = attr >> 7;
					cursor_blink = (console_y == cursor_y) && (console_x == cursor_x) && vga_cursor_blink_state && cursorenable;

					y_offset = (console_y * maxscan * VGA_FRAMEBUFFER_WIDTH) + (console_x * divx);
					//determine index into actual DAC palette
					color16 = vga_attrd[(attr & 0x0F)] | vga_attrd_x14_Cs4;
					//uint32_t color16_bg = vga_attrd[(attr >> 4)] | vga_attrd_x14_Cs4;
					uint16_t color16_bg = vga_attrd[(attr >> 4)] | vga_attrd_x14_Cs4;
					if (vga_attrd_x10_80) { //P5, P4 replace
						color16 = (color16 & 0xCF) | vga_attrd_x14_3s4;
						color16_bg = (color16_bg & 0xCF) | vga_attrd_x14_3s4;
					}
					color16 = vgaColor[color16]; //FG text color
					//color16 = 2;
					//color16 = vgaColor[120]; //FG Green
					//color16 = vgaColor[7]; //FG Monochrome
					//color16 = vgaColor[9]; //FG Blue
					//color16 = vgaColor[11]; //FG Cyan
					//color16 = vga_attrd_x14_Cs4;
					
					//color16_bg = vgaColor[color16_bg]; //BG text color
					color16_bg = vgaColor[color16_bg];

					font_p = &vga_RAM[2][fontbase + ((uint32_t)cc << 5)];
					for (char_y = 0; char_y < maxscan; ++char_y) {
						fontdata = *font_p++;
						for (char_x = 0; char_x < divx; ++char_x) {
							if ((cursor_blink) && (char_y >= cursor_indicator_begin) && (char_y <= cursor_indicator_end)) {
								vga_framebuffer[y_offset + char_x] = color16;
							}
							else {
								if (fontdata & 0b10000000) {
									vga_framebuffer[y_offset + char_x] = color16;
								}
								else {
									vga_framebuffer[y_offset + char_x] = color16_bg;
								}
							}
							fontdata <<= 1;
						}
						y_offset += VGA_FRAMEBUFFER_WIDTH;
					}
					++addr;
				}
				addr += hchars - console_cols;
			}
		}
		//log(Log, "[VIDEO] VGA_MODE_TEXT END");
		break;
	case VGA_MODE_GRAPHICS_8BPP:
		// log(Log, "[VIDEO] vga_update VGA_MODE_GRAPHICS_8BPP");
		for (scy = start_y; scy <= end_y; scy += yscanpixels) {
			y = scy / yscanpixels;
			for (scx = start_x; scx <= end_x; scx += xscanpixels) {
				uint8_t plane;
				uint32_t yadd, xadd;
				x = scx / xscanpixels;
				//x += vga_attrd[0x13] & 0x0F; //WAS COMMENTED
				addr = ((y * xstride) + x) & 0xFFFF;
				plane = addr & 3;
				addr = (addr >> 2) + startaddr;
				cc = vga_RAM[plane][addr & 0xFFFF];
				color16 = vgaColor[cc];
				y_offset = scy * VGA_FRAMEBUFFER_WIDTH;
				for (yadd = 0; yadd < yscanpixels; yadd++) {
					for (xadd = 0; xadd < xscanpixels; xadd++) {
						vga_framebuffer[y_offset + scx + xadd] = color16;
					}
					y_offset += VGA_FRAMEBUFFER_WIDTH;
				}
			}
		}
		break;
	case VGA_MODE_GRAPHICS_4BPP:
		// log(Log, "[VIDEO] vga_update VGA_MODE_GRAPHICS_4BPP");
		{
			y_offset = VGA_FRAMEBUFFER_WIDTH * start_y;
			// Color Plane Enable (AR12)
			uint16_t vga_attrd_x10_80 = vga_attrd[0x10] & 0x80;
			uint16_t vga_attrd_x12 = vga_attrd[0x12];
			uint16_t vga_attrd_x14_Cs4 = (vga_attrd[0x14] & 0xC) << 4;
			uint16_t vga_attrd_x14_3s4 = (vga_attrd[0x14] & 3) << 4;
			uint8_t i;
			for (y = start_y; y <= end_y; ++y) {
				addr = (startaddr + (y * xstride) + (start_x / 8)) & 0xFFFF;
				for (x = start_x; x <= end_x; x += 8) {
					uint16_t v1 = (uint16_t)vga_RAM[0][addr];
					uint16_t v2 = (uint16_t)vga_RAM[1][addr] << 1;
					uint16_t v3 = (uint16_t)vga_RAM[2][addr] << 2;
					uint16_t v4 = (uint16_t)vga_RAM[3][addr] << 3;

					i = 8;
					while (i--)
					{
						cc = ((v1 & 0b1) | (v2 & 0b10) | (v3 & 0b100) | (v4 & 0b1000)) & vga_attrd_x12;
						//determine index into actual DAC palette
						color16 = vga_attrd[cc] | vga_attrd_x14_Cs4; //UPDATED
						if (vga_attrd_x10_80) { //P5, P4 replace
							color16 &= 0xCF;
							color16 |= vga_attrd_x14_3s4;
						}
						vga_framebuffer[y_offset + x + i] = vgaColor[color16];
						v1 >>= 1;
						v2 >>= 1;
						v3 >>= 1;
						v4 >>= 1;
					}

					++addr;
				}
				y_offset += VGA_FRAMEBUFFER_WIDTH;
			}
		}
		break;
	case VGA_MODE_GRAPHICS_2BPP:
		{
			// log(Log, "[VIDEO] vga_update VGA_MODE_GRAPHICS_2BPP");
			uint16_t vga_attrd_x10_80 = vga_attrd[0x10] & 0x80;
			uint16_t vga_attrd_x14_3s4 = (vga_attrd[0x14] & 3) << 4;
			uint16_t vga_attrd_x14_Cs4 = (vga_attrd[0x14] & 0xC) << 4;
			for (scy = start_y; scy <= end_y; scy += yscanpixels) {
				uint8_t isodd;
				y = scy / yscanpixels;
				isodd = y & 1;
				y >>= 1;
				for (scx = start_x; scx <= end_x; scx += xscanpixels) {
					uint32_t yadd, xadd;
					x = scx / xscanpixels;
					//x += vga_attrd[0x13] & 0x0F;
					addr = ((8192 * isodd) + (y * xstride) + (x / pixelsperbyte)) & 0xFFFF;
					addr = addr + startaddr;
					shift = (3 - (x & 3)) << 1;
					cc = (vga_RAM[addr & 1][addr >> 1] >> shift) & 3;
					
					//cc |= ((vga_RAM[2 + addr & 1][addr >> 1] >> shift) & 3) << 2; //ADDED
					cc |= ((vga_RAM[(2 + addr) & 1][addr >> 1] >> shift) & 3) << 2; //ADDED
					// Color Plane Enable (AR12)
					cc &= vga_attrd[0x12]; //ADDED
					
					//determine index into actual DAC palette
					//color16 = vga_attrd[cc] | (vga_attrd[0x14] << 4); ORIGINAL
					color16 = vga_attrd[cc] | vga_attrd_x14_Cs4; //UPDATED
					
					if (vga_attrd_x10_80) { //P5, P4 replace
						color16 = (color16 & 0xCF) | vga_attrd_x14_3s4;
					}
					color16 = vgaColor[color16];
					uint32_t i;
					uint16_t *p = vga_framebuffer + (scy * VGA_FRAMEBUFFER_WIDTH) + scx;
					uint32_t xSkip = VGA_FRAMEBUFFER_WIDTH - xscanpixels;
					for (yadd = 0; yadd < yscanpixels; yadd++) {
						i = xscanpixels;
						while(i--)
						{
							*p++ = color16;
						}
						p += xSkip;
					}
				}
			}
		}
		break;
	case VGA_MODE_GRAPHICS_1BPP:
		// log(Log, "[VIDEO] vga_update VGA_MODE_GRAPHICS_1BPP");
		for (scy = start_y; scy <= end_y; scy += yscanpixels) {
			uint8_t isodd;
			y = scy / yscanpixels;
			isodd = y & 1;
			y >>= 1;
			for (scx = start_x; scx <= end_x; scx += xscanpixels) {
				uint32_t yadd, xadd;
				x = scx / xscanpixels;
				//x += vga_attrd[0x13] & 0x0F;
				addr = ((8192 * isodd) + (y * xstride) + (x / pixelsperbyte)) & 0xFFFF;
				addr = addr + startaddr;
				shift = 7 - (x & 7);
				cc = (vga_RAM[0][addr] >> shift) & 1;
				color16 = cc ? 0xFFFFFFFF : 0x00000000;
				uint32_t i;
				uint16_t *p = vga_framebuffer + (scy * VGA_FRAMEBUFFER_WIDTH) + scx;
				uint32_t xSkip = VGA_FRAMEBUFFER_WIDTH - xscanpixels;
				for (yadd = 0; yadd < yscanpixels; yadd++) {
					i = xscanpixels;
					while(i--)
					{
						*p++ = color16;
					}
					p += xSkip;
				}
			}
		}
		break;
	}
#else
	switch (mode) {
	case VGA_MODE_TEXT:
		dup9 = 1; //TODO: fix this hack
		cursor_x = cursorloc % hchars;
		cursor_y = cursorloc / hchars;
		//cursor_y = (cursorloc / hchars) + 1;
		for (scy = start_y; scy <= end_y; scy++) {
			uint32_t maxscan = ((vga_crtcd[0x09] & 0x1F) + 1);
			y = scy / maxscan;
			for (scx = start_x; scx <= end_x; scx++) {
				uint32_t charcolumn;
				x = scx / divx;
				addr = startaddr + (y * hchars) + x;
				cc = vga_RAM[0][addr];
				attr = vga_RAM[1][addr];
				blink = attr >> 7;
				if (blinkenable) attr &= 0x7F; //enabling text mode blink attribute limits background color selection
				//log(Log, "[VIDEO] VGA_MODE_TEXT %lu, %lu, %lu", fontbase, ((uint32_t)cc * 32), (scy % maxscan));
				fontdata = vga_RAM[2][fontbase + ((uint32_t)cc * 32) + (scy % maxscan)];
				charcolumn = ((scx >> (vga_dbl ? 1 : 0)) % vga_dots);
				if (dup9 && (charcolumn == 0) && (cc >= 0xC0) && (cc <= 0xDF)) {
					charcolumn = 1;
				}
				fontdata = (fontdata >> ((vga_dots - 1) - charcolumn)) & 1;
				
				if ((y == cursor_y) && (x == cursor_x) &&
					//((uint8_t)(scy % 16) >= (vga_crtcd[VGA_REG_DATA_CURSOR_BEGIN] & 31)) &&
					//((uint8_t)(scy % 16) <= (vga_crtcd[VGA_REG_DATA_CURSOR_END] & 31)) &&
					((uint8_t)(scy % 16) >= (vga_crtcd[VGA_REG_DATA_CURSOR_BEGIN + 2])) &&
					((uint8_t)(scy % 16) <= (vga_crtcd[VGA_REG_DATA_CURSOR_END - 2])) &&
					vga_cursor_blink_state && cursorenable) { //cursor should be displayed
					//color32 = vga_attrd[attr & 0x0F] | (vga_attrd[0x14] << 4); //ORIGINAL
					color32 = vga_attrd[attr & 0x0F] | ((vga_attrd[0x14] & 0xC) << 4); //UPDATED
					if (vga_attrd[0x10] & 0x80) { //P5, P4 replace
						color32 = (color32 & 0xCF) | ((vga_attrd[0x14] & 3) << 4);
					}
					color32 = vga_color(color32);
					vga_framebuffer[scy][scx] = color32;
				}
				else {
					if (blinkenable && blink && !vga_cursor_blink_state) {
						//all pixels in character get background color if blink attribute set and blink visible state is false
						fontdata = 0;
					}
					//determine index into actual DAC palette
					//color32 = vga_attrd[fontdata ? (attr & 0x0F) : (attr >> 4)] | (vga_attrd[0x14] << 4); //ORIGINAL
					color32 = vga_attrd[fontdata ? (attr & 0x0F) : (attr >> 4)] | ((vga_attrd[0x14] & 0xc) << 4); //UPDATED
					if (vga_attrd[0x10] & 0x80) { //P5, P4 replace
						color32 = (color32 & 0xCF) | ((vga_attrd[0x14] & 3) << 4);
					}
					color32 = vga_color(color32);
					vga_framebuffer[scy][scx] = color32;
				}
			}
		}
		//log(Log, "[VIDEO] VGA_MODE_TEXT END");
		break;
	case VGA_MODE_GRAPHICS_8BPP:
		for (scy = start_y; scy <= end_y; scy += yscanpixels) {
			y = scy / yscanpixels;
			for (scx = start_x; scx <= end_x; scx += xscanpixels) {
				uint8_t plane;
				uint32_t yadd, xadd;
				x = scx / xscanpixels;
				//x += vga_attrd[0x13] & 0x0F; //WAS COMMENTED
				addr = ((y * xstride) + x) & 0xFFFF;
				plane = addr & 3;
				addr = (addr >> 2) + startaddr;
				cc = vga_RAM[plane][addr & 0xFFFF];
				color32 = vga_color(cc);
				for (yadd = 0; yadd < yscanpixels; yadd++) {
					for (xadd = 0; xadd < xscanpixels; xadd++) {
						vga_framebuffer[scy + yadd][scx + xadd] = color32;
					}
				}
			}
		}
		break;
	case VGA_MODE_GRAPHICS_4BPP:
		for (scy = start_y; scy <= end_y; scy += yscanpixels) {
			y = scy / yscanpixels;
			for (scx = start_x; scx <= end_x; scx += xscanpixels) {
				uint32_t yadd, xadd;
				x = scx / xscanpixels;
				//x += vga_attrd[0x13] & 0x0F; //WAS COMMENTED
				addr = ((y * xstride) + (x / 8)) & 0xFFFF;
				addr = addr + startaddr;
				shift = 7 - (x & 7);
				cc = (vga_RAM[0][addr & 0xFFFF] >> shift) & 1;
				cc |= ((vga_RAM[1][addr & 0xFFFF] >> shift) & 1) << 1;
				cc |= ((vga_RAM[2][addr & 0xFFFF] >> shift) & 1) << 2;
				cc |= ((vga_RAM[3][addr & 0xFFFF] >> shift) & 1) << 3;
				
				// Color Plane Enable (AR12)
				cc &= vga_attrd[0x12]; //ADDED
				
				//determine index into actual DAC palette
				//color32 = vga_attrd[cc] | (vga_attrd[0x14] << 4); //ORIGINAL
				color32 = vga_attrd[cc] | ((vga_attrd[0x14] & 0xC) << 4); //UPDATED
				
				if (vga_attrd[0x10] & 0x80) { //P5, P4 replace
					color32 = (color32 & 0xCF) | ((vga_attrd[0x14] & 3) << 4);
				}
				color32 = vga_color(color32);
				for (yadd = 0; yadd < yscanpixels; yadd++) {
					for (xadd = 0; xadd < xscanpixels; xadd++) {
						vga_framebuffer[scy + yadd][scx + xadd] = color32;
					}
				}
			}
		}
		break;
	case VGA_MODE_GRAPHICS_2BPP:
		for (scy = start_y; scy <= end_y; scy += yscanpixels) {
			uint8_t isodd;
			y = scy / yscanpixels;
			isodd = y & 1;
			y >>= 1;
			for (scx = start_x; scx <= end_x; scx += xscanpixels) {
				uint32_t yadd, xadd;
				x = scx / xscanpixels;
				//x += vga_attrd[0x13] & 0x0F;
				addr = ((8192 * isodd) + (y * xstride) + (x / pixelsperbyte)) & 0xFFFF;
				addr = addr + startaddr;
				shift = (3 - (x & 3)) << 1;
				cc = (vga_RAM[addr & 1][addr >> 1] >> shift) & 3;
				
				//cc |= ((vga_RAM[2 + addr & 1][addr >> 1] >> shift) & 3) << 2; //ADDED
				cc |= ((vga_RAM[(2 + addr) & 1][addr >> 1] >> shift) & 3) << 2; //ADDED
				// Color Plane Enable (AR12)
				cc &= vga_attrd[0x12]; //ADDED
				
				//determine index into actual DAC palette
				//color32 = vga_attrd[cc] | (vga_attrd[0x14] << 4); ORIGINAL
				color32 = vga_attrd[cc] | ((vga_attrd[0x14] & 0xc) << 4); //UPDATED
				
				if (vga_attrd[0x10] & 0x80) { //P5, P4 replace
					color32 = (color32 & 0xCF) | ((vga_attrd[0x14] & 3) << 4);
				}
				color32 = vga_color(color32);
				for (yadd = 0; yadd < yscanpixels; yadd++) {
					for (xadd = 0; xadd < xscanpixels; xadd++) {
						vga_framebuffer[scy + yadd][scx + xadd] = color32;
					}
				}
			}
		}
		break;
	case VGA_MODE_GRAPHICS_1BPP:
		for (scy = start_y; scy <= end_y; scy += yscanpixels) {
			uint8_t isodd;
			y = scy / yscanpixels;
			isodd = y & 1;
			y >>= 1;
			for (scx = start_x; scx <= end_x; scx += xscanpixels) {
				uint32_t yadd, xadd;
				x = scx / xscanpixels;
				//x += vga_attrd[0x13] & 0x0F;
				addr = ((8192 * isodd) + (y * xstride) + (x / pixelsperbyte)) & 0xFFFF;
				addr = addr + startaddr;
				shift = 7 - (x & 7);
				cc = (vga_RAM[0][addr] >> shift) & 1;
				color32 = cc ? 0xFFFFFFFF : 0x00000000;
				for (yadd = 0; yadd < yscanpixels; yadd++) {
					for (xadd = 0; xadd < xscanpixels; xadd++) {
						vga_framebuffer[scy + yadd][scx + xadd] = color32;
					}
				}
			}
		}
		break;
	}
#endif
}

uint8_t Video::vga_readcrtci() {
	return vga_crtci;
}

uint8_t Video::vga_readcrtcd() {
	if (vga_crtci < 0x19) {
		return vga_crtcd[vga_crtci];
	}
	return 0xFF;
}

void Video::vga_writecrtci(uint8_t value) {
	vga_crtci = value & 0x1F;
}

void Video::vga_writecrtcd(uint8_t value) {
	if (vga_crtci > 0x18) return;

	//log(Log, "[VGA] vga_writecrtcd CRTC index %02X = %u\r\n", vga_crtci, value);
	
	vga_crtcd[vga_crtci] = value;

	switch (vga_crtci) {
	case 0x01:
	case 0x12:
	case 0x07:
		vga_calcscreensize();
		break;
	//case 0x09: //WAS DISABLED
	//	vga_scandbl = value & 0x1F; //(value & 0x80) ? 1 : 0;
	//	vga_calcscreensize();
	//	break;
	}
}

static const uint32_t lines[] = { 200, 400, 350, 480 };

void Video::vga_calcscreensize() {
	
	
	//ORIGINAL CODE
	/*
	vga_w = (1 + vga_crtcd[0x01] - ((vga_crtcd[0x05] & 0x60) >> 5)) * vga_dots;
	vga_h = (1 + vga_crtcd[0x12] | ((vga_crtcd[0x07] & 2) ? 0x100 : 0) | ((vga_crtcd[0x07] & 64) ? 0x200 : 0));

	if (((vga_shiftmode & 0x20) == 0) && (vga_seqd[0x01] & 0x08)) {
		vga_w <<= 1;
	}
	log(Log, "[VGA] vga_calcscreensize: %lu x %lu", vga_w, vga_h);
	return;
	*/


	//UPDATED CODE
	// Miscellaneous Output Register
	// 76 Vertical Size  Active Lines
	// 00 200 lines      206 lines
	// 76 Vertical Size  Active Lines  Note
	// 00 200 lines      206 lines     EGA
	// 01 400 lines      414 lines
	// 10 350 lines      362 lines
	// 11 480 lines      496 lines
	
	// SR01 8/9 Dot Clock
	vga_w = 80 * vga_dots;
	vga_h = lines[vga_misc >> 6]; //6
	
	//vga_updateScanlineTiming();
	vm.renderer.markScreenModeChanged(vga_w, vga_h);
	
	#ifdef DEBUG_VIDEO
	log(Log, "[VIDEO] vga_calcscreensize: %lu x %lu", vga_w, vga_h);
	#endif
}

void Video::vga_calcmemorymap() {
	switch (vga_gfxd[0x06] & 0x0C) {
		case 0x00: //0xA0000 - 0xBFFFF (128 KB)
			vga_membase = 0x00000;
			vga_memmask = 0xFFFF;
			break;
		case 0x04: //0xA0000 - 0xAFFFF (64 KB)
			vga_membase = 0x00000;
			vga_memmask = 0xFFFF;
			break;
		case 0x08: //0xB0000 - 0xB7FFF (32 KB)
			vga_membase = 0x10000;
			vga_memmask = 0x7FFF;
			break;
		case 0x0C: //0xB8000 - 0xBFFFF (32 KB)
			vga_membase = 0x18000;
			vga_memmask = 0x7FFF;
			break;
		}
	#ifdef DEBUG_VIDEO
	log(Log, "[VIDEO] vga_calcmemorymap vga_membase = %05X, vga_memmask = %04X\r\n", vga_membase, vga_memmask);
	#endif
}

void Video::vga_updateScanlineTiming() {

	double pixelclock;
	static uint32_t lastw = 0, lasth = 0;
	static double lastFPS = 0;

	//pixel clock select
	if (vga_misc & 0x04) pixelclock = 28322000.0;
	else pixelclock = 25175000.0;

	vga_hblankstart = (uint64_t)vga_crtcd[0x02] * (uint64_t)vga_dots;
	vga_hblankend = ((uint64_t)vga_crtcd[0x02] * (uint64_t)vga_dots) + (((uint64_t)vga_crtcd[0x03] & 0x1F) + 1) * (uint64_t)vga_dots;
	vga_hblanklen = vga_hblankend - vga_hblankstart;
	vga_vblankstart = (uint64_t)vga_crtcd[0x10] | ((uint64_t)(vga_crtcd[0x07] & 0x04) << 6) | ((uint64_t)(vga_crtcd[0x07] & 0x80) << 2);
	vga_vblankend = (uint64_t)vga_crtcd[0x06] | ((uint64_t)(vga_crtcd[0x07] & 0x01) << 8) | ((uint64_t)(vga_crtcd[0x07] & 0x20) << 4);
	vga_vblanklen = vga_vblankend - vga_vblankstart;
	vga_htotal = (uint64_t)vga_crtcd[0x00];
	vga_targetFPS = pixelclock / ((double)(vga_htotal + 5) * (double)vga_dots * (double)vga_vblankend);

	//get ratio of pixel clock vs our timer frequency for interval calculations
	//pixelclock = (double)timing_getFreq() / pixelclock;
	pixelclock = (double)vm.timing.getHostFreq() / pixelclock;
	
	vga_dispinterval = (uint64_t)((double)(vga_htotal + 5) * (double)vga_dots * pixelclock);
	vga_hblankinterval = (uint64_t)((double)vga_hblanklen * pixelclock);
	vga_vblankinterval = (uint64_t)((double)vga_hblankend * (double)vga_vblanklen * pixelclock);
	vga_frameinterval = (uint64_t)((double)vga_hblankend * (double)vga_vblankend * pixelclock);
	/*printf("hblank start = %llu, hblank end = %llu, hblank len = %llu, disp interval = %llu, hblank interval = %llu, freq = %llu\r\n",
		vga_hblankstart, vga_hblankend, vga_hblanklen, vga_dispinterval, vga_hblankinterval, timing_freq);
	printf("vblank start = %llu, vblank end = %llu, vblank len = %llu, vblank interval = %llu, frameinterval = %llu\r\n",
		vga_vblankstart, vga_vblankend, vga_vblanklen, vga_vblankinterval, vga_frameinterval);*/
	if ((lastw != vga_w) || (lasth != vga_h) || (lastFPS != vga_targetFPS)) {
		#ifdef DEBUG_VIDEO
		log(Log, "[VIDEO] vga_updateScanlineTiming Mode switch: %lu x %lu (%.02f Hz)", vga_w, vga_h, vga_targetFPS);
		log(Log, "[VIDEO] vga_updateScanlineTiming vga_dispinterval:%llu vga_hblankinterval:%llu vga_vblankinterval:%llu vga_frameinterval:%llu",
			vga_dispinterval, vga_hblankinterval, vga_vblankinterval, vga_frameinterval);
		#endif
		lastw = vga_w;
		lasth = vga_h;
		lastFPS = vga_targetFPS;
	}

	/*
	timing_updateInterval(vga_hblankTimer, vga_dispinterval);
	timing_updateInterval(vga_hblankEndTimer, vga_hblankinterval);
	timing_timerEnable(vga_hblankTimer);
	timing_timerDisable(vga_hblankEndTimer);
	if (vga_lockFPS == 0) {
		timing_updateIntervalFreq(vga_drawTimer, vga_targetFPS);
	}
	*/
}

uint8_t Video::vga_dologic(uint8_t value, uint8_t latch) {
	switch (vga_logicop) {
	case 0:
		return value;
	case 1:
		return value & latch;
	case 2:
		return value | latch;
	default: //3, but say default to stop MSVC from warning me
		return value ^ latch;
	}
}

void Video::writeVGA(uint32_t addr32, uint8_t value) 
{
	#ifdef DEBUG_VIDEO
	//log(Log, "[VIDEO] writeVGA %0X\r\n", addr32);
	#endif
	vga_writememory(nullptr, addr32, value);
}

void Video::vga_writememory(void* dummy, uint32_t addr, uint8_t value) {
	uint8_t temp, plane;
	if ((vga_misc & 0x02) == 0) return; //RAM writes are disabled
	addr -= 0xA0000;
	addr = (addr - vga_membase) & vga_memmask; //TODO: Is this right?

	if (vga_gfxd[0x05] & 0x10) { //host odd/even mode (text)
		vga_RAM[addr & 1][addr >> 1] = value;
		return;
	}

	if (vga_seqd[0x04] & 0x08) { //chain-4
		vga_RAM[addr & 3][addr >> 2] = value;
		return;
	}

	switch (vga_wmode) {
	case 0:
		for (plane = 0; plane < 4; plane++) {
			if (vga_enableplane & (1 << plane)) { //are we allowed to write to this plane?
				if (vga_gfxd[0x01] & (1 << plane)) { //test enable set/reset bit for plane
					temp = (vga_gfxd[0x00] & (1 << plane)) ? 0xFF : 0x00; //set/reset expansion as source data
				} else { //host data as source
					temp = vga_dorotate(value);
				}
				temp = vga_dologic(temp, vga_latch[plane]);
				temp = (temp & vga_gfxd[0x08]) | (vga_latch[plane] & (~vga_gfxd[0x08]));
				vga_RAM[plane][addr] = temp;
				//log(Log, "[VIDEO] vga_writememory mode 0 write plane %u\r\n", plane);
			}
		}
		break;
	case 1:
		for (plane = 0; plane < 4; plane++) {
			if (vga_enableplane & (1 << plane)) {
				vga_RAM[plane][addr] = vga_latch[plane];
				//log(Log, "[VIDEO] vga_writememory mode 1 write plane %u\r\n", plane);
			}
		}
		break;
	case 2:
		for (plane = 0; plane < 4; plane++) {
			if (vga_enableplane & (1 << plane)) {
				temp = (value & (1 << plane)) ? 0xFF : 0x00;
				temp = vga_dologic(temp, vga_latch[plane]);
				temp = (temp & vga_gfxd[0x08]) | (vga_latch[plane] & (~vga_gfxd[0x08]));
				vga_RAM[plane][addr] = temp;
			}
		}
		break;
	case 3:
		for (plane = 0; plane < 4; plane++) {
			if (vga_enableplane & (1 << plane)) {
				temp = (vga_gfxd[0x00] & (1 << plane)) ? 0xFF : 0x00;
				temp = (vga_dorotate(value) & vga_gfxd[0x08]) | (temp & (~vga_gfxd[0x08])); //bit mask logic
				vga_RAM[plane][addr] = temp;
			}
		}
		break;
	}
}

uint8_t Video::readVGA(uint32_t addr32) 
{
	#ifdef DEBUG_VIDEO
	//log(Log, "[VIDEO] readVGA %0X", addr32);
	#endif
	return vga_readmemory(nullptr, addr32);
	return (0); //this won't be reached, but without it some compilers give a warning
}

uint8_t Video::vga_readmemory(void* dummy, uint32_t addr) {
	uint8_t plane, ret;

	addr -= 0xA0000;
	addr = (addr - vga_membase) & vga_memmask; //TODO: Is this right?

	if (vga_gfxd[0x05] & 0x10) { //host odd/even mode (text)
		return vga_RAM[addr & 1][addr >> 1];
	}

	if (vga_seqd[0x04] & 0x08) { //chain-4
		return vga_RAM[addr & 3][addr >> 2];
	}

	vga_latch[0] = vga_RAM[0][addr];
	vga_latch[1] = vga_RAM[1][addr];
	vga_latch[2] = vga_RAM[2][addr];
	vga_latch[3] = vga_RAM[3][addr];

	if (vga_rmode == 0) {
		return vga_RAM[vga_readmap][addr];
	} else {
		//TODO: Is this correct?
		ret = 0;
		for (plane = 0; plane < 4; plane++) {
			if (vga_gfxd[0x07] & (1 << plane)) { //color don't care bit check
				if ((vga_RAM[plane][addr] & 0x0F) == (vga_gfxd[0x02] & 0x0F)) { //compare RAM value with color compare register
					ret |= 1 << plane; //set bit if true
				}
			}
		}
		return ret;
	}
}

bool Video::portWriteHandler(uint16_t portnum, uint8_t value)
{
	uint8_t* portram = vm.ports.portram;
	union CPU::_bytewordregs_& regs = vm.cpu.regs;

	#ifdef DEBUG_VIDEO
	//log(Log, "[VIDEO] portWriteHandler portnum:%0X", portnum);
	#endif
	
	switch (portnum) {
	case 0x3B4:
		if ((vga_misc & 1) == 0) {
			vga_writecrtci(value);
		}
		break;
	case 0x3B5:
		if ((vga_misc & 1) == 0) {
			vga_writecrtcd(value);
		}
		break;
	case 0x3C0:
	case 0x3C1:
		if (vga_attrflipflop == 0) {
			vga_attri = value & 0x1F;
			vga_attrpal = value & 0x20;
		}
		else {
			if (vga_attri < 0x15) {
				vga_attrd[vga_attri] = value;
			}
		}
		vga_attrflipflop ^= 1;
		break;
	case 0x3C7:
		vga_DAC.state = VGA_DAC_MODE_READ;
		vga_DAC.index = value;
		vga_DAC.step = 0;
		break;
	case 0x3C8:
		vga_DAC.state = VGA_DAC_MODE_WRITE;
		vga_DAC.index = value;
		vga_DAC.step = 0;
		break;
	case 0x3C9: //UPDATED
		//vga_DAC.pal[vga_DAC.index][vga_DAC.step++] = value & 0x3F;
		
		//RGB666 to RGB888 //ADDED
		vga_DAC.pal[vga_DAC.index][vga_DAC.step++] = value << 2 | (value >> 4 & 0x03);
		//vga_DAC.pal[vga_DAC.index][vga_DAC.step++] = (value << 2) | (value & 0x03);
		
		//#ifdef DEBUG_VIDEO
		//log(Log, "[VIDEO] vga_DAC.pal[%0X][%0X] = %0X", vga_DAC.index, vga_DAC.step, value);
		//#endif
		
		if (vga_DAC.step == 3) {
			//ORIGINAL CODE
			//vga_palette[vga_DAC.index][0] = vga_DAC.pal[vga_DAC.index][0] << 2;
			//vga_palette[vga_DAC.index][1] = vga_DAC.pal[vga_DAC.index][1] << 2;
			//vga_palette[vga_DAC.index][2] = vga_DAC.pal[vga_DAC.index][2] << 2;

		#if defined(ARDUINO) || (VIDEO_FRAMEBUFFER_DEPTH == 16)
			//vgaColor[vga_DAC.index] = ((((vga_DAC.pal[vga_DAC.index][0])&0xF8) << 8) | (((vga_DAC.pal[vga_DAC.index][1])&0xFC) << 3) | ((vga_DAC.pal[vga_DAC.index][2]) >> 3));
			if (vm.config.monitorDisplay) {
			switch (vm.config.monitorDisplay) {
		  	case MONITOR_DISPLAY_AMBER:
					vgaColor[vga_DAC.index] = ((vga_DAC.pal[vga_DAC.index][0]) & 0xF8) << 8; //Amber Gas Plasma
					break;
				case MONITOR_DISPLAY_GREEN:
					vgaColor[vga_DAC.index] = ((vga_DAC.pal[vga_DAC.index][1]) & 0xFC) << 3; //Green CRT Monochrome
					break;
				case MONITOR_DISPLAY_BLUE:
					vgaColor[vga_DAC.index] = (vga_DAC.pal[vga_DAC.index][2] & 0xFF) << 1; //Blue LCD
					break;
				case MONITOR_DISPLAY_BLUE_TOSH:
					//vgaColor[vga_DAC.index] = (vga_DAC.pal[vga_DAC.index][2] | 0x13) << 1; //Toshiba Blue LCD
					vgaColor[vga_DAC.index] = (vga_DAC.pal[vga_DAC.index][2] | 0x11) << 1; //Toshiba Blue LCD
					break;				
				case MONITOR_DISPLAY_TEAL_TERM:
					vgaColor[vga_DAC.index] = (vga_DAC.pal[vga_DAC.index][2] & 0xFF) << 3; //Teal Terminal
					break;
				case MONITOR_DISPLAY_GREEN_TERM:
					vgaColor[vga_DAC.index] = ((vga_DAC.pal[vga_DAC.index][1] | 0x10) << 7); //Green Terminal
					break;
				case MONITOR_DISPLAY_AMBER_TERM:
					vgaColor[vga_DAC.index] = ((vga_DAC.pal[vga_DAC.index][0] & 0xFB) << 8); //Amber Terminal
					break;
				default:
					vgaColor[vga_DAC.index] = ((((vga_DAC.pal[vga_DAC.index][0]) & 0xF8) << 8) | (((vga_DAC.pal[vga_DAC.index][1]) & 0xFC) << 3) | ((vga_DAC.pal[vga_DAC.index][2]) >> 3));
					break;
				}
			} else {
				vgaColor[vga_DAC.index] = ((((vga_DAC.pal[vga_DAC.index][0]) & 0xF8) << 8) | (((vga_DAC.pal[vga_DAC.index][1]) & 0xFC) << 3) | ((vga_DAC.pal[vga_DAC.index][2]) >> 3));
			}
		#endif
			
			//UPDATED CODE
			vga_DAC.step = 0;
			vga_DAC.index++;
		}
		break;
	case 0x3C2:
		vga_misc = value;
		break;
	case 0x3C4:
		vga_seqi = value & 0x1F;
		break;
	case 0x3C5:
		if (vga_seqi < 0x05) {
			vga_seqd[vga_seqi] = value;
			switch (vga_seqi) {
			case 0x01:
				vga_dots = (value & 0x01) ? 8 : 9;
				vga_dbl = (value & 0x08) ? 1 : 0; //HOW DOES THIS AFFECT TEXT MODE CURSOR ??
				//vga_calcscreensize(); //MAYBE COMMENT THIS ??
				break;
			case 0x02:
				vga_enableplane = value & 0x0F;
				break;
			}
		}
		break;
	case 0x3CE:
		vga_gfxi = value & 0x1F;
		break;
	case 0x3CF:
		if (vga_gfxi < 0x09) {
			vga_gfxd[vga_gfxi] = value;
			switch (vga_gfxi) {
			case 0x03:
				vga_rotate = value & 7;
				vga_logicop = (value >> 3) & 3;
				break;
			case 0x04:
				vga_readmap = value & 3;
				break;
			case 0x05:
				vga_wmode = value & 3;
				vga_rmode = (value >> 3) & 1;
				vga_shiftmode = (value >> 5) & 3;
				break;
			case 0x06:
				vga_calcmemorymap();
				break;
			}
		}
		break;
	case 0x3D4:
		if ((vga_misc & 1) == 1) {
			vga_writecrtci(value);
		}
		break;
	case 0x3D5:
		if ((vga_misc & 1) == 1) {
			vga_writecrtcd(value);
		}
		break;
	}

	return true;
}

bool Video::portReadHandler(uint16_t portnum, uint8_t& outValue)
{
	outValue = 0xFF;
	#ifdef DEBUG_VIDEO
	//log(Log, "[VIDEO] portReadHandler portnum:%03X", portnum);
	#endif
	
	switch (portnum) {
	case 0x3B4:
		if ((vga_misc & 1) == 0) {
			outValue = vga_readcrtci();
			return true;
		}
		break;
	case 0x3B5:
		if ((vga_misc & 1) == 0) {
			outValue = vga_readcrtcd();
			return true;
		}
		break;
	case 0x3C0:
		if (vga_attrflipflop == 0) {
			outValue = vga_attri | vga_attrpal;
		}
		else {
			if (vga_attri < 0x15) {
				outValue = vga_attrd[vga_attri];
			}
		}
		break;
	case 0x3C1:
		if (vga_attri < 0x15) {
			outValue = vga_attrd[vga_attri];
			return true;
		}
		break;
	case 0x3C4:
		outValue = vga_seqi;
		return true;
	case 0x3C5:
		if (vga_seqi < 0x05) {
			outValue = vga_seqd[vga_seqi];
			return true;
		}
		break;
	case 0x3C7:
		outValue = vga_DAC.state;
		return true;
	case 0x3C8:
		outValue = vga_DAC.index;
		return true;
	case 0x3C9:
		//outValue = vga_DAC.pal[vga_DAC.index][vga_DAC.step++]; //ORIGINAL
		outValue = vga_DAC.pal[vga_DAC.index][vga_DAC.step++] >> 2; //UPDATED
		if (vga_DAC.step == 3) {
			vga_DAC.step = 0;
			vga_DAC.index++;
		}
		break;
	case 0x3CC:
		outValue = vga_misc;
		return true;
	case 0x3CE:
		outValue = vga_gfxi;
		return true;
	case 0x3CF:
		if (vga_gfxi < 0x09) {
			outValue = vga_gfxd[vga_gfxi];
			return true;
		}
		break;
	case 0x3D4:
		if ((vga_misc & 1) == 1) {
			outValue = vga_readcrtci();
			return true;
		}
		break;
	case 0x3D5:
		if ((vga_misc & 1) == 1) {
			outValue = vga_readcrtcd();
			return true;
		}
		break;
	case 0x3DA:
		//updatedscreen = true;
		vga_attrflipflop = 0; //because VGA is weird
		//outValue = vga_status1; //NEED TO FIX THE TIMING
		outValue = port3da; //USE THIS FOR NOW
		return true;
	}
	return true;
}

