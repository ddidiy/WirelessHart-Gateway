var methods = ["config.getGroupVariables",
               "config.setVariable",
               "misc.applyConfigChanges",
			   "user.logout"];
                   
function InitGatewayPage() {
    SetPageCommonElements();
    InitJSON();
    GetData();
}

function SaveData() {
    ClearOperationResult("spnOperationResult");
    
    if(ValidateInput()) {
    	var gwTag = document.getElementById("txtGWTag");    	
       	var txtCacheReadResponseTimeout = document.getElementById("txtCacheReadResponseTimeout");
       	var txtCacheBurstResponseTimeout = document.getElementById("txtCacheBurstResponseTimeout");   	
    	var appJoinKey = document.getElementById("txtAppJoinKey");
	    var appLoggingLevel = document.getElementsByName("rbAppLoggingLevel");
        var stackLoggingLevel = document.getElementsByName("rbStackLoggingLevel");
        
        var saveFailed = false;
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            if (!service.config.setVariable({configFile : FILE_GATEWAY_INI,group : "WH_GATEWAY", varName : "LONG_TAG", varValue: gwTag.value})) {
                alert("Error saving GW Tag !");
                saveFailed = true;   
            }            
            if (!service.config.setVariable({group : "WH_GATEWAY", varName : "CACHE_READ_RESP_TIMEOUT", varValue: txtCacheReadResponseTimeout.value})) {
                alert("Error saving GW RESP TIMEOUT !");
                saveFailed = true;   
            }
            if (!service.config.setVariable({group : "WH_GATEWAY", varName : "CACHE_BURST_RESP_TIMEOUT", varValue: txtCacheBurstResponseTimeout.value})) {
                alert("Error saving GW BURST RESP TIMEOUT !");
                saveFailed = true;   
            }            
            if (!service.config.setVariable({group : "WH_GATEWAY", varName : "AppJoinKey", varValue: appJoinKey.value})) {
                alert("Error saving App Join Key !");
                saveFailed = true;                                   
            }            
            
            var appLoggingLevelValue;
            for (var i=0; i < appLoggingLevel.length; i++) {
                if (appLoggingLevel[i].checked) {
                    appLoggingLevelValue = i + 1;
                }
            }            
            var stackLoggingLevelValue;
            for (var i=0; i < stackLoggingLevel.length; i++) {
                if (stackLoggingLevel[i].checked) {
                    stackLoggingLevelValue = i + 1;
                }
            }            
            if (!service.config.setVariable({group : "WH_GATEWAY", varName : "LOG_LEVEL_APP", varValue: appLoggingLevelValue})) {
                alert("Error saving App Logging level !");
                saveFailed = true;                   
            }            
            if (!service.config.setVariable({group : "WH_GATEWAY", varName : "LOG_LEVEL_STACK", varValue: stackLoggingLevelValue})) {
                alert("Error saving Stack Logging level !");
                saveFailed = true;                   
            }   
            
            if (!service.misc.applyConfigChanges ({module : "WHart_GW.o"})) {
                alert("Error applying config changes !");
                saveFailed = true;                   
            }            
        } catch(e) {
            HandleException(e, "Unexpected error saving values !");
            return;
        }
       
        if (!saveFailed) {
            DisplayOperationResult("spnOperationResult", "Save completed successfully.");
        }
    }
}


function GetData() {
    ClearOperationResult("spnOperationResult");
    
   	var gwTag = document.getElementById("txtGWTag");
   	var gwTag = document.getElementById("txtGWTag");   	
   	var txtCacheReadResponseTimeout = document.getElementById("txtCacheReadResponseTimeout");
   	var txtCacheBurstResponseTimeout = document.getElementById("txtCacheBurstResponseTimeout");
   	var appJoinKey = document.getElementById("txtAppJoinKey");
    var appLoggingLevel = document.getElementsByName("rbAppLoggingLevel");
    var stackLoggingLevel = document.getElementsByName("rbStackLoggingLevel");
    
    var appLoggingLevelValue;
    var stackLoggingLevelValue;
        
    try {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        var result = service.config.getGroupVariables({group : "WH_GATEWAY"});
        
        for (i = 0; i < result.length; i++) {
            if(result[i].CACHE_READ_RESP_TIMEOUT != null) {
            	txtCacheReadResponseTimeout.value = result[i].CACHE_READ_RESP_TIMEOUT;
            }
            if(result[i].CACHE_BURST_RESP_TIMEOUT != null) {
            	txtCacheBurstResponseTimeout.value = result[i].CACHE_BURST_RESP_TIMEOUT;
            }
            if(result[i].AppJoinKey != null) {
                appJoinKey.value = result[i].AppJoinKey;
            }
            if(result[i].LOG_LEVEL_APP != null) {
                appLoggingLevelValue = result[i].LOG_LEVEL_APP;
            }
            if(result[i].LOG_LEVEL_STACK != null) {
                stackLoggingLevelValue = result[i].LOG_LEVEL_STACK;
            }
        }       
        
        if (appLoggingLevelValue >= 1 && appLoggingLevelValue <= 3) {
            appLoggingLevel[appLoggingLevelValue-1].checked = true;
        }
        if (stackLoggingLevelValue >= 1 && stackLoggingLevelValue <= 3) {
            stackLoggingLevel[stackLoggingLevelValue-1].checked = true;
        }
        
        var result = service.config.getGroupVariables({configFile: FILE_GATEWAY_INI, group : 'WH_GATEWAY'});
        for (var i=0; i<result.length; i++){
            if(result[i].LONG_TAG != null) {
                gwTag.value = result[i].LONG_TAG;
                break;
            };        	
        };
        
    } catch(e) {
         HandleException(e, "Unexpected error reading values !");
    }    
}


function ValidateInput() {
   	var gwTag = document.getElementById("txtGWTag");
	var appJoinKey = document.getElementById("txtAppJoinKey");
    var txtCacheReadResponseTimeout = document.getElementById("txtCacheReadResponseTimeout");
    var txtCacheBurstResponseTimeout = document.getElementById("txtCacheBurstResponseTimeout");
	var appLoggingLevel = document.getElementsByName("rbAppLoggingLevel");
    var stackLoggingLevel = document.getElementsByName("rbStackLoggingLevel");

    if(!ValidateRequired(gwTag, "GW Tag")) {
        return false;
    }
    if(!ValidateRequired(txtCacheReadResponseTimeout, "Cache Read Response Timeout")) {
        return false;
    }
    if (!ValidateNumber(txtCacheReadResponseTimeout, "Cache Read Response Timeout", 1, 2000000000)) {
        return false;
    }
    if(!ValidateRequired(txtCacheBurstResponseTimeout, "Cache Burst Response Timeout")) {
        return false;
    }
    if (!ValidateNumber(txtCacheBurstResponseTimeout, "Cache Burst Response Timeout", 1, 2000000000)) {
        return false;
    }
    /*ValidateRequired(appJoinKey, "App Join Key") && */
    if(!Validate16BHex(appJoinKey, "App Join Key")) {
        return false;
    } 
    if(!ValidateRequiredRadio(appLoggingLevel, "App Logging level")) {
        return false;
    }
    if(!ValidateRequiredRadio(stackLoggingLevel, "Stack Logging level")) {
        return false;
    }
    return true;
}
