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

#include <tmb/lang/String.h>
#include <tmb/lang/System.h>

#include <iostream>

using namespace std;
using namespace tmb;


//extern int main_pic(int argc, char** argv);
extern int main_avr(int argc, char** argv);
extern void showVersion();

int main(int argc, char** argv) {
	
	System::secure();
	
	cout << "--- MicroControllerProgrammer ---" << endl;
	
	if (argc >= 2) {
		String mode = argv[1];
		if (mode == "--version" || mode == "-v") {
			showVersion();
			return 0;
		}
		if (mode == "avr")
			return main_avr(argc - 1, argv + 1);
	}
	cerr << "usage: mcp [avr|-v|--version] ..." << endl;
	return 1;
}

