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
#include <Windows.h>
#include <stdio.h>
#include <new>

#include "../src/VM.h"
#include "../src/DriveManager.h"
#include "SDLInterface.h"

//#include "../data/asciivga.h"
//#include "../data/pcxtbios.h"
//#include "../data/videorom.h"

Faux86::VM* vm86;
Faux86::SDLHostSystemInterface hostInterface;

void HideConsole();
void ShowConsole();
bool IsConsoleVisible();
void SetScreenSize(uint16_t _scrw, uint16_t _scrh);
void SetRenderQuality(uint8_t _mode);
void SetCpuSpeed(uint32_t _speed);
void SetMonitorType(uint8_t _type);
void SendSoftReset();
void SendHardReset();

int main(int argc, char *argv[])
{
	//Faux86::SDLHostSystemInterface hostInterface;
	Faux86::Config vmConfig(&hostInterface);
	
	//Embedded Disks not currently supported
	//vmConfig.biosFile = new Faux86::EmbeddedDisk(pcxtbios, sizeof(pcxtbios));
	//vmConfig.videoRomFile = new Faux86::EmbeddedDisk(videorom, sizeof(videorom));
	//vmConfig.asciiFile = new Faux86::EmbeddedDisk(asciivga, sizeof(asciivga));
	
	//log(Log, "Faux86 Remake (c)2018 James Howard");
	//log(Log, "Based on Fake86 (c)2010-2013 Mike Chambers");
	//log(Log, "Contributions and Updates (c)2023 Curtis aka ArnoldUK");
	//log(Log, "Portable, open-source 8086 XT Emulator");

	vmConfig.parseCommandLine(argc, argv);
	
	//override command parameters
	vmConfig.singleThreaded = true;
	vmConfig.slowSystem = 0;
	//vmConfig.frameDelay = 20; //20ms 50fps
	//vmConfig.audio.sampleRate = 48000; //44100
	//vmConfig.audio.latency = 100;

	vmConfig.framebuffer.width = 800;
	vmConfig.framebuffer.height = 600;
	//vmConfig.resw = 720;
	//vmConfig.resh = 400;
	//vmConfig.renderQuality = 0;
	vmConfig.monitorDisplay = 0;
	
	if (!vmConfig.enableConsole) HideConsole();
	
	//uint8_t* allocSpace = new uint8_t[sizeof(Faux86::VM)];
	//for (size_t n = 0; n < sizeof(Faux86::VM); n++)	allocSpace[n] = 0xff;
	//Faux86::VM* vm86 = new (allocSpace) Faux86::VM(vmConfig);
	
	//Faux86::VM* vm86 = new Faux86::VM(vmConfig);
	vm86 = new Faux86::VM(vmConfig);
	
	if (vm86->init())
	{
		//hostInterface.init(vmConfig.resw, vmConfig.resh, vmConfig.enableMenu);
		hostInterface.init(vm86);
		
		//SetScreenSize(vmConfig.resw, vmConfig.resh);
		
		while (vm86->simulate())
		{
			//hostInterface.tick(*vm86);
			hostInterface.tick();
			//Sleep(1);
		}
	}

	delete vm86;
	
	//ShowConsole();

	return 0;
}

void HideConsole() {
	//if (!IsConsoleVisible()) return;
  ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}

void ShowConsole() {
	//if (IsConsoleVisible()) return;
  ::ShowWindow(::GetConsoleWindow(), SW_SHOW);
}

bool IsConsoleVisible() {
  return ::IsWindowVisible(::GetConsoleWindow()) != FALSE;
}

void SetScreenSize(uint16_t _scrw, uint16_t _scrh)
{
	if ((vm86->config.resw == _scrw) && (vm86->config.resh == _scrh)) return;
	
	vm86->config.resw = _scrw;
	vm86->config.resh = _scrh;
	
	if ((vm86->config.resw == 0) && (vm86->config.resh == 0)) {
		//vm86->config.hostSystemInterface->resize(vm86->renderer.nativeWidth, vm86->renderer.nativeHeight);
		hostInterface.resize(vm86->renderer.nativeWidth, vm86->renderer.nativeHeight);
	} else {
		//vm86->config.hostSystemInterface->resize(_scrw, _scrh);
		hostInterface.resize(_scrw, _scrh);
	}
}

void SetRenderQuality(uint8_t _mode) {
	vm86->config.renderQuality = _mode;
	hostInterface.setrendermode(_mode);
}


void SetCpuSpeed(uint32_t _speed) {
	vm86->config.cpuSpeed = _speed;
	hostInterface.updatetitle();
}

void SetMonitorType(uint8_t _type) {
	vm86->config.monitorDisplay = _type;
	hostInterface.setcolormode(_type);
}

void SendSoftReset() {
	hostInterface.sendkeydown( SDL_GetScancodeFromKey(SDLK_LCTRL) );
	hostInterface.sendkeydown( SDL_GetScancodeFromKey(SDLK_LALT) );
	hostInterface.sendkeydown( SDL_GetScancodeFromKey(SDLK_DELETE) );
	Sleep(100);
	hostInterface.sendkeyup( SDL_GetScancodeFromKey(SDLK_LCTRL) );
	hostInterface.sendkeyup( SDL_GetScancodeFromKey(SDLK_LALT) );
	hostInterface.sendkeyup( SDL_GetScancodeFromKey(SDLK_DELETE) );
}

void SendHardReset() {
	vm86->cpu.reset86();
}
