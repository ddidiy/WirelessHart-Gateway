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

/*
 * DevicesManager.cpp
 *
 *  Created on: May 25, 2009
 *      Author: Andy
 */
#include "DevicesManager.h"

#include <boost/bind.hpp>
#include "AllNetworkManagerCommands.h"
#include "NodeVisibleVerifier.h"
#include "Model/Operations/NivisCustom64765Operation.h"
#include "Model/Operations/Join/WriteNetworkIDOperation.h"
#include "Model/Topology/TopologyTypes.h"
#include <ApplicationLayer/Model/NivisSpecificCommands.h>
#include "operations/WHEngineOperationsVisitor.h"

#include "Model/Topology/Node.h"
#include "../util/ManagerUtils.h"
#include "SMState/SMStateLog.h"
#include <hart7/util/NMLog.h>
#include <boost/bind.hpp>
#include "script/ScriptManager.h"

namespace hart7 {
namespace nmanager {

DevicesManager::DevicesManager(IRequestSend& requestSend_, CommonData& commonData_,
                               operations::WHOperationQueue& operationsQueue_) :
    requestSend(requestSend_), commonData(commonData_), operationsQueue(operationsQueue_),
                changeNotification(commonData, requestSend_), evaluateRoute(commonData, operationsQueue),
                alarmsProcessor(commonData_, operationsQueue), reportsProcessor(commonData_, operationsQueue),
                scriptManager(commonData_.settings.scriptFileName, *this), evaluateClkSrc(commonData, operationsQueue),
                agressiveAdvFlow(requestSend_, commonData_, operationsQueue_)
{
    evaluateRoute.RouteEvaluated = boost::bind(&DevicesManager::CheckDevicesReadyForOperational, this);
    commonData.networkEngine.registerMarkAsChangedCallback(boost::bind(&DevicesManager::markAsChangedFromOperation,
                                                                       this, _1, _2));

    this->nodeAlarmVerifier.reset(new NodeAlarmVerifier(requestSend_, commonData_, operationsQueue_));
    commonData.alarmDispatcher.registerGeneralAlarm788Listener(nodeAlarmVerifier);

    AlarmListenerPointer devManAlarmPointer;
    devManAlarmPointer.reset(&alarmsProcessor);
    commonData.alarmDispatcher.registerGeneralAlarm788Listener(devManAlarmPointer);

    ReportListenerPointer devMan(&reportsProcessor);
    commonData.reportDispatcher.registerReportListener(devMan);

    flagMarkedForReloadingConfiguration = false;
}

void DevicesManager::stopJoin(const stack::WHartAddress & source)
{
    std::map<stack::WHartAddress, JoinFlowHandler>::iterator it = joinHandlers.find(source);
    if (it != joinHandlers.end())
    {
        LOG_WARN("The device " << source << " is already in a join process to parent "
                    << ToStr(it->second.getParentAddress32()) << ". Reset old join ...");
        it->second.StopFlow();
        // !!! iterator it is already deleted; do not use it anymore
    }
    it = joinHandlers.find(source);
    if (it != joinHandlers.end())
    {
        joinHandlers.erase(it);
    }

    std::map<stack::WHartAddress, QuarantineToOperationalFlowHandler>::iterator itQ;
    itQ = quarantineHandlers.find(source);
    if (itQ != quarantineHandlers.end())
    {
        LOG_WARN("The device " << source << " is already in a join process (Q) to parent " <<
                    ToStr(it->second.getParentAddress32()) << ". Reset old join ...");
        itQ->second.StopFlow();
        // !!! iterator it is already deleted; do not use it anymore
    }
}

void DevicesManager::DeviceRegistrationRequest(const stack::WHartAddress & source,
                                               const stack::WHartCommandList & commands)
{

    stopJoin(source);

    joinHandlers.insert(std::make_pair(source, JoinFlowHandler(*this, requestSend, commonData, operationsQueue)));
    JoinFlowHandler & joinHandler = joinHandlers.find(source)->second;
    joinHandler.TimeOut = 180;
    joinHandler.JoinFinished = boost::bind(&DevicesManager::JoinFinished, this, _1, _2);
    joinHandler.ProcessJoinRequest(source, commands/*, joinAttempts[source] > commonData.settings.maxJoinAttemptsBeforeOverride*/);

    if (joinHandler.device.isBackbone)
    {
        stopAdvLinkFlowOnTR(joinHandler.device.nickname);
    }
}

void DevicesManager::markAsChangedFromOperation(IEngineOperation& operation, uint32_t peerAddress)
{
    //DEPRECATED
}

void DevicesManager::markAsChanged(uint16_t address, uint16_t commandId)
{
    //DEPRECATED
}

void DevicesManager::JoinFinished(const HartDevice & device_, bool status)
{
    LOG_WARN("JoinFinished dev=" << std::hex << device_.nickname << ", status=" << status);
    HartDevice device(device_);
    WHartAddress source(device.longAddress);
    std::map<stack::WHartAddress, JoinFlowHandler>::iterator it = joinHandlers.find(source);
    if (it == joinHandlers.end())
    {
        LOG_WARN("Unexpected finished join! Device with address=" << source
                    << " is not currently in a join process.");
        return;
    }

    if (status)
    {
        if (device.isBackbone)
        {
            SendDeviceToOperational(device);
        }
        else if (!device.isGateway)
        {
            //wait for its evaluate route to complete
            LOG_DEBUG("Device with address=" << source << " is waiting to exit quarantine.");
            QuarantineHandlersMap::iterator it = quarantineHandlers.find(device.nickname);
            if (it != quarantineHandlers.end())
            {
                it->second.StopFlow();
                // do not remove yet, allow it to stop gracefully until the new join gets there.
            }

            quarantinedDevices.insert(device);
        }

        LOG_INFO("The device " << source << " has successfully joined the network.");

        commonData.reportDispatcher.dispatchReport787(
                                                      source, //
                                                      commonData.networkEngine.getDevicesTable().getDevice(
                                                                                                           device.nickname).capabilities.neighborSignalLevels);
    }
    else
    {
        LOG_INFO("The device " << source << " has failed to join the network.");
    }
}

void DevicesManager::CheckDevicesReadyForOperational()
{
    LOG_DEBUG("CheckDevicesReadyForOperational()");
    for (std::set<HartDevice>::iterator it = quarantinedDevices.begin(); it != quarantinedDevices.end();)
    {
        Address32 deviceAddress32 = it->nickname;
        LOG_DEBUG("Checking for device with address=" << ToStr(deviceAddress32));

        if (!commonData.networkEngine.existsDevice(deviceAddress32))
        {
            quarantinedDevices.erase(it++);
        }
        else if (commonData.networkEngine.canSendAdvertise(deviceAddress32))
        {
            HartDevice tempDevice = *it;
            quarantinedDevices.erase(it);
            SendDeviceToOperational(tempDevice);

            break; //only one device should get out of quarantine on one evaluated route
        }
        else
        {
            it++;
        }
    }
}

void DevicesManager::SendDeviceToOperational(const HartDevice & device)
{
    LOG_DEBUG("Sending device=" << WHartAddress(device.longAddress) << " to operational...");
    if (quarantineHandlers.find(device.nickname) != quarantineHandlers.end())
    {
        LOG_WARN("There is already a Q2O handler active. Removing old one...");
        quarantineHandlers.erase(device.nickname);
    }
    QuarantineToOperationalFlowHandler & q2oHandler = (quarantineHandlers.insert( std::make_pair(
                                                                      device.nickname,
                                                                      QuarantineToOperationalFlowHandler(device,
                                                                                                         commonData,
                                                                                                         operationsQueue))).first)->second;
    q2oHandler.OnFinished = boost::bind(&DevicesManager::OperationalFinished, this, _1, _2);
    q2oHandler.Start();
}

void DevicesManager::OperationalFinished(const HartDevice & device, bool status)
{
    if (status)
    {
        LOG_INFO("Device with address " << WHartAddress(device.longAddress)
                    << " has successfully entered OPERATIONAL state.");
    }
    else
    {
        LOG_INFO("Device with address " << WHartAddress(device.longAddress)
                    << " has failed to enter OPERATIONAL state.");
        // set the device as removed.
        NE::Model::NetworkEngine::instance().setRemoveStatus(device.nickname);
    }
    std::map<stack::WHartAddress, QuarantineToOperationalFlowHandler>::iterator it =
                quarantineHandlers.find(device.nickname);
    if (it != quarantineHandlers.end())
    {
        quarantineHandlers.erase(it);
    }
}

void DevicesManager::PeriodicTask()
{
    if (flagMarkedForReloadingConfiguration == true)
    {
        flagMarkedForReloadingConfiguration = false;

        if (managerReloadConfigFilesHandler)
        {
            managerReloadConfigFilesHandler();
        }
        LOG_DEBUG("Executing scripts.");
        scriptManager.ExecuteScripts();
    }

    evaluateRoute.EvaluateRoutes();

    evaluateClkSrc.evaluateClkSrc();

    agressiveAdvFlow.TimePassed(3 * 1000);

    changeNotification.notifyDevices();

    operationsQueue.updateDevicesJoinPriority();

    if (operationsQueue.saveTopoloyToDisk == true)
    {
    }

    checkTransceiverAdvertiseBandwidth();

    checkVisibleEdges();

    compactLinks();

    if (LogAllInfo)
    {
        SMState::SMStateLog::logErrAllInfo("User induced (SIGUSR2 sent to NM)");
        hart7::util::NMLog::logDeviceHistory();
        operationsQueue.logOperations();
        LogAllInfo = false;
    }

    // make sure device table and network topology logs are flushed
    SMState::SMStateLog::wakeUpBufferedLogs();
}

void DevicesManager::compactLinks()
{
    //DEPRECATED
}

void DevicesManager::checkTransceiverAdvertiseBandwidth()
{
    LOG_TRACE("checkTransceiverAdvertiseBandwidth()");

    if (flowTr)
    {
        LOG_TRACE("checkTransceiverAdvertiseBandwidth() k3t already exists a flow for adding a new adv link on TR! Returns...");
        return;
    }

    // find the TR node (TODO : should be updated later when there'll be > 1 TR's)
    NodeMap& nodes = commonData.networkEngine.getSubnetTopology().getSubnetNodes();
    for (NodeMap::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        if (it->second.getNodeType() == NE::Model::Topology::NodeType::BACKBONE)
        {
            if (commonData.networkEngine.getDevicesTable().getDeviceStatus(it->first)
                        != NE::Model::DeviceStatus::OPERATIONAL)
            {
                return;
            }

            uint32_t totalBandwidth = 0;
            uint32_t allocationHandler = 0;

            NodeTdma& trNodeTdma = commonData.networkEngine.getSubnetTdma().getNodeTdma(it->first);
            for (int starIndex = 0; starIndex < MAX_STAR_INDEX; ++starIndex)
            {
                TimeslotAllocations::iterator itTimeSlot = trNodeTdma.timeslotAllocationsArray[starIndex].begin();
                for (; itTimeSlot != trNodeTdma.timeslotAllocationsArray[starIndex].end(); ++itTimeSlot)
                {
                    if (itTimeSlot->isTransmission() //
                                && (itTimeSlot->status == Status::ACTIVE //
                                            || itTimeSlot->status == Status::PENDING) //
                                && (itTimeSlot->getLinkType() == NE::Model::Tdma::LinkTypes::NORMAL))
                    {
                        if (itTimeSlot->peerAddress == 0xFFFF)
                        {
                            // TODO: remember the data for deallocation if it should
                            allocationHandler = itTimeSlot->allocationHandler;
                        }
                        totalBandwidth += (uint16_t) itTimeSlot->publishPeriod;
                    }
                }
            }

            LOG_TRACE("checkTransceiverAdvertiseBandwidth() k3t : totalBandwith= " << (int) totalBandwidth);

            try
            {
                if (totalBandwidth <= (uint32_t) commonData.settings.advPublishPeriod
                            * (commonData.settings.transceiverAdvertiseLinksNo - 1))
                {
                    LOG_TRACE("checkTransceiverAdvertiseBandwidth() k3t : ADD new adv link!");

                    flowTr.reset(new NewAdvertiseLinkOnTrFlow(requestSend, commonData, operationsQueue));
                    flowTr->newAdvertiseLinkOnTrFlowFinished
                                = boost::bind(&DevicesManager::newAdvertiseLinkOnTrFlowFinished, this, _1);
                    if ((flowTr->sendNewAdvLink(it->first) == false) //
                                || (flowTr->advertiseEngineOperations->operations.size() == 0))
                    {
                        flowTr.reset();
                    }
                }
                else if (totalBandwidth >= (uint32_t) commonData.settings.advPublishPeriod
                            * (commonData.settings.transceiverAdvertiseLinksNo + 1))
                {
                    if (allocationHandler != 0)
                    {
                        LOG_TRACE("checkTransceiverAdvertiseBandwidth() k3t : REMOVE ADV link allocationHandler = "
                                    << std::hex << (int) allocationHandler);

                        flowTr.reset(new NewAdvertiseLinkOnTrFlow(requestSend, commonData, operationsQueue));
                        flowTr->newAdvertiseLinkOnTrFlowFinished
                                    = boost::bind(&DevicesManager::newAdvertiseLinkOnTrFlowFinished, this, _1);
                        flowTr->sendRemoveAdvLink(it->first, allocationHandler);

                        if (flowTr->advertiseEngineOperations->operations.size() == 0)
                        {
                            flowTr.reset();
                        }
                    }
                    else
                    {
                        LOG_TRACE("checkTransceiverAdvertiseBandwidth() k3t : we should REMOVE ADV link but there is none!");
                    }
                }
            }
            catch (std::exception& ex)
            {
                flowTr.reset();
                LOG_ERROR(ex.what());
            }

            return;
        }

        LOG_TRACE("There is no TR joined yet!");

    }
}

void DevicesManager::newAdvertiseLinkOnTrFlowFinished(bool errorOccurred)
{
    LOG_TRACE("newAdvertiseLinkOnTrFlowFinished() : k3t");
    this->flowTr.reset();
}

void DevicesManager::stopAdvLinkFlowOnTR(uint16_t device)
{
    // only one BBR, so do not check address
    if (flowTr)
    {
        flowTr->StopFlow();
        flowTr.reset();
    }
}

void DevicesManager::checkVisibleEdges()
{
    if (commonData.settings.checkEdgeVisibility == false)
    {
        return;
    }

    LOG_TRACE("checkVisibleEdges() visibleEdgeHandlers.size: " << visibleEdgeHandlers.size());

    //1. check the existing flows to see if they passed the probation period and they didn't receive any alarms
    std::map<VisibleEdge, VisibleEdgeFlowPointer>::iterator it;
    it = visibleEdgeHandlers.begin();
    for (; it != visibleEdgeHandlers.end();)
    {
        LOG_TRACE("checkVisibleEdges(): " << it->second->toString());
        if (it->second->hasTimeOuted())
        {
            LOG_INFO("Visible Edge flow time outed. " << it->second->toString());
            commonData.alarmDispatcher.removeAlarm788Listener(it->second, it->first.first, it->first.second);
            it->second->removeTestLinks();
        }
        else if (it->second->hasRemovedTestLinks())
        {
            LOG_INFO("Visible Edge flow removed test links. " << it->second->toString());
            bool finishedOkBothWays = false;
            commonData.nodeVisibleVerifier.endProcessingVisibleEdge(it->first, finishedOkBothWays);

            if (finishedOkBothWays)
            {
                commonData.nodeVisibleVerifier.removeFinishedVisibleEdge(it->first);
                it->second->doFinalJob(true);
            }
            else
            {
                // create reverse visible edge flow (only here should be created ...)
                VisibleEdge ve = it->first;
                VisibleEdge reverseVe = std::make_pair(ve.second, ve.first);

                LOG_TRACE("create reverse edge : src " << ToStr(reverseVe.first) << ", dest: " << ToStr(reverseVe.second));
                VisibleEdgeFlowPointer vefp(new VisibleEdgeFlow(*this, commonData, operationsQueue));
                visibleEdgeHandlers.insert(std::make_pair(reverseVe, vefp));
                std::map<VisibleEdge, VisibleEdgeFlowPointer>::iterator itVisibleEdgeFlow =
                            visibleEdgeHandlers.find(reverseVe);
                itVisibleEdgeFlow->second->visibleEdgeFinishedHandler
                            = boost::bind(&DevicesManager::VisibleEdgeFinished, this, _1, _2, _3);
                itVisibleEdgeFlow->second->createVisibleEdge(reverseVe.first, reverseVe.second, it->second->getRsl());

                // register handler for alarms
                commonData.alarmDispatcher.registerAlarm788Listener(vefp, reverseVe.first, reverseVe.second);
                commonData.nodeVisibleVerifier.addVisibleEdgeToInProgress(reverseVe);
            }

            visibleEdgeHandlers.erase(it++);
        }
        else
        {
            it++;
        }
    }

    // 2. create new visible flows if the maximum number allows
    bool existsData = true;
    while (existsData && this->visibleEdgeHandlers.size() < commonData.settings.cevMaxFlows)
    {
        VisibleEdge ve = std::make_pair(0, 0);
        uint8_t rsl = 0;
        if (commonData.nodeVisibleVerifier.processNextVisibleEdge(ve, rsl))
        {
            VisibleEdge as = std::make_pair(ve.first, ve.second);
            VisibleEdgeFlowPointer vefp(new VisibleEdgeFlow(*this, commonData, operationsQueue));
            visibleEdgeHandlers.insert(std::make_pair(as, vefp));
            std::map<VisibleEdge, VisibleEdgeFlowPointer>::iterator itVisibleEdgeFlow = visibleEdgeHandlers.find(as);
            itVisibleEdgeFlow->second->visibleEdgeFinishedHandler = boost::bind(&DevicesManager::VisibleEdgeFinished,
                                                                                this, _1, _2, _3);
            itVisibleEdgeFlow->second->createVisibleEdge(ve.first, ve.second, rsl);

            // register handler for alarms
            commonData.alarmDispatcher.registerAlarm788Listener(vefp, ve.first, ve.second);
        }
        else
        {
            existsData = false;
        }
    }
}

void DevicesManager::VisibleEdgeFinished(uint16_t src, uint16_t dest, bool status)
{
    LOG_TRACE("VisibleEdgeFinished()");

    if (status == false)
    {
        LOG_DEBUG("VisibleEdgeFinished() : alarms have been received => cancel edge (" << ToStr(src) << ", " << ToStr(dest) << ")");
    }
    else
    {
        LOG_DEBUG("VisibleEdgeFinished() : edge validated (" << ToStr(src) << ", " << ToStr(dest) << ")");
    }

    // remove flow
    VisibleEdge as = std::make_pair(src, dest);
    std::map<VisibleEdge, VisibleEdgeFlowPointer>::iterator itVisibleEdgeFlow = visibleEdgeHandlers.find(as);
    if (itVisibleEdgeFlow != visibleEdgeHandlers.end())
    {
        this->visibleEdgeHandlers.erase(itVisibleEdgeFlow);
    }
}

uint8_t DevicesManager::getJoinsInProgressNo(Address32 address32, Address32 parentAddress32, std::set<Address32>& addresses)
{
    std::map<stack::WHartAddress, JoinFlowHandler>::iterator it = joinHandlers.begin();
    uint8_t joinsNo = 0;
    for (; it != joinHandlers.end(); ++it)
    {
        if (it->second.isActive && (it->second.getAddress32() != address32) && (it->second.getParentAddress32() == parentAddress32))
        {
            joinsNo++;
            addresses.insert(it->second.getAddress32());
        }
    }

    return joinsNo;
}

void DevicesManager::markForReloadingConfiguration()
{
    //DO NOT LOG, MIGHT LEAD TO CRASH
    flagMarkedForReloadingConfiguration = true;
}

void DevicesManager::DeleteDevice(uint16_t nickname)
{
    nodeAlarmVerifier->DeleteDevice(nickname);
}

void DevicesManager::ChangeNetworkID(uint16_t nickname, uint16_t newNetId)
{
    NE::Model::Operations::WriteNetworkIDOperation *op = new NE::Model::Operations::WriteNetworkIDOperation(nickname, newNetId);
    IEngineOperationPointer opPtr(op);
    opPtr->setDependency(WaveDependency::FIRST);

    NE::Model::Operations::EngineOperationsListPointer opList(new NE::Model::Operations::EngineOperations());
    opList->addOperation(opPtr);

    hart7::nmanager::operations::WHEngineOperationsVisitor visitor(opList, commonData);
    std::vector<hart7::nmanager::operations::WHOperationPointer> operations;
    operationsQueue.generateWHOperations(opList, operations, visitor);
    uint32_t handle;

    if (operationsQueue.addOperations(opList, operations, handle,
                                      std::string("ChangeNetworkID"),
                                      boost::bind(&DevicesManager::ConfirmChangeNetworkIDOperations, this, _1, opList), NULL, false))
    {
        operationsQueue.sendOperations();
    }

}

void DevicesManager::ConfirmChangeNetworkIDOperations(bool success, NE::Model::Operations::EngineOperationsListPointer operations)
{

}





}
}
