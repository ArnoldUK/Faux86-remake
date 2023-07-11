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

#include "VM.h"
#include "Renderer.h"
#include "Profiler.h"
#include "MemUtils.h"

//#include <circle/sched/scheduler.h>

using namespace Faux86;

Mutex Renderer::screenMutex;

//TODO
//inline uint32_t mapRGB(uint8_t b, uint8_t g, uint8_t r)
//{
//	return (r | (g << 8) | (b << 16) | (0xff << 24));
//}

Renderer::Renderer(VM& inVM)
	: vm(inVM)
{
	log(Log,"[RENDERER] Constructed");
	scalemap = nullptr;
}

Renderer::~Renderer()
{
	//delete[] scalemap;
	if (renderSurface && renderSurface != hostSurface)
	{
		RenderSurface::destroy(renderSurface);
	}
}

//void Renderer::init(FrameBufferInterface *framebuffer)
void Renderer::init(uint32_t _fbwidth, uint32_t _fbheight)
{
	log(Log,"[RENDERER] Initialized FrameBuffer %d x %d", _fbwidth, _fbheight);
	
	fbWidth = _fbwidth;
	fbHeight = _fbheight;
	
	fb = &vm.config.hostSystemInterface->getFrameBuffer();
	//fb->init(OUTPUT_FRAMEBUFFER_WIDTH, OUTPUT_FRAMEBUFFER_HEIGHT);
	//fb->init(1024, 1024);
	fb->init(fbWidth, fbHeight);

	hostSurface = fb->getSurface();

#ifdef DOUBLE_BUFFER
	log(Log,"[RENDERER] DOUBLE_BUFFER Enabled");
  renderSurface = RenderSurface::create(fbWidth, fbHeight);
#else
	renderSurface = hostSurface;
#endif

	//InitMutex(screenMutex);

	//vm.taskManager.addTask(new RenderTask(*this));
}

void Renderer::markScreenModeChanged(uint32_t newWidth, uint32_t newHeight)
{
	//log(Log, "[RENDERER] markScreenModeChanged %d x %d", newWidth, newHeight);
	//refreshTextMode();
	screenModeChanged = true;
	nativeWidth = newWidth;
	nativeHeight = newHeight;
	vm.config.hostSystemInterface->resize(newWidth, newHeight);
}


/* STANDARD VIDEO MODES
	00: 40x25 320x200 4-Bit Monochrome TEXT 8x8 (CGA,EGA,MCGA,VGA)
	01: 40x25 320x200 4-Bit 16 color TEXT 8x8 (CGA,EGA,MCGA,VGA)
	02: 80x25 640x400 4-Bit 16 gray scale TEXT 8x16 (CGA,EGA,MCGA,VGA)
	03: 80x25 640x400 4-Bit 16 color TEXT 8x16 (CGA,EGA,MCGA,VGA)
	04: 40x25 320x200 2-Bit 4 color GRAPHICS 8x8 (CGA,EGA,MCGA,VGA)
	05: 40x25 320x200 2-Bit 4 color GRAPHICS 8x8 (CGA,EGA,MCGA,VGA)
	06: 80x25 640x200 1-Bit Monochrome GRAPHICS 8x8 (CGA,EGA,MCGA,VGA)
	07: 80x25 640x400 1-Bit Monochrome TEXT 8x16 (MDA,HERC,EGA,VGA)
	08: 20x25 160x200 4-Bit 16 color GRAPHICS 8x8 (PCjr)
	09: 40x25 320x200 4-Bit 16 color GRAPHICS 8x8 (PCjr)
	0A: 80x25 640x200 2-Bit 4 color GRAPHICS 8x8 (PCjr)
	0B: 40x25 320x200 4-Bit 16 color GRAPHICS 8x8 (EGA)
	0C: 80x25 640x200 2-Bit 4 color GRAPHICS 8x8 (EGA)
	0D: 40x25 320x200 4-Bit 16 color GRAPHICS 8x8 (EGA,VGA)
	0E: 80x25 640x200 4-Bit 16 color GRAPHICS 8x8 (EGA,VGA)
	0F: 80x25 640x350 1-Bit Monochrome GRAPHICS 8x16 (EGA,VGA)
	10: 80x25 640x350 4-Bit 16 color GRAPHICS 8x16 (EGA or VGA with 128K)
			640x350 4 color graphics (64K EGA)
	11: 80x30 640x480 1-Bit Monochrome GRAPHICS 8x16 (MCGA,VGA)
	12: 80x30 640x480 4-Bit 16 color GRAPHICS 8x16 (VGA)
	13: 40x25 320x200 8-Bit 256 color GRAPHICS 8x8 (MCGA,VGA)
*/

void Renderer::draw(uint32_t* pixels, int w, int h, int stride) {
	fb->blit(pixels, w, h, stride);
}


void RenderTask::begin()
{
	cursorprevtick = (uint32_t) vm.timing.getMS();
	vm.video.cursorvisible = 0;
}

constexpr int targetTime = 16; //16ms
uint64_t drawStartTime = 0;
int delayTime = 0;

int RenderTask::update()
{
	return 0;
	/*
	//ProfileBlock block(vm.timing, "RenderTask::update");

	// Blink cursor
	cursorcurtick = (uint32_t)vm.timing.getMS();
	if ((cursorcurtick - cursorprevtick) >= 250) //250
	{
		vm.video.updatedscreen = 1;
		vm.video.cursorvisible = ~vm.video.cursorvisible & 1;
		cursorprevtick = cursorcurtick;
		
		//vm.renderer.markTextDirty(vm.renderer.cursorX, vm.renderer.cursorY);
	}

	//uint64_t drawStartTime = vm.timing.getTicks();
	drawStartTime = vm.timing.getTicks();
	
	//if (vm.video.updatedscreen || vm.config.renderBenchmark)
	//{
	//	vm.video.updatedscreen = 0;
	//	if (renderer.fb != nullptr)
	//	{
	//		renderer.draw();
	//	}
	//	renderer.totalframes++;
	//}

	if (vm.video.updatedscreen)	{
		//renderer.draw();
		//renderer.fb->setPalette(vm.video.getCurrentPalette());
		//renderer.draw();
		vm.video.updatedscreen = false;
	}

	//constexpr int targetTime = 16; //16ms
	//constexpr int targetTime = 20; //32ms
	//int delayTime = targetTime - (int)vm.timing.getElapsedMS(drawStartTime);
	delayTime = targetTime - (int)vm.timing.getElapsedMS(drawStartTime);

	if (delayTime < 1) delayTime = 1;
	if (delayTime > targetTime)	delayTime = targetTime;
	//CScheduler::Get()->Yield();
	return delayTime;
	
	//if (!vm.config.renderBenchmark)
	//{
	//	int delaycalc = vm.config.frameDelay - (uint32_t)(vm.timing.getMS() - cursorcurtick);
	//	if (delaycalc > vm.config.frameDelay) 
	//		delaycalc = vm.config.frameDelay;
	//	return delaycalc;
	//}
	//return 0;
	*/
}

void Renderer::setCursorPosition(uint32_t x, uint32_t y)
{
	/*
	if ( (cursorX == x) && (cursorY == y) ) return;
	markTextDirty(cursorX, cursorY);
	cursorX = x;
	cursorY = y;
	//markTextDirty(cursorX, cursorY);
	*/
}

/*
void Renderer::stretchBlit() 
{
	return;
	//roughBlit();
	simpleBlit();
}
*/

/*
void Renderer::simpleBlit()
{
	//ProfileBlock block(vm.timing, "Renderer::simpleBlit");
	{
		//ProfileBlock block(vm.timing, "Renderer::simpleBlit inner");
		for (uint32_t y = 0; y < hostSurface->height; y++) {
			MemUtils::memcpy(hostSurface->pixels + (y * hostSurface->pitch), renderSurface->pixels + (y * renderSurface->pitch), hostSurface->width);
		}
	}
}
*/

/*
void Renderer::roughBlit() {
	uint32_t srcx, srcy, dstx, dsty, scalemapptr;
	uint8_t* pixels = hostSurface->pixels;
	uint32_t pitch = hostSurface->pitch;
	uint32_t width = hostSurface->width;
	uint32_t height = hostSurface->height;
	{
		scalemapptr = 0;
		for (dsty = 0; dsty < height; dsty++)	{
			srcy = scalemap[scalemapptr++];
			uint8_t* dstPtr = pixels + dsty * pitch;

			for (dstx = 0; dstx < width; dstx++) {
				srcx = scalemap[scalemapptr++];
				*dstPtr++ = renderSurface->get(srcx, srcy);
			}
		}
	}
}
*/

/* NOTE:
	doubleblit is only used when smoothing is not enabled, and the SDL window size
	is exactly double of native resolution for the current video mode. we can take
	advantage of the fact that every pixel is simply doubled both horizontally and
	vertically. this way, we do not need to waste mountains of CPU time doing
	floating point multiplication for each and every on-screen pixel. it makes the
	difference between games being smooth and playable, and being jerky on my old
	400 MHz PowerPC G3 iMac.
*/
/*
void Renderer::doubleBlit() {
	uint32_t srcx, srcy, dstx, dsty, curcolor;
	int32_t ofs;
	uint8_t* pixels = hostSurface->pixels;
	uint32_t pitch = hostSurface->pitch;
	uint32_t width = hostSurface->width;
	uint32_t height = hostSurface->height;

	for (dsty=0; dsty<height; dsty += 2) 
	{
		srcy = (uint32_t) (dsty >> 1);
		ofs = dsty * pitch;
		for (dstx=0; dstx < width; dstx += 2) 
		{
			srcx = (uint32_t) (dstx >> 1);
			curcolor = renderSurface->get(srcx, srcy);
			pixels [ofs+pitch] = curcolor;
			pixels [ofs++] = curcolor;
			pixels [ofs+pitch] = curcolor;
			pixels [ofs++] = curcolor;
		}
	}
}
*/


RenderSurface* RenderSurface::create(uint32_t inWidth, uint32_t inHeight)
{
	log(Log,"[RENDERER] RenderSurface::create %d x %d", inWidth, inHeight);
	RenderSurface* newSurface = new RenderSurface();
	newSurface->width = newSurface->pitch = inWidth;
	newSurface->height = inHeight;
	newSurface->pixels = new uint8_t[inWidth * inHeight];
	return newSurface;
}

void RenderSurface::destroy(RenderSurface* surface)
{
	delete[] surface->pixels;
	delete surface;
}

