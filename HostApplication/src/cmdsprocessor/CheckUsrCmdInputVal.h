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


#ifndef CHECKUSRCMDINPUTVAL_H_
#define CHECKUSRCMDINPUTVAL_H_

#include <nlib/log.h>
#include <nlib/exception.h>

#include <WHartHost/model/DBCommand.h>
#include <WHartHost/model/PublisherInfo.h>
#include <WHartHost/database/DevicesManager.h>
#include <WHartHost/cmdsprocessor/ProcessorExceptions.h>


#include <map>

namespace hart7 {
namespace hostapp {


class CheckUsrCmdInputVal
{
private:
	CheckUsrCmdInputVal();
	CheckUsrCmdInputVal(const CheckUsrCmdInputVal&);

public:
	CheckUsrCmdInputVal(DevicesManager &devices_):devices(devices_){}
private:
	DevicesManager& devices;

//devices
private:
	DevicePtr GetRegisteredDevice(int deviceID);

//command's parameters
private:
	bool GetOptionalParameterValue(DBCommandParameter::ParameterCode parameterCode, const DBCommand::ParametersList& list, int &val);
	int GetParameterValueAsInt(DBCommandParameter::ParameterCode parameterCode, const DBCommand::ParametersList& list, 
		const std::string& parameterName);
	std::string GetParameterValueAsString(DBCommandParameter::ParameterCode parameterCode, 
		const DBCommand::ParametersList& list, const std::string& parameterName);
	void GetParamValuesAsInt(DBCommandParameter::ParameterCode parameterCode, 
		const DBCommand::ParametersList& allParameterslist, std::list<int>& list /*[in/out]*/);
	

//input checking
public:
	void TestUsrParams_ccGetTopology(DBCommand& command/*[in]*/);
	void TestUsrParams_ccBurstConfiguration(DBCommand& command/*[in]*/, DevicePtr &dev/*[out]*/, std::string& p_rPubConfFileName);
	void TestUsrParams_ccNotifSubscribe(DBCommand& command/*[in]*/, DevicePtr &dev/*[out]*/);
	void TestUsrParams_ccNotifUnSubscribe(DBCommand& command/*[in]*/);
	void TestUsrParams_ccTopologyNotify(DBCommand& command/*[in]*/);
	void TestUsrParams_ccGeneralCmd(DBCommand& command/*[in]*/, int &ValueCmdNo/*[out]*/, 
		std::string &ValueDataBytes/*[out]*/, bool &bypassIOCache);
	void TestUsrParams_ccReadValue(DBCommand& command/*[in]*/, PublishChannel & channel/*[out]*/, bool &bypassIOCache);
	void TestUsrParams_ccDevHealthReport(DBCommand& command/*[in]*/, std::list<std::pair<int, MAC> > &list/*[in/out]*/);
	void TestUsrParams_ccNeighborHealthReport(DBCommand& command/*[in]*/, std::list<std::pair<int, MAC> >  &list/*[in/out]*/);
	void TestUsrParams_ccDeviceScheduleLinkReport(DBCommand& command/*[in]*/, std::list<std::pair<int, MAC> > &list/*[in/out]*/);
	void TestUsrParams_ccReadBurstConfig(DBCommand& command/*[in]*/, DevicePtr &dev/*[out]*/);
};


}
}


#endif

