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


#ifndef _AVRGPIOSERPROGDEVICE_H_
#define _AVRGPIOSERPROGDEVICE_H_

#include "IAvrSerialProgDevice.h"
#include <tmb/lang/String.h>
#include <tmb/io/GpioSer.h>

using namespace tmb;

class AvrGpioSerProgDevice : public IAvrSerialProgDevice {
	protected:
		String		_devfile;
		GpioSer		_gpioser;
		
	private: // disable copy operator
		AvrGpioSerProgDevice(const AvrGpioSerProgDevice&);
		void operator= (const AvrGpioSerProgDevice&);
	
	public:
		AvrGpioSerProgDevice(const String& a_devfile, const AvrChipDesc* a_cd);
		virtual ~AvrGpioSerProgDevice();
				
		virtual void open();
		virtual void close();
		virtual void setProgMode(bool a_on);
		
	protected:
		virtual void sipCommit();
		virtual void sipRead();

};

#endif // _AVRGPIOSERPROGDEVICE_H_

