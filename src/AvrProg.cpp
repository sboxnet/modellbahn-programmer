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

#include "AvrProg.h"

#include <tmb/lang/System.h>

AvrProg::AvrProg(IAvrProgDevice* a_pdev, const AvrChipDescriptions& a_cds)
		: _pdev(a_pdev),
		  _chips(a_cds),
		  _sig_checked(false),
		  _prog_enabled(false)    {
	if (!_pdev)
		throw AvrException("AvrProg: no programming device specified");
	
	_chipdesc = _pdev->getChipDesc();
}

AvrProg::~AvrProg() {
	_pdev->close();
	delete _pdev;
}
		
void AvrProg::checkReady() {
	if (!_prog_enabled)
		throw AvrNotReadyError("chip programming not enabled");
	if (!_sig_checked) {
		throw AvrNotReadyError("chip signature still not checked");
	}
}


void AvrProg::initProg() {
	_sig_checked = false;
	_prog_enabled = false;
	_pdev->open();
}
	
void AvrProg::enableProg() {
	_sig_checked = false;
	_prog_enabled = false;
        _pdev->setProgMode(true);
        if (!_pdev->programEnable()) {
            _pdev->setProgMode(false);
            throw AvrOutOfSyncError("out of sync");
        }
    
	_prog_enabled = true;
}

void AvrProg::cleanupProg() {
	_sig_checked = false;
	_prog_enabled = false;
	_pdev->setProgMode(false);
	_pdev->close();
}

void AvrProg::checkSignature() {
	if (!_chipdesc)
		throw AvrNotReadyError("no chip description specified");
	if (!_prog_enabled)
		throw AvrNotReadyError("chip programming not enabled");
	
	uint32 sign = _pdev->readSignature();

	AvrChipDesc* cd = _chips.find(sign);
	if (!cd)
		throw AvrError(String(256).format("unknown chip with signature 0x%x", sign));
	if (_chipdesc != cd)
		throw AvrError(String(256).format("found another chip \'%s\', it is not \'%s\'",
											cd->name.c_str(), _chipdesc->name.c_str()));
	_sig_checked = true;
}

uint32 AvrProg::readSignature() {
	if (!_prog_enabled)
		throw AvrNotReadyError("chip programming not enabled");
	return _pdev->readSignature();
}

byte AvrProg::readLockBits() {
	checkReady();
	if (_chipdesc->prginterface == AvrChipDesc::PrgInterface_PDI) {
		throw AvrError("PLEASE USE fuse read to read locks bits.");
	}
	return _pdev->readLockBits();
}

void AvrProg::writeLockBits(byte a_lb) {
	checkReady();
	if (_chipdesc->prginterface == AvrChipDesc::PrgInterface_PDI) {
		throw AvrError("PLEASE USE fuse write to set locks bits.");
	}
	_pdev->writeLockBits(a_lb);
}

void AvrProg::checkFuseNr(uint a_fusenr) {
	if (a_fusenr < _chipdesc->fuse_desc.size()) {
		if (_chipdesc->fuse_desc[a_fusenr]->size() > 0)
			return;
	}
	throw AvrError(String(256).format("AvrProg: device has no fuse with number %d", (int)a_fusenr));
}

byte AvrProg::readFuseBits(uint a_fusenr) {
	checkReady();
	checkFuseNr(a_fusenr);
	return _pdev->readFuseBits(a_fusenr);
}

void AvrProg::writeFuseBits(uint a_fusenr, byte a_fb) {
	checkReady();
	checkFuseNr(a_fusenr);
	_pdev->writeFuseBits(a_fusenr, a_fb);
}


void AvrProg::readCalibrationBytes(Array<byte>& a_calbytes) {
	checkReady();
	a_calbytes.clear();
	for (uint i = 0; i < _chipdesc->calbytes; i++) {
		a_calbytes.add(_pdev->readCalibrationByte(i));
	}
}

void AvrProg::eraseChip(IAvrProgDevice::EraseMode mode) {
	checkReady();
	_pdev->chipErase(mode);
}

bool AvrProg::verifyChipErased(Progress* a_progress, IAvrProgDevice::EraseMode mode) {
	checkReady();
	
	bool verify_ok = true;
	if (mode == IAvrProgDevice::EraseMode_CHIP || mode == IAvrProgDevice::EraseMode_FLASH
			|| IAvrProgDevice::EraseMode_APP_SECTION || mode == IAvrProgDevice::EraseMode_BOOT_SECTION) {
		uint startaddr = 0;
		uint fsize = 0;
		if (mode == IAvrProgDevice::EraseMode_CHIP || mode == IAvrProgDevice::EraseMode_FLASH) {
			startaddr = 0;
			fsize = _chipdesc->flash_size;
		} else if (mode == IAvrProgDevice::EraseMode_APP_SECTION) {
			startaddr = 0;
			fsize = _chipdesc->flash_app_size;
		} else if (mode == IAvrProgDevice::EraseMode_BOOT_SECTION) {
			startaddr = _chipdesc->flash_app_size;
			fsize = _chipdesc->flash_boot_size;
		}
		if (fsize > 0) {
			if (a_progress)
				a_progress->init(PSFLG_SIZE_WORD, startaddr, fsize - 1);
			Array<uint16> flash;
			_pdev->readProgMem(startaddr, fsize, flash, a_progress);
			for (uint addr = 0; addr < fsize; addr++) {
				if (flash[addr] != 0xffff)
					verify_ok = false;
			}
		}
	}
	if (_chipdesc->eeprom_size > 0 && (mode == IAvrProgDevice::EraseMode_CHIP || mode == IAvrProgDevice::EraseMode_EEPROM)) {
		if (a_progress)
			a_progress->init(PSFLG_IS_EEPROM, 0, _chipdesc->eeprom_size - 1);
		Array<byte> eeprom;
		_pdev->readEeprom(0, _chipdesc->eeprom_size, eeprom, a_progress);
		for (uint addr = 0; addr < _chipdesc->eeprom_size; addr++) {
			if (eeprom[addr] != 0xff)
				verify_ok = false;
		}
	}
	return verify_ok;
}


void AvrProg::readProgMem(Progress* a_progress, Array<uint16>& a_code, uint a_startaddr, int a_size) {
	checkReady();
	
	if (a_startaddr >= _chipdesc->flash_size)
		throw AvrInvalidParameterError("start address out of flash memory address space");
	
	uint size = a_size;
	if (a_size < 0)
		size = _chipdesc->flash_size - a_startaddr;
	if (size == 0) {
		throw AvrInvalidParameterError("requested flash memory read size is 0");
	}
	
	uint endaddr = a_startaddr + size - 1;
	
	if (size > _chipdesc->flash_size || endaddr >= _chipdesc->flash_size)
		throw AvrInvalidParameterError("requested read size out of flash memory address space");
	
	if (a_progress)
		a_progress->init(PSFLG_SIZE_WORD, a_startaddr, endaddr);
	_pdev->readProgMem(a_startaddr, size, a_code, a_progress);
}

bool AvrProg::writeProgMem(Progress* a_progress, const Array<uint16>& a_code, uint a_startaddr) {
	checkReady();

	uint size = a_code.size();
	if (a_startaddr >= _chipdesc->flash_size)
		throw AvrInvalidParameterError("start address out of flash memory address space");
	if (size == 0)
		throw AvrInvalidParameterError("requested flash memory write size is 0");
	uint endaddr = a_startaddr + size - 1;

	if (size > _chipdesc->flash_size || endaddr >= _chipdesc->flash_size)
		throw AvrInvalidParameterError("requested write size out of flash memory address space");
	
	// program
	if (a_progress)
		a_progress->init(PSFLG_SIZE_WORD, a_startaddr & _chipdesc->flash_page_mask,
						 (endaddr & _chipdesc->flash_page_mask) + _chipdesc->flash_page_size - 1);
	Array<uint16> pagebuf(_chipdesc->flash_page_size, 0);
	bool needwrite = false; // only values != 0xff must be programmed
	for (uint addr = a_startaddr; addr <= endaddr; addr++) {
		uint page = addr & _chipdesc->flash_page_mask;
		uint offs = addr & _chipdesc->flash_poff_mask;
		
		if (addr == a_startaddr && offs > 0) { // if a_startaddr is not on a page boundary
			if (_chipdesc->prginterface == AvrChipDesc::PrgInterface_PDI) {
				Array<uint16> pw;
				_pdev->readProgMem(page, offs, pw);
				for (uint i = 0; i < offs; i++) {
					pagebuf[i] = pw[i];
				}
			} else {
				for (uint i = 0; i < offs; i++) {
					pagebuf[i] = 0xffff;		// only 0's are programmed...
				}
			}
		}
		
		pagebuf[offs] = a_code[addr - a_startaddr];
		if (_chipdesc->prginterface == AvrChipDesc::PrgInterface_PDI || pagebuf[offs] != (uint16)0xffff) {
			needwrite = true;
		}
		
		if (addr == endaddr && offs < _chipdesc->flash_poff_mask) { // fill up memory page if necessary
			if (_chipdesc->prginterface == AvrChipDesc::PrgInterface_PDI) {
				Array<uint16> pw;
				int n = _chipdesc->flash_page_size - 1 - offs;
				_pdev->readProgMem(addr+1, n, pw);
				for (int i = 0; i < n; i++) {
					pagebuf[++offs] = pw[i];
				}
			} else {
				int n = _chipdesc->flash_page_size - 1 - offs;
				for (int i = 0; i < n; i++) {
					pagebuf[++offs] = 0xffff;
				}
			}
		}
		
		if (offs == _chipdesc->flash_poff_mask) { // page loaded? if yes, then write it
			if (needwrite) {
				_pdev->writeProgMemPage(page, pagebuf, 0, a_progress);
			} else if (a_progress) {
				a_progress->next(page+offs, 0); // advance progress meter
			}
			needwrite = false;
		}
	}
	
	// verify
	bool verify_ok = true;
	if (a_progress)
		a_progress->init(PSFLG_WRITE_VER|PSFLG_SIZE_WORD, a_startaddr, endaddr);
	Array<uint16> flash;
	_pdev->readProgMem(a_startaddr, size, flash, a_progress);
	for (uint i = 0; i < size; i++) {
		if (flash[i] != a_code[i])
			verify_ok = false;
	}	
	
	return verify_ok;
}

bool AvrProg::writeProgMem(Progress* a_progress, const AvrElf& a_elfobj, uint a_startaddr) {
	
	Array<uint16> code;
	a_elfobj.getCode(code);
	
	return writeProgMem(a_progress, code, a_startaddr);
}

bool AvrProg::verifyProgMem(Progress* a_progress, const Array<uint16>& a_code, uint a_startaddr) {
	checkReady();

	uint size = a_code.size();
	if (a_startaddr >= _chipdesc->flash_size)
		throw AvrInvalidParameterError("start address out of flash memory address space");
	if (size == 0)
		throw AvrInvalidParameterError("requested flash memory verify size is 0");
	uint endaddr = a_startaddr + size - 1;

	if (size > _chipdesc->flash_size || endaddr >= _chipdesc->flash_size)
		throw AvrInvalidParameterError("requested size out of flash memory address space");
	
	bool verify_ok = true;
	if (a_progress)
		a_progress->init(PSFLG_SIZE_WORD, a_startaddr, endaddr);
	Array<uint16> flash;
	_pdev->readProgMem(a_startaddr, size, flash, a_progress);
	for (uint i = 0; i < size; i++) {
		if (flash[i] != a_code[i])
			verify_ok = false;
	}	
	return verify_ok;
}

bool AvrProg::verifyProgMem(Progress* a_progress, const AvrElf& a_elfobj, uint a_startaddr) {
	
	Array<uint16> code;
	a_elfobj.getCode(code);
	
	return verifyProgMem(a_progress, code, a_startaddr);
}


void AvrProg::readEeprom(Progress* a_progress, Array<byte>& a_data, uint a_startaddr, int a_size) {
	checkReady();
	
	if (a_startaddr >= _chipdesc->eeprom_size)
		throw AvrInvalidParameterError("start address out of EEPROM memory address space");
	
	uint size = a_size;
	if (a_size < 0)
		size = _chipdesc->eeprom_size - a_startaddr;
	if (size == 0)
		throw AvrInvalidParameterError("requested EEPROM read size is 0");
	uint endaddr = a_startaddr + size - 1;
	
	if (size > _chipdesc->eeprom_size || endaddr >= _chipdesc->eeprom_size)
		throw AvrInvalidParameterError("requested read size out of EEPROM memory address space");
		
	if (a_progress)
		a_progress->init(PSFLG_IS_EEPROM, a_startaddr, endaddr);
	_pdev->readEeprom(a_startaddr, size, a_data, a_progress);
}

bool AvrProg::writeEeprom(Progress* a_progress, const Array<byte>& a_data, uint a_startaddr) {
	checkReady();
	
	uint size = a_data.size();
	if (a_startaddr >= _chipdesc->eeprom_size)
		throw AvrInvalidParameterError("start address out of EEPROM memory address space");
	if (size == 0)
		throw AvrInvalidParameterError("requested EEPROM write size is 0");
	uint endaddr = a_startaddr + size - 1;

	if (size > _chipdesc->eeprom_size || endaddr >= _chipdesc->eeprom_size)
		throw AvrInvalidParameterError("requested write size out of EEPROM memory address space");
	
	if (_chipdesc->prginterface == AvrChipDesc::PrgInterface_SPI
		   && (!_chipdesc->spi_has_eeprom_page_write || size < _chipdesc->eeprom_page_size) ) {
		// program bytes
		if (a_progress)
			a_progress->init(PSFLG_IS_EEPROM, a_startaddr, endaddr);
		for (uint addr = a_startaddr; addr <= endaddr; addr++) {
			byte w = a_data[addr - a_startaddr];
			_pdev->writeEepromByte(addr, w, a_progress); // löscht und setzt die EEPROM Zelle
		}
		
	} else if (_chipdesc->prginterface == AvrChipDesc::PrgInterface_PDI) {
		if (a_progress)
			a_progress->init(PSFLG_IS_EEPROM, a_startaddr, endaddr);
		// PDI: eeprom page write only writes entries set in the chip's eeprom page buffer
		for (uint addr = a_startaddr; addr <= endaddr; ) {
			uint poffs = addr & _chipdesc->eeprom_poff_mask;
			uint psize = _chipdesc->eeprom_page_size - poffs;
			if (addr + psize - 1 > endaddr) {
				psize = endaddr - addr + 1;
			}
			_pdev->writeEepromPage(addr, a_data, psize, addr - a_startaddr, a_progress);
			addr += psize;
		}
		
	} else {
		// program pages
		if (a_progress)
			a_progress->init(PSFLG_IS_EEPROM, a_startaddr & _chipdesc->eeprom_page_mask,
							 (endaddr & _chipdesc->eeprom_page_mask) + _chipdesc->eeprom_page_size - 1);
		Array<byte> pagebuf(_chipdesc->eeprom_page_size, 0);
		bool needwrite = false;
		for (uint addr = a_startaddr; addr <= endaddr; addr++) {
			uint page = addr & _chipdesc->eeprom_page_mask;
			uint offs = addr & _chipdesc->eeprom_poff_mask;
			
			if (addr == a_startaddr && offs > 0) { // if a_startaddr is not on a page boundary
				for (uint i = 0; i < offs; i++) {
					pagebuf[i] = 0xff;		// only 0's are programmed...
				}
			}
			
			pagebuf[offs] = a_data[addr - a_startaddr];
			if (pagebuf[offs] != (byte)0xff) {
				needwrite = true;
			}
			if (addr == endaddr && offs < _chipdesc->eeprom_poff_mask) { // fill up memory page if necessary
				int n = _chipdesc->eeprom_page_size - 1 - offs;
				for (int i = 0; i < n; i++) {
					pagebuf[++offs] = 0xff;
				}
			}
			
			if (offs == _chipdesc->eeprom_poff_mask) { // page loaded? if yes, then write it
				if (needwrite) {
					_pdev->writeEepromPage(page, pagebuf, _chipdesc->eeprom_page_size, 0, a_progress);
				} else if (a_progress) {
					a_progress->next(page+offs, 0);
				}
				needwrite = false;
			}
		}
	}

	
	// verify
	bool verify_ok = true;
	if (a_progress)
		a_progress->init(PSFLG_IS_EEPROM|PSFLG_WRITE_VER, a_startaddr, endaddr);
	Array<byte> data;
	_pdev->readEeprom(a_startaddr, size, data, a_progress);
	for (uint i = 0; i < size; i++) {
		if (data[i] != a_data[i])
			verify_ok = false;
	}
	return verify_ok;
}

bool AvrProg::verifyEeprom(Progress* a_progress, const Array<byte>& a_data, uint a_startaddr) {
	checkReady();
	
	uint size = a_data.size();
	if (a_startaddr >= _chipdesc->eeprom_size)
		throw AvrInvalidParameterError("start address out of EEPROM memory address space");
	if (size == 0)
		throw AvrInvalidParameterError("requested EEPROM verify size is 0");
	uint endaddr = a_startaddr + size - 1;

	if (size > _chipdesc->eeprom_size || endaddr >= _chipdesc->eeprom_size)
		throw AvrInvalidParameterError("requested size out of EEPROM memory address space");
	
	bool verify_ok = true;
	if (a_progress)
		a_progress->init(PSFLG_IS_EEPROM, a_startaddr, endaddr);
	Array<byte> data;
	_pdev->readEeprom(a_startaddr, size, data, a_progress);
	for (uint i = 0; i < size; i++) {
		if (data[i] != a_data[i])
			verify_ok = false;
	}
	return verify_ok;
}

