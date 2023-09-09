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

#define BUILD_STRING "Faux86-remake v1.21"

#define DEBUG_CONFIG
#define DEBUG_VM
//#define DEBUG_CPU
//#define DEBUG_RAM
//#define DEBUG_DMA
//#define DEBUG_PIC
//#define DEBUG_PIT
//#define DEBUG_PORTS
//#define DEBUG_TIMING
//#define DEBUG_DISK
//#define DEBUG_VIDEO
//#define DEBUG_AUDIO
//#define DEBUG_UART
//#define DEBUG_MOUSE
//#define DEBUG_INPUT


// TODO NOT IMPLEMENTED DO NOT USE!!
//#define DEBUG_CONSOLE 1

//be sure to only define ONE of the CPU_* options at any given time, or
//you will likely get some unexpected/bad results!
//#define CPU_8086
//#define CPU_186
#define CPU_V20
//#define CPU_286
//#define CPU_386

#if defined(CPU_8086)
	#define CPU_CLEAR_ZF_ON_MUL
	#define CPU_ALLOW_POP_CS
#else
	#define CPU_ALLOW_ILLEGAL_OP_EXCEPTION
	#define CPU_LIMIT_SHIFT_COUNT
#endif

#if defined(CPU_V20)
	#define CPU_NO_SALC
#endif

#if defined(CPU_286) || defined(CPU_386)
	#define CPU_286_STYLE_PUSH_SP
#else
	#define CPU_SET_HIGH_FLAGS
#endif

#define TIMING_INTERVAL 15
//#define TIMING_INTERVAL 31

//when USE_PREFETCH_QUEUE is defined, Faux86's CPU emulator uses a 6-byte
//read-ahead cache for opcode fetches just as a real 8086/8088 does.
//by default, i just leave this disabled because it wastes a very very
//small amount of CPU power. however, for the sake of more accurate
//emulation, it can be enabled by uncommenting the line below and recompiling.
//#define USE_PREFETCH_QUEUE

//#define CPU_ADDR_MODE_CACHE

//when compiled with network support, faux86 needs libpcap/winpcap.
//if it is disabled, the ethernet card is still emulated, but no actual
//communication is possible -- as if the ethernet cable was unplugged.
#define NETWORKING_OLDCARD //planning to support an NE2000 in the future

//when DISK_CONTROLLER_ATA is defined, faux86 will emulate a true IDE/ATA1 controller
//card. if it is disabled, emulated disk access is handled by directly intercepting
//calls to interrupt 13h.
//*WARNING* - the ATA controller is not currently complete. do not use!
//#define DISK_CONTROLLER_ATA

#define AUDIO_DEFAULT_SAMPLE_RATE 48000
//#define AUDIO_DEFAULT_SAMPLE_RATE 44100
#define AUDIO_DEFAULT_LATENCY 100
//#define AUDIO_DEFAULT_LATENCY 120

//#define DOUBLE_BUFFER

//#define VIDEO_FRAMEBUFFER_WIDTH 1024
//#define VIDEO_FRAMEBUFFER_HEIGHT 1024
#define VIDEO_FRAMEBUFFER_WIDTH 800
#define VIDEO_FRAMEBUFFER_HEIGHT 800

#define RENDER_QUALITY_NEAREST	0
#define RENDER_QUALITY_LINEAR		1
#define RENDER_QUALITY_BEST			2

#define MONITOR_DISPLAY_COLOR		0
#define MONITOR_DISPLAY_AMBER		1
#define MONITOR_DISPLAY_GREEN		2
#define MONITOR_DISPLAY_BLUE		3

#define HOST_WINDOW_WIDTH			640
#define HOST_WINDOW_HEIGHT		350

#define DEFAULT_RAM_SIZE 0x100000
//#define DEFAULT_RAM_SIZE 0x200000
						 
//#define DEBUG_BLASTER
//#define DEBUG_DMA

//#define BENCHMARK_BIOS

#ifdef _WIN32
#define FUNC_INLINE __forceinline
//#define FUNC_INLINE static __forceinline
#else
#define FUNC_INLINE __attribute__((always_inline))
//#define FUNC_INLINE static __attribute__((always_inline))
#endif

#ifndef _WIN32
#define _stricmp strcasecmp
#endif

#include "Types.h"
#include "HostSystemInterface.h"

namespace Faux86
{
	enum class CpuType
	{
		Cpu8086,
		Cpu186,
		CpuV20,
		Cpu286,
		Cpu386
	};

	class DiskInterface;

	struct Config
	{
		Config(HostSystemInterface* inHostInterface) : hostSystemInterface(inHostInterface) {}

		bool parseCommandLine(int argc, char *argv[]);
		bool loadFD0(const char* str);
		bool loadFD1(const char* str);
		bool loadHD0(const char* str);
		bool loadHD1(const char* str);
		bool loadBiosRom(const char* str);
		bool loadVideoRom(const char* str);
		bool loadBootRom(const char* str);
		bool loadCharRom(const char* str);
		bool setBootDrive(const char* str);

		HostSystemInterface* hostSystemInterface;

		uint32_t ramSize = DEFAULT_RAM_SIZE;
		//CpuType cpuType = CpuType::Cpu286;
		//CpuType cpuType = CpuType::Cpu186;
		CpuType cpuType = CpuType::CpuV20;

		DiskInterface* biosFile = nullptr;
		DiskInterface* ideRomFile = nullptr;
		DiskInterface* romBasicFile = nullptr;
		DiskInterface* videoRomFile = nullptr;
		DiskInterface* asciiFile = nullptr;

		DiskInterface* diskDriveA = nullptr;
		DiskInterface* diskDriveB = nullptr;
		DiskInterface* diskDriveC = nullptr;
		DiskInterface* diskDriveD = nullptr;

		uint8_t bootDrive = 0;
		
		//COM PORT AND IRQ
		//COM1 - 3F8h IRQ4
		//COM2 - 2F8h IRQ3
		//COM3 - 3E8h IRQ4
		//COM4 - 2E8h IRQ3
		uint8_t mousePort = 2;
		struct  
		{
			uint16_t port = 0x2F8;
			uint8_t irq = 3;			
		} mouse;

		//SOUNDBLASTER PORT AND IRQ
		struct  
		{
			uint16_t port = 0x220;
			uint8_t irq = 7;
			uint8_t dma = 1;
			uint8_t model = 2; //Sound Blaster 2.0
			//uint8_t model = 3; //Sound Blaster Pro
		} blaster;

		//ADLIB PORT
		struct  
		{
			uint16_t port = 0x388;
		} adlib; 

		bool useDisneySoundSource = false;
		bool useSoundBlaster = true;
		bool useAdlib = true;
		bool usePCSpeaker = true;

		struct 
		{
			int32_t sampleRate = AUDIO_DEFAULT_SAMPLE_RATE;
			int32_t latency = AUDIO_DEFAULT_LATENCY;
		} audio;
		
		struct
		{
			uint32_t width = VIDEO_FRAMEBUFFER_WIDTH;
			uint32_t height = VIDEO_FRAMEBUFFER_HEIGHT;
		} framebuffer;
		
		bool verbose = false;
		bool useFullScreen = false; //false;
		bool renderBenchmark = false;
		bool noSmooth = true; //true
		bool noScale = true; //false;
		bool enableAudio = true;
		bool enableConsole = true;
		bool enableMenu = false;
		bool singleThreaded = true; //false
		bool slowSystem = false;
		bool enableDebugger = false;
		
		uint16_t resw = HOST_WINDOW_WIDTH;
		uint16_t resh = HOST_WINDOW_HEIGHT;
		
		uint8_t renderQuality = RENDER_QUALITY_BEST;
		uint8_t monitorDisplay = MONITOR_DISPLAY_COLOR;
		//uint32_t cpuSpeed = 0;
		//uint32_t cpuSpeed = 20; //20000000; //20Mhz
		uint32_t cpuSpeed = 20; //10000000; //10Mhz
		uint32_t frameDelay = 20; //20ms per frame rendered (50fps)
		//uint32_t frameDelay = 60;
	};
}
