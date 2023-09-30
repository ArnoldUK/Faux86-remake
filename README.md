# Faux86-remake
This is an improved and updated version of Faux86 XT PC Emulator.
A portable, open-source 8086 Emulator for Win32 and bare metal ARM Raspberry Pi.

Faux86-remake is based on the Fake86 and [XTulator](https://github.com/mikechambers84/XTulator) 8086/8088 emulators
originally developed by Mike Chambers and James Howard.
Most of the code has been updated and re-written in C++ but the core CPU emulation remains mostly the same.

## Current Status

21-09-2023 Release build v1.22 for 32/64Bit Windows
[Faux86-remake Release Build V1.22 Win32](https://github.com/ArnoldUK/Faux86-remake/releases/tag/v1.22-Win32)

21-09-2023 Release build V1.22 for ARM RPi 1,2,3,4
[Faux86-remake Release Build V1.22 ARM-RPi](https://github.com/ArnoldUK/Faux86-remake/releases/tag/v1.22-RPi)

- Updated keyboard input and fixed repeated cursor keys.
- Huge increase in emulation speed on slower computers and all RPi models. 
- Upto 50% Text and Video rendering improvements. Thanks to [moononournation](https://github.com/moononournation)
- Upto 25% Audio performance increase when using basic OPL2 emulation.
- Added option `cpu=#` to settings file for the CPU type and opcode emulation. Default 2 for NEC V20.
- Added option `sndopl3=1` to settings file to enable/disable full Yamaha OPL3 emulation.
- Added more emulated monitor modes to `monitor=#` in settings file.
- Compiler support for ARDUINO. Thanks to [moononournation](https://github.com/moononournation)
- Minor fixes and code refactoring.

# Release Notes
Faux86-remake is still work in progress but fixes many issues and adds more features including:
- Updated CPU opcodes with support for more software and games.
- Improved emulation speed.
- Improved video rendering and mode switching.
- Improved audio emulation.
- Improved disk and file access, including writeable disk images.
- Additional monitor displays supported.
- Updated PCXT BIOS and Video Roms.
- More configuration parameters.
- Fully working mouse control in both DOS and Windows.
- Many bug fixes.

## Emulator Features
- 8086/8088, V20, 80186 and limited 286 instruction set.
- Configurable CPU speeds from 5Mhz upto 100Mhz.
- Custom Hardware BIOS's supported.
- Supports bootable disk images in .img and .raw file format.
- CGA / EGA / VGA Colour Video emulation with most modes supported.
- PC Speaker, Adlib, Soundblaster and Disney SoundSource.
- UART Com Ports.
- Standard PC XT Keyboard.
- Serial Port 2-Button mouse.

## Screenshots
### Booting BIOS and RAM Test
![screenshot1](/screenshots/faux86-remake-screenshot-1.png)

### Planet X3 EGA Color Mode
![screenshot2](/screenshots/faux86-remake-screenshot-2.png)

### SysChk System Info
![screenshot3](/screenshots/faux86-remake-screenshot-3.png)

### Windows 3.0 VGA 16 Colors
![screenshot4](/screenshots/faux86-remake-screenshot-4.png)

### Monitor Emulation Amber Terminal
![screenshot5](/screenshots/faux86-remake-screenshot-5.png)

### Monitor Emulation Green Terminal
![screenshot6](/screenshots/faux86-remake-screenshot-6.png)

### Borland BGI Demo DOS Colors
![screenshot7](/screenshots/faux86-remake-screenshot-7.png)

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
- asciivga.bin (ASCII Char ROM)

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

## Usage with Windows/Linux
The Windows build supports starting the emulator with [command line parameters](PARAMS.md).
If no command line parameters are supplied when running the emulator then a settings
file named `faux86.cfg` is used for configuring and starting the emulator.

Read the [SETTINGS.md](SETTINGS.md) documentation file for more details.


## Usage with Raspberry Pi
Raspberry Pi models 1,2,3 and 4 are supported but models 3 and 4 are still require some improvement.
Faux86 is designed to run 'bare metal' on a Raspberry Pi without any supporting OS.
Running 'bare metal' or directly on hardware means the emulator can boot to bios in less than 2 secs.

The zip archive contains all the kernels and disk images to boot the emulator directly from a supported
Raspberry Pi model. Copy all the files from within the zip archive to a FAT/FAT32 formatted SD-Card and
insert the card into your RPi. The RPi is now ready to boot the emulator from the SD-Card.
A USB keyboard and mouse should be plugged in before booting the emulator (hot plugging is unsupported).

By default Faux86 boots from a floppy image name `fd0.img` which is mounted as Drive A:.
An additional floppy drive `fd1.img` Drive B: can also be used if included on the Sd-Card.
The emulator also supports booting hard disk images `hd0.img` Drive C: and `hd1.img` Drive D:.

### IMPORTANT NOTES PLEASE READ:
Emulation performance and full hardware emulation capabilities depends on the Raspberry Pi model.
Currently all RPi models are capable of running the emulator with the same performance of an
XT 8088 Clone with a 12Mhz V20 CPU and basic speaker and OPL2 sound emulation.

It is recommended to only use a small standard (4Gb or less) SD-Card and not a SDHC or high speed card.
If the emulator fails to boot then using a smaller FAT16 formatted card provides more performance.

Raspberry Pi Models 1 and 2 cannot provide the processing power to fully emulate the Adlib
and Soundblaster sound cards. It is recommended to either disable Adlib and Soundblaster emulation
or reduce the samplerate and use only OPL2 emulation in settings file.
Increase the video rendering time from within the settings file will also boost emulation performance
but frame rates will be much lower.

## Emulator Settings File
The emulator is configured with a settings file named `faux86.cfg` located within the same folder
as the binary or kernel image. For RPi builds the settings file is name `faux86-#.cfg` where #
is the model number of your RPI. This allows RPi builds to use custom settings for each kernel on the
same SD-Card.

Read the [SETTINGS.md](SETTINGS.md) documentation file for more details.


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

Read the [BUILD.md](BUILD.md) documentation file for more details.

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
