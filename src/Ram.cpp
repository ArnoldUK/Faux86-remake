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

#ifdef _WIN32
#include <memory.h>
#elif defined(ARDUINO)
#include <stdio.h>
#include <string.h>
#else
#include <circle/new.h>
#include <circle/util.h>
#endif

#include "Ram.h"
#include "VM.h"
#include "Debugger.h"

using namespace Faux86;

Memory::Memory(VM& inVM) :
	vm(inVM)
{
	log(Log,"[RAM] Memory Constructed");
	RAM = new uint8_t[vm.config.ramSize];
	readonly = new uint8_t[vm.config.ramSize];

	reset();
}

void Memory::reset()
{
	log(Log,"[RAM] Memory Reset");
	memset(RAM, 0, vm.config.ramSize);
	memset(readonly, 0, vm.config.ramSize);
}

Memory::~Memory()
{
	delete[] RAM;
	delete[] readonly;
}

void Memory::writeByte(uint32_t addr32, uint8_t value) 
{
	//uint32_t tempaddr32 = addr32 & MEMORY_MASK;
	addr32 &= MEMORY_MASK;
	
#ifdef CPU_ADDR_MODE_CACHE
	if (!vm.memory.readonly[addr32]) addrcachevalid[addr32] = 0;
#endif

	//if (vm.memory.readonly[tempaddr32] || (tempaddr32 >= 0xC0000)) return;

	//if (RAM[addr32] != NULL) {
		RAM[addr32] = value;
	//}

	if ((addr32 >= 0xA0000) && (addr32 <= 0xBFFFF)) //0x20000
	{
		//vm.renderer.onMemoryWrite(addr32, value);
		//vm.video.writeVGA(tempaddr32 - 0xA0000, value);
		vm.video.writeVGA(addr32, value);
		vm.video.updatedscreen = 1;
	}

	//if (vm.debugger) vm.debugger->onMemoryWrite(tempaddr32);
}

void Memory::writeWord(uint32_t addr32, uint16_t value)
{
	writeByte(addr32, (uint8_t)value);
	writeByte(addr32 + 1, (uint8_t)(value >> 8));
}

uint8_t Memory::readByte(uint32_t addr32) 
{
	//if (addr32 >= 0xC0000UL && addr32 < 0xC0000UL + 0x8000)
	//{
	//	log(Log, "Read vid byte 0x%x : %x", addr32, RAM[addr32]);
	//}

	addr32 &= MEMORY_MASK;
	
	if (!vm.cpu.didbootstrap) 
	{
		RAM[0x410] = 0x41; //ugly hack to make BIOS always believe we have an EGA/VGA card installed
		RAM[0x475] = vm.drives.hdcount; //the BIOS doesn't have any concept of hard drives, so here's another hack
		
		//BELOW WAS COMMENTED OUT
		// 256K video RAM
		//RAM[0x487] = 0x60;

		// base port of active CRT controller : 3B4h = mono, 3D4h = color
		//RAM[0x463] = 0xd4;	
		//RAM[0x464] = 0x03;

		// VGA active
		RAM[0x489] = 0x01;
		//ABOVE WAS COMMENTED OUT
	}
	
	//if (RAM[addr32] != 0) {
		//return (RAM[addr32]);
	//}
	//return (RAM[addr32]);
	
	if ((addr32 >= 0xA0000) && (addr32 <= 0xBFFFF)) {
		return vm.video.readVGA(addr32);
	} else {
		return (RAM[addr32]);
	}


	return 0xFF;
}

uint16_t Memory::readWord(uint32_t addr32) 
{
	return ((uint16_t)readByte(addr32) | (uint16_t)(readByte(addr32 + 1) << 8));
}

uint32_t Memory::loadBinary(uint32_t addr32, DiskInterface* file, uint8_t roflag, uint32_t debugFlags) 
{
	if (!file)
		return 0;
	if (!file->isValid())
	{
		return 0;
	}

	uint32_t fileSize = (uint32_t)file->getSize();
	file->seek(0);
	file->read(&vm.memory.RAM[addr32], fileSize);
	memset((void *)&vm.memory.readonly[addr32], roflag, fileSize);

	//if (vm.debugger) vm.debugger->flagRegion(addr32, fileSize, debugFlags);

	return fileSize;
}
