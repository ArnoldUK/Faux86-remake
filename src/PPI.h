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

/*
	Intel 8255 Programmable Peripheral Interface (PPI)
	This is not complete.
*/

#pragma once

#include "Config.h"
#include "Types.h"
#include "Ports.h"

//#include <stdint.h>


namespace Faux86
{
	class VM;

	// Intel 8255 Programmable Peripheral Interface (PPI)
	class PPI : public PortInterface
	{
	public:
		PPI(VM& inVM);
		
		uint8_t sw2;
		uint8_t portA;
		uint8_t portB;
		uint8_t portC;

		//void tick();
		virtual bool portWriteHandler(uint16_t portnum, uint8_t value) override;
		virtual bool portReadHandler(uint16_t portnum, uint8_t& outValue) override;
		void dramRefreshToggle(void);	

	private:

		VM& vm;
	};

}


