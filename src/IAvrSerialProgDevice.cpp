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


#include "IAvrSerialProgDevice.h"
#include "AvrError.h"

IAvrSerialProgDevice::IAvrSerialProgDevice(const String& a_name, const AvrChipDesc* a_cd)
							: ISerialProgDevice(),
							  IAvrProgDevice(a_name, a_cd)
{
}

IAvrSerialProgDevice::~IAvrSerialProgDevice() {
}


void IAvrSerialProgDevice::setReadMode(bool a_on) {
	throw Exception("IAvrSerialProgDevice::setReadMode() unimplemented");
}

uint32 IAvrSerialProgDevice::sendCmd(uint32 a_cmd, bool a_checkcomm) {
	sipCommitClkWd(0, 0);
	uint32 recv = 0;
	for (int i = 31; i >= 0; i--) {
		recv <<= 1;
		if (sipReadRd())
			recv |= 1;
		
		sipCommitWd((a_cmd & (1ul << i)) != 0);
		sipCommitClk(1);
		sipCommitClk(0);
	}
	sipCommitClkWd(0, 0);
	if (a_checkcomm && (((a_cmd >> 8) & 0x00ffff00) != (recv & 0x00ffff00))) {
		throw AvrException("IAvrSerialProgDevice::sendCmd(): communication error");
	}
	return recv;
}

void IAvrSerialProgDevice::cmdReadProgMem(uint a_addr, uint a_size, Array<uint16>& a_mem, Progress* a_pg) {
	a_mem.setSize(a_size);
	if (a_size == 0)
		return;
	bool needextaddr = _chipdesc->flash_size > 0x10000;
	byte extaddr = (byte)(a_addr >> 16);
	if (needextaddr)
		cmdLoadExtAddrByte(extaddr);
	for (uint i = 0; i < a_size; i++) {
		uint addr = a_addr + i;
		if (needextaddr && (extaddr != (byte)(addr >> 16))) {
			extaddr = (byte)(addr >> 16);
			cmdLoadExtAddrByte(extaddr);
		}
		a_mem[i] = cmdReadProgMem(addr & 0x0ffff);
		if (a_pg)
			a_pg->next(addr, a_mem[i]);
	}
}

void IAvrSerialProgDevice::cmdProgramProgMemPage(uint a_addr, const Array<uint16>& a_mem, uint a_offs, Progress* a_pg) {
	bool needextaddr = _chipdesc->flash_size > 0x10000;	
	if (needextaddr)
		cmdLoadExtAddrByte((byte)(a_addr >> 16));
	
	for (uint i = 0; i < _chipdesc->flash_page_size; i++) {
		cmdLoadProgMemPage(i, a_mem[a_offs+i]);
		if (a_pg)
			a_pg->next((a_addr & _chipdesc->flash_page_mask)+i, a_mem[a_offs+i]);
	}
	cmdWriteProgMemPage(a_addr & _chipdesc->flash_page_mask & 0x0ffff);
}

void IAvrSerialProgDevice::cmdReadEeprom(uint a_addr, uint a_size, Array<byte>& a_mem, Progress* a_pg) {
	a_mem.setSize(a_size);
	for (uint i = 0; i < a_size; i++) {
		a_mem[i] = cmdReadEeprom(a_addr+i);
		if (a_pg)
			a_pg->next(a_addr+i, a_mem[i]);
	}
}

void IAvrSerialProgDevice::cmdProgramEepromPage(uint a_addr, const Array<byte>& a_mem, uint a_size, uint a_offs, Progress* a_pg) {	
	for (uint i = 0; i < a_size; i++) {
		cmdLoadEepromPage(i, a_mem[a_offs+i]);
		if (a_pg)
			a_pg->next((a_addr & _chipdesc->eeprom_page_mask)+i, a_mem[a_offs+i]);
	}
	cmdWriteEepromPage(a_addr & _chipdesc->eeprom_page_mask & 0x0ffff);
}


uint16 IAvrSerialProgDevice::cmdReadProgMem(uint a_addr) {
	uint a = ((a_addr & _chipdesc->flash_mask) << 8) & 0x00ffff00;
	
	uint32 bl = sendCmd(0x20000000ul | a);
	uint32 bh = sendCmd(0x28000000ul | a);
	
	return (bh << 8) | (bl & 0x00ff);
}

void IAvrSerialProgDevice::cmdLoadProgMemPage(uint a_addr, uint16 a_data) {
	uint32 a = ((a_addr & _chipdesc->flash_poff_mask) << 8) & 0x00ffff00;
	sendCmd(0x40000000ul | a | (byte)a_data);
	sendCmd(0x48000000ul | a | (byte)(a_data >> 8));
}

void IAvrSerialProgDevice::cmdWriteProgMemPage(uint a_addr) {
	sendCmd(0x4c000000ul | ((a_addr & _chipdesc->flash_page_mask & 0x0ffff) << 8));
	progWait(_chipdesc->spi_twd_flash+1);
}

byte IAvrSerialProgDevice::cmdReadEeprom(uint a_addr) {
	uint32 a = ((a_addr & _chipdesc->eeprom_mask) << 8) & 0x00ffff00;
	uint32 r = sendCmd(0xa0000000ul | a);
	return r;
}

void IAvrSerialProgDevice::cmdWriteEepromByte(uint a_addr, byte a_data) {
	uint32 a = ((a_addr & _chipdesc->eeprom_mask) << 8) & 0x00ffff00;
	sendCmd(0xc0000000ul | a | a_data);
	progWait(_chipdesc->spi_twd_eeprom+1);
}

void IAvrSerialProgDevice::cmdLoadEepromPage(uint a_addr, byte a_data) {
	uint32 a = ((a_addr & _chipdesc->eeprom_poff_mask) << 8) & 0x00ffff00;
	sendCmd(0xc1000000ul | a | a_data);
}

void IAvrSerialProgDevice::cmdWriteEepromPage(uint a_addr) {
	uint32 a = ((a_addr & _chipdesc->eeprom_page_mask) << 8) & 0x00ffff00;
	sendCmd(0xc2000000ul | a);
	progWait(_chipdesc->spi_twd_eeprom+1);
}

byte IAvrSerialProgDevice::cmdReadLockBits() {
	return sendCmd(0x58000000ul);
}

void IAvrSerialProgDevice::cmdWriteLockBits(byte a_bits) {
	sendCmd(0xace000c0ul | (a_bits & 0x3f));
	progWait(_chipdesc->spi_twd_fuse+1);
}

uint32 IAvrSerialProgDevice::cmdReadSignature() {
	byte s0 = sendCmd(0x30000000ul | (0 << 8));
	byte s1 = sendCmd(0x30000000ul | (1 << 8));
	byte s2 = sendCmd(0x30000000ul | (2 << 8));
	return ((uint32)s0 << 16)|((uint32)s1 << 8)|(uint32)s2;
}

byte IAvrSerialProgDevice::cmdReadFuseBits(uint a_fusenr) {
	switch(a_fusenr) {
		case 0:
			return sendCmd(0x50000000ul);
		case 1:
			return sendCmd(0x58080000ul);
		case 2:
			return sendCmd(0x50080000ul);
		default:
			return 0xff;
	}
}

void IAvrSerialProgDevice::cmdWriteFuseBits(uint a_fusenr, byte a_bits) {
	switch(a_fusenr) {
		case 0:
			sendCmd(0xaca00000ul | a_bits);
			break;
		case 1:
			sendCmd(0xaca80000ul | a_bits);
			break;
		case 2:
			sendCmd(0xaca40000ul | a_bits);
			break;
		default:
			break;
	}
	progWait(_chipdesc->spi_twd_fuse+1);
}

byte IAvrSerialProgDevice::cmdReadCalibrationByte(uint a_addr) {
	return sendCmd(0x38000000ul | ((a_addr & 0x0001) << 8));
}

bool IAvrSerialProgDevice::cmdPollBusy() {
	uint32 r = sendCmd(0xf0000000ul);
	return (r & 0x0001) != 0;
}

void IAvrSerialProgDevice::cmdChipErase(EraseMode mode) {
	sendCmd(0xac800000ul);
	progWait(_chipdesc->spi_twd_erase+1);
}

bool IAvrSerialProgDevice::cmdProgEnable() {
	uint32 r = sendCmd(0xac530000ul, false);
	return (r & 0x0000fffful) == 0x00005300ul;
}

void IAvrSerialProgDevice::cmdLoadExtAddrByte(byte a_extaddr) {
	sendCmd(0x4d000000ul | ((uint)a_extaddr << 8));
}

