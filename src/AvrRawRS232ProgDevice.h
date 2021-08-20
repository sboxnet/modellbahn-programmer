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

#ifndef _AVRRAWRS232PROGDEVICE_H_
#define _AVRRAWRS232PROGDEVICE_H_

#include "IAvrSerialProgDevice.h"
#include <tmb/lang/String.h>
#include <tmb/io/RS232Device.h>

using namespace tmb;

class AvrRawRS232ProgDevice : public IAvrSerialProgDevice {
	protected:
		String		_devfile;
		RS232Device	_rs232;
		bool		_inverted_signals;
		
	private: // disable copy operator
		AvrRawRS232ProgDevice(const AvrRawRS232ProgDevice&);
		void operator= (const AvrRawRS232ProgDevice&);
	
	public:
		AvrRawRS232ProgDevice(const String& a_devfile, bool a_inverted_signals, const AvrChipDesc* a_cd);
		virtual ~AvrRawRS232ProgDevice();
				
		virtual void open();
		virtual void close();
		virtual void setProgMode(bool a_on);
		
	protected:
		virtual void sipCommit();
		virtual void sipRead();
};

#endif // _AVRRAWRS232PROGDEVICE_H_

