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

#include "../src/Config.h"
#include "../src/VM.h"
#include "../src/DriveManager.h"
#include "SDLInterface.h"
#include "settings.h"

#ifndef BUILD_STRING
	#error BUILD_STRING not defined. Add #include "Config.h" to this unit.
#endif

//#include "../data/asciivga.h"
//#include "../data/pcxtbios.h"
//#include "../data/videorom.h"

Faux86::VM* vm86;
Faux86::SDLHostSystemInterface hostInterface;

void LoadSettings(Faux86::Config &cfg);
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

	if (!vmConfig.parseCommandLine(argc, argv)) {
		LoadSettings(vmConfig);
	}
	
	//override settings and command parameters here
	
	vmConfig.singleThreaded = true;
	//vmConfig.slowSystem = 0;
	//vmConfig.frameDelay = 20; //20ms 50fps
	//vmConfig.cpuType = CPU_TYPE_V20;
	//vmConfig.cpuType = CPU_TYPE_8086;
	//vmConfig.cpuSpeed = 12;
	//vmConfig.cpuTiming = 15;
	//vmConfig.audio.sampleRate = 48000; //44100
	//vmConfig.audio.latency = 100;
	//vmConfig.audio.latency = 100;
	//vmConfig.useOPL3 = true;

	//vmConfig.framebuffer.width = 800;
	//vmConfig.framebuffer.height = 800;
	//vmConfig.resw = 720;
	//vmConfig.resh = 400;
	//vmConfig.renderQuality = RENDER_QUALITY_LINEAR;
	//vmConfig.monitorDisplay = MONITOR_DISPLAY_COLOR;
	
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

void LoadSettings(Faux86::Config &cfg) {
	Settings settings("faux86.cfg", "# FAUX86 XT Emulator Settings File", defaultconfig);
	settings.load();
	//settings.defaultconfig[fd0];
	string str;
	str = settings.get("fd0");
	cfg.loadFD0(str.c_str());
	str = settings.get("fd1");
	cfg.loadFD1(str.c_str());
	str = settings.get("hd0");
	cfg.loadHD0(str.c_str());
	str = settings.get("hd1");
	cfg.loadHD1(str.c_str());
	str = settings.get("biosrom");
	cfg.loadBiosRom(str.c_str());
	str = settings.get("videorom");
	cfg.loadVideoRom(str.c_str());
	str = settings.get("bootrom");
	cfg.loadBootRom(str.c_str());
	str = settings.get("charrom");
	cfg.loadCharRom(str.c_str());
	str = settings.get("boot");
	cfg.setBootDrive(str.c_str());
	cfg.enableAudio = !settings.getBool("nosound");
	cfg.useFullScreen = settings.getBool("fullscreen");
	cfg.verbose = settings.getBool("verbose");
	cfg.cpuSpeed = settings.getInt("speed");
	cfg.cpuType= settings.getInt("cpu");
	cfg.cpuTiming = settings.getInt("timing");
	cfg.frameDelay = settings.getInt("delay");
	cfg.slowSystem = settings.getBool("slowsys");
	cfg.singleThreaded = !settings.getBool("multithreaded");
	cfg.resw = settings.getInt("resw");
	cfg.resh = settings.getInt("resh");
	cfg.renderQuality = settings.getInt("render");
	cfg.monitorDisplay = settings.getInt("monitor");
	cfg.mousePort = settings.getInt("mouseport");
	cfg.useDisneySoundSource = settings.getBool("snddisney");
	cfg.useSoundBlaster = settings.getBool("sndblaster");
	cfg.useAdlib = settings.getBool("sndadlib");
	cfg.usePCSpeaker = settings.getBool("sndspeaker");
	cfg.useOPL3 = settings.getBool("sndopl3");
	cfg.audio.latency = settings.getInt("latency");
	cfg.audio.sampleRate = settings.getInt("samprate");
	cfg.enableConsole = settings.getBool("console");
	cfg.enableMenu = settings.getBool("menu");
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
