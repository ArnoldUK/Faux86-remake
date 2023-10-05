# Faux86 Emulator ARM RPi Build
The Faux86-remake source code can be compiled using standard linux and arm build tools.
For compiling and targeting ARM RPi builds the [Circle SDK](https://github.com/rsta2/circle/tree/develop) from the develop branch is required.
The instructions below for cloning and compiling the ARM RPi build were provided by [moononournation](https://github.com/moononournation)

For ARM RPi builds a custom settings file can be used for each model which is performance optimized for
that particular RPi model. The following settings file will be used for each build:

- faux86.cfg      For Windows/Linux binary builds.
- faux86-1.cfg    For RPi Model 1 Kernel.img
- faux86-2.cfg    For RPi Model 2 Kernel7.img
- faux86-3.cfg    For RPi Model 3 kernel8-32.img
- faux86-4.cfg    For RPi Model 4 kernel7l.img

## Github Clone Source
```
git clone -b develop https://github.com/rsta2/circle.git
git clone https://github.com/ArnoldUK/Faux86-remake.git
```

## Install build Toolchain
```
apt update && apt install -y make gcc-arm-linux-gnueabihf gcc-arm-none-eabi
```

## Setup Config Files
- Edit Faux86-remake/pi/Config.mk (Set RPi Model 1-4 in line RASPPI=3)
- Copy Faux86-remake/pi/Config.mk to Circle SDK root folder

## Edit Circle SDK Config Rules
- Edit Circle SDK `Rules.mk` file and change lines to match as below:
```
-include $(CIRCLEHOME)/Config.mk
#-include $(CIRCLEHOME)/Config2.mk	# is not overwritten by "configure"
```

## Compile and Build Circle SDK
- Replace 3 with the target RPi Model 1-4 in the first line below.
```
cd /git/circle/ && ./configure -f -r 3 && ./makeall
cd /git/circle/addon/SDCard && make
cd /git/circle/addon/vc4/sound && make
cd /git/circle/addon/vc4/vchiq && make
cd /git/circle/addon/linux && make
cd /git/circle/addon/fatfs && make
cd /git/circle/addon/Properties && make
```

## Compile and Build Faux86
```
cd /git/Faux86-remake/pi && make clean && make libs && make
```
