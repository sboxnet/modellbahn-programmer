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

#ifndef _AVRPROG_H_
#define _AVRPROG_H_

#include "IAvrProgDevice.h"
#include "AvrError.h"
#include "AvrChipDesc.h"
#include "AvrElf.h"

#include <tmb/lang/Array.h>

using namespace tmb;

class AvrProg {
	
	public:
		enum ProgressFlags {
			PSFLG_IS_EEPROM	= 0x0001,
   			PSFLG_WRITE_VER	= 0x0002,
			PSFLG_SIZE_WORD = 0x0004,
		};
		class Progress : public IAvrProgDevice::Progress {
			public: virtual void init(uint flags, uint a_begaddr, uint a_endaddr) = 0;
		};
	
	protected:
		IAvrProgDevice*				_pdev;
		const AvrChipDesc*			_chipdesc;
		const AvrChipDescriptions&	_chips;
		bool						_sig_checked;
		bool						_prog_enabled;
		
	private:
		AvrProg(const AvrProg&);
		void operator= (const AvrProg&);
		
	public:
		AvrProg(IAvrProgDevice* a_pdev, const AvrChipDescriptions& a_cds);
		virtual ~AvrProg();
		
		const AvrChipDesc* getChipDesc() { return _chipdesc; }
		
		void initProg();
		void enableProg();
		bool isProgEnabled() { return _prog_enabled; }
		void cleanupProg();
		
		void checkSignature();
		bool isSignatureOk() { return _sig_checked; }
		uint32 readSignature();
		
		byte readLockBits();
		void writeLockBits(byte a_lb);
		byte readFuseBits(uint a_fusenr);
		void writeFuseBits(uint a_fusenr, byte a_val);
		void readCalibrationBytes(Array<byte>& a_calbytes);
		
		void eraseChip(IAvrProgDevice::EraseMode mode);
		bool verifyChipErased(Progress* a_progress, IAvrProgDevice::EraseMode mode);
		
		void readProgMem(Progress* a_progress, Array<uint16>& a_code, uint a_startaddr = 0, int a_size = -1);
		bool writeProgMem(Progress* a_progress, const Array<uint16>& a_code, uint a_startaddr = 0);
		bool writeProgMem(Progress* a_progress, const AvrElf& a_elfobj, uint a_startaddr = 0);
		bool verifyProgMem(Progress* a_progress, const Array<uint16>& a_code, uint a_startaddr = 0);
		bool verifyProgMem(Progress* a_progress, const AvrElf& a_elfobj, uint a_startaddr = 0);
	
		void readEeprom(Progress* a_progress, Array<byte>& a_data, uint a_startaddr = 0, int a_size = -1);
		bool writeEeprom(Progress* a_progress, const Array<byte>& a_data, uint a_startaddr = 0);
		bool verifyEeprom(Progress* a_progress, const Array<byte>& a_data, uint a_startaddr = 0);

	protected:
		void checkReady();
		void checkFuseNr(uint a_fusenr);
		
};


#endif // _AVRPROG_H_

