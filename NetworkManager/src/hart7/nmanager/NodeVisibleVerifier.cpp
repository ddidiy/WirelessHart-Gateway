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
 * NodeVisibleVerifier.cpp
 *
 *  Created on: Mar 8, 2010
 *      Author: radu
 */

#include "NodeVisibleVerifier.h"
#include "Common/NEAddress.h"

namespace hart7 {

namespace nmanager {

NodeVisibleVerifier::NodeVisibleVerifier()
{
}

NodeVisibleVerifier::~NodeVisibleVerifier()
{
}

void NodeVisibleVerifier::addVisibleNode(uint16_t source, uint16_t destination, uint8_t rsl)
{
    LOG_TRACE("addVisibleNode() (" << ToStr(source) << ", " << ToStr(destination) << "), rsl: " << std::dec << (int)rsl);

    VisibleEdge ve = std::make_pair(source, destination);
    VisibleEdge reverseVe = std::make_pair(destination, source);

    // if there is no edge or reverse edge in the structures to be evaluated, or in analyzing or finished then adds it
    if ((visibleEdgesInProgress.find(ve) != visibleEdgesInProgress.end()) //
                || (visibleEdgesInProgress.find(reverseVe) != visibleEdgesInProgress.end()) //
                || (visibleEdgesFinished.find(ve) != visibleEdgesFinished.end()) //
                || (visibleEdgesFinished.find(reverseVe) != visibleEdgesFinished.end()) //
                || (visibleEdges.find(ve) != visibleEdges.end()) //
                || (visibleEdges.find(reverseVe) != visibleEdges.end())) //
    {
        {
            LOG_INFO("Edge (or reverse) already in current|analyzing|finished, (" << ToStr(source) << ", " << ToStr(destination) << ")");
            return;
        }
    }

    visibleEdges.insert(ve);
    rslOfVisibleEdges.insert(std::make_pair(ve, rsl));
}

void NodeVisibleVerifier::removeVisibleNode(uint16_t nodeAddress)
{
    LOG_TRACE("removedVisibleNode() (" << ToStr(nodeAddress) << ")");

    std::set<VisibleEdge>::iterator itVe = visibleEdges.begin();
    for (; itVe != visibleEdges.end(); /* increment is in loop */)
    {
        if ((itVe->first == nodeAddress) || (itVe->second == nodeAddress))
        {
            visibleEdges.erase(itVe++);
            rslOfVisibleEdges.erase(*itVe);
            visibleEdgesInProgress.erase(*itVe);
            visibleEdgesFinished.erase(*itVe);
        }
        else
        {
            ++itVe;
        }
    }

}

bool NodeVisibleVerifier::processNextVisibleEdge(VisibleEdge& edge, uint8_t& rsl)
{
    if (this->visibleEdges.size() == 0)
    {
        LOG_TRACE("processNextVisibleEdge() : no visible edge available"
                    <<", visibleEdgesInProgress.size(): " << (int)visibleEdgesInProgress.size()
                    <<", visibleEdgesFinished.size(): " << (int)visibleEdgesFinished.size());
        return false;
    }
    LOG_TRACE("processNextVisibleEdge() visibleEdges.size(): " << (int)visibleEdges.size()
                <<", visibleEdgesInProgress.size(): " << (int)visibleEdgesInProgress.size()
                <<", visibleEdgesFinished.size(): " << (int)visibleEdgesFinished.size());

    std::set<VisibleEdge>::iterator it = this->visibleEdges.begin();

    for (; it != this->visibleEdges.end(); ++it)
    {

        VisibleEdge ve = *it;
        VisibleEdge reverseVe = std::make_pair(it->second, it->first);
        if (this->rslOfVisibleEdges.find(ve) != this->rslOfVisibleEdges.end())
        {
            rsl = this->rslOfVisibleEdges.find(ve)->second;
        }
        // ensure there is no flow in progress for the reverse edge
        if (this->visibleEdgesInProgress.find(reverseVe) == this->visibleEdgesInProgress.end())
        {
            this->visibleEdges.erase(it);
            this->visibleEdgesInProgress.insert(ve);
            edge = ve; // to be returned to caller
            LOG_TRACE("processNextVisibleEdge() (" << ToStr(edge.first) << ", " << ToStr(edge.second) << ")");

            return true;
        }

    }

    return false;
}

void NodeVisibleVerifier::addVisibleEdgeToInProgress(VisibleEdge edge)
{
    LOG_TRACE("addVisibleEdgeToInProgress() : mark this edge directly (" << ToStr(edge.first) << ", " << ToStr(edge.second) << ")");
    this->visibleEdgesInProgress.insert(edge);
}

void NodeVisibleVerifier::endProcessingVisibleEdge(VisibleEdge edge, bool& finishedOkBothWays)
{
    LOG_TRACE("endProcessingVisibleEdge() (" << ToStr(edge.first) << ", " << ToStr(edge.second) << ")");
    finishedOkBothWays = false;

    std::set<VisibleEdge>::iterator it = this->visibleEdgesInProgress.find(edge);
    if (it == visibleEdgesInProgress.end())
    {
        // the visible edge in progress could be deleted if in the meantime the device rejoined
        LOG_ERROR("endProcessingVisibleEdge() : no flow for (" << ToStr(edge.first) << ", " << ToStr(edge.second) << ")");
    }
    else
    {
        this->visibleEdgesInProgress.erase(it);
        this->rslOfVisibleEdges.erase(*it);
        this->visibleEdgesFinished.insert(edge);

        VisibleEdge reverseEdge;
        reverseEdge.first = edge.second;
        reverseEdge.second = edge.first;

        if (this->visibleEdgesFinished.find(reverseEdge) != this->visibleEdgesFinished.end())
        {
            finishedOkBothWays = true;
        }
    }
}

void NodeVisibleVerifier::removeFinishedVisibleEdge(VisibleEdge edge)
{

    std::set<VisibleEdge>::iterator itVe = this->visibleEdgesFinished.find(edge);
    if (itVe != this->visibleEdgesFinished.end())
    {
        this->visibleEdgesFinished.erase(itVe);
    }

    VisibleEdge reverseEdge = std::make_pair(edge.second, edge.first);
    std::set<VisibleEdge>::iterator itReverseVe = this->visibleEdgesFinished.find(reverseEdge);
    if (itReverseVe != this->visibleEdgesFinished.end())
    {
        this->visibleEdgesFinished.erase(itReverseVe);
    }
}

}

}
