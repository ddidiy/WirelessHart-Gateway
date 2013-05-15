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
 * ScriptLineParser.h
 *
 *  Created on: Jul 22, 2010
 *      Author: andrei.petrut
 */

#ifndef SCRIPTLINEPARSER_H_
#define SCRIPTLINEPARSER_H_

#include <nlib/log.h>
#include <boost/tokenizer.hpp>

namespace hart7 {
namespace nmanager {


class DevicesManager;

/**
 * Parses a line of the script, and executes it.
 */
class ScriptLineParser
{

public:
	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	LOG_DEF("Script");

	/**
	 * Constructor.
	 */
	ScriptLineParser(hart7::nmanager::DevicesManager& devicesManager);

	/**
	 * Parses one line, and executes it.
	 */
	void ParseLine(const std::string& line);

private:

	void ProcessCommand(const std::string& line, Tokenizer::iterator begin, Tokenizer::iterator end);

private:
	boost::char_separator<char> tokenizerFunc;
	hart7::nmanager::DevicesManager& devicesManager;

};

}
}

#endif /* SCRIPTLINEPARSER_H_ */
