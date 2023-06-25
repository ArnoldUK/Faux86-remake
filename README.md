# Faux86-remake
This is an improved and updated version of Faux86 XT PC Emulator.
A portable, open-source 8086 Emulator for Win32 and bare metal ARM Raspberry Pi.

## Current Status

25-06-2023 Upload of initial build v1.0 for Windows

# Release Notes
Faux86-remake is still work in progress but fixes many issues and adds more features including:
- Updated CPU opcodes with support for more software and games.
- Improved emulation speed.
- Improved video rendering and mode switching.
- Improved audio emulation.
- Improved disk and file access, including writeable disk images.
- Updated PCXT BIOS and Video Roms.
- More configuration parameters.
- Fully working mouse control in both DOS and Windows.
- Many bug fixes.

## Emulator Features
- 8086, V20, 80186 and 286 instruction set.
- Configurable CPU speeds from 5Mhz upto 100Mhz.
- Custom Hardware BIOS's supported.
- Supports bootable disk images in .img and .raw file format.
- CGA / EGA / VGA Colour Video emulation with most modes supported.
- PC Speaker, Adlib, Soundblaster and Disney SoundSource.
- UART Com Ports.
- Standard PC XT Keyboard.
- Serial Port 2-Button mouse.

# Emulator Requirements

## Machine BIOS ROM
The emulator will not boot without BIOS and Video ROM files. Included within the data folder
are several ROM files for various machines and video cards that are compatible with the emulator.
The default BIOS ROM is an open source Turbo XT BIOS v3.1 by Ya`akov Miles & Jon Petrosky.
If the BIOS ROM size is 8KB then an option to boot to the IBM Basic ROM will be presented at boot.
The naming of ROM files should be as follows but this is not a strict requirement for Windows builds.
- pcxtbios.bin (Turbo XT BIOS)
- videorom.bin (VGA Video ROM)
- rombasic.bin (IBM Basic ROM)

## File Disk Images
The emulator also requires bootable disk images in either .img or .raw file format.
There are several utilities available that can create file based disk images but one of the best
so far I have found is [WinImage](https://www.winimage.com/download.htm).
Floppy disk images should be of the size required for the drive type and hard disk images
can be upto 2GB in size. Disk images appear as real drives in the emulator and be formatted
or partitioned from within the emulator.
The naming of disk images should be as follows but this is not a strict requirement for Windows builds.
- fd0.img (Drive A:)
- fd1.img (Drive B:)
- hd0.img (Drive C:)
- hd1.img (Drive D:)

# Operation Instructions

## Usage with Windows
The Windows build is a Win32 execuatble (64-Bit execuatble will also be available).
The emulator must be run with command line parameters to boot a disk image and will default to ROM Basic
by default if no option parameters are supplied for the disk images.
The following parameter options are currently supported:
- -fd0 filename    Specify a floppy disk image file to use as floppy 0.
- -fd1 filename    Specify a floppy disk image file to use as floppy 1.
- -hd0 filename    Specify a hard disk image file to use as hard drive 0.
- -hd1 filename    Specify a hard disk image file to use as hard drive 1.
- Disk image formats supported are .img and .raw
- -boot #          Specify which BIOS drive ID boot device to use.
- Examples: -boot 0 will boot from floppy 0 (A:).
- -boot 1 will boot from floppy 1 (B:).
- -boot 128 will boot from hard drive 0 (C:).
- -boot rom will boot to ROM BASIC if available.
- Default boot device is Harddisk hd0, if it exists, or floppy fd0.
- -bios filename   Specify an alternate Machine BIOS ROM image.
- -net #           Enable ethernet emulation via winpcap. # is ID of your host network interface to bridge.
- -net #           Enable ethernet emulation via libpcap. # is ID of your host network interface to bridge.
- To get a list of possible interfaces, use -net list.
- -nosound         Disable audio emulation and output.
- -fullscreen      Start Faux86 in fullscreen mode.
- -verbose         Verbose mode. Operation details will be written to stdout.
- -speed           Frequency of the CPU in Mhz. Set to 0 for Maximum Speed.
- Value between 1Mhz and 50Mhz. Default is 10 (10Mhz).	
- -delay           Specify how many milliseconds to render each video frame.
- Value between 1ms and 1000ms. Default is 20ms (50 FPS).
- -slowsys         If your machine is very slow and have audio dropouts. Affects audio quality.
- If you still have dropouts, then also decrease sample rate and/or increase latency.
- -multithreaded # Enable multithread processing.
- -resw #				  Set constant SDL window size width in pixels.
- -resh #				  Set constant SDL window size height in pixels.
- Default width and height is 0 and set automatically.
- -render #				Set render scaling quality mode for SDL window renderer.
- 0 = nearest (fastest low quality).
- 1 = linear (quick good quality).
- 2 = best (slow best quality) (default).
- -sndsource       Enable Disney Sound Source emulation on LPT1.
- -latency #       Change audio buffering and output latency. (default: 100 ms).
- -samprate #      Change audio emulation sample rate. (default: 48000 Hz).
- -console         Enable debug console on stdio during emulation.
- -menu						Enable window menu for changing emulation settings.


## Usage with Raspberry Pi
Raspberry Pi models 1,2,3 and 4 are supported but models 3 and 4 may still need improvement.
Faux86 is designed to be run 'bare metal' on a Raspberry Pi and directly on the hardware without any supporting OS.
Running directly on RPi hardware means the emulator can boot to bios in less than 2 secs.
By default Faux86 boots from the floppy image fd0.img which is mounted as Drive A: in the emulator.
An additional floppy drive (fd1.img Drive B:) can also be used if included on the Sd-Card.
The emulator also supports booting hard disk images (hd0.img Drive C:) (hd1.img Drive D:).
USB keyboard and mouse should be plugged in before booting and must not be removed once detected.

# Building and Compiling Sources
There is currently not a quick and easy way to expalin the build process, but if you are familiar
with C++ and make files then the process should not be too much of a problem for you.
You are very much on your own for compiling as there are far too many factors involved that can
cause a build to fail.

## Compiling for Windows
The Win32 build was compiled with Mingw and the [Dev-C++ v5.11](https://www.bloodshed.net/) IDE.
Dev-C++ is not a very modern IDE and was replaced with [Code::Blocks](https://www.codeblocks.org/) IDE.
A Dev-C++ project file is included with the sources for compiling your own build or you may import and
compile the sources using another preferred IDE.

# Compiling for ARM Raspberry Pi
The ARM build was compiled using the latest [ARM Toolchains](https://gnutoolchains.com/raspberry/).
The platform used was Raspbian Bullseye with the all the latest build tools and toolchains.
This is not a quick and easy setup to get working and there are many extra libraries required
for a sucessfull build.

Along with all the build tools and toolchains, the [C++ Circle SDK](https://github.com/rsta2/circle) is
also required for the Raspberry Pi build to be successful. THe Circle SDK requires some #defines to be
set for the correct Raspberry model being targetted. The makefile is pre-configured for Raspberry Pi 1.

# Credits
Faux86-remake was originally based on the Fake86 emulator by Mike Chambers and James Howard.
Most of the code has been updated and re-written in C++ but the core CPU emulation remains mostly the same.

The Raspberry Pi build uses RSTA2's Circle SDK to interface directly with the Raspberry Pi hardware.
This build would not of been possible without the support from RSTA2 and the Circle SDK. 
For more details about the Raspberry Pi Circle SDK visit [RSTA2 Circle SDK](https://github.com/rsta2/circle)

## Copyright
Based on Fake86 Copyright (C)2010-2013 Mike Chambers.
Faux86 Copyright (C)2018 James Howard.
Faux86-remake (c)2023 Curtis aka ArnoldUK.

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
