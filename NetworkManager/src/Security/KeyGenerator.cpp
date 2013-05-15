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

#include "Security/KeyGenerator.h"

namespace hart7 {

namespace security {

KeyGenerator::KeyGenerator()
{
    this->nextChallenge = 100;
    srand(0x69 ^ time(NULL));
}

KeyGenerator::~KeyGenerator()
{
}

SecurityKey KeyGenerator::generateKey()
{
    SecurityKey securityKey;

    for (int i = 0; i < SecurityKey::LENGTH; i++)
    {
        securityKey.value[i] = rand() % 0xFF;
    }

    return securityKey;
}

Uint32 KeyGenerator::getNextChallenge()
{
    nextChallenge = (nextChallenge + 1) % 0xFFFFFFF0;
    LOG_DEBUG("getNextChallenge() : " << (long long) nextChallenge);
    return nextChallenge;
}

void KeyGenerator::setNextChallenge(Uint32 nextChallenge)
{
    LOG_DEBUG("setNextChallenge : " << (long long) nextChallenge);
    this->nextChallenge = nextChallenge;
}
}
}
