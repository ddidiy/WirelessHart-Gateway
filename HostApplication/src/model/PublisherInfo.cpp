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

#include <WHartHost/model/PublisherInfo.h>



namespace hart7 {
namespace hostapp {

PublishChannelSetT::const_iterator FindFirstChannel(const PublishChannelSetT& p_rSet, uint16_t cmdNo)
{
	for (PublishChannelSetT::const_iterator i = p_rSet.begin() ; i != p_rSet.end(); ++i)
	{
		if (i->cmdNo == cmdNo) return i;
	}

	return p_rSet.end();
}

PublishChannelSetT::const_iterator FindChannel(const PublishChannelSetT& p_rSet, int channelID)
{
	for (PublishChannelSetT::const_iterator i = p_rSet.begin() ; i != p_rSet.end(); ++i)
	{
		if (i->channelID == channelID) return i;
	}
	
	return p_rSet.end();
}

std::string GetPublisherInfoString(PublisherInfo& publisherInfo)
{
    std::ostringstream oss;

    oss << std::endl << std::endl << "BURSTS={";
    for (BurstMessageSetT::iterator itBurst = publisherInfo.burstMessageList.begin(); itBurst != publisherInfo.burstMessageList.end() ; ++itBurst)
    {
        if (itBurst != publisherInfo.burstMessageList.begin()) {
            oss << std::endl << "        ";
        }
        oss << "["
            << (int)itBurst->burstMessage << ", "
            << (int)itBurst->cmdNo << ", "
            << (int)itBurst->updatePeriod << ", "
            << (int)itBurst->maxUpdatePeriod << "]";
    }

    oss << "}" << std::endl << "VARIABLES={";
    for (PublishChannelSetT::iterator itChannel = publisherInfo.channelList.begin(); itChannel != publisherInfo.channelList.end() ; ++itChannel)
    {
        if (itChannel != publisherInfo.channelList.begin()) {
            oss << std::endl << "           ";
        }
        oss << "["
            << (int)itChannel->burstMessage << ", "
            << (int)itChannel->cmdNo << ", "
            << (int)itChannel->deviceVariableSlot << ", "
            << (int)itChannel->deviceVariable << ", "
            << itChannel->name << ", "
            << (int)itChannel->classification
            << (int)itChannel->unitCode << "]";
    }

    oss << "}" << std::endl << "TRIGGERS={";
    for (TriggerSetT::iterator itTrigger = publisherInfo.triggersList.begin(); itTrigger != publisherInfo.triggersList.end() ; ++itTrigger)
    {
        if (itTrigger != publisherInfo.triggersList.begin()) {
            oss << std::endl << "          ";
        }
        oss << "["
            << (int)itTrigger->burstMessage << ", "
            << (int)itTrigger->cmdNo << ", "
            << (int)itTrigger->modeSelection << ", "
            << (int)itTrigger->classification << ", "
            << (int)itTrigger->unitCode << ", "
            << itTrigger->triggerLevel << "]";
    }
    oss << "}" << std::endl;

    return oss.str();
}

std::ostream& operator<<(std::ostream& p_rOut, const BurstState& p_rBurstState)
{
	switch(p_rBurstState)
	{
	case BURST_STATE_NOT_SET:
		p_rOut << "NOT_SET";
		break;
	case BURST_STATE_SET:
		p_rOut << "SET";
		break;
	case BURST_STATE_TIMEOUT:
		p_rOut << "TIMEOUT";
		break;
	case BURST_STATE_ERROR:
		p_rOut << "ERROR";
		break;
	case BURST_STATE_OFF:
		p_rOut << "OFF";
		break;
	}

	return p_rOut;
}

std::string GetSetPublishStateText(SetPublisherState p_eState)
{
    switch (p_eState)
    {
        case SETPUBLISHER_STATE_NONE:               return "NONE";
        case SETPUBLISHER_STATE_READ_BURSTCONFIG:   return "READ BURST CONFIGURATION";
        case SETPUBLISHER_STATE_TURNOFF_BURST:      return "TURNOFF BURST MESSAGE";
        case SETPUBLISHER_STATE_GET_SUBDEVICEINDEX: return "GET SUBDEVICE INDEX";
        case SETPUBLISHER_STATE_CONFIGURE_BURST:    return "CONFIGURE BURST MESSAGE";
        case SETPUBLISHER_STATE_DONE:               return "CONFIGURED";
        default:                                    return "UNKNOWN";
    }
}

std::string GetSetPublishErrorText(SetPublisherError p_eError)
{
    switch (p_eError)
    {
        case SETPUBLISHER_ERROR_NONE:                               return "NONE";
        case SETPUBLISHER_ERROR_READ_BURSTCONFIG:                   return "READ BURST CONFIGURATION";
        case SETPUBLISHER_ERROR_READ_BURSTCONFIG_CMD105_NOT_IMPL:   return "COMMAND 105 NOT IMPLEMENTED";
        case SETPUBLISHER_ERROR_TURNOFF_BURST:                      return "TURN OFF BURST MESSAGE";
        case SETPUBLISHER_ERROR_TURNOFF_BURST_CMD109_NOT_IMPL:      return "COMMAND 109 NOT IMPLEMENTED";
        case SETPUBLISHER_ERROR_GET_SUBDEVICEINDEX:                 return "GET SUBDEVICE INDEX";
        case SETPUBLISHER_ERROR_CONFIGBURST:                        return "CONFIGURE BURST MESSAGE";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD102_NOT_IMPL:        return "COMMAND 102 NOT IMPLEMENTED";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD103_NOT_IMPL:        return "COMMAND 103 NOT IMPLEMENTED";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD104_NOT_IMPL:        return "COMMAND 104 NOT IMPLEMENTED";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD106_NOT_IMPL:        return "COMMAND 106 NOT IMPLEMENTED";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD107_NOT_IMPL:        return "COMMAND 107 NOT IMPLEMENTED";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD108_NOT_IMPL:        return "COMMAND 108 NOT IMPLEMENTED";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD109_NOT_IMPL:        return "COMMAND 109 NOT IMPLEMENTED";
        case SETPUBLISHER_ERROR_CONFIGBURST_BUSY:                   return "DEVICE BUSY";
        case SETPUBLISHER_ERROR_CONFIGBURST_MAP_SUBDEVICE:          return "MAP SUBDEVICE";
        case SETPUBLISHER_ERROR_CONFIGBURST_WRITE_UPDATE_PERIOD:    return "WRITE UPDATE PERIOD";
        case SETPUBLISHER_ERROR_CONFIGBURST_WRITE_TRIGGER:          return "WRITE TRIGGER";
        case SETPUBLISHER_ERROR_CONFIGBURST_WRITE_VARIABLES:        return "WRITE VARIABLE";
        case SETPUBLISHER_ERROR_CONFIGBURST_WRITE_COMMAND_NO:       return "WRITE COMMAND NO";
        case SETPUBLISHER_ERROR_CONFIGBURST_SET_BURST_ON:           return "SET BURST ON";
        case SETPUBLISHER_ERROR_FATAL:                              return "FATAL";
        default:                                                    return "UNKNOWN";
    }
}

std::string GetPublisherErrorList(SetPublisherError error, PublisherState::BurstErrorLST_T& burstErrorList)
{
    std::ostringstream ostr;
    int listSize = burstErrorList.size();
    if (listSize > 0)
    {
        for (PublisherState::BurstErrorLST_T::iterator it = burstErrorList.begin(); it != burstErrorList.end(); ++it)
        {
            listSize--;
            ostr << it->burstNo << "=" << (int)it->setPublisherError << (listSize == 0 ? "" : ",");
        }
    }
    else
    {
        ostr << (int)error;
    }
    return ostr.str();
}

std::string GetPublisherMessageList(std::string message, PublisherState::BurstErrorLST_T& burstErrorList)
{
    std::ostringstream ostr;
    int listSize = burstErrorList.size();
    if (listSize > 0)
    {
        for (PublisherState::BurstErrorLST_T::iterator it = burstErrorList.begin(); it != burstErrorList.end(); ++it)
        {
            listSize--;
            ostr << it->setPublisherMessage << (listSize == 0 ? "" : ",");
        }
    }
    else
    {
        ostr << message;
    }
    return ostr.str();
}

}
}
