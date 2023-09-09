BOOTLOADER

The Circle project has an integrated serial bootloader, which can be used to
speed-up the development process and to make it more comfortable. The bootloader
has been adapted from the well-known bootloader by David Welch.

To use the bootloader, you have to do the following:

1. You need a serial USB adapter (3.3V level) connected to GPIO14/15 (Broadcom
numbering) of your Raspberry Pi computer on one side and to your development
machine on the other.

2. On your development machine there must be a python3 interpreter (including the
python3-pyserial module) installed. You may also need the terminal program
"putty".

3. Add the following lines to the file Config.mk in the Circle root directory:

	SERIALPORT = /dev/ttyUSB0	(device of your serial USB adapter)
	FLASHBAUD = 115200		(baud rate for loading the application)
	USERBAUD = 115200		(baud rate used by the application)

By increasing FLASHBAUD you can speed-up to loading process very much. It
depends on your serial adapter, which baud rates are possible. Common values are
460800, 921600 or higher.

USERBAUD is the baud rate, used by the application itself, if it communicates
using the serial interface.

4. The Circle project libraries used in your application have to be build
manually using "./makeall" as described in the main README.md file.

5. You have to prepare a SD card, which starts your Raspberry Pi in bootloader
mode. Go to the boot/ subdirectory and enter:

	make all

After completion copy the files from boot/ to the SD card (copy the file
"config.txt" only for AArch64 operation), which must have a FAT partition.
Please note, that you have to repeat this step, if you change the parameter
FLASHBAUD in Config.mk. The built bootloader is specific for this baud rate. Put
the SD card into your Raspberry Pi, which is connected to your development
machine and power it on.

6. The bootloader starts when you go to the subdirectory of your application (or
sample program) in a shell and enter:

	make flash (creates kernel.hex file and sends over the serial port)
	make cat (dumps serial output to the console window)

7. If your application uses the serial interface itself, you can start the
terminal program "putty" directly from the shell. You have to enter "make
monitor" like you have done for "flash" before and putty should open with the
right communication parameters.

8. To start another development cycle, power off and on the Raspberry Pi, and
after rebuilding do again "make flash".

Connect to bootloader over serial connection
--------------------------------------------
You can connect the Raspberry Pi to a PC using a USB-Serial cable.
Console connection serial parameters:
    Speed (baud rate): 115200
    Bits: 8
    Parity: None
    Stop Bits: 1
    Flow Control: None

NOTE FOR RASPBERRY PI 3:
The Raspberry pi 3 may require the option "enable_uart=1" in /boot/config.txt

Windows users can use the following console commands to transfer the kernel.hex file.
Setup the COM port with the serial parameters above. Use the correct COM port assigned
to your USB-Serial cable in device manager.

Recieve data from COM port:
type \\.\COM4 >> "data.txt"

Generate .hex file from kernel.elf
arm-none-eabi-objcopy kernel.elf -O ihex kernel.hex

mode COM4 /STATUS
mode COM4: BAUD=115200 PARITY=n DATA=8
rem set /p x="magicstring" <nul >\\.\COM4
set /p x="R" <nul >\\.\COM4
copy /A kernel.hex COM4
echo g > \\.\COM4

Linux users can use terminal for communicating over the serial connection.

Built-in (standard) Serial Ports: /dev/ttyS0, /dev/ttyS1
USB Serial Ports: /dev/ttyUSB0, /dev/ttyUSB1
Some USB Serial Adapters may appear as: /dev/ttyACM0

1. Check the tty USB device is available:
# ls -l /dev/ttyUSB0

2. Check the user is a member of the "dialout" group or add the user to the group if not.
# id
# sudo usermod -a -G dialout username

3. Choose the method to upload the kernel.hex firmware.

Super Easy Way Using GNU Screen (use CTRL+A then k to exit):
# screen /dev/ttyUSB0 115200

Super Easy Way Using Minicom (use CTRL+A then x to exit):
# minicom -b 115200 -o -D Port_Name


Using the New New Flash Tool "Flashy"
-------------------------------------

The above procedure describes flashing the device using the traditional python3 based flash
tool and bootloader.  There is also a new flash tool and bootloader that provide improved 
features:

* The new flash tool waits for ack messages from the bootloader to ensure it's ready. If 
  the device isn't ready it will wait until you reset it.

* Improved reliability as the new bootloader now performs checksums and reports any errors 
  with the transfer.

* A new "fast transfer mode" effectively doubles the transfer rate by sending binary instead
  of hex encoded data.  RPi 2 can handle 2M Baud in fast mode (equivalent to 4M Baud with 
  the old flasher) and RPi 3 can handler 3M baud (equivalent to 6M).

* The new flasher tool can send magic reboot strings and wait for the device to become ready 
  before starting the transfer - which can be faster than pessimistic delays and hoping the
  device is ready.

* After flashing the new flasher tool can automatically switch into monitor mode (switching 
  baud rates if necessary) to view the output of the flashed program.

* Ability to reset the bootloader to recover from a previously cancelled or failed transfer.

* Ability to introduce a "go delay" - a delay that the bootloader will stall for between 
  receiving the go command and launching the user program.  This can be used if you need
  to see the very start of the debug log and your monitor program takes too long to start.


The new flasher tool is written in JavaScript and requires NodeJS to be installed.  To use it:

1. Make sure you have NodeJS installed

2. Go to the `tools/flashy` sub-folder and run `npm install` to install the required 
   serial port module.

3. If you're currently using an old version of the bootloader, rebuild the latest version and
   copy it to the SD card. (This is optional as the new flash script works with the old 
   bootloader but most new features won't work)

4. In your Config.mk set the variable `USEFLASHY`, along with the other serial port settings:

        USEFLASHY = 1
        REBOOTMAGIC = <magicstring>
        FLASHYFLAGS = <other command line args for the flashy script>
        SERIALPORT = /dev/ttyUSB0
        FLASHBAUD = 115200
        USERBAUD = 115200

5. Flash the device as per before:

        make flash

6. If the device is ready the transfer will start immediately, otherwise a "waiting for device"
   message will be displayed until you power cycle or reset the device.

You can also manually launch the flasher tool like so...

    node tools/flashy/flashy.js /dev/ttyUSB0 kernel7.hex --flashBaud:2000000

Or, pass additional args via the make script:

    make flash FLASHYFLAGS=--godelay:500

For a full set of command line args, run the script with --help

    node tools/flashy/flashy.js --help

For details on using this under WSL, see the [Windows build instructions](windows-build.txt)

If you want to use Flashy with the Bluetooth modules HC-05 or HC-06, please see
[these instructions](https://github.com/rsta2/circle/pull/312).
