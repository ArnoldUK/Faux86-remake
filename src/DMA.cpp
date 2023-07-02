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

/* 
	Intel i8237 Direct Memory Access controller
	Functions to emulate the Intel 8237 DMA controller.
  the Sound Blaster emulation functions rely on this!
*/

#include "VM.h"
#include "DMA.h"
#include "MemUtils.h"

using namespace Faux86;

uint8_t DMA::read (uint8_t channel) 
{
	uint8_t ret;
	if (channels[channel].masked) return (128);
	if (channels[channel].autoinit && (channels[channel].count > channels[channel].reload) ) channels[channel].count = 0;
	if (channels[channel].count > channels[channel].reload) return (128);
	//if (channels[channel].direction) ret = RAM[channels[channel].page + channels[channel].addr + channels[channel].count];
	//	else ret = RAM[channels[channel].page + channels[channel].addr - channels[channel].count];
	if (channels[channel].direction == 0)
	{
		ret = vm.memory.RAM[channels[channel].page + channels[channel].addr + channels[channel].count];
	}
	else
	{
		ret = vm.memory.RAM[channels[channel].page + channels[channel].addr - channels[channel].count];
	}
	channels[channel].count++;
	return (ret);
}

bool DMA::portWriteHandler(uint16_t addr, uint8_t value)
{
	uint8_t channel = value & 3;
	#ifdef DEBUG_DMA
	log(Log,"[DMA] portWriteHandler (0x%X, %X)", addr, value);
	#endif
	switch (addr) {
		case 0x2: //channel 1 address register
			if (flipflop == 1) channels[1].addr = (channels[1].addr & 0x00FF) | ( (uint32_t) value << 8);
			else channels[1].addr = (channels[1].addr & 0xFF00) | value;
			#ifdef DEBUG_DMA
			if (flipflop == 1) log(Log,"[DMA] DMA channel 1 address register = %04X", channels[1].addr);
			#endif
			flipflop = ~flipflop & 1;
			break;
		case 0x3: //channel 1 count register
			if (flipflop == 1) channels[1].reload = (channels[1].reload & 0x00FF) | ( (uint32_t) value << 8);
			else channels[1].reload = (channels[1].reload & 0xFF00) | value;
			if (flipflop == 1) {
				if (channels[1].reload == 0) channels[1].reload = 65536;
				channels[1].count = 0;
				#ifdef DEBUG_DMA
				log(Log,"[DMA] DMA channel 1 reload register = %04X", channels[1].reload);
				#endif
			}
			flipflop = ~flipflop & 1;
			break;
		case 0xA: //write single mask register
			//channel = value & 3;
			channels[channel].masked = (value >> 2) & 1;
			#ifdef DEBUG_DMA
			log(Log,"[DMA] DMA channel %u masking = %u", channel, channels[channel].masked);
			#endif
			break;
		case 0xB: //write mode register
			//channel = value & 3;
			channels[channel].direction = (value >> 5) & 1;
			channels[channel].autoinit = (value >> 4) & 1;
			channels[channel].writemode = (value >> 2) & 1; //not quite accurate
			#ifdef DEBUG_DMA
			log(Log,"[DMA] DMA channel %u write mode reg: direction = %u, autoinit = %u, write mode = %u",
			  channel, channels[channel].direction, channels[channel].autoinit, channels[channel].writemode);
			#endif
			break;
		case 0xC: //clear byte pointer flip-flop
			#ifdef DEBUG_DMA
			log(Log,"[DMA] DMA cleared byte pointer flip-flop");
			#endif
			flipflop = 0;
			break;
		case 0x83: //DMA channel 1 page register
			channels[1].page = (uint32_t) value << 16;
			#ifdef DEBUG_DMA
			log(Log,"[DMA] DMA channel 1 page base = %05X", channels[1].page);
			#endif
			break;
		}
	return true;
}

bool DMA::portReadHandler(uint16_t addr, uint8_t& outValue)
{
	#ifdef DEBUG_DMA
	log(Log,"[DMA] portReadHandler (0x%X)", addr);
	#endif
	if (addr & 0x80) {
    // this is a DMA page register
  } else {
	switch (addr) {
		case 3:
			if (flipflop == 1)
			{
				outValue = (channels[1].reload >> 8);
				return true;
			}
			else
			{
				outValue = (channels[1].reload);
				return true;
			}
			flipflop = ~flipflop & 1;
			break;
		}
	}
	outValue = 0;
	return true;
}

DMA::DMA(VM& inVM)
	: vm(inVM)
{
	log(Log,"[DMA] Constructed");
}

void DMA::init()
{
	log(Log,"[DMA] Initialized");
	MemUtils::memset (channels, 0, sizeof (Channel) * NumDMAChannels );

	vm.ports.setPortRedirector(0x00, 0x0F, this);
	vm.ports.setPortRedirector(0x80, 0x8F, this);
}
