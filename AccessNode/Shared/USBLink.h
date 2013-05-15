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

/***************************************************************************
                          CUSBLink.h  -  description
                             -------------------
    begin                : Mon Sept 13 2010
    email                : nicolae.bodislav@nivis.com
 ***************************************************************************/



#ifndef __USB_LINK_H
#define __USB_LINK_H

#include <stdio.h>
#include <stdint.h>
#include "Common.h"
#include <libusb.h>
#include <utility>
#include <list>

/// @addtogroup libshared
/// @{

#define DEFAULT_READ_ENDPOINT 0x81
#define DEFAULT_WRITE_ENDPOINT 0x02


///////////////////////////////////////////////////////////////////////////////
///@author      Nicolae Bodislav
///@date        13.09.2010
///@brief       Declaration of the CUSBLink class for communication with a device on USB
///////////////////////////////////////////////////////////////////////////////
class CUSBLink
{
//types
public:
    enum {MAX_LENGTH = 511};
public:
    struct USBDeviceDescriptor
    {
        uint16_t m_nVendorID;
        uint16_t m_nProductID;
        uint8_t m_nDeviceAddress;
        uint8_t m_nBusNumber;

        bool operator==(const USBDeviceDescriptor &p_oOther) const {return (m_nVendorID == p_oOther.m_nVendorID && m_nProductID == p_oOther.m_nProductID && m_nDeviceAddress == p_oOther.m_nDeviceAddress && m_nBusNumber == p_oOther.m_nBusNumber); }
    };

    enum DealocState
    {
        LIBUSB    = 1,
        OPEN,
        INTERFACE,
        TRANSFER
    };

//variables
protected:

    bool m_bRawLog;

private:
    libusb_context *m_pContex;
    struct libusb_device_handle *m_pDevh;
    libusb_device *m_pDev;
    struct libusb_transfer *m_pData_transfer;
    unsigned char m_pData[MAX_LENGTH];
    bool  m_bHaveData;
    DealocState m_enumDealoc;
    int m_nTrasferedLength;
    uint16_t m_nCb;
    unsigned char m_cReadEndpoint;
    unsigned char m_cWriteEndpoint;


//methods
private:
    void Transfer(bool p_bHaveData, int p_nTransferedLength);
    static void Cb_TransferRead(struct libusb_transfer *transfer);
    static void Cb_TransferWrite(struct libusb_transfer *transfer);

public:
    CUSBLink(bool p_bRawLog = true);
    ~CUSBLink();

    static std::list<USBDeviceDescriptor> DevicesList();

    int OpenLink(CUSBLink::USBDeviceDescriptor p_oDevDesc);

    virtual int WriteBulk(const unsigned char* p_pData, uint16_t p_nLegth);

    virtual int ReadBulk(unsigned char* p_pData, uint16_t p_nLegth, unsigned int p_nTimeout = 10);

    void InitEndpoints(unsigned char p_cReadEndpoint, unsigned char p_cWriteEndpoint){ m_cReadEndpoint = p_cReadEndpoint; m_cWriteEndpoint = p_cWriteEndpoint; }

};

///@author      Gabriel Ghervase
///@date        15.03.2011
///@details     Declaration of the CFragmentedUSBLink class for communication with a device on USB.
///             The class sends to / receives from the usb device packets of the following format:
///             |Byte no| Description                                   |
///             |   0   | Bits 7 ... 4 : version = 0                    |
///             |       | Bit 3 : 1 if more packets, 0 if last packet   |
///             |       | Bits 2 ... 0 : reserved, must be 0            |
///             |   1   | Packet no                                     |
///             | 2...3 | Message ID                                    |
///             | 4..63 | Data                                          |
///
///             Sent data is automatically fragmented
///             Received data is built from the received fragments and sent to the user
///////////////////////////////////////////////////////////////////////////////
class CFragmentedUSBLink : public CUSBLink
{
private:

    static const int FRAGMENT_SIZE = 64; // 60 data + 4 header
    static const int FRAGMENT_DATA_SIZE = 60; // 60 data + 4 header

    struct USBFragment
    {
    public:
        USBFragment() : reserved(0), notLastFragment(0), version(0), pktNo(0), msgId(0)
        {
        }

    private:
        uint8_t reserved : 3;

    public:
        uint8_t notLastFragment : 1;
        uint8_t version : 4;

    public:
        uint8_t pktNo;
        uint16_t msgId;
        uint8_t data[FRAGMENT_DATA_SIZE];
    }__attribute__((packed));

private:
    uint16_t m_nMsgId;

public:
    CFragmentedUSBLink(bool p_bRawLog = true) : CUSBLink(p_bRawLog), m_nMsgId(0) {}

    int WriteBulk(const unsigned char* p_pData, uint16_t p_nLegth);
    int ReadBulk(unsigned char* p_pData, uint16_t p_nLegth, unsigned int p_nTimeout = 10);

private:
    bool putFragment(const USBFragment& p_rFragment);
    bool getFragment(USBFragment& p_rFragment, unsigned int p_nTimeout = 10);
};



/// @}
#endif //__USB_LINK_H
