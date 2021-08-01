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


#include "AvrGpioUsbXmegaProgDevice.h"
#include "AvrError.h"

#include "gpio-usb.h"


AvrGpioUsbXmegaProgDevice::AvrGpioUsbXmegaProgDevice(const AvrChipDesc* a_cd, uint a_sclkfreq)
							: IAvrSerialProgDevice("AvrGpioUsbXmegaProgDevice", a_cd),
							  _sclkfreq(a_sclkfreq)
{
	if (a_cd->arch != AvrChipDesc::Arch_XMEGA || a_cd->prginterface != AvrChipDesc::PrgInterface_PDI) {
		throw AvrException(_name + " does not support chip " + a_cd->name);
	}
}

AvrGpioUsbXmegaProgDevice::~AvrGpioUsbXmegaProgDevice() {
	close();
}



uint AvrGpioUsbXmegaProgDevice::getSclkFreq() {
	return 0;
}

void AvrGpioUsbXmegaProgDevice::open() {
	int rc;

	close();
	
	_usbdev.open(VENDOR_ID, PRODUCT_ID);
	
	cout << "successfully opened USB device: gpio-usb" << endl;
	
	_usbdev.controlTrans(GPIO_USB_REQTYPE, CMD_ENTER_PDI, 0, 0);
}

void AvrGpioUsbXmegaProgDevice::close() {
	_sip = 0;
	_mode = 0;
	if (_usbdev.isOpen()) {
		_usbdev.controlTrans(GPIO_USB_REQTYPE, CMD_LEAVE_PDI, 0, 0);
	}
	_usbdev.close();
}

void AvrGpioUsbXmegaProgDevice::sipCommit() {
	throw Exception("AvrGpioUsbXmegaProgDevice::sipCommit(): unimplemented");
}

void AvrGpioUsbXmegaProgDevice::sipRead() {
	throw Exception("AvrGpioUsbXmegaProgDevice::sipRead(): unimplemented");
}

uint32 AvrGpioUsbXmegaProgDevice::sendCmd(uint32 a_cmd, bool a_checkcomm) {
	throw Exception("AvrGpioUsbXmegaProgDevice::sendCmd(): unimplemented");
}



void AvrGpioUsbXmegaProgDevice::setProgMode(bool a_on) {
	if (a_on) {
		_usbdev.controlTrans(GPIO_USB_REQTYPE, CMD_NVMC_ENABLE, 0, 0);
		byte status = getStatus();
		if (status) {
			throw AvrException(String("AvrGpioUsbXmegaProgDevice::setProgMode(): failed to set programming mode on: status=")+(uint)status);
		}
	} else {
		_usbdev.controlTrans(GPIO_USB_REQTYPE, CMD_NVMC_DISABLE, 0, 0);
	}
}

byte AvrGpioUsbXmegaProgDevice::getStatus() {
	byte status = 0;
	int rc = _usbdev.controlTransIn(GPIO_USB_REQTYPE, CMD_PDI_GETSTATUS, 0, 0, &status, 1);
	if (rc != 1) {
		throw AvrException("AvrGpioUsbXmegaProgDevice::getStatus(): failed to get status");
	}
	return status;
}

void AvrGpioUsbXmegaProgDevice::checkStatus() {
	byte status = getStatus();
	if (status) {
		throw AvrError(String("PDI status: ") << (uint)status);
	}
}

void AvrGpioUsbXmegaProgDevice::cmdReadProgMem(uint a_addr, uint a_size, Array<uint16>& a_mem, Progress* a_pg) {
	a_mem.setSize(a_size);
	if (a_size == 0)
		return;
	
	uint addr = a_addr;
	uint offs = 0;
	for (uint i = a_size; i > 0; ) {
		uint sz = 0x200 - (addr & 0x0ff); // read 512 word chunks
		if (sz > i)
			sz = i;
		uint16* p = a_mem.data() + offs;
		int rc = _usbdev.controlTransIn(GPIO_USB_REQTYPE,
						CMD_NVMC_FLASH_READ, addr >> 16, addr & 0xffff, (byte*)p, sz * 2);
		if (rc != (sz * 2)) {
			checkStatus();
			throw AvrError("command CMD_NVMC_FLASH_READ: usb error");
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

void AvrGpioUsbXmegaProgDevice::cmdProgramProgMemPage(uint a_addr, const Array<uint16>& a_mem, uint a_offs, Progress* a_pg) {
	uint addr = a_addr & _chipdesc->flash_page_mask;
	int rc = _usbdev.controlTransOut(GPIO_USB_REQTYPE,
						CMD_NVMC_FLASH_WRITE_PAGE, addr >> 16, addr & 0x0ffff, (byte*)(a_mem.data() + a_offs),
						_chipdesc->flash_page_size * 2);
	if (rc != _chipdesc->flash_page_size * 2) {
		checkStatus();
		throw AvrError("command CMD_NVMC_FLASH_WRITE_PAGE: usb error");
	}
	checkStatus();
	if (a_pg) {
		for (uint k = 0; k < _chipdesc->flash_page_size; k++) {
			a_pg->next(addr + k, a_mem[a_offs + k]);
		}
	}
}

void AvrGpioUsbXmegaProgDevice::cmdReadEeprom(uint a_addr, uint a_size, Array<byte>& a_mem, Progress* a_pg) {
	a_mem.setSize(a_size);
	if (a_size == 0)
		return;
	
	uint addr = a_addr;
	uint offs = 0;
	for (uint i = a_size; i > 0; ) {
		uint sz = 128;
		if (sz > i)
			sz = i;
		byte* p = a_mem.data() + offs;
		int rc = _usbdev.controlTransIn(GPIO_USB_REQTYPE,
						CMD_NVMC_EEPROM_READ, addr>>16, addr & 0x0ffff, p, sz);
		if (rc != sz) {
			checkStatus();
			throw AvrError("command CMD_NVMC_EEPROM_READ: usb error");
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

void AvrGpioUsbXmegaProgDevice::cmdProgramEepromPage(uint a_addr, const Array<byte>& a_mem, uint a_size, uint a_offs, Progress* a_pg) {	
	uint addr = a_addr;
	int rc = _usbdev.controlTransOut(GPIO_USB_REQTYPE,
						CMD_NVMC_EEPROM_WRITE, addr>>16, addr & 0x0ffff, a_mem.data() + a_offs,
						a_size);
	if (rc != a_size) {
		checkStatus();
		throw AvrError("command CMD_NVMC_EEPROM_WRITE: usb error");
	}
	checkStatus();
	if (a_pg) {
		for (uint k = 0; k < a_size; k++) {
			a_pg->next(addr + k, a_mem[a_offs + k]);
		}
	}
}

uint16 AvrGpioUsbXmegaProgDevice::cmdReadProgMem(uint a_addr) {
	throw Exception("AvrGpioUsbXmegaProgDevice::cmdReadProgMem(): unimplemented");
}

void AvrGpioUsbXmegaProgDevice::cmdWriteEepromByte(uint a_addr, byte a_data) {
	int rc = _usbdev.controlTransOut(GPIO_USB_REQTYPE,
						CMD_NVMC_EEPROM_WRITE, a_addr>>16, a_addr & 0x0ffff, &a_data, 1);
	if (rc != 1) {
		checkStatus();
		throw AvrError("command CMD_NVMC_EEPROM_WRITE: usb error");
	}
	checkStatus();
}

void AvrGpioUsbXmegaProgDevice::cmdLoadProgMemPage(uint a_addr, uint16 a_data) {
	throw Exception("AvrGpioUsbXmegaProgDevice::cmdLoadProgMemPage(): unimplemented");
}
void AvrGpioUsbXmegaProgDevice::cmdWriteProgMemPage(uint a_addr) {
	throw Exception("AvrGpioUsbXmegaProgDevice::cmdWriteProgMemPage(): unimplemented");
}
byte AvrGpioUsbXmegaProgDevice::cmdReadEeprom(uint a_addr) {
	throw Exception("AvrGpioUsbXmegaProgDevice::cmdReadEeprom(): unimplemented");
}
void AvrGpioUsbXmegaProgDevice::cmdLoadEepromPage(uint a_addr, byte a_data) {
	throw Exception("AvrGpioUsbXmegaProgDevice::cmdLoadEepromPage(): unimplemented");
}
void AvrGpioUsbXmegaProgDevice::cmdWriteEepromPage(uint a_addr) {
	throw Exception("AvrGpioUsbXmegaProgDevice::cmdWriteEepromPage(): unimplemented");
}
byte AvrGpioUsbXmegaProgDevice::cmdReadLockBits() {
	throw Exception("AvrGpioUsbXmegaProgDevice::cmdReadLockBits(): unimplemented");
}
void AvrGpioUsbXmegaProgDevice::cmdWriteLockBits(byte a_bits) {
	throw Exception("AvrGpioUsbXmegaProgDevice::cmdWriteLockBits(): unimplemented");
}

uint32 AvrGpioUsbXmegaProgDevice::cmdReadSignature() {
	byte sign[6];
	int rc = _usbdev.controlTransIn(GPIO_USB_REQTYPE, CMD_NVMC_GET_DEVID, 0, 0, sign, 4);
	if (rc != 4) {
		checkStatus();
		throw AvrError("command CMD_NVMC_GET_DEVID: usb error");
	}
	checkStatus();
	return ((uint32)sign[0] << 16)|((uint32)sign[1] << 8)|sign[2];
}

byte AvrGpioUsbXmegaProgDevice::cmdReadFuseBits(uint a_fusenr) {
	byte fuse;
	int rc = _usbdev.controlTransIn(GPIO_USB_REQTYPE, CMD_NVMC_FUSE_READ, a_fusenr, 0, &fuse, 1);
	if (rc != 1) {
		checkStatus();
		throw AvrError("command CMD_NVMC_FUSE_READ: usb error");
	}
	checkStatus();
	return fuse;
}

void AvrGpioUsbXmegaProgDevice::cmdWriteFuseBits(uint a_fusenr, byte a_bits) {
	_usbdev.controlTrans(GPIO_USB_REQTYPE, CMD_NVMC_FUSE_WRITE, a_fusenr, a_bits);
	checkStatus();
}

byte AvrGpioUsbXmegaProgDevice::cmdReadCalibrationByte(uint a_addr) {
	byte val;
	int rc = _usbdev.controlTransIn(GPIO_USB_REQTYPE, CMD_NVMC_SIGNROW_READ, a_addr, 0, &val, 1);
	if (rc != 1) {
		checkStatus();
		throw AvrError("command CMD_NVMC_SIGNROW_READ: usb error");
	}
	checkStatus();
	return val;
}

bool AvrGpioUsbXmegaProgDevice::cmdPollBusy() {
	throw Exception("AvrGpioUsbXmegaProgDevice::cmdPollBusy(): unimplemented");
}

void AvrGpioUsbXmegaProgDevice::cmdChipErase(EraseMode mode) {
	if (mode == EraseMode_FLASH) {
		throw AvrError("cmdChipErase(): EraseMode_FLASH not supported!");
	} else {
		_usbdev.controlTrans(GPIO_USB_REQTYPE, CMD_NVMC_ERASE, mode, 0);
	}
	checkStatus();
}

void AvrGpioUsbXmegaProgDevice::cmdLoadExtAddrByte(byte a_extaddr) {
	throw Exception("AvrGpioUsbXmegaProgDevice::cmdLoadExtAddrByte(): unimplemented");
}

bool AvrGpioUsbXmegaProgDevice::cmdProgEnable() {
	// not needed
	return true;
}

void AvrGpioUsbXmegaProgDevice::progWait(uint a_ms) {
	// wait is done by programming device software
	//System::wait_ms(a_ms);
}

