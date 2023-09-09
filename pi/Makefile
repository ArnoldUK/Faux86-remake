#
# Makefile Usage:
# make clean
# make hex
# make flash
# screen -L -Logfile serial.log /dev/ttyUSB0 115200
#

CIRCLEHOME = ../../circle
SRCDIR = ../src
#OBJDIR = ../src/obj
#_OBJS	= $(patsubst %.c, %.o, $(SRC))
#OBJS	= $(addprefix $(OBJDIR), $(_OBJS))

#INCLUDE += ../src

# Define USE_FATFS to use /addon/fatfs
# Default uses the /circle/lib/fs/fat/fatfs 
USE_FATFS = 1

OBJS	=  kernel.o main.o settings.o CircleHostInterface.o PWMSound.o VCHIQSound.o \
	  $(SRCDIR)/Adlib.o \
	  $(SRCDIR)/Audio.o \
	  $(SRCDIR)/Config.o \
	  $(SRCDIR)/CPU.o \
	  $(SRCDIR)/Debugger.o \
	  $(SRCDIR)/DisneySoundSource.o \
	  $(SRCDIR)/DMA.o \
	  $(SRCDIR)/DriveManager.o \
	  $(SRCDIR)/InputManager.o \
	  $(SRCDIR)/MemUtils.o \
	  $(SRCDIR)/opl3.o \
	  $(SRCDIR)/PCSpeaker.o \
	  $(SRCDIR)/PIC.o \
	  $(SRCDIR)/PIT.o \
	  $(SRCDIR)/Ports.o \
	  $(SRCDIR)/Ram.o \
	  $(SRCDIR)/Renderer.o \
	  $(SRCDIR)/SerialMouse.o \
	  $(SRCDIR)/SoundBlaster.o \
	  $(SRCDIR)/TaskManager.o \
	  $(SRCDIR)/Timing.o \
		$(SRCDIR)/UART.o \
	  $(SRCDIR)/Video.o \
	  $(SRCDIR)/VM.o

#LIBS	= $(CIRCLEHOME)/lib/usb/libusb.a \
#	  $(CIRCLEHOME)/lib/input/libinput.a \
#	  $(CIRCLEHOME)/lib/fs/libfs.a \
#	  $(CIRCLEHOME)/lib/libcircle.a

LIBS	= $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	  $(CIRCLEHOME)/addon/vc4/sound/libvchiqsound.a \
	  $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
	  $(CIRCLEHOME)/addon/linux/liblinuxemu.a \
	  $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
		$(CIRCLEHOME)/addon/Properties/libproperties.a \
	  $(CIRCLEHOME)/lib/sched/libsched.a \
	  $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/sound/libsound.a \
	  $(CIRCLEHOME)/lib/libcircle.a
 
#ifeq ($(strip $(USE_FATFS)),1)
#LIBS	+= $(CIRCLEHOME)/addon/fatfs/libfatfs.a
#else
#LIBS	+= $(CIRCLEHOME)/lib/fs/fat/libfatfs.a
#endif

include ./Config.mk

# compiler flags
# -fsyntax-only
# -finline-functions
# all warnings flaged as errors
# -Werror
# -nostdlib

# ARM compile optimizations
# vfp is alias for vfpv2
# neon is alias for neon-vfpv3
# Raspberry Pi 1 	-mcpu=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp
# Raspberry Pi 2   	-mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4                      
# BeagleBone Black  	-mcpu=cortex-a8 -mfloat-abi=hard -mfpu=neon
# Altera Cyclone V5 	-mcpu=cortex-a9 -mfloat-abi=hard -mfpu=neon
# Raspberry Pi 3 	-mcpu=cortex-a53 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits
# Amazon EC2 A1 	-mcpu=cortex-a72 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits
# Raspberry Pi 4 	-mcpu=cortex-a72 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits

# GCC compiler tuning options for popular ARM-based boards:
# Board 	GCC extra tuning flags
# Raspberry Pi 1 	-mtune=arm1176jzf-s
# Raspberry Pi 2   	-mtune=cortex-a7      
# BeagleBone Black  	-mtune=cortex-a8
# Altera Cyclone V5 	-mtune=cortex-a9      
# Raspberry Pi 3 	-mtune=cortex-a53
# Raspberry Pi 4 	-mtune=cortex-a72

# Compiler optimizations
# Faster performance
# -O2, -O3, -Ofast, -Omax
# Balanced code size reduction and fast performance
# -Os

# CPPFLAGS = -I$(SRCDIR) -Wall -Werror -Wfatal-errors -Wno-unused-variable -O3
# CFLAGS = -I$(SRCDIR) -Wall -Werror -Wfatal-errors -Wno-unused-variable -O3

CPPFLAGS = -I$(SRCDIR) -Wall -Wfatal-errors -Wno-unused-variable -Wno-maybe-uninitialized -Wno-unused-but-set-variable -Os
CFLAGS = -I$(SRCDIR) -Wall -Wfatal-errors -Wno-unused-variable -Wno-maybe-uninitialized -Wno-unused-but-set-variable -Os

ifeq ($(strip $(RASPPI)),1)
CPPFLAGS += -marm -mcpu=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp -mtune=arm1176jzf-s
CFLAGS += -marm -mcpu=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp -mtune=arm1176jzf-s
else ifeq ($(strip $(RASPPI)),2)
CPPFLAGS += -marm -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4
CFLAGS += -marm -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4
else ifeq ($(strip $(RASPPI)),3)
CPPFLAGS += -marm -mcpu=cortex-a53 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits
CFLAGS += -marm -mcpu=cortex-a53 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits
else ifeq ($(strip $(RASPPI)),4)
CPPFLAGS += -marm -mcpu=cortex-a72 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits
CFLAGS += -marm -mcpu=cortex-a72 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits
else
$(error RASPPI must be set to 1,2,3 or 4)
endif

ifeq ($(strip $(USE_FATFS)),1)
CFLAGS += -DUSE_FATFS
CPPFLAGS += -DUSE_FATFS
endif

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)

# Function used to check variables ( make show VARNAME )
show-%: ; @echo $*=$($*)

hex: $(TARGET).img
	@echo "  COPY  $(TARGET).hex"
	@$(OBJCOPY) $(TARGET).elf -O ihex $(TARGET).hex

clean:
	$(RM) *.o $(SRCDIR)/*.o \
	$(RM) *.d $(SRCDIR)/*.d \

libs:
	make -C $(CIRCLEHOME)/addon/SDCard/ 
	make -C $(CIRCLEHOME)/addon/vc4/sound/
	make -C $(CIRCLEHOME)/addon/vc4/vchiq/
	make -C $(CIRCLEHOME)/addon/linux/
	make -C $(CIRCLEHOME)/addon/fatfs/
	make -C $(CIRCLEHOME)/addon/Properties/
	make -C $(CIRCLEHOME)/lib/sched/
	make -C $(CIRCLEHOME)/lib/usb/
	make -C $(CIRCLEHOME)/lib/input/
	make -C $(CIRCLEHOME)/lib/fs/
	make -C $(CIRCLEHOME)/lib/sound/
	make -C $(CIRCLEHOME)/lib/

