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

#include <nlib/log.h>
#include <Shared/Config.h>
#include <stdlib.h>

#include <WHartHost/PublisherConf.h>
#include <WHartHost/Utils.h>


//file
int PublisherConf::LoadConfigFile(const char *pFileName/*[in]*/, CIniParser& parser /*[in/out]*/)
{
	return !parser.Load(pFileName, "rb", true) ? -1 : 0;
}

int PublisherConf::LoadConfigFileWrite(const char *pFileName/*[in]*/, CIniParser& parser /*[in/out]*/)
{
	return !parser.Load(pFileName, "r+", true) ? -1 : 0;
}

//mac
int PublisherConf::LoadFirstMAC(CIniParser& parser /*[in]*/, hart7::hostapp::MAC& mac/*[in/out]*/)
{
    const char *pgroup = NULL;
	LOG_INFO_APP( "[LoadPublishers]: loading first mac...");
    if (!(pgroup = parser.FindGroup(NULL, false, true, true)))
	{
		LOG_WARN_APP( "[LoadPublishers]: no mac found");
        return -1;
	}
	if (pgroup[hart7::hostapp::MAC::TEXT_SIZE] != '\0')
	{
		LOG_ERROR_APP( "[LoadPublishers]: read mac has not the proper format.");
		return -1;
	}


    std::string strMac = pgroup;
	LOG_INFO_APP( "[LoadPublishers]: mac=" << strMac << " read from conf.");
    try
    {
        mac = hart7::hostapp::MAC(strMac);
    }
    catch (...)
    {
        return -1;
    }
    return 0;
}

int PublisherConf::LoadNextMAC(CIniParser& parser /*[in]*/, hart7::hostapp::MAC& mac/*[in/out]*/)
{
    const char *pgroup = NULL;
	LOG_INFO_APP( "[LoadPublishers]: loading next mac...");
    if (!(pgroup = parser.FindGroup(NULL, true, true, true)))
	{
		LOG_WARN_APP( "[LoadPublishers]: no mac found");
		return -1;
	}


	if (pgroup[hart7::hostapp::MAC::TEXT_SIZE] != '\0')
	{
		LOG_ERROR_APP( "[LoadPublishers]: read mac has not the proper format.");
		return -1;
	}


    std::string strMac = pgroup;
	LOG_INFO_APP("[LoadPublishers]: mac=" << strMac << " read from conf." );

    try
    {
        mac = hart7::hostapp::MAC(strMac);
    }
    catch (...)
    {
        return -1;
    }
    return 0;
}

//burst message
/*BURST=<command number>, <burst message>, <update period>, <maximum update period>*/
int PublisherConf::LoadBurstMessage(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, int burstMessageID, hart7::hostapp::BurstMessage& burstMessage/*[in/out]*/)
{
	char szLogVarString[256] = "";
	LOG_INFO_APP( "[LoadPublishers]: loading burst message...");

	if (!parser.GetVarRawString(NULL, "BURST", szLogVarString, sizeof(szLogVarString), burstMessageID))
		return -1;
	LOG_INFO_APP( "[LoadPublishers]: BURST[" << mac << "]=" << szLogVarString);

	unsigned int cmdNo, burstMsg;
	char szSubdevMAC[32] = "", exData[2];

	int rv = sscanf( szLogVarString, "%u, %u, %lf, %lf, %s, %1s "
		, &cmdNo
		, &burstMsg
		, &(burstMessage.updatePeriod)
		, &(burstMessage.maxUpdatePeriod)
        , szSubdevMAC
		, exData
	);

	if(rv != 4 && rv != 5)
	{
	    LOG_ERROR_APP("[LoadPublishers]: read " << rv << " items from buffer instead of 4 or 5");
		return -1;
	}

	if (rv == 4)
	{
	    hart7::hostapp::MAC emptyMAC;
	    burstMessage.subDeviceMAC = emptyMAC;
	}
	else
	{
		try
		{
			hart7::hostapp::MAC subdevMAC(szSubdevMAC);
			burstMessage.subDeviceMAC = subdevMAC;
	    }
	    catch (...)
	    {
	        return -1;
	    }
	}

	char burstNo[10]="";
	snprintf(burstNo,sizeof(burstNo),"%d",burstMessageID);	
	char szVarName[] = "BURST_SET_PUBLISH_PERIOD_";
	strcat(szVarName, burstNo);
	
	if (parser.GetVarRawString(NULL, szVarName, szLogVarString, sizeof(szLogVarString)))
	{
		std::stringstream(szLogVarString) >> burstMessage.updatePeriod;
		LOG_INFO_APP("[LoadPublishers]: BURST_SET_PUBLISH_PERIOD loaded: updatePeriod=" << burstMessage.updatePeriod << " overrides base update period.");
	}
	burstMessage.cmdNo = (unsigned short)cmdNo;
	burstMessage.burstMessage = (unsigned char)burstMsg;

	return 0;
}

int PublisherConf::LoadBurstMessages(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, hart7::hostapp::BurstMessageSetT& list/*[in/out]*/, std::list<double>& burstUpdatePeriods/*[in/out]*/)
{
	list.clear();

	hart7::hostapp::BurstMessage burstMessage;
	int burstMessageID = 0;

	if (LoadBurstMessage(parser, mac, burstMessageID, burstMessage) < 0)
		return -1;

	burstUpdatePeriods.push_back(burstMessage.updatePeriod);
	
	list.insert(burstMessage);

	while (LoadBurstMessage(parser, mac, ++burstMessageID, burstMessage) == 0)
	{
		burstUpdatePeriods.push_back(burstMessage.updatePeriod);
		list.insert(burstMessage);
	}

	return 0;
}

//channel
/* VARIABLE=<command number>, <burst message>, <device variable code>, <name>, <device variable slot> <device variable classification>, <units code> */
int PublisherConf::LoadChannel(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, int channelID, hart7::hostapp::PublishChannel& channel/*[in/out]*/)
{
	char szLogVarString[256] = "";
	LOG_INFO_APP("[LoadPublishers]: loading channel...");
	if (!parser.GetVarRawString(NULL, "VARIABLE", szLogVarString, sizeof(szLogVarString), channelID))
		return -1;
    LOG_INFO_APP( "[LoadPublishers]: VARIABLE[" << mac << "]=" << szLogVarString);

	//
	unsigned int cmdNo, burstMessage, deviceVariable, deviceVariableSlot, classification, unitCode;
	char name[255];
	char exData[2];
	int rv = sscanf( szLogVarString, "%u , %u , %u , %[^,] , %u , %u , %u, %1s "
		, &cmdNo
		, &burstMessage
		, &deviceVariable
		, name
		, &deviceVariableSlot
		, &classification
		, &unitCode
		, exData) ;

	if(rv != 7)
	{
		LOG_ERROR_APP("[LoadPublishers]: read " << rv << " items from buffer instead of 7");
		return -1;
	}

	channel.cmdNo				= (unsigned short)cmdNo;
	channel.burstMessage		= (unsigned char)burstMessage;
	channel.deviceVariable		= (unsigned char)deviceVariable;
	channel.deviceVariableSlot	= (unsigned char)deviceVariableSlot;
	channel.classification		= (unsigned char)classification;
	channel.unitCode			= (unsigned char)unitCode;
	channel.name = name;

	return 0;
}

int PublisherConf::LoadChannels(CIniParser& parser /*[in]*/,
                                const hart7::hostapp::MAC& mac,
                                hart7::hostapp::PublishChannelSetT& list /*[in/out]*/)
{
	list.clear();

	hart7::hostapp::PublishChannel channel;
	int channelID = 0;

	if (LoadChannel(parser, mac, channelID, channel) < 0)
	{	return -1;
	}

	list.insert(channel);

	while (LoadChannel(parser, mac, ++channelID, channel) == 0)
	{	list.insert(channel);
	}

	return 0;
}

//trigger
/*TRIGGER=<command number>, <burst message>, <burst trigger mode selection>, <device variable classification>, <units code>, <trigger level>*/
int PublisherConf::LoadTrigger(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, int triggerID, hart7::hostapp::Trigger& trigger/*[in/out]*/)
{
	char szLogVarString[256] = "";
	LOG_INFO_APP( "[LoadPublishers]: loading trigger...");
	if (!parser.GetVarRawString(NULL, "TRIGGER", szLogVarString, sizeof(szLogVarString), triggerID))
		return -1;
    LOG_INFO_APP( "[LoadPublishers]: TRIGGER[" << mac << "]=" << szLogVarString);
	
	unsigned int cmdNo, burstMessage, modeSelection, classification, unitCode;
	float triggerLevel;
	char exData[2];
	int rv = sscanf( szLogVarString, "%u , %u , %u , %u , %u , %f, %1s  "
		, &cmdNo
		, &burstMessage
		, &modeSelection
		, &classification
		, &unitCode
		, &triggerLevel
		, exData);

	if(rv != 6)
	{
	    LOG_ERROR_APP("[LoadPublishers]: read " << rv << " items from buffer instead of 6");
		return -1;
	}

	trigger.cmdNo = (unsigned short)(cmdNo);
	trigger.burstMessage = (unsigned char)burstMessage;
	trigger.modeSelection = (unsigned char)modeSelection;
	trigger.classification = (unsigned char)classification;
	trigger.unitCode = (unsigned char)unitCode;
	trigger.triggerLevel = triggerLevel;

	return 0;
}

int PublisherConf::LoadTriggers(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, hart7::hostapp::TriggerSetT& list/*[in/out]*/)
{
	list.clear();

	hart7::hostapp::Trigger trigger;
	int triggerID = 0;

	if (LoadTrigger(parser, mac, triggerID, trigger) < 0)
		return -1;
	list.insert(trigger);

	while (LoadTrigger(parser, mac, ++triggerID, trigger) == 0)
	{
		list.insert(trigger);
	}

	return 0;
}

int PublisherConf::LoadPublishers(const char *pFileName/*[in]*/, hart7::hostapp::PublisherInfoMAP_T& data /*[in/out]*/, std::list<double>& burstUpdatePeriods/*[out]*/)
{
	CIniParser parser;
	data.clear();
	if (LoadConfigFile(pFileName, parser) < 0)
	{
		LOG_WARN_APP("[LoadPublishers]: invalid config file");
		data.clear();
		return -1;
	}

	hart7::hostapp::MAC mac;
	if (LoadFirstMAC(parser, mac) < 0)
	{
		LOG_WARN_APP("[LoadPublishers]: invalid config file - Invalid mac");
		data.clear();
		return -1;
	}


	hart7::hostapp::PublishChannelSetT chList_;
	hart7::hostapp::BurstMessageSetT burstList_;
	hart7::hostapp::TriggerSetT triggerList_;
	hart7::hostapp::PublisherInfo::BurstMessageStateMap burstMessageStateList_;

	if (LoadBurstMessages(parser, mac, burstList_, burstUpdatePeriods) < 0)
	{
		LOG_WARN_APP("[LoadPublishers]: invalid config file - invalid burst messages, so skip burst messages...");
	}

	if(LoadBurstMessageStats(parser, mac, burstMessageStateList_) < 0)
	{
		LOG_WARN_APP("[LoadPublishers]: invalid config file - invalid burst messages stat");
	}

	if (LoadChannels(parser, mac, chList_) < 0)
	{
		LOG_WARN_APP("[LoadPublishers]: invalid config file - invalid channel, so skip channels...");
	}

	if (LoadTriggers(parser, mac, triggerList_) < 0)
	{
		LOG_WARN_APP("[LoadPublishers]: invalid config file - invalid triggers, so skip triggers...");
	}

	int burstNoTotalCmd105;
	if(LoadBurstNoTotalCmd105(parser,mac,burstNoTotalCmd105) < 0)
	{
		LOG_WARN_APP("[LoadPublishers]: variable BURST_NO_TOTAL_CMD_105 dosen't exist, set default value ...");
		burstNoTotalCmd105 = hart7::hostapp::PublisherInfo::g_nUnsetBurstNoTotalCmd105;
	}

	CheckBurstMessages(mac,burstList_,chList_,triggerList_);

	if (data.insert(hart7::hostapp::PublisherInfoMAP_T::value_type(mac, hart7::hostapp::PublisherInfo (burstList_, chList_, triggerList_, burstNoTotalCmd105, burstMessageStateList_))).second == false)
	{
		LOG_WARN_APP("[LoadPublishers]: error adding mac=" << mac);
	}
	else
	{
	    data[mac].autodetectState = hart7::hostapp::AUTODETECT_NONE;
	    LoadAutodetect(parser, data[mac].autodetectState);
	}

	while(LoadNextMAC(parser, mac) == 0)
	{
		if (LoadBurstMessages(parser, mac, burstList_, burstUpdatePeriods) < 0)
		{
			LOG_WARN_APP("[LoadPublishers]: invalid burst messages");
			//continue;
		}

		if(LoadBurstMessageStats(parser, mac, burstMessageStateList_) < 0)
		{
			LOG_INFO_APP("[LoadPublishers]: no burst messages states");
			//continue;
		}

		if (LoadChannels(parser, mac, chList_) < 0)
		{
			LOG_WARN_APP("[LoadPublishers]: invalid channel for mac= " << mac );
			//continue;
		}

		if (LoadTriggers(parser, mac, triggerList_) < 0)
		{
			LOG_WARN_APP("[LoadPublishers]: invalid triggers for mac= " << mac);
		}

		int burstNoTotCmd105;
		if(LoadBurstNoTotalCmd105(parser,mac,burstNoTotCmd105) < 0)
		{
			LOG_INFO_APP("[LoadPublishers]: BURST_NO_TOTAL_CMD_105 is unset - set default value ...");
			burstNoTotCmd105 = hart7::hostapp::PublisherInfo::g_nUnsetBurstNoTotalCmd105;
		}

		CheckBurstMessages(mac,burstList_,chList_,triggerList_);

		if (data.insert(hart7::hostapp::PublisherInfoMAP_T::value_type(mac, hart7::hostapp::PublisherInfo (burstList_, chList_, triggerList_, burstNoTotCmd105, burstMessageStateList_))).second == false)
		{
			LOG_WARN_APP("[LoadPublishers]: error adding mac=" << mac);
		}
	    else
	    {
	        data[mac].autodetectState = hart7::hostapp::AUTODETECT_NONE;
	        LoadAutodetect(parser, data[mac].autodetectState);
	    }
	}

	return 0;
}

//burst stat
//BURST_SET_STAT = BURST_MSG_NO, {NOT_SET, SET, OFF, TIMEOUT, ERROR}
int PublisherConf::LoadBurstMessageStats(CIniParser& parser , const hart7::hostapp::MAC& mac, hart7::hostapp::PublisherInfo::BurstMessageStateMap& list)
{	
	char szDefaultName[] = "BURST_SET_STAT_";
	char szVarName[20]="";
	strcpy(szVarName,szDefaultName);
	char szVarValue[20]="";
	uint8_t burstMessageNo;
	hart7::hostapp::BurstState burstState = hart7::hostapp::BURST_STATE_NOT_SET;

	list.clear();

	if(parser.FindFirstVar(szVarName,sizeof(szVarName),szVarValue,sizeof(szVarValue)))
	{
		if(!strncmp(szDefaultName,szVarName,strlen(szDefaultName)))
		{
			burstMessageNo = strtol(szVarName+strlen(szDefaultName),NULL,10);
			if(!strcmp(szVarValue ,"NOT_SET")) burstState = hart7::hostapp::BURST_STATE_NOT_SET;
			if(!strcmp(szVarValue ,"SET")) burstState = hart7::hostapp::BURST_STATE_SET;
			if(!strcmp(szVarValue ,"TIMEOUT")) burstState = hart7::hostapp::BURST_STATE_TIMEOUT;
			if(!strcmp(szVarValue ,"ERROR")) burstState = hart7::hostapp::BURST_STATE_ERROR;
			if(!strcmp(szVarValue ,"OFF")) burstState = hart7::hostapp::BURST_STATE_OFF;

			LOG_INFO_APP("[LoadPublishers]: Load BURST_SET_STAT: " << szVarValue );

			list.insert(hart7::hostapp::PublisherInfo::BurstMessageStateMap::value_type(burstMessageNo,burstState));
		}
		strcpy(szVarName,szDefaultName);
	}	
	else return -1;

	while(parser.FindNextVar(szVarName,sizeof(szVarName),szVarValue,sizeof(szVarValue)))
	{
		if(!strncmp(szDefaultName,szVarName,strlen(szDefaultName)))
		{
			burstMessageNo = strtol(szVarName+strlen(szDefaultName),NULL,10);
			if(!strcmp(szVarValue ,"NOT_SET")) burstState = hart7::hostapp::BURST_STATE_NOT_SET;
			if(!strcmp(szVarValue ,"SET")) burstState = hart7::hostapp::BURST_STATE_SET;
			if(!strcmp(szVarValue ,"TIMEOUT")) burstState = hart7::hostapp::BURST_STATE_TIMEOUT;
			if(!strcmp(szVarValue ,"ERROR")) burstState = hart7::hostapp::BURST_STATE_ERROR;
			if(!strcmp(szVarValue ,"OFF")) burstState = hart7::hostapp::BURST_STATE_OFF;
			LOG_INFO_APP("[LoadPublishers]: Load BURST_SET_STAT: " << szVarValue );
			list.insert(hart7::hostapp::PublisherInfo::BurstMessageStateMap::value_type(burstMessageNo,burstState));
		}
		strcpy(szVarName,szDefaultName);
	}

	return 0;
}

// get status for auto-detection process
// AUTO_DETECT = DONE | NOT_IMPLEMENT_105
int PublisherConf::LoadAutoDetectStatus(const char *pFileName, hart7::hostapp::DevicesAutodetectMAP_T& data)
{
	hart7::hostapp::AutodetectState autodetectState;
	data.clear();

	CIniParser parser;
	if (LoadConfigFile(pFileName, parser) < 0)
	{
		LOG_WARN_APP("[LoadPublishers]: invalid config file");
		data.clear();
		return -1;
	}

	hart7::hostapp::MAC mac;
	if (LoadFirstMAC(parser, mac) < 0)
	{
		LOG_WARN_APP("[LoadPublishers]: There is no MAC");
		data.clear();
		return -1;
	}

	if (LoadAutodetect(parser, autodetectState) == 0)
	{
		data[mac] = autodetectState;
		LOG_WARN_APP("[LoadPublishers]: no AUTO_DETECT variable for MAC=" << mac);
	}
	else
	{
		data[mac] = hart7::hostapp::AUTODETECT_NONE;
		LOG_DEBUG_APP("[LoadPublishers]: load MAC[" << mac << "]=" << (int)autodetectState);
	}

	while(LoadNextMAC(parser, mac) == 0)
	{
		if (LoadAutodetect(parser, autodetectState) == 0)
		{
			data[mac] = autodetectState;
			LOG_WARN_APP("[LoadPublishers]: no AUTO_DETECT variable for MAC=" << mac);
		}
		else
		{
			data[mac] = hart7::hostapp::AUTODETECT_NONE;
			LOG_DEBUG_APP("[LoadPublishers]: load MAC[" << mac << "]=" << (int)autodetectState);
		}
	}

	return 0;
}

int PublisherConf::LoadBurstNoTotalCmd105(CIniParser& parser , const hart7::hostapp::MAC& mac, int& burstNoTotalCmd105)
{
	char szLogVarString[256] = "";
	LOG_INFO_APP( "[LoadPublishers]: loading BURST_NO_TOTAL_CMD_105 ...");
	if (!parser.GetVarRawString(NULL, "BURST_NO_TOTAL_CMD_105", szLogVarString, sizeof(szLogVarString)))
		return -1;
	LOG_INFO_APP( "[LoadPublishers]: BURST_NO_TOTAL_CMD_105=" << szLogVarString << " read from conf.");
	sscanf( szLogVarString, "%d", &burstNoTotalCmd105);
	return 0;
}
//
int PublisherConf::SetBurstNoTotalCmd105(const char *pFileName/*[in]*/, const hart7::hostapp::MAC& mac,int BurstNoTotalCmd105)
{

	//skip if no filename
	if (strlen(pFileName) == 0)
		return -1;

	CIniParser parser;
	if (LoadConfigFileWrite(pFileName, parser) < 0)
	{
		LOG_WARN_APP("[LoadPublishers]: invalid config file");
		return -1;
	}

	char buf[10]="";
	sprintf(buf,"%d",BurstNoTotalCmd105);

	int retCode = parser.SetVarRawString(mac.ToString().c_str() , "BURST_NO_TOTAL_CMD_105", buf, 0, true);//0- error 1- success

	return retCode;
}

int PublisherConf::SetBurstNoTotalCmd105(CIniParser& parser, const hart7::hostapp::MAC& mac,int BurstNoTotalCmd105)
{
    char buf[10]="";
    sprintf(buf,"%d",BurstNoTotalCmd105);

    int retCode = parser.SetVarRawString(mac.ToString().c_str() , "BURST_NO_TOTAL_CMD_105", buf, 0, true);//0- error 1- success
    return (retCode == 1) ? 0 : -1;
}

//int PublisherConf::SetBurstState(const char *pFileName/*[in]*/, const hart7::hostapp::MAC& mac, uint8_t burstNo, hart7::hostapp::BurstState val, const char * message)
//{
//    return SetBurstState(pFileName, mac, burstNo, val, NULL);
//}

//
int PublisherConf::SetBurstState(const char *pFileName/*[in]*/, const hart7::hostapp::MAC& mac, uint8_t burstNo, hart7::hostapp::BurstState val, const char * message)
{
	if (strlen(pFileName) == 0) // skip if no filename
		return -1;

	CIniParser parser;
	if (LoadConfigFileWrite(pFileName, parser) < 0)
	{
		LOG_WARN_APP("[EditPublishers]: invalid config file");
		return -1;
	}
	char buf[20]="";
	char szVal[100]="";
	sprintf(buf,"%s%u","BURST_SET_STAT_",burstNo);

	switch(val)
	{
		case hart7::hostapp::BURST_STATE_NOT_SET:
			strcpy(szVal , "NOT_SET");
			break;
		case hart7::hostapp::BURST_STATE_SET:
			strcpy(szVal , "SET");
			break;
		case hart7::hostapp::BURST_STATE_TIMEOUT:
			strcpy(szVal , "TIMEOUT");
			break;
		case hart7::hostapp::BURST_STATE_ERROR:
		    if (message)
	            sprintf(szVal,"ERROR (%s)", message);
		    else
		        strcpy(szVal , "ERROR");
			break;
		case hart7::hostapp::BURST_STATE_OFF:
			strcpy(szVal, "OFF");
			break;
		default:
			LOG_WARN_APP("[EditPublishers]: invalid value for BurstState");
			return -1;
	}

	int retCode = parser.SetVarRawString(mac.ToString().c_str(), buf, szVal, 0, true);
	return retCode;
}

//
int PublisherConf::SetBurstState(CIniParser& parser, const hart7::hostapp::MAC& mac, const uint8_t burstNo, const hart7::hostapp::BurstState val)
{
    char buf[20]="";
    char szVal[20]="";
    sprintf(buf,"%s%u","BURST_SET_STAT_", burstNo);

    switch(val)
    {
        case hart7::hostapp::BURST_STATE_NOT_SET:
            strcpy(szVal , "NOT_SET");
            break;
        case hart7::hostapp::BURST_STATE_SET:
            strcpy(szVal , "SET");
            break;
        case hart7::hostapp::BURST_STATE_TIMEOUT:
            strcpy(szVal , "TIMEOUT");
            break;
        case hart7::hostapp::BURST_STATE_ERROR:
            strcpy(szVal , "ERROR");
            break;
        case hart7::hostapp::BURST_STATE_OFF:
            strcpy(szVal, "OFF");
            break;
        default:
            LOG_WARN_APP("[EditPublishers]: invalid value for BurstState");
            return -1;
    }

    int retCode = parser.SetVarRawString(mac.ToString().c_str(), buf, szVal, 0, true);
    return (retCode == 1) ? 0 : -1;
}

//
int PublisherConf::SetBurstStates(CIniParser& parser, const hart7::hostapp::MAC& mac, const hart7::hostapp::PublisherInfo::BurstMessageStateMap& list)
{
    hart7::hostapp::PublisherInfo::BurstMessageStateMap::const_iterator it = list.begin();
    for (; it != list.end(); ++it) {
        int result = SetBurstState(parser, mac, (*it).first, (*it).second);
        if (result < 0)
            return -1;
    }
    return 0;
}
int PublisherConf::SetBurstPublishPeriod(const char *pFileName/*[in]*/, const hart7::hostapp::MAC& mac, uint8_t burstNo, float val)
{
	//skip if no filename
	if (strlen(pFileName) == 0)
		return -1;

	CIniParser parser;
	if (LoadConfigFileWrite(pFileName, parser) < 0)
	{
		LOG_WARN_APP("[EditPublishers]: invalid config file");
		return -1;
	}
	char buf[30]="";
	char szVal[20]="";
	sprintf(buf,"BURST_SET_PUBLISH_PERIOD_%u",burstNo);

	sprintf(szVal, "%.3f", val );

	int retCode = parser.SetVarRawString(mac.ToString().c_str(), buf, szVal, 0, true);
	return retCode;
}
//
void PublisherConf::CheckBurstMessages(hart7::hostapp::MAC mac, hart7::hostapp::BurstMessageSetT& burstList_, hart7::hostapp::PublishChannelSetT& chList_, hart7::hostapp::TriggerSetT& triggerList_)
{
	bool found = false;
	hart7::hostapp::BurstMessageSetT::const_iterator bit;

	//we search for channels with wrong burst messages
	hart7::hostapp::PublishChannelSetT::const_iterator cit = chList_.begin();
	for( ; cit != chList_.end(); )
	{
		found = false;
		bit = burstList_.begin();
		for( ; bit != burstList_.end() && !found; ++bit)
		{
			if(bit->burstMessage == cit->burstMessage)
				found=true;
		}

		if(!found)
		{
			LOG_WARN_APP("Invalid Publiser Config for mac= "<<mac.ToString()<<" channel set for inexisting BurstMessage, channel deleted ");
			hart7::hostapp::PublishChannelSetT::const_iterator nextPos = cit;
			advance(nextPos,1);
			chList_.erase(cit);
			cit = nextPos;
			continue;
		}
		++cit;
	}

	//we search for trrigers with wrong burst messages
	hart7::hostapp::TriggerSetT::const_iterator tit = triggerList_.begin();
	for( ; tit != triggerList_.end(); )
	{
		found = false;
		bit = burstList_.begin();
		for( ; bit != burstList_.end() && !found; ++bit)
		{
			if(bit->burstMessage == tit->burstMessage)
				found=true;
		}

		if(!found)
		{
			LOG_WARN_APP("Invalid Publiser Config for mac= "<<mac<<" trigger set for inexisting BurstMessage: ");
			hart7::hostapp::TriggerSetT::const_iterator nextPos = tit;
			advance(nextPos,1);
			triggerList_.erase(tit);
			tit = nextPos;
			continue;
		}
		++tit;
	}

}

int PublisherConf::LoadAutodetect(CIniParser& parser, hart7::hostapp::AutodetectState& state)
{
    char szVarString[30] = "";

    if (!parser.GetVarRawString(NULL, "AUTO_DETECT", szVarString, sizeof(szVarString)))
        return -1;
    if (strcmp(szVarString, "DONE") == 0)
        state = hart7::hostapp::AUTODETECT_DONE;
    else if (strcmp(szVarString, "NOT_IMPLEMENT_105") == 0)
        state = hart7::hostapp::AUTODETECT_NOT_IMPLEMENT_105;
    else if (strcmp(szVarString, "DELETED") == 0)
        state = hart7::hostapp::AUTODETECT_DELETED;
    else
        return -1;

    LOG_INFO_APP( "[LoadAutodetect]: loading AUTO_DETECT = " << szVarString);
    return 0;
}

// Writes in Publisher file: AUTO_DETECT = [DONE|IN_PROGRESS], if device is autodetected
// RETURN: 0 = success, -1 = error
int PublisherConf::SetAutodetectState(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, hart7::hostapp::AutodetectState val)
{
    char buf[20]="";
    char szVal[20]="";
    sprintf(buf,"%s","AUTO_DETECT");

    switch(val)
    {
        case hart7::hostapp::AUTODETECT_DONE:
            strcpy(szVal , "DONE");
            break;
        case hart7::hostapp::AUTODETECT_IN_PROGRESS:
            strcpy(szVal , "IN_PROGRESS");
            break;
        case hart7::hostapp::AUTODETECT_NOT_IMPLEMENT_105:
            strcpy(szVal , "NOT_IMPLEMENT_105");
            break;
        case hart7::hostapp::AUTODETECT_NONE:
            LOG_WARN_APP("[SetAutodetectState]: AUTO_DETECT is NONE");
            return 0;
        default:
            LOG_WARN_APP("[SetAutodetectState]: invalid value for BurstState");
            return -1;
    }

    int retCode = parser.SetVarRawString(mac.ToString().c_str(), buf, szVal, 0, true);
//    LOG_INFO_APP("[SetAutodetectState] - AUTO_DETECT = " << szVal);

    return (retCode == 1) ? 0 : -1;
}

// Writes in Publishing file: BURST = [cmdNo, burstMessage, updatePeriod, maxUpdatePeriod]
// RETURN: 0 = success, -1 = error
int PublisherConf::SaveBurstMessage(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::BurstMessage &burst/*[in]*/, int pos)
{
    char buf[20]="";
    char szVal[256]="";
    sprintf(buf,"%s","BURST");
    hart7::hostapp::MAC macEmpty(0);

    if (burst.subDeviceMAC == macEmpty || burst.subDeviceMAC == mac)
    {
        sprintf(szVal,"%u, %u, %u, %u", burst.cmdNo, burst.burstMessage, (unsigned int)burst.updatePeriod, (unsigned int)burst.maxUpdatePeriod);
    }
    else
    {
        sprintf(szVal,"%u, %u, %u, %u, %s", burst.cmdNo, burst.burstMessage, (unsigned int)burst.updatePeriod, (unsigned int)burst.maxUpdatePeriod, burst.subDeviceMAC.ToString().c_str());
    }

    //    LOG_INFO_APP("[PublisherConf]:SaveBurstMessage - BURST = " << szVal);

    int retCode = parser.SetVarRawString(mac.ToString().c_str(), buf, szVal, pos, true);
    return (retCode == 1) ? 0 : -1;
}

// Writes in Publishing file: VARIABLE = [cmdNo, burstMessage, deviceVariable, name, deviceVariableSlot, classification, unitCode]
// RETURN: 0 = success, -1 = error
int PublisherConf::SaveChannel(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::PublishChannel &channel/*[in]*/, int pos)
{
    char buf[20]="";
    char szVal[256]="";
    sprintf(buf,"%s","VARIABLE");
    sprintf(szVal,"%u, %u, %u, %s, %u, %u, %u", channel.cmdNo, channel.burstMessage, channel.deviceVariable, channel.name.c_str(), channel.deviceVariableSlot, channel.classification, channel.unitCode);

//    LOG_INFO_APP("[PublisherConf]:SaveChannel - VARIABLE = " << szVal);

    int retCode = parser.SetVarRawString(mac.ToString().c_str(), buf, szVal, pos, true);
    return (retCode == 1) ? 0 : -1;
}

// Writes in Publishing file: TRIGGER = [cmdNo, burstMessage, modeSelection, classification, unitCode, triggerLevel]
// RETURN: 0 = success, -1 = error
int PublisherConf::SaveTrigger(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::Trigger &trigger/*[in]*/, int pos)
{
    char buf[20]="";
    char szVal[256]="";
    sprintf(buf,"%s","TRIGGER");
    sprintf(szVal,"%u, %u, %u, %u, %u, %f", trigger.cmdNo, trigger.burstMessage, trigger.modeSelection, trigger.classification, trigger.unitCode, trigger.triggerLevel);

//    LOG_INFO_APP("[PublisherConf]:SaveTrigger - TRIGGER = " << szVal);

    int retCode = parser.SetVarRawString(mac.ToString().c_str(), buf, szVal, pos, true);
    return (retCode == 1) ? 0 : -1;
}

// Writes burst message list in Publishing file
// RETURN: 0 = success, -1 = error
int PublisherConf::SaveBurstMessages(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::BurstMessageSetT &list/*[in]*/)
{
    hart7::hostapp::BurstMessageSetT::iterator it = list.begin();
    for (int i=0; it != list.end(); ++it) {
        int result = SaveBurstMessage(parser, mac, (*it), i++);
        if (result < 0)
            return -1;
    }
    return 0;
}

// Writes channel list in Publishing file
// RETURN: 0 = success, -1 = error
int PublisherConf::SaveChannels(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::PublishChannelSetT &list/*[in]*/)
{
    hart7::hostapp::PublishChannelSetT::iterator it = list.begin();
    for (int i=0; it != list.end(); ++it) {
        int result = SaveChannel(parser, mac, (*it), i++);
        if (result < 0)
            return -1;
    }
    return 0;
}

// Writes trigger list in Publishing file
// RETURN: 0 = success, -1 = error
int PublisherConf::SaveTriggers(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::TriggerSetT &list/*[in]*/)
{
    hart7::hostapp::TriggerSetT::iterator it = list.begin();
    for (int i=0; it != list.end(); ++it) {
        int result = SaveTrigger(parser, mac, (*it), i++);
        if (result < 0)
            return -1;
    }
    return 0;
}

// Writes Publisher info in Publishing file
// RETURN: 0 = success, -1 = error
int PublisherConf::SavePublisher(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::PublisherInfo &data /*[in]*/)
{
    if (SaveBurstMessages(parser, mac, data.burstMessageList) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - cannot save burst messages");
        return -1;
    }
    if (SaveChannels(parser, mac, data.channelList) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - cannot save channels");
        return -1;
    }
    if (SaveTriggers(parser, mac, data.triggersList) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - cannot save triggers");
        return -1;
    }
    if (SetBurstNoTotalCmd105(parser, mac, data.burstNoTotalCmd105) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - cannot set BURST_NO_TOTAL_CMD_105 variable");
        return -1;
    }
    if (SetAutodetectState(parser, mac, data.autodetectState) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - cannot set AUTO_DETECT variable");
        return -1;
    }
	return 0;
}

// Writes Publisher info in Publishing file
// RETURN: 0 = success, -1 = error
int PublisherConf::SavePublisher(const char *pFileName/*[in]*/, hart7::hostapp::MAC& mac, const hart7::hostapp::PublisherInfo &data /*[in]*/)
{
    CIniParser parser;
    if (strlen(pFileName) == 0 || LoadConfigFileWrite(pFileName, parser) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - Invalid config file");
        return -1;
    }

    if (SaveBurstMessages(parser, mac, data.burstMessageList) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - cannot save burst messages");
        return -1;
    }
    if (SaveChannels(parser, mac, data.channelList) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - cannot save channels");
        return -1;
    }
    if (SaveTriggers(parser, mac, data.triggersList) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - cannot save triggers");
        return -1;
    }
    if (SetAutodetectState(parser, mac, data.autodetectState) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - cannot set AUTO_DETECT variable");
        return -1;
    }
    if (SetBurstNoTotalCmd105(parser, mac, data.burstNoTotalCmd105) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - cannot set BURST_NO_TOTAL_CMD_105 variable");
        return -1;
    }
    if (SetBurstStates(parser, mac, data.burstMessagesState) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - cannot set burst states");
        return -1;
    }
    return 0;
}

// Writes Publisher info in Publishing file
// RETURN: 0 = success, -1 = error
int PublisherConf::SavePublisherNotImpl105(const char *pFileName/*[in]*/, hart7::hostapp::MAC& mac)
{
    CIniParser parser;
    if (strlen(pFileName) == 0 || LoadConfigFileWrite(pFileName, parser) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - Invalid config file");
        return -1;
    }

    if (SetAutodetectState(parser, mac, hart7::hostapp::AUTODETECT_NOT_IMPLEMENT_105) < 0)
    {
        LOG_WARN_APP("[SavePublisher] - cannot set AUTO_DETECT variable");
        return -1;
    }
    return 0;
}

int PublisherConf::DeletePublisher(const char *pFileName/*[in]*/, const hart7::hostapp::MAC& mac)
{
    CIniParser parser;
    if (strlen(pFileName) == 0 || LoadConfigFileWrite(pFileName, parser) < 0)
    {
        LOG_WARN_APP("[DeletePublisher] - Invalid config file");
        return -1;
    }

    int retCode = parser.DeleteGroup(mac.ToString().c_str(), true);
    return (retCode) ? 0 : -1;
}
