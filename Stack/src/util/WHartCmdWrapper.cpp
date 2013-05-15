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

#include <WHartStack/util/WHartCmdWrapper.h>

#include <WHartStack/WHartSubApplicationData.h>
#include <WHartStack/util/Binarization.h>
#include <Shared/UtilsSolo.h>
#include <WHartStack/CommInterfaceOverIP/MessageIPSerializer.h>
#include <iostream>
#include <iomanip>
#include <nlib/detail/bytes_print.h>

namespace hart7 {
namespace stack {

const ParseExecuteComposerEntry* CHartCmdWrapper::s_pParseReqComposeResp = NULL;
const ParseExecuteComposerEntry* CHartCmdWrapper::s_pComposeReqParseResp = NULL;

int CHartCmdWrapper::s_nComposeReqParseRespSize = 0;
int CHartCmdWrapper::s_nParseReqComposeRespSize = 0;

uint8_t CHartCmdWrapper::computeCheckByte(const uint8_t* p_pData, int len)
{
    uint8_t chkByte = 0;

    for (int i = 0; i < len; i++)
    {
        chkByte ^= p_pData[i];
    }

    return chkByte;
}

CHartCmdWrapper::CHartCmdWrapper() :
    m_isFromAppHostClient(false),
    m_nFormat(transport::Packet::miHartDirectPDU)
{
    m_pParsedData = NULL;
    m_pRawData = NULL;
    m_pMetaCmdsInfo = NULL;
    ReleaseAll();
}

CHartCmdWrapper::~CHartCmdWrapper()
{
    ReleaseAll();
}

void CHartCmdWrapper::ReleaseAll()
{
    if (m_pRawData)
    {
        delete m_pRawData;
        m_pRawData = NULL;
    }
    if (m_pParsedData)
    {
        delete m_pParsedData;
        m_pParsedData = NULL;
    }

    if (m_pMetaCmdsInfo)
    {
        delete m_pMetaCmdsInfo;
        m_pMetaCmdsInfo = NULL;
    }

    m_u16CmdId = g_sCmdIdInvalid;
    m_i16ResponseCode = g_sRspCodeNotRsp;

    m_u8DeviceStatus = 0;

    m_bHasRaw = false;
    m_nRawLen = 0;

    m_bHasParsed = false;
    m_nParseLen = 0;

    m_nDelimiter = 0;
    m_nPolingAddress = 0;
}

//return header length
//	<= 0 error
int CHartCmdWrapper::ExtractDirectCommonHeader(bool p_bIsResponse, BinaryStream * p_pStream, uint16_t * p_pu16CmdId,
                                               int * p_pnLen, uint8_t & errorCodeResponse, int16_t * p_pi16ResponseCode, uint8_t * p_pu8DeviceStatus /*= NULL*/)
{
    // Command     ByteCount       Data --------------------------------------------------------------------------------------------------->
    // Cmd = 31    ByteCount       Bit 7 = 0 then Response Code        FieldDeviceStatus       ExtendedCommandNumber       ResponseDataBytes
    // Network Management Specification - spec085r1.1-depr.pdf : pag 51/100

    int nHeaderLen = 2 + 1; //cmdID + nLen

    if (p_bIsResponse)
    {
        nHeaderLen++; //response code
        if (p_pu8DeviceStatus)
        {
            nHeaderLen++;
        }
    }

    if (p_pStream->remainingBytes < nHeaderLen)
    {
        errorCodeResponse = RCS_E05_TooFewDataBytesReceived;
        LOG_WARN("CHartCmdWrapper::ExtractDirectCommonHeader: command too small; max=" << p_pStream->remainingBytes);
        return -1;
    }

    //parse Wireless Application Command Header
    uint16_t u16CmdId;
    STREAM_READ_UINT16(p_pStream, &u16CmdId);

    uint8_t dataLength;
    STREAM_READ_UINT8(p_pStream, &dataLength);

    if (dataLength > p_pStream->remainingBytes)
    {
        errorCodeResponse = RCS_E05_TooFewDataBytesReceived;
        LOG_WARN("CHartCmdWrapper::ExtractDirectCommonHeader: cmdId=" << (int) u16CmdId << " dataLength=" << (int) dataLength
                    << " > remainingBytes=" << (int) p_pStream->remainingBytes);
        return -1;
    }

    if ((!p_bIsResponse) && IsBurstPublishAsRequest(u16CmdId, dataLength))
    {
        p_bIsResponse = true;
    }

    uint8_t u8RspCode;
    if (p_bIsResponse)
    {
        STREAM_READ_UINT8(p_pStream, &u8RspCode);
        dataLength--;
        *p_pi16ResponseCode = u8RspCode;
        if (p_pu8DeviceStatus)
        {
            STREAM_READ_UINT8(p_pStream, p_pu8DeviceStatus);
            dataLength--;
        }
    }
    else
    {
        *p_pi16ResponseCode = g_sRspCodeNotRsp;
    }
    *p_pu16CmdId = u16CmdId;
    *p_pnLen = dataLength;

    return nHeaderLen;
}

//return header length
//	<= 0 error
int CHartCmdWrapper::ExtractWiredCommonHeader(bool p_bIsResponse, BinaryStream * p_pStream, uint16_t * p_pu16CmdId,
                                              int * p_pnLen, uint8_t & errorCodeResponse, int16_t * p_pi16ResponseCode,
                                              uint8_t * p_u8DeviceStatus/* = NULL */, bool p_bEmbeddedCommandHeader /*= false*/)
{
    //minimum header size; can be bigger if we have an extended command
    int nHeaderLen = 1 + 1; //cmdID + nLen
    if (p_bIsResponse)
    {
        nHeaderLen++; //response code
        if (p_u8DeviceStatus != 0)
        {
            nHeaderLen++;
        }
    }

    if (p_pStream->remainingBytes < nHeaderLen)
    {
        errorCodeResponse = RCS_E05_TooFewDataBytesReceived;
        LOG_WARN("CHartCmdWrapper::ExtractWiredCommonHeader: command too small (remainingBytes < headerLen); remainingBytes=" << p_pStream->remainingBytes
                 << ", headerLen=" << nHeaderLen);
        return -1;
    }

    uint16_t u16CmdId;
    STREAM_READ_UINT8(p_pStream, &u16CmdId);

    uint8_t dataLength;
    STREAM_READ_UINT8(p_pStream, &dataLength);

    if (dataLength > p_pStream->remainingBytes)
    {
        errorCodeResponse = RCS_E05_TooFewDataBytesReceived;
        LOG_INFO("CHartCmdWrapper::ExtractWiredCommonHeader: cmdId=" << (int) u16CmdId << " len=" << dataLength
                    << " > max=" << p_pStream->remainingBytes);
        return -1;
    }

    LOG_DEBUG_APP("isFromAppHostClient = " << m_isFromAppHostClient);
    if (!m_isFromAppHostClient)
    {
        // support for Device X
        if ((!p_bIsResponse) && IsBurstPublishAsRequest(u16CmdId, dataLength))
        {
            p_bIsResponse = true;
        }
    }

    uint8_t u8RspCode;
    if (p_bIsResponse)
    {
        STREAM_READ_UINT8(p_pStream, &u8RspCode);
        dataLength--;
        *p_pi16ResponseCode = u8RspCode;
        if (p_u8DeviceStatus != 0)
        {
            STREAM_READ_UINT8(p_pStream, p_u8DeviceStatus);
            dataLength--;
        }
    }
    else
    {
        *p_pi16ResponseCode = g_sRspCodeNotRsp;
    }

    // check for extended command
    if (u16CmdId == 0x1F)
    {
        if (p_pStream->remainingBytes > sizeof(u16CmdId)) // last byte no matter - check byte
        {
            STREAM_READ_UINT16(p_pStream, &u16CmdId);
            dataLength -= 2;
            nHeaderLen += 2;
        }
        else
        {
            LOG_ERROR("CHartCmdWrapper::ExtractWiredCommonHeader: extended command - too few bytes received - cmdId="
                        << (int) u16CmdId << ", required=" << sizeof(u16CmdId) + 1 << ", remaining bytes="
                        << (int) p_pStream->remainingBytes);

            if (p_pStream->remainingBytes > 1)
            {
                dataLength -= p_pStream->remainingBytes - 1;
                nHeaderLen += p_pStream->remainingBytes - 1;
            }
        }
    }

    *p_pu16CmdId = u16CmdId;
    *p_pnLen = dataLength;

    return nHeaderLen;
}

int CHartCmdWrapper::WriteWiredCommonHeader(bool p_bIsResponse, BinaryStream & p_rStream, uint16_t p_u16CmdId,
                                            int p_nLen, int16_t p_i16ResponseCode, uint8_t * p_pu8DeviceStatus /*=NULL*/)
{
    int byteCount = p_nLen;

    int writtenBytes = 0;

    //write command or extended command marker
    if (p_u16CmdId > 0xFF)
    {
        STREAM_WRITE_UINT8(&p_rStream, 0x1F);
        byteCount += 2;
        writtenBytes += 2;
    }
    else
    {
        STREAM_WRITE_UINT8(&p_rStream, static_cast<uint8_t> (p_u16CmdId));
        writtenBytes++;
    }

    if (p_bIsResponse)
    {
        STREAM_WRITE_UINT8(&p_rStream, static_cast<uint8_t> (byteCount + ((p_pu8DeviceStatus > 0) ? 2 : 1)));
        STREAM_WRITE_UINT8(&p_rStream, p_i16ResponseCode);
        writtenBytes += 2;
        if (p_pu8DeviceStatus != 0)
        {
            STREAM_WRITE_UINT8(&p_rStream, *p_pu8DeviceStatus);
            writtenBytes++;
        }
    }
    else
    {
        STREAM_WRITE_UINT8(&p_rStream, byteCount);
        writtenBytes++;
    }

    //write extended command
    if (p_u16CmdId > 0xFF)
    {
        STREAM_WRITE_UINT16(&p_rStream, p_u16CmdId);
        writtenBytes += 2;
    }

    return writtenBytes;
}

int CHartCmdWrapper::WriteDirectCommonHeader(bool p_bIsResponse, BinaryStream& p_rStream, uint16_t p_u16CmdId,
                                             int p_nLen, int16_t p_i16ResponseCode, uint8_t* p_pu8DeviceStatus /*=NULL*/)
{
    int writtenBytes = 0;

    STREAM_WRITE_UINT16(&p_rStream, p_u16CmdId);
    STREAM_WRITE_UINT8(&p_rStream, p_nLen);
    writtenBytes += 3;

    if (p_bIsResponse)
    {
        STREAM_WRITE_UINT8(&p_rStream, p_i16ResponseCode);
        writtenBytes++;
        if (p_pu8DeviceStatus != 0)
        {
            STREAM_WRITE_UINT8(&p_rStream, *p_pu8DeviceStatus);
            writtenBytes++;
        }
    }

    return writtenBytes;
}

int CHartCmdWrapper::LoadMetaCmd77Header(bool p_bIsResponse, BinaryStream & stream, uint8_t & errorCodeResponse)
{
    if (!m_pMetaCmdsInfo)
    {
        m_pMetaCmdsInfo = new TMetaCmdsInfo;
    }

    //save to Meta command space
    m_pMetaCmdsInfo->m_u16MetaCmdId = m_u16CmdId;
    m_pMetaCmdsInfo->m_i16MetaCmdResponseCode = m_i16ResponseCode;
    m_pMetaCmdsInfo->m_u8MetaCmdDeviceStatus = m_u8DeviceStatus;

    int requiredBytes = 1 //m_nCard
                         + 1 // m_nChannel
                         + 1 // m_nDelimiter
                         + 1; // min(m_nDevicePollingAddress or m_oInnerCmdUniqueId)
    if (!p_bIsResponse)
    {
        requiredBytes++; // m_nTransmitPreambleCount
    }

    if (stream.remainingBytes < requiredBytes)
    {
        errorCodeResponse = RCS_E05_TooFewDataBytesReceived;
        LOG_WARN("CHartCmdWrapper::LoadMetaCmd77Header: command too small (remainingBytes < requiredBytes); remainingBytes=" << stream.remainingBytes << ", requiredBytes=" << requiredBytes);
        return -1;
    }

//    if (p_pStream->remainingBytes < requiredBytes)
//    {
//        LOG_WARN("CHartCmdWrapper::ExtractWiredCommonHeader: command too small (remainingBytes < requiredBytes); remainingBytes=" << p_pStream->remainingBytes
//                 << ", requiredBytes=" << requiredBytes);
//        return -1;
//    }

    STREAM_READ_UINT8(&stream, &(m_pMetaCmdsInfo->m_oCmdSubDevice.m_nCard));
    STREAM_READ_UINT8(&stream, &(m_pMetaCmdsInfo->m_oCmdSubDevice.m_nChannel));

    if (!p_bIsResponse)
    {
        STREAM_READ_UINT8(&stream, &(m_pMetaCmdsInfo->m_oCmdSubDevice.m_nTransmitPreambleCount));
    }

    STREAM_READ_UINT8(&stream, &(m_pMetaCmdsInfo->m_oCmdSubDevice.m_nDelimiter));

    // LOG_DEBUG("LoadMetaCmd77Header - Delimiter=" << (int)m_pMetaCmdsInfo->m_oCmdSubDevice.m_nDelimiter);
    //bit 7 from delimiter tells address size

    if (((m_pMetaCmdsInfo->m_oCmdSubDevice.m_nDelimiter & HART_DELIM_ADDR_TYPE_MASK) == HART_DELIM_ADDR_TYPE_SHORT))
    {
        // 1 byte polling addr
        STREAM_READ_UINT8(&stream, &(m_pMetaCmdsInfo->m_oCmdSubDevice.m_nDevicePollingAddress));
    }
    else
    {
        requiredBytes = 5;
        if (stream.remainingBytes < requiredBytes)
        {
            errorCodeResponse = RCS_E05_TooFewDataBytesReceived;
            LOG_WARN("CHartCmdWrapper::ExtractWiredCommonHeader: command too small (remainingBytes < requiredBytes); remainingBytes=" << stream.remainingBytes << ", requiredBytes=" << requiredBytes);
            return -1;
        }

        // 5 bytes addr
        STREAM_READ_BYTES(&stream, &m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes, sizeof(m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes));
        // LOG_DEBUG("LoadMetaCmd77Header m_oInnerCmdUniqueId=" << m_pMetaCmdsInfo->m_oInnerCmdUniqueId);
    }

    if (ExtractWiredCommonHeader(p_bIsResponse, &stream, &m_u16CmdId, &m_nRawLen, errorCodeResponse, &m_i16ResponseCode, &m_u8DeviceStatus, true) < 0)
    {
        ReleaseAll();
        return -1;
    }

    return 0;
}

bool CHartCmdWrapper::IsBurstPublishAsRequest(uint16_t p_nCmdId, int p_nLen)
{
    switch (p_nCmdId)
    {
        case CMDID_C001_ReadPrimaryVariable:
            if (p_nLen > C001_ReqSize && p_nLen >= C001_RespSize)
            {
                return true;
            }
        break;
        case CMDID_C002_ReadLoopCurrentAndPercentOfRange:
            if (p_nLen > C002_ReqSize && p_nLen >= C002_RespSize)
            {
                return true;
            }
        break;
        case CMDID_C003_ReadDynamicVariablesAndLoopCurrent:
            if (p_nLen > C003_ReqSize && p_nLen >= C003_RespSize)
            {
                return true;
            }
        break;
        case CMDID_C009_ReadDeviceVariablesWithStatus:
            if (p_nLen > C009_ReqSize && p_nLen >= C009_RespSize)
            {
                return true;
            }
        break;
        case CMDID_C033_ReadDeviceVariables:
            if (p_nLen > C033_ReqSize && p_nLen >= C033_RespSize)
            {
                return true;
            }
        break;
            //case CMDID_C119_AcknowledgeEventNotification:
            //case CMDID_C123...
        default:
            return false;
        break;
    }

    return false;
}

// returns the number of bytes consumed
//	< 0 -- error
// note if p_bReadDeviceStatus == false -> is not be read from stream
int CHartCmdWrapper::LoadRawWHart(bool p_bIsResponse, const void * p_pData, int p_nMaxLen, uint8_t & errorCodeResponse, bool p_bReadDeviceStatus)
{
    ReleaseAll();

    uint8_t* pu8DeviceStatus = NULL;

    BinaryStream stream;
    STREAM_INIT(&stream, (const uint8_t*) p_pData, p_nMaxLen);

    if (p_bReadDeviceStatus)
    {
        pu8DeviceStatus = &m_u8DeviceStatus;
    }
    if (ExtractDirectCommonHeader(p_bIsResponse, &stream, &m_u16CmdId, &m_nRawLen, errorCodeResponse, &m_i16ResponseCode, pu8DeviceStatus) < 0)
    {
        LOG_WARN("CHartCmdWrapper::LoadRawWHart - ExtractDirectCommonHeader fail; data=" << nlib::detail::BytesToString((const uint8_t*) p_pData, p_nMaxLen));
        return -1;
    }

    if (p_bIsResponse && IsResponseCodeError(m_i16ResponseCode))
    {
        m_bHasRaw = true;
        m_nRawLen = 0;
        return stream.nextByte - (uint8_t*) p_pData;
    }

    if (m_u16CmdId == CMDID_C64765_NivisMetaCommand)
    {
        int nMetaCmdHeaderLen = 2 + 5; //

        if (stream.remainingBytes < nMetaCmdHeaderLen)
        {
            errorCodeResponse = RCS_E05_TooFewDataBytesReceived;
            LOG_WARN("CHartCmdWrapper::LoadRawWHart: C64765_NivisMetaCommand: too small data=" << nlib::detail::BytesToString((const uint8_t*) p_pData, p_nMaxLen));
            return -1;
        }

        if (!m_pMetaCmdsInfo)
        {
            m_pMetaCmdsInfo = new TMetaCmdsInfo;
        }

        //save to Meta command space
        m_pMetaCmdsInfo->m_u16MetaCmdId = m_u16CmdId;
        m_pMetaCmdsInfo->m_i16MetaCmdResponseCode = m_i16ResponseCode;
        m_pMetaCmdsInfo->m_u8MetaCmdDeviceStatus = m_u8DeviceStatus;

        STREAM_READ_UINT16(&stream, &m_pMetaCmdsInfo->m_oCmdNivisMeta.m_u16ExtDeviceType);
        STREAM_READ_BYTES(&stream, &m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes, sizeof(m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes));

        if (ExtractDirectCommonHeader(p_bIsResponse, &stream, &m_u16CmdId, &m_nRawLen, errorCodeResponse, &m_i16ResponseCode, pu8DeviceStatus) < 0)
        {
            LOG_WARN("CHartCmdWrapper::LoadHartWired - ExtractDirectCommonHeader fail; NivisMetaCommand - data=" << nlib::detail::BytesToString((const uint8_t*) p_pData, p_nMaxLen));

            ReleaseAll();
            return -1;
        }
    }
    else if (m_u16CmdId == CMDID_C077_SendCommandToSubDevice)
    {
        int ret = LoadMetaCmd77Header(p_bIsResponse, stream, errorCodeResponse);
        if (ret < 0)
        {
            return ret;
        }
    }

    if (m_nRawLen > 0)
    {
        m_pRawData = new uint8_t[m_nRawLen]; //m_nRawLen < 256 -> if this throws we are in trouble anyway
        STREAM_READ_BYTES(&stream, m_pRawData, m_nRawLen);
    }

    m_bHasRaw = true;

    return stream.nextByte - (uint8_t*) p_pData;
}

//return the no of bytes of command
// < 0 --error
int CHartCmdWrapper::GetRawWHart(void* p_pData, int p_nMaxLen, bool p_bWriteDeviceStatus)
{
    if (!HasRaw())
    {
        if (GetRawFromParsed() != RCS_N00_Success)
        {
            LOG_WARN("CHartCmdWrapper::GetRawWHart: no raw data");
            return -1;
        }
    }

    int nExtra = (IsResponse() ? 1 : 0) + (p_bWriteDeviceStatus ? 1 : 0);
    int nHeaderLen = 2 + 1 + nExtra; //cmdID + nLen //+ response
    int nMetaCmdHeader = 0;

    if (m_pMetaCmdsInfo)
    {
        if (m_pMetaCmdsInfo->m_u16MetaCmdId == CMDID_C64765_NivisMetaCommand)
        {
            nMetaCmdHeader = 2 + 5 + 2 + 1 + nExtra; // ext_dev_type(2) + uniqueId(5) + cmdId(2) + len(1) //+ rsp
        }
        else if (m_pMetaCmdsInfo->m_u16MetaCmdId == CMDID_C077_SendCommandToSubDevice)
        {
            // card + channel + preamble + delimiter + address + cmdId(1) + len(1) //+ rsp +
            nMetaCmdHeader = 1 + 1 + 1 + 1 + ((m_pMetaCmdsInfo->m_oCmdSubDevice.m_nDevicePollingAddress != 0) ? 1 : 5)
                        + 1 + 1 + nExtra;
        }
    }

    if (nHeaderLen + m_nRawLen + nMetaCmdHeader > p_nMaxLen)
    {
        LOG_WARN("CHartCmdWrapper::GetRawWHart: buffer supplied too small");
        return -1;
    }

    BinaryStream stream;
    STREAM_INIT(&stream, (const uint8_t*) p_pData, p_nMaxLen);

    if (m_pMetaCmdsInfo)
    {
        WriteDirectCommonHeader(IsResponse(), stream, m_pMetaCmdsInfo->m_u16MetaCmdId, m_nRawLen + nMetaCmdHeader,
                                m_pMetaCmdsInfo->m_i16MetaCmdResponseCode,
                                p_bWriteDeviceStatus ? &(m_pMetaCmdsInfo->m_u8MetaCmdDeviceStatus) : 0);

        switch (m_pMetaCmdsInfo->m_u16MetaCmdId)
        {
            case CMDID_C64765_NivisMetaCommand:
                STREAM_WRITE_UINT16(&stream, m_pMetaCmdsInfo->m_oCmdNivisMeta.m_u16ExtDeviceType);
                STREAM_WRITE_BYTES(&stream, &m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes,
                                   sizeof(m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes));
                WriteDirectCommonHeader(IsResponse(), stream, m_u16CmdId, m_nRawLen + nExtra, m_i16ResponseCode,
                                        p_bWriteDeviceStatus ? &m_u8DeviceStatus : 0);
            break;
            case CMDID_C077_SendCommandToSubDevice:
                STREAM_WRITE_UINT8(&stream, m_pMetaCmdsInfo->m_oCmdSubDevice.m_nCard);
                STREAM_WRITE_UINT8(&stream, m_pMetaCmdsInfo->m_oCmdSubDevice.m_nChannel);
                if (!IsResponse())
                {
                    STREAM_WRITE_UINT8(&stream, m_pMetaCmdsInfo->m_oCmdSubDevice.m_nTransmitPreambleCount);
                }

                STREAM_WRITE_UINT8(&stream, m_pMetaCmdsInfo->m_oCmdSubDevice.m_nDelimiter);

                if ((m_pMetaCmdsInfo->m_oCmdSubDevice.m_nDelimiter & HART_DELIM_ADDR_TYPE_MASK)
                            == HART_DELIM_ADDR_TYPE_SHORT)
                {
                    // Polling 1-byte address
                    STREAM_WRITE_UINT8(&stream, m_pMetaCmdsInfo->m_oCmdSubDevice.m_nDevicePollingAddress);
                }
                else
                {
                    // Unique 5-byte address
                    STREAM_WRITE_BYTES(&stream, &m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes,
                                       sizeof(m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes));
                }
                WriteWiredCommonHeader(IsResponse(), stream, m_u16CmdId, m_nRawLen + nExtra, m_i16ResponseCode,
                                       p_bWriteDeviceStatus ? &m_u8DeviceStatus : 0);
            break;
            default:
                LOG_ERROR("GetRawWHart: meta cmd unknown cmdId=" << m_pMetaCmdsInfo->m_u16MetaCmdId);
                return -1;
        }
    }
    else
    {
        WriteDirectCommonHeader(IsResponse(), stream, m_u16CmdId, m_nRawLen + nExtra, m_i16ResponseCode,
                                p_bWriteDeviceStatus ? &m_u8DeviceStatus : 0);
    }

    if (m_nRawLen > 0)
    {
        STREAM_WRITE_BYTES(&stream, m_pRawData, m_nRawLen);
    }

    return stream.nextByte - (uint8_t*) p_pData;
}

void CHartCmdWrapper::LoadRaw(uint16_t p_u16CmdId, int p_nLen, void* p_pData, int p_nResponseCode, int p_nDeviceStatus)
{
    ReleaseAll();

    if (p_nLen < 0 || p_nLen > 128)
    {
        LOG_WARN("CHartCmdWrapper::LoadRaw: len too large" << p_nLen);
        return;
    }

    m_nRawLen = p_nLen;
    m_u16CmdId = p_u16CmdId;
    m_i16ResponseCode = p_nResponseCode;

    if (m_nRawLen)
    {
        m_pRawData = new uint8_t[m_nRawLen]; //m_nRawLen < 256 -> if this throws we are in trouble anyway
        memcpy(m_pRawData, (const uint8_t*) p_pData, m_nRawLen);
    }

    m_u8DeviceStatus = p_nDeviceStatus;

    m_bHasRaw = true;
}

void CHartCmdWrapper::LoadParsed(uint16_t p_u16CmdId, int p_nLen, void* p_pData, int p_nResponseCode,
                                 int p_nDeviceStatus)
{
    ReleaseAll();

    if (p_nLen < 0 || p_nLen > 1024)
    {
        LOG_WARN("CHartCmdWrapper::LoadParsed: len too large" << p_nLen);
        return;
    }

    m_nParseLen = p_nLen;
    m_u16CmdId = p_u16CmdId;
    m_i16ResponseCode = p_nResponseCode;

    if (m_nParseLen)
    {
        m_pParsedData = new uint8_t[m_nParseLen]; //m_nParseLen < 1024 -> if this throws we are in trouble anyway
        memcpy(m_pParsedData, (const uint8_t*) p_pData, m_nParseLen);
    }

    m_u8DeviceStatus = p_nDeviceStatus;
    m_bHasParsed = true;
}

void CHartCmdWrapper::ReleaseParsedData()
{
    if (m_pParsedData)
    {
        delete m_pParsedData;
        m_pParsedData = NULL;
    }
    m_nParseLen = 0;
    m_bHasParsed = 0;
}

// returns success if the command can be composed even with the outside set ResponseCode is not success
//		returns an WHart error code for errors generated by this class
uint8_t CHartCmdWrapper::GetRawFromParsed()
{
    if (HasRaw())
    {
        return RCS_N00_Success;
    }

    if (!HasParsed())
    {
        LOG_ERROR("CHartCmdWrapper::GetRawFromParsed: no raw and no parsed data");
        return RCS_E01_Undefined1;
    }

    if (IsResponse() && IsResponseCodeError(m_i16ResponseCode))
    {
        m_bHasRaw = true;
        m_nParseLen = 0;
        m_pParsedData = NULL;
        return RCS_N00_Success;
    }

    const ParseExecuteComposerEntry* pCmdsTable;
    int nTableSize;

    if (!IsResponse())
    {
        if (!s_pComposeReqParseResp)
        {
            LOG_ERROR("CHartCmdWrapper::GetRawFromParsed: no s_pComposeReqParseResp set");
            return RCS_E01_Undefined1;
        }
        pCmdsTable = s_pComposeReqParseResp;
        nTableSize = s_nComposeReqParseRespSize;
    }
    else
    {
        if (!s_pParseReqComposeResp)
        {
            LOG_ERROR("CHartCmdWrapper::GetRawFromParsed: no s_pParseReqComposeResp set");
            return RCS_E01_Undefined1;
        }

        pCmdsTable = s_pParseReqComposeResp;
        nTableSize = s_nParseReqComposeRespSize;
    }

    int tableIndex = FindEntry(pCmdsTable, nTableSize, m_u16CmdId);
    ComposeFunction composer = (tableIndex < 0) ? 0 : pCmdsTable[tableIndex].fnComposer;

    if (!composer)
    {
        LOG_ERROR("CHartCmdWrapper::GetRawFromParsed: no composer function for cmdId=" << m_u16CmdId);
        return RCS_E64_CommandNotImplemented;
    }

    uint8_t tmp[255];
    BinaryStream stream;
    STREAM_INIT(&stream, tmp, sizeof(tmp));

    ComposerContext context;
    context.cmdId = m_u16CmdId;

    uint8_t u8Res = (*composer)(m_pParsedData, &context, &stream);

    if (u8Res != RCS_N00_Success)
    {
        if (IsResponse())
        {
            m_i16ResponseCode = u8Res;
        }
        else
        {
            LOG_WARN("CHartCmdWrapper::GetRawFromParsed: composer error=" << (int) u8Res);
            return u8Res;
        }
    }

    m_nRawLen = stream.nextByte - tmp;

    if (m_nRawLen)
    {
        m_pRawData = new uint8_t[m_nRawLen]; //m_nRawLen < 256 -> if this throws we are in trouble anyway
        memcpy(m_pRawData, (const uint8_t*) tmp, m_nRawLen);
    }

    m_bHasRaw = true;

    return RCS_N00_Success;
}

// returns success if the command can be parsed even with the outside set ResponseCode is not success
//		returns an WHart error code for errors generated by this class
uint8_t CHartCmdWrapper::GetParsedFromRaw()
{
    if (HasParsed())
    {
        LOG_INFO("CHartCmdWrapper::GetParsedFromRaw: Success.");
        return RCS_N00_Success;
    }

    if (!HasRaw())
    {
        LOG_ERROR("CHartCmdWrapper::GetParsedFromRaw: no raw and no parsed data");
        return RCS_E01_Undefined1;
    }

    if (IsResponse() && IsResponseCodeError(m_i16ResponseCode))
    {
        m_bHasParsed = true;
        m_nRawLen = 0;
        m_pRawData = NULL;
        LOG_INFO("CHartCmdWrapper::GetParsedFromRaw: Success.");
        return RCS_N00_Success;
    }

    const ParseExecuteComposerEntry* pCmdsTable;
    int nTableSize;
    if (IsResponse())
    {
        if (!s_pComposeReqParseResp)
        {
            LOG_ERROR("CHartCmdWrapper::GetParsedFromRaw: no s_pComposeReqParseResp set");
            return RCS_E01_Undefined1;
        }
        pCmdsTable = s_pComposeReqParseResp;
        nTableSize = s_nComposeReqParseRespSize;

    }
    else
    {
        if (!s_pParseReqComposeResp)
        {
            LOG_ERROR("CHartCmdWrapper::GetParsedFromRaw: no s_pParseReqComposeResp set");
            return RCS_E01_Undefined1;
        }

        pCmdsTable = s_pParseReqComposeResp;
        nTableSize = s_nParseReqComposeRespSize;
    }

    int tableIndex = FindEntry(pCmdsTable, nTableSize, m_u16CmdId);
    ParseFunction parser = 0;
    GetParsedLenFctType getParsedLen = 0;
    GetBinLenFctType getBinLen = 0;

    if (tableIndex >= 0)
    {
        const ParseExecuteComposerEntry& tableEntry = pCmdsTable[tableIndex];
        parser = tableEntry.fnParser;
        getParsedLen = tableEntry.fnGetParsedLen;
        getBinLen = tableEntry.fnGetBinLen;
        if (parser == 0 || getParsedLen == 0 || getBinLen == 0)
        {
            LOG_ERROR("CHartCmdWrapper::GetParsedFromRaw: parser =" << parser << ", getParsedLen =" << getParsedLen
                        << ", getBinLen =" << getParsedLen);
            return RCS_E01_Undefined1;
        }
    }
    else
    {
        LOG_ERROR("CHartCmdWrapper::GetParsedFromRaw: Cmd not implemented");
        return RCS_E64_CommandNotImplemented;
    }

    uint8_t tmp[512];
    //int k=0;
    BinaryStream stream;
    STREAM_INIT(&stream, (const uint8_t*) m_pRawData, m_nRawLen);

    ParserContext context;

    context.allocatedCommandSize = 512;
    context.maxNeededParsedSize = 512;

    if (stream.remainingBytes < getBinLen())
    {
        LOG_ERROR("CHartCmdWrapper::GetParsedFromRaw: Too few data bytes received. Remaining Bytes=" << (int)stream.remainingBytes << ", Requested Bytes=" << (int)getBinLen());
        return RCS_E05_TooFewDataBytesReceived; //too few bytes
    }

    context.maxNeededParsedSize = getParsedLen();
    uint8_t u8Res = (*parser)(tmp, &context, &stream);
    if (u8Res != RCS_N00_Success)
    {
        LOG_WARN(" -- CHartCmdWrapper::GetParsedFromRaw: parser error=" << (int) u8Res);
        return u8Res;
    }

    m_nParseLen = context.maxNeededParsedSize;

    if (m_nParseLen)
    {
        m_pParsedData = new uint8_t[m_nParseLen]; //m_nRawLen < 256 -> if this throws we are in trouble anyway

        memcpy(m_pParsedData, (const uint8_t*) tmp, m_nParseLen);

    }

    m_bHasParsed = true;

    return RCS_N00_Success;
}

//returns <0 error
int CHartCmdWrapper::LoadHartDirect(bool p_bIsResponse, const void * p_pData, int p_nMaxLen, WHartUniqueID & p_oDeviceUniqueID, uint8_t & errorCodeResponse)
{
    if (p_nMaxLen < 5 + 2 + 1)
    {
        LOG_WARN("CHartCmdWrapper::LoadHartDirect: command too small data=" << nlib::detail::BytesToString((const uint8_t*) p_pData, p_nMaxLen));
        errorCodeResponse = RCS_E05_TooFewDataBytesReceived;
        return -1;
    }

    memcpy(&p_oDeviceUniqueID.bytes, p_pData, sizeof(p_oDeviceUniqueID.bytes));

    int nParsed = LoadRawWHart(p_bIsResponse, (uint8_t*) p_pData + sizeof(p_oDeviceUniqueID.bytes), p_nMaxLen - sizeof(p_oDeviceUniqueID.bytes), errorCodeResponse, true);

    if (nParsed < 0)
    {
        LOG_ERROR("CHartCmdWrapper::LoadHartDirect: header parsing error data="
                  << nlib::detail::BytesToString((const uint8_t*)((const uint8_t*) p_pData + sizeof(p_oDeviceUniqueID.bytes)), p_nMaxLen - sizeof(p_oDeviceUniqueID.bytes)));
        return -1;
    }

    if (m_pMetaCmdsInfo)
    {
        memcpy(m_pMetaCmdsInfo->m_oMetaCmdUniqueId.bytes, p_oDeviceUniqueID.bytes, sizeof(p_oDeviceUniqueID.bytes));
        memcpy(p_oDeviceUniqueID.bytes, m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes, sizeof(p_oDeviceUniqueID.bytes));
    }

    return nParsed + sizeof(p_oDeviceUniqueID.bytes);
}

// return number of bytes read from stream
//			< 0 on error
int CHartCmdWrapper::GetHartDirect(void * p_pData, int p_nMaxLen, const WHartUniqueID & p_oDeviceUniqueID)
{
    if ((int) sizeof(p_oDeviceUniqueID.bytes) > p_nMaxLen)
    {
        LOG_WARN("CHartCmdWrapper::GetHartDirectReq: buffer supplied too small");
        return -1;
    }

    if (m_pMetaCmdsInfo)
        memcpy(p_pData, m_pMetaCmdsInfo->m_oMetaCmdUniqueId.bytes, sizeof(m_pMetaCmdsInfo->m_oMetaCmdUniqueId.bytes));
    else
        memcpy(p_pData, p_oDeviceUniqueID.bytes, sizeof(p_oDeviceUniqueID.bytes));

    int nLen = GetRawWHart((uint8_t*) p_pData + sizeof(p_oDeviceUniqueID.bytes), p_nMaxLen - sizeof(p_oDeviceUniqueID.bytes), IsResponse());

    if (nLen < 0)
    {
        LOG_ERROR("CHartCmdWrapper::GetHartDirectReq: error at formatting message");
        return -1;
    }

    return nLen + sizeof(p_oDeviceUniqueID.bytes);
}

int CHartCmdWrapper::LoadHartWired(bool p_bIsResponse, const void * p_pData, int p_nMaxLen, WHartUniqueID & p_oDeviceUniqueID, uint8_t & errorCodeResponse)
{
    m_nFormat = transport::Packet::miHartWiredPDU;

    // Delimiter     Address     [ExpansionBytes]      Command     ByteCount    [Data]    CheckByte
    // The ExpansionBytes and Data field are optional and may not be present in some messages.
    // Token-Passing Data Link Layer Specification - spec081r8.2-depr.pdf : pag. 18/68

    int nPreHeader = 1 + 1;//delimiter + address
    int nHeaderLen = 1 + 1 + (p_bIsResponse ? 2 : 0); //cmdId + nLen + (responseCode + deviceStatus)?

    if (p_nMaxLen < nPreHeader + nHeaderLen)
    {
        errorCodeResponse = RCS_E05_TooFewDataBytesReceived;
        LOG_WARN("CHartCmdWrapper::LoadHartWiredReq: command too small - p_nMaxLen < nPreHeader + nHeaderLen; p_nMaxLen="
                 << p_nMaxLen << ", nPreHeader=" << nPreHeader << ", nHeaderLen=" << nHeaderLen);
        return -1;
    }

    BinaryStream stream;
    STREAM_INIT(&stream, (const uint8_t*) p_pData, p_nMaxLen);

    STREAM_READ_UINT8(&stream, &m_nDelimiter);

    //bit 7 from delimiter tells address size
    uint8_t addressSize = ((m_nDelimiter & HART_DELIM_ADDR_TYPE_MASK) == HART_DELIM_ADDR_TYPE_SHORT) ? 1 : 5;
    if (addressSize == 1)
    {
        STREAM_READ_UINT8(&stream, &m_nPolingAddress);
        uint8_t sixBitPollingAddress = m_nPolingAddress & 0x3F;

        if ((sixBitPollingAddress != 0) && (sixBitPollingAddress != 63))
        {
            LOG_WARN("CHartCmdWrapper::LoadHartWiredReq: poling address is " << (int) sixBitPollingAddress << " ; should be 0 or 63 ");
        }

        memcpy(p_oDeviceUniqueID.bytes, Gateway_UniqueID().bytes, sizeof(p_oDeviceUniqueID.bytes));
    }
    else
    {
        // addressSize == 5
        STREAM_READ_BYTES(&stream, p_oDeviceUniqueID.bytes, sizeof(p_oDeviceUniqueID.bytes));
    }

    //bits 6 and 5 tell nb. of exp. bytes
    uint8_t expansionBytesSize = (m_nDelimiter >> 5) & 0x11;
    uint8_t expansionBytes[3];
    if (expansionBytesSize > 0)
    {
        memset(expansionBytes, 0, 3);
        STREAM_READ_BYTES(&stream, expansionBytes, expansionBytesSize);
        nHeaderLen += expansionBytesSize; //adjust header length
    }

    if (ExtractWiredCommonHeader(p_bIsResponse, &stream, &m_u16CmdId, &m_nRawLen, errorCodeResponse, &m_i16ResponseCode, &m_u8DeviceStatus) < 0)
    {
        LOG_WARN("CHartCmdWrapper::LoadHartWired - ExtractWiredCommonHeader fail; data=" << nlib::detail::BytesToString((const uint8_t*) p_pData, p_nMaxLen));
        return -1;
    }

    //TODO - fix code duplication
    if (m_u16CmdId == CMDID_C64765_NivisMetaCommand)
    {
        int nMetaCmdHeaderLen = 2 + 5; //

        if (stream.remainingBytes < nMetaCmdHeaderLen)
        {
            LOG_WARN("CHartCmdWrapper::LoadRawWHart: C64765_NivisMetaCommand: too small data=" << nlib::detail::BytesToString((const uint8_t*) p_pData, p_nMaxLen));
            return -1;
        }

        if (!m_pMetaCmdsInfo)
        {
            m_pMetaCmdsInfo = new TMetaCmdsInfo;
        }

        //save to Meta command space
        m_pMetaCmdsInfo->m_u16MetaCmdId = m_u16CmdId;

        if (p_bIsResponse)
        {
            m_pMetaCmdsInfo->m_i16MetaCmdResponseCode = m_i16ResponseCode;
            m_pMetaCmdsInfo->m_u8MetaCmdDeviceStatus = m_u8DeviceStatus;
        }

        memcpy(m_pMetaCmdsInfo->m_oMetaCmdUniqueId.bytes, p_oDeviceUniqueID.bytes, sizeof(p_oDeviceUniqueID.bytes));

        STREAM_READ_UINT16(&stream, &m_pMetaCmdsInfo->m_oCmdNivisMeta.m_u16ExtDeviceType);
        STREAM_READ_BYTES(&stream, &m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes,
                          sizeof(m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes));

        if (ExtractDirectCommonHeader(p_bIsResponse, &stream, &m_u16CmdId, &m_nRawLen, errorCodeResponse, &m_i16ResponseCode, &m_u8DeviceStatus) < 0)
        {
            LOG_WARN("CHartCmdWrapper::LoadHartWired - ExtractDirectCommonHeader fail; data=" << nlib::detail::BytesToString((const uint8_t*) p_pData, p_nMaxLen));

            ReleaseAll();
            return -1;
        }

    }
    else if (m_u16CmdId == CMDID_C077_SendCommandToSubDevice)
    {
        int ret = LoadMetaCmd77Header(p_bIsResponse, stream, errorCodeResponse);
        if (ret < 0)
        {
            //errorCodeResponse = m_i16ResponseCode;
            //if (m_pMetaCmdsInfo)
            //{
            //	memcpy(m_pMetaCmdsInfo->m_oMetaCmdUniqueId.bytes, p_oDeviceUniqueID.bytes, sizeof(p_oDeviceUniqueID.bytes));
            //	memcpy( p_oDeviceUniqueID.bytes, m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes, sizeof(p_oDeviceUniqueID.bytes));
            //}
            return ret;
        }
    }

    if (m_nRawLen > 0)
    {
        m_pRawData = new uint8_t[m_nRawLen]; //m_nRawLen < 256 -> if this throws we are in trouble anyway
        STREAM_READ_BYTES(&stream, m_pRawData, m_nRawLen);
    }

    uint8_t checkByte;
    STREAM_READ_UINT8(&stream, &checkByte);

    int nbOfReadBytes = stream.nextByte - (uint8_t*) p_pData;

    if ( uint8_t computedChkByte = computeCheckByte(static_cast<const uint8_t*>(p_pData), nbOfReadBytes - 1/*chkbyte not included*/ ) != checkByte )
    {
        LOG_WARN("CHartCmdWrapper::LoadHartWiredReq: CheckByte error :  " << std::hex << (int) (computedChkByte)
                    << " vs " << (int) (checkByte));
        return -1;
    }

    if (m_pMetaCmdsInfo)
    {
        memcpy(m_pMetaCmdsInfo->m_oMetaCmdUniqueId.bytes, p_oDeviceUniqueID.bytes, sizeof(p_oDeviceUniqueID.bytes));
        memcpy(p_oDeviceUniqueID.bytes, m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes, sizeof(p_oDeviceUniqueID.bytes));
    }

    m_bHasRaw = true;

    return nbOfReadBytes;
}

int CHartCmdWrapper::GetHartWired(void* p_pData, int p_nMaxLen, const WHartUniqueID& p_oDeviceUniqueID,
                                  uint8_t p_nDelimiter)
{
    if (!HasRaw())
    {
        if (GetRawFromParsed() != RCS_N00_Success)
        {
            LOG_WARN("CHartCmdWrapper::GetHartWiredReq: no raw data");
            return -1;
        }
    }

    int nExtra = (IsResponse() ? 2 : 0);
    int nPreHeader = 1 + 1; //delimiter + address
    int nHeaderLen = 1 + 1 + nExtra; //cmdID + nLen + (response + devStatus)?
    int nMetaCmdHeader = 0;

    if (m_pMetaCmdsInfo)
    {
        if (m_pMetaCmdsInfo->m_u16MetaCmdId == CMDID_C64765_NivisMetaCommand)
        {
            nMetaCmdHeader = 2 + 5 + 2 + 1 + nExtra; // ext_dev_type(2) + uniqueId(5) + cmdId(2) + len(1) //+ rsp
        }
        else if (m_pMetaCmdsInfo->m_u16MetaCmdId == CMDID_C077_SendCommandToSubDevice)
        {
            // Common Practice Command Specification - spec151r9.1-depr page 88 of 160
            nMetaCmdHeader = 1 // card
                        + 1 // channel
                        + 1 // embeddedDelimiter
                        + 5 // embeddedAddress(1/5)
                        + 1 // embeddedCmdId(1)
                        + 1 // embeddedLen(1)
                        + nExtra // (rsp + dev status)(embeddedData)?
                        + (IsResponse() ? 0 : 1); //preamble (on request only)
        }
    }

    if (nPreHeader + nHeaderLen + m_nRawLen > p_nMaxLen)
    {
        LOG_WARN("CHartCmdWrapper::GetHartWiredReq: buffer supplied too small");
        return -1;
    }

    BinaryStream stream;
    STREAM_INIT(&stream, (const uint8_t*) p_pData, p_nMaxLen);

    //write delimiter
    m_nDelimiter = p_nDelimiter;

    STREAM_WRITE_UINT8(&stream, m_nDelimiter);

    //write address
    if (m_pMetaCmdsInfo != 0)
    {
        STREAM_WRITE_BYTES(&stream, m_pMetaCmdsInfo->m_oMetaCmdUniqueId.bytes,
                           sizeof(m_pMetaCmdsInfo->m_oMetaCmdUniqueId.bytes));
    }
    else
    {
        uint8_t addressSize = ((m_nDelimiter & HART_DELIM_ADDR_TYPE_MASK) == HART_DELIM_ADDR_TYPE_SHORT) ? 1 : 5;

        WHartUniqueID pAddr;

        memcpy(pAddr.bytes, p_oDeviceUniqueID.bytes, sizeof(p_oDeviceUniqueID.bytes));

        if ((m_nDelimiter & HART_DELIM_FRAME_TYPE_MASK) == HART_DELIM_FRAME_TYPE_BURST)
        {
            pAddr.bytes[0] |= HART_LONG_ADDR_BURST;
        }
        if (addressSize == 5)
        {
            STREAM_WRITE_BYTES(&stream, pAddr.bytes, sizeof(pAddr.bytes));
        }
        else
        {
            STREAM_WRITE_UINT8(&stream, m_nPolingAddress);
        }
    }

    if (m_pMetaCmdsInfo != 0 && IsResponseCodeError(m_pMetaCmdsInfo->m_i16MetaCmdResponseCode))
    {
        WriteWiredCommonHeader(IsResponse(), stream, m_pMetaCmdsInfo->m_u16MetaCmdId, m_nRawLen + nMetaCmdHeader,
                               m_pMetaCmdsInfo->m_i16MetaCmdResponseCode, &(m_pMetaCmdsInfo->m_u8MetaCmdDeviceStatus));
    }
    else if (m_pMetaCmdsInfo != 0)
    {
        WriteWiredCommonHeader(IsResponse(), stream, m_pMetaCmdsInfo->m_u16MetaCmdId, m_nRawLen + nMetaCmdHeader,
                               m_pMetaCmdsInfo->m_i16MetaCmdResponseCode, &(m_pMetaCmdsInfo->m_u8MetaCmdDeviceStatus));

        switch (m_pMetaCmdsInfo->m_u16MetaCmdId)
        {
            case CMDID_C64765_NivisMetaCommand:
                STREAM_WRITE_UINT16(&stream, m_pMetaCmdsInfo->m_oCmdNivisMeta.m_u16ExtDeviceType);
                STREAM_WRITE_BYTES(&stream, &m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes,
                                   sizeof(m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes));

                //write inner cmd
                STREAM_WRITE_UINT16(&stream, m_u16CmdId);
                STREAM_WRITE_UINT8(&stream, m_nRawLen + (IsResponse() ? 2 : 0));

                if (IsResponse())
                {
                    STREAM_WRITE_UINT8(&stream, m_i16ResponseCode);
                    STREAM_WRITE_UINT8(&stream, m_u8DeviceStatus);
                }
            break;
            case CMDID_C077_SendCommandToSubDevice:
                STREAM_WRITE_UINT8(&stream, m_pMetaCmdsInfo->m_oCmdSubDevice.m_nCard);
                STREAM_WRITE_UINT8(&stream, m_pMetaCmdsInfo->m_oCmdSubDevice.m_nChannel);

                if (!IsResponse())
                {
                    STREAM_WRITE_UINT8(&stream, m_pMetaCmdsInfo->m_oCmdSubDevice.m_nTransmitPreambleCount);
                }

                m_pMetaCmdsInfo->m_oCmdSubDevice.m_nDelimiter = HART_DELIM_ADDR_TYPE_LONG | HART_DELIM_FRAME_TYPE_D2M;
                STREAM_WRITE_UINT8(&stream, m_pMetaCmdsInfo->m_oCmdSubDevice.m_nDelimiter);

                if ((m_pMetaCmdsInfo->m_oCmdSubDevice.m_nDelimiter & HART_DELIM_ADDR_TYPE_MASK)
                            == HART_DELIM_ADDR_TYPE_SHORT)
                {
                    //Polling 1-byte address
                    STREAM_WRITE_UINT8(&stream, m_pMetaCmdsInfo->m_oCmdSubDevice.m_nDevicePollingAddress);
                }
                else
                {
                    //Unique 5-byte address
                    STREAM_WRITE_BYTES(&stream, &m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes,
                                       sizeof(m_pMetaCmdsInfo->m_oInnerCmdUniqueId.bytes));
                }
                WriteWiredCommonHeader(IsResponse(), stream, m_u16CmdId, m_nRawLen, m_i16ResponseCode,
                                       &m_u8DeviceStatus);
            break;
            default:
                LOG_ERROR("GetHartWiredReq: meta cmd unknown cmdId=" << m_pMetaCmdsInfo->m_u16MetaCmdId);
                return -1;
        }
    }
    else
    {
        WriteWiredCommonHeader(IsResponse(), stream, m_u16CmdId, m_nRawLen, m_i16ResponseCode, &m_u8DeviceStatus);
    }

    //write data bytes
    if (m_nRawLen > 0)
    {
        STREAM_WRITE_BYTES(&stream, m_pRawData, m_nRawLen);
    }

    //write checkByte
    uint8_t checkByte = computeCheckByte(static_cast<const uint8_t*> (p_pData), stream.nextByte - (uint8_t*) p_pData);
    STREAM_WRITE_UINT8(&stream, checkByte);

    return stream.nextByte - (uint8_t*) p_pData;
}

void CHartCmdWrapper::MetaCmdInfoSet(TMetaCmdsInfo* p_pMetaCmdsInfo)
{
    if (!p_pMetaCmdsInfo)
    {
        //delete
        delete m_pMetaCmdsInfo;
        m_pMetaCmdsInfo = NULL;
        return;
    }

    if (!m_pMetaCmdsInfo)
    {
        m_pMetaCmdsInfo = new TMetaCmdsInfo;
    }

    memcpy(m_pMetaCmdsInfo, p_pMetaCmdsInfo, sizeof(TMetaCmdsInfo));
}

void CmdList_MetaCmdInfoSet(CHartCmdWrapperList& p_rCmdList, TMetaCmdsInfo* p_pMetaCmdsInfo)
{
    CHartCmdWrapperList::const_iterator itCmds;
    for (itCmds = p_rCmdList.begin(); itCmds != p_rCmdList.end(); itCmds++)
    {
        (*itCmds)->MetaCmdInfoSet(p_pMetaCmdsInfo);
    }
}

void CmdList_MetaCmdInfoSet_ToNM(CHartCmdWrapperList& p_rCmdList, WHartUniqueID& p_rInnerUniqueId)
{
    TMetaCmdsInfo oMetaInfo;

    oMetaInfo.m_u16MetaCmdId = CMDID_C64765_NivisMetaCommand;
    oMetaInfo.m_oMetaCmdUniqueId = NetworkManager_UniqueID();
    oMetaInfo.m_oCmdNivisMeta.m_u16ExtDeviceType = NetworkManager_Nickname(); //NM nick and ext type are the same
    oMetaInfo.m_oInnerCmdUniqueId = p_rInnerUniqueId;

    CmdList_MetaCmdInfoSet(p_rCmdList, &oMetaInfo);
}

//returns the number bytes read
//		< 0 is error
int HartWired_ReadCmdList(bool p_bIsResponse, const void * p_pData, int p_nMaxLen, bool isFromAppHostClient, WHartUniqueID & p_oDeviceUniqueID,
                          CHartCmdWrapperList & p_rCmdList, uint8_t & errorCodeResponse, uint8_t p_u8DeviceStatus)
{
    uint8_t * u8APDU = (uint8_t*) p_pData;
    int nPos = 0;

    if (p_nMaxLen >= 0)
    {
        LOG_INFO_APP("HartWired_ReadCmdList: " << GetHex(p_pData, p_nMaxLen, ' '));
    }

    while (nPos < p_nMaxLen)
    {
        CHartCmdWrapper::Ptr pCmd(new CHartCmdWrapper);
        pCmd->IsFromAppHostClient(isFromAppHostClient);

        int nNextPos = pCmd->LoadHartWired(p_bIsResponse, u8APDU + nPos, p_nMaxLen - nPos, p_oDeviceUniqueID, errorCodeResponse);
        // LOG_DEBUG("HartWired_ReadCmdList - p_oDeviceUniqueID=" << p_oDeviceUniqueID);

        if (nNextPos <= 0)
        {
            LOG_ERROR("HartWired_ReadCmdList - LoadHartWired fail; data=" << nlib::detail::BytesToString((const uint8_t*) (u8APDU + nPos), p_nMaxLen - nPos));
            if (errorCodeResponse)
            {
                pCmd->SetResponseCode(errorCodeResponse);
                p_rCmdList.push_back(pCmd);
                LOG_WARN("LoadHartWired failed! errorCodeResponse=" << (int)errorCodeResponse << ", cmd=" << *pCmd);
            }
            else
            {
                LOG_ERROR("HartWired_ReadCmdList unknown error.");
            }
            return -1;
        }

        nPos += nNextPos;

        if (p_bIsResponse)
        {
            pCmd->SetDeviceStatusFromTL(p_u8DeviceStatus);
        }

        p_rCmdList.push_back(pCmd);
    }
    return nPos;
}

//returns the number bytes written
//		< 0 is error
int HartWired_WriteCmdList(void* p_pData, int p_nMaxLen, const WHartUniqueID& p_oDeviceUniqueID,
                           CHartCmdWrapperList& p_rCmdList, uint8_t p_nDelimiter)
{
    uint8_t* buffer = (uint8_t*) p_pData;
    uint16_t writtenBytes = 0;

    CHartCmdWrapperList::const_iterator itCmds;
    for (itCmds = p_rCmdList.begin(); itCmds != p_rCmdList.end(); itCmds++)
    {
        if (writtenBytes >= p_nMaxLen)
        {
            LOG_ERROR("HartWiredReq_WriteCmdList: cmds still to write but not enough space in buffer");
            return -1;
        }

        CHartCmdWrapper::Ptr pCmd = *itCmds;

        int nCurrent = pCmd->GetHartWired(buffer + writtenBytes, p_nMaxLen - writtenBytes, p_oDeviceUniqueID,
                                          p_nDelimiter);

        if (nCurrent <= 0)
        {
            return -1;
        }
        writtenBytes += nCurrent;
    }

    if (writtenBytes >= 0)
    {
        LOG_INFO_APP("HartWired_WriteCmdList: " << GetHex(p_pData, writtenBytes, ' '));
    }

    return writtenBytes;
}

//returns the number bytes read
//		< 0 is error
int HartDirect_ReadCmdList(bool p_bIsResponse, const void* p_pData, int p_nMaxLen, bool isFromAppHostClient, WHartUniqueID& p_oDeviceUniqueID,
                           CHartCmdWrapperList& p_rCmdList, uint8_t & errorCodeResponse, uint8_t p_u8DeviceStatus)
{
    uint8_t * u8APDU = (uint8_t*) p_pData;
    int nPos = 0;

    while (nPos < p_nMaxLen)
    {
        CHartCmdWrapper::Ptr pCmd(new CHartCmdWrapper);
        pCmd->IsFromAppHostClient(isFromAppHostClient);

        int nNextPos = pCmd->LoadHartDirect(p_bIsResponse, u8APDU + nPos, p_nMaxLen - nPos, p_oDeviceUniqueID, errorCodeResponse);

        if (nNextPos <= 0)
        {
            LOG_ERROR("HartDirect_ReadCmdList - LoadHartDirect fail; data=" << nlib::detail::BytesToString((const uint8_t*) (u8APDU + nPos), p_nMaxLen - nPos));

            if (errorCodeResponse)
            {
                pCmd->SetResponseCode(errorCodeResponse);
                p_rCmdList.push_back(pCmd);
                LOG_WARN("LoadHartDirect failed! errorCodeResponse=" << (int)errorCodeResponse << ", cmd=" << *pCmd);
            }
            else
            {
                LOG_ERROR("HartDirect_ReadCmdList unknown error.");
            }
            return -1;
        }

        nPos += nNextPos;

        if (p_bIsResponse)
        {
            pCmd->SetDeviceStatusFromTL(p_u8DeviceStatus);
        }

        p_rCmdList.push_back(pCmd);
    }
    return nPos;
}

//returns the number bytes written
//		< 0 is error
int HartDirect_WriteCmdList(void* p_pData, int p_nMaxLen, const WHartUniqueID& p_oDeviceUniqueID,
                            CHartCmdWrapperList& p_rCmdList)
{
    uint8_t* buffer = (uint8_t*) p_pData;
    uint16_t writtenBytes = 0;

    CHartCmdWrapperList::const_iterator itCmds;
    for (itCmds = p_rCmdList.begin(); itCmds != p_rCmdList.end(); itCmds++)
    {
        if (writtenBytes >= p_nMaxLen)
        {
            LOG_ERROR("HartDirect_WriteCmdList: cmds still to write but not enough space in buffer");
            return -1;
        }

        CHartCmdWrapper::Ptr pCmd = *itCmds;

        int nCurrent = pCmd->GetHartDirect(buffer + writtenBytes, p_nMaxLen - writtenBytes, p_oDeviceUniqueID);

        if (nCurrent <= 0)
        {
            return -1;
        }
        writtenBytes += nCurrent;
    }
    return writtenBytes;
}

//returns the number bytes read
//		< 0 is error
int RawWHart_ReadCmdList(bool p_bIsResponse, const void * p_pData, int p_nMaxLen, CHartCmdWrapperList & p_rCmdList, uint8_t & errorCodeResponse, uint8_t p_u8DeviceStatus)
{
    uint8_t* u8APDU = (uint8_t*) p_pData;
    int nPos = 0;

    while (p_nMaxLen > nPos)
    {
        CHartCmdWrapper::Ptr pCmd(new CHartCmdWrapper);

        int nCrt = pCmd->LoadRawWHart(p_bIsResponse, u8APDU + nPos, p_nMaxLen - nPos, errorCodeResponse);
        if (nCrt <= 0)
        { //if one bad drop all? or drop all after
            LOG_ERROR("RawWHart_ReadCmdList - LoadRawWHart failed; data=" << nlib::detail::BytesToString((const uint8_t*) u8APDU + nPos, p_nMaxLen - nPos));
            return -1;
        }
        nPos += nCrt;
        if (p_bIsResponse)
        {
            pCmd->SetDeviceStatusFromTL(p_u8DeviceStatus);
        }

        //LOG_INFO("GatewayStack::ExtractWHartCmds " << pCmd );

        p_rCmdList.push_back(pCmd);
    }

    return nPos;
}

//returns the number bytes written
//		< 0 is error
int RawWHart_WriteCmdList(void * p_pData, int p_nMaxLen, const CHartCmdWrapperList& p_rCmdList)
{
    uint8_t* buffer = (uint8_t*) p_pData;
    uint16_t writtenBytes = 0;

    CHartCmdWrapperList::const_iterator itCmds;
    for (itCmds = p_rCmdList.begin(); itCmds != p_rCmdList.end(); itCmds++)
    {
        if (writtenBytes >= p_nMaxLen)
        {
            LOG_ERROR("RawWHart_WriteCmdList: cmds still to write but not enough space in buffer");
            return -1;
        }

        CHartCmdWrapper::Ptr pCmd = *itCmds;

        int nCurrent = pCmd->GetRawWHart(buffer + writtenBytes, p_nMaxLen - writtenBytes);

        if (nCurrent <= 0)
        {
            return -1;
        }
        writtenBytes += nCurrent;
    }

    return writtenBytes;
}

std::ostream& operator<<(std::ostream& out, const CHartCmdWrapper& p_rCmd)
{
    return operator<<(out, &p_rCmd);
}

std::ostream & operator<<(std::ostream & out, const CHartCmdWrapper * p_pCmd)
{
    if (!p_pCmd)
    {
        out << " cmd: NONE ";
        return out;
    }
    CHartCmdWrapper * pCmd = (CHartCmdWrapper*) p_pCmd;
    out << std::dec << " cmdId=" << (int) pCmd->GetCmdId();

    if (pCmd->IsResponse())
    {
        out << " rc=" << (int) pCmd->GetResponseCode() << " ds=" << (int) pCmd->GetDeviceStatus();
    }
    else
    {
        out << " req";
    }

    if (pCmd->HasRaw())
    {
        out << " Raw" << " len=" << (int) pCmd->GetRawDataLen() << " data=" << GetHex(pCmd->GetRawData(), pCmd->GetRawDataLen(), ' ');
    }
    else if (pCmd->HasParsed())
    {
        out << " Parsed" << " len=" << (int) pCmd->GetParsedDataLen() << " data=" << GetHex(pCmd->GetParsedData(), pCmd->GetParsedDataLen(), ' ');
    }
    else
    {
        out << "EMPTY ";
    }
    if (pCmd->MetaCmdInfoGet())
    {
        out << " -- mCmdId=" << (int) pCmd->MetaCmdInfoGet()->m_u16MetaCmdId << " mAddr=" << pCmd->MetaCmdInfoGet()->m_oMetaCmdUniqueId << " inAddr=" << pCmd->MetaCmdInfoGet()->m_oInnerCmdUniqueId;
    }
    return out;
}

std::ostream& operator<<(std::ostream & out, const CHartCmdWrapperList & p_rCmdList)
{
    CHartCmdWrapperList::const_iterator itCmd = p_rCmdList.begin();
    for (; itCmd != p_rCmdList.end(); itCmd++)
    {
        out << " [" << *itCmd << " ] ";
    }
    return out;
}

std::ostream & operator<< (std::ostream & stream, const CHartCmdWrapperShortDetails & hartCmdWrapperShortDetails)
{
    CHartCmdWrapper & hartCmdWrapper = (CHartCmdWrapper &)hartCmdWrapperShortDetails.hartCmdWrapper;
    stream << std::dec << " cmdId=" << hartCmdWrapper.GetCmdId();

    if (hartCmdWrapper.IsResponse())
    {
        stream << " rc=" << hartCmdWrapper.GetResponseCode() << " ds=" << (int)hartCmdWrapper.GetDeviceStatus();
    }
    else
    {
        stream << " req";
    }

    if (hartCmdWrapper.HasRaw())
    {
        stream << " Raw" << " len=" << hartCmdWrapper.GetRawDataLen();
    }
    else if (hartCmdWrapper.HasParsed())
    {
        stream << " Parsed" << " len=" << hartCmdWrapper.GetParsedDataLen();
    }
    else
    {
        stream << "EMPTY ";
    }
    if (hartCmdWrapper.MetaCmdInfoGet())
    {
        const TMetaCmdsInfo * pTMetaCmdsInfo = hartCmdWrapper.MetaCmdInfoGet();
        stream << " -- mCmdId=" << (int) pTMetaCmdsInfo->m_u16MetaCmdId << " mAddr=" << pTMetaCmdsInfo->m_oMetaCmdUniqueId << " inAddr=" << pTMetaCmdsInfo->m_oInnerCmdUniqueId;
    }
    return stream;
}

std::ostream & operator<< (std::ostream & stream, const CHartCmdWrapperListShortDetails & hartCmdWrapperListShortDetails)
{
    const CHartCmdWrapperList & hartCmdWrapperList = hartCmdWrapperListShortDetails.hartCmdWrapperList;
    CHartCmdWrapperList::const_iterator itCmd = hartCmdWrapperList.begin();
    for (; itCmd != hartCmdWrapperList.end(); itCmd++)
    {
        stream << " [" << CHartCmdWrapperShortDetails(*(*itCmd)) << " ] ";
    }
    return stream;
}



} // namespace stack
} // namespace hart7

