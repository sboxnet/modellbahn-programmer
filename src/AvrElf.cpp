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


#include "AvrElf.h"
#include <tmb/io/File.h>

#include <elf.h>
#include <string.h>

AvrElf::AvrElf(const String& a_filename) {
	
	File(a_filename).load(_elf);
	
	if (_elf.size() < sizeof(Elf32_Ehdr))
		throw Exception("not an ELF file");
	const Elf32_Ehdr* elf = (const Elf32_Ehdr*)_elf.data();
	if (memcmp(elf, ELFMAG, SELFMAG) != 0 || elf->e_ehsize != sizeof(*elf))
		throw Exception("not an ELF file");
	
	if (elf->e_ident[EI_CLASS] != ELFCLASS32)
		throw Exception("not an ELF32 file");
	if (elf->e_ident[EI_DATA] != ELFDATA2LSB)
		throw Exception("data format is not little endian");
	if (elf->e_type != ET_EXEC)
		throw Exception("not an executable ELF");
	if (elf->e_machine != EM_AVR)
		throw Exception("ELF is not for Atmel AVR");
	
	if (elf->e_shoff >= _elf.size() || elf->e_shoff < elf->e_ehsize)
		throw Exception("damaged ELF header: e_shoff");
	
	if (elf->e_shnum == 0)
		throw Exception("no sections in ELF file");
	if (elf->e_shentsize != sizeof(Elf32_Shdr))
		throw Exception("damaged ELF header: e_shentsize");
	const Elf32_Shdr* sha = (const Elf32_Shdr*)((byte*)elf + elf->e_shoff);
	
	const char* shstrs = (const char*)((byte*)elf + sha[elf->e_shstrndx].sh_offset);
	
	// find .text segment
	_shtext = NULL;
	_shdata = NULL;
	for (int i = 0; i < elf->e_shnum; i++) {
		const Elf32_Shdr& sh = sha[i];
		if (sh.sh_name == SHN_UNDEF || sh.sh_name >= SHN_LORESERVE)
			continue;
		if (sh.sh_type != SHT_PROGBITS)
			continue;
		if (strncmp(".text", shstrs + sh.sh_name, 6) == 0) {
			if (_shtext)
				throw Exception("more than one .text segment found");
			_shtext = &sh;
		}
		else if (strncmp(".data", shstrs + sh.sh_name, 6) == 0) {
			if (_shdata)
				throw Exception("more than one .data segment found");
			_shdata = &sh;
		}		
	}
	if (!_shtext)
		throw Exception("no .text segment found");
	
/*	cout << "name: " << shtext->sh_name << endl
			 << "type: " << shtext->sh_type << endl
			 << "flags: " << shtext->sh_flags << endl
			 << "addr: " << shtext->sh_addr << endl
			 << "offs: " << shtext->sh_offset << endl
			 << "size: " << shtext->sh_size << endl
			 << "link: " << shtext->sh_link << endl
			 << "info: " << shtext->sh_info << endl
			 << "align: " << shtext->sh_addralign << endl
			 << "es: " << shtext->sh_entsize << endl;
*/

}

AvrElf::~AvrElf() {
}

void AvrElf::getCode(Array<uint16>& a_code) const {
	int textsize = getTextSegSize();
	int datasize = getDataSegSize();
	a_code.clear();
	a_code.setSize(textsize + datasize);

	int codeidx = 0;
	uint16* p = (uint16*)(((byte*)_elf.data()) + ((const Elf32_Shdr*)_shtext)->sh_offset);
	for (int i = 0; i < textsize; i++) {
		a_code[codeidx++] = p[i];
	}
	if (_shdata) {
		p = (uint16*)(((byte*)_elf.data()) + ((const Elf32_Shdr*)_shdata)->sh_offset);
		for (int i = 0; i < datasize; i++) {
			a_code[codeidx++] = p[i];
		}
	}
}

int AvrElf::getCodeSize() const { // in 16bit words
	return getTextSegSize() + getDataSegSize();
}

int AvrElf::getTextSegSize() const {
	if (_shtext) {
		return (((const Elf32_Shdr*)_shtext)->sh_size+1) / 2;
	}
	return 0;
}

int AvrElf::getDataSegSize() const {
	if (_shdata) {
		return (((const Elf32_Shdr*)_shdata)->sh_size+1) / 2;
	}
	return 0;
}


