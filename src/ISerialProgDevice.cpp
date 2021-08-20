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


#include "ISerialProgDevice.h"

ISerialProgDevice::ISerialProgDevice()
		: _sip(0), _mode(0) {
}

ISerialProgDevice::~ISerialProgDevice() {
}

bool ISerialProgDevice::sipclk() const {
	return _sip.has(SIP_CLK);
}
void ISerialProgDevice::sipclk(bool c) {
	_sip.set(SIP_CLK, c);
}

bool ISerialProgDevice::sipwd() const {
	return _sip.has(SIP_WD);
}
void ISerialProgDevice::sipwd(bool d) {
	_sip.set(SIP_WD, d);
}

bool ISerialProgDevice::siprd() const {
	return _sip.has(SIP_RD);
}

const Flags& ISerialProgDevice::mode() const {
	return _mode;
}

void ISerialProgDevice::sipCommitClkWd(bool clk, bool data) {
	sipclk(clk);
	sipwd(data);
	sipCommit();
}
void ISerialProgDevice::sipCommitClk(bool clk) {
	sipclk(clk);
	sipCommit();
}
void ISerialProgDevice::sipCommitWd(bool data) {
	sipwd(data);
	sipCommit();
}
bool ISerialProgDevice::sipReadRd() {
	sipRead();
	return siprd();
}

