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


#include "AvrGpioSerAvrProgDevice.h"
#include "AvrError.h"

AvrGpioSerAvrProgDevice::AvrGpioSerAvrProgDevice(const String& a_devfile, const AvrChipDesc* a_cd, uint a_sclkfreq, uint a_flags)
							: AvrGpioSerAvrRawProgDevice(a_devfile, a_cd, a_sclkfreq, a_flags)
{
	_name = "AvrGpioSerAvrProgDevice";
	if (a_cd->arch != AvrChipDesc::Arch_AVR || a_cd->prginterface != AvrChipDesc::PrgInterface_SPI) {
		throw AvrException(_name + " does not support chip " + a_cd->name);
	}
}

AvrGpioSerAvrProgDevice::~AvrGpioSerAvrProgDevice() {
}
		


class MyProgress : public GpioSer::IProgress {
	private: IAvrProgDevice::Progress*	_pg;
	private: uint                       _extaddr;
	public: MyProgress(IAvrProgDevice::Progress* a_pg, uint a_extaddr) : _pg(a_pg), _extaddr(a_extaddr) {}
	public: void extaddr(uint a_extaddr) { _extaddr = a_extaddr; }
	public: virtual void next(uint addr, uint val) {
				if (_pg) {
					_pg->next(_extaddr + addr, val);
//cout << hex << "0x" << (_extaddr+addr) << ": 0x" << val << dec << endl << endl;
				}
			}
};

void AvrGpioSerAvrProgDevice::cmdReadProgMem(uint a_addr, uint a_size, Array<uint16>& a_mem, Progress* a_pg) {
	a_mem.setSize(a_size);
	if (a_size == 0)
		return;
	MyProgress mpg(a_pg, 0);
	try {
		if (_chipdesc->flash_size > 0x10000) { // need extended address...
			uint addr = a_addr;
			a_mem.clear();
			Array<uint16> buf;
			// read based on 64k boundaries
			for (uint i = a_size; i > 0; ) {
				buf.clear();
				mpg.extaddr(addr & 0xffff0000);
				cmdLoadExtAddrByte(addr >> 16);
				uint sz = 0x10000 - (addr & 0x0ffff);
				if (sz > i)
					sz = i;
				_gpioser.cmdAvrReadProgMem(addr & 0x0ffff, sz, buf, &mpg);
				i -= sz;
				addr += sz;
				for (uint k = 0; k < buf.size(); k++) {
					a_mem.add(buf[k]);
				}
			}
		} else {
			_gpioser.cmdAvrReadProgMem(a_addr, a_size, a_mem, &mpg);
		}
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

void AvrGpioSerAvrProgDevice::cmdProgramProgMemPage(uint a_addr, const Array<uint16>& a_mem, uint a_offs, Progress* a_pg) {
	try {
		if (_chipdesc->flash_size > 0x10000)
			cmdLoadExtAddrByte(a_addr >> 16);
		MyProgress mpg(a_pg, a_addr & 0xffff0000);
		_gpioser.cmdAvrWriteProgMemPage(a_addr & 0x0ffff, _chipdesc->flash_page_exp, a_mem, a_offs, &mpg);
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

void AvrGpioSerAvrProgDevice::cmdReadEeprom(uint a_addr, uint a_size, Array<byte>& a_mem, Progress* a_pg) {
	a_mem.setSize(a_size);
	if (a_size == 0)
		return;
	MyProgress mpg(a_pg, 0);
	try {
		_gpioser.cmdAvrReadEeprom(a_addr, a_size, a_mem, &mpg);
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

void AvrGpioSerAvrProgDevice::cmdProgramEepromPage(uint a_addr, const Array<byte>& a_mem, uint a_size, uint a_offs, Progress* a_pg) {	
	MyProgress mpg(a_pg, 0);
	try {
		_gpioser.cmdAvrWriteEepromPage(a_addr, _chipdesc->eeprom_page_exp, a_mem, a_offs, &mpg);
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

void AvrGpioSerAvrProgDevice::cmdWriteEepromByte(uint a_addr, byte a_data) {
	try {
		_gpioser.cmdAvrWriteEepromByte(a_addr, a_data);
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

byte AvrGpioSerAvrProgDevice::cmdReadLockBits() {
	try {
		return _gpioser.cmdAvrReadLockBits();
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

void AvrGpioSerAvrProgDevice::cmdWriteLockBits(byte a_bits) {
	try {
		_gpioser.cmdAvrWriteLockBits(a_bits);
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

uint32 AvrGpioSerAvrProgDevice::cmdReadSignature() {
	try {
		return _gpioser.cmdAvrReadSignature();
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

byte AvrGpioSerAvrProgDevice::cmdReadFuseBits(uint a_fusenr) {
	try {
		return _gpioser.cmdAvrReadFuseBits(a_fusenr);
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

void AvrGpioSerAvrProgDevice::cmdWriteFuseBits(uint a_fusenr, byte a_bits) {
	try {
		_gpioser.cmdAvrWriteFuseBits(a_fusenr, a_bits);
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

byte AvrGpioSerAvrProgDevice::cmdReadCalibrationByte(uint a_addr) {
	try {
		return _gpioser.cmdAvrReadCalibrationByte(a_addr);
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

void AvrGpioSerAvrProgDevice::cmdChipErase(EraseMode mode) {
	try {
		_gpioser.cmdAvrChipErase();
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

void AvrGpioSerAvrProgDevice::cmdLoadExtAddrByte(byte a_extaddr) {
	try {
		_gpioser.cmdAvrLoadExtAddrByte(a_extaddr);
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

bool AvrGpioSerAvrProgDevice::cmdProgEnable() {
	try {
		return _gpioser.cmdAvrProgramEnable();
	} catch(const GpioSer::AvrException& ex) {
		throw AvrException(ex.msg());
	}
}

void AvrGpioSerAvrProgDevice::progWait(uint a_ms) {
	// wait is done by programming device software
	//System::wait_ms(a_ms);
}
