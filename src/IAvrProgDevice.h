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

#ifndef _IAVRPROGDEVICE_H_
#define _IAVRPROGDEVICE_H_

#include <tmb/lang/Exception.h>
#include <tmb/lang/String.h>
#include <tmb/lang/Array.h>
#include "AvrChipDesc.h"

using namespace tmb;

class IAvrProgDevice {
	public:
		enum EraseMode {
			EraseMode_CHIP = 0,
			EraseMode_APP_SECTION = 1,
			EraseMode_BOOT_SECTION = 2,
			EraseMode_EEPROM = 3,
			EraseMode_FLASH = 4,
		};
		
		class Progress {
			public: Progress() {}
			public: virtual ~Progress() {}
			public: virtual void next(uint addr, uint val) = 0;
		};
		
	protected:
		String				_name;
		const AvrChipDesc*	_chipdesc;
		
	private:
		IAvrProgDevice(const IAvrProgDevice&);
		void operator=(const IAvrProgDevice&);
		
	public:
		IAvrProgDevice(const String& a_name, const AvrChipDesc* a_cd);
		virtual ~IAvrProgDevice() {}
		
		virtual const String& name() const;
		virtual const AvrChipDesc* getChipDesc() const;

		virtual void open() = 0;
		virtual void close() = 0;
		virtual void setProgMode(bool a_on) = 0;

		
		virtual void readProgMem(uint a_addr, uint a_size, Array<uint16>& a_mem, Progress* a_pg = 0);
		virtual void writeProgMemPage(uint a_addr, const Array<uint16>& a_mem, uint a_offs = 0, Progress* a_pg = 0);
		virtual void readEeprom(uint a_addr, uint a_size, Array<byte>& a_mem, Progress* a_pg = 0);
		virtual void writeEepromPage(uint a_addr, const Array<byte>& a_mem, uint a_size, uint a_offs = 0, Progress* a_pg = 0);
		virtual void writeEepromByte(uint a_addr, byte a_byte, Progress* a_pg = 0);
		virtual byte readLockBits();
		virtual void writeLockBits(byte a_bits);
		virtual uint32 readSignature();
		virtual byte readFuseBits(uint a_fusenr);
		virtual void writeFuseBits(uint a_fusenr, byte a_fuses);
		virtual byte readCalibrationByte(uint a_addr);
		virtual void chipErase(EraseMode mode);
		virtual bool programEnable();

	protected:
		virtual void cmdReadProgMem(uint a_addr, uint a_size, Array<uint16>& a_mem, Progress* a_pg) = 0;
		virtual void cmdProgramProgMemPage(uint a_addr, const Array<uint16>& a_mem, uint a_offs, Progress* a_pg) = 0;
		virtual void cmdReadEeprom(uint a_addr, uint a_size, Array<byte>& a_mem, Progress* a_pg) = 0;
		virtual void cmdProgramEepromPage(uint a_addr, const Array<byte>& a_mem, uint a_size, uint a_offs, Progress* a_pg) = 0;
		
		virtual uint16 cmdReadProgMem(uint a_addr) = 0;
		virtual void cmdLoadProgMemPage(uint a_addr, uint16 a_data) = 0;
		virtual void cmdWriteProgMemPage(uint a_addr) = 0;
		virtual byte cmdReadEeprom(uint a_addr) = 0;
		virtual void cmdWriteEepromByte(uint a_addr, byte a_data) = 0;
		virtual void cmdLoadEepromPage(uint a_addr, byte a_data) = 0;
		virtual void cmdWriteEepromPage(uint a_addr) = 0;
		virtual byte cmdReadLockBits() = 0;
		virtual void cmdWriteLockBits(byte a_bits) = 0;
		virtual uint32 cmdReadSignature() = 0;
		virtual byte cmdReadFuseBits(uint a_fusenr) = 0;
		virtual void cmdWriteFuseBits(uint a_fusenr, byte a_bits) = 0;
		virtual byte cmdReadCalibrationByte(uint a_addr) = 0;
		virtual bool cmdPollBusy() = 0;
		virtual void cmdChipErase(EraseMode mode) = 0;
		virtual bool cmdProgEnable() = 0;
		virtual void cmdLoadExtAddrByte(byte a_extaddr) = 0;
		
		virtual void progWait(uint a_ms);
	
};

#endif
