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
#include "PicProg.h"
#include "ParPortProgDevice.h"
#include "util.h"

PicProg::PicProg(ISerialProgDevice* a_pd) : _pd(a_pd) {
	// for PIC 16F84
	_prgmem_size = 1024;
	_eeprom_size = 64;
	_cfgmem_addr = 0x2000;
	_cfgmem_size = 8;
}

PicProg::~PicProg() {
}

void PicProg::clkwait() {
	wait_ms(5);
}

void PicProg::sendbit(int a_bit) {
	_pd->sipCommitClkWd(1, a_bit);
	clkwait();
	_pd->sipCommitClk(0);
	clkwait();
	_pd->sipCommitWd(1); // -> open collector WD signal to high 
	clkwait();
}

int PicProg::recvbit() {
	_pd->sipCommitClkWd(1, 1);
	clkwait();
	bool bit = _pd->sipReadRd();
	_pd->sipCommitClk(0);
	clkwait();
	return bit;
}

void PicProg::sendcmd(int cmd) {
	for (int i = 0; i < 6; i++)
		sendbit(cmd & (1 << i));
}

void PicProg::senddata(uint16 data) {
	sendbit(0);
	for (int i = 0; i < 14; i++)
		sendbit(data & (1 << i));
	sendbit(0);
}

uint16 PicProg::recvdata() {
	_pd->setReadMode(true);
	recvbit();
	uint16 data = 0;
	for (int i = 0; i < 14; i++) {
		if (recvbit())
			data |= (1 << i);
	}
	recvbit();
	_pd->setReadMode(false);
	return data;
}

void PicProg::beg_progmode() {
	_pd->progEnable(true);
}

void PicProg::end_progmode() {
	_pd->progEnable(false);
}

void PicProg::cmd_loadcfg(uint16 data) {
	printf("cmd: load configuration: 0x%x\n", (int)data);
	sendcmd(0x00);
	senddata(data);
}

void PicProg::cmd_loaddata_pm(uint16 data) {
	printf("cmd: load data program memory: 0x%x\n", (int)data);
	sendcmd(0x02);
	senddata(data);
}

uint16 PicProg::cmd_readdata_pm() {
	printf("cmd: read data program memory: ");
	sendcmd(0x04);
	uint16 d = recvdata();
	printf("0x%x\n", (int)d);
	return d;
}

void PicProg::cmd_incaddr() {
	printf("cmd: increment address\n");
	sendcmd(0x06);
}

void PicProg::cmd_beg_erase_prog() {
	printf("cmd: begin erase and program cycle\n");
	sendcmd(0x08);
}

void PicProg::cmd_loaddata_dm(uint16 data) {
	sendcmd(0x03);
	senddata(data);
}

uint16 PicProg::cmd_readdata_dm() {
	sendcmd(0x05);
	return recvdata();
}


//void PicProg::cmd_bulkerase_pm() {
//	sendcmd(0x09);
//}

//void PicProg::cmd_bulkerase_dm() {
//	sendcmd(0x0b);
//}



void PicProg::cmd_1() {
	printf("cmd: 1\n");
	sendcmd(0x01);
}

void PicProg::cmd_7() {
	printf("cmd: 7\n");
	sendcmd(0x07);
}


// check if addr2,len2 is in addr,len
static inline bool check_range(uint16 addr, int len, uint16 addr2, int len2) {
	return (addr2 >= addr && addr2 + len2 <= addr + len);
}


void PicProg::prog_code(const ArrayP<struct code_block>& code) {
	if (code.size() == 0) {
		throw Exception("prog_code: no code blocks supplied");
	}
	uint16 pc = 0;
	for (int i = 0; i < code.size(); i++) {
		uint16 a = code[i]->addr;
		int l = code[i]->len();
		if (a < pc) {
			String msg;
			msg.format("prog_code: code blocks intersect or addresses are not sorted in rising order: addr=0x%x - 0x%x", (int)a, (int)(a + l - 1));
			throw Exception(msg);
		}
		if (!check_range(0, _prgmem_size, a, l) || !check_range(_cfgmem_addr, _cfgmem_size, a, l)) {
			String msg;
			msg.format("prog_code: code block intersects with unimplemented memory: addr=0x%x - 0x%x", (int)a, (int)(a + l - 1));
			throw Exception(msg);
		}
		pc = a + l;
	}
	
	pc = 0;
	
	beg_progmode();
	
	for (int k = 0; k < code.size(); k++) {
		struct code_block* cb = code[k];
		if (cb->addr >= _cfgmem_addr && pc < _cfgmem_addr) {
			pc = _cfgmem_addr;
			cmd_loadcfg(0x3fff); // set pc in pic to cfgmem_addr !
		}

		while (pc < cb->addr) {
			cmd_incaddr();
			pc++;
		}
		for (unsigned i = 0; i < cb->len(); i++) {
			uint16 c = cb->code[i] & PIC_WORD_MASK;
			uint16 d = cmd_readdata_pm();
			if (d != c) {
				cmd_loaddata_pm(c);
				cmd_beg_erase_prog();
				wait_ms(50);
				d = cmd_readdata_pm();
				if (d != c) {
					end_progmode();
					
					String msg;
					msg.format("program verify error at 0x%x : 0x%x != 0x%x\n", (int)pc, (int)c, (int)d);
					throw Exception(msg);
				}
			}
			cmd_incaddr();
			pc++;
		}
	}
	end_progmode();
}

void PicProg::prog_cfg(uint16 cfg) {
	cfg &= PIC_WORD_MASK;
	beg_progmode();
	cmd_loadcfg(cfg);
	for (int i = 0; i < 7; i++)
		cmd_incaddr();
	u_int16_t d = cmd_readdata_pm();
	if (d != cfg) {
		cmd_loaddata_pm(cfg);
		cmd_beg_erase_prog();
		wait_ms(50);
		d = cmd_readdata_pm();
		if (cfg != d) {
			end_progmode();
					
			String msg;
			msg.format("cfg verify error: 0x%x != 0x%x\n", (int)cfg, (int)d);
			throw Exception(msg);
		}
	}
	end_progmode();
}

int PicProg::read_code(struct code_block& code, int len, uint16 addr) {
	if (addr >= _prgmem_size) {
		String msg;
		msg.format("read_code: code block is in unimplemented memory: addr=0x%x - ?", (int)addr);
		throw Exception(msg);
	}
	if (len < 0)
		len = _prgmem_size - addr;
	if (!check_range(0, _prgmem_size, addr, len)) {
		String msg;
		msg.format("read_code: code block intersects with unimplemented memory: addr=0x%x - 0x%x", (int)addr, (int)(addr + len - 1));
		throw Exception(msg);
	}
	
	code.addr = addr;
	code.code.setSize(len);
	
	if (len <= 0)
		return 0;
	
	beg_progmode();
	
	for (uint16 i = 0; i < addr; i++) {
		cmd_incaddr();
	}
	
	for (int i = 0; i < len; i++) {
		code.code[i] = cmd_readdata_pm();
		cmd_incaddr();
	}
	
	end_progmode();
	
	return len;
}

uint16 PicProg::read_cfg() {
	beg_progmode();
	cmd_loadcfg(0x3fff);
	for (int i = 0; i < 7; i++)
		cmd_incaddr();
	uint16 d = cmd_readdata_pm();
	end_progmode();
	return d;
}

void PicProg::disable_code_protection() {
	beg_progmode();
	cmd_loadcfg(0x3fff);
	for (int i = 0; i < 7; i++) 
		cmd_incaddr();
	cmd_1();
	cmd_7();
	cmd_beg_erase_prog();
	wait_ms(100);
	cmd_1();
	cmd_7();
	end_progmode();
}


*/
