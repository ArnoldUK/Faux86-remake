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
	Intel i8259 Programmable Interrupt Controller
	Note: this is not a complete 8259 implementation, but for the purposes
  of a PC, it's all we need.
*/

#include "VM.h"
#include "PIC.h"

using namespace Faux86;

bool PIC::portReadHandler(uint16_t portnum, uint8_t& outValue)
{
	outValue = 0;
	
	#ifdef DEBUG_PIC
	log(Log,"[PIC::I8259] portReadHandler port 0x%X", portnum);
	#endif

	switch (portnum & 1) {
   case 0:
	   if (readmode == 0) outValue = irr;
	   else outValue = isr;
	  break;
   case 1: //read mask register
		outValue = imr;
		break;
	}

	return true;
}

/*
//ORIGINAL FAUX86 CODE
bool PIC::portWriteHandler(uint16_t portnum, uint8_t value)
{
	 uint8_t i;
	 switch (portnum & 1) {
		case 0:
		 if (value & 0x10) { //begin initialization sequence
			icwstep = 1;
			imr = 0; //clear interrupt mask register
			icw[icwstep++] = value;
			return true;
		 }
		 if ((value & 0x98)==8) { //it's an OCW3
			if (value & 2) readmode = value & 2;
		 }
		 if (value & 0x20) { //EOI command
			vm.input.markKeyEventHandled();
			for (i=0; i<8; i++)
			if ((isr >> i) & 1) {
			   isr ^= (1 << i);
			   if (i == 0 && vm.cpu.makeupticks > 0) 
			   { 
				   vm.cpu.makeupticks = 0; 
				   irr |= 1; 
			   }
			   return true;
			}
		 }
		 break;
		case 1:
		 if ((icwstep==3) && (icw[1] & 2)) icwstep = 4; //single mode, so don't read ICW3
		 if (icwstep<5) { icw[icwstep++] = value; return true; }
		 //if we get to this point, this is just a new IMR value
		 imr = value;
		 break;
	 }

	 return true;
}
*/

//UPDATED CODE
bool PIC::portWriteHandler(uint16_t portnum, uint8_t value) {
	#ifdef DEBUG_PIC
	log(Log,"[PIC::I8259] portWriteHandler port 0x%X: %X", portnum, value);
	#endif

	uint8_t i;
	
	switch (portnum & 1) {
	case 0:
		if (value & 0x10) { //ICW1
			imr = 0x00;
			icwstep = 1; //2;
			//icw[1] = value;
			icw[icwstep++] = value;
			//readmode = 0;
			return true;
		}
		if ((value & 0x98) == 8) { //it's an OCW3
			if (value & 2) readmode = value & 2; //
		}
		if (value & 0x20) { //EOI command
			//vm.input.markKeyEventHandled();
			for (i = 0; i < 8; i++)
			if ((isr >> i) & 1) {
				isr ^= (1 << i);
				if (i == 0 && vm.cpu.makeupticks > 0) { 
				 vm.cpu.makeupticks = 0; 
				 irr |= 1; 
				}
				return true;
			}
		 }
		 break;
		
		/*
		if ((value & 0x08) == 0) { //OCW2
			#ifdef DEBUG_PIC
			//LOGDEBUG("[I8259] OCW2 = %02X\r\n", value);
			#endif
			ocw[2] = value;
			switch (value & 0xE0) {
			case 0x60: //specific EOI
				irr &= ~(1 << (value & 0x03));
				isr &= ~(1 << (value & 0x03));
				break;
			case 0x40: //no operation
				break;
			case 0x20: //non-specific EOI
				irr &= ~isr;
				isr = 0x00;
				break;
			default: //other
				#ifdef DEBUG_PIC
				//LOGDEBUG("[I8259] Unhandled EOI type: %u\r\n", value & 0xE0);
				#endif
				break;
			}
		}	else { //OCW3
			#ifdef DEBUG_PIC
			//LOGDEBUG("[I8259] OCW3 = %02X\r\n", value);
			#endif
			ocw[3] = value;
			if (value & 0x02) readmode = value & 1;
		}
		break;
	*/
	case 1:
		/*
		switch (icwstep) {
		case 2: //ICW2
			icw[2] = value;
			intoffset = value & 0xF8;
			if (icw[1] & 0x02) icwstep = 4;
			else icwstep = 3;
			break;
		case 3: //ICW3
			icw[3] = value;
			if (icw[1] & 0x01) icwstep = 4;
			else icwstep = 5; //done with ICWs
			break;
		case 4: //ICW4
			icw[4] = value;
			icwstep = 5; //done with ICWs
			break;
		case 5: //just set IMR value now
			imr = value;
			break;
		}
		break;
		*/
		if ((icwstep == 3) && (icw[1] & 2)) icwstep = 4; // single mode, so don't read ICW3
    if (icwstep < 5) {
			icw[icwstep++] = value;
      return true;
    }
    // if we get to this point, this is just a new IMR value
    imr = value;
    break;
	}
	return true;
}


/*
//ORIGINAL FAUX86 CODE
uint8_t PIC::nextintr() 
{
	uint8_t i, tmpirr;
	tmpirr = irr & (~imr); //XOR request register with inverted mask register
	for (i = 0; i < 8; i++)
	{
		if ((tmpirr >> i) & 1)
		{
			irr ^= (1 << i);
			isr |= (1 << i);
			return(icw[2] + i);
		}
	}
	return(0); //this won't be reached, but without it the compiler gives a warning
}
*/


//XTULATOR CODE
uint8_t PIC::nextintr() 
{
	uint8_t ret = 0;
	uint8_t i, tmpirr;
	tmpirr = irr & (~imr); //XOR request register with inverted mask register
	for (i = 0; i < 8; i++)	{
		if ((tmpirr >> i) & 1) {
			//irr &= ~(1 << i);
			//isr |= (1 << i);
			irr ^= (1 << i);
			isr |= (1 << i);
			ret = icw[2] + i;
			break;
		}
	}
	// keyboard services required
  if (ret == 9) {
    //i8255_key_required();
    vm.input.markKeyEventHandled();
  }
  // return the next interrupt number
  return ret;
	//return(0); //this won't be reached, but without it the compiler gives a warning
}

bool PIC::irqpending(void) {
  return (irr & (~imr)) != 0;
}

void PIC::doirq(uint8_t irqnum) 
{
	irr |= (1 << irqnum);
	//irr |= (1 << irqnum) & (~imr);
}

PIC::PIC(VM& inVM) 
	: imr(0)		//mask register
	, irr(0)		//request register
	, isr(0)		//service register
	, icwstep(0)	//used during initialization to keep track of which ICW we're at
	, icw {0, 0, 0, 0, 0}
	, ocw {0, 0, 0, 0, 0}
	//, intoffset(0)	//interrupt vector offset FAUX86
	, intoffset(8)	//interrupt vector offset XTULATOR
	, priority(0)	//which IRQ has highest priority
	, autoeoi(0)	//automatic EOI mode
	, readmode(0)	//remember what to return on read register from OCW3
	, enabled(0)
	, vm(inVM)
{
	log(Log,"[PIC::I8259] Constructed");
	vm.ports.setPortRedirector(0x20, 0x21, this);
}

