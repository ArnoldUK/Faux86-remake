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
#include <strings.h>
#elif defined(ARDUINO)
#include <strings.h>
#else
#include <circle/util.h>
#endif

#include "Config.h"
#include "DriveManager.h"
//#include "VM.h"

using namespace Faux86;

#ifdef COMMAND_LINE_PARSING
#include <stdio.h>

uint32_t hextouint(char *src) {
	uint32_t tempuint = 0, cc;
	uint16_t i;

	for (i=0; i<strlen(src); i++) {
		cc = src[i];
		if (cc == 0) break;
		if ((cc >= 'a') && (cc <= 'F')) cc = cc - 'a' + 10;
			else if ((cc >= 'A') && (cc <= 'F')) cc =  cc - 'A' + 10;
			else if ((cc >= '0') && (cc <= '9')) cc = cc - '0';
			else return(0);
		tempuint <<= 4;
		tempuint |= cc;
	}
	return(tempuint);
}

void showhelp () {
	printf("Faux86 Remake (c)2018 James Howard\n");
	printf("Based on Fake86 (c)2010-2013 Mike Chambers\n");
	printf("Contributions and Updates (c)2023 Curtis aka ArnoldUK\n");
	printf("Portable, open-source 8086 XT Emulator\n");
	printf("\n");
	printf("Start Faux86 with parameters. Use -h for help on supported parameters.\n");
	printf("Start Faux86 without parameters. Default settings will be loaded from faux86.cfg.\n");
	printf("Supported command lines parameters are:\n"
		"  -fd0 filename    Specify a floppy disk image file to use as floppy 0.\n"
		"  -fd1 filename    Specify a floppy disk image file to use as floppy 1.\n"
		"  -hd0 filename    Specify a hard disk image file to use as hard drive 0.\n"
		"  -hd1 filename    Specify a hard disk image file to use as hard drive 1.\n"
		"                   Disk image formats supported are .img and .raw\n" 
		"  -boot #          Specify which disk boot device to use. Examples:\n"
		"                   -boot fd0 will boot from floppy 0 (A:).\n"
		"                   -boot fd1 will boot from floppy 1 (B:).\n"
		"                   -boot hd0 will boot from hard drive 0 (C:).\n"
		"                   -boot hd1 will boot from hard drive 1 (D:).\n"		
		"                   -boot rom will boot to ROM BASIC if available.\n"
		"                    Default boot device is fd0 or hd0 if exists.\n"
		"  -biosrom file    Specify an alternate Machine BIOS ROM file image.\n"
		"  -videorom file   Specify an alternate Video ROM file image.\n"
		"  -bootrom file    Specify an alternate Boot ROM file image.\n"
		"  -charrom file    Specify an alternate ASCII Char ROM file image.\n"
		#ifdef NETWORKING_ENABLED
		#ifdef _WIN32
		"  -net #           Enable ethernet emulation via winpcap, where # is the\n"
		#else
		"  -net #           Enable ethernet emulation via libpcap, where # is the\n"
		#endif
		"                   numeric ID of your host's network interface to bridge.\n"
		"                   To get a list of possible interfaces, use -net list\n"
		#endif
		"  -nosound         Disable all audio emulation and sound output.\n"
		"  -fullscreen      Start Faux86 in fullscreen mode.\n"
		"  -verbose         Verbose mode. Operation details will be written to stdout.\n"
		"  -cpu #           Set the CPU type and opcode emulation. Default is 2 for NEC V20 CPU.\n"	
		"                   0 = 8086/8088 CPU_TYPE_8086.\n"
		"                   1 = 80186 CPU_TYPE_186.\n"
		"                   2 = NEC V20 CPU_TYPE_V20.\n"
		"                   3 = 286 CPU_TYPE_286 LIMITED OPCODES.\n"
		"                   4 = 386 CPU_TYPE_386 NOT SUPPORTED.\n"	
		"  -speed #         Frequency of the CPU in Mhz. Set to 0 for Maximum Speed.\n"
		"  -timing #        Set number CPU clock timing calls to update display,input,audio.\n"		
		"                   Value should be either 1,3,7,15,31,63,127,255 depending on system.\n"
		"                   Lower values make more CPU clock timing calls. Default is 15.\n"		
		"  -delay #         Specify how many milliseconds to render each video frame.\n"
		"                   Value between 1ms and 1000ms. Default is 20ms (50 FPS).\n"
		"  -slowsys         If your machine is very slow and you have audio dropouts,\n"
		"                   use this option to sacrifice audio quality to compensate.\n"
		"                   If you still have dropouts, then also decrease sample rate\n"
		"                   and/or increase latency.\n"
		"  -multithreaded # Enable multithread processing.\n"
		"  -resw #          Set constant SDL window size width in pixels.\n"
		"  -resh #          Set constant SDL window size height in pixels.\n"
		"                   Default width and height is 0 and set automatically\n"
		"  -render #        Set render scaling quality mode for SDL window renderer.\n"
		"                   0 = nearest (fastest low quality).\n"
		"                   1 = linear (quick good quality).\n"
		"                   2 = best (slow best quality) (default).\n"
		"  -monitor #       Set monitor display type to emulate.\n"
		"                   0 = Color VGA (default).\n"
		"                   1 = Amber Gas Plasma.\n"
		"                   2 = Green CRT Monochrome.\n"
		"                   3 = Blue LCD.\n"
		"                   4 = Toshiba Blue LCD.\n"
		"                   5 = Teal Terminal.\n"
		"                   6 = Green Terminal.\n"
		"                   7 = Amber Terminal.\n"								
//		"  -smooth          Apply smoothing to screen rendering.\n"
//		"  -noscale         Disable 2x scaling of low resolution video modes.\n"
		"  -mouseport #     Serial Mouse COM Port #.\n"
		"                   1 = COM1 IO:3F8H IRQ:4\n"
		"                   2 = COM2 IO:2F8H IRQ:3 (default).\n"
		"                   3 = COM3 IO:3E8H IRQ:4\n"
		"                   4 = COM4 IO:2E8H IRQ:3\n"		
		"  -snddisney       Enable Disney Sound Source emulation on LPT1.\n"
		"  -sndblaster      Enable SoundBlaster emulation (requires Adlib to be enabled).\n"
		"  -sndadlib        Enable Adlib emulation (required for SoundBlaster emulation).\n"
		"  -sndopl3         Enable OPL3 emulation (default OPL2 requires Adlib enabled.\n"	
		"  -sndspeaker      Enable PC Speaker emulation.\n"
		"  -latency #       Change audio buffering and output latency. (default: 100 ms)\n"
		"  -samprate #      Change audio emulation sample rate. (default: 48000 Hz)\n"
		"  -console         Enable debug console on stdio during emulation.\n"
		"  -menu            Enable window menu for changing emulation settings.\n"
//		"  -oprom addr rom  Inject a custom option ROM binary at an address in hex.\n"
//		"                   Example: -oprom F4000 monitor.bin\n"
//		"                   This loads the data from monitor.bin at 0xF4000.\n"

		"\nThis program is free software; you can redistribute it and/or\n"
		"modify it under the terms of the GNU General Public License\n"
		"as published by the Free Software Foundation; either version 2\n"
		"of the License, or (at your option) any later version.\n\n"

		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n");

	exit(0);
}
#endif

/*
Config::Config(HostSystemInterface* inHostInterface)
: hostSystemInterface(inHostInterface)
{
}
*/

bool Config::parseCommandLine(int argc, char *argv[]) 
{
	#ifdef COMMAND_LINE_PARSING
	
	#ifdef DEBUG_CONFIG
	log(Log, "[CONFIG::parseCommandLine] argc %d", argc);
	#endif
	
	int i, abort = 0;
	//const char* biosFilePath = PATH_DATAFILES "pcxtbios.bin";

	uint8_t ethif;	// TODO: FIX

	if (argc < 2) {
		//printf ("Start Faux86 with parameters. Use -h for help on supported parameters.\n");
		//printf ("Start Faux86 without parameters. Default settings will be loaded from faux86.cfg.\n");
		//exit(0);
		return false;
	}	

	bootDrive = 0;
	ethif = 254;
	for (i=1; i<argc; i++) {
    if (strcmpi (argv[i], "-h") == 0) showhelp();
    if (strcmpi (argv[i], "-?") == 0) showhelp();
    if (strcmpi (argv[i], "-help") == 0) showhelp();
		
		if (strcmpi (argv[i], "-fd0") == 0) {
			i++;
			loadFD0(argv[i]);
			//diskDriveA = hostSystemInterface->openFile(argv[i]);
		}
		else if (strcmpi (argv[i], "-fd1") == 0) {
			i++;
			loadFD1(argv[i]);
			//diskDriveB = hostSystemInterface->openFile(argv[i]);
		}
		else if (strcmpi (argv[i], "-hd0") == 0) {
			i++;
			loadHD0(argv[i]);
			//diskDriveC = hostSystemInterface->openFile(argv[i]);
		}
		else if (strcmpi (argv[i], "-hd1") == 0) {
			i++;
			loadHD1(argv[i]);
			//diskDriveD = hostSystemInterface->openFile(argv[i]);
		}
		else if (strcmpi (argv[i], "-net") == 0) {
			i++;
			if (strcmpi (argv[i], "list") == 0) ethif = 255;
			else ethif = atoi (argv[i]);
		}
		else if (strcmpi (argv[i], "-boot") == 0) {
			i++;
			setBootDrive(argv[i]);
			/*
			if (strcmpi (argv[i], "fd0") == 0) bootDrive = 0;
			else if (strcmpi (argv[i], "fd1") == 0) bootDrive = 1;
			else if (strcmpi (argv[i], "hd0") == 0) bootDrive = 128;
			else if (strcmpi (argv[i], "hd1") == 0) bootDrive = 254;
			else if (strcmpi (argv[i], "rom") == 0) bootDrive = 255;
			else bootDrive = atoi(argv[i]);
			*/
		}
		else if (strcmpi (argv[i], "-mouseport") == 0) {
			i++;
			mousePort = atoi(argv[i]);
		}
		else if (strcmpi (argv[i], "-snddisney") == 0) {
			i++;
			useDisneySoundSource = true;
		}
		else if (strcmpi (argv[i], "-sndblaster") == 0) {
			i++;
			useSoundBlaster = true;
		}
		else if (strcmpi (argv[i], "-sndadlib") == 0) {
			i++;
			useAdlib = true;
		}
		else if (strcmpi (argv[i], "-sndopl3") == 0) {
			i++;
			useOPL3 = true;
		}
		else if (strcmpi (argv[i], "-sndspeaker") == 0) {
			i++;
			usePCSpeaker = true;
		}
		else if (strcmpi (argv[i], "-latency") == 0) {
			i++;
			audio.latency = atol (argv[i]);
		}
		else if (strcmpi (argv[i], "-samprate") == 0) {
			i++;
			audio.sampleRate = atol (argv[i]);
		}
		else if (strcmpi (argv[i], "-multithreaded") == 0) {
			i++;
			singleThreaded = false;
		}
		else if (strcmpi (argv[i], "-biosrom") == 0) {
			i++;
			loadBiosRom(argv[i]);
			//biosFile = hostSystemInterface->openFile(argv[i]);
		}
		else if (strcmpi (argv[i], "-videorom") == 0) {
			i++;
			loadVideoRom(argv[i]);
			//videoRomFile = hostSystemInterface->openFile(argv[i]);
		}
		else if (strcmpi (argv[i], "-bootrom") == 0) {
			i++;
			loadBootRom(argv[i]);
			//romBasicFile = hostSystemInterface->openFile(argv[i]);
		}
		else if (strcmpi (argv[i], "-charrom") == 0) {
			i++;
			loadCharRom(argv[i]);
			//asciiFile = hostSystemInterface->openFile(argv[i]);
		}
		else if (strcmpi (argv[i], "-resw") == 0) {
			i++;
			resw = (uint16_t) atoi (argv[i]);
		}
		else if (strcmpi (argv[i], "-resh") == 0) {
			i++;
			resh = (uint16_t) atoi (argv[i]);
		}
		else if (strcmpi (argv[i], "-render") == 0) {
			i++;
			renderQuality = (uint8_t) atoi (argv[i]);
		}
		else if (strcmpi (argv[i], "-monitor") == 0) {
			i++;
			monitorDisplay = (uint8_t) atoi (argv[i]);
		}
		else if (strcmpi (argv[i], "-cpu") == 0) {
			i++;
			cpuType = (uint8_t) atoi (argv[i]);
		}
		else if (strcmpi (argv[i], "-speed") == 0) {
			i++;
			cpuSpeed = (uint32_t) atol (argv[i]);
		}
		else if (strcmpi (argv[i], "-timing") == 0) {
			i++;
			cpuTiming = (uint32_t) atol (argv[i]);
		}
		else if (strcmpi (argv[i], "-debugger") == 0) {
			enableDebugger = true;
		}
		else if (strcmpi (argv[i], "-noscale") == 0) noScale = true;
		else if (strcmpi (argv[i], "-verbose") == 0) verbose = 1;
		else if (strcmpi (argv[i], "-smooth") == 0) noSmooth = false;
		else if (strcmpi (argv[i], "-fps") == 0) renderBenchmark = 1;
		else if (strcmpi (argv[i], "-nosound") == 0) enableAudio = false;
		else if (strcmpi (argv[i], "-fullscreen") == 0) useFullScreen = true;
		else if (strcmpi (argv[i], "-delay") == 0) frameDelay = atol(argv[++i]);
		else if (strcmpi (argv[i], "-console") == 0) enableConsole = true;
		else if (strcmpi (argv[i], "-menu") == 0) enableMenu = true;
		else if (strcmpi (argv[i], "-slowsys") == 0) slowSystem = true;
		else if (strcmpi (argv[i], "-oprom") == 0) {
			// TODO
			//i++;
			//tempuint = hextouint (argv[i++]);
			//vm.memory.loadBinary(tempuint, new ImagedDisk(argv[i]), 0);
		}
		else {
			printf("Unrecognized parameter: %s\n", argv[i]);
			printf("Use -h for a list of supported parameters.\n");
			exit(1);
		}
	}

	if (!biosFile)
	{
		//biosFile = hostSystemInterface->openFile(biosFilePath);
		biosFile = hostSystemInterface->openFile(PATH_DATAFILES "pcxtbios.bin");
	}
	if (!romBasicFile)
	{
		romBasicFile = hostSystemInterface->openFile(PATH_DATAFILES "rombasic.bin");
	}
	if (!videoRomFile)
	{
		videoRomFile = hostSystemInterface->openFile(PATH_DATAFILES "videorom.bin");
	}
	if (!asciiFile)
	{
		asciiFile = hostSystemInterface->openFile(PATH_DATAFILES "asciivga.dat");
	}

	// Clamp values
	if (audio.sampleRate < 22050) audio.sampleRate = AUDIO_DEFAULT_SAMPLE_RATE;
	else if (audio.sampleRate > 48000) audio.sampleRate = AUDIO_DEFAULT_SAMPLE_RATE;
	if (audio.latency < 32)	audio.latency = 32;
	else if (audio.latency > 512) audio.latency = 512;
	
	if (cpuSpeed > 200) cpuSpeed = 200;
	if (cpuTiming < 1) cpuTiming = TIMING_INTERVAL;
	if (cpuTiming > 255) cpuTiming = TIMING_INTERVAL;
	if (cpuType > CPU_TYPE_386) cpuType = CPU_TYPE_V20;
	if (cpuType < CPU_TYPE_8086) cpuType = CPU_TYPE_V20;
	
	if (resw > 1024) resw = HOST_WINDOW_WIDTH;
	if (resh > 1024) resh = HOST_WINDOW_HEIGHT;
	if (renderQuality > 2) renderQuality = RENDER_QUALITY_LINEAR;
	if (monitorDisplay > 7) monitorDisplay = 0;
	//COM PORT AND IRQ
		//COM1 - 3F8h IRQ4
		//COM2 - 2F8h IRQ3
		//COM3 - 3E8h IRQ4
		//COM4 - 2E8h IRQ3
	switch (mousePort) {
		case 1: 
			mouse.port = 0x3F8;
			mouse.irq = 4;
			break;
		case 2:
		default:
			mouse.port = 0x2F8;
			mouse.irq = 3;
			break;
		case 3: 
			mouse.port = 0x3E8;
			mouse.irq = 4;
			break;
		case 4: 
			mouse.port = 0x2E8;
			mouse.irq = 3;
			break;
	}
#endif
	return true;
}

bool Config::loadFD0(const char* str) {
	if ((str == nullptr) || (str[0] == char(0))) return false;
	log(Log, "[CONFIG] loadFD0 %s\n", str);
	diskDriveA = hostSystemInterface->openFile(str);
	if (!diskDriveA) return false;
	return true;
}

bool Config::loadFD1(const char* str) {
	if ((str == nullptr) || (str[0] == char(0))) return false;
	log(Log, "[CONFIG] loadFD1 %s\n", str);
	diskDriveB = hostSystemInterface->openFile(str);
	if (!diskDriveB) return false;
	return true;	
}

bool Config::loadHD0(const char* str) {
	if ((str == nullptr) || (str[0] == char(0))) return false;
	log(Log, "[CONFIG] loadHD0 %s\n", str);
	diskDriveC = hostSystemInterface->openFile(str);
	if (!diskDriveC) return false;
	return true;	
}

bool Config::loadHD1(const char* str) {
	if ((str == nullptr) || (str[0] == char(0))) return false;
	log(Log, "[CONFIG] loadHD1 %s\n", str);
	diskDriveD = hostSystemInterface->openFile(str);
	if (!diskDriveD) return false;
	return true;	
}

bool Config::loadBiosRom(const char* str) {
	if ((str == nullptr) || (str[0] == char(0))) return false;
	log(Log, "[CONFIG] loadBiosRom %s\n", str);
	biosFile = hostSystemInterface->openFile(str);
	if (!biosFile) return false;
	return true;	
}

bool Config::loadVideoRom(const char* str) {
	if ((str == nullptr) || (str[0] == char(0))) return false;
	log(Log, "[CONFIG] loadVideoRom %s\n", str);
	videoRomFile = hostSystemInterface->openFile(str);
	if (!videoRomFile) return false;
	return true;	
}

bool Config::loadBootRom(const char* str) {
	if ((str == nullptr) || (str[0] == char(0))) return false;
	log(Log, "[CONFIG] loadBootRom %s\n", str);
	romBasicFile = hostSystemInterface->openFile(str);
	if (!romBasicFile) return false;
	return true;	
}

bool Config::loadCharRom(const char* str) {
	if ((str == nullptr) || (str[0] == char(0))) return false;
	log(Log, "[CONFIG] loadCharRom %s\n", str);
	asciiFile = hostSystemInterface->openFile(str);
	if (!asciiFile) return false;
	return true;	
}

bool Config::setBootDrive(const char* str) {
	if ((str == nullptr) || (str[0] == char(0))) return false;
	log(Log, "[CONFIG] setBootDrive %s\n", str);
	if (strcmpi(str, "fd0") == 0) bootDrive = 0;
	else if (strcmpi(str, "fd1") == 0) bootDrive = 1;
	else if (strcmpi(str, "hd0") == 0) bootDrive = 128;
	else if (strcmpi(str, "hd1") == 0) bootDrive = 254;
	else if (strcmpi(str, "rom") == 0) bootDrive = 255;
	else bootDrive = atoi(str);
	return true;	
}
