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
                          CUSBLink.cpp  -  description
                             -------------------
    begin                : Mon Sept 13 2010
    email                : nicolae.bodislav@nivis.com
 ***************************************************************************/
///////////////////////////////////////////////////////////////////////////////
///@author      Nicolae Bodislav
///@date        13.09.2010
///@brief       Definition of the CUSBLink class for communication with a device on USB
///////////////////////////////////////////////////////////////////////////////

#include <Shared/LogDefs.h>
#include "USBLink.h"

///////////////////////////////////////////////////////////////////////////////
///@author      Nicolae Bodislav
///@brief       Constructor for CUSBLink
///@param       none
///@retval      none
///////////////////////////////////////////////////////////////////////////////
CUSBLink::CUSBLink(bool p_bRawLog /*= true*/)
{
    m_bRawLog = p_bRawLog;
    m_pDevh = NULL;
    m_pDev = NULL;
    m_pData_transfer = NULL;
    m_nCb = 0;
    m_bHaveData = false;
    m_nTrasferedLength = 0;
    m_cWriteEndpoint = DEFAULT_WRITE_ENDPOINT;//default value
    m_cReadEndpoint = DEFAULT_READ_ENDPOINT;//default value

    int r = 1;
    r = libusb_init(&m_pContex);
    if (r < 0) {
        NLOG_ERR("[CUSBLink] Failed to initialise libusb!");
    }
    m_enumDealoc = LIBUSB;
}

///////////////////////////////////////////////////////////////////////////////
///@author      Nicolae Bodislav
///@brief       Destructor for CUSBLink
///@param       none
///@retval      none
///////////////////////////////////////////////////////////////////////////////
CUSBLink::~CUSBLink()
{
    switch(m_enumDealoc)
    {
    case TRANSFER:
        libusb_free_transfer(m_pData_transfer);
    case INTERFACE:
        libusb_release_interface(m_pDevh, 0);
    case OPEN:
        libusb_close(m_pDevh);
    case LIBUSB:
        libusb_exit(m_pContex);
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
///@author      Nicolae Bodislav
///@brief       Static function that returns a list of the devices connected on USB.
///@param       none
///@retval      A list of USBDeviceDescriptor.
///////////////////////////////////////////////////////////////////////////////
std::list<CUSBLink::USBDeviceDescriptor> CUSBLink::DevicesList()
{
    std::list<CUSBLink::USBDeviceDescriptor> devices;
    CUSBLink::USBDeviceDescriptor devDesc;

    int r;
    int cnt;

    libusb_context *pContex;
    r = libusb_init(&pContex);
    if (r < 0) {
        NLOG_ERR("[CUSBLink][DevicesList] Failed to initialise libusb!");
        return devices;
    }

    libusb_device **devs;

    cnt = libusb_get_device_list(pContex, &devs);

    if (cnt < 0)
    {
        NLOG_ERR("[CUSBLink][DevicesList] Failed to get USB Devices!");
        libusb_exit(pContex);
        return devices;
    }

    struct libusb_device *dev;
    int i = 0;

    while ((dev = devs[i++]) != NULL)
    {
        struct libusb_device_descriptor desc;

        r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0)
        {
            NLOG_ERR("[CUSBLink][DevicesList] Failed to get device descriptor");
            continue;
        }

        devDesc.m_nVendorID = desc.idVendor;
        devDesc.m_nProductID = desc.idProduct;
        devDesc.m_nBusNumber = libusb_get_bus_number(dev);
        devDesc.m_nDeviceAddress = libusb_get_device_address(dev);
        devices.push_back(devDesc);
    }

    libusb_free_device_list(devs, 1);

    libusb_exit(pContex);

    return devices;
}

///////////////////////////////////////////////////////////////////////////////
///@author      Nicolae Bodislav
///@brief       Creates a link with an USB device.
///@param       p_oDevDesc - descriptor for the USB device.
///@retval      0 - Link ok created, < 0 - Error.
///////////////////////////////////////////////////////////////////////////////
int CUSBLink::OpenLink(CUSBLink::USBDeviceDescriptor p_oDevDesc)
{
    if(m_pDevh == NULL)
    {
        int r;
        int cnt;

        libusb_device **devs;
        struct libusb_device *found = NULL;
        struct libusb_device *dev;

        cnt = libusb_get_device_list(m_pContex, &devs);
        if (cnt < 0)
        {
            NLOG_ERR("[CUSBLink][OpenLink] Failed to get USB Devices!");
            return -1;
        }
        int i = 0;
        for(i=0; i<cnt; i++)
        {
            dev = devs[i];
            struct libusb_device_descriptor desc;
            r = libusb_get_device_descriptor(dev, &desc);
            if (r < 0)
                return -1;
            if (desc.idVendor == p_oDevDesc.m_nVendorID && desc.idProduct == p_oDevDesc.m_nProductID && (libusb_get_bus_number(dev) == p_oDevDesc.m_nBusNumber) && (libusb_get_device_address(dev) == p_oDevDesc.m_nDeviceAddress))
            {
                found = dev;
                break;
            }
        }

        if (found)
        {
            r = libusb_open(found, &m_pDevh);
            m_pDev = found;
            if (r < 0)
            {
                NLOG_ERR("[CUSBLink][OpenLink] Failed to open link");
                m_pDevh = NULL;
            }
        }
        libusb_free_device_list(devs, 1);

        if(m_pDevh != NULL)
        {
            m_enumDealoc = CUSBLink::OPEN;
            return 0;
        }
        else
            return -EIO;
    }
    else
    {
        NLOG_ERR("[CUSBLink][OpenLink] Device already open.");
        return -1;
    }
}

///////////////////////////////////////////////////////////////////////////////
///@author      Nicolae Bodislav
///@brief       Callback called internally when the read is done or the timeout expires.
///@param       transfer - struct libusb_transfer that contains data from the transfer.
///@retval      none.
///////////////////////////////////////////////////////////////////////////////
void CUSBLink::Cb_TransferRead(struct libusb_transfer *transfer)
{
    CUSBLink* c = (CUSBLink*)transfer->user_data;

    NLOG_DBG("[CUSBLink][Cb_TransferRead] Data transfer status %d\n", transfer->status);

    if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
        if(transfer->status == LIBUSB_TRANSFER_TIMED_OUT)
        {   c->Transfer(false, 0);//if timed out then no data
        }
        else
        {   c->Transfer(false, -1);//an error occurred
        }
        return;
    }
    c->Transfer(true, transfer->actual_length);
}

///////////////////////////////////////////////////////////////////////////////
///@author      Nicolae Bodislav
///@brief       Callback called internally when the write is done or the timeout expires.
///@param       transfer - struct libusb_transfer that contains data from the transfer.
///@retval      none.
///////////////////////////////////////////////////////////////////////////////
void CUSBLink::Cb_TransferWrite(struct libusb_transfer *transfer)
{
    CUSBLink* c = (CUSBLink*)transfer->user_data;
    NLOG_DBG("[CUSBLink][Cb_TransferWrite] Data transfer status %d\n", transfer->status);
    if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
    {
        c->Transfer(false, 0);
        NLOG_ERR("[CUSBLink][Cb_TransferWrite] Transfer not completed");
    }
    c->Transfer(false, transfer->actual_length);
}


///////////////////////////////////////////////////////////////////////////////
///@author      Nicolae Bodislav
///@brief       Function called internally to set variables if the transfer ended ok or with error.
///@param       p_bHaveData - shows if data was readed from the device.
///@param       p_nTransferedLength - the length of the data readed.
///@retval      none.
///////////////////////////////////////////////////////////////////////////////
void CUSBLink::Transfer(bool p_bHaveData, int p_nTransferedLength)
{
    m_bHaveData = p_bHaveData;
    if(!m_bHaveData)
    {
        libusb_free_transfer(m_pData_transfer);
        m_pData_transfer = NULL;
    }
    m_nTrasferedLength = p_nTransferedLength;
    m_nCb = 1;
}

///////////////////////////////////////////////////////////////////////////////
///@author      Nicolae Bodislav
///@brief       Reads data from the USB device on bulk.
///@param       [out]p_pData - Buffer to store output data
///@param       [in]p_nLegth - Size of the output buffer
///@param       [in]p_nTimeout - How much to wait for data before abandoning the read operation
///@retval      size of the data written in the output buffer or < 0 for error
///////////////////////////////////////////////////////////////////////////////
int CUSBLink::ReadBulk(unsigned char* p_pData, uint16_t p_nLegth, unsigned int p_nTimeout /*= 10*/ )
{
    m_nCb = 0;
    m_nTrasferedLength = 0;

    if(m_pDevh == NULL)
    {
        NLOG_ERR("[CUSBLink][ReadBulk] Device not open.");
        return -1;
    }

    int rez = libusb_claim_interface(m_pDevh, 0);
    if (rez < 0)
    {
        NLOG_ERR("[CUSBLink][ReadBulk] Couldn't claim interface.");
        return -1;
    }

    m_enumDealoc = CUSBLink::INTERFACE;

    m_pData_transfer = NULL;
    m_pData_transfer = libusb_alloc_transfer(0);
    if (!m_pData_transfer)
    {
        NLOG_ERR("[CUSBLink][ReadBulk] Transfer couldn't be allocated.");
        return -ENOMEM;
    }

    m_pDev = libusb_get_device(m_pDevh);
    int max_packet_size = libusb_get_max_packet_size(m_pDev, m_cReadEndpoint);

    libusb_fill_bulk_transfer(m_pData_transfer, m_pDevh, m_cReadEndpoint, m_pData, max_packet_size, Cb_TransferRead, this, p_nTimeout);

    m_enumDealoc = CUSBLink::TRANSFER;

    rez = libusb_submit_transfer(m_pData_transfer);

    if (rez < 0)
    {
        NLOG_ERR("[CUSBLink][ReadBulk] Transfer couldn't be submitted.");

        libusb_cancel_transfer(m_pData_transfer);
        m_enumDealoc = CUSBLink::INTERFACE;

        while (!m_nCb)
        {   rez = libusb_handle_events(m_pContex);
            if (rez < 0)
            {   break;
            }
        }

        libusb_free_transfer(m_pData_transfer);

        return -1;
    }

    while (!m_nCb)
    {   rez = libusb_handle_events(m_pContex);
        if (rez < 0)
        {   libusb_free_transfer(m_pData_transfer);
            return -1;
        }
    }


    if(m_bHaveData)
    {
        if(m_nTrasferedLength > 0)
        {   memcpy(p_pData, m_pData, m_nTrasferedLength);
            if (m_bRawLog)
            {   NLOG_DBG_HEX("[CUSBLink][ReadBulk] Received packet:", m_pData, m_nTrasferedLength);
            }
        }
        else
            p_pData = NULL;
    }

    return m_nTrasferedLength;
}

///////////////////////////////////////////////////////////////////////////////
///@author      Nicolae Bodislav
///@brief       Writes data from the USB device on bulk.
///@param       [in]p_pData - the buffer that contains the data to write
///@param       [in]p_nLength - size of the data contained in the buffer
///@retval      how many bytes were actually written; or < 0 for error
///////////////////////////////////////////////////////////////////////////////
int CUSBLink::WriteBulk(const unsigned char* p_pData, uint16_t p_nLength)
{
    m_nCb = 0;
    m_nTrasferedLength = 0;
    if(p_nLength > MAX_LENGTH)
    {
         NLOG_ERR("[CUSBLink][WriteBulk] To much data to write. Maz is %d", MAX_LENGTH);
         return -1;
    }

    if(m_pDevh == NULL)
    {
        NLOG_ERR("[CUSBLink][WriteBulk] Device not open.");
        return -1;
    }

    int rez = libusb_claim_interface(m_pDevh, 0);
    if (rez < 0)
    {
        NLOG_ERR("[CUSBLink][WriteBulk] USB_claim_interface error %d\n", rez);
        return -2;
    }

    m_enumDealoc = CUSBLink::INTERFACE;

    m_pData_transfer = NULL;
    m_pData_transfer = libusb_alloc_transfer(0);
    if (!m_pData_transfer)
    {
        NLOG_ERR("[CUSBLink][WriteBulk] Transfer couldn't be allocated.");
        return -ENOMEM;
    }

    libusb_fill_bulk_transfer(m_pData_transfer, m_pDevh, m_cWriteEndpoint, (unsigned char*)p_pData, p_nLength, Cb_TransferRead, this, 1000);

    m_enumDealoc = CUSBLink::TRANSFER;


    rez = libusb_submit_transfer(m_pData_transfer);

    if (rez < 0)
    {
        NLOG_ERR("[CUSBLink][WriteBulk] Transfer couldn't be submitted.");

        libusb_cancel_transfer(m_pData_transfer);
        m_enumDealoc = CUSBLink::INTERFACE;

        while (!m_nCb)
        {   rez = libusb_handle_events(m_pContex);
            if (rez < 0)
            {   break;
            }
        }

        libusb_free_transfer(m_pData_transfer);

        return -1;
    }

    while (!m_nCb)
    {
        rez = libusb_handle_events(m_pContex);
        if (rez < 0)
        {
            libusb_free_transfer(m_pData_transfer);
            return -1;
        }
    }

    if (m_bRawLog)
    {   NLOG_DBG_HEX("[CUSBLink][WriteBulk] Wrote packet:", p_pData, m_nTrasferedLength);
    }

    return m_nTrasferedLength;
}

///////////////////////////////////////////////////////////////////////////////
///@author      Gabriel Ghervase
///@brief       Writes a fragment
///@param       [in]p_rFragment - the fragment to write
///@retval      true if fragment was written; false otherwise
///////////////////////////////////////////////////////////////////////////////
bool CFragmentedUSBLink::putFragment(const USBFragment& p_rFragment)
{
    int retval = CUSBLink::WriteBulk((unsigned char*)&p_rFragment, sizeof(USBFragment));
    if ( retval < 0 )
    {
        NLOG_ERR("[CFragmentedUSBLink][WriteBulk] Error writing - Abort");
        return false;
    }

    if (retval == 0)
    {
        NLOG_WARN("[CFragmentedUSBLink][WriteBulk] wrote 0 bytes out of %d", sizeof(USBFragment));
        return false ; //no error ; nothing tor read
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///@author      Gabriel Ghervase
///@brief       Reads a fragment from the USB device
///@param       [out]p_rFragment - the fragment where to write the received data
///@param       [in]p_nTimeout - timeout
///@retval      true - got fragment ; false otherwise
///////////////////////////////////////////////////////////////////////////////
bool CFragmentedUSBLink::getFragment(USBFragment& p_rFragment, unsigned int p_nTimeout/* = 10*/)
{
    int retval = CUSBLink::ReadBulk((unsigned char*)&p_rFragment, sizeof(USBFragment), p_nTimeout);

    if ( retval < 0)
    {
        NLOG_ERR("[CFragmentedUSBLink][ReadBulk] Error reading - Abort");
        return false;
    }

    if (retval == 0)
    {
        NLOG_WARN("[CFragmentedUSBLink][ReadBulk] Nothing to read");
        return false ; //no error ; nothing tor read
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
///@author      Gabriel Ghervase
///@brief       Writes data from the USB device in a fragmented manner (data is split and sent as individual fragments)
///@param       [in]p_pData - the buffer that contains the data to write
///@param       [in]p_nLength - size of the data contained in the buffer
///@retval      how many bytes were actually written; or < 0 for error
///////////////////////////////////////////////////////////////////////////////
int CFragmentedUSBLink::WriteBulk(const unsigned char* p_pData, uint16_t p_nLegth)
{
    if (m_bRawLog)
    {   NLOG_DBG_HEX("[CFragmentedUSBLink][WriteBulk] Message to fragment:", p_pData, p_nLegth);
    }

    USBFragment ofragment;
    int fragsNb = (p_nLegth / FRAGMENT_DATA_SIZE) + ((p_nLegth % FRAGMENT_DATA_SIZE != 0) ? 1 : 0);

    ofragment.msgId = m_nMsgId++;

    for (int i = 0 ; i < fragsNb; i++)
    {
        ofragment.notLastFragment = (i != fragsNb - 1);
        ofragment.pktNo = (uint8_t)i;
        memcpy( ofragment.data,
                p_pData + i * FRAGMENT_DATA_SIZE,
                ofragment.notLastFragment ? FRAGMENT_DATA_SIZE : p_nLegth - i * FRAGMENT_DATA_SIZE );

        if ( !putFragment(ofragment) )
        {
            NLOG_ERR("[CFragmentedUSBLink][WriteBulk] Error sending packet %d of %d - abort sending", i, fragsNb);
            return 0;
        }
    }

    return p_nLegth;
}

///////////////////////////////////////////////////////////////////////////////
///@author      Ghervase Gabriel
///@brief       Reads fragments from the USB device and reconstructs the final message from the fragments
///@param       [out]p_pData - Buffer to store output data
///@param       [in]p_nLegth - Size of the output buffer
///@param       [in]p_nTimeout - How much to wait for data before abandoning the read operation
///@retval      size of the data written in the output buffer or < 0 for error
///////////////////////////////////////////////////////////////////////////////
int CFragmentedUSBLink::ReadBulk(unsigned char* p_pData, uint16_t p_nLegth, unsigned int p_nTimeout/* = 10*/)
{
    USBFragment ofragment;

    uint8_t expectedPktNo = 0;
    int expectedMessageId = -1; //uninitialized
    int bytesReceivedSoFar = 0;

    while (getFragment(ofragment, p_nTimeout))
    {
        //check messageId - if unexpected, drop previously read data and restart with new messageID
        if (expectedMessageId != ofragment.msgId)
        {
            expectedMessageId = ofragment.msgId;
            //dump previously received data (if any)
            expectedPktNo = 0;
            bytesReceivedSoFar = 0;
        }

        // ignore - duplicate packet received
        if (ofragment.pktNo < expectedPktNo)
        {   continue;
        }

        // packets should arrive in order ; abort receiving if not
        if (ofragment.pktNo > expectedPktNo)
        {
            NLOG_ERR("[CFragmentedUSBLink][ReadBulk] Bad pktNo received : %d > expected %d - Abort reading ",
                (int)(ofragment.pktNo), (int)(expectedPktNo));
            break;
        }


        if (bytesReceivedSoFar + FRAGMENT_DATA_SIZE > p_nLegth)
        {   NLOG_ERR("[CFragmentedUSBLink][ReadBulk] Buffer too small; is %d and we need %d - Abort reading",
                (int)p_nLegth, (int)(bytesReceivedSoFar + FRAGMENT_DATA_SIZE));
            break;
        }

        memcpy(p_pData + bytesReceivedSoFar, ofragment.data, FRAGMENT_DATA_SIZE);

        bytesReceivedSoFar += FRAGMENT_DATA_SIZE;
        expectedPktNo++;

        if (ofragment.notLastFragment)
        {   continue;
        }

        return bytesReceivedSoFar;
    }

    //error - abort reading
    return -1;
}


