/***************************************************************************
 *   Copyright (C) 2014
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


#ifndef _AVRGPIOUSBXMEGAPROGDEVICE_H_
#define _AVRGPIOUSBXMEGAPROGDEVICE_H_

#include "AvrError.h"
#include "IAvrSerialProgDevice.h"
#include <tmb/lang/String.h>
#include <tmb/io/UsbDevice.h>

using namespace tmb;

class AvrGpioUsbXmegaProgDevice : public IAvrSerialProgDevice {
	
	public:
		enum CMD {
			CMD_PDI_ENABLE         = 0x40,
			CMD_PDI_DISABLE        = 0x41,
			CMD_PDI_LDS            = 0x42,
			CMD_PDI_STS            = 0x43,
			CMD_PDI_LD             = 0x44,
			CMD_PDI_ST             = 0x45,
			CMD_PDI_LDCS           = 0x46,
			CMD_PDI_STCS           = 0x47,
			CMD_PDI_REPEAT         = 0x48,
			CMD_PDI_KEY            = 0x49,
			CMD_PDI_GETSTATUS      = 0x4a,
			CMD_ENTER_PDI          = 0x4b,
			CMD_LEAVE_PDI          = 0x4c,
			
			CMD_NVMC_ENABLE           = 0x50,
			CMD_NVMC_DISABLE          = 0x51,
			CMD_NVMC_FLASH_READ       = 0x52,
			CMD_NVMC_FLASH_WRITE_PAGE = 0x53,
			CMD_NVMC_EEPROM_READ      = 0x54,
			CMD_NVMC_EEPROM_WRITE     = 0x55,
			CMD_NVMC_FUSE_READ        = 0x56,
			CMD_NVMC_FUSE_WRITE       = 0x57,
			CMD_NVMC_SIGNROW_READ     = 0x58,
			CMD_NVMC_NVM_READ         = 0x59,
			CMD_NVMC_GET_DEVID        = 0x60,
			CMD_NVMC_ERASE            = 0x61,
		};
	
	protected:
		uint		_sclkfreq;
		UsbDevice	_usbdev;
		
	private: // disable copy operator
		AvrGpioUsbXmegaProgDevice(const AvrGpioUsbXmegaProgDevice&);
		void operator= (const AvrGpioUsbXmegaProgDevice&);
	
	public:
		AvrGpioUsbXmegaProgDevice(const AvrChipDesc* a_cd, uint a_sclkfreq = 0);
		virtual ~AvrGpioUsbXmegaProgDevice();
				
	protected:
		byte getStatus();
		void checkStatus();
		
		virtual void cmdReadProgMem(uint a_addr, uint a_size, Array<uint16>& a_mem, Progress* a_pg);
		virtual void cmdProgramProgMemPage(uint a_addr, const Array<uint16>& a_mem, uint a_offs, Progress* a_pg);
		virtual void cmdReadEeprom(uint a_addr, uint a_size, Array<byte>& a_mem, Progress* a_pg);
		virtual void cmdProgramEepromPage(uint a_addr, const Array<byte>& a_mem, uint a_size, uint a_offs, Progress* a_pg);

		virtual uint16 cmdReadProgMem(uint a_addr);
		virtual void cmdLoadProgMemPage(uint a_addr, uint16 a_data);
		virtual void cmdWriteProgMemPage(uint a_addr);
		virtual byte cmdReadEeprom(uint a_addr);
		virtual void cmdWriteEepromByte(uint a_addr, byte a_data);
		virtual void cmdLoadEepromPage(uint a_addr, byte a_data);
		virtual void cmdWriteEepromPage(uint a_addr);
		virtual byte cmdReadLockBits();
		virtual void cmdWriteLockBits(byte a_bits);
		virtual uint32 cmdReadSignature();
		virtual byte cmdReadFuseBits(uint a_fusenr);
		virtual void cmdWriteFuseBits(uint a_fusenr, byte a_bits);
		virtual byte cmdReadCalibrationByte(uint a_addr);
		virtual bool cmdPollBusy();
		virtual void cmdChipErase(EraseMode mode);
		virtual bool cmdProgEnable();
		virtual void cmdLoadExtAddrByte(byte a_extaddr);

		virtual void progWait(uint a_ms);
		
	public:
		virtual uint getSclkFreq();
				
		virtual void open();
		virtual void close();
		virtual void setProgMode(bool a_on);
		
	protected:
		virtual uint32 sendCmd(uint32 a_cmd, bool a_checkcomm = true);

	private:
		virtual void sipCommit();
		virtual void sipRead();

};

#endif // _AVRGPIOUSBXMEGAPROGDEVICE_H_

