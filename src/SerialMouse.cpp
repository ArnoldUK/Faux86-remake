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
#include "MemUtils.h"
#include "SerialMouse.h"

using namespace Faux86;

SerialMouse::SerialMouse(VM& inVM)
	: bufptr(0)
	, vm(inVM)
{
	log(Log, "[MOUSE] Constructed");
}

//void SerialMouse::init(UART& _uart) {
void SerialMouse::init() {
	log(Log, "[MOUSE] Initialized");
	//uart = _uart;
	//uint16_t basePort = vm.config.mouse.port;
	//vm.ports.setPortRedirector(basePort, basePort + 7, this);
}


void SerialMouse::bufsermousedata(uint8_t value) {
	if (bufptr >= MOUSE_BUFFER_LEN) return;
	//if (bufptr == 0) vm.pic.doirq(vm.config.mouse.irq);
	buf[bufptr++] = value;
}

void SerialMouse::getsermousedata(uint8_t &bufData) {
	//if (vm.uartcom1.uart.rxnew) return;
	//if (rxnew) return;
	bufData = buf[0];
	if (bufptr == 0) return;
	//bufData = buf[0];
	vm.uartcom1.rxdata(buf[0]);
	MemUtils::memmove(buf, buf+1, MOUSE_BUFFER_LEN-1);
	bufptr--;
}

void SerialMouse::rxpoll(uint8_t rxnew) {
	if (vm.uartcom1.uart.rxnew) return;
	//if (rxnew) return;
	if (bufptr == 0) return;
	//reg[4] = ~reg[4] & 1;
	//log(Log, "[MOUSE] rxpoll %d", bufptr);
	
	vm.uartcom1.rxdata(buf[0]);
	MemUtils::memmove(buf, buf+1, MOUSE_BUFFER_LEN-1);
	bufptr--;
}

/*
//ORIGINAL FAUX86 CODE
bool SerialMouse::portWriteHandler(uint16_t portnum, uint8_t value) {
	log(Log, "[MOUSE] portWriteHandler port %X %X", portnum, portnum & 7);
	
	uint8_t oldreg;

	portnum &= 7;
	oldreg = reg[portnum];
	reg[portnum] = value;
	switch (portnum) 
	{
		case 4: //modem control register
			if ( (value & 1) != (oldreg & 1) ) { //software toggling of this register
					bufptr = 0; //causes the mouse to reset and fill the buffer
					//Sleep(14); //delay 14ms
					bufsermousedata('M'); //with a bunch of ASCII 'M' characters.
					//bufsermousedata('M'); //this is intended to be a way for
					//bufsermousedata('M'); //drivers to verify that there is
					//bufsermousedata('M'); //actually a mouse connected to the port.
					//bufsermousedata('M');
					//bufsermousedata('M');
					//Sleep(14); //delay 14ms
					//bufsermousedata('M'); //Send 'M' after 14ms
					//Sleep(63); //Logitech mouse send '3' 63ms
					bufsermousedata('3'); //logitech mouse
				}
			break;
	}
	return true;
}
*/

/*
//ORIGINAL FAUX86 CODE
bool SerialMouse::portReadHandler(uint16_t portnum, uint8_t &outValue) {
	//printf("[DEBUG] Serial mouse, port %X in\n", portnum);
	portnum &= 7;
	switch (portnum) {
		case 0: //data receive
			outValue = buf[0];
			//MemUtils::memmove(buf, &buf[1], MOUSE_BUFFER_LEN-1);
			MemUtils::memmove(buf, buf+1, MOUSE_BUFFER_LEN-1);
			bufptr--;
			if (bufptr < 0) bufptr = 0;
    	if (bufptr > 0) vm.pic.doirq(vm.config.mouse.irq);
    	//vm.pic.doirq(vm.config.mouse.irq);
			reg[4] = ~reg[4] & 1;
			return true;
		//case 0x02:
			//outValue = 0x07;
			//vm.pic.doirq(vm.config.mouse.irq);
			//return true;
		case 0x05: //line status register (read-only)
			//if (bufptr > 0) outValue = 1;
			//else outValue = 0;
			//outValue |= 0x60;
			if (bufptr > 0) outValue = 0x61;
			else outValue = 0x60;
			return true;
		//case 0x07:
		//	outValue = 0xFF; // uart->scratch;
		//	return true;
	}
	outValue = (reg[portnum & 7]);
	return true;
}
*/


void SerialMouse::togglereset(uint8_t value) {
	#ifdef DEBUG_MOUSE
	log(Log, "[MOUSE] togglereset %u", value);
	#endif
	//PORT 0x04: //MCR
	//uint8_t oldreg;
	//portnum &= 0x07;
	//oldreg = reg[4];
	//reg[4] = value;
	
	//THIS NEEDS FIXING AS NOT ALL DRIVERS DETECT A MOUSE CORRECTLY
	//software toggling of this register
	//causes the mouse to reset and fill the buffer
	if ((lasttoggle != 0x03) && ((value & 0x03) == 0x03)) {
		bufptr = 0; 
		//Sleep(14); //delay before buffering 14ms
		bufsermousedata('M'); //send 0x4D ASCII 'M' characters.
		//bufsermousedata('M'); //buffer with more ASCII 'M' characters.
		//bufsermousedata('M'); //buffer with more ASCII 'M' characters.
		//bufsermousedata('M'); //buffer with more ASCII 'M' characters.
		//bufsermousedata('M'); //buffer with more ASCII 'M' characters.
		//bufsermousedata('M'); //buffer with more ASCII 'M' characters.
		//Sleep(63); //Logitech mouse send '3' after 63ms
		//bufsermousedata('3');
		#ifdef DEBUG_MOUSE
		log(Log, "[MOUSE] register toggle %u %u", value, lasttoggle);
		#endif
	}
	lasttoggle = value & 0x03;
}

/*
//ORIGINAL FAUX86 CODE
void SerialMouse::triggerEvent (uint8_t buttons, int8_t xrel, int8_t yrel) {
	//log(Log, "[DEBUG] SerialMouse buttons:%X X:%d Y:%d\n", buttons, xrel, yrel);
	uint8_t highbits = 0;
	if (xrel < 0) highbits = 3;
	else highbits = 0;
	if (yrel < 0) highbits |= 12;
	//mouse_addbuf(0x40 | ((yrel & 0xC0) >> 4) | ((xrel & 0xC0) >> 6) | (mouse_state.left ? 0x20 : 0x00) | (mouse_state.right ? 0x10 : 0x00));
	bufsermousedata (0x40 | (buttons << 4) | highbits);
	bufsermousedata (xrel & 0x3F); //63
	bufsermousedata (yrel & 0x3F); //63
}
*/

void SerialMouse::triggerEvent(uint8_t buttons, int8_t xrel, int8_t yrel) {
	#ifdef DEBUG_MOUSE
	log(Log, "[MOUSE] triggerEvent buttons:%u X:%d Y:%d", buttons, xrel, yrel);
	#endif
	//mouse_addbuf(0x40 | ((yrel & 0xC0) >> 4) | ((xrel & 0xC0) >> 6) | (mouse_state.left ? 0x20 : 0x00) | (mouse_state.right ? 0x10 : 0x00));
	
	//uint8_t highbits = (((yrel>>6) & 0x3) << 2)|((xrel >> 6) & 0x3);
	//uint8_t highbits = (0x40 | ((yrel & 0xC0) >> 4) | ((xrel & 0xC0) >> 6));
	//uint8_t highbits = 0x80 | 0x40 | (((yrel >> 6) & 0x03) << 2) | (((xrel >> 6) & 0x03) << 0);
	uint8_t highbits = ((yrel & 0xC0) >> 4) | ((xrel & 0xC0) >> 6);

	bufsermousedata(0x40 | highbits | (buttons << 4));
	//bufsermousedata(0x80 | 0x40 | highbits | (buttons << 4));
	
	// second horizontal
	bufsermousedata(xrel & 0x3F); //63
	// third vertical
	bufsermousedata(yrel & 0x3F); //63
}

void SerialMouse::handleButtonDown(ButtonType button) {
	switch (button)
	{
	case ButtonType::Left:
		buttonState |= 2;
		break;
	case ButtonType::Right:
		buttonState |= 1;
		break;
	case ButtonType::Middle:
		//buttonState |= 3;
		break;
	}

	triggerEvent(buttonState, 0, 0);
}

void SerialMouse::handleButtonUp(ButtonType button) {
	switch (button)
	{
	case ButtonType::Left:
		buttonState &= ~2;
		break;
	case ButtonType::Right:
		buttonState &= ~1;
		break;
	case ButtonType::Middle:
		//buttonState &= ~3;
		break;
	}

	triggerEvent(buttonState, 0, 0);
}

void SerialMouse::handleMove(int8_t xrel, int8_t yrel) {
	triggerEvent(buttonState, xrel, yrel);
}
