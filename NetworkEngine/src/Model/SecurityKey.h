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

#ifndef SECURITYKEY_H_
#define SECURITYKEY_H_

#include "Common/NETypes.h"
#include "Misc/Convert/Convert.h"
#include "Misc/Marshall/Stream.h"

using namespace NE::Common;
using namespace NE::Misc::Convert;
using namespace NE::Misc::Marshall;

namespace NE {
namespace Model {

typedef Uint8 KeyID;

struct SecurityKey {

    public:

        static const Uint8 LENGTH = 16;

        Uint8 value[LENGTH];

        SecurityKey() {
            for (int i = 0; i < LENGTH; i++) {
                value[i] = 0;
            }
        }

        SecurityKey(const SecurityKey& other) {
            for (int i = 0; i < LENGTH; i++) {
                value[i] = other.value[i];
            }
        }

        //TODO:[andy] - changed mehtod to copy FROM the parameter to the current instance. Please notify me if the intent was the
        //	other way around.
        void copyKey(Uint8* value_) {
            for (int i = 0; i < LENGTH; i++) {
                value[i] = value_[i];
            }
        }

        void loadString(const char* text) {
            int tmp;
            const char* crt;

            crt = text;

            for (int i = 0; i < LENGTH; i++) {
                sscanf(crt, "%02X", &tmp);
                value[i] = (Uint8) tmp;
                crt += 2;
                if (*crt == ' ')
                {
                    crt++;
                }
            }
        }

        bool operator<(const SecurityKey &compare) const {
            for (int i = 0; i < LENGTH; i++) {
                if (value[i] != compare.value[i]) {
                    return value[i] < compare.value[i];
                }
            }

            return false;
        }

        SecurityKey& operator=(const SecurityKey& other) {
            for (int i = 0; i < LENGTH; i++) {
                value[i] = other.value[i];
            }

            return *this;
        }

        void marshall(OutputStream& stream) {
            for (Uint8 i = 0; i < LENGTH; i++) {
                stream.write(value[i]);
            }
        }

        void unmarshall(InputStream& stream) {
            for (Uint8 i = 0; i < LENGTH; i++) {
                stream.read(value[i]);
            }
        }

        std::string toString() {
            Bytes bytes(&value[0], &value[LENGTH]);
            return bytes2string(bytes);
        }
};

}
}

#endif /*SECURITYKEY_H_*/
