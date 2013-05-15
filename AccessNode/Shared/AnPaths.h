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

/********************************************************************
* Name:        AnPaths.h
* Author:      Claudiu Hobeanu
* Date:        9/5/2005   14:57
* Description: contain defines for the main paths and files used by AN
* @addtogroup libshared
* @{
*********************************************************************/

#ifndef _AN_PATHS_H
#define _AN_PATHS_H

#ifdef FS_TREE_REL
#define NIVIS_FIRMWARE		"./"
#define NIVIS_PROFILE		"../profile/"
#define NIVIS_ACTIVITY_FILES	"../activity_files/"
#define NIVIS_TMP		"../tmp/"
#endif

#ifdef HW_I386
#ifdef RELEASE_ISA
#define NIVIS_FIRMWARE		"/opt/isa100/firmware/"
#define NIVIS_PROFILE		"/etc/opt/isa100/profile/"
#define NIVIS_ACTIVITY_FILES	"/var/opt/activity_files_isa100/"
#define NIVIS_TMP	"/tmp/isa100/"
#undef FS_TREE_REL
#endif //RELEASE_ISA
#ifdef RELEASE_WHART
#define NIVIS_FIRMWARE		"/opt/whart/firmware/"
#define NIVIS_PROFILE		"/etc/opt/whart/profile/"
#define NIVIS_ACTIVITY_FILES	"/var/opt/activity_files_whart/"
#define NIVIS_TMP	"/tmp/whart/"
#undef FS_TREE_REL
#endif //RELEASE_WHART
#endif //HW_I386

#ifndef NIVIS_FIRMWARE
#define NIVIS_FIRMWARE		"/access_node/firmware/"
#endif

#ifndef NIVIS_PROFILE
#define NIVIS_PROFILE		"/access_node/profile/"
#endif

#ifndef NIVIS_ACTIVITY_FILES
#define NIVIS_ACTIVITY_FILES	"/access_node/activity_files/"
#endif

#ifndef NIVIS_TMP
#define NIVIS_TMP	"/tmp/"
#endif


//to be removed in time
#define ACTIVITY_DATA_PATH	"/access_node/activity_files/"

//#define NIVIS_FIRMWARE		"/access_node/"
//#define NIVIS_PROFILE		"/access_node/"
//#define ACTIVITY_DATA_PATH	"/access_node/"



//pipes
#define PIPE_000    NIVIS_TMP "pipe_000.pipe"
#define PIPE_001    NIVIS_TMP "pipe_001.pipe"
#define PIPE_010    NIVIS_TMP "pipe_010.pipe"
#define PIPE_011    NIVIS_TMP "pipe_011.pipe"
#define PIPE_1XX    NIVIS_TMP "pipe_1xx.pipe"

////////////////////////////////////////////////////////////////////////// HM pipes
#define PIPE_2HISTORY   NIVIS_TMP "2HISTORY.pipe"
#define PIPE_HM2CC      NIVIS_TMP "HM2CC.pipe"
#define PIPE_CC2HM		NIVIS_TMP "CC2HM.pipe"
//////////////////////////////////////////////////////////////////////////

#define PIPE_2CM		NIVIS_TMP "2CM.pipe"
#define PIPE_2LOC_CMD	NIVIS_TMP "2LOC_CMD.pipe"
#define PIPE_DNC2CC     NIVIS_TMP "DNC2CC.pipe"
#define PIPE_RF2CC      NIVIS_TMP "RF2CC.pipe"
#define PIPE_CC2RF      NIVIS_TMP "CC2RF.pipe"
#define PIPE_CC2RA      NIVIS_TMP "CC2RA.pipe"
#define PIPE_RA2CC      NIVIS_TMP "RA2CC.pipe"
#define PIPE_2MESH		NIVIS_TMP "2mesh.pipe"
#define PIPE_2LOCAL		NIVIS_TMP "2local.pipe"
#define PIPE_2EMAIL_SENDER		NIVIS_TMP "2email_sender.pipe"
#define PIPE_EMAIL_SENDER_ACK_RESP	NIVIS_TMP "email_sender_ack.pipe"

//END PIPES

//log files...
#define S_LOG_FILE			NIVIS_TMP "scheduler.log"
#define RA_LOG_FILE			NIVIS_TMP "remote_access.log"
#define DNC_LOG_FILE		NIVIS_TMP "node_connection.log"
#define UI_LOG_FILE			NIVIS_TMP "user_interface.log"
#define CCCM_LOG_FILE		NIVIS_TMP "wan.log"
#define EVT_LOG_FILE		NIVIS_TMP "rule_file_mgr.log"
#define HM_LOG_FILE			NIVIS_TMP "history.log"
#define DNS_LOG_FILE		NIVIS_TMP "dn_simulator.log"
#define RF_LOG_FILE			NIVIS_TMP "rf_repeater.log"
#define SDI_PASS_LOG_FILE	NIVIS_TMP "sdi12_pass.log"
#define CM_LOG_FILE			NIVIS_TMP "comm_mgr.log"

#define EVENTS_FILE     NIVIS_TMP "rule_file"

#define APM_SH_FILE		NIVIS_FIRMWARE "apm.sh"

//main config file in the same folder with app...
#define INI_FILE			NIVIS_PROFILE "config.ini"
#define FW_CFG_FILE			NIVIS_FIRMWARE"fw.cfg"
#define LOCAL_ACTIONS_FILE	NIVIS_PROFILE "local_actions.cfg"
#define DST_CFG_FILE		NIVIS_PROFILE "DST.cfg"

#define INTERFACES_FILE		ACTIVITY_DATA_PATH "interfaces.ini"

#define PARAM_MAP_FILE		ACTIVITY_DATA_PATH "param_map.cfg"

//nc use these files from TMP and if they are dirty and for a while -> cp to ACTIVITY_DATA
//#define MESH_NODES_FILE		"MeshNodes.ini"
//#define MESH_LINKS_FILE		"MeshLinks.ini"

#define MESH_REPORT_FILE	NIVIS_TMP "MeshReport.txt"

#define DN_LOCAL_ADDR_FILE	"DN_Local_Address"
#define DN_LINKS_FILE		"DN_Links"

/// @}
#endif
