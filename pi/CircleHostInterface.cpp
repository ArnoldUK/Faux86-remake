/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
  Copyright (C)2010-2013 Mike Chambers

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
#include <circle/bcmframebuffer.h>
#include <circle/usb/usbkeyboard.h>
#include <circle/input/mouse.h>
#include <circle/devicenameservice.h>
#include <circle/string.h>
#include <circle/interrupt.h>
#include <circle/sched/scheduler.h>
#include <circle/util.h>
#include "CircleHostInterface.h"
#include "FatFsDisk.h"
#include "Keymap.h" 
#include "VM.h"
#include "kernel.h"
#include "PWMSound.h"
#include "VCHIQSound.h"

#include "MemUtils.h"

//#pragma GCC optimize("O2,inline")

extern uint32_t vga_framebuffer[800][800];

static unsigned uBlitLoops = 0;
static unsigned uChipsetloops = 0;
static unsigned uTimeStart, uTimeEnd = 0;
static unsigned uBlitCalls = 0;
static unsigned uCpuCalls = 0;

static unsigned GetMillis(void) {
	unsigned uticks = CTimer::Get()->GetClockTicks();
	if (uticks) return (uticks / 1000);
	return 0;
}

static unsigned BeginCodeTiming(void) {
	return GetMillis();
}

static unsigned EndCodeTiming(unsigned ustart) {
	return (GetMillis() - ustart);	
}

using namespace Faux86;

CircleHostInterface* CircleHostInterface::instance;

#if USE_FAKE_FRAMEBUFFER
uint8_t* fakeFrameBuffer = nullptr;
#endif

void Faux86::log(Faux86::LogChannel channel, const char* message, ...)
{
#if USE_SERIAL_LOGGING
	va_list myargs;
	va_start(myargs, message);
	
	//if(channel == Faux86::LogChannel::LogRaw)
	//{
	//	CString Message;
	//	Message.Format (message, myargs);
	//	CLogger::Get()->GetTarget()->Write ((const char *) Message, Message.GetLength ());
	//}
	//else
	{
		CLogger::Get()->WriteV("Faux86", LogNotice, message, myargs);
	}
	va_end(myargs);
#endif
}

void CircleFrameBufferInterface::init(uint32_t desiredWidth, uint32_t desiredHeight)
{	
	log(LogVerbose, "[CircleFrameBufferInterface] Initialized");
	#if USE_FAKE_FRAMEBUFFER
	log(LogVerbose, "Creating Fake Frame Buffer");
	surface = RenderSurface::create(640, 400);
  //surface = RenderSurface::create(640, 480);
	//log(LogVerbose, "Created at %x", (uint32_t)surface->pixels);
	
	for(int n = 0; n < 640 * 400; n++) {
  //for(int n = 0; n < 640 * 480; n++) {
		surface->pixels[n] = 0xcd;
	}
	//log(LogVerbose, "Cleared!");
	#else
	//frameBuffer = 
	//frameBuffer = new CBcmFrameBuffer (desiredWidth, desiredHeight, 8);
	//frameBuffer = new CBcmFrameBuffer (desiredWidth, desiredHeight, 16);
	//frameBuffer->Initialize();

	//surface = new RenderSurface();
	//surface->width = frameBuffer->GetWidth();
	//surface->height = frameBuffer->GetHeight();
	//surface->pitch = frameBuffer->GetPitch();
	//surface->pixels = (uint8_t*) frameBuffer->GetBuffer();
	#endif
}

void CircleFrameBufferInterface::resize(uint32_t desiredWidth, uint32_t desiredHeight)
{
	log(LogVerbose, "[CircleFrameBufferInterface] resize %lu x %lu", desiredWidth, desiredHeight);
	
	if(surface->width == desiredWidth && surface->height == desiredHeight) return;
	#if USE_FAKE_FRAMEBUFFER
	return;
	#endif
	
	//delete frameBuffer;
	//CTimer::Get()->SimpleMsDelay(1000);
	CTimer::Get()->SimpleMsDelay(500);
	
	//frameBuffer = new CBcmFrameBuffer(desiredWidth, desiredHeight, 8);
	//frameBuffer = new CBcmFrameBuffer (desiredWidth, desiredHeight, 16);
	//frameBuffer->Initialize();
	
	//surface->width = frameBuffer->GetWidth();
	//surface->height = frameBuffer->GetHeight();
	//surface->pitch = frameBuffer->GetPitch();
	//surface->pixels = (uint8_t*) frameBuffer->GetBuffer();

}

RenderSurface* CircleFrameBufferInterface::getSurface()
{ 
	return surface;
}

void CircleFrameBufferInterface::setPalette(Palette* palette)
{
	#if USE_FAKE_FRAMEBUFFER
	return;
	#endif
	
	log(LogVerbose, "[CircleFrameBufferInterface] setPalette");
	
	for(int n = 0; n < 256; n++) {
		uint32_t colour = (0xff << 24) | (palette->colours[n].r) | (palette->colours[n].g << 8) | (palette->colours[n].b << 16);
		frameBuffer->SetPalette32 (n, colour);
	}
	frameBuffer->UpdatePalette();
}

/*
void CircleFrameBufferInterface::present()
{
	
}
*/

//#define D3DCOLOR_ARGB(a,r,g,b) ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff))) 

static uint32_t VGAtoRGB(uint32_t c) {
	return (c |	(c << 8) | (c << 16));
}

static uint32_t RGBtoColor(uint32_t r, uint32_t g, uint32_t b) {
#ifdef __BIG_ENDIAN__
	return ( (r << 24) | (g << 16) | (b << 8) );
#else
	return (r | (g << 8) | (b << 16) );
#endif
}

static uint16_t RGB32toRGB16(uint32_t _pixel) {
	unsigned int red = (_pixel & 0x00FF0000) >> 16;
	unsigned int green = (_pixel & 0x0000FF00) >> 8;
	unsigned int blue =  _pixel & 0x000000FF;
	return (red >> 3 << 11) + (green >> 2 << 5) + (blue >> 3);
}

static uint16_t RGB32toRGB16_2(uint32_t _pixel) {
	unsigned int alpha = (_pixel & 0x000000F0) >> 4;
	unsigned int red = (_pixel & 0x0000F000) >> 8;
	unsigned int green = (_pixel & 0x00F00000) >> 12;
	unsigned int blue =  (_pixel & 0xF0000000) >> 16;
	return alpha | red | green | blue;
}

static unsigned g_seed;
static unsigned randomNumber(void) {
	g_seed = (214013*g_seed+2531011);
	return (g_seed>>16)&0x7fff;
}

void CircleFrameBufferInterface::blit(uint32_t *pixels, int w, int h, int stride)
{
	//renderer.fb->setPalette(vm.video.getCurrentPalette());
	//if (screenModeChanged)
	//{
	//	fb->resize(nativeWidth, nativeHeight);
	//	createScaleMap();
	//	screenModeChanged = false;
	//}
	
	//uTimeStart = BeginCodeTiming();
	
	uint32_t sw = frameBuffer->GetWidth();
	uint32_t sh = frameBuffer->GetHeight();
	//uint32_t sp = frameBuffer->GetPitch();
	
	//TScreenColor *pScreenBuffer = (TScreenColor *)(uintptr)p2DGraphics->GetBuffer();
	TScreenColor *pScreenBuffer = (TScreenColor *)(uintptr)frameBuffer->GetBuffer();
	TScreenColor sColor;
	//uint32_t vidptr;
	
	//memcpy(pScreenBuffer, pixels, 256000);
	
	//uint8_t cRed = randomNumber() % 31;
	//uint8_t cGreen = randomNumber() % 31;
	//uint8_t cBlue = randomNumber() % 31;
	/*
	for (int y = 0; y < h; y++) { //sh
		//MemUtils::memcpy(surface->pixels + (y * surface->pitch), pixels + (y * stride), surface->width);
		//memcpy(surface->pixels + (y * surface->pitch), pixels + (y * stride), surface->width);
		//memcpy(pScreenBuffer + (y * sp), pixels + (y * stride), sw);
		//pScreenBuffer[y] = y * 10;
		//MemUtils::memcpy((uint8_t*)pScreenBuffer + sp * y, (uint8_t*)pixels + stride * y, stride);
		//memcpy((uint8_t*)pScreenBuffer + sp * y, (uint8_t*)pixels + stride * y, stride);
		//for (int i = 0; i < sp; i++) {
		for (int i = 0; i < w; i++) { //sw
			uint32_t vs = *pixels; //*pixels + (y * (stride/sizeof(uint32_t)));
			sColor = RGB32toRGB16(vs); //vs & 0xFF0000FF;  //RGB32toRGB16(vs);
			//sColor = COLOR16(cRed, cGreen, cBlue);
      pScreenBuffer[i + (y * sw)] = sColor;
			pixels++;
    }
		//CScheduler::Get()->Yield();
	}
	CScheduler::Get()->Yield();
	return;
	*/
	
	//uint32_t _pixel;
	//unsigned int red, green, blue;
	//int x, y;
	for (int y = 0; y < h; y++) { //400
		for (int x = 0; x < w; x++) { //640
			//vidptr = y * 640 + x;
			//sColor = pixels[y+x]; //RGB32toRGB16(pixels[y][x]);
			//sColor = COLOR16(cRed, cGreen, cBlue);
			sColor = RGB32toRGB16(vga_framebuffer[y][x]);
			//sColor = TScreenColor(vga_framebuffer[y][x]);
			pScreenBuffer[x + (y * sw)] = sColor;
			/*
			_pixel = vga_framebuffer[y][x];
			red = (_pixel & 0x00FF0000) >> 16;
			green = (_pixel & 0x0000FF00) >> 8;
			blue =  _pixel & 0x000000FF;
			pScreenBuffer[x + (y * sw)] = TScreenColor( (red >> 3 << 11) + (green >> 2 << 5) + (blue >> 3) );
			*/
			//screen->SetPizel(x, y, sColor);
			//p2DGraphics->DrawPixel(x, y, sColor);
		}
		//CScheduler::Get()->Yield();
	}
	//CScheduler::Get()->Yield();
	//CScheduler::Get()->MsSleep(1);
	
	/*
	uTimeEnd = EndCodeTiming(uTimeStart);
	uBlitCalls++;
	if (uBlitCalls == 10) {
		uBlitCalls = 0;
		log(Log, "[VIDEO] BLIT CALLS TIME [%u]", uTimeEnd);
	}
	*/

}

void CircleAudioInterface::init(VM& vm)
{
#if USE_PWM_SOUND
	pwmSound = new PWMSound(vm.audio, &interruptSystem, vm.audio.sampleRate);
	pwmSound->Start();
#endif
#if USE_VCHIQ_SOUND
	vchiqSound = new VCHIQSound(vm.audio, &vchiqDevice, vm.audio.sampleRate);
	vchiqSound->Start();
#endif
}

void CircleAudioInterface::shutdown()
{
	if(pwmSound) {
		pwmSound->Cancel();
		delete pwmSound;
	}
	if(vchiqSound) {
		vchiqSound->Cancel();
		delete vchiqSound;
	}
}

CircleTimerInterface::CircleTimerInterface()
{
	lastTimerSample = CTimer::GetClockTicks();
}

uint64_t CircleTimerInterface::getHostFreq()
{
	return CLOCKHZ;
}

uint64_t CircleTimerInterface::getTicks()
{
	uint32_t timerSample = CTimer::GetClockTicks();
	uint32_t delta = timerSample >= lastTimerSample ? (timerSample - lastTimerSample) : (0xffffffff - lastTimerSample) + timerSample;
	lastTimerSample = timerSample;
	currentTick += delta;
	return currentTick;
}

DiskInterface* CircleHostInterface::openFile(const char* filename)
{
	return FatFsDisk::open(filename);
} 

//CircleHostInterface::CircleHostInterface(CDeviceNameService& deviceNameService, CInterruptSystem& interruptSystem, CVCHIQDevice& inVchiqDevice, CScreenDevice& screenDevice, C2DGraphics 2DGraphics)
CircleHostInterface::CircleHostInterface(CDeviceNameService& deviceNameService, CInterruptSystem& interruptSystem, CVCHIQDevice& inVchiqDevice, CScreenDevice& screenDevice)
: audio(interruptSystem, inVchiqDevice)
{
	instance = this;
	
	screen = (CScreenDevice*)&screenDevice;
	//p2DGraphics = (C2DGraphics*)&2DGraphics;
	
	frameBuffer.frameBuffer = screen->GetFrameBuffer();
	
	keyboard = (CUSBKeyboardDevice*)deviceNameService.GetDevice("ukbd1", FALSE);
	
	if(keyboard) {
		keyboard->RegisterKeyStatusHandlerRaw(keyStatusHandlerRaw, TRUE);
	}
	
	CMouseDevice* mouse = (CMouseDevice*)deviceNameService.GetDevice("mouse1", FALSE);
	
	if(mouse)	{
		mouse->RegisterStatusHandler(mouseStatusHandler);
	}
}

void CircleHostInterface::init(VM* inVM)
{
	vm = inVM;
	log(Log, "[CircleHostInterface] Initialized");
	//frameBuffer.frameBuffer = screen->GetFrameBuffer();
}

void CircleHostInterface::resize(uint32_t desiredWidth, uint32_t desiredHeight)
{
	log(Log, "[CircleHostInterface] resize %lu x %lu", desiredWidth, desiredHeight);
	if ( (screen->GetWidth() == desiredWidth) && (screen->GetHeight() == desiredHeight) ) return;
	screen->Resize(desiredWidth, desiredHeight);
	vm->config.resw = desiredWidth;
	vm->config.resh = desiredHeight;
}

//void CircleHostInterface::tick(VM& vm)
void CircleHostInterface::tick()
{
	while(inputBufferSize > 0)
	{
		//CScheduler::Get()->Yield();
		
		InputEvent& event = inputBuffer[inputBufferPos];
		
		switch(event.eventType)
		{
			case EventType::KeyPress:
			{
				vm->input.handleKeyDown(event.scancode);
			}
			break;
			case EventType::KeyRelease:
			{
				vm->input.handleKeyUp(event.scancode);
			}
			break;
			case EventType::MousePress:
			{
				vm->mouse.handleButtonDown(event.mouseButton);
			}
			break;
			case EventType::MouseRelease:
			{
				vm->mouse.handleButtonUp(event.mouseButton);
			}
			break;
			case EventType::MouseMove:
			{
				vm->mouse.handleMove(event.mouseMotionX, event.mouseMotionY);
			}
			break;
		}
		
		inputBufferPos++;
		inputBufferSize --;
		if(inputBufferPos >= MaxInputBufferSize) {
			inputBufferPos = 0;
		}
	}
}

void CircleHostInterface::queueEvent(InputEvent& inEvent)
{
	if(inputBufferSize < MaxInputBufferSize) {
		int writePos = (inputBufferPos + inputBufferSize) % MaxInputBufferSize;
		inputBuffer[writePos] = inEvent;
		instance->inputBufferSize ++;
	}
}

void CircleHostInterface::queueEvent(EventType eventType, u16 scancode)
{
	if(scancode != 0) {
		InputEvent newEvent;
		newEvent.eventType = eventType;
		newEvent.scancode = scancode;
		queueEvent(newEvent);
	}
}

void CircleHostInterface::mouseStatusHandler (unsigned nButtons, int nDisplacementX, int nDisplacementY, int nWheelMove)
{
	InputEvent newEvent;
	
	// Mouse presses
	if((nButtons & MOUSE_BUTTON_LEFT) && !(instance->lastMouseButtons & MOUSE_BUTTON_LEFT))
	{
		newEvent.eventType = EventType::MousePress;
		newEvent.mouseButton = SerialMouse::ButtonType::Left;
		instance->queueEvent(newEvent);
	}
	if((nButtons & MOUSE_BUTTON_RIGHT) && !(instance->lastMouseButtons & MOUSE_BUTTON_RIGHT))
	{
		newEvent.eventType = EventType::MousePress;
		newEvent.mouseButton = SerialMouse::ButtonType::Right;
		instance->queueEvent(newEvent);
	}
	
	// Mouse releases
	if(!(nButtons & MOUSE_BUTTON_LEFT) && (instance->lastMouseButtons & MOUSE_BUTTON_LEFT))
	{
		newEvent.eventType = EventType::MouseRelease;
		newEvent.mouseButton = SerialMouse::ButtonType::Left;
		instance->queueEvent(newEvent);
	}
	if(!(nButtons & MOUSE_BUTTON_RIGHT) && (instance->lastMouseButtons & MOUSE_BUTTON_RIGHT))
	{
		newEvent.eventType = EventType::MouseRelease;
		newEvent.mouseButton = SerialMouse::ButtonType::Right;
		instance->queueEvent(newEvent);
	}

	// Motion events	
	if(nDisplacementX != 0 || nDisplacementY != 0)
	{
		newEvent.eventType = EventType::MouseMove;
		newEvent.mouseMotionX = (s8) nDisplacementX;
		newEvent.mouseMotionY = (s8) nDisplacementY;
		instance->queueEvent(newEvent);
	}
	
	instance->lastMouseButtons = nButtons;
}

void CircleHostInterface::keyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])
{
	for(int n = 0; n < 8; n++) {
		int mask = 1 << n;
		bool wasPressed = (instance->lastModifiers & mask) != 0;
		bool isPressed = (ucModifiers & mask) != 0;
		if(!wasPressed && isPressed) {
			instance->queueEvent(EventType::KeyPress, modifier2xtMapping[n]);
		}
		else if(wasPressed && !isPressed)	{
			instance->queueEvent(EventType::KeyRelease, modifier2xtMapping[n]);
		}
	}
		
	for(int n = 0; n < 6; n++) {
		if(instance->lastRawKeys[n] != 0)	{
			bool inNewBuffer = false;
			
			for(int i = 0; i < 6; i++) {
				if(instance->lastRawKeys[n] == RawKeys[i]) {
					inNewBuffer = true;
					break;
				}
			}
			
			if(!inNewBuffer && instance->inputBufferSize < MaxInputBufferSize) {
				instance->queueEvent(EventType::KeyRelease, usb2xtMapping[instance->lastRawKeys[n]]);
			}
		}
	}

	for(int n = 0; n < 6; n++) {
		if(RawKeys[n] != 0)	{
			bool inLastBuffer = false;
			
			for(int i = 0; i < 6; i++) {
				if(instance->lastRawKeys[i] == RawKeys[n]) {
					inLastBuffer = true;
					break;
				}
			}
			
			if(!inLastBuffer && instance->inputBufferSize < MaxInputBufferSize)	{
				instance->queueEvent(EventType::KeyPress, usb2xtMapping[RawKeys[n]]);
			}
		}
	}

	for(int n = 0; n < 6; n++) {
		instance->lastRawKeys[n] = RawKeys[n];
	}

	instance->lastModifiers = ucModifiers;
}
