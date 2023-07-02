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

#include "Config.h"
#include "DriveManager.h"
#include "VM.h"

using namespace Faux86;

#ifndef PATH_DATAFILES
#define PATH_DATAFILES ""
#endif

#ifndef _WIN32
#define strcmpi strcasecmp
#else
#define strcmpi _strcmpi
#endif

#ifdef _WIN32
#define COMMAND_LINE_PARSING
#endif

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
	
	printf("Faux86 requires parameters to run.\nValid parameters:\n"
		"  -fd0 filename    Specify a floppy disk image file to use as floppy 0.\n"
		"  -fd1 filename    Specify a floppy disk image file to use as floppy 1.\n"
		"  -hd0 filename    Specify a hard disk image file to use as hard drive 0.\n"
		"  -hd1 filename    Specify a hard disk image file to use as hard drive 1.\n"
		"										Disk image formats supported are .img and .raw\n" 
		"  -boot #          Specify which BIOS drive ID boot device to use.\n"
		"                   Examples: -boot 0 will boot from floppy 0 (A:).\n"
		"                             -boot 1 will boot from floppy 1 (B:).\n"
		"                             -boot 128 will boot from hard drive 0 (C:).\n"
		"                             -boot 255 will boot from hard drive 1 (D:).\n"		
		"                             -boot rom will boot to ROM BASIC if available.\n"
		"                   Default boot device is hard drive 0, if it exists.\n"
		"                   Otherwise, the default is floppy 0.\n"
		"  -bios filename   Specify an alternate Machine BIOS ROM image.\n"
		#ifdef NETWORKING_ENABLED
		#ifdef _WIN32
		"  -net #           Enable ethernet emulation via winpcap, where # is the\n"
		#else
		"  -net #           Enable ethernet emulation via libpcap, where # is the\n"
		#endif
		"                   numeric ID of your host's network interface to bridge.\n"
		"                   To get a list of possible interfaces, use -net list\n"
		#endif
		"  -nosound         Disable audio emulation and output.\n"
		"  -fullscreen      Start Faux86 in fullscreen mode.\n"
		"  -verbose         Verbose mode. Operation details will be written to stdout.\n"
		"  -speed           Frequency of the CPU in Mhz. Set to 0 for Maximum Speed.\n"
		"                   Value between 1Mhz and 50Mhz. Default is 10 (10Mhz).\n"		
		"  -delay           Specify how many milliseconds to render each video frame.\n"
		"                   Value between 1ms and 1000ms. Default is 20ms (50 FPS).\n"
		"  -slowsys         If your machine is very slow and you have audio dropouts,\n"
		"                   use this option to sacrifice audio quality to compensate.\n"
		"                   If you still have dropouts, then also decrease sample rate\n"
		"                   and/or increase latency.\n"
		"  -multithreaded # Enable multithread processing.\n"
		"  -resw #				  Set constant SDL window size width in pixels.\n"
		"  -resh #				  Set constant SDL window size height in pixels.\n"
		"										Default width and height is 0 and set automatically\n"
		"  -render #				Set render scaling quality mode for SDL window renderer.\n"
		"										0 = nearest (fastest low quality).\n"
		"										1 = linear (quick good quality).\n"
		"										2 = best (slow best quality) (default).\n"
		"  -monitor #				Set monitor display type to emulate.\n"
		"										0 = Color VGA (default).\n"
		"										1 = Amber Gas Plasma.\n"
		"										2 = Green CRT Monochrome.\n"
		"										3 = Blue LCD.\n"
//		"  -smooth          Apply smoothing to screen rendering.\n"
//		"  -noscale         Disable 2x scaling of low resolution video modes.\n"
		"  -sndsource       Enable Disney Sound Source emulation on LPT1.\n"
		"  -latency #       Change audio buffering and output latency. (default: 100 ms)\n"
		"  -samprate #      Change audio emulation sample rate. (default: 48000 Hz)\n"
		"  -console         Enable debug console on stdio during emulation.\n"
		"  -menu						Enable window menu for changing emulation settings.\n"
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

	exit (0);
}
#endif

void Config::parseCommandLine(int argc, char *argv[]) 
{
	#ifdef DEBUG_CONFIG
	log(Log, "[CONFIG::parseCommandLine] argc %d", argc);
	#endif
				
	#ifdef COMMAND_LINE_PARSING
	int i, abort = 0;
	const char* biosFilePath = PATH_DATAFILES "pcxtbios.bin";

	uint8_t ethif;	// TODO: FIX

	if (argc < 2) {
		printf ("Start Faux86 with parameters. Use -h for help and supported parameters.\n");
		#ifndef _WIN32
		exit (0);
		#endif
	}	

	bootDrive = 254;
	ethif = 254;
	for (i=1; i<argc; i++) {
    if (strcmpi (argv[i], "-h") == 0) showhelp ();
    else if (strcmpi (argv[i], "-?") == 0) showhelp ();
    else if (strcmpi (argv[i], "-help") == 0) showhelp ();
		else if (strcmpi (argv[i], "-fd0") == 0) {
			i++;
			diskDriveA = hostSystemInterface->openFile(argv[i]);
		}
		else if (strcmpi (argv[i], "-fd1") == 0) {
			i++;
			diskDriveB = hostSystemInterface->openFile(argv[i]);
		}
		else if (strcmpi (argv[i], "-hd0") == 0) {
			i++;
			diskDriveC = hostSystemInterface->openFile(argv[i]);
		}
		else if (strcmpi (argv[i], "-hd1") == 0) {
			i++;
			diskDriveD = hostSystemInterface->openFile(argv[i]);
		}
		else if (strcmpi (argv[i], "-net") == 0) {
			i++;
			if (strcmpi (argv[i], "list") == 0) ethif = 255;
			else ethif = atoi (argv[i]);
		}
		else if (strcmpi (argv[i], "-boot") == 0) {
			i++;
			if (strcmpi (argv[i], "rom") == 0) bootDrive = 255;
			else bootDrive = atoi (argv[i]);
		}
		else if (strcmpi (argv[i], "-sndsource") == 0) {
			i++;
			useDisneySoundSource = true;
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
		else if (strcmpi (argv[i], "-bios") == 0) {
			i++;
			biosFilePath = argv[i];
		}
		else if (strcmpi (argv[i], "-videorom") == 0) {
			i++;
			videoRomFile = hostSystemInterface->openFile(argv[i]);
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
		else if (strcmpi (argv[i], "-speed") == 0) {
			i++;
			cpuSpeed = (uint32_t) atol (argv[i]);
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
		else if (strcmpi (argv[i], "-slowsys") == 0) slowSystem = 1;
		else if (strcmpi (argv[i], "-oprom") == 0) {
			// TODO
			//i++;
			//tempuint = hextouint (argv[i++]);
			//vm.memory.loadBinary(tempuint, new ImagedDisk(argv[i]), 0);
		}
		else {
			printf("Unrecognized parameter: %s\n", argv[i]);
			exit (1);
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
	if (audio.sampleRate < 4000)
		audio.sampleRate = 4000;
	else if (audio.sampleRate > 96000)
		audio.sampleRate = 96000;
	if (audio.latency < 10)
		audio.latency = 10;
	else if (audio.latency > 1000)
		audio.latency = 1000;
	if (resw > 1024) resw = 640;
	if (resh > 1024) resh = 350;
	if (renderQuality > 2) renderQuality = 2;
	if (monitorDisplay > 3) monitorDisplay = 0;

#endif
}


