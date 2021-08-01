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
#include "PicProg.h"

#include <tmb/io/IOException.h>
#include <tmb/io/File.h>

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include <iostream>

using namespace std;
using namespace tmb;


bool prepare(ObjPtr<PicProg>& p) {
	ParPortProgDevice* pd = new ParPortProgDevice();
	p.assign(new PicProg(pd));
	pd->open();
	printf("ready? y/n: ");
	char c = getchar();
	printf("\n");
	return c == 'y';
}


byte get_inhx_byte(const char*& s) {
	if (isxdigit(s[0]) && isxdigit(s[1])) {
		char buf[3] = { s[0], s[1], 0 };
		u_int8_t b = strtoul(buf, NULL, 16);
		s += 2;
		return b;
	}
	return 0;
}

uint16 get_inhx_word(const char*& s) {
	byte b1 = get_inhx_byte(s);
	byte b2 = get_inhx_byte(s);
	return ((uint16)b1 << 8) + (uint16)b2;
}

void read_inhx(const char* fname, ArrayP<struct code_block>& code) {
	errno = 0;
	FILE* f = fopen(fname, "r");
	if (!f) {
		throw IOException("unable to open inhx* input file", errno);
	}
	
	code.clear();
	
	bool err = false;
	char* line = NULL;
	size_t len = 0;
	ssize_t r;
	while ((r = getline(&line, &len, f)) != -1) {
		if (len < 11 || line[0] != ':') {
			err = true;
			cerr << "invalid hex file line: " << line << endl;
			break;
		}
		
		const char* p = line + 1;
		
		byte   numb = get_inhx_byte(p);
		uint16 addr = get_inhx_word(p) / 2;
		byte   type = get_inhx_byte(p);
		
		if (type == 1) // end record
			break;
		if (type != 0) // no data record?
			continue;
		
		
		if (code.size() == 0 || addr > code.getLast()->addr + code.getLast()->code.size()) {
			struct code_block* c = new struct code_block;
			c->addr = addr;
			code.add(c);
		
		} else if (addr < code.getLast()->addr + code.getLast()->code.size()) {
			err = true;
			cerr << "hex file line address below previous address: " << line << endl;
			break;
		}
		
		while (numb > 0) {
			byte b1 = get_inhx_byte(p);
			byte b2 = (numb > 1 ? get_inhx_byte(p) : 0);
			uint16 w = ((uint16)b2 << 8) + (uint16)b1;
			code.getLast()->code.add(w & PIC_WORD_MASK);
			numb -= 2;
			
		//	printf("code dump: 0x%x  0x%x\n", (int)(cur.addr + cur.code.size() - 1),
		//									  (int)w);
		}
	}
	if (line)
		free(line);
	
	fclose(f);
//	printf("code[0]->len() == %d\n", code[0]->len());	
	if (err)
		throw IOException("errors in inhx* file");
}


int main_pic(int a_argc, char** a_argv) {
	try {
		ObjPtr<PicProg> pic;
		if (a_argc == 3 && strcmp("-pcode", a_argv[1]) == 0) {
			ArrayP<struct code_block> code(AutoDelete_ALL);
			read_inhx(a_argv[2], code);
			cout << "program code from file " << a_argv[2] << ":\n";
			if (prepare(pic)) {
				pic->prog_code(code);
			}
		
		} else if (a_argc == 3 && strcmp("-rcode", a_argv[1]) == 0) {
			struct code_block code;
			if (prepare(pic)) {
				pic->read_code(code, -1, 0);
				FILE* f = fopen(a_argv[2], "w");
				if (!f) {
					perror("can not write to output file");
				} else  {
					for (int i = 0; i < code.len(); i++) {
						fprintf(f, "0x%x: 0x%x\n", i, (int)code.code[i]);
					}
					fclose(f);
				}
			}
			
		} else if (a_argc == 3 && strcmp("-pcfg", a_argv[1]) == 0) {
			uint16 cfg = strtoul(a_argv[2], NULL, 0);
			printf("program configuration word: 0x%x\n", (int)cfg);
			if (prepare(pic)) {
				pic->prog_cfg(cfg);
				printf("done\n");
			}
		
		} else if (a_argc == 2 && strcmp("-rcfg", a_argv[1]) == 0) {
			if (prepare(pic)) {
				uint16 cfg = pic->read_cfg();
				printf("cfg = 0x%x\n", (int)cfg);
			}
		
		} else if (a_argc == 2 && strcmp("-dcp", a_argv[1]) == 0) {
			cout << "disable code protection and erase device?\n";
			if (prepare(pic)) {
				pic->disable_code_protection();
			}
			
		} else {
			cerr << "usage:\n"
				 << " mcp pic -pcode <code-inhx*-file> # program code into chip\n"
				 << " mcp pic -rcode <output-file>     # read code from chip\n"
				 << " mcp pic -pcfg <cfg-word>         # program configuration word\n"
				 << " mcp pic -rcfg                    # read configuration word\n"
				 << " mcp pic -dcp                     # disable code protection and erase chip\n";
			return 1;
		}
	} catch(ErrorException ex) {
		cerr << ex.msgErrno() << endl;
		return 1;
	} catch(Exception ex) {
		cerr << ex.msg() << endl;
		return 1;
	}
	return 0;
}
*/
