/***************************************************************************
 *   Copyright (C) 2014
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


#include "AvrGpioUsbAvrProgDevice.h"
#include "AvrError.h"


#include "gpio-usb.h"


AvrGpioUsbAvrProgDevice::AvrGpioUsbAvrProgDevice(const AvrChipDesc* a_cd, uint a_sclkfreq, uint a_flags)
							: AvrGpioUsbAvrRawProgDevice(a_cd, a_sclkfreq, a_flags)
{
	_name = "AvrGpioUsbAvrProgDevice";
	if (a_cd->arch != AvrChipDesc::Arch_AVR || a_cd->prginterface != AvrChipDesc::PrgInterface_SPI) {
		throw AvrException(_name + " does not support chip " + a_cd->name);
	}
}

AvrGpioUsbAvrProgDevice::~AvrGpioUsbAvrProgDevice() {
}

void AvrGpioUsbAvrProgDevice::checkStatus() {
	byte ret[2];
	int rc = _usbdev.controlTransIn(GPIO_USB_REQTYPE,
				CMD_AVR_GET_STATUS, 0, 0, ret, 2);
	if (rc != 2) {
		throw AvrError("command AVR_GET_STATUS: usb error");
	}
	if (ret[1]) { // error flags
		String msg = "GPIO error flags:";
		if (ret[1] & AVR_ERRF_NOSYNC) {
			msg += " NOSYNC";
		}
		if (ret[1] & AVR_ERRF_POLL_TIMEOUT) {
			msg += " POLL_TIMEOUT";
		}
		throw AvrError(msg);
	}
}

void AvrGpioUsbAvrProgDevice::cmdReadProgMem(uint a_addr, uint a_size, Array<uint16>& a_mem, Progress* a_pg) {
	a_mem.setSize(a_size);
	if (a_size == 0)
		return;
	
	bool needextaddr = _chipdesc->flash_size > 0x10000;
	byte extaddr = (byte)(a_addr >> 16);
	uint addr = a_addr;
	uint offs = 0;
	if (needextaddr) {
		cmdLoadExtAddrByte(extaddr);
	}
	for (uint i = a_size; i > 0; ) {
		if (needextaddr && (extaddr != (byte)(addr >> 16))) {
			extaddr = (byte)(addr >> 16);
			cmdLoadExtAddrByte(extaddr);
		}
		uint sz = 0x100 - (addr & 0x0ff); // read 256 byte chunks
		if (sz > i)
			sz = i;
		uint16* p = a_mem.data() + offs;
		int rc = _usbdev.controlTransIn(GPIO_USB_REQTYPE,
						CMD_AVR_READ_PRGMEM, addr & 0x0ffff, 0, (byte*)p, sz * 2);
		if (rc != (sz * 2)) {
			throw AvrError("command AVR_READ_PRGMEM: usb error");
		}
		
		if (a_pg) {
			for (uint k = 0; k < sz; k++) {
				a_pg->next(addr + k, p[k]);
			}
		}
		i -= sz;
		addr += sz;
		offs += sz;
	}
	checkStatus();
}

void AvrGpioUsbAvrProgDevice::cmdProgramProgMemPage(uint a_addr, const Array<uint16>& a_mem, uint a_offs, Progress* a_pg) {
	uint addr = a_addr & _chipdesc->flash_page_mask;
	if (_chipdesc->flash_size > 0x10000) {
		cmdLoadExtAddrByte(addr >> 16);
	}
	int rc = _usbdev.controlTransOut(GPIO_USB_REQTYPE,
						CMD_AVR_WRITE_PRGMEM_PAGE, addr & 0x0ffff, 0,(byte*)(a_mem.data() + a_offs),
						_chipdesc->flash_page_size * 2);
	if (rc != _chipdesc->flash_page_size * 2) {
		throw AvrError("command AVR_WRITE_PRGMEM_PAGE: usb error");
	}
	checkStatus();
	if (a_pg) {
		for (uint k = 0; k < _chipdesc->flash_page_size; k++) {
			a_pg->next(addr + k, a_mem[a_offs + k]);
		}
	}
}

void AvrGpioUsbAvrProgDevice::cmdReadEeprom(uint a_addr, uint a_size, Array<byte>& a_mem, Progress* a_pg) {
	a_mem.setSize(a_size);
	if (a_size == 0)
		return;
	
	uint addr = a_addr;
	uint offs = 0;
	for (uint i = a_size; i > 0; ) {
		uint sz = 64;
		if (sz > i)
			sz = i;
		byte* p = a_mem.data() + offs;
		int rc = _usbdev.controlTransIn(GPIO_USB_REQTYPE,
						CMD_AVR_READ_EEPROM, addr & 0x0ffff, 0, p, sz);
		if (rc != sz) {
			throw AvrError("command AVR_READ_EEPROM: usb error");
		}
		if (a_pg) {
			for (uint k = 0; k < sz; k++) {
				a_pg->next(addr + k, p[k]);
			}
		}
		i -= sz;
		addr += sz;
		offs += sz;
	}
	checkStatus();
}

void AvrGpioUsbAvrProgDevice::cmdProgramEepromPage(uint a_addr, const Array<byte>& a_mem, uint a_size, uint a_offs, Progress* a_pg) {	
	uint addr = a_addr & _chipdesc->eeprom_page_mask;
	int rc = _usbdev.controlTransOut(GPIO_USB_REQTYPE,
						CMD_AVR_WRITE_EEPROM_PAGE, addr & 0x0ffff, 0, a_mem.data() + a_offs,
						a_size);
	if (rc != a_size) {
		throw AvrError("command AVR_WRITE_EEPROM_PAGE: usb error");
	}
	checkStatus();
	if (a_pg) {
		for (uint k = 0; k < a_size; k++) {
			a_pg->next(addr + k, a_mem[a_offs + k]);
		}
	}
}




