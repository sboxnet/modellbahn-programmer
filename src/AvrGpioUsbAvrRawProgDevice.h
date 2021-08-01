/***************************************************************************
 *   Copyright (C) 2013
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


#ifndef _AVRGPIOUSBAVRRAWPROGDEVICE_H_
#define _AVRGPIOUSBAVRRAWPROGDEVICE_H_

#include "IAvrSerialProgDevice.h"
#include <tmb/lang/String.h>
#include <tmb/io/UsbDevice.h>

using namespace tmb;

class AvrGpioUsbAvrRawProgDevice : public IAvrSerialProgDevice {
	public:
		enum {
			FLG_NOPOLL		= 0x01
		};
		
	protected:
		enum {
			AVRWAIT_POLL	= 0x00,
			AVRWAIT_8MS     = 0x04,
			AVRWAIT_16MS    = 0x08,
			AVRWAIT_64MS    = 0x0c,

			AVR_ERRF_NOSYNC        = 0x01,
			AVR_ERRF_POLL_TIMEOUT  = 0x02,
		};
		enum CMDS {
			CMD_CLR_ERROR_FLAGS = 0x00,
			CMD_GET_ERROR_FLAGS = 0x01,
			CMD_GET_VERSION     = 0x02,
			CMD_GET_VERSION_STR = 0x03,
			CMD_GET_FLAGS       = 0x04,
			CMD_GET_RECVBUFSIZE = 0x05,
			CMD_ENTER_AVRMODE   = 0x08,
			CMD_LEAVE_AVRMODE   = 0x09,
			CMD_READ_IO_REG     = 0x0a,
			CMD_PUT_IO_REG      = 0x0b,
			CMD_SOFT_RESET      = 0x0e,
			
			CMD_AVR_PROGMODE    = 0x10,
			CMD_AVR_RAWCMD      = 0x11,
			CMD_AVR_READ_PRGMEM = 0x12,
			CMD_AVR_READ_EEPROM = 0x13,
			CMD_AVR_WRITE_PRGMEM_PAGE   = 0x14,
			CMD_AVR_WRITE_EEPROM_PAGE   = 0x15,
			CMD_AVR_WRITE_EEPROM_BYTE   = 0x16,
			CMD_AVR_LOAD_EXT_ADDR   = 0x17,
			CMD_AVR_CHIP_ERASE      = 0x18,
			CMD_AVR_PROGRAMENABLE   = 0x19,
			CMD_AVR_READ_SIGNATURE  = 0x1a,
			CMD_AVR_READ_LOCKBITS   = 0x1b,
			CMD_AVR_WRITE_LOCKBITS  = 0x1c,
			CMD_AVR_READ_FUSEBITS   = 0x1d,
			CMD_AVR_WRITE_FUSEBITS  = 0x1e,
			CMD_AVR_READ_CALIBR     = 0x1f,
			CMD_AVR_GET_STATUS      = 0x20,
		};

	protected:
		uint		_sclkfreq;
		UsbDevice	_usbdev;
		uint		_flags;
		
	private: // disable copy operator
		AvrGpioUsbAvrRawProgDevice(const AvrGpioUsbAvrRawProgDevice&);
		void operator= (const AvrGpioUsbAvrRawProgDevice&);
	
	public:
		AvrGpioUsbAvrRawProgDevice(const AvrChipDesc* a_cd, uint a_sclkfreq = 0, uint a_flags = 0);
		virtual ~AvrGpioUsbAvrRawProgDevice();
		
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

