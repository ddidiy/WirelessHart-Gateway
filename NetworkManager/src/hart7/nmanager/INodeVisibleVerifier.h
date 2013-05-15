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
 * INodeVisibleVerifier.h
 *
 *  Created on: Mar 8, 2010
 *      Author: radu
 */

#ifndef INODEVISIBLEVERIFIER_H_
#define INODEVISIBLEVERIFIER_H_

#include "WHartStack/WHartTypes.h"

namespace hart7 {

namespace nmanager {

typedef std::pair<uint16_t, uint16_t> VisibleEdge;

/**
 * When a visible node report is coming from the field a class of this type will be informed about it.
 * @author Radu Pop
 */
class INodeVisibleVerifier
{

    public:

        virtual ~INodeVisibleVerifier()
        {
        }

        /**
         * Adds this pair to be evaluated.
         */
        virtual void addVisibleNode(uint16_t source, uint16_t destination, uint8_t rsl) = 0;

        /**
         * Remove any active flows and reports of visible nodes in which the nodeAddress was involved.
         */
        virtual void removeVisibleNode(uint16_t nodeAddress) = 0;

        /**
         * Returns the next available visible edge to be verified.
         */
        virtual bool processNextVisibleEdge(VisibleEdge& edge, uint8_t& rsl) = 0;

        /**
         * When a processing visible edge is finished calling this method will remove it from the visibleEdgesInProgress.
         */
        virtual void endProcessingVisibleEdge(VisibleEdge edge, bool& finishedOkBothWays) = 0;

        /**
         * Removed from finished visible edges the edge and the reverse edge.
         */
        virtual void removeFinishedVisibleEdge(VisibleEdge edge) = 0;

        /**
         * Adds this edge directly to in progress. Useful when we analyze a reverse edge
         * (and this edge is not present in visible edges)
         */
        virtual void addVisibleEdgeToInProgress(VisibleEdge edge) = 0;
};

}

}

#endif /* INODEVISIBLEVERIFIER_H_ */
