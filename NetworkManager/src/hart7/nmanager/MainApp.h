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

/**
 * MainApp.h
 *
 *  Created on: Dec 05, 2008
 *      Author: Radu Pop
 */

#ifndef MAINAPP_H_
#define MAINAPP_H_

#include "ManagerStack.h"
#include "../../NMSettingsLogic.h"
#include "../util/NMLog.h"
#include <boost/function.hpp>

namespace hart7 {

namespace nmanager {

/**
 * Main application.
 */
class MainApp
{
        LOG_DEF("h7.n.MainApp")
        ;

    public:

        MainApp(const std::string& pathToLog, const std::string& pathToScript);

        virtual ~MainApp();

        /**
         * Starts application, processing command line arguments
         */
        void Run(int argc, char* argv[]);

        /**
         * Reload method called from the HUP handler. Only should mark reload, and do the actual reloading from the main cycle.
         */
        void Reload();

        /**
         * Actual reload handler that gets called from the main cycle.
         */
        void ReloadConfigFiles();

    private:

        std::string pathToLog;

        std::string pathToScript;

        hart7::util::NMSettingsLogic settingsLogic;

        boost::function0<void> managerMarkForReloadingConfigFilesHandler;

        boost::function0<void> managerHupHandler;

};

}
}

#endif /* MAINAPP_H_ */
