CMDLINE OPTIONS

Create a file cmdline.txt on the SD(HC) card used for booting. Multiple options
must be placed on a single line delimited by a space character. The following
options are available:

width=640 height=480		Changes screen size (default is maximum size)

logdev=ttyS1			Set device used for log messages
				("tty1" for screen (default), "ttyS1" for UART,
				 "ttyS1" should be used if no screen is attached)

loglevel=4			Control amount of generated log messages
				(0: only panic, 1: also errors, 2: also warnings,
				 3: also notices, 4: also debug output (default))

keymap=UK			Select keyboard mapping ("DE", "ES", "FR", "IT", "UK" or "US")
				The default mapping can be selected at the end of the file
				include/circle/sysconfig.h (normally "DE").

usbpowerdelay=510		Delay in milliseconds between powering on an USB device on an attached
				USB hub (both internal and external) and accessing the device
				(default 510, which is sometimes not enough to detect a device)

usbspeed=full			Set speed of the whole USB to full speed (12 Mbps) instead of
				high speed (480 Mbps) as workaround in case of problems
				(does work on Raspberry Pi 1-3 and Zero only)

usbboost=true			Speed-up handling of USB bulk split transactions to the maximum.
				This may be especially important for USB MIDI keyboards, which
				lose MIDI events (does work on Raspberry Pi 1-3 and Zero only).

usbignore=int3-0-0		Prevent loading a driver for an USB interface or device, which
				is normally supported by Circle, but does not functioning with
				a specific USB device. Can be specified only once.

sounddev=sndpwm			Set device used for sound output
				("sndpwm" for PWM (via headphone jack), "sndi2s" for I2S (external
				 hardware required), "sndvchiq" for VCHIQ (HDMI or headphone jack),
				 "sndhdmi" for HDMI (without VCHIQ), "sndusb" for USB (external
				 hardware required), default depends on application)

soundopt=0			Select destination for sound output with "sounddev=sndvchiq"
				(0: detected automatically (default), 1: headphone jack, 2: HDMI)

fast=true			Set maximum CPU speed (if class CCPUThrottle is in the system,
				default is low speed on Raspberry Pi 2/3/4/Zero)

socmaxtemp=60			Set maximum temperature of the SoC (the main chip) to be enforced
				in degrees Celsius (if class CCPUThrottle is in the system,
				range 40 to 78, default 60)

gpiofanpin=2			GPIO pin number (SoC number, not header position) to which the
				control line of a (Raspberry Pi 4) Case Fan is connected. The fan
				will be switched on, when "socmaxtemp" will be reached (if class
				CCPUThrottle is in the system). The CPU speed is not throttled,
				when this option is used.

touchscreen=140,3960,340,3920	Set calibration coordinates for USB touchscreens
				(minimum x, maximum x, minimum y, maximum y)
				Use tools/touchscreen-calibrator to determine the values!
