/*
* Copyright (C) 2013 Nivis LLC.
* Email:   opensource@nivis.com
* Website: http://www.nivis.com
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3 of the License.
* 
* Redistribution and use in source and binary forms must retain this
* copyright notice.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

// DevMem.h: interface for the CDevMem class.
//
/// @addtogroup libshared
/// @{
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEVMEM_H__AE79C8FA_D3D1_42C7_8294_5738391C6A8E__INCLUDED_)
#define AFX_DEVMEM_H__AE79C8FA_D3D1_42C7_8294_5738391C6A8E__INCLUDED_


#define ARM9_GPIO_BASE	0x80840000

#define ARM9_PADR	0x80840000
#define ARM9_PBDR	0x80840004
	
#define ARM9_PADDR	0x80840010
#define ARM9_PBDDR	0x80840014

class CDevMem  
{
public:
	CDevMem();
	virtual ~CDevMem();

	// method Open is not necessary to be called, other methods call it if needed  
	//	use Open to optimize the usage if you expect to access values from 2 or more memory pages
	//	parameter p_unAddress should indicate the lowest address you intend to access (or a address on the same page)
	int Open (unsigned int p_unAddress);
	void Close();

public:
	void* MapAddress (unsigned int p_unAddress);
	void WriteInt (unsigned int p_unAddress, unsigned int p_unValue);
	unsigned int ReadInt (unsigned int p_unAddress);

	void WriteBit(unsigned int p_unAddress, unsigned int p_nBit, int p_unValue);
	unsigned int ReadBit(unsigned int p_unAddress, unsigned int p_nBit);

	void WriteByte(unsigned int p_unAddress, unsigned char p_unValue);
	unsigned char ReadByte(unsigned int p_unAddress);

	void WriteShort(unsigned int p_unAddress, unsigned short p_unValue);
	unsigned short ReadShort(unsigned int p_unAddress);

private:
	int m_nFd;
	unsigned int	m_unBaseAddr;
	unsigned char*	m_pBaseMap;

};

/// @}
#endif // !defined(AFX_DEVMEM_H__AE79C8FA_D3D1_42C7_8294_5738391C6A8E__INCLUDED_)
