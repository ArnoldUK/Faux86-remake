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
	Intel i8253 Programmable Interval Timer.
  Functions required for the timer interrupt and PC speaker to be emulated!
*/

#include "VM.h"
#include "PCSpeaker.h"
#include "PIT.h"

using namespace Faux86;


void PIT::tick() {
	//log(Log,"[PIT::I8253] tick");
	//The PIT timing needs working on return for now and handle in Timining.cpp
	return;
	
	uint8_t i;

	for (i = 0; i < 3; i++) {
		if ((i == 2) && (mode[2] != 3)) vm.pcSpeaker.setGateState(PC_SPEAKER_GATE_TIMER2, 0);
		if (active[i]) {
		//timing_calls++;
		//if (timing_calls > 1000) {
		//	timing_calls = 0;
			//debug_debug("[I8253] i8253_tickCallback %u mode=%u counter=%lu", i, i8253->mode[i], i8253->counter[i]);
		//}
		switch (mode[i]) {
		case 0: //interrupt on terminal count
			counter[i] -= 25;//25;
			if (counter[i] <= 0) {
				counter[i] = 0;
				out[i] = 1;
				if (i == 0) vm.pic.doirq(0);
			}
			break;
		case 2: //rate generator
			//if (i == 0) vm.pic.doirq(0);
			//if (timing_calls > 50000) {
			//	timing_calls = 0;
			//	debug_debug("[I8253] i8253_tickCallback %u mode=%u counter=%lu", i, i8253->mode[i], i8253->counter[i]);
			//}
			counter[i] -= 25;//25;
			if (counter[i] <= 0) {
				out[i] ^= 1;
				//if (out[i] == 0) {
					if (i == 0) vm.pic.doirq(0);
				//}
				counter[i] += reload[i];
			}
			break;
		case 3: //square wave generator
			//if (i == 0) vm.pic.doirq(0);
			counter[i] -= 50;//50;
			if (counter[i] <= 0) {
				out[i] ^= 1;
				if (out[i] == 0) {
					if (i == 0) vm.pic.doirq(0);
				}
				if (i == 2) vm.pcSpeaker.setGateState(PC_SPEAKER_GATE_TIMER2, (reload[i] < 50) ? 0 : out[i]);
				counter[i] += reload[i];
			}
			break;
		default:
			#ifdef DEBUG_PIT
			//debug_log(DEBUG_DETAIL, "I8253: Unknown mode %u on counter %u\r\n", i8253->mode[i], i);
			#endif
			break;
		}
	}
	}
}


//PREVIOUS CODE WORKS!!
bool PIT::portWriteHandler(uint16_t portnum, uint8_t value)
{
	uint8_t curbyte = 0;
	portnum &= 3;
	switch (portnum) 
	{
		case 0: //load counters
		case 1:
		case 2: //channel data
			if ( (accessmode[portnum] == Mode::LoByte) || ( (accessmode[portnum] == Mode::Toggle) && (bytetoggle[portnum] == 0) ) ) 
				curbyte = 0;
			else if ( (accessmode[portnum] == Mode::HiByte) || ( (accessmode[portnum] == Mode::Toggle) && (bytetoggle[portnum] == 1) ) ) 
				curbyte = 1;
			if (curbyte == 0) 
			{ //low byte
				chandata[portnum] = (chandata[portnum] & 0xFF00) | value;
			}
			else 
			{   //high byte
				chandata[portnum] = (chandata[portnum] & 0x00FF) | ( (uint16_t) value << 8);
			}
			if (chandata[portnum] == 0) effectivedata[portnum] = 65536;
			else effectivedata[portnum] = chandata[portnum];
			active[portnum] = 1;
			vm.timing.tickgap = (uint64_t) ( (float) vm.timing.getHostFreq() / (float) ( (float) 1193182 / (float) effectivedata[0]) );
			//vm.timing.i8253tickgap = (uint64_t) ( (float) vm.timing.getHostFreq() / (float) ( (float) 1193182 / (float) effectivedata[0]) );
			if (accessmode[portnum] == Mode::Toggle) 
				bytetoggle[portnum] = (~bytetoggle[portnum]) & 1;
			chanfreq[portnum] = (float) ( (uint32_t) ( ( (float) 1193182.0 / (float) effectivedata[portnum]) * (float) 1000.0) ) / (float) 1000.0;
			//printf("[DEBUG] PIT channel %u counter changed to %u (%f Hz)\n", portnum, chandata[portnum], chanfreq[portnum]);
			break;
		case 3: //mode command
			accessmode[value>>6] = (value >> 4) & 3;
			if (accessmode[value>>6] == Mode::Toggle) 
				bytetoggle[value>>6] = 0;
			break;
	}
	return true;
}


/*
bool PIT::portWriteHandler(uint16_t portnum, uint8_t value)
{
	uint8_t sel, rl, loaded = 0;
	portnum &= 3;

	switch (portnum) {
	case 0: //load counters
	case 1:
	case 2:
		switch (rlmode[portnum]) {
		case 1: //MSB only
			reload[portnum] = (int32_t)value << 8;
			active[portnum] = 1;
			loaded = 1;
			break;
		case 2: //LSB only
			reload[portnum] = value;
			active[portnum] = 1;
			loaded = 1;
			break;
		case 3: //LSB, then MSB
			if (dataflipflop[portnum] == 0) { //LSB
				reload[portnum] = (reload[portnum] & 0xFF00) | value;
			} else { //MSB
				reload[portnum] = (reload[portnum] & 0x00FF) | ((int32_t)value << 8);
				counter[portnum] = reload[portnum];
				if (reload[portnum] == 0) {
					reload[portnum] = 65536;
				}
				active[portnum] = 1;
				loaded = 1;
				
				//ADDED
				//tick();
				vm.timing.tickgap = (uint64_t) ( (float) vm.timing.getHostFreq() / (float) ( (float) 1193182 / (float) reload[0]) );
				//vm.timing.tickgap = (uint64_t) ( (float) vm.timing.getHostFreq() / (float) ( (float) 119318 / (float) reload[0]) );
				if (accessmode[portnum] == Mode::Toggle) 
				bytetoggle[portnum] = (~bytetoggle[portnum]) & 1;
				chanfreq[portnum] = (float) ( (uint32_t) ( ( (float) 1193182.0 / (float) reload[portnum]) * (float) 1000.0) ) / (float) 1000.0;
			
				#ifdef DEBUG_PIT
				//debug_log(DEBUG_DETAIL, "I8253: Counter %u reload = %d\r\n", portnum, i8253->reload[portnum]);
				#endif
			}
			dataflipflop[portnum] ^= 1;
			break;
		}
		if (loaded) switch (mode[portnum]) {
		case 0:
		case 1:
			out[portnum] = 0;
			break;
		case 2:
		case 3:
			out[portnum] = 1;
			break;
		}
		break;
	case 3: //control word
		sel = value >> 6;
		if (sel == 3) { //illegal
			return true;
		}
		rl = (value >> 4) & 3; //read/load mode
		if (rl == 0) { //counter latching operation
			latch[sel] = counter[sel];
		} else { //set mode
			rlmode[sel] = rl;
			mode[sel] = (value >> 1) & 7;
			if (mode[sel] & 0x02) {
				mode[sel] &= 3; //MSB is "don't care" if bit 1 is set
			}
			bcd[sel] = value & 1;
			#ifdef DEBUG_PIT
			//debug_log(DEBUG_DETAIL, "I8253: Counter %u mode = %u\r\n", sel, i8253->mode[sel]);
			#endif
		}
		dataflipflop[sel] = 0;
		break;
	}
	return true;
}
*/


//PREVIOUS CODE WORKS!!
bool PIT::portReadHandler(uint16_t portnum, uint8_t& outValue)
{
	outValue = 0;
	uint8_t curbyte = 0;
	portnum &= 3;
	
	switch (portnum) 
	{
		case 0:
		case 1:
		case 2: //channel data
			if ( (accessmode[portnum] == 0) || (accessmode[portnum] == Mode::LoByte) || ( (accessmode[portnum] == Mode::Toggle) && (bytetoggle[portnum] == 0) ) ) 
				curbyte = 0;
			else if ( (accessmode[portnum] == Mode::HiByte) || ( (accessmode[portnum] == Mode::Toggle) && (bytetoggle[portnum] == 1) ) ) 
				curbyte = 1;
			if ( (accessmode[portnum] == 0) || (accessmode[portnum] == Mode::LoByte) || ( (accessmode[portnum] == Mode::Toggle) && (bytetoggle[portnum] == 0) ) ) 
				curbyte = 0;
			else if ( (accessmode[portnum] == Mode::HiByte) || ( (accessmode[portnum] == Mode::Toggle) && (bytetoggle[portnum] == 1) ) ) 
				curbyte = 1;
			if ( (accessmode[portnum] == 0) || (accessmode[portnum] == Mode::Toggle) ) 
				bytetoggle[portnum] = (~bytetoggle[portnum]) & 1;
			if (curbyte == 0) 
			{ //low byte
				outValue = ( (uint8_t) counter[portnum]);
			}
			else 
			{   //high byte
				outValue = ( (uint8_t) (counter[portnum] >> 8) );
			}
			return true;
	}
	return true;
}


/*
bool PIT::portReadHandler(uint16_t portnum, uint8_t& outValue) {
	uint8_t ret;
	portnum &= 3;

	if (portnum == 3) {
		outValue = 0xFF; //no read of control word possible
		return true;
	}

	switch (rlmode[portnum]) {
	case 1: //MSB only
		outValue = latch[portnum] >> 8;
	case 2: //LSB only
		outValue = (uint8_t)latch[portnum];
	default: //LSB, then MSB (case 3, but say default so MSVC stops warning me about control paths not all returning a value)
		if (dataflipflop[portnum] == 0) { //LSB
			outValue = (uint8_t)latch[portnum];
		} else { //MSB
			outValue = latch[portnum] >> 8;
		}
		dataflipflop[portnum] ^= 1;
	}
	
	return true;
}
*/


PIT::PIT(VM& inVM)
	: chandata { 0, 0, 0 }
	, accessmode { 0, 0, 0 }
	, bytetoggle { 0, 0, 0 }
	, effectivedata { 0, 0, 0 }
	, chanfreq { 0, 0, 0 }
	, active { 0, 0, 0 }
	, counter { 0, 0, 0 }
	, reload { 0, 0, 0 }
	, mode { 0, 0, 0 }
	, dataflipflop { 0, 0, 0 }
	, bcd { 0, 0, 0 }
	, rlmode { 0, 0, 0 }
	, latch { 0, 0, 0 }
	, out { 0, 0, 0 }
	, vm(inVM)
{
	log(Log,"[PIT::I8253] Constructed");
	//uint32_t utimer = timing_addTimer(i8253_tickCallback, (void*)(&i8253->cbdata), 48000, TIMING_ENABLED, "[i8253] tickCallback"); //79545.47

	vm.ports.setPortRedirector(0x40, 0x43, this);
}

