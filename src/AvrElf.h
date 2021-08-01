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

#ifndef _AVRELF_H_
#define _AVRELF_H_

#include <tmb/lang/Exception.h>
#include <tmb/lang/String.h>
#include <tmb/lang/Array.h>

using namespace tmb;

class AvrElf {

	protected:
		Array<byte>	_elf;
		const void*	_shtext;
		const void* _shdata;
	
	public:
		AvrElf(const String& a_filename);
		virtual ~AvrElf();
		
		// get code of .text segment
		void getCode(Array<uint16>& a_code)  const;
		// get size of code in .text segment, in 16bit words
		int getCodeSize() const;
		
		int getTextSegSize() const; // in 16bit words
		int getDataSegSize() const; // in 16bit words
};

#endif // _AVRELF_H_
