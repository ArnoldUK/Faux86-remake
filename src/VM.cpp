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
#include "DriveManager.h"
#include "Debugger.h"

using namespace Faux86;

extern uint8_t renderbenchmark;

extern void VideoThread();

#ifdef CPU_ADDR_MODE_CACHE
extern uint64_t cached_access_count, uncached_access_count;
#endif

uint64_t starttick, endtick;

uint8_t cgaonly = 0;
uint8_t use_debug_console = 0;

extern void isa_ne2000_init (uint16_t baseport, uint8_t irq);

#ifdef NETWORKING_ENABLED
extern void initpcap();
extern void dispatch();
#endif

void inithardware() {
#ifdef NETWORKING_ENABLED
	if (ethif != 254) initpcap();
#endif
#ifndef NETWORKING_OLDCARD
	log(Log, "[VM] Novell NE2000 ethernet adapter");
	isa_ne2000_init (0x300, 6);
#endif
}

class MainEmulationTask : public Task
{
	VM& vm;

public:
	MainEmulationTask(VM& inVM) : vm(inVM) {}

	int update() override
	{
		int delay = 0;
		
		//THIS CODE NEEDS TO BE MORE ACCURATE
		//vm.timing.tick();
		
		if (!vm.config.cpuSpeed)	{
			//vm.cpu.exec86(4700); //4.7Mhz
			//vm.cpu.exec86(10000); //10Mhz
			//vm.cpu.exec86(100000); //100Mhz
			vm.cpu.exec86(10000);
		}	else {
			//vm.cpu.exec86(vm.config.cpuSpeed / 100); //100 rpi
			//vm.cpu.exec86(vm.config.cpuSpeed / 1000); //win32 (4700000 = 4.7Mhz)
			#ifdef _WIN32
			vm.cpu.exec86(vm.config.cpuSpeed * 100); //10Mhz win32
			#elif defined(ARDUINO)
			vm.cpu.exec86(vm.config.cpuSpeed * 100); //10Mhz
			#else
			vm.cpu.exec86(vm.config.cpuSpeed * 1000); //10Mhz RPi
			#endif
			if (vm.config.enableAudio) {
				while (!vm.audio.isAudioBufferFilled()) {
					vm.timing.tick();
					vm.audio.tick();
				}
			}
			#ifdef _WIN32
			delay = 1;
			#else
			//delay = 0;	
			#endif
		}
		return delay;
	}
};

#ifdef DEBUG_CONSOLE
#ifdef _WIN32
void runconsole (void *dummy);
#else
void *runconsole (void *dummy);
#endif
#endif

VM::VM(Config& inConfig)
	: config(inConfig)
	, cpu(*this)
	, memory(*this)
	, ports(*this)
	, pic(*this)
	, pit(*this)
	, dma(*this)
	, drives(*this)
	, video(*this)
	, audio(*this)
	, adlib(*this)
	, blaster(*this, adlib)
	, soundSource(*this)
	, pcSpeaker(*this)
	, uartcom1(*this)
	, mouse(*this)
	, renderer(*this)
	, input(*this)
	, timing(*this)
	, taskManager(*this)
	, running(true)
{
	log(Log, "[VM] Constructed");
}

bool VM::init()
{
	log(Log, "[VM] Initialized");

	if (config.enableDebugger) debugger = new Debugger(*this);
	
	//config.hostSystemInterface.getFrameBuffer();
	renderer.init(config.framebuffer.width, config.framebuffer.height);
	audio.init();

	if (config.usePCSpeaker) pcSpeaker.init();
	if (config.useAdlib) adlib.init();
	if (config.useSoundBlaster) blaster.init();	
	if (config.useDisneySoundSource) soundSource.init();
	
	dma.init();

	uartcom1.init(config.mouse.port, config.mouse.irq, nullptr, nullptr, nullptr, nullptr);
	mouse.init();

	timing.init();

	if (!config.biosFile || !config.biosFile->isValid())
	{
		log(LogFatal, "[VM] Failed to load Machine BIOS file!");
		return false;
	}

	if (!config.asciiFile || !config.asciiFile->isValid())
	{
		log(LogFatal, "[VM] Failed to load Video ASCII Char file!");
		return false;
	}

	uint32_t biosSize = (uint32_t) config.biosFile->getSize();
	memory.loadBinary((uint32_t)(DEFAULT_RAM_SIZE - biosSize), config.biosFile, 1, MemArea_BIOS);

	//memory.loadBinary(0xA0000UL, config.asciiFile, 1);

#ifdef DISK_CONTROLLER_ATA
	if (!memory.loadBinary(0xD0000UL, config.ideRomFile, 1))
	{
		log(LogFatal, "[VM] Failed to load Disk Controller ROM file!");
		return false;
	}
#endif
	if (biosSize <= 8192) 
	{
		log(Log, "[VM] Machine ROM biosSize <= 8192 Loading Basic Boot ROM");
		if (!memory.loadBinary(0xF6000UL, config.romBasicFile, 0)) {
			log(Log, "[VM] Failed to load Basic Boot ROM file!");
		}
		if (!memory.loadBinary(0xC0000UL, config.videoRomFile, 1, MemArea_VGABIOS))
		{
			log(LogFatal, "[VM] Failed to load Video ROM file!");
			return false;
		}
	}

	if (debugger)
	{
		debugger->flagRegion(0x400, 256, MemArea_BDA);
		debugger->flagRegion(0, 0x3FF, MemArea_InterruptTable);
		//debugger->addDataBreakpoint(0x487);
	}
	
	video.init();

	drives.insertDisk(DRIVE_A, config.diskDriveA);
	drives.insertDisk(DRIVE_B, config.diskDriveB);
	drives.insertDisk(DRIVE_C, config.diskDriveC);
	drives.insertDisk(DRIVE_D, config.diskDriveD);

	if (config.bootDrive == 254)
	{
		if (drives.isDiskInserted(DRIVE_C))
			config.bootDrive = DRIVE_C;
		else if (drives.isDiskInserted(DRIVE_A))
			config.bootDrive = DRIVE_A;
		else
			config.bootDrive = 0xFF; //ROM BASIC fallback
	}

	log(Log, "[VM] Resetting CPU..");
	running = true;
	cpu.reset86();

	inithardware();

	#ifdef DEBUG_CONSOLE
	log(Log, "[VM] Debug Console Enabled");
	if (use_debug_console) {
	#ifdef _WIN32
		_beginthread(runconsole, 0, (void*) this);
	#else
		pthread_create(&consolethread, NULL, (void *)runconsole, (void*) this);
	#endif
	}
	#endif

	taskManager.addTask(new MainEmulationTask(*this));
	
	return true;
}

VM::~VM()
{
}

bool VM::simulate()
{
	input.tick();

	#ifdef NETWORKING_ENABLED
	if (ethif < 254) dispatch();
	#endif

	taskManager.tick();

	return running;
}

