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

#ifndef _AVRERROR_H_
#define _AVRERROR_H_

#include <tmb/lang/Exception.h>

using namespace tmb;

class AvrException : public Exception {
	public: AvrException(const String& a_msg) : Exception(a_msg) {}
};

class AvrError : public AvrException {
	public: AvrError(const String& a_msg) : AvrException(a_msg) {}
};

class AvrOutOfSyncError  : public AvrError {
	public: AvrOutOfSyncError(const String& a_msg) : AvrError(a_msg) {}
};

class AvrInvalidParameterError : public AvrError {
	public: AvrInvalidParameterError(const String& a_msg) : AvrError(a_msg) {}
};

class AvrNotReadyError : public AvrError {
	public: AvrNotReadyError(const String& a_msg) : AvrError(a_msg) {}
};

class AvrAddressError : public AvrError {
	public: AvrAddressError(const String& a_msg) : AvrError(a_msg) {}
};


#endif // _AVRERROR_H_
