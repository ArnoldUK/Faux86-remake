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

#include "Types.h"

namespace Faux86
{
	class VM;

	class InputManager
	{
	public:
		InputManager(VM& inVM);

		void tick();

		void handleKeyDown(uint16_t scancode);
		void handleKeyUp(uint16_t scancode);

		void markKeyEventHandled() { keyboardWaitAck = false; }

	private:
		void queueData(uint8_t data);

		VM& vm;

		constexpr static int MaxKeyboardBufferSize = 32;

		uint8_t keyboardBuffer[MaxKeyboardBufferSize];
		bool keyboardWaitAck = false;
		int keyboardBufferSize = 0;
		int keyboardBufferPos = 0;
	};
}

