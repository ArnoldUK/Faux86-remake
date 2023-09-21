
/*
  XTulator: A portable, open-source 80186 PC emulator.
  Copyright (C)2020 Mike Chambers
  
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

//#include <stdint.h>
//#include "i8259.h"

#include "Config.h"
#include "Types.h"
#include "Ports.h"



#define UART_IRQ_MSR_ENABLE			0x08
#define UART_IRQ_LSR_ENABLE			0x04
#define UART_IRQ_TX_ENABLE			0x02
#define UART_IRQ_RX_ENABLE			0x01

#define UART_PENDING_RX				0x01
#define UART_PENDING_TX				0x02
#define UART_PENDING_MSR			0x04
#define UART_PENDING_LSR			0x08

typedef struct {
	uint8_t rx;
	uint8_t tx;
	uint8_t rxnew;
	uint8_t dlab;
	uint8_t ien; //interrupt enable register
	uint8_t iir; //interrupt identification register
	uint8_t lcr; //line control register
	uint8_t mcr; //modem control register
	uint8_t lsr; //line status register
	uint8_t msr; //modem status register
	uint8_t lastmsr; //to calculate delta bits
	uint8_t scratch;
	uint16_t divisor;
	uint8_t irq;
	uint8_t pendirq;
	void* udata;
	void* udata2;
	void (*txCb)(void*, uint8_t);
	void (*mcrCb)(void*, uint8_t);
	//I8259_t* i8259;
} UART_t;

namespace Faux86
{
	class VM;
	
	class UART : public PortInterface
	{
	public:
		UART(VM& inVM);
		~UART(void);
		
		//void writeport(uint16_t addr, uint8_t value);
		//uint8_t readport(uint16_t addr);
		void rxdata(uint8_t value);
		void init(uint16_t base, uint8_t irq, void (*tx)(void*, uint8_t), void* udata, void (*mcr)(void*, uint8_t), void* udata2);

		UART_t uart;
		
		virtual bool portWriteHandler(uint16_t portnum, uint8_t value) override;
		virtual bool portReadHandler(uint16_t portnum, uint8_t &outValue) override;

	private:
		VM& vm;
	};
}
