/***************************************************************************
 *   Copyright (C) 2008-2013
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
#include "AvrChipDesc.h"
#include "AvrProg.h"
#include "AvrRawRS232ProgDevice.h"
#include "AvrGpioSerProgDevice.h"
#include "AvrGpioSerAvrProgDevice.h"
#include "AvrGpioUsbAvrRawProgDevice.h"
#include "AvrGpioUsbAvrProgDevice.h"
#include "AvrGpioUsbXmegaProgDevice.h"

#include <stdio.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iostream>
//#include <strstream>
#include <readline/readline.h>
#include <readline/history.h>
#include <tmb/lang/String.h>
#include <tmb/lang/StringTokenizer.h>
#include <tmb/io/File.h>
#include <stdlib.h>
#include <ctype.h>

using namespace std;
using namespace tmb;


#define VERSION_STR  "AVR ATtiny/ATmega/ATxmega mcp module\nVersion 2.0 Mai 2019\nCopyright (c) 2009-2019 by Thomas Maier <balagi@justmail.de>\n"
AvrChipDescriptions g_chipdescs;

#define TOSTR(_s)	#_s
#define MAX_SCLK_FREQ	100000

class AvrContext {
	
	protected:
		String		_chipname;
		String  	_serial_dev_file;
		AvrProg*	_avr_prog;
		const AvrChipDesc*	_chipdesc;
		uint		_sclkfreq;
		
	public:
		AvrContext()
				: _serial_dev_file("/dev/ttyS0"),
				  _avr_prog(0),
				  _chipdesc(0),
				  _sclkfreq(MAX_SCLK_FREQ)
		{}
		
		~AvrContext() {
		}
		
		const String& chipname() {
			return _chipname;
		}
		void chipname(const String& a_cn) {
			const AvrChipDesc* cd = g_chipdescs.find(a_cn);
			if (!cd) {
				throw NotFoundException(String(64) << "unknown chip name: " << a_cn);
			}
			_chipdesc = cd;
			_chipname = a_cn;
		}
		const AvrChipDesc* chipdesc() {
			return _chipdesc;
		}
		
		const String& serialDeviceFile() {
			return _serial_dev_file;
		}
		void serialDeviceFile(const String& a_sidev) {
			if (a_sidev.isEmpty())
				throw Exception("no serial device file specified");
			_serial_dev_file = a_sidev;
		}
	
		AvrProg* avrProg() {
			return _avr_prog;
		}
		void setAvrProg(AvrProg* a_pg) {
			_avr_prog = a_pg;
		}
		
		uint sclkFreq() {
			return _sclkfreq;
		}
		void sclkFreq(uint f) {
			_sclkfreq = f;
		}
};

typedef bool (*CmdFunc)(AvrContext& ctxt, const String& cmd, StringTokenizer& tz); 


static void usage() {
	cerr << "usage:" << endl
		 << "  mcp avr --prgdev=<prog device name> --chip=<chipname> [--sdev=<serial device file>]" << endl
		 << "          [--chipprops=<avr chips property file>] [--sclkfreq=<freq>] [--nopoll] [--noask]" << endl
		 << "          [--cmd=<commands>]" << endl
		 << "  mcp avr {--knownchips | --printspecs=<chipname>} [--chipprops=<avr chips property file>]" << endl
		 << "  mcp avr [--version | -v]" << endl
		 << endl
		 << "  known programming devices:" << endl
		 << "    rs232-pos	      Raw RS232 programming device" << endl
		 << "    rs232-neg        Raw RS232 programming device with inverted signals" << endl
		 << "    gpio-ser-pin     GPIO-Ser raw pin programming device" << endl
		 << "    gpio-ser-avr     GPIO-Ser AVR mode programming device" << endl
		 << "    gpio-ser-avr-raw GPIO-Ser AVR raw mode programming device" << endl
		 << "    gpio-usb-avr     GPIO-USB AVR mode programming device" << endl
		 << "    gpio-usb-avr-raw GPIO-USB AVR raw mode programming device" << endl
		 << "    gpio-usb-xmega   GPIO-USB AVR PDI mode programming device" << endl
		 << endl
		 << "  commands:" << endl
		 << "	 help" << endl
		 << "    knownchips" << endl
		 << "    chipspecs <chipname>"<< endl
		 << "    readsig"  << endl
		 << "    readlocks" << endl
		 << "    readfuses" << endl
		 << "    readcali" << endl
		 << "    erase chip|flash|appsec|bootsec|eeprom [verify]" << endl
		 << "    verifyerased chip|flash|appsec|bootsec|eeprom" << endl
		 << "    writefuse <fusenr> <byte>" << endl
		 << "    writelocks <byte>" << endl
		 << "    readflash <filename> [<startaddr> [<endaddr>]]" << endl
		 << "    showflash [<startaddr> [<endaddr>]]" << endl
		 << "    readeeprom <filename> [<startaddr> [<endaddr>]]" << endl
		 << "    showeeprom [<startaddr> [<endaddr>]]" << endl
		 << "    progflash [elf:|avr:|bin:]<filename> [<startaddr>]" << endl
		 << "    putflash <startaddr> <word>" << endl
		 << "    progeeprom <filename> [<startaddr>]" << endl
		 << "    puteeprom <startaddr> <byte>" << endl
		 << "    verifyflash [elf:|avr:|bin:]<elfobj> [<startaddr>]" << endl
		 << "    verifyeeprom <filename> [<startaddr>]" << endl
		 << endl
		 << "  arguments:" << endl
		 << "    addr: address in Hex 0x*, or decimal" << endl
		 << "    chipname: name of the chip, see --knownchips" << endl
		 << "    fusenr: Fuse Nr (1,2,3,4,5,..., same as in 'readfuses')" << endl
		 << "    byte: hex or decimal" << endl
		 << "    elf|avr|bin prefix: elf=file is a elf" << endl
		 << "                        avr=same as elf" << endl
		 << "                        bin=Raw Binary File" << endl
		 << endl
//		 << VERSION_STR << endl
	;
}

static void usageError() {
	cerr << "ERROR: wrong usage" << endl;
	usage();
}

void showVersion() {
	cout << VERSION_STR << endl;
}

static void check_chip_name(AvrContext& a_ctxt) {
	if (!a_ctxt.avrProg() || !a_ctxt.chipdesc()) {
		throw Exception("internal error");
	}
	a_ctxt.avrProg()->checkSignature();
}

static bool cmd_print_known_chips(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: knownchips";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "knownchips")
		return false;
	if (tz.hasNext()) {
		cout << "error: " << usage << endl;
	} else {
		g_chipdescs.printKnownChips();
	}
	return true;
}

static bool cmd_print_chip_specs(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: chipspecs <chipname>";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "chipspecs")
		return false;
	
	String chipname;
	try {
		if (tz.hasNext()) {
			chipname = tz.next();
		} else {
			chipname = ctxt.chipname();
		}
		if (tz.hasNext() || chipname.length() == 0)
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	AvrChipDesc* cd = g_chipdescs.find(chipname);
	if (!cd) {
		cout << "error: chip unknown: " << chipname << endl;
	} else {
		cd->print();
	}
	return true;
}


static bool cmd_read_signature(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: readsig";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "readsig")
		return false;

	if (tz.hasNext()) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	
	uint32 sig = ctxt.avrProg()->readSignature();
	
	printf("chip found with signature 0x%x\n", sig);
	AvrChipDesc* cd = g_chipdescs.find(sig);
	if (!cd) {
		cout << "-> unknown chip" << endl;
	} else {
		cout << "-> chip is: " << cd->name << "   with " << (cd->flash_size * 2) << " byte flash memory" << endl;
	}
	
	return true;
}

static bool cmd_read_lock_bits(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: readlocks";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "readlocks")
		return false;

	if (tz.hasNext()) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	byte b = ctxt.avrProg()->readLockBits();
	printf("lock bits: 0x%x\n", (int)b);
	return true;
}


static void print_fuse_bits(int fusenr, const Array<String>* fusedesc, byte fusebits) {
	printf("fuse %2d:  0x%02x  ", fusenr, (int)fusebits);
	if (fusedesc->size() >= 8) {
		for (int i = 7; i >= 0; i--) {
			printf(" %s=%d", fusedesc->get(i).c_str(), (fusebits & (1<<i)) ? 1 : 0);
		}
	}
	printf("\n");
}

static bool cmd_read_fuse_bits(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: readfuses";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "readfuses")
		return false;

	if (tz.hasNext()) {
		cout << "error: " << usage << endl;
		return true;
	}
	for (int i = 0; i < ctxt.chipdesc()->fuse_desc.size(); i++) {
		if (ctxt.chipdesc()->fuse_desc[i]->size() == 0)
			continue;
		byte v = ctxt.avrProg()->readFuseBits(i);
		print_fuse_bits(i, ctxt.chipdesc()->fuse_desc[i], v);
	}
	return true;
}

static bool cmd_read_calibration(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: readcali";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "readcali")
		return false;

	if (tz.hasNext()) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	Array<byte> calbytes;
	ctxt.avrProg()->readCalibrationBytes(calbytes);
	for (uint i = 0; i < calbytes.size(); i++) {
		printf("calibration byte 0x%02x: %3d (0x%02x)\n", i, (int)calbytes[i], (int)calbytes[i]);
	}
	if (calbytes.size() == 0) {
		printf("chip has no calibration bytes\n");
	}
}




class Progress : public AvrProg::Progress {
	protected:
		String	_type;
		uint	_cnt;
		uint	_flags;
		uint	_begaddr;
		uint	_endaddr;
		String  _ascii;
	public:
		Progress(const String& a_type) : _type(a_type), _cnt(0), _flags(0), _begaddr(0), _endaddr(0) {}
		uint percent(uint a_cur, uint a_beg, uint a_end);
		virtual void init(uint a_flags, uint a_begaddr, uint a_endaddr);
		virtual void next(uint addr, uint val);
		void setascii(char c, uint idx);
};

void Progress::init(uint a_flags, uint a_begaddr, uint a_endaddr) {
	_cnt = 0;
	_flags = a_flags;
	_begaddr = a_begaddr;
	_endaddr = a_endaddr;
	printf("%s %s: start address: 0x%x,  end address: 0x%x,  size: %d %s\n",
			(_flags & AvrProg::PSFLG_WRITE_VER) ? "verify" : _type.c_str(),
		    (_flags & AvrProg::PSFLG_IS_EEPROM) ? "eeprom" : "flash",
			_begaddr, _endaddr, _endaddr - _begaddr + 1,
			(_flags & AvrProg::PSFLG_IS_EEPROM) ? "bytes" : "words"
  		);
	
	if (_type == "show") {
		if (_flags & AvrProg::PSFLG_IS_EEPROM) {
			printf("Byte:   Dump                                              Ascii\n");
		} else {
			printf("Word:   Dump (Little Endian)                              Ascii\n");
		}
	}
}

void Progress::setascii(char c, uint idx) {
	if (::isprint(c)) {
		_ascii[idx] = c;
	}
}

uint Progress::percent(uint a_cur, uint a_beg, uint a_end) {
	if (a_end == 0 || a_beg >= a_end || a_cur > a_end)
		return 100;
	return (a_cur - a_beg) * 100 / (a_end - a_beg);
}

void Progress::next(uint a_addr, uint a_val) {	
/*	if (a_addr == _begaddr && _cnt == 0) {
		printf("%s: start address: 0x%x,  end address: 0x%x,  size: %d\n",
						(_flags & AvrProg::PSFLG_WRITE_VER) ? "verify" : _type.c_str(),
						_begaddr, _endaddr, _endaddr - _begaddr + 1);
	}
*/
	if (_type == "show") {
		if (_cnt == 0) {
			printf("0x%04x:", a_addr);
			_ascii = "................";
		}
		if (_flags & AvrProg::PSFLG_IS_EEPROM) {
			printf(" %02x", (a_val & 0x00ff));
			setascii((char)a_val, _cnt++);
		} else {
			printf(" %02x-%02x", (a_val & 0x0ff), ((a_val >> 8) & 0x0ff));
			setascii((char)a_val, _cnt++);
			setascii((char)(a_val >> 8), _cnt++);
		}
		_cnt &= 0x0f;
		if (_cnt == 0 || a_addr == _endaddr) {
			while (_cnt > 0 && _cnt < 16) {
				printf("   ");
				_cnt++;
			}
			printf("  |%s|\n", _ascii.c_str());
		}
		
	} else {
		printf("\r%s %s: %d%%   address=0x%04x", (_flags & AvrProg::PSFLG_WRITE_VER) ? "verify" : _type.c_str(),
												(_flags & AvrProg::PSFLG_IS_EEPROM ? "eeprom" : "flash"),
												percent(a_addr, _begaddr, _endaddr),
												a_addr);
		fflush(stdout);
		if (a_addr == _endaddr)
			printf("\n");
	}
}



void verify_chip_erased(AvrContext& ctxt, IAvrProgDevice::EraseMode emode, const String& erasemode) {
	cout << "verify if " << erasemode << " is erased..." << endl;
	Progress pg("verify");
	if (ctxt.avrProg()->verifyChipErased(&pg, emode)) {
		cout << erasemode << " is erased and empty" << endl;
	} else {
		cout << erasemode << " is NOT erased!!!" << endl;
	}
}


static bool cmd_erase(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: erase chip|flash|appsec|bootsec|eeprom [verify]";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "erase")
		return false;

	bool verify = false;
	String erasemode = "chip";
	try {
		String typ = tz.next();
		if (typ == "chip" || typ == "flash" || typ == "appsec" || typ == "bootsec" || typ == "eeprom") {
			erasemode = typ;
		} else {
			throw NotFoundException();
		}
		if (tz.hasNext()) {
			if (tz.next() == "verify")
				verify = true;
			else
				throw NotFoundException();
		}
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	if (ctxt.chipdesc()->prginterface == AvrChipDesc::PrgInterface_SPI
		  && erasemode != "chip" && erasemode != "flash") {
		cout << "error: erase mode not supported on this chip: " << erasemode << endl;
		return true;
	}

	IAvrProgDevice::EraseMode emode = IAvrProgDevice::EraseMode_CHIP;
	if (erasemode == "chip")
		emode = IAvrProgDevice::EraseMode_CHIP;
	else if (erasemode == "flash")
		emode = IAvrProgDevice::EraseMode_FLASH;
	else if (erasemode == "appsec")
		emode = IAvrProgDevice::EraseMode_APP_SECTION;
	else if (erasemode == "bootsec")
		emode = IAvrProgDevice::EraseMode_BOOT_SECTION;
	else if (erasemode == "eeprom")
		emode = IAvrProgDevice::EraseMode_EEPROM;

	if (emode == IAvrProgDevice::EraseMode_FLASH) {
		Array<byte> eeprom;
		byte fuse_val;
		byte eesave_mask = ctxt.chipdesc()->eesave_mask;
		uint eesave_fusenr = ctxt.chipdesc()->eesave_fusenr;
		if (eesave_mask) {
			cout << "device has a EESAVE fuse" << endl;
			fuse_val = ctxt.avrProg()->readFuseBits(eesave_fusenr);
			if (fuse_val & eesave_mask) {
				cout << "switch EESAVE fuse on temporarily" << endl;
				ctxt.avrProg()->writeFuseBits(eesave_fusenr, fuse_val & ~eesave_mask);
			} else {
				cout << "EESAVE fuse already on" << endl;
			}
		} else {
			cout << "device has no EESAVE fuse" << endl;
			Progress readpg("read");
			ctxt.avrProg()->readEeprom(&readpg, eeprom); // read full eeprom
		}
		cout << "erase " << erasemode << " ..." << endl;
		ctxt.avrProg()->eraseChip(IAvrProgDevice::EraseMode_CHIP);
		cout << erasemode << " erased" << endl;
		if (verify) {
			verify_chip_erased(ctxt, emode, erasemode);
		}
		if (eesave_mask) {
			if (fuse_val & eesave_mask) {
				cout << "switch EESAVE fuse off again" << endl;
				ctxt.avrProg()->writeFuseBits(eesave_fusenr, fuse_val);
			}
		} else {
			cout << "writing back eeprom" << endl;
			Progress writepg("write");
			if (ctxt.avrProg()->writeEeprom(&writepg, eeprom)) {
				cout << endl << "OK" << endl;
			} else {
				cout << endl << "FAILED" << endl;
			}
		}
		
	} else if (ctxt.chipdesc()->prginterface == AvrChipDesc::PrgInterface_PDI) {
		cout << "erase " << erasemode << " ..." << endl;
		ctxt.avrProg()->eraseChip(emode);		
		cout << erasemode << " erased" << endl;
		if (verify) {
			verify_chip_erased(ctxt, emode, erasemode);
		}
		
	} else if (ctxt.chipdesc()->prginterface == AvrChipDesc::PrgInterface_SPI) {
		cout << "erase " << erasemode << " ..." << endl;
		ctxt.avrProg()->eraseChip(IAvrProgDevice::EraseMode_CHIP);
		cout << erasemode << " erased" << endl;
		if (verify) {
			verify_chip_erased(ctxt, emode, erasemode);
		}
	}
	return true;
}

static bool cmd_verify_erased(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: verifyerased chip|flash|appsec|bootsec|eeprom";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "verifyerased")
		return false;
	
	String erasemode;
	try {
		String typ = tz.next();
		if (typ == "chip" || typ == "flash" || typ == "appsec" || typ == "bootsec" || typ == "eeprom") {
			erasemode = typ;
		} else {
			throw NotFoundException();
		}
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	IAvrProgDevice::EraseMode emode = IAvrProgDevice::EraseMode_CHIP;
	if (erasemode == "chip")
		emode == IAvrProgDevice::EraseMode_CHIP;
	else if (erasemode == "flash")
		emode == IAvrProgDevice::EraseMode_FLASH;
	else if (erasemode == "appsec")
		emode == IAvrProgDevice::EraseMode_APP_SECTION;
	else if (erasemode == "bootsec")
		emode == IAvrProgDevice::EraseMode_BOOT_SECTION;
	else if (erasemode == "eeprom")
		emode = IAvrProgDevice::EraseMode_EEPROM;
	
	verify_chip_erased(ctxt, emode, erasemode);
	return true;
}


static bool cmd_write_lock_bits(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: writelocks <byte>";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "writelocks")
		return false;

	byte bits = 0;
	String bs;
	try {
		bs = tz.next();
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	try { bits = (byte)bs.toUnsignedLong(); }
	catch(const NumberFormatException&) {
		cout << "error: invalid lock byte" << endl;
		return true;
	}
	printf("writing lock bits: 0x%x\n", (uint)bits);
	ctxt.avrProg()->writeLockBits(bits);
	return true;
}

static bool cmd_write_fuse_bits(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: writefuse <fusenr> <byte>";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "writefuse")
		return false;

	byte fuse = 0;
	uint fusenr = 0;
	String bs;
	String fnr;
	try {
		fnr = tz.next();
		bs = tz.next();
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	try { fusenr = (uint)fnr.toUnsignedLong(); }
	catch(const NumberFormatException&) {
		cout << "error: invalid fuse number" << endl;
		return true;
	}
	try { fuse = (byte)bs.toUnsignedLong(); }
	catch(const NumberFormatException&) {
		cout << "error: invalid fuse byte" << endl;
		return true;
	}
	printf("writing fuse %d byte: 0x%x\n", fusenr, (uint)fuse);
	ctxt.avrProg()->writeFuseBits(fusenr, fuse);
	return true;
}

static bool cmd_read_flash(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: readflash <filename> [<startaddr> [<endaddr>]]";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "readflash")
		return false;

	String filename;
	uint startaddr = 0;
	uint endaddr = 0;
	bool have_endaddr = false;
	try {
		filename = tz.next();
		if (tz.hasNext()) {
			String sa = tz.next();
			try { startaddr = sa.toUnsignedLong(); }
			catch(const NumberFormatException&) {
				cout << "error: invalid start address" << endl;
				return true;
			}
			if (tz.hasNext()) {
				String ea = tz.next();
				try { endaddr = ea.toUnsignedLong(); }
				catch(const NumberFormatException&) {
					cout << "error: invalid end address" << endl;
					return true;
				}
				have_endaddr = true;
				if (endaddr < startaddr) {
					cout << "error: end address < start address" << endl;
					return true;
				}
			}
		}
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	Array<uint16> mem;
	Progress readpg("read");
	ctxt.avrProg()->readProgMem(&readpg, mem, startaddr, (have_endaddr ? endaddr - startaddr + 1 : -1));
	cout << endl;
	
	FILE* fd = fopen(filename.c_str(), "w");
	if (!fd) {
		cout << "error: can not create output file" << endl;
	} else {
		if (fwrite(mem.data(), 2, mem.size(), fd) != mem.size())
			cout << "error: failed to write to output file" << endl;
		else
			cout << "OK" << endl;
		fclose(fd);
	}
	return true;
}

static bool cmd_show_flash(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: showflash [<startaddr> [<endaddr>]]";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "showflash")
		return false;

	uint startaddr = 0;
	uint endaddr = 0;
	bool have_endaddr = false;
	try {
		if (tz.hasNext()) {
			String sa = tz.next();
			try { startaddr = sa.toUnsignedLong(); }
			catch(const NumberFormatException&) {
				cout << "error: invalid start address" << endl;
				return true;
			}
			if (tz.hasNext()) {
				String ea = tz.next();
				try { endaddr = ea.toUnsignedLong(); }
				catch(const NumberFormatException&) {
					cout << "error: invalid end address" << endl;
					return true;
				}
				have_endaddr = true;
				if (endaddr < startaddr) {
					cout << "error: end address < start address" << endl;
					return true;
				}
			}
		}
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	Array<uint16> mem;
	Progress readpg("show");
	ctxt.avrProg()->readProgMem(&readpg, mem, startaddr, (have_endaddr ? endaddr - startaddr + 1 : -1));
	cout << endl;
	return true;
}

static bool cmd_read_eeprom(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: readeeprom <filename> [<startaddr> [<endaddr>]]";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "readeeprom")
		return false;

	String filename;
	uint startaddr = 0;
	uint endaddr = 0;
	bool have_endaddr = false;
	try {
		filename = tz.next();
		if (tz.hasNext()) {
			String sa = tz.next();
			try { startaddr = sa.toUnsignedLong(); }
			catch(const NumberFormatException&) {
				cout << "error: invalid start address" << endl;
				return true;
			}
			if (tz.hasNext()) {
				String ea = tz.next();
				try { endaddr = ea.toUnsignedLong(); }
				catch(const NumberFormatException&) {
					cout << "error: invalid end address" << endl;
					return true;
				}
				have_endaddr = true;
				if (endaddr < startaddr) {
					cout << "error: end address < start address" << endl;
					return true;
				}
			}
		}
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	Array<byte> mem;
	Progress readpg("read");
	ctxt.avrProg()->readEeprom(&readpg, mem, startaddr, (have_endaddr ? endaddr - startaddr + 1 : -1));
	cout << endl;
	
	FILE* fd = fopen(filename.c_str(), "w");
	if (!fd) {
		cout << "error: can not create output file" << endl;
	} else {
		if (fwrite(mem.data(), 1, mem.size(), fd) != mem.size())
			cout << "error: failed to write to output file" << endl;
		else
			cout << "OK" << endl;
		fclose(fd);
	}
	return true;
}

static bool cmd_show_eeprom(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: showeeprom [<startaddr> [<endaddr>]]";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "showeeprom")
		return false;

	uint startaddr = 0;
	uint endaddr = 0;
	bool have_endaddr = false;
	try {
		if (tz.hasNext()) {
			String sa = tz.next();
			try { startaddr = sa.toUnsignedLong(); }
			catch(const NumberFormatException&) {
				cout << "error: invalid start address" << endl;
				return true;
			}
			if (tz.hasNext()) {
				String ea = tz.next();
				try { endaddr = ea.toUnsignedLong(); }
				catch(const NumberFormatException&) {
					cout << "error: invalid end address" << endl;
					return true;
				}
				have_endaddr = true;
				if (endaddr < startaddr) {
					cout << "error: end address < start address" << endl;
					return true;
				}
			}
		}
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	Array<byte> mem;
	Progress readpg("show");
	ctxt.avrProg()->readEeprom(&readpg, mem, startaddr, (have_endaddr ? endaddr - startaddr + 1 : -1));
	cout << endl;
	return true;
}


static void get_code_from_file(const String& filename, Array<uint16> &code) {
	bool hasprefix = filename.startsWith("bin:")
	              || filename.startsWith("elf:")
				  || filename.startsWith("avr:");
	
	if (filename.startsWith("bin:") || (!hasprefix && filename.endsWith(".bin"))) {
		String fn = filename;
		if (filename.startsWith("bin:")) {
			fn = filename.substring(4);
		}
		
		Array<byte> cnt;
		File(fn).load(cnt);
		uint cntsz = cnt.size();
		if (cntsz == 0) {
			throw Exception("file is empty");
		}
		if (cntsz > 0x100000) {
			throw Exception("size of file is very big, can not load");
		}
		
		code.clear();
		code.setSize(((cntsz + 1) & ~1) / 2); // round up to word boundary
		uint codesz = code.size();
		for (uint bc = 0, wc = 0; bc < cntsz; ) {
			byte b1 = cnt[bc++];
			byte b2 = (bc < cntsz ? cnt[bc++] : 0xff);
			if (wc >= codesz) {
				throw Exception("code buffer overflow");
			}
			code[wc++] = ((uint16)b2 << 8)|b1;
		}
		
	} else if (filename.startsWith("elf:") || filename.startsWith(".avr")
			|| (!hasprefix && (filename.endsWith(".elf") || filename.endsWith(".avr")))) {
		String fn = filename;
		if (filename.startsWith("elf:") || filename.startsWith("avr:")) {
			fn = filename.substring(4);
		}
		AvrElf elfobj(fn);
		elfobj.getCode(code);
	
	} else {
		throw Exception(String("unknown file format: ") + filename);
	}
}

static bool cmd_prog_flash(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: progflash [elf:|avr:|bin:]<filename> [<startaddr>]";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "progflash")
		return false;

	String filename;
	uint startaddr = 0;
	try {
		filename = tz.next();
		if (tz.hasNext()) {
			String sa = tz.next();
			try { startaddr = sa.toUnsignedLong(); }
			catch(const NumberFormatException&) {
				cout << "error: invalid start address" << endl;
				return true;
			}
		}
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	Array<uint16> code;
	try {
		get_code_from_file(filename, code);
	} catch(const Exception& ex) {
		cout << "error: can not read code: " << ex.msg() << endl;
		return true;
	}
	
	Progress writepg("prog");
	if (ctxt.avrProg()->writeProgMem(&writepg, code, startaddr)) {
		cout << endl << "OK" << endl;
	} else {
		cout << endl << "FAILED" << endl;
	}
	return true;
}

static bool cmd_put_flash(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: putflash <startaddr> <word>";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "putflash")
		return false;

	uint startaddr = 0;
	uint16 word = 0;
	try {
		String sa = tz.next();
		try { startaddr = sa.toUnsignedLong(); }
		catch(const NumberFormatException&) {
			cout << "error: invalid start address" << endl;
			return true;
		}
		String w = tz.next();
		try { word = w.toUnsignedLong(); }
		catch(const NumberFormatException&) {
			cout << "error: invalid word" << endl;
			return true;
		}
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	Array<uint16> code;
	code.add(word);
	Progress writepg("prog");
	if (ctxt.avrProg()->writeProgMem(&writepg, code, startaddr)) {
		cout << endl << "OK" << endl;
	} else {
		cout << endl << "FAILED" << endl;
	}
	return true;
}

static bool cmd_prog_eeprom(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: progeeprom <filename> [<startaddr>]";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "progeeprom")
		return false;

	String filename;
	uint startaddr = 0;
	try {
		filename = tz.next();
		if (tz.hasNext()) {
			String sa = tz.next();
			try { startaddr = sa.toUnsignedLong(); }
			catch(const NumberFormatException&) {
				cout << "error: invalid start address" << endl;
				return true;
			}
		}
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	Array<byte> data;
	try {
		File(filename).load(data);
	} catch(const Exception& ex) {
		cout << "error: can not read input file: " << ex.msg() << endl;
		return true;
	}

	Progress writepg("write");
	if (ctxt.avrProg()->writeEeprom(&writepg, data, startaddr)) {
		cout << endl << "OK" << endl;
	} else {
		cout << endl << "FAILED" << endl;
	}
	return true;
}

static bool cmd_put_eeprom(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: puteeprom <startaddr> <byte>";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "puteeprom")
		return false;

	uint startaddr = 0;
	uint16 by = 0;
	try {
		String sa = tz.next();
		try { startaddr = sa.toUnsignedLong(); }
		catch(const NumberFormatException&) {
			cout << "error: invalid start address" << endl;
			return true;
		}
		String b = tz.next();
		try { by = b.toUnsignedLong(); }
		catch(const NumberFormatException&) {
			cout << "error: invalid byte" << endl;
			return true;
		}
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	Array<byte> data;
	data.add(by);
	Progress writepg("prog");
	if (ctxt.avrProg()->writeEeprom(&writepg, data, startaddr)) {
		cout << endl << "OK" << endl;
	} else {
		cout << endl << "FAILED" << endl;
	}
	return true;
}


static bool cmd_verify_flash(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: verifyflash [elf:|avr:|bin:]<elfobj> [<startaddr>]";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "verifyflash")
		return false;

	String filename;
	uint startaddr = 0;
	try {
		filename = tz.next();
		if (tz.hasNext()) {
			String sa = tz.next();
			try { startaddr = sa.toUnsignedLong(); }
			catch(const NumberFormatException&) {
				cout << "error: invalid start address" << endl;
				return true;
			}
		}
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	Array<uint16> code;
	try {
		get_code_from_file(filename, code);
	} catch(const Exception& ex) {
		cout << "error: can not read code: " << ex.msg() << endl;
		return true;
	}
	
	Progress writepg("verify");
	if (ctxt.avrProg()->verifyProgMem(&writepg, code, startaddr)) {
		cout << endl << "OK" << endl;
	} else {
		cout << endl << "FAILED" << endl;
	}
	return true;
}

static bool cmd_verify_eeprom(AvrContext& ctxt, const String& cmd, StringTokenizer& tz) {
	const char* usage = "usage: verifyeeprom <filename> [<startaddr>]";
	if (cmd == "help") {
		cout << usage << endl;
		return false;
	}
	if (cmd != "verifyeeprom")
		return false;

	String filename;
	uint startaddr = 0;
	try {
		filename = tz.next();
		if (tz.hasNext()) {
			String sa = tz.next();
			try { startaddr = sa.toUnsignedLong(); }
			catch(const NumberFormatException&) {
				cout << "error: invalid start address" << endl;
				return true;
			}
		}
		if (tz.hasNext())
			throw NotFoundException();
	} catch(const NotFoundException&) {
		cout << "error: " << usage << endl;
		return true;
	}
	
	Array<byte> data;
	try {
		File(filename).load(data);
	} catch(const Exception& ex) {
		cout << "error: can not read input file: " << ex.msg() << endl;
		return true;
	}
	
	Progress writepg("verify");
	if (ctxt.avrProg()->verifyEeprom(&writepg, data, startaddr)) {
		cout << endl << "OK" << endl;
	} else {
		cout << endl << "FAILED" << endl;
	}
	return true;
}



CmdFunc cmd_functions[] = {
	cmd_print_known_chips,
	cmd_print_chip_specs,
	cmd_read_signature,
	cmd_read_lock_bits,
	cmd_read_fuse_bits,
	cmd_read_calibration,
	cmd_erase,
	cmd_verify_erased,
    cmd_write_fuse_bits,
	cmd_write_lock_bits,
	cmd_read_flash,
	cmd_show_flash,
	cmd_read_eeprom,
    cmd_show_eeprom,
	cmd_prog_flash,
	cmd_put_flash,
	cmd_prog_eeprom,
	cmd_put_eeprom,
	cmd_verify_flash,
	cmd_verify_eeprom,
	NULL
};

int main_avr(int argc, char** argv) {
	AvrContext ctxt;
	String propfname = "avrchips.properties";
	
	try {	
		if (argc < 1) {
			usage();
			return 0;
		}
		String prgdev;
		bool knownchips = false;
		String printspecs;
		String chipname;
		bool gotsclkfreq = false;
		bool opt_nopoll = false;
		bool opt_noask = false;
		String opt_commands = "";
		
		for (int i = 1; i < argc; i++) {
			String arg = argv[i];
			if (arg == "--help" || arg == "-h") {
				usage();

			} else if (arg == "--version" || arg == "-v") {
				showVersion();
				break;
				
			} else if (arg.startsWith("--chip=")) {
				String cn = arg.substring(7);
				if (cn.isEmpty()) {
					usageError();
					return 1;
				}
				chipname = cn;
				
			} else if (arg.startsWith("--sdev=")) {
				String dn = arg.substring(7);
				if (dn.isEmpty()) {
					usageError();
					return 1;
				}
				ctxt.serialDeviceFile(dn);
				
			} else if (arg.startsWith("--prgdev=")) {
				String dn = arg.substring(9);
				if (dn.isEmpty()) {
					usageError();
					return 1;
				}
				prgdev = dn;
			
			} else if (arg.startsWith("--sclkfreq=")) {
				String dn = arg.substring(11);
				uint mult = 1;
				if (dn.endsWithIgnoreCase("k")) {
					mult = 1000;
					dn = dn.substring(0, dn.length() - 1);
				}
				if (dn.isEmpty()) {
					usageError();
					return 1;
				}
				try {
					uint freq = dn.toUnsignedLong();
					if (freq > MAX_SCLK_FREQ || (freq * mult) > MAX_SCLK_FREQ) {
						cerr << "ERROR: --sclkfreq parameter to high (> " TOSTR(MAX_SCLK_FREQ) ")" << endl;
						return 1;
					}
					ctxt.sclkFreq(freq * mult);
					gotsclkfreq = true;
				} catch(const NumberFormatException&) {
					cerr << "ERROR: --sclkfreq parameter invalid" << endl;
					return 1;
				}
			
			} else if (arg == "--nopoll") {
				opt_nopoll = true;
			
			} else if (arg == "--noask") {
				opt_noask = true;
				
			} else if (arg.startsWith("--chipprops=")) {
				String dn = arg.substring(12);
				if (dn.isEmpty()) {
					usageError();
					return 1;
				}
				propfname = dn;
			
			} else if (arg == "--knownchips") {
				knownchips = true;
				
			} else if (arg.startsWith("--printspecs=")) {
				String dn = arg.substring(13);
				if (dn.isEmpty()) {
					usageError();
					return 1;
				}
				printspecs = dn;
			
			} else if (arg.startsWith("--cmd=")) {
cout << "--- cmd:" << arg << "----------------------------------------" << endl;
String dn = arg.substring(6);
if (dn.isEmpty()) {
	usageError();
	return 1;
}
char *p = (char *) dn;
opt_commands = p;
cout << "==== cmd: " << opt_commands << endl;
//				String dn = arg.substring(6);
//				if (dn.isEmpty()) {
//					usageError();
//					return 1;
//				}
//
//				opt_commands.append("\n");
//				opt_commands.append(dn);
				
			} else {
				cerr << "ERROR: unknown argument: " << arg << endl;
				usageError();
				return 1;
			}
		}
		
		cout << VERSION_STR << endl;
		cout << "Loading AVR Chips specs from " << propfname << endl;
		g_chipdescs.load(propfname);
		
		if (knownchips) {
			g_chipdescs.printKnownChips();
			return 0;
		}
		if (!printspecs.isEmpty()) {
			AvrChipDesc* cd = g_chipdescs.find(printspecs);
			if (!cd) {
				cout << "error: chip unknown: " << printspecs << endl;
				return 1;
			} else {
				cd->print();
				return 0;
			}
		}
		
		
		if (prgdev.isEmpty()) {
			cerr << "ERROR: no programming device specified!" << endl;
			usageError();
			return 1;
		}
		if (chipname.isEmpty()) {
			cerr << "ERROR: no chip specified!" << endl;
			usageError();
			return 1;
		}
		
		ctxt.chipname(chipname);
		
		IAvrSerialProgDevice* pdev = NULL;
		int sclkf = -1;
		if (prgdev == "rs232-pos") {
			pdev = new AvrRawRS232ProgDevice(ctxt.serialDeviceFile(), false, ctxt.chipdesc());
		} else if (prgdev == "rs232-neg") {
			pdev = new AvrRawRS232ProgDevice(ctxt.serialDeviceFile(), true, ctxt.chipdesc());
		} else if (prgdev == "gpio-ser-pin") {
			pdev = new AvrGpioSerProgDevice(ctxt.serialDeviceFile(), ctxt.chipdesc());
		} else if (prgdev == "gpio-ser-avr-raw") {
			AvrGpioSerAvrRawProgDevice* p = new AvrGpioSerAvrRawProgDevice(ctxt.serialDeviceFile(), ctxt.chipdesc(),
														ctxt.sclkFreq(),
														opt_nopoll ? AvrGpioSerAvrRawProgDevice::FLG_NOPOLL : 0);
			sclkf = p->getSclkFreq();
			pdev = p;
		} else if (prgdev == "gpio-ser-avr") {
			AvrGpioSerAvrProgDevice* p = new AvrGpioSerAvrProgDevice(ctxt.serialDeviceFile(), ctxt.chipdesc(),
														ctxt.sclkFreq(),
														opt_nopoll ? AvrGpioSerAvrRawProgDevice::FLG_NOPOLL : 0);
			sclkf = p->getSclkFreq();
			pdev = p;
			
		} else if (prgdev == "gpio-usb-avr-raw") {
			AvrGpioUsbAvrRawProgDevice* p = new AvrGpioUsbAvrRawProgDevice(ctxt.chipdesc(),
														ctxt.sclkFreq(),
														opt_nopoll ? AvrGpioUsbAvrRawProgDevice::FLG_NOPOLL : 0);
			sclkf = p->getSclkFreq();
			pdev = p;
			ctxt.serialDeviceFile("USB");
			
		} else if (prgdev == "gpio-usb-avr") {
			AvrGpioUsbAvrProgDevice* p = new AvrGpioUsbAvrProgDevice(ctxt.chipdesc(),
														ctxt.sclkFreq(),
														opt_nopoll ? AvrGpioUsbAvrProgDevice::FLG_NOPOLL : 0);
			sclkf = p->getSclkFreq();
			pdev = p;
			ctxt.serialDeviceFile("USB");

			
		} else if (prgdev == "gpio-usb-xmega") {
			AvrGpioUsbXmegaProgDevice* p = new AvrGpioUsbXmegaProgDevice(ctxt.chipdesc(),
														ctxt.sclkFreq());
			sclkf = p->getSclkFreq();
			pdev = p;
			ctxt.serialDeviceFile("USB");

		} else {
			cerr << "ERROR: unknown programming device " << prgdev << endl;
			usageError();
			return 1;
		}

		cout << "Using programming device: " << pdev->name() << endl;
		if (sclkf >= 0) {
			cout << "  requested SCLK frequency:   " << ctxt.sclkFreq() << endl;
			cout << "  device uses SCLK frequency: " << sclkf << endl;
		} else if (gotsclkfreq) {
			cout << "  Warning: --sclkfreq option not supported for this device" << endl;
		}
		cout << "Using device for communication: " << ctxt.serialDeviceFile() << endl;
		
		AvrProg pg(pdev, g_chipdescs);
		ctxt.setAvrProg(&pg);
		cout << "Initializing communication." << endl;
		pg.initProg();
	
		if (!opt_noask) {
			cout << "Please connect the AVR chip to the programmer. Continue? y/n: " << flush;
			int c = fgetc(stdin);
			// eat up newline
			int nl = c;
			while (nl != '\n' && nl != EOF) {
				nl = fgetc(stdin);
			}
			cout << endl;
			if (c != 'y')
				return 0;
		}
		
		cout << "Enabling programming mode on the chip." << endl;
        pg.enableProg();
        
		cout << "Check if chip is: " << ctxt.chipname() << endl;
		check_chip_name(ctxt);
		cout << "Connected chip is: " << ctxt.chipname() << endl;
		
		istream* infile;
        std::stringstream cmd;
		opt_commands.replace(';','\n');
		String commands;
        char   cm [256];
		bool istty = false;
        if (opt_commands.isEmpty()) {
			infile = &cin;
            istty = (isatty(0) == 1);
			if (istty) {
				rl_bind_key('\t', rl_insert);
				stifle_history(100);
			}
		} else {
            //cout << "opt_cmd:" << opt_commands.c_str() << endl;
            cmd.str(opt_commands.c_str());
            infile = &cmd;
//			::strcpy(cm, opt_commands.c_str());
            //commands = commands.c_str();
		}
		
		for(;;) {
			if (infile->eof()) {
				cout << "EOF" << endl;
				break;
			}
			String input_line;
			if (istty) {
				char* line = readline("AVR> ");
				if (line == NULL)
					continue;
				input_line = line;
				free(line);
				input_line.stripWhiteSpace();
				if (input_line.isEmpty())
					continue;
				add_history(input_line.c_str());
			} else {
				char inbuffer[1024];
				inbuffer[0] = 0;
				infile->getline(inbuffer, sizeof(inbuffer)-2);
				if (infile->bad()) {
					cout << "ERROR: failed to read input" << endl;
					break;
				}
                input_line = inbuffer;
                cout << "CMDS: " << input_line << endl;
/*
                if (::strcmp(commands.str(), cm) == 0)
                {
					cout << inbuffer << endl;
				}
				input_line = inbuffer;
*/
			}
			
			StringTokenizer tz(input_line, "#", "\"\'");
			String cmd;
			try {
				cmd = tz.next();
			} catch(const NotFoundException& ex) {
				continue;
			}
//cout << "Token: " << cmd << endl;
			
			if (cmd == "exit" || cmd == "quit")
				break;
			
			bool cmdhelp = false;
			if (cmd == "help" || cmd == "?") {
				cmdhelp = true;
				cmd = "help";
			}
			
			try {
				CmdFunc* pcf = cmd_functions;
				for (; *pcf; pcf++) {
					if ( (*pcf)(ctxt, cmd, tz) )
						break;
				}
				if (!cmdhelp && *pcf == NULL) {
					cout << "error: unknown command" << endl;
				}
			} catch(const AvrError& err) {
				cout << "error: " << err.msg() << endl;
			}
		}
		
		cout << "Disabling programming mode on the chip." << endl;
		pg.cleanupProg();
		cout << "End." << endl;
		
	} catch(ErrorException ex) {
		cerr << "ERROR: " << ex.msgErrno() << endl;
		return 1;
	} catch(Exception ex) {
		cerr << "ERROR: " << ex.msg() << endl;
		return 1;
	}
	return 0;
}
