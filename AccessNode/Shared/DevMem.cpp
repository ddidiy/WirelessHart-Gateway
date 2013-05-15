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

// DevMem.cpp: implementation of the CDevMem class.
//
//////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <unistd.h>
#include <time.h>

#include "Common.h"


#include "DevMem.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define MAP_SIZE	12288UL
#define MAP_MASK	(MAP_SIZE - 1)

CDevMem::CDevMem()
{
	m_nFd = -1;
	m_unBaseAddr = 0;
	m_pBaseMap = NULL;
}

CDevMem::~CDevMem()
{
	Close();
}

int CDevMem::Open(unsigned int p_unAddress)
{
	Close();

	void *	    map_base;

    // open /dev/mem for accessing mem
    if ((m_nFd = open("/dev/mem", O_RDWR | O_SYNC)) < 0)
	{
		LOG_ERR( "CDevMem::Open: Couldn't open /dev/mem device");
    	return 0;
	}

    //target = strtoul (ARM_IOPORT_BASE, 0 , 0);
	
	if ((map_base = mmap (0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_nFd, p_unAddress & ~MAP_MASK)) == (void *)-1)
	{
		LOG_ERR( "CArmDPort::Init: Couldn't map IO ports");
		Close();
    	return 0;
	}
	m_unBaseAddr = p_unAddress & ~MAP_MASK;

    m_pBaseMap = (unsigned char *)map_base;
    LOG( "CDevMem::Open: mem %x mapped at %p", m_unBaseAddr, m_pBaseMap );
	return 1;
}

void CDevMem::Close()
{
	if (m_nFd >= 0)
	{
		close(m_nFd);
	}
	if (m_pBaseMap)
	{	munmap(m_pBaseMap, MAP_SIZE);
		m_pBaseMap = NULL;
	}
	m_unBaseAddr = 0;
	
}

void* CDevMem::MapAddress(unsigned int p_unAddress)
{
	if (m_nFd >= 0)
	{
		if ( m_unBaseAddr <= p_unAddress && p_unAddress < (m_unBaseAddr + MAP_SIZE) ) 
		{
			return m_pBaseMap + (p_unAddress - m_unBaseAddr);
		}
	}

	if (!Open(p_unAddress))
	{
		return NULL;
	}

	return m_pBaseMap + (p_unAddress - m_unBaseAddr);
}

unsigned int CDevMem::ReadInt(unsigned int p_unAddress)
{
	unsigned int* pReg = (unsigned int*) MapAddress(p_unAddress);

	return pReg ? *pReg : 0;
}

void CDevMem::WriteInt(unsigned int p_unAddress, unsigned int p_unValue)
{
	unsigned int* pReg = (unsigned int*) MapAddress(p_unAddress);

	if (pReg)
	{	*pReg = p_unValue;
	}
}

unsigned short CDevMem::ReadShort(unsigned int p_unAddress)
{
	unsigned short* pReg = (unsigned short*) MapAddress(p_unAddress);

	return pReg ? *pReg : 0;
}

void CDevMem::WriteShort(unsigned int p_unAddress, unsigned short p_unValue)
{
	unsigned short* pReg = (unsigned short*) MapAddress(p_unAddress);

	if (pReg)
	{	*pReg = p_unValue;
	}
}

unsigned char CDevMem::ReadByte(unsigned int p_unAddress)
{
	unsigned char* pReg = (unsigned char*) MapAddress(p_unAddress);

	return pReg ? *pReg : 0;
}

void CDevMem::WriteByte(unsigned int p_unAddress, unsigned char p_unValue)
{
	unsigned char* pReg = (unsigned char*) MapAddress(p_unAddress);

	if (pReg)
	{	*pReg = p_unValue;
	}
}

unsigned int CDevMem::ReadBit(unsigned int p_unAddress, unsigned int p_nBit)
{
	unsigned int* pReg = (unsigned int*) MapAddress(p_unAddress);

	return pReg ? ((*pReg & (1<<p_nBit)) != 0) : 0;
}

void CDevMem::WriteBit(unsigned int p_unAddress, unsigned int p_nBit, int p_unValue)
{
	unsigned int* pReg = (unsigned int*) MapAddress(p_unAddress);

	if (!pReg)
	{	return;	
	}

	if (p_unValue)
	{
		*pReg |= (1<<p_nBit);
	}
	else
	{
		*pReg &= ~(1<<p_nBit);
	}

}
