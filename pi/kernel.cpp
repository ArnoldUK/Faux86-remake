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
#include "kernel.h"
#include <circle/string.h>
#include "CircleHostInterface.h"
#include "VM.h"
#include "CircleDeviceDisk.h"
#include "settings.h"

// Embedded disks / ROMS
#if USE_EMBEDDED_BOOT_FLOPPY
#include "../data/dosboot.h"
#endif
#if USE_EMBEDDED_ROM_IMAGES
#include "../data/asciivga.h"
#include "../data/pcxtbios.h"
#include "../data/videorom.h"
#endif

//#pragma GCC optimize("O2,inline")

#define REQUIRED_KERNEL_MEMORY (0x2000000)

#if KERNEL_MAX_SIZE < REQUIRED_KERNEL_MEMORY
//#error "Not enough kernel space: change KERNEL_MAX_SIZE in circle/sysconfig.h to at least 32 megabytes and recompile circle"
#endif

//#ifdef RASPPI1
//#error "RASPPI1 DEFINED"
//#endif

//#define AUDIO_SAMPLE_RATE 48000
//#define AUDIO_SAMPLE_RATE 44100
//#define AUDIO_SAMPLE_RATE 22050

static const char FromKernel[] = "kernel";

LOGMODULE("kernel");

CKernel *CKernel::instance;

using namespace Faux86;

CKernel::CKernel(void) :
	m_ActLED(false),
	//#if USE_SCREEN_DEVICE
	//m_Screen(640, 400, true),
	//m_Screen(640, 350),
	m_Screen(m_Options.GetWidth(), m_Options.GetHeight()),
	//#endif
	//m_2DGraphics(m_Options.GetWidth(), m_Options.GetHeight(), false),
	m_Timer(&m_Interrupt),
	m_Logger(m_Options.GetLogLevel(), &m_Timer),
	//m_DWHCI(&m_Interrupt, &m_Timer),
	m_USBHCI(&m_Interrupt, &m_Timer, FALSE),
	#if USE_MMC_MOUNTING
	m_EMMC(&m_Interrupt, &m_Timer, &m_ActLED),
	#endif
	//m_VCHIQ(&m_Memory, &m_Interrupt)
	m_VCHIQ(CMemorySystem::Get(), &m_Interrupt)
	//CircleHostInterface(&m_Screen, &m_2DGraphics, &FatFsInterface, &AudioInterface)
{
	instance = this;
	m_ActLED.Blink(3);
}

CKernel::~CKernel(void)
{
}

CKernel* CKernel::Get(void) {
	return instance;
}

boolean CKernel::Initialize(void)
{
	boolean bOK = TRUE;
	
	//log(Log, "[KERNEL] CKernel::Initialize");
	
	//#if USE_SCREEN_DEVICE
	if (bOK) {
		bOK = m_Screen.Initialize();
	}
	//#endif

	if (bOK) {
		//log(Log, "[KERNEL] Serial::Initialize");
		bOK = m_Serial.Initialize(115200);
	}

	if (bOK) {
		//log(Log, "[KERNEL] CDevice::Initialize");
		CDevice *pTarget = m_DeviceNameService.GetDevice(m_Options.GetLogDevice(), FALSE);
		if (pTarget == 0)	{
			//#if USE_SCREEN_DEVICE
			pTarget = &m_Screen;
			//#else
			//pTarget = &m_Serial;
			//#endif
		}

		bOK = m_Logger.Initialize(pTarget);
	}

	if (bOK) {
		log(Log, "[KERNEL] Interrupt::Initialize");
		bOK = m_Interrupt.Initialize();
	}

	if (bOK) {
		log(Log, "[KERNEL] Timer::Initialize");
		bOK = m_Timer.Initialize();
	}

	if (bOK) {
		log(Log, "[KERNEL] USBHCI::Initialize");
		//bOK = m_DWHCI.Initialize();
		bOK = m_USBHCI.Initialize();
	}

#if USE_MMC_MOUNTING	
	if (bOK) {
		log(Log, "[KERNEL] EMMC::Initialize");
		bOK = m_EMMC.Initialize();
	}
#endif

#if USE_VCHIQ_SOUND
	if (bOK) {
		log(Log, "[KERNEL] VCHIQ::Initialize");
		bOK = m_VCHIQ.Initialize();
	}
#endif

	if(bOK) {
		log(Log, "[KERNEL] mount filesystem");
		#ifdef USE_FATFS
		bOK = (f_mount(&m_FileSystem, FATFS_DRIVE, 1) == FR_OK);
		if (!bOK) log(Log, "[KERNEL] mount filesystem failed!");
		#else
		m_DiskPartition = m_DeviceNameService.GetDevice(FATFS_PARTITION, TRUE);
		if (m_DiskPartition == 0) {
			log(Log, "[KERNEL] mount disk partition %s failed!", FATFS_PARTITION);
			bOK = false;
		} else if (!m_FileSystem.Mount(pPartition))	{
			log(Log, "[KERNEL] mount filesystem on %s failed!", FATFS_PARTITION);
			bOK = false;
		}
		}
		#endif
	}

	if(bOK)	{
		log(Log, "[KERNEL] create circlehostinterface");
		//HostInterface = new CircleHostInterface(&m_Screen, &m_2DGraphics, &FatFsInterface, &AudioInterface)
		//HostInterface = new CircleHostInterface(m_DeviceNameService, m_Interrupt, m_VCHIQ, m_Screen, m_2DGraphics);
		HostInterface = new CircleHostInterface(m_DeviceNameService, m_Interrupt, m_VCHIQ, m_Screen);
		vmConfig = new Config(HostInterface);
		
		log(Log, "[KERNEL] loading emulation settings file %s", VM_SETTINGS_FILE);
		#ifdef USE_FATFS
		vmSettings = new Settings(&m_FileSystem, FATFS_DRIVE VM_SETTINGS_FILE);
		#else
		vmSettings = new Settings(&m_FileSystem, VM_SETTINGS_FILE);	
		#endif
		
		if (!vmSettings->Load()) {
			log(Log, "[KERNEL] Load settings file failed!.");
		}	else {
			log(Log, "[KERNEL] Dump emulation settings");
			vmSettings->Dump();
		}
		
		log(Log, "[KERNEL] loading emulation ROM files");

		#if USE_MMC_MOUNTING
		//vmConfig->diskDriveC = new CircleDeviceDisk(&m_EMMC);
		vmConfig->biosFile = HostInterface->openFile("SD:/pcxtbios.bin");
		vmConfig->romBasicFile = HostInterface->openFile("SD:/rombasic.bin");
		vmConfig->videoRomFile = HostInterface->openFile("SD:/videorom.bin");
		vmConfig->asciiFile = HostInterface->openFile("SD:/asciivga.dat");
    
		vmConfig->diskDriveA = HostInterface->openFile("SD:/fd0.img");
		vmConfig->diskDriveC = HostInterface->openFile("SD:/hd0.img");
		#else
		//vmConfig->diskDriveC = HostInterface->openFile("SD:/hd0.img");
		//CDevice *pUMSD1 = m_DeviceNameService.GetDevice ("umsd1", TRUE);
		//if(pUMSD1) {
		//	vmConfig->diskDriveD = new CircleDeviceDisk(pUMSD1);
		//}
		#endif

		#if USE_EMBEDDED_ROM_IMAGES
		if(!vmConfig->biosFile)
			vmConfig->biosFile = new EmbeddedDisk(pcxtbios, sizeof(pcxtbios));
		if(!vmConfig->videoRomFile)
			vmConfig->videoRomFile = new EmbeddedDisk(videorom, sizeof(videorom));
		if(!vmConfig->asciiFile)
			vmConfig->asciiFile = new EmbeddedDisk(asciivga, sizeof(asciivga));
    if(!vmConfig->romBasicFile)
			vmConfig->romBasicFile = new EmbeddedDisk(romBasicFile, sizeof(romBasicFile));
		#endif
		#if USE_EMBEDDED_BOOT_DISK
		if(!vmConfig->diskDriveA)
			vmConfig->diskDriveA = new EmbeddedDisk(dosboot, sizeof(dosboot));
		#endif

		//TODO FOR RPI
		//vmConfig->parseCommandLine(argc, argv);
		
		vmConfig->singleThreaded = !vmSettings->GetInt("multithreaded", 0);
		vmConfig->enableAudio = !vmSettings->GetInt("nosound", 0);
		vmConfig->useDisneySoundSource = vmSettings->GetInt("snddisney", 0);
		vmConfig->useSoundBlaster = vmSettings->GetInt("sndblaster", 0);
		vmConfig->useAdlib = vmSettings->GetInt("sndadlib", 0);
		vmConfig->usePCSpeaker = vmSettings->GetInt("sndspeaker", 1);
		vmConfig->slowSystem = vmSettings->GetInt("slowsys", 1);
		vmConfig->frameDelay = vmSettings->GetInt("delay", 120);
		vmConfig->audio.sampleRate = vmSettings->GetInt("samprate", 22050);
		vmConfig->audio.latency = vmSettings->GetInt("latency", 200);
		vmConfig->framebuffer.width = vmSettings->GetInt("fbwidth", 800);
		vmConfig->framebuffer.height = vmSettings->GetInt("fbheight", 800);
		vmConfig->resw = vmSettings->GetInt("resw", 640);
		vmConfig->resh = vmSettings->GetInt("resh", 350);
		vmConfig->renderQuality = vmSettings->GetInt("render", 1);
		vmConfig->monitorDisplay = vmSettings->GetInt("monitor", 0);
		vmConfig->cpuSpeed = vmSettings->GetInt("speed", 8);
		
		
		//override command parameters
		/*
		vmConfig->singleThreaded = true;
		vmConfig->enableAudio = true;
		vmConfig->useDisneySoundSource = false;
		vmConfig->useSoundBlaster = false;
		vmConfig->useAdlib = true;
		vmConfig->usePCSpeaker = true;
		vmConfig->slowSystem = true;
		vmConfig->frameDelay = 130; //200; //20ms 50fps
		vmConfig->audio.sampleRate = 22050;  //32000 //44100 //48000;
		vmConfig->audio.latency = 200; //100;
		vmConfig->framebuffer.width = 800; //640;
		vmConfig->framebuffer.height = 800; //480;
		vmConfig->resw = 640; //640
		vmConfig->resh = 350; //400
		vmConfig->renderQuality = 0;
		vmConfig->cpuSpeed = 8;
		*/
		
		
		
		#ifdef RASPPI1
		//RPI 1 Optimized with Sound
		/*
		vmConfig->slowSystem = true;
		vmConfig->cpuSpeed = 8;
		vmConfig->singleThreaded = true;
		vmConfig->enableAudio = true;
		vmConfig->useAdlib = true;
		vmConfig->audio.sampleRate = 22050; //32000;
		vmConfig->audio.latency = 160;
		vmConfig->frameDelay = 120;
		vmConfig->monitorDisplay = 0;
		*/
		#endif
		
		#ifdef RASPPI2
		//RPI 2 Optimized with Sound
		//vmConfig->slowSystem = 1;
		//vmConfig->cpuSpeed = 12;
		//vmConfig->frameDelay = 120;
		//vmConfig->audio.sampleRate = 32000;
		//vmConfig->audio.latency = 120;
		//vmConfig->useAdlib = true;
		#endif
		
		#ifdef RASPPI3
		//RPI 3 Optimized with Sound
		//vmConfig->slowSystem = 1;
		//vmConfig->cpuSpeed = 12;
		//vmConfig->frameDelay = 70;
		//vmConfig->audio.sampleRate = 32000;
		//vmConfig->audio.latency = 120;
		//vmConfig->useAdlib = true;
		#endif
		
		log(Log, "[KERNEL] Creating Virtual Machine");
		vm = new VM(*vmConfig);

		log(Log, "[KERNEL] Initializing Virtual Machine");
		bOK = vm->init();
		
		log(Log, "[KERNEL] Initializing Host Interface");
		HostInterface->init(vm);
	}
	
	return bOK;
}

#define TEST_STATIC_ARRAY 0
#define TEST_DYNAMIC_ARRAY 0

#if TEST_STATIC_ARRAY
constexpr int bigArraySize = 8 * 1024 * 1024;
unsigned char bigArray[bigArraySize] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
#endif


TShutdownMode CKernel::Run(void)
{
	m_Logger.Write (FromKernel, LogNotice, "[KERNEL] Compile time: " __DATE__ " " __TIME__);
	//m_Logger.Write(FromKernel, LogNotice, "Begin mem test");
	
#if TEST_DYNAMIC_ARRAY
	m_Logger.Write(FromKernel, LogNotice, "Alloc");
	unsigned long int size = 32 * 1024 * 1024;
	unsigned char* test = new unsigned char[size];
	assert(test);
	
	m_Logger.Write(FromKernel, LogNotice, "Write mem");
	for(unsigned long n = 0; n < size; n++)
	{
		test[n] = 0xFE;
	}

	m_Logger.Write(FromKernel, LogNotice, "Read mem");
	for(unsigned long n = 0; n < size; n++)
	{
		assert(test[n] == 0xFE);
	}

	m_Logger.Write(FromKernel, LogNotice, "Free");
	delete[] test;
#endif

#if TEST_STATIC_ARRAY
	m_Logger.Write(FromKernel, LogNotice, "Write static mem");
	for(unsigned long n = 0; n < bigArraySize; n++)
	{
		bigArray[n] = 0xFE;
	}

	m_Logger.Write(FromKernel, LogNotice, "Read static mem");
	for(unsigned long n = 0; n < bigArraySize; n++)
	{
		assert(bigArray[n] == 0xFE);
	}
#endif
	//m_Logger.Write(FromKernel, LogNotice, "End mem test");

	//#if USE_SCREEN_DEVICE
	unsigned int rcount = 0;
	//#endif
	
	while(true)
	{
		vm->simulate();
		//HostInterface->tick(*vm);
		HostInterface->tick();
		
		//#if USE_SCREEN_DEVICE
		//m_Screen.Rotor(0, rcount++);
		//#endif

		/*
		rcount++;
		if (rcount == 1000) {
			rcount = 0;
			//m_ActLED.Blink(1);
			log(Log, "[KERNEL] CKernel::Run 1000 loops");
		}
		*/
		
		//m_Scheduler.Yield();
		//m_Timer.MsDelay(1);
	}
	
	return ShutdownHalt;
}

