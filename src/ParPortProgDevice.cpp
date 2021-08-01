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

/*
#include "ParPortProgDevice.h"
#include "util.h"

#include <tmb/io/IOException.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>



#define BIT_CLK		0x01
#define BIT_WDATA	0x02
#define BIT_PROGEN	0x04

#define BIT_RDATA	0x80

ParPortProgDevice::ParPortProgDevice() {
	_fd = -1;
}

ParPortProgDevice::~ParPortProgDevice() {
	close();
}

void ParPortProgDevice::open() throw(Exception) {
	close();
	
	errno = 0;
	_fd = ::open("/proc/driver/picprogpp", O_RDWR);
	if (_fd < 0) {
		_fd = -1;
		throw IOException("unable to open picprogpp device", errno);
	}
	
	progEnable(false);
}

void ParPortProgDevice::close() {
	if (_fd >= 0) {
		try { progEnable(false); } catch(...) {}
		::close(_fd);
		_fd = -1;
	}
}

void ParPortProgDevice::writeport(byte b) throw(Exception) {
	errno = 0;
	if (::write(_fd, &b, 1) != 1) {
		throw IOException("write error on picprogpp device", errno);
	}
}

void ParPortProgDevice::sipCommit() throw(Exception) {
	// Note: the programmer hardware inverts CLK and WDATA !!
	byte port = 0;
	if (_mode.has(MODE_PROGEN)) {
		if (!sipclk())
			port |= BIT_CLK;
		if (!sipwd() && !(_mode.has(MODE_READ)))
			port |= BIT_WDATA;
		
		port |= BIT_PROGEN;
	}
	writeport(port);
}

void ParPortProgDevice::sipRead() throw(Exception) {
	// Note: bit 7 is /busy signal from parport and the programmer hardware inverts RDATA !!
	//       So, no inversion here.
	byte port = 0;
	errno = 0;
	if (::read(_fd, &port, 1) != 1) {
		throw IOException("read error on picprogpp device", errno);
	}
	
	_sip.set(SIP_RD, (port & BIT_RDATA) != 0);
}

void ParPortProgDevice::progEnable(bool a_on) throw(Exception) {
	if (a_on) {
		_sip = 0;
		_mode.clear(MODE_READ);
		_mode.set(MODE_PROGEN);
		
		writeport(0);
		wait_ms(1);
		writeport(BIT_CLK|BIT_WDATA); // set RB6 and RB7 to low
		wait_ms(1);
		writeport(BIT_CLK|BIT_WDATA|BIT_PROGEN); // enable programming voltage on _MCLR
		wait_ms(1);
		
	} else {
		_sip = 0;
		_mode.clear(MODE_READ|MODE_PROGEN);
		writeport(0);
	}
}

void ParPortProgDevice::setReadMode(bool a_on) throw(Exception) {
	_mode.set(MODE_READ, a_on);
	sipCommit();
}

*/
