/***************************************************************************
 *   Copyright (C) 2008
 *   by Thomas Maier <balagi@justmail.de>
 *
 *   Copyright: See COPYING file that comes with this distribution         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 ***************************************************************************/



#include "AvrGpioSerProgDevice.h"
#include "AvrError.h"
#include <tmb/lang/System.h>

enum {
	BIT_SCLK	= 7,
	BIT_MISO	= 6,
	BIT_MOSI	= 5,
	BIT_RESET	= 4,
	BIT_POWER	= 3,
};



AvrGpioSerProgDevice::AvrGpioSerProgDevice(const String& a_devfile, const AvrChipDesc* a_cd)
							: IAvrSerialProgDevice("AvrGpioSerProgDevice", a_cd),
							  _devfile(a_devfile),
							  _gpioser(a_devfile)
{
	if (a_cd->arch != AvrChipDesc::Arch_AVR || a_cd->prginterface != AvrChipDesc::PrgInterface_SPI) {
		throw AvrException(_name + " does not support chip " + a_cd->name);
	}
}

AvrGpioSerProgDevice::~AvrGpioSerProgDevice() {
	close();
}
		
void AvrGpioSerProgDevice::open() {
	close();
	
	_gpioser.open();
	try {
		// reset gpio
		_gpioser.cmdSoftReset();
		System::wait_ms(100);
		
		// check communication
		uint errf = _gpioser.cmdGetErrorFlags();
		if (errf != 0) {
			close();
			throw IOException("GPIO-Ser error flags are set");
		}
		
		_gpioser.cmdPutOut((0<<BIT_SCLK)|(0<<BIT_MISO)|(0<<BIT_MOSI)|(0<<BIT_RESET)|(1<<BIT_POWER)|0xff07);
		_gpioser.cmdPutDDR((1<<BIT_SCLK)|(0<<BIT_MISO)|(1<<BIT_MOSI)|(1<<BIT_RESET)|(1<<BIT_POWER));
				
	} catch(const TryAgainException&) {
		close();
		throw Exception("no response from GPIO-Ser");
	}
}

void AvrGpioSerProgDevice::close() {
	_sip = 0;
	_mode = 0;
	_gpioser.close();
}

void AvrGpioSerProgDevice::sipCommit() {
	byte m = 0;
	if (_mode.has(MODE_PROG)) {
		if (sipclk())
			m |= (1<<(BIT_SCLK-4));
		if (sipwd())
			m |= (1<<(BIT_MOSI-4));
	}
	try {
		_gpioser.cmdPutOut4Bits(1, m); // bits 7 .. 4
	} catch(const TryAgainException&) {
		throw Exception("unable to write to serial device");
	}
}

void AvrGpioSerProgDevice::sipRead() {
	uint b;
	try {
		b = _gpioser.cmdGetPinBit(BIT_MISO);
	} catch(const TryAgainException&) { throw Exception("no response from GPIO-Ser"); }
	_sip.set(SIP_RD, b != 0);
}

void AvrGpioSerProgDevice::setProgMode(bool a_on) {
	_sip = 0;
	try {
		if (a_on) {
			_gpioser.cmdPutOutByte(0, (0<<BIT_SCLK)|(0<<BIT_MISO)|(0<<BIT_MOSI)|(0<<BIT_RESET)|(1<<BIT_POWER)|0x07); // SCLK, MOSI and Reset low
			_gpioser.cmdClrOutBit(BIT_POWER); // POWER on
			System::wait_ms(100); // wait 100 ms
		} else {
			// execute ATTiny shutdown sequence after programming
			_gpioser.cmdPutOut4Bits(1, 0x0); // SCLK, MOSI and Reset low
			_gpioser.cmdPutOut4Bits(1, 1<<(BIT_RESET-4)); // Reset high
			_gpioser.cmdPutOutByte(0, (0<<BIT_SCLK)|(0<<BIT_MISO)|(0<<BIT_MOSI)|(0<<BIT_RESET)|(1<<BIT_POWER)|0x07); // SCLK, MOSI and Reset low and Power off
			_gpioser.cmdPutDDR(0x0000); // all DDR to input
		}
	} catch(const TryAgainException&) {
		throw Exception("unable to write to serial device");
	}
	_mode.set(MODE_PROG, a_on);	
}

