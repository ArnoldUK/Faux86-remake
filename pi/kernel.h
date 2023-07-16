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
#ifndef _kernel_h
#define _kernel_h

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/bcmframebuffer.h> //FrameBuffer Device set max_framebuffers=N in config.txt for multiple FrameBuffers
#include <circle/2dgraphics.h> //2D Graphics Renderer
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <SDCard/emmc.h>
#include <circle/types.h>
//#include <circle/usb/dwhcidevice.h>
#include <circle/usb/usbhcidevice.h> //CUSBHCIDevice Alias for CDWHCIDevice and CXHCIDevice
#include <vc4/vchiq/vchiqdevice.h>
#include <circle/sched/scheduler.h>

#ifdef USE_FATFS
#include <fatfs/ff.h> //Generic FAT Filesystem Module (by ChaN)
#else
#include <circle/fs/fat/fatfs.h> //CFatFileSystem Native filesystem with FAT16 and FAT32 support using C++ classes
#endif

#ifdef RASPPI
	#if RASPPI == 1
		#define RASPPI1
	#elif RASPPI == 2
		#define RASPPI2
	#elif RASPPI == 3
		#define RASPPI3
	#elif RASPPI == 4
		#define RASPPI4
	#else
		#error "TARGET RPI MODEL NOT SUPPORTED"
	#endif
#else
	#error "RASPPI UNDEFINED TARGET RPI MODEL NOT KNOWN"
#endif

//#define FATFS_PARTITION						"umsd1"
#define FATFS_PARTITION						"emmc1-1"
#define FATFS_DRIVE								"SD:/"
#ifdef RASPPI1
	#define VM_SETTINGS_FILE					"faux86-1.cfg"
#elif defined RASPPI2
	#define VM_SETTINGS_FILE					"faux86-2.cfg"
#elif defined RASPPI3
	#define VM_SETTINGS_FILE					"faux86-3.cfg"
#elif defined RASPPI4
	#define VM_SETTINGS_FILE					"faux86-4.cfg"
#else
	#define VM_SETTINGS_FILE					"faux86.cfg"
#endif

#define USE_SCREEN_DEVICE					1
#define USE_FAKE_FRAMEBUFFER			0
#define USE_EMBEDDED_BOOT_DISK		0
#define USE_EMBEDDED_ROM_IMAGES		0
#define USE_MMC_MOUNTING					1
#define USE_SERIAL_LOGGING				1
#define USE_PWM_SOUND							0
#define USE_VCHIQ_SOUND						1

enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot
};

namespace Faux86
{
	class VM;
	class Config;
	class Settings;
	class CircleHostInterface;
}

class CKernel
{
public:
	CKernel (void);
	~CKernel (void);
	static CKernel* Get(void);

	boolean Initialize (void);

	TShutdownMode Run (void);


private:
	// do not change this order
	//CMemorySystem			m_Memory; //depreciated now automatically static created
	CActLED							m_ActLED; //Onboard LED
	CKernelOptions			m_Options; //Provides command line options from cmdline.txt
	CDeviceNameService	m_DeviceNameService; //Maps device names to a pointer to the device object
	//#if USE_SCREEN_DEVICE
	CScreenDevice				m_Screen; //Send serial output to the screen display (FrameBuffer)
	//#endif
	//C2DGraphics					m_2DGraphics; //2D Graphics Renderer
	CSerialDevice				m_Serial; //Access to the serial interface (UART)
	CExceptionHandler		m_ExceptionHandler; //Reports system faults (abort exceptions) for debugging
	CInterruptSystem		m_Interrupt; //Interrupt (IRQ and FIQ) handling
	CTimer							m_Timer; //Provides several time services
	CLogger							m_Logger; //System logging facility
	CScheduler					m_Scheduler; //Scheduler and Tasks service
	//CDWHCIDevice				m_DWHCI;
	CUSBHCIDevice				m_USBHCI; //Alias for CDWHCIDevice and CXHCIDevice
	#if USE_MMC_MOUNTING
	CEMMCDevice					m_EMMC;
	#endif
	CVCHIQDevice				m_VCHIQ;
	
	#ifdef USE_FATFS
	FATFS								m_FileSystem; //Generic FAT Filesystem Module (by ChaN) 
	#else
	CFATFileSystem			m_FileSystem; //Circle Native filesystem subsystem with FAT16 and FAT32 support using C++ classes
	CDevice 						*m_DiskPartition;
	#endif
	
	//CBcmFrameBuffer 	m_pFrameBuffer;
	//CFATFileSystem		m_FileSystem;
	Faux86::CircleHostInterface		*HostInterface;
	Faux86::Config								*vmConfig;
	Faux86::Settings							*vmSettings;
	Faux86::VM										*vm;
	
	static CKernel								*instance;
};

#endif
