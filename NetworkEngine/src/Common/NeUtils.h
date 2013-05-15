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
 * NeUtils.h
 *
 *  Created on: Aug 12, 2009
 *      Author: Catalin Pop
 */

#ifndef NEUTILS_H_
#define NEUTILS_H_

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <string>

namespace NE {

namespace Common {

namespace NeUtils {

int parseLine(char* line) {
    int i = strlen(line);
    while (*line > '9')
        line++;
    line[i - 3] = '\0';
    i = atoi(line);
    return i;
}

int getVmSize() { //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "VmSize:", 7) == 0) {
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

int getVmRSS() { //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

void getStatusString(std::string& str) { //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    char line[128];

    while (fgets(line, 128, file) != NULL) {
        str.append(line);
    }
    fclose(file);
}

}

}

}

#endif /* NEUTILS_H_ */
