#!/bin/sh

. common.sh


rel_start_modules_config()
{
	#TAKE CARE: nc local must be started first. cc_comm, history, rf_repeater must be started before scheduler
	_BEFORE_START_="cp take_system_snapshot.sh /tmp,rm -f /tmp/*.pipe,rm -f /tmp/shared_var.shd,watchdog&" #
	_AFTER_START_="sys_monitor.sh&,ln -sf /tmp/nm.log /tmp/WHart_NM.o.log"
	#_AFTER_START_=" "
	
	_NC_LOCAL_=""
	
	#TAKE CARE: must have a space between module and &, for correct wdt watch
	#MDL_SystemManager="sm-run.sh cleanup_on_boot, sm-run.sh start SystemManager"
	MDL_SystemManager="WHart_NM.o /access_node/profile/ > /tmp/SystemManagerProcess.log 2>&1 &"
	#MDL_MonitorHost="etc/start.sh"
	MDL_MonitorHost="MonitorHost &"
	MDL_isa_gw="WHart_GW.o &"
	MDL_backbone="whaccesspoint &"
	MDL_scgi_svc="/access_node/firmware/www/wwwroot/scgi_svc &"
	MDL_modbus_gw="modbus_gw &"
	MDL_=""
	
	_SCHD_="scheduler &"
}

rel_stop_modules_config()
{
	# can this be shared with start.sh ?
	MODULES="watchdog whaccesspoint WHart_GW.o WHart_NM.o MonitorHost sys_monitor.sh scheduler modbus_gw sm_log_upload.sh sm_extra_logs_cleaner.sh scgi_svc "
	MOD_SML="watchdog whaccesspoint WHart_GW.o WHart_NM.o MonitorHost sys_monitor.sh scheduler modbus_gw sm_log_upload.sh sm_extra_logs_cleaner.sh"
	MOD_EXC_WTD="whaccesspoint WHart_GW.o WHart_NM.o MonitorHost sys_monitor.sh scheduler modbus_gw sm_log_upload.sh sm_extra_logs_cleaner.sh scgi_svc "
}


rel_renice_modules()
{
	Renice "watchdog"  -10
	Renice "whaccesspoint"  -9
	Renice "modbus_gw" -7
	Renice "scgi_svc" -5
	#every message goes through gw should have a priority >= NM
	Renice "WHart_GW.o" -3
	Renice "WHart_NM.o" -2
	Renice "MonitorHost" -1
}