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

#include "AvrChipDesc.h"

#include <tmb/util/Properties.h>
#include <tmb/lang/StringTokenizer.h>

#include <stdio.h>


AvrChipDesc::AvrChipDesc() 
	: fuse_desc(AutoDelete_ALL) {
}

AvrChipDesc::~AvrChipDesc() {
}


static void printFuses(const ArrayP<Array<String> >&fuse_desc) {
	for (int k = 0; k < fuse_desc.size(); k++) {
		printf("fuse %d:          ", k);
		Array<String>* fd = fuse_desc[k];
		if (fd->size() > 0) {
			bool first = true;
			for (int i = fd->size() - 1; i >= 0; i--) {
				if (!first)
					printf(":");
				else
					first = false;
				printf("%s", (*fd)[i].c_str());
			}
			printf("\n");
		} else {
			printf("none\n");
		}
	}
}

void AvrChipDesc::print() const {
	printf("Name:              %s\n", name.c_str());
	if (arch == Arch_XMEGA) {
		printf("Architecture:      xmega\n");
	} else {
		printf("Architecture:      avr\n");
	}
	printf("Signature:         0x%x,0x%x,0x%x\n", (int)sigByte(0), (int)sigByte(1), (int)sigByte(2));
	printf("Flash size:        %u 16bit words\n", flash_size);
	printf("EEPROM size:       %u bytes\n", eeprom_size);
	printf("RAM size:          %u bytes\n", ram_size);
	printf("RAM start addr:    0x%x\n",   ram_beg_addr);
	printf("RAM end addr:      0x%x\n",   ram_end_addr);
	printf("IO space size:     %u bytes\n", io_size);
	printf("IO start addr:     0x%x\n",   io_beg_addr);
	printf("IO end addr:       0x%x\n",   io_end_addr);
	printf("Flash addr mask:          0x%x\n", flash_mask);
	printf("Flash page size:          %u\n",   flash_page_size);
	printf("Flash page exponent:      %u\n",   flash_page_exp);
	printf("Flash page offset mask:   0x%x\n", flash_poff_mask);
	printf("Flash num pages:          %u\n",   flash_num_pages);
	printf("Flash page mask:          0x%x\n", flash_page_mask);
	printf("EEPROM addr mask:         0x%x\n", eeprom_mask);
	printf("EEPROM page size:         %u\n",   eeprom_page_size);
	printf("EEPROM page exponent:     %u\n",   eeprom_page_exp);
	printf("EEPROM page offset mask:  0x%x\n", eeprom_poff_mask);
	printf("EEPROM num pages:         %u\n",   eeprom_num_pages);
	printf("EEPROM page mask:         0x%x\n", eeprom_page_mask);
	printf("calibration bytes:        %d\n", calbytes);
	printFuses(fuse_desc);
	if (eesave_mask) {
		printf("eesave fuse:              fuse %d, bit mask 0x%x\n", eesave_fusenr, eesave_mask);
	} else {
		printf("eesave fuse:              none\n");
	}
	if (prginterface == PrgInterface_SPI) {
		printf("program interface:        SPI\n");
		printf("twd flash:                %u\n", spi_twd_flash);
		printf("twd eeprom:               %u\n", spi_twd_eeprom);
		printf("twd erase:                %u\n", spi_twd_erase);
		printf("twd fuse:                 %u\n", spi_twd_fuse);
		printf("has poll command:         %s\n", spi_has_cmdpoll ? "yes":"no");
		printf("has eeprom page write:    %s\n", spi_has_eeprom_page_write ? "yes":"no");
	}
	if (prginterface == PrgInterface_PDI) {
		printf("program interface:        PDI\n");
	}
}

byte AvrChipDesc::sigByte(uint n) const{
	switch(n) {
		case 0:	return (signature >> 16);
		case 1: return (signature >> 8);
		case 2: return signature;
		default: return 0;
	}
}




AvrChipDescriptions::AvrChipDescriptions()
		: _chips(AutoDelete_ALL) {
}

AvrChipDescriptions::~AvrChipDescriptions() {
}


AvrChipDesc* AvrChipDescriptions::find(uint32 a_sign) const {
	for (int i = 0; i < _chips.size(); i++) {
		const AvrChipDesc* p = _chips[i];
		if (p->signature == a_sign) {
			return _chips[i];
		}
	}
	return NULL;
}

AvrChipDesc* AvrChipDescriptions::find(const String& a_name) const {
	for (int i = 0; i < _chips.size(); i++) {
		if (a_name.equalsIgnoreCase(_chips[i]->name)) {
			return _chips[i];
		}
	}
	return NULL;
}

void AvrChipDescriptions::printKnownChips() const {
	for (int i = 0; i < _chips.size(); i++) {
		const AvrChipDesc* p = _chips[i];
		printf("  %s  flash:%u bytes  signature:0x%x\n", p->name.c_str(), p->flash_size*2, p->signature);
	}
}





static uint32 get_mask(uint32 v) {
	uint mask = 0;
	while ((mask + 1) < v) {
		mask = (mask << 1) | 1;
	}
	return mask;
}

static uint get_exp(uint32 v) {
	uint exp = 0;
	while ( (1ul << exp) < v && exp < 32 ) {
		exp++;
	}
	return exp;
}

static bool is_power_of_2(uint32 v) {
	uint n = 0; // count number of 1's
	for (uint i = 0; i < 32; i++) {
		if ((1ul << i) & v)
			n++;
	}
	return n <= 1;
}

bool AvrChipDescriptions::parseFuseDesc(int fuseidx, const String& chipkey, AvrChipDesc* cd, const HashMap<String>& parmap) {
	String desc;
	String key;
	if (fuseidx == 0 && parmap.containsKey(chipkey + ".lfuse")) {
		key = chipkey + ".lfuse";
	} else if (fuseidx == 1 && parmap.containsKey(chipkey + ".hfuse")) {
		key = chipkey + ".hfuse";
	} else if (fuseidx == 2 && parmap.containsKey(chipkey + ".efuse")) {
		key = chipkey + ".efuse";
	} else if (parmap.containsKey(chipkey + ".fuse." + fuseidx)) {
		key = chipkey + ".fuse." + fuseidx;
	} else {
		return false;
	}
	desc = parmap.get(key);
	desc.stripWhiteSpace();

	ObjPtr<Array<String> > arr(new Array<String>);
	
	if (desc != "none") {
		arr->setSize(8);

		StringTokenizer tz(desc);
		tz.setSeparatorChars(":");
		for (int i = 7; i >= 0; i--) {
			try {
				String v = tz.next();
				(*arr)[i] = v;
				if (v.equalsIgnoreCase("EESAVE")) {
					if (cd->eesave_mask) {
						throw ParseException("more then one EESAVE fuse in fuse description: "+key);
					}
					cd->eesave_fusenr = fuseidx;
					cd->eesave_mask = (1 << i);
				}
			} catch(const NotFoundException& ex) {
				throw ParseException("less then 8 bits in fuse description: "+key);
			}
		}
		if (tz.hasNext()) {
			throw ParseException("more then 8 bits in fuse description: "+key);
		}
	}
	cd->fuse_desc.add(arr.detach());
	return true;
}

void AvrChipDescriptions::load(const String& a_filename) {
	Properties props;
	props.load(a_filename);
	
	const HashMap<String>& m = props.map();
	HashMap<String>::Key mkey = 0;
	String* val;
	const String* key;
	while ((mkey = m.nextKey(mkey, &val, &key)) != 0) {	
		if (key->startsWith("chip.")) {
			String chipkey = key->substring(5);
			if (chipkey.contains('.'))
				throw ParseException("invalid chip.* property, contains more than one period: " + *key);
			ObjPtr<AvrChipDesc> cd(new AvrChipDesc());
			cd->name = *val;
			cd->name.stripWhiteSpace();
			if (cd->name.length() == 0)
				throw ParseException("no chip name for property " + *key);

			String arch;
			try {
				arch = props.get(chipkey + ".arch");
			} catch(const NotFoundException& ex) {
				throw ParseException("mandatory property not found: " + ex.getMessage());
			} 
			if (arch == "avr") {
				cd->arch = AvrChipDesc::Arch_AVR;
			} else if (arch == "xmega"){
				cd->arch = AvrChipDesc::Arch_XMEGA;
			} else {
				throw ParseException("unknown architecture specified for "+chipkey);
			}
			
			try {
				cd->signature = props.getUInt32(chipkey + ".signature") & 0x00ffffff;
				
				cd->flash_size = props.getUInt32(chipkey + ".flash.size");
				if (cd->arch == AvrChipDesc::Arch_XMEGA) {
					cd->flash_app_size = props.getUInt32(chipkey + ".flash.app.size");
					cd->flash_boot_size = props.getUInt32(chipkey + ".flash.boot.size");
				} else {
					cd->flash_app_size = cd->flash_size;
					cd->flash_boot_size = 0;
				}
				cd->eeprom_size = props.getUInt32(chipkey + ".eeprom.size");
				cd->ram_beg_addr = props.getUInt32(chipkey + ".ram.addr");
				cd->ram_size = props.getUInt32(chipkey + ".ram.size");
				cd->ram_end_addr = cd->ram_beg_addr + cd->ram_size - 1;
				cd->io_beg_addr  = props.getUInt32(chipkey + ".iospace.addr");
				cd->io_size  = props.getUInt32(chipkey + ".iospace.size");
				cd->io_end_addr = cd->io_beg_addr + cd->io_size - 1;
				cd->flash_page_size = props.getUInt32(chipkey + ".flash.page.size");
				cd->eeprom_page_size = props.getUInt32(chipkey + ".eeprom.page.size");
				cd->calbytes = props.getUInt32(chipkey + ".calbytes");
				
				String pi = props.get(chipkey + ".pi");
				
				cd->spi_twd_flash = 0;
				cd->spi_twd_eeprom = 0;
				cd->spi_twd_erase = 0;
				cd->spi_twd_fuse = 0;
				cd->spi_has_cmdpoll = 0;
				cd->spi_has_eeprom_page_write = 0;
				if (pi == "pdi") {
					cd->prginterface = AvrChipDesc::PrgInterface_PDI;
				} else if (pi == "spi") {
					cd->prginterface = AvrChipDesc::PrgInterface_SPI;
					cd->spi_twd_flash = props.getUInt32(chipkey + ".spi.twd.flash");
					cd->spi_twd_eeprom = props.getUInt32(chipkey + ".spi.twd.eeprom");
					cd->spi_twd_erase = props.getUInt32(chipkey + ".spi.twd.erase");
					cd->spi_twd_fuse = props.getUInt32(chipkey + ".spi.twd.fuse");
					cd->spi_has_cmdpoll = props.getUInt32(chipkey + ".spi.cmdpoll", AvrChipDesc::DEFAULT_HAS_CMDPOLL);
					cd->spi_has_eeprom_page_write = props.getUInt32(chipkey + ".spi.cmdeeprompgwr", AvrChipDesc::DEFAULT_HAS_CMDEEPROMPAGEWRITE);
				}
				cd->eesave_mask = 0;
				cd->eesave_fusenr = 0;

				int fuseidx = 0;
				while (true) {
					if (!parseFuseDesc(fuseidx, chipkey, cd, m))
						break;
					fuseidx++;
				}
			} catch(const NotFoundException& ex) {
				throw ParseException("mandatory property not found: " + ex.getMessage());
			} catch(const NumberFormatException& ex) {
				throw ParseException(ex.getMessage());
			}
			
			if (find(cd->name) != 0) {
				throw ParseException("duplicate chip name: " + cd->name);
			}
			
			// verify values
			if (cd->flash_size < 64 || cd->flash_size > AvrChipDesc::MAX_FLASH_SIZE)
				throw ParseException(chipkey + ".flash.size with unsupported value");
			if (cd->eeprom_size > AvrChipDesc::MAX_EEPROM_SIZE)
				throw ParseException(chipkey + ".eeprom.size with unsupported value");
			if (cd->flash_page_size > (1<<AvrChipDesc::MAX_PAGE_SIZE_EXP) || cd->flash_page_size < 1
					|| !is_power_of_2(cd->flash_page_size)
					|| cd->flash_page_size > cd->flash_size)
				throw ParseException(chipkey + ".flash.page.size with invalid value");
			if (cd->eeprom_page_size > (1<<AvrChipDesc::MAX_PAGE_SIZE_EXP)
					|| !is_power_of_2(cd->eeprom_page_size)
					|| cd->eeprom_page_size > cd->eeprom_size)
				throw ParseException(chipkey + ".eeprom.page.size with invalid value");
			if (cd->flash_size % cd->flash_page_size != 0)
				throw ParseException(chipkey + ".flash.size not a multiple of .flash.page.size");
			if (cd->eeprom_page_size > 0) {
				if (cd->eeprom_size % cd->eeprom_page_size != 0)
					throw ParseException(chipkey + ".eeprom.size not a multiple of .eeprom.page.size");
			}
			if (cd->arch == AvrChipDesc::Arch_AVR) {
				if (cd->calbytes < 1 || cd->calbytes > 4)
					throw ParseException(chipkey + ".calbytes with invalid value");
				if (cd->prginterface != AvrChipDesc::PrgInterface_SPI) {
					throw ParseException("architecture AVR only supports SPI program interface!");
				}
			} else if (cd->arch == AvrChipDesc::Arch_XMEGA) {
				if (cd->prginterface != AvrChipDesc::PrgInterface_PDI) {
					throw ParseException("architecture XMEGA only supports PDI program interface!");
				}
				if (cd->flash_app_size + cd->flash_boot_size != cd->flash_size) {
					throw ParseException("flash app section size + boot section size != flash size");
				}
			}
			// calculate other values
			cd->flash_mask  = get_mask(cd->flash_size);
			cd->eeprom_mask = get_mask(cd->eeprom_size);
			
			cd->flash_poff_mask  = get_mask(cd->flash_page_size);
			cd->eeprom_poff_mask = get_mask(cd->eeprom_page_size);
			
			cd->flash_num_pages  = cd->flash_size / cd->flash_page_size;
			cd->eeprom_num_pages = cd->eeprom_page_size > 0 ? cd->eeprom_size / cd->eeprom_page_size : 0;
			
			cd->flash_page_mask  = cd->flash_mask & ~(cd->flash_poff_mask);
			cd->eeprom_page_mask = cd->eeprom_mask & ~(cd->eeprom_poff_mask);
			
			cd->flash_page_exp  = get_exp(cd->flash_page_size);
			cd->eeprom_page_exp = get_exp(cd->eeprom_page_size);
			
			_chips.add(cd.detach());
		}
	}
}
