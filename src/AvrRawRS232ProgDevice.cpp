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

#include "AvrRawRS232ProgDevice.h"
#include "AvrError.h"

#include <tmb/lang/System.h>

#include <termios.h>
#include <sys/ioctl.h>

#define SBIT_SCLK	TIOCM_RTS
#define SBIT_MOSI	TIOCM_DTR
#define SBIT_MISO	TIOCM_DSR


AvrRawRS232ProgDevice::AvrRawRS232ProgDevice(const String& a_devfile, bool a_inverted_signals, const AvrChipDesc* a_cd)
							: IAvrSerialProgDevice(a_inverted_signals ?
													"AvrRawRS232ProgDevice (inverted signals)" :
													"AvrRawRS232ProgDevice", a_cd),
							  _devfile(a_devfile),
							  _rs232(RS232Device::Flag_OPEN_RDWR, a_devfile),
							  _inverted_signals(a_inverted_signals) {
	if (a_cd->arch != AvrChipDesc::Arch_AVR || a_cd->prginterface != AvrChipDesc::PrgInterface_SPI) {
		throw AvrException(_name + " does not support chip " + a_cd->name);
	}
}

AvrRawRS232ProgDevice::~AvrRawRS232ProgDevice() {
	close();
}
		
void AvrRawRS232ProgDevice::open() {
	close();
	
	_rs232.open();
	
	try {
		_rs232.ioctl(TCXONC, TCOOFF); // suspend output
		
		// set SCLK and MOSI to low
		int m = (_inverted_signals ? (SBIT_SCLK|SBIT_MOSI) : 0);
		_rs232.ioctl(TIOCMSET, &m);
	
		System::wait_ms(50);
		
	} catch(const Exception& ex) {
		_rs232.close();
		throw;
	}
}

void AvrRawRS232ProgDevice::close() {
	_sip = 0;
	_mode = 0;
	if (_rs232.fd() >= 0) {
		//int m = (a_inverted_signals ? TIOCM_RTS|TIOCM_DTR : 0);
		//try { _rs232->ioctl(TIOCMSET, &m); } catch(...) {}
		_rs232.close();
	}
}

void AvrRawRS232ProgDevice::sipCommit() {
	int m = 0;
	if (_mode.has(MODE_PROG)) {
		if (sipclk())
			m |= SBIT_SCLK;
		if (sipwd())
			m |= SBIT_MOSI;
	}
	if (_inverted_signals)
		m ^= (SBIT_SCLK|SBIT_MOSI);
	_rs232.ioctl(TIOCMSET, &m);
	System::wait_us(250);
}

void AvrRawRS232ProgDevice::sipRead() {
	int m = 0;
	_rs232.ioctl(TIOCMGET, &m);	
	if (_inverted_signals)
		m ^= SBIT_MISO;
	_sip.set(SIP_RD, (m & SBIT_MISO) != 0);
}

void AvrRawRS232ProgDevice::setProgMode(bool a_on) {
	_sip = 0;
	int m = (_inverted_signals ? (SBIT_SCLK|SBIT_MOSI) : 0);
	_rs232.ioctl(TIOCMSET, &m);
	_mode.set(MODE_PROG, a_on);
	if (a_on)
		System::wait_ms(100);	
}

