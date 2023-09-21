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
	Intel 8255 Programmable Peripheral Interface (PPI)
	This is not complete.
*/


#include "VM.h"
#include "PCSpeaker.h"
#include "PPI.h"

//#define DEBUG_PPI

#ifdef DEBUG_PPI
static uint32_t timingcalls = 0;
#endif

using namespace Faux86;

bool PPI::portReadHandler(uint16_t portnum, uint8_t& outValue) {
	#ifdef DEBUG_PPI
	log(Log,"[PPI::I8255] portReadHandler port 0x%X", portnum);
	#endif
	portnum &= 7;
	switch (portnum) {
	case 0:
		//outValue = keystate->scancode;
	case 1:
		outValue = portB;
	case 2:
		if (portB & 8) {
			outValue = sw2 >> 4;
		} else {
			outValue = sw2 & 0x0F;
		}
	}
	outValue = 0xFF;
	return true;
}

bool PPI::portWriteHandler(uint16_t portnum, uint8_t value) {
	#ifdef DEBUG_PPI
	log(Log,"[PPI::I8255] portWriteHandler port 0x%X: %X", portnum, value);
	#endif
	portnum &= 7;
	switch (portnum) {
	case 0:
		//keystate->scancode = 0xAA;
		break;
	case 1:
		if (value & 0x01) {
			//vm.pcSpeaker.selectGate(PC_SPEAKER_USE_TIMER2);
			#ifdef DEBUG_PPI
			log(Log,"[PPI::I8255] PC_SPEAKER_USE_TIMER2");
			#endif
		} else {
			vm.pcSpeaker.selectGate(PC_SPEAKER_USE_DIRECT);
			#ifdef DEBUG_PPI
			log(Log,"[PPI::I8255] PC_SPEAKER_USE_DIRECT");
			#endif
		}
		//vm.pcSpeaker.setGateState(PC_SPEAKER_GATE_DIRECT, (value >> 1) & 1);
		#ifdef DEBUG_PPI
		log(Log,"[PPI::I8255] PC_SPEAKER_GATE_DIRECT = %u", (value >> 1) & 1);
		#endif
		if ((value & 0x40) && !(portB & 0x40)) {
			//keystate->scancode = 0xAA;
			#ifdef DEBUG_PPI
			log(Log,"[PPI::I8255] Keyboard Reset");
			#endif
		}
		portB = (value & 0xEF) | (portB & 0x10);
		break;
	}
	return true;
}

void PPI::dramRefreshToggle(void) {
	#ifdef DEBUG_PPI
	timingcalls++;
	if (timingcalls > 100000) {
		timingcalls = 0;
		log(Log,"[PPI::I8255] dramRefreshToggle\r\n");
	}
	#endif
	portB ^= 0x10; //simulate DRAM refresh toggle required by some BIOS
}

/*
void i8255_init(I8255_t* i8255, KEYSTATE_t* keystate, PCSPEAKER_t* pcspeaker) {
	memset(i8255, 0, sizeof(I8255_t));
	i8255->keystate = keystate;
	i8255->pcspeaker = pcspeaker;

	if (videocard == VIDEO_CARD_VGA) {
		i8255->sw2 = 0x46;
	}
	else if (videocard == VIDEO_CARD_CGA) {
		i8255->sw2 = 0x66;
	}

	ports_cbRegister(0x60, 6, (void*)i8255_readport, NULL, (void*)i8255_writeport, NULL, i8255);
	timing_addTimer(i8255_refreshToggle, i8255, 66667, TIMING_ENABLED, "[i8255] i8255_refreshToggle");
}
*/

PPI::PPI(VM& inVM)
	: sw2(0)
	, portA(0)
	, portB(0)
	, portC(0)
	, vm(inVM)
{
	log(Log,"[PPI::I8255] Constructed");
	
	//sw2 = 0x66; //VIDEO_CARD_CGA
	sw2 = 0x46; //VIDEO_CARD_VGA
	
	vm.ports.setPortRedirector(0x60, 0x66, this);
}

