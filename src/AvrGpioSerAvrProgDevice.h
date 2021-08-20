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


#ifndef _AVRGPIOSERAVRPROGDEVICE_H_
#define _AVRGPIOSERAVRPROGDEVICE_H_

#include "AvrGpioSerAvrRawProgDevice.h"

using namespace tmb;

class AvrGpioSerAvrProgDevice : public AvrGpioSerAvrRawProgDevice {
		
	private: // disable copy operator
		AvrGpioSerAvrProgDevice(const AvrGpioSerAvrProgDevice&);
		void operator= (const AvrGpioSerAvrProgDevice&);
	
	public:
		AvrGpioSerAvrProgDevice(const String& a_devfile, const AvrChipDesc* a_cd, uint a_sclkfreq = 0, uint a_flags = 0);
		virtual ~AvrGpioSerAvrProgDevice();
				
	protected:
		virtual void cmdReadProgMem(uint a_addr, uint a_size, Array<uint16>& a_mem, Progress* a_pg);
		virtual void cmdProgramProgMemPage(uint a_addr, const Array<uint16>& a_mem, uint a_offs, Progress* a_pg);
		virtual void cmdReadEeprom(uint a_addr, uint a_size, Array<byte>& a_mem, Progress* a_pg);
		virtual void cmdProgramEepromPage(uint a_addr, const Array<byte>& a_mem, uint a_size, uint a_offs, Progress* a_pg);

		virtual void cmdWriteEepromByte(uint a_addr, byte a_data);

        virtual byte cmdReadLockBits();
		virtual void cmdWriteLockBits(byte a_bits);
		virtual uint32 cmdReadSignature();
		virtual byte cmdReadFuseBits(uint a_fusenr);
		virtual void cmdWriteFuseBits(uint a_fusenr, byte a_bits);
		virtual byte cmdReadCalibrationByte(uint a_addr);

        virtual void cmdChipErase(EraseMode mode);
		virtual bool cmdProgEnable();
		virtual void cmdLoadExtAddrByte(byte a_extaddr);

		virtual void progWait(uint a_ms);
};

#endif // _AVRGPIOSERAVRPROGDEVICE_H_

