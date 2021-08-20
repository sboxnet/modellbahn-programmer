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


#include "IAvrProgDevice.h"
#include "AvrError.h"

#include <tmb/lang/System.h>


IAvrProgDevice::IAvrProgDevice(const String& a_name, const AvrChipDesc* a_cd)
		: _name(a_name), _chipdesc(a_cd)
{
	if (_chipdesc == NULL) {
		throw Exception("IAvrProgDevice: required chip description not set");
	}
}

const String& IAvrProgDevice::name() const {
	return _name;
}

const AvrChipDesc* IAvrProgDevice::getChipDesc() const {
	return _chipdesc;
}



void IAvrProgDevice::readProgMem(uint a_addr, uint a_size, Array<uint16>& a_mem, Progress* a_pg) {
	if (a_addr >=_chipdesc->flash_size || (a_addr+a_size) > _chipdesc->flash_size || a_size > _chipdesc->flash_size)
		throw AvrAddressError("IAvrProgDevice::readProgMem(): address beyond program memory size");
	cmdReadProgMem(a_addr, a_size, a_mem, a_pg);
}

void IAvrProgDevice::writeProgMemPage(uint a_addr, const Array<uint16>& a_mem, uint a_offs, Progress* a_pg) {
	if (a_addr >=_chipdesc->flash_size)
		throw AvrAddressError("IAvrProgDevice::writeProgMemPage(): address beyond program memory size");
	if (a_addr & _chipdesc->flash_poff_mask)
		throw AvrAddressError("IAvrProgDevice::writeProgMemPage(): address not on page boundary");
	if (a_mem.size() < a_offs + _chipdesc->flash_page_size)
		throw AvrInvalidParameterError("IAvrProgDevice::writeProgMemPage(): not a page in program word array");
	cmdProgramProgMemPage(a_addr, a_mem, a_offs, a_pg);
}

void IAvrProgDevice::readEeprom(uint a_addr, uint a_size, Array<byte>& a_mem, Progress* a_pg) {
	if (a_addr >=_chipdesc->eeprom_size || (a_addr+a_size) > _chipdesc->eeprom_size || a_size > _chipdesc->eeprom_size)
		throw AvrAddressError("IAvrProgDevice::readEeprom(): address beyond EEPROM size");
	cmdReadEeprom(a_addr, a_size, a_mem, a_pg);
}

void IAvrProgDevice::writeEepromPage(uint a_addr, const Array<byte>& a_mem, uint a_size, uint a_offs, Progress* a_pg) {
	if (a_addr + a_size > _chipdesc->eeprom_size)
		throw AvrAddressError("IAvrProgDevice::writeEepromPage(): address beyond EEPROM size");
	if (a_size == 0)
		throw AvrInvalidParameterError("IAvrProgDevice::writeEepromPage(): size is 0");
	if (_chipdesc->prginterface == AvrChipDesc::PrgInterface_SPI) {
		if (a_addr & _chipdesc->eeprom_poff_mask)
			throw AvrAddressError("IAvrProgDevice::writeEepromPage(): address not on page boundary");
		if (a_size != _chipdesc->eeprom_page_size)
			throw AvrInvalidParameterError("IAvrProgDevice::writeEepromPage():size is not equal eeprom page size");
	}
	if (a_mem.size() < a_offs + a_size)
		throw AvrInvalidParameterError("IAvrProgDevice::writeEepromPage(): not a page in EEPROM byte array");
	cmdProgramEepromPage(a_addr, a_mem, a_size, a_offs, a_pg);
}

void IAvrProgDevice::writeEepromByte(uint a_addr, byte a_byte, Progress* a_pg) {
	if (a_addr >=_chipdesc->eeprom_size)
		throw AvrAddressError("IAvrProgDevice::writeEepromByte(): address beyond EEPROM size");
	cmdWriteEepromByte(a_addr, a_byte);
	if (a_pg)
		a_pg->next(a_addr, a_byte);
}

byte IAvrProgDevice::readLockBits() {
	return cmdReadLockBits();
}

void IAvrProgDevice::writeLockBits(byte a_bits) {
	cmdWriteLockBits(a_bits);
}

uint32 IAvrProgDevice::readSignature() {
	return cmdReadSignature();
}

byte IAvrProgDevice::readFuseBits(uint a_fusenr) {
	if (a_fusenr < _chipdesc->fuse_desc.size()) {
		if (_chipdesc->fuse_desc[a_fusenr]->size() > 0) {
			return cmdReadFuseBits(a_fusenr);
		}
	}
	throw AvrInvalidParameterError("IAvrProgDevice::readFuseBits(): fuse does not exist: "+a_fusenr);
}

void IAvrProgDevice::writeFuseBits(uint a_fusenr, byte a_fuses) {
	if (a_fusenr < _chipdesc->fuse_desc.size()) {
		if (_chipdesc->fuse_desc[a_fusenr]->size() > 0) {
			cmdWriteFuseBits(a_fusenr, a_fuses);
			return;
		}
	}
	throw AvrInvalidParameterError("IAvrProgDevice::writeFuseBits(): fuse does not exist: "+a_fusenr);
}

byte IAvrProgDevice::readCalibrationByte(uint a_addr) {
	if (a_addr >= _chipdesc->calbytes)
		throw AvrInvalidParameterError("IAvrProgDevice::readCalibrationByte(): calibration byte does not exist: "+a_addr);
	return cmdReadCalibrationByte(a_addr);
}

void IAvrProgDevice::chipErase(EraseMode mode) {
	if (_chipdesc->prginterface == AvrChipDesc::PrgInterface_SPI && mode != EraseMode_CHIP) {
		throw AvrInvalidParameterError("IAvrProgDevice::chipErase(): erase mode not supported");
	}
	cmdChipErase(mode);
}

bool IAvrProgDevice::programEnable() {
	return cmdProgEnable();
}

void IAvrProgDevice::progWait(uint a_ms) {
	System::wait_ms(a_ms);
}

