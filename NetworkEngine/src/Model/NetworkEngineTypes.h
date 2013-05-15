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
 * NetworkEngineTypes.h
 *
 *  Created on: Aug 18, 2008
 *      Author: catalin.pop
 */

#ifndef NETWORKENGINETYPES_H_
#define NETWORKENGINETYPES_H_
#include "Common/NETypes.h"
#include "Common/NEAddress.h"
#include <iomanip>
#include <map>

namespace NE {
namespace Model {

struct ChannelDiagnostic {
        // for radio channel x:
        Uint16 txSuccessful; // count of successful transmissions (broadcast or ACK/NAK received)
        Uint8 txCCABackoff; // count of attempted transmissions that were aborted due to CCA
        Uint8 txFailed; // count of transmissions where no ACK/NAK was received in response

        void toString(std::ostringstream& stream) {
            stream << "{" << "txSuccessful: " << std::hex << std::setw(2) << std::setfill('0') << (int) txSuccessful
                        << ", txCCABackoff: " << std::hex << std::setw(2) << std::setfill('0') << (int) txCCABackoff
                        << ", txFailed: " << std::hex << std::setw(2) << std::setfill('0') << (int) txFailed << "}";
        }
        void toIndentString(std::ostringstream& stream) {
            stream << "{" << "S=" << std::hex << std::setw(2) << std::setfill('0') << (int) txSuccessful << ", CCA=" << std::hex
                        << std::setw(2) << std::setfill('0') << (int) txCCABackoff << ", F=" << std::hex << std::setw(2)
                        << std::setfill('0') << (int) txFailed << "}";
        }
};

struct ChannelState {
        bool currentState;
        bool newState;
        Uint16 successSum;
        Uint16 ccaSum;
        Uint16 failedSum;

        ChannelState() :
            currentState(false), newState(false), successSum(0), ccaSum(0), failedSum(0) {
        }

};

typedef std::vector<ChannelDiagnostic> ChannelDiagnosticList;

typedef std::map<Uint8, ChannelState> SuperframeChannelsState;

struct ChannelsPublicationData {
        Address64 publicationDeviceAddress;
        ChannelDiagnosticList channelsDiagnosticsList;

        ChannelsPublicationData(const Address64& publicationDeviceAddress_, ChannelDiagnosticList& channelsDiagnosticsList_) :
            publicationDeviceAddress(publicationDeviceAddress_), channelsDiagnosticsList(channelsDiagnosticsList_) {

        }
};

/** A list of Publication packages. */
typedef std::vector<ChannelsPublicationData> PublicationsList;

/** A map containing latest n publications for a device */
typedef std::map<Address64, PublicationsList> DevicesPublicationsMap;

}
}

#endif /* NETWORKENGINETYPES_H_ */
