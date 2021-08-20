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

#ifndef _PICPROG_H_
#define _PICPROG_H_

/*
#include "ISerialProgDevice.h"
#include <tmb/lang/ArrayP.h>

using namespace tmb;

struct code_block {
	uint16			addr;
	Array<uint16>	code;
	
	int len() { return code.size(); }
};

//#define MAX_PRGMEM_ADDR	(prgmem_size - 1)
//#define MAX_EEPROM_ADDR (eeprom_size -1)
//#define CFGWORD_ADDR (cfgmem_addr + 7)
#define PIC_WORD_SIZE  14
#define PIC_WORD_MASK  0x3fff

class PicProg {
	
	protected:
		uint16	_prgmem_size;
		uint16	_eeprom_size;
		uint16	_cfgmem_addr;
		uint16	_cfgmem_size;

	
	protected:
		ObjPtr<ISerialProgDevice> _pd;
		
		void clkwait();
		void sendbit(int bit);
		int  recvbit();
		void sendcmd(int cmd);
		void senddata(uint16 data);
		uint16 recvdata();
		void beg_progmode();
		void end_progmode();
		
		void      cmd_loadcfg(uint16 data);
		void      cmd_loaddata_pm(uint16 data);
		uint16    cmd_readdata_pm();
		void      cmd_incaddr();
		void      cmd_beg_erase_prog();
		void      cmd_loaddata_dm(uint16 data);
		uint16    cmd_readdata_dm();
		//void cmd_bulkerase_pm();
		//void cmd_bulkerase_dm();
		void      cmd_1();
		void      cmd_7();
		
	public:
		PicProg(ISerialProgDevice* pd);
		virtual ~PicProg();
			
		void  prog_code(const ArrayP<struct code_block>& code);
	//	void prog_data(u_int8_t* data, int len, int offs = 0);
		void prog_cfg(uint16 cfg);
	
		int  read_code(struct code_block& code, int len, uint16 addr = 0);
	//	void read_data(u_int8_t* data, int len, int offs = 0);
		uint16 read_cfg();

		void disable_code_protection();
};

*/

#endif // _PICPROG_H_
