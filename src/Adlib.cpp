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

#include "Adlib.h"
#include "VM.h"
#include "MemUtils.h"

using namespace Faux86;


bool Adlib::portWriteHandler(uint16_t portnum, uint8_t value)
{
	if (portnum & 1) {
		//OPL3_WriteReg(&opl3, targetRegister, value);
		OPL3_WriteRegBuffered(&opl3, targetRegister, value);

		if (targetRegister == 4) {
			timerRegister = (value & 0x80) ? 0 : value;
		}
	}	else {
		targetRegister = value;
	}

	return true;
}


/*
bool Adlib::portWriteHandler(uint16_t portnum, uint8_t value) {
    static Bit16u port = 0;
    switch (portnum) {
    case 0x388:
			port = value;
			break;
    case 0x389:
			if (port == 0x04) opl3.data4 = value;
			OPL3_WriteRegBuffered(&opl3, (Bit16u)port, value);
			break;
    }
	return true;
}
*/


bool Adlib::portReadHandler(uint16_t portnum, uint8_t& outValue) 
{
	uint8_t status = timerRegister ? 0x80 : 0;
	status += (timerRegister & 1) * 0x40 + (timerRegister & 2) * 0x10;
	outValue = status;
	return true;
}


/*
bool Adlib::portReadHandler(uint16_t portnum, uint8_t& outValue) {
    portnum &= 1;
    if (portnum == 0) { //status port
			outValue = (opl3.data4 & 0x01) ? 0x40 : 0x00;
			outValue |= (opl3.data4 & 0x02) ? 0x20 : 0x00;
			outValue |= outValue ? 0x80 : 0x00;
    } else {
       outValue = 0xFF;
    }
	return true;
}
*/

int16_t Adlib::generateSample() 
{
	int16_t buffer[2];
	OPL3_Generate(&opl3, buffer);

	// Currently force to mono
	return buffer[0];
}

void Adlib::tick() 
{
}
	
Adlib::Adlib(VM& inVM)
	: vm(inVM)
{
	//#ifdef DEBUG_AUDIO
	log(Log,"[ADLIB] Constructed");
	//#endif
}

void Adlib::init()
{
	//#ifdef DEBUG_AUDIO
	log(Log,"[ADLIB] Initialized");
	//#endif
	uint16_t baseport = vm.config.adlib.port;
	vm.ports.setPortRedirector(baseport, baseport + 1, this);
	OPL3_Reset(&opl3, vm.config.audio.sampleRate);
}
