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

#ifndef PUBLISHERINFO_H_
#define PUBLISHERINFO_H_

#include <set>
#include <map>
#include <string>
#include <list>

#include <Shared/MicroSec.h>
#include <WHartHost/model/MAC.h>

namespace hart7 {
namespace hostapp {

enum BurstState
{
	BURST_STATE_NOT_SET = 0,
	BURST_STATE_SET,
    BURST_STATE_OFF,
	BURST_STATE_TIMEOUT,
	BURST_STATE_ERROR
};

enum SetPublisherState
{
    SETPUBLISHER_STATE_NONE = 0,
    SETPUBLISHER_STATE_READ_BURSTCONFIG,
    SETPUBLISHER_STATE_TURNOFF_BURST,
    SETPUBLISHER_STATE_GET_SUBDEVICEINDEX,
    SETPUBLISHER_STATE_CONFIGURE_BURST,
    SETPUBLISHER_STATE_DONE,
    SETPUBLISHER_STATE_UNDEFINED
};

enum SetPublisherError
{
    SETPUBLISHER_ERROR_NONE = 0,
    SETPUBLISHER_ERROR_READ_BURSTCONFIG,
    SETPUBLISHER_ERROR_READ_BURSTCONFIG_CMD105_NOT_IMPL,
    SETPUBLISHER_ERROR_TURNOFF_BURST,
    SETPUBLISHER_ERROR_TURNOFF_BURST_CMD109_NOT_IMPL,
    SETPUBLISHER_ERROR_GET_SUBDEVICEINDEX,
    SETPUBLISHER_ERROR_CONFIGBURST,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD102_NOT_IMPL,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD103_NOT_IMPL,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD104_NOT_IMPL,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD106_NOT_IMPL,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD107_NOT_IMPL,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD108_NOT_IMPL,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD109_NOT_IMPL,
    SETPUBLISHER_ERROR_CONFIGBURST_BUSY,
    SETPUBLISHER_ERROR_CONFIGBURST_MAP_SUBDEVICE,
    SETPUBLISHER_ERROR_CONFIGBURST_WRITE_UPDATE_PERIOD,
    SETPUBLISHER_ERROR_CONFIGBURST_WRITE_TRIGGER,
    SETPUBLISHER_ERROR_CONFIGBURST_WRITE_VARIABLES,
    SETPUBLISHER_ERROR_CONFIGBURST_WRITE_COMMAND_NO,
    SETPUBLISHER_ERROR_CONFIGBURST_SET_BURST_ON,
    SETPUBLISHER_ERROR_FATAL,
    SETPUBLISHER_ERROR_UNDEFINED
};

enum AutodetectState
{
    AUTODETECT_NONE = 0,
    AUTODETECT_IN_PROGRESS = 1,
    AUTODETECT_DELAYED = 2,
    AUTODETECT_NOT_IMPLEMENT_105 = 3,
    AUTODETECT_DONE = 4,
    AUTODETECT_DELETED = 5
};

typedef std::map<MAC, hart7::hostapp::AutodetectState> DevicesAutodetectMAP_T;

struct BurstMessage
{
	unsigned short	cmdNo;
	unsigned char 	burstMessage;
	double 			updatePeriod;
	double 			maxUpdatePeriod;
	MAC             subDeviceMAC;

	bool operator < (const BurstMessage& other) const
	{
		return (((cmdNo << 16) | burstMessage) < ((other.cmdNo << 16) | other.burstMessage));
	}
	
	bool operator == (const BurstMessage& other)  const
	{
        return (((cmdNo << 16) | burstMessage) == ((other.cmdNo << 16) | other.burstMessage));
	}
};

struct PublishChannel
{
    unsigned short  cmdNo;
    unsigned char   deviceVariableSlot;
    unsigned char   deviceVariable;
    unsigned char   classification;
    unsigned char   unitCode;
    std::string name;
    unsigned char burstMessage;
    mutable int     channelID;  //for optimization

    bool operator < (const PublishChannel& other) const
    {
        return (((cmdNo << 16) | (burstMessage << 8) | deviceVariableSlot) < ((other.cmdNo << 16) | (other.burstMessage << 8) | other.deviceVariableSlot));
    }

    bool operator == (const PublishChannel& other)  const
    {
        return (burstMessage == other.burstMessage) && (cmdNo == other.cmdNo) && (deviceVariableSlot == other.deviceVariableSlot);
    }
};

struct Trigger
{	
	unsigned short	cmdNo;
	unsigned char 	burstMessage;
	unsigned char 	modeSelection;
	unsigned char 	classification;
	unsigned char 	unitCode;
	float			triggerLevel;

	bool operator < (const Trigger& other) const
	{
		return (((cmdNo << 16) | burstMessage) < ((other.cmdNo << 16) | other.burstMessage));
	}
	
	bool operator == (const Trigger& other)  const
	{
		return (((cmdNo << 16) | burstMessage) == ((other.cmdNo << 16) | other.burstMessage));
	}
};

typedef std::set<BurstMessage>      BurstMessageSetT;
typedef std::set<PublishChannel>    PublishChannelSetT;
typedef std::set<Trigger>           TriggerSetT;

extern PublishChannelSetT::const_iterator FindFirstChannel(const PublishChannelSetT& p_rSet, uint16_t cmdNo);
extern PublishChannelSetT::const_iterator FindChannel(const PublishChannelSetT& p_rSet, int channelID);

struct PublisherInfo
{

    typedef std::map<uint8_t, BurstState> BurstMessageStateMap;

	const static int	g_nUnsetBurstNoTotalCmd105 = -1;
	int					burstNoTotalCmd105;
	// state for DiscoveryBurstConfig task
    AutodetectState 	autodetectState;

	/*burst messages */
	CMicroSec lastConfigurationTime;
	int noOfBurstMessagesThatFailedToConfigure; /*with the state either timeout or not set*/
	BurstMessageStateMap burstMessagesState;

	bool				hasNotification;	//for optimization
	int					burstSetConfigShortDelayRetries;

	BurstMessageSetT	burstMessageList;
	PublishChannelSetT	channelList;
	TriggerSetT			triggersList;
		
	bool				reconfigBurst;
	bool 				flushDelayedResponses; 

	PublisherInfo() :
		burstNoTotalCmd105(0),
        autodetectState(AUTODETECT_NONE),
		noOfBurstMessagesThatFailedToConfigure(0),
		hasNotification(false),
		burstSetConfigShortDelayRetries(0),
		reconfigBurst(false),
		flushDelayedResponses(false)
 	{ }

	PublisherInfo(BurstMessageSetT& burstMessageList_, PublishChannelSetT& channelList_, TriggerSetT& triggersList_,
				  int burstNoTotalCmd105_, BurstMessageStateMap& burstMessagesState_ ) :
		burstNoTotalCmd105(burstNoTotalCmd105_),
        autodetectState(AUTODETECT_NONE),
		noOfBurstMessagesThatFailedToConfigure(0),
		burstMessagesState(burstMessagesState_),
		hasNotification(false),
		burstMessageList(burstMessageList_),
		channelList(channelList_),
		triggersList(triggersList_),
		reconfigBurst(false),
		flushDelayedResponses(false)
	{ }

};

struct BurstError
{
    int                 burstNo;
    SetPublisherError   setPublisherError;
    std::string         setPublisherMessage;

    BurstError(int burstNo_, SetPublisherError setPublisherError_, std::string setPublisherMessage_) :
        burstNo(burstNo_),
        setPublisherError(setPublisherError_),
        setPublisherMessage(setPublisherMessage_)
    {}
};

struct PublisherState
{
    typedef std::list<BurstError> BurstErrorLST_T;

    SetPublisherState   setPublisherState;
    SetPublisherError   setPublisherError;
    std::string         setPublisherMessage;
    BurstErrorLST_T     burstErrorList;
    bool                stateChanged;

    PublisherState() :
        setPublisherState(SETPUBLISHER_STATE_NONE),
        setPublisherError(SETPUBLISHER_ERROR_NONE),
        setPublisherMessage(""),
        stateChanged(false)
    { }

    PublisherState(SetPublisherState setPublisherState_, SetPublisherError setPublisherError_, std::string setPublisherMessage_, bool stateChanged_) :
        setPublisherState(setPublisherState_),
        setPublisherError(setPublisherError_),
        setPublisherMessage(setPublisherMessage_),
        stateChanged(stateChanged_)
    { }

    void InitPublisherState()
    {
        setPublisherState = SETPUBLISHER_STATE_NONE;
        setPublisherError = SETPUBLISHER_ERROR_NONE;
        setPublisherMessage = "";
        burstErrorList.clear();
        stateChanged = false;
    }
};

typedef std::map<MAC, PublisherState> PublisherStateMAP_T;

extern std::string GetPublisherInfoString(PublisherInfo& publisherInfo);

extern std::string GetSetPublishStateText(SetPublisherState p_eState);
extern std::string GetSetPublishErrorText(SetPublisherError p_eError);
extern std::string GetPublisherErrorList(SetPublisherError error, PublisherState::BurstErrorLST_T& burstErrorList);
extern std::string GetPublisherMessageList(std::string message, PublisherState::BurstErrorLST_T& burstErrorList);

typedef std::map<MAC, PublisherInfo> PublisherInfoMAP_T;

std::ostream& operator<<(std::ostream& p_rOut, const BurstState& p_rBurstState);


} //namespace hostapp
} //namespace hart7


#endif 
