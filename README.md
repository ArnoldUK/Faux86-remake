# Faux86-remake
This is an improved and updated version of Faux86 XT PC Emulator.
A portable, open-source 8086 Emulator for Win32 and bare metal ARM Raspberry Pi.

## Current Status

Preparing Upload of initial build v1.0

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

# Emulator Features
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


## Usage with Raspberry Pi
Raspberry Pi models 1,2,3 and 4 are supported but models 3 and 4 may still need improvement.
Faux86 is designed to be run 'bare metal' on a Raspberry Pi and directly on the hardware without any supporting OS.
Running directly on RPi hardware means the emulator can boot to bios in less than 2 secs.
By default Faux86 boots from the floppy image fd0.img which is mounted as Drive A: in the emulator.
An additional floppy drive (fd1.img Drive B:) can also be used if included on the Sd-Card.
The emulator also supports booting hard disk images (hd0.img Drive C:) (hd1.img Drive D:).
USB keyboard and mouse should be plugged in before booting and must not be removed once detected.

# Credits
Faux86-remake was originally based on the Fake86 emulator by Mike Chambers. 
A lot of the code has been updated and re-written in C++ but the core CPU emulation remains mostly the same.

Faux86 uses RSTA2's Circle library to interface directly with the Raspberry Pi hardware.
https://github.com/rsta2/circle

## Copyright
Based on Fake86 Copyright (C)2010-2013 Mike Chambers
Faux86 Copyright (C)2018 James Howard
Faux86-remake (c)2023 Curtis aka ArnoldUK

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
