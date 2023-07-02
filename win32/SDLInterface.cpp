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

/*
#ifdef _WIN32
#include <SDL.h>
#include <SDL_syswm.h>
#include <Windows.h>
#else
#include <SDL.h>
#include <sys/time.h>
#endif
*/

#include "SDLInterface.h"
#include "StdioDiskInterface.h"
#include "../src/VM.h"
#include "Keymap.h"

extern void initmenus(HWND hwnd);

using namespace Faux86;

//#define SDL_PIXEL_FORMAT	SDL_PIXELFORMAT_RGBA8888
//#define SDL_PIXEL_FORMAT		SDL_PIXELFORMAT_RGB24
//#define SDL_PIXEL_FORMAT		SDL_PIXELFORMAT_ARGB8888
#define SDL_PIXEL_FORMAT		SDL_PIXELFORMAT_RGB888

bool sdlconsole_ctrl = 0;
bool sdlconsole_alt = 0;

SDLHostSystemInterface::SDLHostSystemInterface()
{
	//if (SDL_Init(SDL_INIT_VIDEO)) return -1;
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
}

//void SDLHostSystemInterface::init(uint32_t desiredWidth, uint32_t desiredHeight, bool showmenu)
void SDLHostSystemInterface::init(VM* inVM)
{
	vm = inVM;
	uint32_t desiredWidth = HOST_WINDOW_WIDTH; //vm->config.resw;
	uint32_t desiredHeight = HOST_WINDOW_HEIGHT; //vm->config.resh;
	bool showmenu = vm->config.enableMenu;
	
	setrendermode(vm->config.renderQuality);
	
	sdlWindow = SDL_CreateWindow(
		BUILD_STRING,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		desiredWidth, desiredHeight,
		//SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		SDL_WINDOW_RESIZABLE);
		
	if (sdlWindow == nullptr) return;
	frameBufferInterface.sdlWindow = sdlWindow;
	frameBufferInterface.colorEmulation = vm->config.monitorDisplay;
	//frameBufferInterface.setSDLWindow(sdlWindow);
	
	#ifdef _WIN32
	if (showmenu) {
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(sdlWindow, &wmInfo);
		sdlhwnd = wmInfo.info.win.window;
		initmenus(sdlhwnd);
	}
	#endif

	//SDL_SetWindowSize(sdlWindow, desiredWidth, desiredHeight);
	SDL_SetWindowSize(sdlWindow, vm->config.resw, vm->config.resh);
	
	updatetitle();
	
	//frameBufferInterface.init(vm->config.resw, vm->config.resh);
}

void SDLHostSystemInterface::resize(uint32_t desiredWidth, uint32_t desiredHeight)
{
	log(Log, "[SDL] SDLHostSystemInterface::resize %lu x %lu", desiredWidth, desiredHeight);
	//if ((scrWidth == desiredWidth) && (scrHeight == desiredHeight)) return;
	scrWidth = desiredWidth;
	scrHeight = desiredHeight;
	SDL_SetWindowSize(sdlWindow, scrWidth, scrHeight);	
}

void SDLAudioInterface::init(VM& vm)
{
	SDL_AudioSpec wanted;

	log(Log, "[SDL] SDLAudioInterface::init Initializing audio stream... ");

	wanted.freq = vm.config.audio.sampleRate;
	wanted.format = AUDIO_U8;
	wanted.channels = 1;
	wanted.samples = (uint16_t)((vm.config.audio.sampleRate / 1000) * vm.config.audio.latency) >> 1;
	wanted.callback = fillAudioBuffer;
	wanted.userdata = &vm;

	if (SDL_OpenAudio(&wanted, NULL) <0) {
		log(Log, "[SDL] SDLAudioInterface::init Error: %s", SDL_GetError());
	}
	else {
		log(Log, "[SDL] SDLAudioInterface::init (%lu Hz, %lu ms, %lu sample latency)",
			vm.config.audio.sampleRate, vm.config.audio.latency, wanted.samples);
	}

	SDL_PauseAudio(0);
}

void SDLAudioInterface::shutdown()
{
	SDL_PauseAudio(1);
}

void SDLAudioInterface::fillAudioBuffer(void *udata, uint8_t *stream, int len)
{
	VM* vm = (VM*)(udata);
	vm->audio.fillAudioBuffer(stream, len);
}

SDLHostSystemInterface::~SDLHostSystemInterface()
{
	SDL_Quit();
}

DiskInterface* SDLHostSystemInterface::openFile(const char* filename)
{
	return new StdioDiskInterface(filename);
}

#ifdef _WIN32
HWND SDLHostSystemInterface::getSdlHWND(void) {
	return sdlhwnd;
}
#endif

#ifdef _WIN32
SDL_SysWMinfo SDLHostSystemInterface::getSdlWMinfo(void) {
	return wmInfo;
}
#endif

/*
SDLFrameBufferInterface::SDLFrameBufferInterface(SDL_Window* sdlWnd)
{
	sdlWindow = sdlWnd;
}
*/
//void SDLFrameBufferInterface::init(SDL_Window* sdlWnd, uint32_t desiredWidth, uint32_t desiredHeight)
void SDLFrameBufferInterface::init(uint32_t desiredWidth, uint32_t desiredHeight)
{
	//#ifdef SDL_DEBUG
	log(LogVerbose, "[SDL] FrameBufferInterface::init %lu x %lu", desiredWidth, desiredHeight);
	//#endif

	fbwidth = desiredWidth;
	fbheight = desiredHeight;
	
	//sdlWindow = frameBufferInterface.sdlWindow;
	
	resize(fbwidth, fbheight);
}

void SDLFrameBufferInterface::resize(uint32_t desiredWidth, uint32_t desiredHeight)
{
	#ifdef SDL_DEBUG
	log(LogVerbose, "[SDL] FrameBufferInterface::resize %lu x %lu", desiredWidth, desiredHeight);
	#endif
	
	if ( (fbwidth == desiredWidth) && (fbheight == desiredHeight) ) return;
	
	fbwidth = desiredWidth;
	fbheight = desiredHeight;
	
	if (appRenderer != nullptr) SDL_DestroyRenderer(appRenderer);
	if (screenTexture != nullptr) SDL_DestroyTexture(screenTexture);
	if (screenPixels != nullptr) SDL_FreeSurface(screenPixels);
	appRenderer = nullptr;
	screenTexture = nullptr;
	screenPixels = nullptr;
	
	screenPixels = SDL_CreateRGBSurfaceWithFormat(0, desiredWidth, desiredHeight, 32, SDL_PIXEL_FORMAT);
	//appRenderer = SDL_GetRenderer(sdlWindow);
	appRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);
	//appRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_SOFTWARE);
	//appRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_PRESENTVSYNC);
	//appRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
	if (appRenderer == nullptr) return;

	screenTexture = SDL_CreateTexture(appRenderer,
		SDL_PIXEL_FORMAT, SDL_TEXTUREACCESS_STREAMING,
		fbwidth, fbheight);
		
	if (screenTexture == nullptr) return;
	
	//if ( (scrWidth == 0) && (scrHeight == 0) )
		//SDL_SetWindowSize(sdlWindow, desiredWidth, desiredHeight);

	return;
}

RenderSurface* SDLFrameBufferInterface::getSurface()
{
	return &renderSurface;
}

void SDLFrameBufferInterface::setPalette(Palette* palette)
{
	return;
	SDL_Color colours[256];

	for (int n = 0; n < 256; n++)
	{
		colours[n].r = palette->colours[n].r;
		colours[n].g = palette->colours[n].g;
		colours[n].b = palette->colours[n].b;
	}
	SDL_SetPaletteColors(screenPixels->format->palette, colours, 0, 256);
}

/*
void SDLFrameBufferInterface::present() 
{
	return;
	
	SDL_BlitSurface(surface, nullptr, screenSurface, nullptr);

	void* pixels;
	int pitch;
	SDL_LockTexture(screenTexture, nullptr, &pixels, &pitch);
	SDL_ConvertPixels(screenSurface->w, screenSurface->h, screenSurface->format->format, screenSurface->pixels, screenSurface->pitch, SDL_PIXEL_FORMAT, pixels, pitch);
	SDL_UnlockTexture(screenTexture);
	
	//SDL_UpdateTexture(screenTexture, nullptr, screenSurface->pixels, screenSurface->pitch); //ADDED
	SDL_RenderClear(appRenderer); //ADDED
	SDL_RenderCopy(appRenderer, screenTexture, nullptr, nullptr);
	SDL_RenderPresent(appRenderer);
}
*/

void SDLFrameBufferInterface::blit(uint32_t *pixels, int w, int h, int stride) {
	
	//log(LogVerbose, "[SDL] FrameBufferInterface::blit %d x %d x %d", w, h, stride);

	if ((w != fbwidth) || (h != fbheight)) {
		//log(LogVerbose, "[SDL] FrameBufferInterface::blit %lu x %lu", fbwidth, fbheight);
		resize(w, h);
	}
	
	/*
	//unsigned char* dstPixels = nullptr;
	void* dstPixels;
	int dstStride;
	int sdlres;
	
	//sdlres = SDL_LockTexture(screenTexture, nullptr, (void**)pixels, &stride);
	sdlres = SDL_LockTexture(screenTexture, nullptr, &dstPixels, &dstStride);
	if (sdlres == 0) {
	for (unsigned int i = 0; i < fbheight * dstStride; ++i)
	{
	  //memcpy(dstPixels, pixels, dstStride);
	  //pixels += dstStride;
	  //dstPixels += dstStride;
	  //(uint8_t*)dstPixels[i] = 255;
	}
	memcpy(dstPixels, pixels, 255);
	SDL_UnlockTexture(screenTexture);
	}	else {
		log(Log, "[SDL] SDL_LockTexture %d %s", sdlres, SDL_GetError());
	}
	*/
	
	
	//SDL_Surface* screen = SDL_GetWindowSurface(sdlWindow);
	//SDL_Surface* screenPixels = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
  //SDL_Surface* screenPixels = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXEL_FORMAT);
  //SDL_FillRect(screenPixels, NULL, 0); //clear pixels to black background

	if (screenPixels == nullptr) screenPixels = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXEL_FORMAT);
	//SDL_FillRect(screenPixels, NULL, SDL_MapRGB(screenPixels->format, 0, 0, 100));
	//SDL_SetColorKey(screenPixels, SDL_TRUE, SDL_MapRGB(screenPixels->format, 0, 0, 255));
	
	//write the pixels
  SDL_LockSurface(screenPixels);
  {
    int pitch = screenPixels->pitch;
    for (unsigned int y = 0; y < h; y++)	{
		  memcpy((uint8_t*)screenPixels->pixels + pitch * y, (uint8_t*)pixels + stride * y, stride);
		}
  }
  SDL_UnlockSurface(screenPixels);
  
  //SDL_BlitSurface(spixels, NULL, screen, NULL);
  //SDL_BlitSurface(spixels, nullptr, screenSurface, nullptr);
  //SDL_UpdateTexture(screenTexture, nullptr, pixels, stride);
  SDL_UpdateTexture(screenTexture, nullptr, screenPixels->pixels, screenPixels->pitch);
	//SDL_SetTextureAlphaMod(screenTexture, 200);
  //SDL_SetTextureBlendMode(screenTexture, SDL_BLENDMODE_ADD); //SDL_BLENDMODE_BLEND
  
  if (colorEmulation) {
  	uint8_t r, g, b;
	  //uint8_t c;
	  SDL_GetTextureColorMod(screenTexture, &r, &g, &b);
	  //c = (r + g + b) / 3;
	  //c = 0.212671f * r + 0.715160f * g + 0.072169f * b;
	  //r = c >> 16 & 0xFF;
	  //g = c >> 8 & 0xFF;
	  //b = c & 0xFF;
	  switch (colorEmulation) {
	  	case MONITOR_DISPLAY_AMBER:
				SDL_SetTextureColorMod(screenTexture, 225, 40, 0); //Red channel only
				break;
			case MONITOR_DISPLAY_GREEN:
				SDL_SetTextureColorMod(screenTexture, 0, 200, 0); //Green channel only
				break;
			case MONITOR_DISPLAY_BLUE:
				SDL_SetTextureColorMod(screenTexture, 0, 40, 255); //Blue channel only
				break;
		}
	} else SDL_SetTextureColorMod(screenTexture, 255, 255, 255);
	
	//SDL_SetTextureAlphaMod(screenTexture, 127);
	//SDL_SetRenderDrawColor(appRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	
  SDL_RenderClear(appRenderer);
  SDL_RenderCopy(appRenderer, screenTexture, NULL, NULL);
  SDL_RenderPresent(appRenderer);
  //SDL_UpdateWindowSurface(sdlWindow);
	return;
  
	/*
	if (lasttime != 0) {
		int i, avgcount;
		uint64_t curavg;
		char tmp[64];
		sdlconsole_frameTime[sdlconsole_frameIdx++] = curtime - lasttime;
		if (sdlconsole_frameIdx == 30) {
			sdlconsole_frameIdx = 0;
			avgcount = 0;
			curavg = 0;
			for (i = 0; i < 30; i++) {
				if (sdlconsole_frameTime[i] != 0) {
					curavg += sdlconsole_frameTime[i];
					avgcount++;
				}
			}
			curavg /= avgcount;
			sprintf(tmp, "%.2f FPS", (double)((timing_getFreq() * 10) / curavg) / 10);
			sdlconsole_setTitle(tmp);
		}
	}
	*/
	//lasttime = curtime;
}

void SDLFrameBufferInterface::SetColorEmulation(uint8_t _colormode) {
	colorEmulation = _colormode;
}


/*
void SDLFrameBufferInterface::black_white_dither(SDL_Surface* s) {
  uint32_t x, y;
  
  // Ordered dither kernel
  uint16_t map[8][8] = {
    { 1, 49, 13, 61, 4, 52, 16, 64 },
    { 33, 17, 45, 29, 36, 20, 48, 32 },
    { 9, 57, 5, 53, 12, 60, 8, 56 },
    { 41, 25, 37, 21, 44, 28, 40, 24 },
    { 3, 51, 15, 63, 2, 50, 14, 62 },
    { 25, 19, 47, 31, 34, 18, 46, 30 },
    { 11, 59, 7, 55, 10, 58, 6, 54 },
    { 43, 27, 39, 23, 42, 26, 38, 22 }
  };
  
  for(y = 0; y < s->h; ++y) {
    for(x = 0; x < s->w; ++x) {
      uint32_t pix = getpixel(s, x, y);
      uint8_t r, g, b;
      SDL_GetRGB(pix, s->format, &r, &g, &b);
      
      // Convert the pixel value to grayscale i.e. intensity
      float in = .299 * r + .587 * g + .114 * b;
      
      // Apply the ordered dither kernel
      uint16_t val = in + in * map[y % 8][x % 8] / 63;
      
      // If >= 192 choose white, else choose black
      if(val >= 192)
        val = 255;
      else
        val = 0;
      
      // Put the pixel back in the image
      putpixel(s, x, y, SDL_MapRGB(s->format, val, val, val));
    }
  }
}
*/

/*
void SDLFrameBufferInterface::setSDLWindow(SDL_Window* sdlwnd) {
	sdlWindow = sdlwnd;
}
*/

uint64_t SDLTimerInterface::getHostFreq() 
{
#ifdef _WIN32
	LARGE_INTEGER queryperf;
	QueryPerformanceFrequency(&queryperf);
	return queryperf.QuadPart;
#else
	return 1000000;
#endif
}

uint64_t SDLTimerInterface::getTicks() 
{
#ifdef _WIN32
	LARGE_INTEGER queryperf;
	QueryPerformanceCounter(&queryperf);
	return queryperf.QuadPart;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)tv.tv_sec * (uint64_t)1000000 + (uint64_t)tv.tv_usec;
#endif
}

uint8_t SDLHostSystemInterface::translatescancode(uint16_t keyval)
{
	switch (keyval) {
	case 0x1B:
		return (1);
		break; //Esc
	case 0x30:
		return (0xB);
		break; //zero
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
		return (keyval - 0x2F);
		break; //other number keys
	case 0x2D:
		return (0xC);
		break; //-_
	case 0x3D:
		return (0xD);
		break; //=+
	case 0x8:
		return (0xE);
		break; //backspace
	case 0x9:
		return (0xF);
		break; //tab
	case 0x71:
		return (0x10);
		break;
	case 0x77:
		return (0x11);
		break;
	case 0x65:
		return (0x12);
		break;
	case 0x72:
		return (0x13);
		break;
	case 0x74:
		return (0x14);
		break;
	case 0x79:
		return (0x15);
		break;
	case 0x75:
		return (0x16);
		break;
	case 0x69:
		return (0x17);
		break;
	case 0x6F:
		return (0x18);
		break;
	case 0x70:
		return (0x19);
		break;
	case 0x5B:
		return (0x1A);
		break;
	case 0x5D:
		return (0x1B);
		break;
	case 0xD:
	case 0x10F:
		return (0x1C);
		break; //enter
	case 0x131:
	case 0x132:
		return (0x1D);
		break; //ctrl
	case 0x61:
		return (0x1E);
		break;
	case 0x73:
		return (0x1F);
		break;
	case 0x64:
		return (0x20);
		break;
	case 0x66:
		return (0x21);
		break;
	case 0x67:
		return (0x22);
		break;
	case 0x68:
		return (0x23);
		break;
	case 0x6A:
		return (0x24);
		break;
	case 0x6B:
		return (0x25);
		break;
	case 0x6C:
		return (0x26);
		break;
	case 0x3B:
		return (0x27);
		break;
	case 0x27:
		return (0x28);
		break;
	case 0x60:
		return (0x29);
		break;
	case 0x130:
		return (0x2A);
		break; //left shift
	case 0x5C:
		return (0x2B);
		break;
	case 0x7A:
		return (0x2C);
		break;
	case 0x78:
		return (0x2D);
		break;
	case 0x63:
		return (0x2E);
		break;
	case 0x76:
		return (0x2F);
		break;
	case 0x62:
		return (0x30);
		break;
	case 0x6E:
		return (0x31);
		break;
	case 0x6D:
		return (0x32);
		break;
	case 0x2C:
		return (0x33);
		break;
	case 0x2E:
		return (0x34);
		break;
	case 0x2F:
		return (0x35);
		break;
	case 0x12F:
		return (0x36);
		break; //right shift
	case 0x13C:
		return (0x37);
		break; //print screen
	case 0x133:
	case 0x134:
		return (0x38);
		break; //alt
	case 0x20:
		return (0x39);
		break; //space
	case 0x12D:
		return (0x3A);
		break; //caps lock
	case 0x11A:
	case 0x11B:
	case 0x11C:
	case 0x11D:
	case 0x11E:
	case 0x11F:
	case 0x120:
	case 0x121:
	case 0x122:
	case 0x123:
		return (keyval - 0x11A + 0x3B);
		break; //F1 to F10
	case 0x12C:
		return (0x45);
		break; //num lock
	case 0x12E:
		return (0x46);
		break; //scroll lock
	case 0x116:
	case 0x107:
		return (0x47);
		break; //home
	case 0x111:
	case 0x108:
		return (0x48);
		break; //up
	case 0x118:
	case 0x109:
		return (0x49);
		break; //pgup
	case 0x10D:
		return (0x4A);
		break; //keypad -
	case 0x114:
	case 0x104:
		return (0x4B);
		break; //left
	case 0x105:
		return (0x4C);
		break; //center
	case 0x113:
	case 0x106:
		return (0x4D);
		break; //right
	case 0x10E:
		return (0x4E);
		break; //keypad +
	case 0x117:
	case 0x101:
		return (0x4F);
		break; //end
	case 0x112:
	case 0x102:
		return (0x50);
		break; //down
	case 0x119:
	case 0x103:
		return (0x51);
		break; //pgdn
	case 0x115:
	case 0x100:
		return (0x52);
		break; //ins
	case 0x7F:
	case 0x10A:
		return (0x53);
		break; //del
	default:
		return (0);
	}
}

//void SDLHostSystemInterface::tick(VM& vm)
void SDLHostSystemInterface::tick()
{
	SDL_Event event;
	int mx = 0, my = 0;
	int8_t xrel, yrel;

	/*
	SDL_Keymod modstates = SDL_GetModState();
	if (modstates | (KMOD_LCTRL | KMOD_LALT))  {
		SDL_SetWindowGrab(frameBufferInterface.appWindow, SDL_FALSE);
		SDL_SetRelativeMouseMode(SDL_FALSE);
		SDL_ShowCursor(SDL_ENABLE);
	}
	*/

	if (SDL_PollEvent(&event)) 
	{
		switch (event.type) {
		case SDL_KEYDOWN:
			//log(Log,"[SDL] SDL_KEYDOWN %lu", event.key.keysym.scancode);
			
			vm->input.handleKeyDown(usb2xtMapping[event.key.keysym.scancode]);

			sdlconsole_ctrl = (event.key.keysym.sym == SDLK_LCTRL);
			sdlconsole_alt = (event.key.keysym.sym == SDLK_LALT);
			//if (event.key.keysym.sym == SDLK_TAB && !event.repeat) {
			if (sdlconsole_ctrl) {
				SDL_SetWindowGrab(frameBufferInterface.sdlWindow, SDL_FALSE);
				SDL_SetRelativeMouseMode(SDL_FALSE);
				SDL_ShowCursor(SDL_ENABLE);
			}

			//SDL_Keymod modstates = SDL_GetModState();
			//if (modstates && KMOD_LCTRL) {
			//	SDL_SetWindowGrab(frameBufferInterface.appWindow, SDL_FALSE);
			//	SDL_ShowCursor(SDL_ENABLE);
			//}

			break;
		case SDL_KEYUP:
			vm->input.handleKeyUp(usb2xtMapping[event.key.keysym.scancode]);
			break;
		case SDL_MOUSEBUTTONDOWN:
			SDL_SetWindowGrab(frameBufferInterface.sdlWindow, SDL_TRUE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
			SDL_ShowCursor(SDL_DISABLE);
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				vm->mouse.handleButtonDown(SerialMouse::ButtonType::Left);
			}
			else if(event.button.button == SDL_BUTTON_RIGHT)
			{
				vm->mouse.handleButtonDown(SerialMouse::ButtonType::Right);
			}
			// TODO grab mouse
			break;
		case SDL_MOUSEBUTTONUP:
			if (SDL_GetWindowGrab(frameBufferInterface.sdlWindow) == SDL_FALSE) break;
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				vm->mouse.handleButtonUp(SerialMouse::ButtonType::Left);
			}
			else if (event.button.button == SDL_BUTTON_RIGHT)
			{
				vm->mouse.handleButtonUp(SerialMouse::ButtonType::Right);
			}
			break;
		case SDL_MOUSEMOTION:
			if (SDL_GetWindowGrab(frameBufferInterface.sdlWindow) == SDL_FALSE) break;
			xrel = (event.motion.xrel < -128) ? -128 : (int8_t)event.motion.xrel;
			xrel = (event.motion.xrel > 127) ? 127 : (int8_t)event.motion.xrel;
			yrel = (event.motion.yrel < -128) ? -128 : (int8_t)event.motion.yrel;
			yrel = (event.motion.yrel > 127) ? 127 : (int8_t)event.motion.yrel;
			vm->mouse.handleMove((int8_t)xrel, (int8_t)yrel);
			
			//SDL_GetRelativeMouseState(&mx, &my);
			//vm.mouse.handleMove((int8_t)mx, (int8_t)my);
			//SDL_WarpMouse(frameBufferInterface.getWidth() / 2, frameBufferInterface.getHeight() / 2);
			break;
		case SDL_QUIT:
			vm->running = false;
			break;
		default:
			break;
		}
	}
}

void SDLHostSystemInterface::updatetitle() {
	char strbuff[64];
	if (vm->config.cpuSpeed == 0) {
  	snprintf(strbuff, sizeof(strbuff), "%s (Speed Auto Full)", BUILD_STRING);
	} else {
		snprintf(strbuff, sizeof(strbuff), "%s (Speed %u%s)", BUILD_STRING, vm->config.cpuSpeed, "Mhz");
	}
	SDL_SetWindowTitle(sdlWindow, strbuff);
}

void SDLHostSystemInterface::setcolormode(uint8_t _mode) {
	frameBufferInterface.SetColorEmulation(_mode);
}

void SDLHostSystemInterface::sendkeydown(uint8_t scancode) {
	vm->input.handleKeyDown(usb2xtMapping[scancode]);
}

void SDLHostSystemInterface::sendkeyup(uint8_t scancode) {
	vm->input.handleKeyUp(usb2xtMapping[scancode]);
}

void SDLHostSystemInterface::setrendermode(uint8_t _mode) {
	switch(_mode) {
		case RENDER_QUALITY_NEAREST:
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
			break;
		case RENDER_QUALITY_LINEAR:
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
			break;
		case RENDER_QUALITY_BEST:
		default:
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
	}
}


void Faux86::log(Faux86::LogChannel channel, const char* message, ...)
{
	const bool enableLogRaw = false;

	if (channel == LogRaw && !enableLogRaw)
		return;

	va_list myargs;
	va_start(myargs, message);
	vprintf(message, myargs);
	va_end(myargs);

	if (channel != LogRaw)
	{
		printf("\n");
	}
}
