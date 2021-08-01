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


#ifndef _AVRGPIOUSBAVRPROGDEVICE_H_
#define _AVRGPIOUSBAVRPROGDEVICE_H_

#include "AvrGpioUsbAvrRawProgDevice.h"
#include "AvrError.h"

using namespace tmb;

class AvrGpioUsbAvrProgDevice : public AvrGpioUsbAvrRawProgDevice {
		
	private: // disable copy operator
		AvrGpioUsbAvrProgDevice(const AvrGpioUsbAvrProgDevice&);
		void operator= (const AvrGpioUsbAvrProgDevice&);
	
	public:
		AvrGpioUsbAvrProgDevice(const AvrChipDesc* a_cd, uint a_sclkfreq = 0, uint a_flags = 0);
		virtual ~AvrGpioUsbAvrProgDevice();
				
	protected:
		void checkStatus();
		
		virtual void cmdReadProgMem(uint a_addr, uint a_size, Array<uint16>& a_mem, Progress* a_pg);
		virtual void cmdProgramProgMemPage(uint a_addr, const Array<uint16>& a_mem, uint a_offs, Progress* a_pg);
		virtual void cmdReadEeprom(uint a_addr, uint a_size, Array<byte>& a_mem, Progress* a_pg);
		virtual void cmdProgramEepromPage(uint a_addr, const Array<byte>& a_mem, uint a_size, uint a_offs, Progress* a_pg);
};

#endif // _AVRGPIOSERAVRPROGDEVICE_H_

