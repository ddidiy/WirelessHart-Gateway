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

#include "Security/MockKeyGenerator.h"

namespace hart7 {

namespace security {

MockKeyGenerator::MockKeyGenerator()
{
}

MockKeyGenerator::~MockKeyGenerator()
{
}

SecurityKey MockKeyGenerator::generateKey()
{
    SecurityKey key;

    key.loadString("C0 C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF");
    return key;
}

Uint32 MockKeyGenerator::getNextChallenge()
{
    return 2;
}

}
}
