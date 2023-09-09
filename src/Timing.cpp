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
#include "Timing.h"

using namespace Faux86;

bool hblankenabled = true;
uint64_t lasthblank, lasthblankend = 0;

TimingScheduler::TimingScheduler(VM& inVM)
	: vm(inVM)
{
	log(Log,"[TIMING] Constructed");
}


void TimingScheduler::init()
{
	log(Log,"[TIMING] Initialized");
	
	curtick = getTicks();

	lastvgacursortick = lastdrawtick = lasti8253tick = lastmousetick = lastblastertick = lastadlibtick = lastssourcetick = lastsampletick = lastscanlinetick = lasttick = curtick;
	scanlinetiming = getHostFreq() / 31500; //0.317ms 317 31.74
	ssourceticks = getHostFreq() / 8000; //0.125ms 1250 125
	adlibticks = getHostFreq() / vm.config.audio.sampleRate; //0.208ms 48000; //20
	speakerticks = getHostFreq() / vm.config.audio.sampleRate;
	//mouseticks = getHostFreq() / (115200 / 9); //0.078ms
	//mouseticks = getHostFreq() / (115200 / 6); //0.043ms
	//mouseticks = getHostFreq() / (115200 / 20); //1.73ms
	mouseticks = getHostFreq() / (115200 / 10);
	if (vm.config.enableAudio) sampleticks = getHostFreq() / vm.config.audio.sampleRate;
	else sampleticks = -1;
	
	//i8253tickgap = getHostFreq() / 119318; //0.0083ms 83 8
	i8253tickgap = getHostFreq() / 1193182; //0.0083ms 83 8
	
	//drawticks = getHostFreq() / 50; //20ms 200000 20000
	//drawticks = getHostFreq() / 30; //33ms
	//drawticks = getHostFreq() / 100; //10ms
	drawticks = getHostFreq() / (1000 / vm.config.frameDelay);
	vgacursorticks = getHostFreq() / 3.75; //266ms 2666666 266666
	vgahblanktiming = scanlinetiming;
	vgahblankendtiming = getHostFreq() / 9;
	lasthblank = lasthblankend = curtick;
}

void TimingScheduler::tick() 
{
	uint8_t i8253chan;

	curtick = getTicks();
	
	if (curtick >= (lastscanlinetick + scanlinetiming) ) {
	//if (curtick >= (lastscanlinetick + vm.video.vga_dispinterval) ) {
		//vm.video.vga_hblankCallback(0);
		//vm.video.vga_hblankEndCallback(0);
		curscanline = (curscanline + 1) % 525;
		if (curscanline > 479) {
			//log(Log,"[TIMING] VGA scanlinetiming %llu %llu", curscanline, vm.video.vga_dispinterval);
			vm.video.port3da = 8;
			//vm.video.vga_hblankEndCallback(0);
			//vm.video.vga_hblankCallback(0);
		}	else {
			vm.video.port3da = 0;
			//vm.video.vga_hblankCallback(0);
			//vm.video.vga_hblankEndCallback(0);
		}
		if (curscanline & 1) {
			vm.video.port3da |= 1;
			//vm.video.vga_hblankEndCallback(0);
		}
		//pit0counter++;
		//vm.video.vga_updateScanlineTiming();
		lastscanlinetick = curtick - (curtick - (lastscanlinetick + scanlinetiming) );
	}
	
	
	if (vm.pit.active[0]) { //timer interrupt channel on i8253
		if (curtick >= (lasttick + tickgap) ) {
			lasttick = curtick;
			vm.pic.doirq(0);
		}
	}

	if (curtick >= (lasti8253tick + i8253tickgap) ) {
		for (i8253chan = 0; i8253chan < 3; i8253chan++) {
			if (vm.pit.active[i8253chan]) {
				if (vm.pit.counter[i8253chan] < 10) vm.pit.counter[i8253chan] = vm.pit.chandata[i8253chan];
				vm.pit.counter[i8253chan] -= 10; //10
				//vm.pit.tick();
			}
		}
		lasti8253tick = curtick;
	}
		
	/*	
	if (curtick >= (lasthblank + vgahblanktiming) ) {
		hblankenabled = true;
		vm.video.vga_hblankCallback(0);
		//lasthblank = curtick;
		lasthblank = curtick - (curtick - (lasthblank + vgahblanktiming) );
	}
	
	if (hblankenabled) {
		if (curtick >= (lasthblankend + vgahblankendtiming) ) {
			hblankenabled = false;
			vm.video.vga_hblankEndCallback(0);
			//lasthblankend = curtick;
			lasthblankend = curtick - (curtick - (lasthblankend + vgahblankendtiming) );
		}
	}
	*/

	if (curtick >= (lastmousetick + mouseticks) ) {
		//if (vm.config.useMouse) vm.mouse.rxpoll(0);
		vm.mouse.rxpoll(0);
		lastmousetick = curtick - (curtick - (lastmousetick + mouseticks) );
	}
	
	if (vm.config.enableAudio) {
		/*
		if (vm.config.usePCSpeaker) {
		if (curtick >= (lastspeakertick + speakerticks) ) {
				vm.pit.tick();
				vm.pcSpeaker.tick();
				lastspeakertick = curtick - (curtick - (lastspeakertick + speakerticks) );
			}
		}
		*/
		
		if (vm.config.useDisneySoundSource) {
		if (curtick >= (lastssourcetick + ssourceticks) ) {
				vm.soundSource.tick();
				lastssourcetick = curtick - (curtick - (lastssourcetick + ssourceticks) );
			}
		}

		if (vm.config.useSoundBlaster) {
		if (vm.blaster.samplerate > 0) {
			if (curtick >= (lastblastertick + vm.blaster.sampleticks) ) {
				vm.blaster.tick();
				lastblastertick = curtick - (curtick - (lastblastertick + vm.blaster.sampleticks) );
			}
			}
		}
		
		if (vm.config.useAdlib) {
		if (curtick >= (lastadlibtick + adlibticks) ) {
				vm.adlib.tick();
				lastadlibtick = curtick - (curtick - (lastadlibtick + adlibticks) );
			}
		}

		if (curtick >= (lastsampletick + sampleticks) ) {
			vm.audio.tick();
			if (vm.config.slowSystem) {
				vm.audio.tick();
				vm.audio.tick();
				//vm.audio.tick();
				}
			lastsampletick = curtick - (curtick - (lastsampletick + sampleticks) );
		}
	}
	
	if (curtick >= (lastvgacursortick + vgacursorticks)) {
		//log(Log,"[TIMING] VGA Cursor Ticks %llu %llu", lastvgacursortick, getElapsedMS(lastvgacursortick));
		vm.video.vga_blinkCallback(0);
		lastvgacursortick = curtick - (curtick - (lastvgacursortick + vgacursorticks));
	}

	if (curtick >= (lastdrawtick + drawticks)) {
		//log(Log,"[TIMING] VGA Draw Ticks %llu %llu", lastdrawtick, getElapsedMS(lastdrawtick));
		vm.video.vga_drawCallback(0);
		lastdrawtick = curtick - (curtick - (lastdrawtick + drawticks) );
	}
}

uint64_t TimingScheduler::getTicks()
{
	return vm.config.hostSystemInterface->getTimer().getTicks();
}

uint64_t TimingScheduler::getHostFreq()
{
	return vm.config.hostSystemInterface->getTimer().getHostFreq();
}

uint64_t TimingScheduler::getElapsed(uint64_t prevTick)
{
	return getTicks() - prevTick;
}

uint64_t TimingScheduler::getElapsedMS(uint64_t prevTick)
{
	return getElapsed(prevTick) * 1000 / getHostFreq();
}

uint64_t TimingScheduler::getMS()
{
	return (getTicks() * 1000) / getHostFreq();
}
