var methods = ["config.getGroupVariables",
               "config.setVariable",
               "misc.applyConfigChanges",
			   "user.logout"];


function InitAccessPointPage() {
    SetPageCommonElements();
    InitJSON();
    GetData();
}


function GetData() {
    ClearOperationResult("spnOperationResult");
    
    var eui64 = document.getElementById("txtEUI64");    
    var bbrTag = document.getElementById("txtBBRTag");    
    var networkID = document.getElementById("txtNetworkID");
    
    var appJoinKey = document.getElementById("txtAppJoinKey");
    var serialName = document.getElementById("txtSerialName");
    var stackLoggingLevel = document.getElementsByName("rbStackLoggingLevel");
    var stackLoggingLevelValue;
    
    eui64.value = "";
    bbrTag.value = "";
    networkID.value = "";
    appJoinKey.value ="";
    serialName.value ="";	
    for (var i=0; i<=2; i++){
    	stackLoggingLevel[i].checked = false;	
    };
    
    try {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        var result = service.config.getGroupVariables({group : "ACCESS_POINT"});
        for (i = 0; i < result.length; i++) {
          if(result[i].AP_EUI64 != null) {
              eui64.value = result[i].AP_EUI64;
          } 
          if(result[i].AP_TAG != null) {
              bbrTag.value = result[i].AP_TAG;
          } 
          if(result[i].NETWORK_ID != null) {
        	  networkID.value = result[i].NETWORK_ID;
          } 
          if(result[i].AppJoinKey != null) {
              appJoinKey.value = result[i].AppJoinKey;
          } 
          if(result[i].TTY_DEV != null) {
              serialName.value = result[i].TTY_DEV;
          } 
          if(result[i].LOG_LEVEL_STACK != null) {
              stackLoggingLevelValue = result[i].LOG_LEVEL_STACK;
          } 
       }
       if (stackLoggingLevelValue >= 1 && stackLoggingLevelValue <= 3) {
           stackLoggingLevel[stackLoggingLevelValue-1].checked = true;
       }
    } catch(e) {
        HandleException(e, "Unexpected error reading values !");
    }
}


function SaveData() {
    ClearOperationResult("spnOperationResult");
    
    if(ValidateInput()) {
        var eui64 = document.getElementById("txtEUI64");
        var bbrTag = document.getElementById("txtBBRTag");
        var networkID = document.getElementById("txtNetworkID");
        
        var appJoinKey = document.getElementById("txtAppJoinKey");
        var serialName = document.getElementById("txtSerialName");
        var stackLoggingLevel = document.getElementsByName("rbStackLoggingLevel");

        var saveFailed = false;
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        
            if (!service.config.setVariable({group : "ACCESS_POINT", varName : "AP_EUI64", varValue: eui64.value})) {
                alert("Error saving EUI64 !");
                saveFailed = true;
            }
            if (!service.config.setVariable({group : "ACCESS_POINT", varName : "AP_TAG", varValue: bbrTag.value})) {
                alert("Error saving AP Tag !");
                saveFailed = true;
            }
            if (!service.config.setVariable({group : "ACCESS_POINT", varName : "NETWORK_ID", varValue: networkID.value})) {
                alert("Error saving NETWORK ID !");
                saveFailed = true;
            } else {
            	CreateCookie("NETWORK_ID",networkID.value, null);
            	SetPageCommonElements();
            }
            if(!service.config.setVariable({group : "ACCESS_POINT", varName : "AppJoinKey", varValue: appJoinKey.value})) {
                alert("Error saving App Join Key !");
                saveFailed = true;
            }
            if (!service.config.setVariable({group : "ACCESS_POINT", varName : "TTY_DEV", varValue: serialName.value})) {
                alert("Error saving Serial Name !");
                saveFailed = true;
            }
            var stackLoggingLevelValue;
            for (var i=0; i < stackLoggingLevel.length; i++) {
                if (stackLoggingLevel[i].checked) {
                    stackLoggingLevelValue = i + 1;
                }
            }
            if (!service.config.setVariable({group : "ACCESS_POINT", varName : "LOG_LEVEL_STACK", varValue: stackLoggingLevelValue})) {
                alert("Error saving Stack Logging level !");
                saveFailed = true;                   
            }
        } catch(e) {
            HandleException(e, "Unexpected error saving values !");
            return
        }

        try {
            if (!service.misc.applyConfigChanges({module : "whaccesspoint"})) {
                alert("The Access Point process is not running !");
                return                   
            };        	
        } catch(e) {
            HandleException(e, "The Access Point process is not running !");
            return
        };	
        
        if(!saveFailed) {
            DisplayOperationResult("spnOperationResult", "Save completed successfully.");
        };
    };
}


function ValidateInput() {
    var eui64 = document.getElementById("txtEUI64");
    var bbrTag = document.getElementById("txtBBRTag");
    var networkID = document.getElementById("txtNetworkID");
    
    var appJoinKey = document.getElementById("txtAppJoinKey");
    var serialName = document.getElementById("txtSerialName");
    var stackLoggingLevel = document.getElementsByName("rbStackLoggingLevel");
        
    if(!ValidateRequired(eui64, "EUI64")) {
        return false;
    }
    if(!ValidateEUI64(eui64, "EUI64")) {
        return false;
    }
    if(!ValidateRequired(bbrTag, "AP Tag")) {
        return false;
    }
    if(!ValidateRequired(networkID, "NETWORK ID") || !ValidateDinamicHex(networkID, "NETWORK ID", 4)) {	
        return false;
    }
    if(!ValidateRequired(appJoinKey, "App Join Key")) {
        return false;
    }
    if(!Validate16BHex(appJoinKey, "App Join Key")) {
        return false;
    }     
    if(!ValidateRequired(serialName, "Serial Name")) {
        return false;
    }
    if(!ValidateRequiredRadio(stackLoggingLevel, "Stack Logging level")) {
        return false;
    }
    return true;
}
