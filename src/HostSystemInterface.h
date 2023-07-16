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

#include "Types.h"

namespace Faux86
{
	enum LogChannel
	{
		Log,
		LogError,
		LogVerbose,
		LogFatal,
		LogRaw,
		LogDebugger
	};

	// Must be implemented by host system
	void log(LogChannel channel, const char* message, ...);

	class DiskInterface;
	class VM;
	class Palette;
	struct RenderSurface;

	class FrameBufferInterface
	{
	public:
		virtual void init(uint32_t desiredWidth, uint32_t desiredHeight) {}
		virtual void resize(uint32_t desiredWidth, uint32_t desiredHeight) {}
		virtual RenderSurface* getSurface() = 0;

		virtual void setPalette(Palette* palette) = 0;

		//virtual void present() {}
		
		virtual void blit(uint32_t *pixels, int w, int h, int stride) = 0;
		
		//virtual uint32_t getWidth() = 0;
		//virtual uint32_t getHeight() = 0;
	};

	class TimerInterface
	{
	public:
		virtual uint64_t getHostFreq() = 0;
		virtual uint64_t getTicks() = 0;
	};

	class AudioInterface
	{
	public:
		virtual void init(VM& vm) = 0;
		virtual void shutdown() = 0;
	};

	class HostSystemInterface
	{
	public:
		//virtual void init(uint32_t desiredWidth, uint32_t desiredHeight, bool showmenu);
		virtual void init(VM* inVM);
		virtual void resize(uint32_t desiredWidth, uint32_t desiredHeight);
		virtual FrameBufferInterface& getFrameBuffer() = 0;
		virtual TimerInterface& getTimer() = 0;
		virtual AudioInterface& getAudio() = 0;
		virtual DiskInterface* openFile(const char* filename) { return nullptr; }
		
		uint32_t scrWidth = 0;
		uint32_t scrHeight = 0;
		
		VM* vm;
	};
}
