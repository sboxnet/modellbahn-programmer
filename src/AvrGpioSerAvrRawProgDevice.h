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


#ifndef _AVRGPIOSERAVRRAWPROGDEVICE_H_
#define _AVRGPIOSERAVRRAWPROGDEVICE_H_

#include "IAvrSerialProgDevice.h"
#include <tmb/lang/String.h>
#include <tmb/io/GpioSer.h>

using namespace tmb;

class AvrGpioSerAvrRawProgDevice : public IAvrSerialProgDevice {
	public:
		enum {
			FLG_NOPOLL		= 0x01,
		};

	protected:
		String		_devfile;
		GpioSer		_gpioser;
		uint		_sclkfreq;
		uint		_flags;
		
	private: // disable copy operator
		AvrGpioSerAvrRawProgDevice(const AvrGpioSerAvrRawProgDevice&);
		void operator= (const AvrGpioSerAvrRawProgDevice&);
	
	public:
		AvrGpioSerAvrRawProgDevice(const String& a_devfile, const AvrChipDesc* a_cd, uint a_sclkfreq = 0, uint a_flags = 0);
		virtual ~AvrGpioSerAvrRawProgDevice();
		
		virtual uint getSclkFreq();
				
		virtual void open();
		virtual void close();
		virtual void setProgMode(bool a_on);
		
	protected:
		virtual uint32 sendCmd(uint32 a_cmd, bool a_checkcomm = true);

	private:
		virtual void sipCommit();
		virtual void sipRead();

};

#endif // _AVRGPIOSERAVRRAWPROGDEVICE_H_

