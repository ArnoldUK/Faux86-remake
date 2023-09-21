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
#pragma once

#include "Config.h"
#include "Types.h"

//#define MOUSE_BUFFER_LEN		16
#define MOUSE_BUFFER_LEN		60

namespace Faux86
{
	class VM;

	//class SerialMouse : public PortInterface
	class SerialMouse
	{
	public:
		enum class ButtonType
		{
			Left,
			Right,
			Middle
		};

		SerialMouse(VM& inVM);

		//void init(UART& _uart);
		void init();

		void handleButtonDown(ButtonType button);
		void handleButtonUp(ButtonType button);
		void handleMove(int8_t xrel, int8_t yrel);
		
		void togglereset(uint8_t value);
		void rxpoll(uint8_t rxnew);
		
		void getsermousedata(uint8_t &bufData);

		//virtual bool portWriteHandler(uint16_t portnum, uint8_t value) override;
		//virtual bool portReadHandler(uint16_t portnum, uint8_t &outValue) override;
		//UART& uart;
	private:
		void triggerEvent(uint8_t buttons, int8_t xrel, int8_t yrel);
		void bufsermousedata(uint8_t value);

		//uint8_t reg[8];
		uint8_t buf[MOUSE_BUFFER_LEN] = {0};
		int8_t bufptr = 0;

		uint8_t lasttoggle = 0;
		uint8_t buttonState = 0;

		VM& vm;
	};
}


