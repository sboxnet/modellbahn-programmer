/***************************************************************************
 *   Copyright (C) 2013
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



#include "AvrGpioUsbAvrRawProgDevice.h"
#include "AvrError.h"

#include "gpio-usb.h"

#include <tmb/lang/System.h>



enum FLAGS {
	FLG_AVRMODE		= 0x0001,
	FLG_AVRPROGMODE	= 0x0002,
	FLG_AVRWAIT0	= 0x0004,
	FLG_AVRWAIT1	= 0x0008,
};


enum { 
	AVRSCLKPERIOD_MIN = 10, // usec
	AVRSCLKFREQ_MAX   = 100000, // Hz
	AVRSCLKFREQ_MIN	 = AVRSCLKFREQ_MAX / 256
};


static uint calcAvrSclkPeriod(uint a_sclkfreq) {
	// SCLK frequency: max 100kHz
	uint sclk_period;
	if (a_sclkfreq == 0 || a_sclkfreq >= AVRSCLKFREQ_MAX) {
		sclk_period = 0; // == 10us   , see: gpio-ser command cmdAvrProgMode()
	} else {
		sclk_period = ((AVRSCLKFREQ_MAX + a_sclkfreq - 1) / a_sclkfreq) - 1; // round up to next higher period or next lower frequency: 50kHz, 33kHz, 25kHz, ...
		if (sclk_period > 255)
			sclk_period = 255;	// max: 2.55ms or 390.6Hz
	}
	return sclk_period;
}



AvrGpioUsbAvrRawProgDevice::AvrGpioUsbAvrRawProgDevice(const AvrChipDesc* a_cd, uint a_sclkfreq, uint a_flags)
							: IAvrSerialProgDevice("AvrGpioUsbAvrRawProgDevice", a_cd),
							  _sclkfreq(a_sclkfreq),
							  _flags(a_flags)
{
	if (a_cd->arch != AvrChipDesc::Arch_AVR || a_cd->prginterface != AvrChipDesc::PrgInterface_SPI) {
		throw AvrException(_name + " does not support chip " + a_cd->name);
	}
}

uint AvrGpioUsbAvrRawProgDevice::getSclkFreq() {
	return AVRSCLKFREQ_MAX / (calcAvrSclkPeriod(_sclkfreq) + 1);
}

AvrGpioUsbAvrRawProgDevice::~AvrGpioUsbAvrRawProgDevice() {
	close();
}

void AvrGpioUsbAvrRawProgDevice::open() {
	int rc;

	close();
	
	_usbdev.open(VENDOR_ID, PRODUCT_ID);
	
	cout << "successfully opened USB device: gpio-usb" << endl;
	
	_usbdev.controlTrans(GPIO_USB_REQTYPE, CMD_ENTER_AVRMODE, 0, 0);
}

void AvrGpioUsbAvrRawProgDevice::close() {
	_sip = 0;
	_mode = 0;
	if (_usbdev.isOpen()) {
		_usbdev.controlTrans(GPIO_USB_REQTYPE, CMD_LEAVE_AVRMODE, 0, 0);
	}
	_usbdev.close();
}

void AvrGpioUsbAvrRawProgDevice::sipCommit() {
	throw Exception("AvrGpioUsbAvrRawProgDevice::sipCommit(): unimplemented");
}

void AvrGpioUsbAvrRawProgDevice::sipRead() {
	throw Exception("AvrGpioUsbAvrRawProgDevice::sipRead(): unimplemented");
}

static inline uint Max(uint a, uint b) {
	return (a > b ? a : b);
}

void AvrGpioUsbAvrRawProgDevice::setProgMode(bool a_on) {
	uint8_t flags = 0;
	uint8_t sclk_period = calcAvrSclkPeriod(_sclkfreq);
	if (a_on) {
		if (_chipdesc->spi_has_cmdpoll && !(_flags & FLG_NOPOLL)) {
			flags |= AVRWAIT_POLL;
		} else {
			uint wait = Max(Max(_chipdesc->spi_twd_flash, _chipdesc->spi_twd_eeprom),
							Max(_chipdesc->spi_twd_erase, _chipdesc->spi_twd_fuse));
			if (wait <= 8)
				flags |= AVRWAIT_8MS;
			else if (wait <= 16)
				flags |= AVRWAIT_16MS;
			else
				flags |= AVRWAIT_64MS;
		}
		flags |= FLG_AVRPROGMODE;
	}
	uint8_t rcflags = 0;
	
	int rc = _usbdev.controlTransIn(GPIO_USB_REQTYPE,
						CMD_AVR_PROGMODE, ((uint16_t)flags << 8)|sclk_period, 0,
						&rcflags, 1);
	if (rc != 1) {
		throw AvrException("command AVR_RAWCMD: missing return value");
	}

	if (a_on) {
		if (!(rcflags & FLG_AVRPROGMODE)) {
			throw AvrException("AvrGpioUsbAvrRawProgDevice::setProgMode(): failed to set programming mode on");
		}
		
		System::wait_ms(100);
	}
}

uint32 AvrGpioUsbAvrRawProgDevice::sendCmd(uint32 a_cmd, bool a_checkcomm) {
	uint8_t buf[4] = { 0, };

	int rc = _usbdev.controlTransIn(GPIO_USB_REQTYPE,
						CMD_AVR_RAWCMD, (uint16_t)(a_cmd >> 16), (uint16_t)a_cmd,
						buf, 3);
	if (rc != 3) {
		throw AvrException("command AVR_RAWCMD: missing return values");
	}
	uint32_t recv = ((uint32_t)buf[0] << 16)|((uint32_t)buf[1] << 8)|buf[2];
	if (a_checkcomm && (((a_cmd >> 8) & 0x00ffff00) != (recv & 0x00ffff00))) {
		throw AvrException("AvrGpioUsbAvrRawProgDevice::sendCmd(): communication error");
	}
	return recv;
}

