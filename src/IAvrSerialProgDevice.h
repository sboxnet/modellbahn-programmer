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

#ifndef _IAVRSERIALPROGDEVICE_H_
#define _IAVRSERIALPROGDEVICE_H_

#include "ISerialProgDevice.h"
#include "IAvrProgDevice.h"
#include "AvrChipDesc.h"

class IAvrSerialProgDevice : protected ISerialProgDevice, public IAvrProgDevice {
	
	private: // disable copy operator
		IAvrSerialProgDevice(const IAvrSerialProgDevice&);
		void operator= (const IAvrSerialProgDevice&);
	
	public:
		IAvrSerialProgDevice(const String& a_name, const AvrChipDesc* a_cd);
		virtual ~IAvrSerialProgDevice();
		
		virtual void open() = 0;
		virtual void close() = 0;
		virtual void setProgMode(bool a_on) = 0;
		
	protected:
		virtual uint32 sendCmd(uint32 a_cmd, bool a_checkcomm = true);

		virtual void cmdReadProgMem(uint a_addr, uint a_size, Array<uint16>& a_mem, Progress* a_pg);
		virtual void cmdProgramProgMemPage(uint a_addr, const Array<uint16>& a_mem, uint a_offs, Progress* a_pg);
		virtual void cmdReadEeprom(uint a_addr, uint a_size, Array<byte>& a_mem, Progress* a_pg);
		virtual void cmdProgramEepromPage(uint a_addr, const Array<byte>& a_mem, uint a_size, uint a_offs, Progress* a_pg);
		
		virtual uint16 cmdReadProgMem(uint a_addr);
		virtual void cmdLoadProgMemPage(uint a_addr, uint16 a_data);
		virtual void cmdWriteProgMemPage(uint a_addr);
		virtual byte cmdReadEeprom(uint a_addr);
		virtual void cmdWriteEepromByte(uint a_addr, byte a_data);
		virtual void cmdLoadEepromPage(uint a_addr, byte a_data);
		virtual void cmdWriteEepromPage(uint a_addr);
		virtual byte cmdReadLockBits();
		virtual void cmdWriteLockBits(byte a_bits);
		virtual uint32 cmdReadSignature();
		virtual byte cmdReadFuseBits(uint a_fusenr);
		virtual void cmdWriteFuseBits(uint a_fusenr, byte a_bits);
		virtual byte cmdReadCalibrationByte(uint a_addr);
		virtual bool cmdPollBusy();
		virtual void cmdChipErase(EraseMode mode);
		virtual bool cmdProgEnable();
		virtual void cmdLoadExtAddrByte(byte a_extaddr);

	private:
		virtual void setReadMode(bool a_on);
};

#endif
