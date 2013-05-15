var methods = ["config.getGroupVariables",
               "config.setVariable",
               "isa100.sm.getLogLevel",
               "isa100.sm.setLogLevel",
               "misc.applyConfigChanges",
               "user.logout"];


function InitNetworkManagerPage() {
    SetPageCommonElements();
    InitJSON();
    GetData();
}
             
               
function SaveData() {
    ClearOperationResult("spnOperationResult");
    
    if(ValidateInput()) {    	
        var networkTag  = document.getElementById("txtNetworkTag"); 
    	var maxDeviceNumber = document.getElementById("txtMaxDeviceNumber");
    	var managementBandwidth = document.getElementById("txtManagementBandwidth");
	    var gatewayBandwidth = document.getElementById("txtGatewayBandwidth");
	    var joinBandwidth = document.getElementById("txtJoinBandwidth");
	    var healthReportsPeriod = document.getElementById("txtHealthReportsPeriod");
	    var discoveryReportsPeriod = document.getElementById("txtDiscoveryReportsPeriod");
	    var keepAlivePeriod = document.getElementById("txtKeepAlivePeriod");	    
	    
        var saveFailed = false;
        
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            if (!service.config.setVariable({configFile : FILE_CONFIG_INI, group : "NETWORK_MANAGER", varName : "NETWORK_MANAGER_TAG", varValue: networkTag.value})) {
                alert("Error saving Network Manager Tag !");
                saveFailed = true;
            }
            if (!service.config.setVariable({configFile : FILE_CONFIG_INI, group : "NETWORK_MANAGER", varName : "NETWORK_MAX_NODES", varValue: maxDeviceNumber.value})) {
                alert("Error saving Max device number (NSD) !");
                saveFailed = true;
            }
            if (!service.config.setVariable({configFile : FILE_CONFIG_INI, group : "NETWORK_MANAGER", varName : "MANAGEMENT_BANDWIDTH", varValue: managementBandwidth.value})) {
                alert("Error saving Management bandwidth (s) !");
                saveFailed = true;                
            }
            if (!service.config.setVariable({configFile : FILE_CONFIG_INI, group : "NETWORK_MANAGER", varName : "GATEWAY_BANDWIDTH", varValue: gatewayBandwidth.value})) {
                alert("Error saving Gateway bandwidth (s) !");
                saveFailed = true; 
            }
            if (!service.config.setVariable({configFile : FILE_CONFIG_INI, group : "NETWORK_MANAGER", varName : "JOIN_BANDWIDTH", varValue: joinBandwidth.value})) {
                alert("Error saving Join bandwidth (s) !");
                saveFailed = true; 
            }
            if (!service.config.setVariable({configFile : FILE_CONFIG_INI, group : "NETWORK_MANAGER", varName : "HEALTH_REPORTS_PERIOD", varValue: healthReportsPeriod.value})) {
                alert("Error saving Health reports period (m) !");
                saveFailed = true; 
            }
            if (!service.config.setVariable({configFile : FILE_CONFIG_INI, group : "NETWORK_MANAGER", varName : "DISCOVERY_REPORTS_PERIOD", varValue: discoveryReportsPeriod.value})) {
                alert("Error saving Discovery reports period (m) !");
                saveFailed = true; 
            }
            if (!service.config.setVariable({configFile : FILE_CONFIG_INI, group : "NETWORK_MANAGER", varName : "KEEP_ALIVE_PERIOD", varValue: keepAlivePeriod.value})) {
                alert("Error saving Keep alive period (s)!");
                saveFailed = true; 
            }
            if (!service.config.setVariable({configFile : FILE_CONFIG_INI, group : "NETWORK_MANAGER", varName : "CHANNEL_MAP", varValue: GetChannelsList()})) {
                alert("Error saving Channel list !");
                saveFailed = true; 
            }
            if (!service.misc.applyConfigChanges({module : "WHart_NM.o"})) {
                alert("Error applying config changes !");
                saveFailed = true;
            }
        } catch(e) {
            HandleException(e, "Unexpected error saving values !");
            return;
        }
        if (!saveFailed) {
        	GetNetworkID();
            DisplayOperationResult("spnOperationResult", "Save completed successfully.");
        }
    }
}


function GetData() {
    ClearOperationResult("spnOperationResult");
    var networkTag  = document.getElementById("txtNetworkTag");
    var maxDeviceNumber = document.getElementById("txtMaxDeviceNumber");
  	var managementBandwidth = document.getElementById("txtManagementBandwidth");
    var gatewayBandwidth = document.getElementById("txtGatewayBandwidth");
    var joinBandwidth = document.getElementById("txtJoinBandwidth");
    var healthReportsPeriod  = document.getElementById("txtHealthReportsPeriod");
    var discoveryReportsPeriod  = document.getElementById("txtDiscoveryReportsPeriod");
    var keepAlivePeriod  = document.getElementById("txtKeepAlivePeriod");
    
    try  {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        var result = service.config.getGroupVariables({configFile : FILE_CONFIG_INI, group : "NETWORK_MANAGER"});
            
        for (var i=0; i<result.length; i++) {
            if(result[i].NETWORK_MANAGER_TAG != null) {
            	networkTag.value = result[i].NETWORK_MANAGER_TAG;
            }            
            if(result[i].NETWORK_MAX_NODES != null) {
                 maxDeviceNumber.value = result[i].NETWORK_MAX_NODES;
            }
            if(result[i].MANAGEMENT_BANDWIDTH != null) {
            	managementBandwidth.value = result[i].MANAGEMENT_BANDWIDTH;
            }
            if(result[i].GATEWAY_BANDWIDTH != null) {
            	gatewayBandwidth.value = result[i].GATEWAY_BANDWIDTH;
            }
            if(result[i].JOIN_BANDWIDTH != null) {
            	joinBandwidth.value = result[i].JOIN_BANDWIDTH;
            }
            if(result[i].HEALTH_REPORTS_PERIOD != null) {
            	healthReportsPeriod.value = result[i].HEALTH_REPORTS_PERIOD;
            }
            if(result[i].DISCOVERY_REPORTS_PERIOD != null) {
            	discoveryReportsPeriod.value = result[i].DISCOVERY_REPORTS_PERIOD;
            }
            if(result[i].KEEP_ALIVE_PERIOD != null) {
            	keepAlivePeriod.value = result[i].KEEP_ALIVE_PERIOD;
            }
            if(result[i].CHANNEL_MAP != null) {                    	
                SetChannelsList(result[i].CHANNEL_MAP);
            }
        }
    } catch(e) {
        HandleException(e, "Unexpected error reading values !");
    }    
}


function ValidateInput() {	
	var networkTag  = document.getElementById("txtNetworkTag");
	var maxDeviceNumber = document.getElementById("txtMaxDeviceNumber");
	var managementBandwidth = document.getElementById("txtManagementBandwidth");
	var gatewayBandwidth = document.getElementById("txtGatewayBandwidth");
    var joinBandwidth = document.getElementById("txtJoinBandwidth");
    var healthReportsPeriod = document.getElementById("txtHealthReportsPeriod");
    var discoveryReportsPeriod = document.getElementById("txtDiscoveryReportsPeriod");
    var keepAlivePeriod = document.getElementById("txtKeepAlivePeriod");
       
	if(!ValidateRequired(networkTag, "Network Manager Tag")) {
        return false;
    } else if (networkTag.length>32) {
    	alert("Invalid Network Manager Tag");
    }
    if(!ValidateRequired(maxDeviceNumber, "Max device number (NSD)")) {
        return false;
    }
    if(!ValidateNumber(maxDeviceNumber, "Max device number (NSD)", 1, 50)) {
        return false;
    }
    if(!ValidateRequired(managementBandwidth, "Management bandwidth (s)")) {
        return false;
    }
    if(!ValidateNumber(managementBandwidth, "Management bandwidth (s)", 1, 64)) {
        return false;
    }
    if(!ValidateRequired(gatewayBandwidth, "Gateway bandwidth (s)")) {
        return false;
    }
    if(!ValidateNumber(gatewayBandwidth, "Gateway bandwidth (s)", 1, 64)) {
        return false;
    }
    if(!ValidateRequired(joinBandwidth, "Join bandwidth (s)")) {
        return false;
    }
    if(!ValidateNumber(joinBandwidth, "Join bandwidth (s)", 1, 64)) {
        return false;
    }
    if(!ValidateRequired(healthReportsPeriod, "Health reports period (m)")) {
        return false;
    }
    if(!ValidateNumber(healthReportsPeriod, "Health reports period (m)", 1, 60*24)) {
        return false;
    }
    if(!ValidateRequired(discoveryReportsPeriod, "Discovery reports period (m)")) {
        return false;
    }
    if(!ValidateNumber(discoveryReportsPeriod, "Discovery reports period (m)", 1, 60*24)) {
        return false;
    }
    if(!ValidateRequired(keepAlivePeriod, "Keep alive period (s)")) {
        return false;
    }
    if(!ValidateNumber(keepAlivePeriod, "Keep alive period", 1, 64)) {
        return false;
    }
    return true;
}

function GetChannelsList() {
    var intValue = 0;    
    for (var i=0; i<15; i++) {
        if (document.getElementById("chkChannel" + i).checked) {
        	intValue = intValue | Math.pow(2,i);
        }
    }       
    return  DecToHex(intValue);
}

function SetChannelsList(hexVal) {
	var intValue = HexToDec(hexVal);
    for (var i=0; i<15 ; i++){    	
    	if (((intValue >> i) & 1) == 1){
    		document.getElementById("chkChannel" + i).checked = true;	
    	} else {
    		document.getElementById("chkChannel" + i).checked = false;
    	}        
    }
}
