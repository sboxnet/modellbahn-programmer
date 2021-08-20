/***************************************************************************
 *   Copyright (C) 2009
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



#include "AvrGpioSerAvrRawProgDevice.h"
#include "AvrError.h"

#include <tmb/lang/System.h>

static inline uint Max(uint a, uint b) {
	return (a > b ? a : b);
}

AvrGpioSerAvrRawProgDevice::AvrGpioSerAvrRawProgDevice(const String& a_devfile, const AvrChipDesc* a_cd, uint a_sclkfreq, uint a_flags)
							: IAvrSerialProgDevice("AvrGpioSerAvrRawProgDevice", a_cd),
							  _devfile(a_devfile),
							  _gpioser(a_devfile, 500), // 500 ms communication timeout
							  _sclkfreq(a_sclkfreq),
							  _flags(a_flags)
{
	if (a_cd->arch != AvrChipDesc::Arch_AVR || a_cd->prginterface != AvrChipDesc::PrgInterface_SPI) {
		throw AvrException(_name + " does not support chip " + a_cd->name);
	}
}

uint AvrGpioSerAvrRawProgDevice::getSclkFreq() {
	return _gpioser.getAvrSclkFreqFrom(_sclkfreq);
}

AvrGpioSerAvrRawProgDevice::~AvrGpioSerAvrRawProgDevice() {
	close();
}

void AvrGpioSerAvrRawProgDevice::open() {
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
		
		_gpioser.cmdEnterAvrMode();
		
	} catch(const TryAgainException&) {
		close();
		throw Exception("no response from GPIO-Ser");
	}
}

void AvrGpioSerAvrRawProgDevice::close() {
	_sip = 0;
	_mode = 0;
	try { _gpioser.cmdLeaveAvrMode(); } catch(...) {}
	_gpioser.close();
}

void AvrGpioSerAvrRawProgDevice::sipCommit() {
	throw Exception("AvrGpioSerAvrRawProgDevice::sipCommit(): unimplemented");
}

void AvrGpioSerAvrRawProgDevice::sipRead() {
	throw Exception("AvrGpioSerAvrRawProgDevice::sipRead(): unimplemented");
}


void AvrGpioSerAvrRawProgDevice::setProgMode(bool a_on) {
	GpioSer::AvrWait avrwait = GpioSer::AVRWAIT_POLL;
	if (a_on) {
		if (_chipdesc->spi_has_cmdpoll && !(_flags & FLG_NOPOLL)) {
			avrwait = GpioSer::AVRWAIT_POLL;
		} else {
			uint wait = Max(Max(_chipdesc->spi_twd_flash, _chipdesc->spi_twd_eeprom),
							Max(_chipdesc->spi_twd_erase, _chipdesc->spi_twd_fuse));
			if (wait <= 8)
				avrwait = GpioSer::AVRWAIT_8MS;
			else if (wait <= 16)
				avrwait = GpioSer::AVRWAIT_16MS;
			else
				avrwait = GpioSer::AVRWAIT_64MS;
		}
	}
	if (!_gpioser.cmdAvrProgMode(a_on, avrwait, _sclkfreq) && a_on) {
//cout << hex << "flags: 0x" << (uint)_gpioser.cmdGetFlags() << dec << endl;
//cout << hex << "errflags: 0x" << (uint)_gpioser.cmdGetErrorFlags() << dec << endl;
		throw AvrException("AvrGpioSerAvrRawProgDevice::setProgMode(): failed to set programming mode on");
	}
//cout << hex << "flags: 0x" << (uint)_gpioser.cmdGetFlags() << dec << endl;
//cout << hex << "errflags: 0x" << (uint)_gpioser.cmdGetErrorFlags() << dec << endl;
	if (a_on) {
		System::wait_ms(100);
	}
}

uint32 AvrGpioSerAvrRawProgDevice::sendCmd(uint32 a_cmd, bool a_checkcomm) {
	uint32 recv = _gpioser.cmdAvrRawCmd(a_cmd);
	if (a_checkcomm && (((a_cmd >> 8) & 0x00ffff00) != (recv & 0x00ffff00))) {
		throw AvrException("AvrGpioSerAvrRawProgDevice::sendCmd(): communication error");
	}
	return recv;
}

