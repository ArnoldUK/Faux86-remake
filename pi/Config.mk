# The RASPPI value for each model set in the Makefile:
# RASPPI 	Target 	Models 	Optimized for
# 1 	kernel.img 	A, B, A+, B+, Zero, (CM) 	ARM1176JZF-S
# 2 	kernel7.img 	2, 3, Zero 2, (CM3) 	Cortex-A7
# 3 	kernel8-32.img 	3, Zero 2, (CM3) 	Cortex-A53
# 4 	kernel7l.img 	4B, 400, CM4 	Cortex-A72

RASPPI=1
AARCH=32
PREFIX=arm-none-eabi-
CLANG=0
MULTICORE=0
KEYMAP=UK
QEMU=0
CPP17=0
FORCE=0

# improved timing
#REALTIME=1
#NO_USB_SOF_INTR=1
#USE_USB_SOF_INTR=1

DEFINE += -DNO_SDHOST
DEFINE += -DNO_BUSY_WAIT
DEFINE += -DNO_REALTIME
DEFINE += -DNO_USB_SOF_INTR
#DEFINE += -DNO_SD_HIGH_SPEED
#DEFINE += -DUSE_USB_FIQ


# STDLIB_SUPPORT=0 to build without external libs (old setting)
# STDLIB_SUPPORT=1 to build with libgcc.a only (default now)
# STDLIB_SUPPORT=2 to build with C standard library (external newlib) too
# STDLIB_SUPPORT=3 to build with C++ standard library too
# STDLIB_SUPPORT=2

# Screen Display Color Depth
# 8bit 16bit or 32bit
# DEFINE += -DDEPTH=16

# Maximum 2MB (Default) 4MB 8MB 16MB 32MB Kernel Heap Size
# Recompile Circle after changing size 
# DEFINE += -DKERNEL_MAX_SIZE=0x200000
# DEFINE += -DKERNEL_MAX_SIZE=0x400000
# DEFINE += -DKERNEL_MAX_SIZE=0x800000
# DEFINE += -DKERNEL_MAX_SIZE=0x1000000
DEFINE += -DKERNEL_MAX_SIZE=0x2000000

# Bootloader Serial Flash
# SERIALPORT=COM4
SERIALPORT=/dev/ttyUSB0
FLASHBAUD=115200
# FLASHBAUD=230400
# FLASHBAUD=460800
USERBAUD=115200
