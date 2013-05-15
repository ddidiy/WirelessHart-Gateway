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
 * NodeVisibleVerifier.h
 *
 *  Created on: Mar 8, 2010
 *      Author: radu
 */

#ifndef NODEVISIBLEVERIFIER_H_
#define NODEVISIBLEVERIFIER_H_

#include <set>
#include <nlib/log.h>
#include "INodeVisibleVerifier.h"

namespace hart7 {

namespace nmanager {

/**
 * This is a container for (source, destination) pairs.
 * This pairs are created after a report is received from the field.
 * From time to time some of the pairs are taken from the current set and added to the in progress set.
 * @author Radu Pop
 */
class NodeVisibleVerifier: public INodeVisibleVerifier
{

        LOG_DEF("h7.n.NodeVisibleVerifier");

    private:

        // represents the visible edges to be processed
        std::set<VisibleEdge> visibleEdges;

        // represents the RSL of the visible edges
        std::map<VisibleEdge, uint8_t> rslOfVisibleEdges;

        // the visible edges that are analyzed (they're removed from visibleEdges and added here)
        std::set<VisibleEdge> visibleEdgesInProgress;

        // keeps the edges that were validated until the reverse pairs are validated too
        std::set<VisibleEdge> visibleEdgesFinished;

    public:

        NodeVisibleVerifier();

        virtual ~NodeVisibleVerifier();

        /**
         * If there is no visible edge flow in progress for this edge or for the reverse edge
         * then add it to the list of the visible edges to be evaluated.
         */
        void addVisibleNode(uint16_t source, uint16_t destination, uint8_t rsl);

        /**
         * Remove any active flows and reports of visible nodes in which the nodeAddress was involved.
         */
        void removeVisibleNode(uint16_t nodeAddress);

        /**
         * Returns the next visible edge to be processed. Finds a new visible edge that has no REVERSE edge
         * flow in progress.
         * If there is no other visible edge then return false.
         */
        bool processNextVisibleEdge(VisibleEdge& edge, uint8_t& rsl);

        /**
         * When a processing visible edge is finished calling this method will remove it from the visibleEdgesInProgress.
         * finishedOkBothWays is set to true if the reverse edge was also confirmed.
         */
        void endProcessingVisibleEdge(VisibleEdge edge, bool& finishedOkBothWays);

        /**
         * Removed from finished visible edges the edge and the reverse edge.
         */
        void removeFinishedVisibleEdge(VisibleEdge edge);

        /**
         * Adds this edge directly to in progress. Useful when we analyze a reverse edge
         * (and this edge is not present in visible edges)
         */
        void addVisibleEdgeToInProgress(VisibleEdge edge);
};

}

}

#endif /* NODEVISIBLEVERIFIER_H_ */
