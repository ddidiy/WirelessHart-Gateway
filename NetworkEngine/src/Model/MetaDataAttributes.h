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
 * MetaDataAttributes.h
 *
 *  Created on: Apr 6, 2009
 *      Author: ioanpocol
 */

#ifndef METADATAATTRIBUTES_H_
#define METADATAATTRIBUTES_H_

#include <stdint.h>
#include "Model/Capabilities.h"
#include "Model/IEngineExceptions.h"
#include "Common/logging.h"


namespace NE {
namespace Model {

#define DEFAULT_USED_SERVICES 1
#define DEFAULT_USED_SESSIONS 1
#define DEFAULT_USED_ROUTES 2
#define DEFAULT_USED_SOURCE_ROUTES 1
#define DEFAULT_USED_GRAPHS 2
#define DEFAULT_USED_NEIGHBORS 1
#define DEFAULT_USED_LINKS 3
#define DEFAULT_USED_SUPERFRAMES 1

class MetaDataAttributes;
typedef boost::shared_ptr<MetaDataAttributes> MetaDataAttributesPointer;

/**
 * Represent the meta data of the corresponding structured attributes.
 */
class MetaDataAttributes {
    LOG_DEF("I.M.MetaDataAttributes");

    private:

        /**
         * This is the join priority of a device. It is directly influenced by the number of free resources.
         * The more free resources there are the bigger the priority. 0 - it's the max priority, 15 - the least
         * priority. This field will be updated when the updated when the values of the <code>used</code>
         * fields will be updated.
         */
        uint8_t joinPriority;

        Capabilities capabilities;

        bool isTotalServicesSet;
        bool isTotalRoutesSet;
        bool isTotalSourceRoutesSet;
        bool isTotalGraphsSet;
        bool isTotalGraphNeighborsSet;
        bool isTotalSuperframesSet;
        bool isTotalLinksSet;
        bool isTotalDiagnosticsSet;

        // only for logging
        Address64 euidAddress;

        Uint16 usedServices;
        Uint16 usedRoutes;
        Uint16 usedSourceRoutes;
        Uint16 usedGraphs;
        Uint16 usedGraphNeighbors; // also known as connections
        Uint16 usedSuperframes;
        Uint16 usedLinks;
        Uint16 usedDiagnostics;
        Uint16 maxNeighbors;

        Uint16 totalServices;
        Uint16 totalRoutes;
        Uint16 totalSourceRoutes;
        Uint16 totalGraphs;
        Uint16 totalGraphNeighbors;
        Uint16 totalSuperframes;
        Uint16 totalLinks;
        Uint16 totalDiagnostics;

        void updateJoinPriority() {

            if (capabilities.isBackbone()) {
                //test asd : make them all join through proxy
                this->joinPriority = 0;
                return;
            }

            if (capabilities.isGateway()) {
                return;
            }

            uint16_t linkPriority = (uint16_t) ((usedLinks * 15.0) / totalLinks);
            uint16_t routePriority = (uint16_t) ((usedRoutes * 15.0) / totalRoutes);

            uint16_t maxP = 15;

            this->joinPriority = std::min(std::max(linkPriority, routePriority), maxP);

        }

    public:

        MetaDataAttributes() {
            joinPriority = 0;

            isTotalServicesSet = false;
            isTotalRoutesSet = false;
            isTotalSourceRoutesSet = false;
            isTotalGraphsSet = false;
            isTotalGraphNeighborsSet = false;
            isTotalSuperframesSet = false;
            isTotalLinksSet = false;
            isTotalDiagnosticsSet = false;

            usedServices = DEFAULT_USED_SERVICES;
            totalServices = 4;

            usedRoutes = DEFAULT_USED_ROUTES;
            totalRoutes = 8;

            usedSourceRoutes = DEFAULT_USED_SOURCE_ROUTES;
            totalSourceRoutes = 8;

            usedGraphs = DEFAULT_USED_GRAPHS;
            totalGraphs = 4;

            usedGraphNeighbors = DEFAULT_USED_NEIGHBORS;
            totalGraphNeighbors = 8;

            maxNeighbors = 16;

            usedSuperframes = DEFAULT_USED_SUPERFRAMES;
            totalSuperframes = 5;

            usedLinks = DEFAULT_USED_LINKS;
            totalLinks = 15;

            usedDiagnostics = 1;
            totalDiagnostics = 3;
        }

        void setJoinPriority(uint8_t joinPriority_) {
            this->joinPriority = joinPriority_;
        }

        uint8_t getJoinPriority() {
            return this->joinPriority;
        }

        /**
         * This method calculates a join priority that will consider the resources needed by a join flow in progress.
         */
        uint8_t getJoinInProgressPriority(uint16_t currentJoinsInProgressNo) {
            if (capabilities.isBackbone()) {
                return 0;
            }

            uint16_t linkPriority = (uint16_t) (((usedLinks + currentJoinsInProgressNo * (DEFAULT_USED_LINKS - 1))
                        * 15.0) / totalLinks);
            uint16_t graphPriority = (uint16_t) (((usedGraphs + currentJoinsInProgressNo * DEFAULT_USED_GRAPHS) * 15.0)
                        / totalGraphs);
            uint16_t maxP = 15;

            return std::min(std::max(linkPriority, graphPriority), maxP);
        }

        void init(const Capabilities& capabilities) {
            this->capabilities = capabilities;
            euidAddress = capabilities.euidAddress;

            usedServices = DEFAULT_USED_SERVICES;
            usedRoutes = DEFAULT_USED_ROUTES;
            usedSourceRoutes = DEFAULT_USED_SOURCE_ROUTES;
            usedGraphs = DEFAULT_USED_GRAPHS;
            usedGraphNeighbors = DEFAULT_USED_NEIGHBORS;
            usedSuperframes = DEFAULT_USED_SUPERFRAMES;
            usedLinks = DEFAULT_USED_LINKS;
            usedDiagnostics = 1;

            totalServices = 0xFFFF;
            totalRoutes = 0xFFFF;
            totalSourceRoutes = 0xFFFF;
            totalGraphs = 0xFFFF;
            totalGraphNeighbors = 0xFFFF;
            totalSuperframes = 0xFFFF;
            totalLinks = 0xFFFF;
            totalDiagnostics = 0xFFFF;

            if (capabilities.isGateway()) {
                totalServices = 250;
                totalRoutes = 0;
                totalSourceRoutes = 0;
                totalGraphs = 0;
                totalGraphNeighbors = 0;
                totalSuperframes = 0;
                totalLinks = 0;
                totalDiagnostics = 0;
            } else if (capabilities.isBackbone()) {
                totalServices = 3;
                totalRoutes = 14;
                totalSourceRoutes = 14;
                totalGraphs = 14;
                totalGraphNeighbors = 8;
                totalSuperframes = 10;
                totalLinks = 30;
                totalDiagnostics = 1;
            } else if (capabilities.isRouting()) {
                totalServices = 4;
                totalRoutes = 4;
                totalSourceRoutes = 4;
                totalGraphs = 4;
                totalGraphNeighbors = 8;
                totalSuperframes = 5;
                totalLinks = 15;
                totalDiagnostics = 3;
            } else if (capabilities.isFieldDevice()) {
                totalServices = 3;
                totalRoutes = 2;
                totalSourceRoutes = 2;
                totalGraphs = 2;
                totalGraphNeighbors = 2;
                totalSuperframes = 3;
                totalLinks = 9;
                totalDiagnostics = 2;
            }
        }

        bool getIsTotalServicesSet() const {
            return isTotalServicesSet;
        }

        void setIsTotalServicesSet(bool isTotalServicesSet) {
            this->isTotalServicesSet = isTotalServicesSet;
        }

        bool getIsTotalRoutesSet() const {
            return isTotalRoutesSet;
        }

        void setIsTotalRoutesSet(bool isTotalRoutesSet) {
            this->isTotalRoutesSet = isTotalRoutesSet;
        }

        bool getIsTotalSourceRoutesSet() const {
            return isTotalSourceRoutesSet;
        }

        void setIsTotalSourceRoutesSet(bool isTotalSourceRoutesSet) {
            this->isTotalSourceRoutesSet = isTotalSourceRoutesSet;
        }

        bool getIsTotalGraphsSet() const {
            return isTotalGraphsSet;
        }

        void setIsTotalGraphsSet(bool isTotalGraphsSet) {
            this->isTotalGraphsSet = isTotalGraphsSet;
        }

        bool getIsTotalGraphNeighborsSet() const {
            return isTotalGraphNeighborsSet;
        }

        void setIsTotalGraphNeighborsSet(bool isTotalGraphNeighborsSet) {
            this->isTotalGraphNeighborsSet = isTotalGraphNeighborsSet;
        }

        bool getIsTotalSuperframesSet() const {
            return isTotalSuperframesSet;
        }

        void setIsTotalSuperframesSet(bool isTotalSuperframesSet) {
            this->isTotalSuperframesSet = isTotalSuperframesSet;
        }

        bool getIsTotalLinksSet() const {
            return isTotalLinksSet;
        }

        void setIsTotalLinksSet(bool isTotalLinksSet_) {
            this->isTotalLinksSet = isTotalLinksSet_;
        }

        bool getIsTotalDiagnosticsSet() const {
            return isTotalDiagnosticsSet;
        }

        void setIsTotalDiagnosticsSet(bool isTotalDiagnosticsSet) {
            this->isTotalDiagnosticsSet = isTotalDiagnosticsSet;
        }

        bool updateTotalAndUsedLinksFromRemaining(uint16_t remainingLinksNo) {
            LOG_DEBUG("updateTotalAndUsedLinksFromRemaining remainingLinksNo= " << (int) remainingLinksNo
                        << ", isTotalLinksSet=" << (int) isTotalLinksSet);
            if (isTotalLinksSet == false) {

                if (capabilities.isBackbone()) {
                    // for backbone there is no default link used
                    totalLinks = remainingLinksNo + 1;
                } else {
                    totalLinks = remainingLinksNo + DEFAULT_USED_LINKS;
                }

                isTotalLinksSet = true;

                usedLinks = DEFAULT_USED_LINKS;
                return true;
            }

            usedLinks = totalLinks - remainingLinksNo;

            updateJoinPriority();

            return false;
        }

        bool updateTotalAndUsedServicesFromRemaining(uint16_t remainingServicesNo) {
            LOG_DEBUG("updateTotalAndUsedServicesFromRemaining remainingServicesNo= " << (int) remainingServicesNo
                        << ", isTotalServicesSet=" << (int) isTotalServicesSet);
            if (isTotalServicesSet == false) {
                totalServices = remainingServicesNo + DEFAULT_USED_SERVICES;
                isTotalServicesSet = true;

                usedServices = DEFAULT_USED_SERVICES;
                return true;
            }

            usedServices = totalServices - remainingServicesNo;

            updateJoinPriority();

            return false;
        }

        bool updateTotalAndUsedGraphsFromRemaining(uint16_t remainingGraphsNo) {
            LOG_DEBUG("updateTotalAndUsedGraphsFromRemaining remainingGraphsNo= " << (int) remainingGraphsNo
                        << ", isTotalGraphsSet=" << (int) isTotalGraphsSet);
            if (isTotalGraphsSet == false) {
                totalGraphs = remainingGraphsNo + DEFAULT_USED_GRAPHS;
                isTotalGraphsSet = true;

                usedGraphs = DEFAULT_USED_GRAPHS;
                return true;
            }

            usedGraphs = totalGraphs - remainingGraphsNo;

            updateJoinPriority();

            return false;
        }

        bool updateTotalAndUsedGraphNeighborsFromRemaining(uint16_t remainingGraphNeighborsNo) {
            LOG_DEBUG("updateTotalAndUsedGraphNeighborsFromRemaining remainingGraphNeighborsNo= "
                        << (int) remainingGraphNeighborsNo << ", isTotalGraphNeighborsSet="
                        << (int) isTotalGraphNeighborsSet);
            if (isTotalGraphNeighborsSet == false) {
                totalGraphNeighbors = remainingGraphNeighborsNo + DEFAULT_USED_GRAPHS;
                isTotalGraphNeighborsSet = true;

                usedGraphNeighbors = DEFAULT_USED_GRAPHS;
                return true;
            }

            usedGraphNeighbors = totalGraphNeighbors - remainingGraphNeighborsNo;

            updateJoinPriority();

            return false;
        }

        bool updateTotalAndUsedSuperframesFromRemaining(uint16_t remainingSuperframesNo) {
            LOG_DEBUG("updateTotalAndUsedSuperframesFromRemaining remainingSuperframesNo= "
                        << (int) remainingSuperframesNo << ", isTotalSuperframesSet=" << (int) isTotalSuperframesSet);
            if (isTotalSuperframesSet == false) {
                totalSuperframes = remainingSuperframesNo + DEFAULT_USED_SUPERFRAMES;
                isTotalSuperframesSet = true;

                usedSuperframes = DEFAULT_USED_SUPERFRAMES;
                return true;
            }

            usedSuperframes = totalSuperframes - remainingSuperframesNo;

            updateJoinPriority();

            return false;
        }

        bool updateTotalAndUsedRoutesFromRemaining(uint16_t remainingRoutesNo) {
            LOG_DEBUG("updateTotalAndUsedRoutesFromRemaining remainingRoutesNo= " << (int) remainingRoutesNo
                        << ", isTotalRoutesSet=" << (int) isTotalRoutesSet);
            if (isTotalRoutesSet == false) {
                totalRoutes = remainingRoutesNo + DEFAULT_USED_ROUTES;
                isTotalRoutesSet = true;

                usedRoutes = DEFAULT_USED_ROUTES;
                return true;
            }

            usedRoutes = totalRoutes - remainingRoutesNo;

            updateJoinPriority();

            return false;
        }

        bool updateTotalAndUsedSourceRoutesFromRemaining(uint16_t remainingSourceRoutesNo) {
            LOG_DEBUG("updateTotalAndUsedSourceRoutesFromRemaining remainingSourceRoutesNo= "
                        << (int) remainingSourceRoutesNo << ", isTotalSourceRoutesSet=" << (int) isTotalSourceRoutesSet);
            if (isTotalSourceRoutesSet == false) {
                totalSourceRoutes = remainingSourceRoutesNo + DEFAULT_USED_SOURCE_ROUTES;
                isTotalSourceRoutesSet = true;

                usedSourceRoutes = DEFAULT_USED_SOURCE_ROUTES;
                return true;
            }

            usedSourceRoutes = totalSourceRoutes - remainingSourceRoutesNo;

            updateJoinPriority();

            return false;
        }

        Address64 getEuidAddress() {
            return euidAddress;
        }

        Uint16 getUsedServices() {
            return usedServices;
        }
        void setUsedServices(Uint16 usedServices_) {
            usedServices = usedServices_;
        }

        Uint16 getUsedRoutes() {
            return usedRoutes;
        }
        void setUsedRoutes(Uint16 usedRoutes_) {
            usedRoutes = usedRoutes_;
        }

        Uint16 getUsedSourceRoutes() {
            return usedSourceRoutes;
        }
        void setUsedSourceRoutes(Uint16 usedSourceRoutes_) {
            usedSourceRoutes = usedSourceRoutes_;
        }

        Uint16 getUsedGraphs() {
            return usedGraphs;
        }
        void setUsedGraphs(Uint16 usedGraphs_) {
            usedGraphs = usedGraphs_;
        }

        Uint16 getUsedGraphNeighbors() {
            return usedGraphNeighbors;
        }
        void setUsedGraphNeighbors(Uint16 usedNeighbors_) {
            usedGraphNeighbors = usedNeighbors_;
        }

        Uint16 getUsedSuperframes() {
            return usedSuperframes;
        }
        void setUsedSuperframes(Uint16 usedSuperframes_) {
            usedSuperframes = usedSuperframes_;
        }

        Uint16 getUsedLinks() {
            return usedLinks;
        }

        void setUsedLinks(Uint16 usedLinks_) {
            usedLinks = usedLinks_;
        }

        Uint16 getUsedDiagnostics() {
            return usedDiagnostics;
        }
        void setUsedDiagnostics(Uint16 usedDiagnostics_) {
            usedDiagnostics = usedDiagnostics_;
        }

        Uint16 getTotalServices() {
            return totalServices;
        }
        void setTotalServices(Uint16 totalServices_) {
            totalServices = totalServices_;
        }

        Uint16 getTotalRoutes() {
            return totalRoutes;
        }
        void setTotalRoutes(Uint16 totalRoutes_) {
            totalRoutes = totalRoutes_;
        }

        Uint16 getTotalSourceRoutes() {
            return totalSourceRoutes;
        }
        void setTotalSourceRoutes(Uint16 totalSourceRoutes_) {
            totalSourceRoutes = totalSourceRoutes_;
        }

        Uint16 getTotalGraphs() {
            return totalGraphs;
        }
        void setTotalGraphs(Uint16 totalGraphs_) {
            totalGraphs = totalGraphs_;
        }

        Uint16 getTotalGraphNeighbors() {
            return totalGraphNeighbors;
        }
        void setTotalGraphNeighbors(Uint16 totalNeighbors_) {
            totalGraphNeighbors = totalNeighbors_;
        }

        Uint16 getTotalSuperframes() {
            return totalSuperframes;
        }
        void setTotalSuperframes(Uint16 totalSuperframes_) {
            totalSuperframes = totalSuperframes_;
        }

        Uint16 getTotalLinks() {
            return totalLinks;
        }
        void setTotalLinks(Uint16 totalLinks_) {
            totalLinks = totalLinks_;
        }

        Uint16 getMaxNeighbors() {
            return maxNeighbors;
        }
        void setMaxNeighbors(Uint16 maxNeighbors_) {
            maxNeighbors = (Uint16)(0.9 * maxNeighbors_);   // reduce max neighbors with 10% because of an issue of computing max neighbors
        }


        Uint16 getTotalDiagnostics() {
            return totalDiagnostics;
        }
        void setTotalDiagnostics(Uint16 totalDiagnostics_) {
            totalDiagnostics = totalDiagnostics_;
        }

        void resetAttributes() {
            usedServices = 0;
            usedRoutes = 0;
            usedSourceRoutes = 0;
            usedGraphs = 0;
            usedGraphNeighbors = 0;
            usedSuperframes = 0;
            usedLinks = 0;
            usedDiagnostics = 1;
        }

        std::string toString() {
            std::ostringstream stream;
            stream << "JoinPriority= " << (int) joinPriority;
            stream << ", Services= " << (int) usedServices << "/" << (int) totalServices;
            stream << ", Routes=" << (int) usedRoutes << "/" << (int) totalRoutes;
            stream << ", SourceRoutes=" << (int) usedSourceRoutes << "/" << (int) totalSourceRoutes;
            stream << ", Graphs=" << (int) usedGraphs << "/" << (int) totalGraphs;
            stream << ", GraphNeighbors=" << (int) usedGraphNeighbors << "/" << (int) totalGraphNeighbors;
            stream << ", Superframes=" << (int) usedSuperframes << "/" << (int) totalSuperframes;
            stream << ", Links=" << (int) usedLinks << "/" << (int) totalLinks;
            stream << ", Diagnostics=" << (int) usedDiagnostics << "/" << (int) totalDiagnostics;

            return stream.str();
        }
};

}
}

#endif /* METADATAATTRIBUTES_H_ */
