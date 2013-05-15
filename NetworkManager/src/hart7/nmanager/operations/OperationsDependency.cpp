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
 * OperationsDependency.cpp
 *
 *  Created on: Nov 9, 2009
 *      Author: Andy
 */

#include "OperationsDependency.h"

namespace hart7 {
namespace nmanager {
namespace operations {

using namespace NE::Model::Operations;

OperationsDependency::OperationsDependency(NE::Model::Operations::EngineOperations& operations_) :
    operations(operations_), subnetTopology(NE::Model::NetworkEngine::instance().getSubnetTopology())
{
    EngineOperationsVector::iterator it_ops = operations.operations.begin();
    for (; it_ops != operations.operations.end(); it_ops++)
    {
        queueByAddress.insert(pair<Address32, IEngineOperationPointer> ((*it_ops)->getOwner(), *it_ops));
    }
}

void OperationsDependency::inGeneral(EngineOperationsVector ops)
{
    EngineOperationsVector ChangePriorityOps;
    getOperationsByType(ChangePriorityOps, EngineOperationType::CHANGE_JOIN_PRIORITY, ops);
    EngineOperationsVector SuperFrameAddedOps;
    getOperationsByType(SuperFrameAddedOps, EngineOperationType::ADD_SUPERFRAME, ops);
    EngineOperationsVector LinkAddedOps;
    getOperationsByType(LinkAddedOps, EngineOperationType::ADD_LINK, ops);
    EngineOperationsVector NGAddedOps;
    getOperationsByType(NGAddedOps, EngineOperationType::ADD_NEIGHBOR_GRAPH, ops);
    EngineOperationsVector RouteAddedOps;
    getOperationsByType(RouteAddedOps, EngineOperationType::ADD_ROUTE, ops);
    EngineOperationsVector ServiceAddedOps;
    getOperationsByType(ServiceAddedOps, EngineOperationType::ADD_SERVICE, ops);
    EngineOperationsVector LinkRemoveOps;
    getOperationsByType(LinkRemoveOps, EngineOperationType::REMOVE_LINK, ops);
    EngineOperationsVector RouteRemOps;
    getOperationsByType(RouteRemOps, EngineOperationType::REMOVE_ROUTE, ops);
    EngineOperationsVector SetSrcClockOps;
    getOperationsByType(SetSrcClockOps, EngineOperationType::SET_CLOCK_SOURCE, ops);

    EngineOperationsVector LinkRemShOps, LinkRemTxOps, LinkRemRxOps;
    EngineOperationsVector LinkAddedTxOps, LinkAddedRxOps;

    EngineOperationsVector::iterator it_sfa = SuperFrameAddedOps.begin();
    EngineOperationsVector::iterator it_la = LinkAddedOps.begin();
    EngineOperationsVector::iterator it_larx = LinkAddedRxOps.begin();
    EngineOperationsVector::iterator it_latx = LinkAddedTxOps.begin();
    EngineOperationsVector::iterator it_nga = NGAddedOps.begin();
    EngineOperationsVector::iterator it_ra = RouteAddedOps.begin();
    EngineOperationsVector::iterator it_sa = ServiceAddedOps.begin();
    EngineOperationsVector::iterator it_lr = LinkRemoveOps.begin();
    EngineOperationsVector::iterator it_rr = RouteRemOps.begin();
    EngineOperationsVector::iterator it_ssc = SetSrcClockOps.begin();
    EngineOperationsVector::iterator it_lrsh, it_lrtx, it_lrrx;

    for (it_ssc = SetSrcClockOps.begin(); it_ssc != SetSrcClockOps.end(); it_ssc++)
        NGAddedOps.push_back(*it_ssc);

    //separate shared and non shared linkremove ops
    for (it_lr = LinkRemoveOps.begin(); it_lr != LinkRemoveOps.end(); it_lr++)
    {
        if (((boost::shared_ptr<LinkRemovedOperation>&) (*it_lr))->shared)
            LinkRemShOps.push_back(*it_lr);
        else
        {
            if (((boost::shared_ptr<LinkRemovedOperation>&) (*it_lr))->transmission)
                LinkRemTxOps.push_back(*it_lr);
            else if (((boost::shared_ptr<LinkRemovedOperation>&) (*it_lr))->reception)
                LinkRemRxOps.push_back(*it_lr);
        }
    }

    //separate tx and rx linkadded ops
    for (it_la = LinkAddedOps.begin(); it_la != LinkAddedOps.end(); it_la++)
    {
        if (((boost::shared_ptr<LinkAddedOperation>&) (*it_la))->transmission)
            LinkAddedTxOps.push_back(*it_la);
        else if (((boost::shared_ptr<LinkAddedOperation>&) (*it_la))->reception)
            LinkAddedRxOps.push_back(*it_la);
    }

    createDependencies(LinkAddedRxOps, SuperFrameAddedOps);
    if (LinkAddedRxOps.empty())
        LinkAddedRxOps = SuperFrameAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(LinkAddedTxOps, LinkAddedRxOps);
    if (LinkAddedTxOps.empty())
        LinkAddedTxOps = LinkAddedRxOps; //copy vector, change it, maybe with pointers or something

    createDependencies(NGAddedOps, LinkAddedTxOps);
    if (NGAddedOps.empty())
        NGAddedOps = LinkAddedTxOps; //copy vector, change it, maybe with pointers or something

    createDependencies(RouteAddedOps, NGAddedOps);
    if (RouteAddedOps.empty())
        RouteAddedOps = NGAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(ServiceAddedOps, RouteAddedOps);
    if (ServiceAddedOps.empty())
        ServiceAddedOps = RouteAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(LinkRemShOps, ServiceAddedOps);
    if (LinkRemShOps.empty())
        LinkRemShOps = ServiceAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(RouteRemOps, LinkRemShOps);
    if (RouteRemOps.empty())
        RouteRemOps = LinkRemShOps; //copy vector, change it, maybe with pointers or something

    createDependencies(LinkRemTxOps, RouteRemOps);
    if (LinkRemTxOps.empty())
        LinkRemTxOps = RouteRemOps; //copy vector, change it, maybe with pointers or something

    createDependencies(LinkRemRxOps, LinkRemTxOps);
    if (LinkRemRxOps.empty())
        LinkRemRxOps = LinkRemTxOps; //copy vector, change it, maybe with pointers or something

    //createDependencies(noties769, LinkRemRxOps);
    createDependencies(ChangePriorityOps, LinkRemRxOps);
}

void OperationsDependency::onOutboundAttach()
{
    using namespace NE::Model::Operations;

    Address32 child = operations.getRequesterAddress32();
    Address32 parent = NE::Model::NetworkEngine::instance().getDevice(child).parent32;


    EngineOperationsVector ChangePriorityOps;
    getOperationsByType(ChangePriorityOps, EngineOperationType::CHANGE_JOIN_PRIORITY, operations.operations);
    EngineOperationsVector SuperFrameAddedOps;
    getOperationsByType(SuperFrameAddedOps, EngineOperationType::ADD_SUPERFRAME, operations.operations);
    EngineOperationsVector LinkAddedOps;
    getOperationsByType(LinkAddedOps, EngineOperationType::ADD_LINK, operations.operations);
    EngineOperationsVector NGAddedOps;
    getOperationsByType(NGAddedOps, EngineOperationType::ADD_NEIGHBOR_GRAPH, operations.operations);
    createDependenciesForNAOps(NGAddedOps);
    EngineOperationsVector NGRemOps;
    getOperationsByType(NGRemOps, EngineOperationType::REMOVE_NEIGHBOR_GRAPH, operations.operations);
    createDependenciesForNROps(NGRemOps);
    EngineOperationsVector RouteAddedOps;
    getOperationsByType(RouteAddedOps, EngineOperationType::ADD_ROUTE, operations.operations);
    EngineOperationsVector ServiceAddedOps;
    getOperationsByType(ServiceAddedOps, EngineOperationType::ADD_SERVICE, operations.operations);
    EngineOperationsVector LinkRemoveOps;
    getOperationsByType(LinkRemoveOps, EngineOperationType::REMOVE_LINK, operations.operations);
    EngineOperationsVector RouteRemOps;
    getOperationsByType(RouteRemOps, EngineOperationType::REMOVE_ROUTE, operations.operations);
    EngineOperationsVector SetSrcClockOps;
    getOperationsByType(SetSrcClockOps, EngineOperationType::SET_CLOCK_SOURCE, operations.operations);
    EngineOperationsVector ReadCapsOp;
    getOperationsByType(ReadCapsOp, EngineOperationType::READ_WIRELESS_DEVICE_CAPABILITIES, operations.operations);

    EngineOperationsVector LinkRemShOps, LinkRemTxOps, LinkRemRxOps;
    EngineOperationsVector LinkAddedTxOps, LinkAddedRxOps;
    EngineOperationsVector LinkAddedTxOpsChild, LinkAddedRxOpsChild;
    EngineOperationsVector SfAddedOpChild;
    EngineOperationsVector LinkAddedTxOpParent;


    EngineOperationsVector::iterator it_sfa = SuperFrameAddedOps.begin();
    EngineOperationsVector::iterator it_la = LinkAddedOps.begin();
    EngineOperationsVector::iterator it_larx = LinkAddedRxOps.begin();
    EngineOperationsVector::iterator it_latx = LinkAddedTxOps.begin();
    EngineOperationsVector::iterator it_nga = NGAddedOps.begin();
    EngineOperationsVector::iterator it_ra = RouteAddedOps.begin();
    EngineOperationsVector::iterator it_sa = ServiceAddedOps.begin();
    EngineOperationsVector::iterator it_lr = LinkRemoveOps.begin();
    EngineOperationsVector::iterator it_rr = RouteRemOps.begin();
    EngineOperationsVector::iterator it_ssc = SetSrcClockOps.begin();
    EngineOperationsVector::iterator it_lrsh, it_lrtx, it_lrrx;

    for (it_sfa =  SuperFrameAddedOps.begin(); it_sfa != SuperFrameAddedOps.end(); )
    {
        if ((*it_sfa)->getOwner() == child)
        {
            SfAddedOpChild.push_back(*it_sfa);
            it_sfa = SuperFrameAddedOps.erase(it_sfa);
        }
        else
        {
            it_sfa++;
        }
    }

    for (it_ssc = SetSrcClockOps.begin(); it_ssc != SetSrcClockOps.end(); it_ssc++)
        NGAddedOps.push_back(*it_ssc);

    //separate shared and non shared linkremove ops
    for (it_lr = LinkRemoveOps.begin(); it_lr != LinkRemoveOps.end(); it_lr++)
    {
        if (((boost::shared_ptr<LinkRemovedOperation>&) (*it_lr))->shared)
            LinkRemShOps.push_back(*it_lr);
        else
        {
            if (((boost::shared_ptr<LinkRemovedOperation>&) (*it_lr))->transmission)
                LinkRemTxOps.push_back(*it_lr);
            else if (((boost::shared_ptr<LinkRemovedOperation>&) (*it_lr))->reception)
                LinkRemRxOps.push_back(*it_lr);
        }
    }

    //separate tx and rx linkadded ops
    for (it_la = LinkAddedOps.begin(); it_la != LinkAddedOps.end(); it_la++)
    {
        if (((boost::shared_ptr<LinkAddedOperation>&) (*it_la))->transmission)
        {
            if (((boost::shared_ptr<LinkAddedOperation>&)(*it_la))->peerAddress ==  child) {
                    LinkAddedTxOpParent.push_back(*it_la);
            } else if ((*it_la)->getOwner() ==  child) {
                    LinkAddedTxOpsChild.push_back(*it_la);
            } else {
                    LinkAddedTxOps.push_back(*it_la);
            }
        }
        if (((boost::shared_ptr<LinkAddedOperation>&) (*it_la))->reception)
        {
            if ((*it_la)->getOwner() == child) {
                    LinkAddedRxOpsChild.push_back(*it_la);
            } else {
                    LinkAddedRxOps.push_back(*it_la);
            }
        }
    }

    createDependencies(LinkAddedRxOps, SuperFrameAddedOps);
    if (LinkAddedRxOps.empty())
        LinkAddedRxOps = SuperFrameAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(ReadCapsOp, LinkAddedRxOps); // add read Device Caps in the same packet as the Rx Link add

    createDependencies(LinkAddedTxOps, LinkAddedRxOps);
    if (LinkAddedTxOps.empty())
        LinkAddedTxOps = LinkAddedRxOps; //copy vector, change it, maybe with pointers or something

    createDependencies(NGAddedOps, LinkAddedTxOps);
    if (NGAddedOps.empty())
        NGAddedOps = LinkAddedTxOps; //copy vector, change it, maybe with pointers or something

    createDependencies(RouteAddedOps, NGAddedOps);
    if (RouteAddedOps.empty())
        RouteAddedOps = NGAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(ServiceAddedOps, RouteAddedOps);
    if (ServiceAddedOps.empty())
        ServiceAddedOps = RouteAddedOps; //copy vector, change it, maybe with pointers or something

    // child operations -> Rx, Tx depend on Sf, should be sent in the same packet.
    createDependencies(LinkAddedTxOpsChild, SfAddedOpChild);
    createDependencies(LinkAddedRxOpsChild, SfAddedOpChild);
    createDependencies(LinkAddedTxOpsChild, LinkAddedRxOpsChild);   // just so that the remaining of ops will be sent after both.
    // child operations -> send ops only after route has been added on NM, graph to child has been created.
    createDependencies(SfAddedOpChild, ServiceAddedOps);

    // send Tx link on parent after child packet.
    createDependencies(LinkAddedTxOpParent, SfAddedOpChild);
    createDependencies(LinkAddedTxOpParent, SuperFrameAddedOps);
    createDependencies(LinkAddedTxOpParent, LinkAddedTxOpsChild);
    createDependencies(LinkAddedTxOpParent, LinkAddedRxOpsChild);


    createDependencies(NGRemOps, LinkAddedTxOpParent);
    if (NGRemOps.empty())
        NGRemOps = LinkAddedTxOpParent; //copy vector, change it, maybe with pointers or something

    createDependencies(LinkRemShOps, NGRemOps);
    if (LinkRemShOps.empty())
        LinkRemShOps = NGRemOps; //copy vector, change it, maybe with pointers or something


    createDependencies(RouteRemOps, LinkRemShOps);
    if (RouteRemOps.empty())
        RouteRemOps = LinkRemShOps; //copy vector, change it, maybe with pointers or something


    //creates dependencies between shared links removed and link add
    createDependencies(LinkRemShOps,LinkAddedTxOpsChild);
    createDependencies(LinkRemShOps, LinkAddedTxOps);

    createDependencies(LinkRemTxOps, RouteRemOps);
    if (LinkRemTxOps.empty())
        LinkRemTxOps = RouteRemOps; //copy vector, change it, maybe with pointers or something


    createDependencies(LinkRemRxOps, LinkRemTxOps);
    if (LinkRemRxOps.empty())
        LinkRemRxOps = LinkRemTxOps; //copy vector, change it, maybe with pointers or something

    createDependencies(ChangePriorityOps, LinkRemRxOps);

}

//Creates dependencies for the operations in the engine operation list involved in the evaluate route process
void OperationsDependency::onEvaluateNextPath()
{

    using namespace NE::Model::Operations;

    EngineOperationsVector ChangePriorityOps;
    getOperationsByType(ChangePriorityOps, EngineOperationType::CHANGE_JOIN_PRIORITY, operations.operations);
    EngineOperationsVector SuperFrameAddedOps;
    getOperationsByType(SuperFrameAddedOps, EngineOperationType::ADD_SUPERFRAME, operations.operations);
    EngineOperationsVector LinkAddedOps;
    getOperationsByType(LinkAddedOps, EngineOperationType::ADD_LINK, operations.operations);
    EngineOperationsVector NGAddedOps;
    getOperationsByType(NGAddedOps, EngineOperationType::ADD_NEIGHBOR_GRAPH, operations.operations);
    createDependenciesForNAOps(NGAddedOps);
    EngineOperationsVector NGRemOps;
    getOperationsByType(NGRemOps, EngineOperationType::REMOVE_NEIGHBOR_GRAPH, operations.operations);
    createDependenciesForNROps(NGRemOps);
    EngineOperationsVector RouteAddedOps;
    getOperationsByType(RouteAddedOps, EngineOperationType::ADD_ROUTE, operations.operations);
    EngineOperationsVector ServiceAddedOps;
    getOperationsByType(ServiceAddedOps, EngineOperationType::ADD_SERVICE, operations.operations);
    EngineOperationsVector LinkRemoveOps;
    getOperationsByType(LinkRemoveOps, EngineOperationType::REMOVE_LINK, operations.operations);
    EngineOperationsVector RouteRemOps;
    getOperationsByType(RouteRemOps, EngineOperationType::REMOVE_ROUTE, operations.operations);
    EngineOperationsVector SetSrcClockOps;
    getOperationsByType(SetSrcClockOps, EngineOperationType::SET_CLOCK_SOURCE, operations.operations);

    EngineOperationsVector LinkRemShOps, LinkRemTxOps, LinkRemRxOps;
    EngineOperationsVector LinkAddedTxOps, LinkAddedRxOps;

    EngineOperationsVector::iterator it_sfa = SuperFrameAddedOps.begin();
    EngineOperationsVector::iterator it_la = LinkAddedOps.begin();
    EngineOperationsVector::iterator it_larx = LinkAddedRxOps.begin();
    EngineOperationsVector::iterator it_latx = LinkAddedTxOps.begin();
    EngineOperationsVector::iterator it_nga = NGAddedOps.begin();
    EngineOperationsVector::iterator it_ra = RouteAddedOps.begin();
    EngineOperationsVector::iterator it_sa = ServiceAddedOps.begin();
    EngineOperationsVector::iterator it_lr = LinkRemoveOps.begin();
    EngineOperationsVector::iterator it_rr = RouteRemOps.begin();
    EngineOperationsVector::iterator it_ssc = SetSrcClockOps.begin();
    EngineOperationsVector::iterator it_lrsh, it_lrtx, it_lrrx;

    for (it_ssc = SetSrcClockOps.begin(); it_ssc != SetSrcClockOps.end(); it_ssc++)
        NGAddedOps.push_back(*it_ssc);

    //separate shared and non shared linkremove ops
    for (it_lr = LinkRemoveOps.begin(); it_lr != LinkRemoveOps.end(); it_lr++)
    {
        if (((boost::shared_ptr<LinkRemovedOperation>&) (*it_lr))->shared)
            LinkRemShOps.push_back(*it_lr);
        else
        {
            if (((boost::shared_ptr<LinkRemovedOperation>&) (*it_lr))->transmission)
                LinkRemTxOps.push_back(*it_lr);
            else if (((boost::shared_ptr<LinkRemovedOperation>&) (*it_lr))->reception)
                LinkRemRxOps.push_back(*it_lr);
        }
    }

    //separate tx and rx linkadded ops
    for (it_la = LinkAddedOps.begin(); it_la != LinkAddedOps.end(); it_la++)
    {
        if (((boost::shared_ptr<LinkAddedOperation>&) (*it_la))->transmission)
            LinkAddedTxOps.push_back(*it_la);
        if (((boost::shared_ptr<LinkAddedOperation>&) (*it_la))->reception)
            LinkAddedRxOps.push_back(*it_la);
    }

    createDependencies(LinkAddedRxOps, SuperFrameAddedOps);
    if (LinkAddedRxOps.empty())
        LinkAddedRxOps = SuperFrameAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(LinkAddedTxOps, LinkAddedRxOps);
    if (LinkAddedTxOps.empty())
        LinkAddedTxOps = LinkAddedRxOps; //copy vector, change it, maybe with pointers or something

    createDependencies(NGAddedOps, LinkAddedTxOps);
    if (NGAddedOps.empty())
        NGAddedOps = LinkAddedTxOps; //copy vector, change it, maybe with pointers or something

    createDependencies(RouteAddedOps, NGAddedOps);
    if (RouteAddedOps.empty())
        RouteAddedOps = NGAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(ServiceAddedOps, RouteAddedOps);
    if (ServiceAddedOps.empty())
        ServiceAddedOps = RouteAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(NGRemOps, ServiceAddedOps);
    if (NGRemOps.empty())
        NGRemOps = ServiceAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(LinkRemShOps, NGRemOps);
    if (LinkRemShOps.empty())
        LinkRemShOps = NGRemOps; //copy vector, change it, maybe with pointers or something


    createDependencies(RouteRemOps, LinkRemShOps);
    if (RouteRemOps.empty())
        RouteRemOps = LinkRemShOps; //copy vector, change it, maybe with pointers or something

    createDependencies(LinkRemTxOps, RouteRemOps);
    if (LinkRemTxOps.empty())
        LinkRemTxOps = RouteRemOps; //copy vector, change it, maybe with pointers or something


    createDependencies(LinkRemRxOps, LinkRemTxOps);
    if (LinkRemRxOps.empty())
        LinkRemRxOps = LinkRemTxOps; //copy vector, change it, maybe with pointers or something

    //    createDependencies(noties769, LinkRemRxOps);
    createDependencies(ChangePriorityOps, LinkRemRxOps);
}

//Sets dependencies for the operations in the engine operation list involved in the join process
void OperationsDependency::onJoinDevice()
{
    using namespace NE::Model::Operations;

    EngineOperationsVector WriteTimerOps;
    getOperationsByType(WriteTimerOps, EngineOperationType::SET_KEEP_ALIVE_PERIOD, operations.operations);

    EngineOperationsVector ChangePriorityOps;
    getOperationsByType(ChangePriorityOps, EngineOperationType::CHANGE_JOIN_PRIORITY, operations.operations);

    EngineOperationsVector SuperFrameAddedOps;
    getOperationsByType(SuperFrameAddedOps, EngineOperationType::ADD_SUPERFRAME, operations.operations);

    EngineOperationsVector LinkAddedOps;
    getOperationsByType(LinkAddedOps, EngineOperationType::ADD_LINK, operations.operations);

    EngineOperationsVector NGAddedOps;
    getOperationsByType(NGAddedOps, EngineOperationType::ADD_NEIGHBOR_GRAPH, operations.operations);

    EngineOperationsVector RouteAddedOps;
    getOperationsByType(RouteAddedOps, EngineOperationType::ADD_ROUTE, operations.operations);

    EngineOperationsVector ServiceAddedOps;
    getOperationsByType(ServiceAddedOps, EngineOperationType::ADD_SERVICE, operations.operations);

    EngineOperationsVector LinkAddedTxOps, LinkAddedRxOps, LinkAddedShOps;

       EngineOperationsVector::iterator it_sfa;
    EngineOperationsVector::iterator it_la;
    EngineOperationsVector::iterator it_larx;
    EngineOperationsVector::iterator it_latx;
    EngineOperationsVector::iterator it_nga;
    EngineOperationsVector::iterator it_ra;
    EngineOperationsVector::iterator it_sa;
    EngineOperationsVector::iterator it_wt;
    EngineOperationsVector::iterator it_cp;
    //    EngineOperationsVector::iterator it_shrp = SetHealthRepPerOps.begin();

    //separate tx and rx linkadded ops
    for (it_la = LinkAddedOps.begin(); it_la != LinkAddedOps.end(); it_la++)
    {
        if (((boost::shared_ptr<LinkAddedOperation>&) (*it_la))->shared)
        {
            LinkAddedShOps.push_back(*it_la);
            LinkAddedTxOps.push_back(*it_la);
            continue;
        }
        if (((boost::shared_ptr<LinkAddedOperation>&) (*it_la))->transmission)
        {
            LinkAddedTxOps.push_back(*it_la);
            continue;
        }
        if (((boost::shared_ptr<LinkAddedOperation>&) (*it_la))->reception)
        {
            LinkAddedRxOps.push_back(*it_la);
            continue;
        }
    }

    for (it_wt = WriteTimerOps.begin(); it_wt != WriteTimerOps.end(); it_wt++)
        SuperFrameAddedOps.push_back(*it_wt);

    //createDependencies(x, y) - x depends on y
    createDependencies(LinkAddedRxOps, SuperFrameAddedOps);
    if (LinkAddedRxOps.empty())
        LinkAddedRxOps = SuperFrameAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(LinkAddedTxOps, LinkAddedRxOps);
    if (LinkAddedTxOps.empty())
        LinkAddedTxOps = LinkAddedRxOps; //copy vector, change it, maybe with pointers or something


    //createDependencies(NGAddedOps, LinkAddedTxOps);
    if (NGAddedOps.empty())
        NGAddedOps = LinkAddedTxOps; //copy vector, change it, maybe with pointers or something

    createDependencies(RouteAddedOps, NGAddedOps);
    if (RouteAddedOps.empty())
        RouteAddedOps = NGAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(ServiceAddedOps, RouteAddedOps);
    if (ServiceAddedOps.empty())
        ServiceAddedOps = RouteAddedOps; //copy vector, change it, maybe with pointers or something

    createDependencies(ChangePriorityOps, ServiceAddedOps);
}

void OperationsDependency::onAllocateAdvertise()
{

    using namespace NE::Model::Operations;

    EngineOperationsVector LinkAddedOpsDEV;
    getOperationsByTypeAndAddr(LinkAddedOpsDEV, operations.getRequesterAddress32(), EngineOperationType::ADD_LINK,
                               operations.operations);
    EngineOperationsVector WriteTimerOpsDEV;
    getOperationsByTypeAndAddr(WriteTimerOpsDEV, operations.getRequesterAddress32(),
                               EngineOperationType::SET_KEEP_ALIVE_PERIOD, operations.operations);
    EngineOperationsVector SuperFrameAddedOpsDEV;
    getOperationsByTypeAndAddr(SuperFrameAddedOpsDEV, operations.getRequesterAddress32(),
                               EngineOperationType::ADD_SUPERFRAME, operations.operations);

    EngineOperationsVector SrcRouteAddedOpsNM;
    getOperationsByTypeAndAddr(SrcRouteAddedOpsNM, 0xf980, EngineOperationType::ADD_SOURCEROUTE, operations.operations);

    EngineOperationsVector ServiceAddedOpsNM;
    getOperationsByTypeAndAddr(ServiceAddedOpsNM, 0xf980, EngineOperationType::ADD_SERVICE, operations.operations);

    EngineOperationsVector RouteAddedOpsNM;
    getOperationsByTypeAndAddr(RouteAddedOpsNM, 0xf980, EngineOperationType::ADD_ROUTE, operations.operations);

    //createDependencies(x, y) - x depends on y
    createDependencies(SrcRouteAddedOpsNM, RouteAddedOpsNM);
    if (SrcRouteAddedOpsNM.empty())
        SrcRouteAddedOpsNM = RouteAddedOpsNM;

    //createDependencies(x, y) - x depends on y
    createDependencies(ServiceAddedOpsNM, SrcRouteAddedOpsNM);
    if (ServiceAddedOpsNM.empty())
        ServiceAddedOpsNM = SrcRouteAddedOpsNM;

    //createDependencies(x, y) - x depends on y
    createDependencies(WriteTimerOpsDEV, ServiceAddedOpsNM);
    if (WriteTimerOpsDEV.empty())
        WriteTimerOpsDEV = ServiceAddedOpsNM;

    //createDependencies(x, y) - x depends on y
    createDependencies(SuperFrameAddedOpsDEV, WriteTimerOpsDEV);
    if (SuperFrameAddedOpsDEV.empty())
        SuperFrameAddedOpsDEV = WriteTimerOpsDEV;

    createDependencies(LinkAddedOpsDEV, SuperFrameAddedOpsDEV);
    //    if (LinkAddedOpsDEV.empty()) LinkAddedOpsDEV = WriteTimerOpsNM;
}

void OperationsDependency::onCheckRemovedDevices()
{

    using namespace NE::Model::Operations;

    EngineOperationsVector NGRemOps;
    getOperationsByType(NGRemOps, EngineOperationType::REMOVE_NEIGHBOR_GRAPH, operations.operations);
    //createDependenciesForNROps(NGRemOps);
    EngineOperationsVector ServiceRemOps;
    getOperationsByType(ServiceRemOps, EngineOperationType::REMOVE_SERVICE, operations.operations);
    EngineOperationsVector LinkRemoveOps;
    getOperationsByType(LinkRemoveOps, EngineOperationType::REMOVE_LINK, operations.operations);
    EngineOperationsVector RouteRemOps;
    getOperationsByType(RouteRemOps, EngineOperationType::REMOVE_ROUTE, operations.operations);

    EngineOperationsVector::iterator it_ngr = NGRemOps.begin();
    EngineOperationsVector::iterator it_sr = ServiceRemOps.begin();
    EngineOperationsVector::iterator it_lr = LinkRemoveOps.begin();
    EngineOperationsVector::iterator it_rr = RouteRemOps.begin();

    createDependencies(RouteRemOps, ServiceRemOps, true);
    if (RouteRemOps.empty())
        RouteRemOps = ServiceRemOps; //copy vector, change it, maybe with pointers or something

    createDependencies(NGRemOps, RouteRemOps, true);
    if (NGRemOps.empty())
        NGRemOps = RouteRemOps; //copy vector, change it, maybe with pointers or something

    createDependencies(LinkRemoveOps, NGRemOps, true);
}

Uint16 OperationsDependency::findGraphID()
{
    return operations.getGraphId();
}

void OperationsDependency::ProcessDependencies()
{
    //	LOG_INFO("processDependencies");

    switch (operations.getNetworkEngineEventType())
    {

        case NetworkEngineEventType::JOIN_REQUEST:
        {
            onJoinDevice();
            LOG_INFO("processDependencies:JOIN_REQUEST");
            break;
        }
        case NetworkEngineEventType::EVALUATE_NEXT_ROUTE:
        {
            bool isoutboundAttach = false;
            try
            {
                if (NE::Model::NetworkEngine::instance().getDevice(operations.getRequesterAddress32()).getAction()
                    == DeviceAction::ATTACH_OUT)
                {
                    isoutboundAttach = true;
                }
            }
            catch (...)
            {
            }

            if (isoutboundAttach)
            {
                onOutboundAttach();
                LOG_INFO("processDependencies:EVALUATE_NEXT_ROUTE_OUTBOUND_ATTACH");
            }
            else
            {
                onEvaluateNextPath();
                LOG_INFO("processDependencies:EVALUATE_NEXT_ROUTE");
            }
            break;
        }

        case NetworkEngineEventType::MAKE_ROUTER:
        {
            onAllocateAdvertise();
            LOG_INFO("processDependencies:MAKE_ROUTER");
            break;
        }

        case NetworkEngineEventType::REMOVE_DEVICES:
        case NetworkEngineEventType::REMOVE_DEVICES_UDP:
        {
            onCheckRemovedDevices();
            LOG_INFO("processDependencies:REMOVE_DEVICE");
            break;
        }

        default:
        {
            break;
        }
    }
}

/**
Creates dependencies between the NeghborGraphAddedOps
The NGAOps are inserted in the correct order by the SelectBestPeer algorithm (form destination to source)
**/
void OperationsDependency::createDependenciesForNAOps(EngineOperationsVector ops)
{
    if (!ops.empty())
    {
        EngineOperationsVector::iterator it = ops.begin();
        for (; it != ops.end(); ++it)
        {
            if ((it+1) != ops.end())
            {
                (*(it+1))->setOperationDependency(*it);
            }
        }
    }

}
/**
Creates dependencies between the NeghborGraphAddedOps
TODO: MIHAI - The NGROps for the source are sent
**/
void OperationsDependency::createDependenciesForNROps(EngineOperationsVector ops)
{

    if (!ops.empty())
    {
        EngineOperationsVector::iterator it = ops.begin();
        for (; it != ops.end(); ++it)
        {
            if ((it+1) != ops.end())
            {
                (*it)->setOperationDependency(*(it+1));
            }
        }
    }
}

void OperationsDependency::createDependencies(EngineOperationsVector lowPLevel, EngineOperationsVector highPLevel, bool separateUDP)
{

    EngineOperationsVector::iterator it_low = lowPLevel.begin();
    EngineOperationsVector::iterator it_high = highPLevel.begin();

    NE::Model::DevicesTable& dt = NetworkEngine::instance().getDevicesTable();

    for (it_low = lowPLevel.begin(); it_low != lowPLevel.end(); it_low++)
        for (it_high = highPLevel.begin(); it_high != highPLevel.end(); it_high++)
        {
            try
            {
                bool islowUDP =  dt.getDevice((*it_low)->getOwner()).isUdp();
                bool ishighUDP =  dt.getDevice((*it_high)->getOwner()).isUdp();
                if (!separateUDP || (islowUDP == ishighUDP))
                {
                    (*it_low)->setOperationDependency(*it_high);
                }
            }
            catch (std::exception& ex)
            {
                LOG_WARN("Ex occured while creating dependencies. ex=" << ex.what());
            }
        }
}

//filter the operation list by operation type
Uint16 OperationsDependency::getOperationsByType(EngineOperationsVector &foundOperations,
                                                 EngineOperationType::EngineOperationTypeEnum opType,
                                                 EngineOperationsVector op)
{

    Uint16 found = 0;
    EngineOperationsVector::iterator it = op.begin();

    for ( ; it != op.end(); ++it )
    {
        if ( (*it)->getOperationType() == opType && (*it)->getState() == NE::Model::Operations::OperationState::GENERATED )
        {
            foundOperations.push_back(*it);
            found++;
        }
    }

    return found;
}

//returns a list of pointers to operations with the name matching opName and the owner address matching address32
Uint16 OperationsDependency::getOperationsByTypeAndAddr(EngineOperationsVector &foundOperations, Address32 address32,
                                                        EngineOperationType::EngineOperationTypeEnum opType,
                                                        EngineOperationsVector op)
{

    EngineOperationsVector::iterator it = op.begin();
    Uint16 found = 0;

    for ( ; it != op.end(); ++it )
    {

        if ( ((*it)->getOperationType() == opType) && ((*it)->getOwner() == address32) &&
                    (*it)->getState() == NE::Model::Operations::OperationState::GENERATED )
        {
            foundOperations.push_back(*it);
            found++;
        }
    }

    return found;
}
//NOT IN USED
//returns a list of pointers to operations with the name matching opName and the owner address matching address32
Uint16 OperationsDependency::getOperationsByTypeAndAddrs(EngineOperationsVector &foundOperations,
                                                         std::vector<Address32> address32,
                                                         EngineOperationType::EngineOperationTypeEnum opType,
                                                         NE::Model::Operations::EngineOperations op)
{
    EngineOperationsVector::iterator it = op.operations.begin();
    std::vector<Address32>::iterator it_addr = address32.begin();
    Uint16 found = 0;

    return found;
}

//NOT IN USED
Uint16 OperationsDependency::getChangeNotOpsByNots(EngineOperationsVector &foundOperations, Address32 address32,
                                                   uint16_t notie, NE::Model::Operations::EngineOperations op)
{

    EngineOperationsVector::iterator it = op.operations.begin();
    Uint16 found = 0;


    return found;
}

//dirFlag - true (lastOp depends on currentOp); false (currentOp depends on lastOp)
void OperationsDependency::determineGraph(Address32 currentAddress, Address32 destination, Uint16 graphId,
                                          Uint16 searchId, IEngineOperationPointer lastOp, EngineOperationsVector ops,
                                          bool dirFlag)
{

    if (currentAddress == destination)
    {
        return;
    }

    SubnetTopology & subnetTopology = NetworkEngine::instance().getSubnetTopology();

    if (!subnetTopology.existsNode(currentAddress))
    {
        LOG_ERROR("determineGraph - in graphId=" << ToStr(graphId) << ", the address doesn't exist: " << ToStr(currentAddress));
        return;
    }

    LOG_DEBUG("determineGraph - in graphId=" << ToStr(graphId));

    Node& node = subnetTopology.getNode(currentAddress);

    if (node.getSearchId() == searchId)
    {
        return;
    }
    else
    {
        node.setSearchId(searchId);
    }

    EdgeList& edges = node.getOutBoundEdges();
    IEngineOperationPointer lastOp_ = lastOp;

    for (EdgeList::iterator it = edges.begin(); it != edges.end(); it++)
    {

        if (it->existsGraphNeighbor(graphId))
        {

            //search for operations
            LOG_DEBUG("determineGraph - find neighbor on graphId=" << ToStr(graphId));
            EngineOperationsVector::iterator it_ops = ops.begin();
            for (; it_ops != ops.end(); it_ops++)
            {
                if (((*it_ops)->getOwner() == currentAddress))
                {
                    LOG_DEBUG("determineGraph - find operation on graphId=" << ToStr(graphId) << " for " << currentAddress);

                    //insert dependencies between operations for currentAddress


                    if (lastOp)
                    {

                        //if

                        LOG_DEBUG("determineGraph - lastOp");
                        if (dirFlag)
                        {
                            lastOp->setOperationDependency(*it_ops);
                            lastOp_ = (*it_ops);
                        }
                        else
                        {
                            (*it_ops)->setOperationDependency(lastOp);
                            lastOp_ = (*it_ops);
                        }
                    }
                    break;
                }
            }
            determineGraph(it->getDestination(), destination, graphId, searchId, lastOp_, ops, dirFlag);
        }
    }
}

void OperationsDependency::createOnGraphDependencies(Address32 currentAddress, Address32 destination, Uint16 graphId,
                                                     Uint16 searchId, OperationsTypeMap lastOp)
{
    if (currentAddress == destination)
    {
        return;
    }

    SubnetTopology & subnetTopology = NetworkEngine::instance().getSubnetTopology();
    if (!subnetTopology.existsNode(currentAddress))
    {
        LOG_ERROR("createOnGraphDependencies - in graphId=" << ToStr(graphId) << ", the address doesn't exist: " << ToStr(currentAddress));
        return;
    }

    LOG_DEBUG("createOnGraphDependencies - in graphId=" << ToStr(graphId));

    Node& node = subnetTopology.getNode(currentAddress);
    if (node.getSearchId() == searchId)
    {
        return;
    }
    else
    {
        node.setSearchId(searchId);
    }

    OperationsTypeMap lastOp_ = lastOp;

    // create dependencies for currentAddres
    // get operations with owner address curentAddress
    pair<OperationsAddressMap::iterator, OperationsAddressMap::iterator> result =
                queueByAddress.equal_range(currentAddress);
    EngineOperationsVector op;
    OperationsAddressMap::iterator it_oam;

    // for all operations with owner address == with currentAddress
    for (it_oam = result.first; it_oam != result.second; it_oam++)
    {
        LOG_DEBUG("createOnGraphDependencies - found operations for" << ToStr(currentAddress));

        op.push_back((*it_oam).second);

        // create dependencies for add neighbor operations
        if ((*it_oam).second->getOperationType() == EngineOperationType::ADD_NEIGHBOR_GRAPH)
        {
            LOG_DEBUG("createOnGraphDependencies - found NeighborAddedOperation " );

            pair<OperationsTypeMap::iterator, OperationsTypeMap::iterator> result2 =
                        lastOp.equal_range(EngineOperationType::ADD_NEIGHBOR_GRAPH);
            OperationsTypeMap::iterator it_otm;

            if (result2.first == result2.second)
            {
                lastOp_.insert(
                               pair<EngineOperationType::EngineOperationTypeEnum, IEngineOperationPointer> (
                                                                                                            EngineOperationType::ADD_NEIGHBOR_GRAPH,
                                                                                                            (*it_oam).second));
            }
            else
            {
                for (it_otm = result2.first; it_otm != result2.second; ++it_otm)
                {
                    (*it_otm).second->setOperationDependency((*it_oam).second);

                    lastOp_.erase((*it_otm).first);
                    lastOp_.insert(
                                   pair<EngineOperationType::EngineOperationTypeEnum, IEngineOperationPointer> (
                                                                                                                EngineOperationType::ADD_NEIGHBOR_GRAPH,
                                                                                                                (*it_oam).second));
                }
            }
        }
    }

    //create dependencies between operations for the same address
    if (op.size() != 0)
        inGeneral(op);

    EdgeList& edges = node.getOutBoundEdges();
    for (EdgeList::iterator it = edges.begin(); it != edges.end(); it++)
    {

        LOG_DEBUG("createOnGraphDependencies - found neighbor on graphId=" << ToStr(graphId));

        if (it->existsGraphNeighbor(graphId))
        {
            createOnGraphDependencies(it->getDestination(), destination, graphId, searchId, lastOp_);
        }
    }
}

bool OperationsDependency::existsOperationWithOwner(
                                                    std::set<NE::Model::Operations::IEngineOperationPointer>& operations,
                                                    Address32 owner)
{
    std::set<NE::Model::Operations::IEngineOperationPointer>::iterator itDependent = operations.begin();
    for (; itDependent != operations.end(); ++itDependent)
    {
        if ((*itDependent)->getOwner() == owner)
        {
            return true;
        }
    }

    return false;
}

}
}
}
