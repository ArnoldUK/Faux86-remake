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

#ifdef _WIN32
#include <SDL.h>
#include <SDL_syswm.h>
#include <Windows.h>
#else
#include <SDL.h>
#include <sys/time.h>
#endif

#include "../src/HostSystemInterface.h"
#include "../src/Renderer.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;

//extern void initmenus(HWND hwnd);

namespace Faux86
{
	class VM;
	
	class SDLFrameBufferInterface : public FrameBufferInterface
	{
	public:
		virtual void init(uint32_t desiredWidth, uint32_t desiredHeight) override;	
		virtual void resize(uint32_t desiredWidth, uint32_t desiredHeight) override;
		virtual RenderSurface* getSurface() override;
		virtual void setPalette(Palette* palette) override;
		//virtual void present() override;
		virtual void blit(uint32_t *pixels, int w, int h, int stride) override;
		
		//void black_white_dither(SDL_Surface* s);
		void SetColorEmulation(uint8_t _colormode);

		SDL_Window* sdlWindow = nullptr;
		uint8_t colorEmulation = 0;
	private:
		uint32_t fbwidth;
		uint32_t fbheight;
		
		SDL_Renderer* appRenderer = nullptr;
		SDL_Surface* screenPixels = nullptr;
		SDL_Surface* screenSurface = nullptr;
		SDL_Texture* screenTexture = nullptr;

		RenderSurface renderSurface;
	};

	class SDLTimerInterface : public TimerInterface
	{
	public:
		virtual uint64_t getHostFreq() override;
		virtual uint64_t getTicks() override;
	};

	class SDLAudioInterface : public AudioInterface
	{
	public:
		virtual void init(VM& vm) override;
		virtual void shutdown() override;

	private:
		static void fillAudioBuffer(void *udata, uint8_t *stream, int len);
	};

	class SDLHostSystemInterface : public HostSystemInterface
	{
	public:
		SDLHostSystemInterface();
		virtual ~SDLHostSystemInterface();
		//virtual void init(uint32_t desiredWidth, uint32_t desiredHeight, bool showmenu) override;
		virtual void init(VM* inVM) override;
		virtual void resize(uint32_t desiredWidth, uint32_t desiredHeight) override;
		
		virtual AudioInterface& getAudio() override { return audioInterface; }
		virtual FrameBufferInterface& getFrameBuffer() override { return frameBufferInterface; }
		virtual TimerInterface& getTimer() override { return timerInterface;  }
		virtual DiskInterface* openFile(const char* filename) override;

		//void tick(VM& vm);
		void tick();
		void updatetitle();
		void setrendermode(uint8_t _mode);
		void setcolormode(uint8_t _mode);
		void sendkeydown(uint8_t scancode);
		void sendkeyup(uint8_t scancode);
		
		SDL_Window* sdlWindow = nullptr;
		
		#ifdef _WIN32
		HWND getSdlHWND(void);
		SDL_SysWMinfo getSdlWMinfo(void);
		#endif

	private:
		//VM& vm;
		char* sdltitle;

		#ifdef _WIN32
		HWND sdlhwnd;
		SDL_SysWMinfo wmInfo;
		#endif
		
		uint8_t translatescancode(uint16_t keyval);

		SDLAudioInterface audioInterface;
		SDLFrameBufferInterface frameBufferInterface;
		SDLTimerInterface timerInterface;
	};

};
