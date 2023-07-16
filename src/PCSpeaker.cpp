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

#include "VM.h"
#include "PCSpeaker.h"

using namespace Faux86;

PCSpeaker::PCSpeaker(VM& inVM)
	: vm(inVM)
{
	log(Log,"[PCSPEAKER] Constructed");
}

int16_t PCSpeaker::generateSample() 
{
	//if (!vm->config->usePCSpeaker) return 0;
	if (!enabled) return 0;
	
	int16_t speakervalue;
	//float chanfreq = vm.pit.chanfreq[2];

	speakerfullstep = (uint64_t) ( (float)vm.config.audio.sampleRate / (float)vm.pit.chanfreq[2] );
	if (speakerfullstep < 2) speakerfullstep = 2;

	speakerhalfstep = speakerfullstep >> 1;
	if (speakercurstep < speakerhalfstep) speakervalue = 32;
	else speakervalue = -32;

	speakercurstep = (speakercurstep + 1) % speakerfullstep;
	return (speakervalue);
}

void PCSpeaker::setGateState(uint8_t gate, uint8_t value) {
	//debug_notice("[pcspeaker] pcspeaker_setGateState request");
	speakergate[gate] = value;
}

void PCSpeaker::selectGate(uint8_t value) {
	//debug_notice("[pcspeaker] pcspeaker_selectGate request");
	speakergateselect = value;
}

void PCSpeaker::tick() {
	//debug_debug("[pcspeaker] pcspeaker_callback");
	
	if (speakergateselect == PC_SPEAKER_USE_TIMER2) {
		if (speakergate[PC_SPEAKER_GATE_TIMER2] && speakergate[PC_SPEAKER_GATE_DIRECT]) {
			if (speakeramplitude < 15000) {
				speakeramplitude += PC_SPEAKER_MOVEMENT;
			}
		}
		else {
			if (speakeramplitude > 0) {
				speakeramplitude -= PC_SPEAKER_MOVEMENT;
			}
		}
		//speakeramplitude = 0;
	}
	else {
		if (speakergate[PC_SPEAKER_GATE_DIRECT]) {
			if (speakeramplitude < 15000) {
				speakeramplitude += PC_SPEAKER_MOVEMENT;
			}
		}
		else {
			if (speakeramplitude > 0) {
				speakeramplitude -= PC_SPEAKER_MOVEMENT;
			}
		}
	}
	if (speakeramplitude > 15000) speakeramplitude = 15000;
	if (speakeramplitude < 0) speakeramplitude = 0;
}

void PCSpeaker::init() {
	speakergateselect = PC_SPEAKER_GATE_DIRECT;
	//timing_addTimer(tick, spk, SAMPLE_RATE, TIMING_ENABLED, "[SPEAKER] callback");
}

int16_t PCSpeaker::getSample() {
	//return 0;
	return speakeramplitude;
}
