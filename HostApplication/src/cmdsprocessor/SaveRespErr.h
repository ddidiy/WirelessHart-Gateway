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

#ifndef SAVERESPERR_H_
#define SAVERESPERR_H_


#include <WHartHost/model/DBCommand.h>

namespace hart7 {
namespace hostapp {

class DBCommandsManager;

class SaveRespErr
{
private:
	SaveRespErr();
	SaveRespErr(const SaveRespErr&);

public:
	SaveRespErr(DBCommandsManager &p_rDbCmdManager, DBCommand &p_rDbCommand): 
	  m_rDbCmdManager(p_rDbCmdManager), m_rDbCommand(p_rDbCommand){}

//save db command error
public:
	void CommandFailed(DBCommand::ResponseStatus errorCode);


//check errors
public:
	// wireless
	static bool IsErr_769(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_778(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_779(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_780(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_783(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_784(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_785(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_787(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_788(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_789(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_790(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_791(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_800(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_801(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_802(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_803(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_814(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_818(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_832(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_833(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_834(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_837(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_839(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_840(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_965(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_966(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_967(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_968(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_969(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_970(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_971(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_973(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_974(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_975(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_976(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_977(int p_nHostErrCode, int p_nWhErrCode);
	//universal
	static bool IsErr_000(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_001(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_002(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_003(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_009(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_020(int p_nHostErrCode, int p_nWhErrCode);
	//common practice
	static bool IsErr_033(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_074(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_084(int p_nHostErrCode, int p_nWhErrCode);
    static bool IsErr_101(int p_nHostErrCode, int p_nWhErrCode);
    static bool IsErr_102(int p_nHostErrCode, int p_nWhErrCode);
    static bool IsErr_103(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_104(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_105(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_107(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_108(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_109(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_111(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_112(int p_nHostErrCode, int p_nWhErrCode);
	static bool IsErr_178(int p_nHostErrCode, int p_nWhErrCode);

	static std::string GetErrText(int command, int p_nHostErrCode, int p_nWhErrCode);

// save errors
public:
	// wireless
	void SaveErr_778(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_779(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_785(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_802(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_803(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_814(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_832(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_833(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_834(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_837(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_839(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_840(int p_nHostErrCode, int p_nWhErrCode);
	//universal
	void SaveErr_000(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_001(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_002(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_003(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_009(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_020(int p_nHostErrCode, int p_nWhErrCode);
	//common practice
	void SaveErr_033(int p_nHostErrCode, int p_nWhErrCode);
    void SaveErr_074(int p_nHostErrCode, int p_nWhErrCode);
    void SaveErr_084(int p_nHostErrCode, int p_nWhErrCode);
    void SaveErr_101(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_103(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_104(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_105(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_107(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_108(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_109(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_111(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_112(int p_nHostErrCode, int p_nWhErrCode);
	void SaveErr_178(int p_nHostErrCode, int p_nWhErrCode);

//
private:
	DBCommandsManager& m_rDbCmdManager;
	DBCommand& m_rDbCommand;
};

}
}

#endif
