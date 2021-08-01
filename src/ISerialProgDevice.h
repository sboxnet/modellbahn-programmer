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

#ifndef _ISERIALPROGDEVICE_H_
#define _ISERIALPROGDEVICE_H_

#include <tmb/lang/Exception.h>
#include <tmb/lang/String.h>

using namespace tmb;

class ISerialProgDevice {
	public:
 		// serial interface pins
		enum { SIP_CLK = 0x01,	// serial clock
			   SIP_WD  = 0x02,	// write data to chip
	  		   SIP_RD  = 0x04,	// read data from chip
		};
		enum { MODE_READ = 0x01, MODE_PROG = 0x02, };
		
	protected:
		Flags	_sip;
		Flags	_mode;
	
	private: // disable copy operator
		ISerialProgDevice(const ISerialProgDevice&);
		void operator= (const ISerialProgDevice&);
		
	public:
		ISerialProgDevice();
		virtual ~ISerialProgDevice();
				
		virtual void open() = 0;
		virtual void close() = 0;
		virtual void setProgMode(bool a_on) = 0;
		
	protected:
		virtual void sipCommit() = 0;
		virtual void sipRead() = 0;
		virtual void setReadMode(bool a_on) = 0;
		
		bool sipclk() const;
		void sipclk(bool c);
		
		bool sipwd() const;
		void sipwd(bool d);
		
		bool siprd() const;
		
		const Flags& mode() const;
		
		void sipCommitClkWd(bool clk, bool data);
		void sipCommitClk(bool clk);
		void sipCommitWd(bool data);
		bool sipReadRd();

};

#endif // _ISERIALPROGDEVICE_H_
