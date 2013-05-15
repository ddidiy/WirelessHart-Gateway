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

#ifndef PUBLISHERCONF_H_
#define PUBLISHERCONF_H_

#include <WHartHost/model/Device.h>
#include <WHartHost/model/PublisherInfo.h>


#include <list>

const double MAX_UPDATE_PERIOD = 3600;

class CIniParser;
class PublisherConf
{

//file
private:
	int LoadConfigFile(const char *pFileName/*[in]*/, CIniParser& parser /*[in/out]*/);
	int LoadConfigFileWrite(const char *pFileName/*[in]*/, CIniParser& parser /*[in/out]*/);

//mac
private:
	int LoadFirstMAC(CIniParser& parser /*[in]*/, hart7::hostapp::MAC& mac/*[in/out]*/);
	int LoadNextMAC(CIniParser& parser /*[in]*/, hart7::hostapp::MAC& mac/*[in/out]*/);

//burst message
private:
	int LoadBurstMessage(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, int burstMessageID, hart7::hostapp::BurstMessage& burstMessage/*[in/out]*/);
	int LoadBurstMessages(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, hart7::hostapp::BurstMessageSetT& list/*[in/out]*/, std::list<double>& burstUpdatePeriods/*[in/out]*/);
    int SaveBurstMessage(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::BurstMessage& burst/*[in]*/, int pos);
    int SaveBurstMessages(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::BurstMessageSetT& list/*[in]*/);

//burst message stat
	int LoadBurstMessageStats(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, hart7::hostapp::PublisherInfo::BurstMessageStateMap& list/*[in/out]*/);

 
//channel
private:
	int LoadChannel(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, int channelID, hart7::hostapp::PublishChannel& channel/*[in/out]*/);
	int LoadChannels( CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, hart7::hostapp::PublishChannelSetT& list/*[in/out]*/);
    int SaveChannel(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::PublishChannel& channel/*[in]*/, int pos);
    int SaveChannels(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::PublishChannelSetT& list/*[in]*/);

//trigger
private:
	int LoadTrigger(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, int triggerID, hart7::hostapp::Trigger& trigger/*[in/out]*/);
	int LoadTriggers(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, hart7::hostapp::TriggerSetT& list/*[in/out]*/);
    int SaveTrigger(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::Trigger& trigger/*[in/out]*/, int pos);
    int SaveTriggers(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::TriggerSetT& list/*[in/out]*/);

private:
	int LoadBurstNoTotalCmd105(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, int& burstNoTotalCmd105);

private:
	int SavePublisher(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac, const hart7::hostapp::PublisherInfo& data /*[in]*/);

private:
	void CheckBurstMessages(hart7::hostapp::MAC mac, hart7::hostapp::BurstMessageSetT& burstList_, hart7::hostapp::PublishChannelSetT& chList_, hart7::hostapp::TriggerSetT& triggerList_);
	
public:
	// it loads data from file until an invalid data it is read;
	// ret values:
	//	0 - ok
	// -1 - invalid config file (if no valid data has been read)
	int LoadPublishers(const char *pFileName/*[in]*/, hart7::hostapp::PublisherInfoMAP_T& data /*[in/out]*/, std::list<double>& burstUpdatePeriods/*[out]*/);
	int LoadAutoDetectStatus(const char *pFileName, hart7::hostapp::DevicesAutodetectMAP_T& data);
	int LoadAutodetect(CIniParser& parser, hart7::hostapp::AutodetectState& state);

    int SavePublisher(const char *pFileName/*[in]*/, hart7::hostapp::MAC& mac, const hart7::hostapp::PublisherInfo& data /*[in]*/);
    int SavePublisherNotImpl105(const char *pFileName/*[in]*/, hart7::hostapp::MAC& mac);
	int SavePublishers(const char *pFileName/*[in]*/, hart7::hostapp::PublisherInfoMAP_T& data/*[in]*/);
    int DeletePublisher(const char *pFileName/*[in]*/, const hart7::hostapp::MAC& mac);

	//sets data
	int SetBurstNoTotalCmd105(const char *pFileName/*[in]*/, const hart7::hostapp::MAC& mac,int BurstNoTotalCmd105);
    int SetBurstNoTotalCmd105(CIniParser& parser, const hart7::hostapp::MAC& mac,int BurstNoTotalCmd105);
    int SetBurstState(const char *pFileName/*[in]*/, const hart7::hostapp::MAC& mac, uint8_t burstNo, hart7::hostapp::BurstState val, const char * message);
    int SetBurstState(CIniParser& parser, const hart7::hostapp::MAC& mac, const uint8_t burstNo, const hart7::hostapp::BurstState val);
    int SetBurstStates(CIniParser& parser, const hart7::hostapp::MAC& mac, const hart7::hostapp::PublisherInfo::BurstMessageStateMap& list);
	int SetBurstPublishPeriod(const char *pFileName/*[in]*/, const hart7::hostapp::MAC& mac, uint8_t burstNo, float val);

private:
    int SetAutodetectState(CIniParser& parser /*[in]*/, const hart7::hostapp::MAC& mac/*[in]*/, const hart7::hostapp::AutodetectState val/*[in]*/);
};

#endif
