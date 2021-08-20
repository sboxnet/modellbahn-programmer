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

#ifndef _AVRCHIPDESC_H_
#define _AVRCHIPDESC_H_

#include <tmb/lang/String.h>
#include <tmb/lang/ArrayP.h>
#include <tmb/io/File.h>
#include <tmb/util/Properties.h>

using namespace tmb;


class AvrChipDesc {
	public:
		enum FuseNr {
			Fuse_LOW = 0,
			Fuse_HIGH = 1,
			Fuse_EXT = 2,
		};
		enum {
			DEFAULT_HAS_CMDPOLL		= 0,
			DEFAULT_HAS_CMDEEPROMPAGEWRITE = 0,
			
			MAX_FLASH_SIZE			= 0x01000000ul,
			MAX_EEPROM_SIZE			= 0x00010000ul,
			MAX_PAGE_SIZE_EXP		= 15,
		};
		
		enum PrgInterface {
			PrgInterface_SPI = 0,
			PrgInterface_PDI = 1,
		};
		
		enum Arch {
			Arch_AVR = 0,
			Arch_XMEGA = 1,
		};
		
	public:
		String			name;			// chip name
		Arch			arch;			// architecture
		uint32			signature;   	// signature
		uint			flash_size;		// size of flash memory in words (16bit)
		uint			flash_app_size;
		uint			flash_boot_size;
		uint			eeprom_size;	// size of eeprom in bytes
		uint			ram_beg_addr;	// RAM start address
		uint			ram_end_addr;	// RAM end address
		uint			io_beg_addr;	// IO space start address
		uint			io_end_addr;	// IO space end address
		uint			flash_page_size;	// flash page size in words
		uint			eeprom_page_size;	// eeprom page size in bytes
		uint			calbytes;		// number of calibration bytes
		PrgInterface    prginterface;
		uint			spi_twd_flash;		// wait delay (ms) for flash
		uint			spi_twd_eeprom;		// wait delay (ms) for eeprom
		uint			spi_twd_erase;		// wait delay (ms) for erase chip
		uint			spi_twd_fuse;		// wait delay (ms) for fuses/locks
		uint			spi_has_cmdpoll;	// has poll command
		uint			spi_has_eeprom_page_write;	// has eeprom page write command
		ArrayP<Array<String> > fuse_desc;
		
		// calculated values...
		uint			flash_mask;
		uint			eeprom_mask;
		uint			ram_size;
		uint			io_size;
		uint			flash_poff_mask;
		uint			flash_num_pages;
		uint			flash_page_mask;
		uint			flash_page_exp;		// page size exponent:  2^n
		uint			eeprom_poff_mask;
		uint			eeprom_num_pages;
		uint			eeprom_page_mask;
		uint			eeprom_page_exp;	// page size exponent:  2^n
		uint			eesave_fusenr;
		uint8			eesave_mask;

	public:
		AvrChipDesc();
		~AvrChipDesc();
	private:
		AvrChipDesc(const AvrChipDesc&);
		void operator=(const AvrChipDesc&);
		
	public:
		void print() const;
		byte sigByte(uint n) const;
};

class AvrChipDescriptions {
	protected:
		ArrayP<AvrChipDesc>	_chips;
		
	public:
		AvrChipDescriptions();
		~AvrChipDescriptions();
	private:
		AvrChipDescriptions(const AvrChipDescriptions&);
		void operator=(const AvrChipDescriptions&);
		bool parseFuseDesc(int fuseidx, const String& chipkey, AvrChipDesc* cd, const HashMap<String>& parmap);

	public:
		AvrChipDesc* find(uint32 a_sign) const;
		AvrChipDesc* find(const String& a_name) const;
		void printKnownChips() const;
		
		void load(const String& a_filename);
};



#endif // _AVRCHIPDESC_H_
