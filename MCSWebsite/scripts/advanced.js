var methods = ["config.setVariable",
               "config.getConfig",
               "file.create",
               "user.logout",
               "misc.getGatewayNetworkInfo",
               "misc.setGatewayNetworkInfo",
               "misc.softwareReset",
               "misc.hardReset",
               "misc.restartApp",
               "misc.getNtpServers",
               "misc.setNtpServers"];

//contains all the config file content, populated once on load, and after new variable addition or variable edit
var CONFIG;
//used for reloading CONFIG after a new variable is added
var customVariableAdded = false;
//used for reloading SubnetId when pressing cancel on Custom case
var tmpSubnet = "";

function InitAdvancedPage() {
    SetPageCommonElements();    
    InitJSON();
    var txtNameServer = document.getElementById("txtNameServer");
    txtNameServer.value = "";
    var lstNameServers = document.getElementById("lstNameServers");
    lstNameServers.options[0] = null;

    var txtNTPServer = document.getElementById("txtNTPServer");
    txtNTPServer.value = "";
    var lstNTPServers = document.getElementById("lstNTPServers");
    lstNTPServers.options[0] = null;

    var ckMAC = document.getElementById("ckMAC");
    ckMAC.checked = false;
    
    ReadConfig();
    if(CONFIG != null) {
        PopulateSections();
        PopulateVariables();
        GetVR900Settings();
    }
}

//SECTIONS/VARIABLES
function SetVariable() {
    ClearOperationResults();
    var section = document.getElementById("ddlSection");
    var variable = document.getElementById("ddlVariable");	
	var val = document.getElementById("txtValue");
    if(!ValidateRequired(val, "Value")) {
        return;
    }
    
    var saveFailed = false;
    try {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        if (!service.config.setVariable({configFile : FILE_CONFIG_INI, group : section.value, varName : variable.value, varValue: val.value})) {
            alert ("Error saving variable !");
            saveFailed = true;
        }
    } catch(e) {
       HandleException(e, "Unexpected error saving value !");
       return;
    }

    ReadConfig();
    ResetSectionsVariables();
    
    if (!saveFailed) {
        DisplayOperationResult("spnSetResult", "Set variable completed successfully.");            
    }    
}

function ResetSectionsVariables() {
    ClearOperationResults();
	var txtVal = document.getElementById("txtValue");
	txtVal.value = "";   	
	var txtSection = document.getElementById("txtSection");
	
	if (txtSection.style.display == "inline") { //custom mode
        document.getElementById("txtVariable").value = "";
        
        txtSection.value = "";
      	txtSection.focus();        
    } else {   //standard mode
        PopulateVariableValue();
      	txtVal.focus();
    }
}

function ReadConfig() {
    try {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        CONFIG = service.config.getConfig({configFile : FILE_CONFIG_INI});
    } catch(e) {
        HandleException(e, "Unexpected error reading config !");
    }
}

function PopulateSections() {
    var ddlSection = document.getElementById("ddlSection");    
    ClearList("ddlSection");
    if (CONFIG != null) {
        for(i = 0;i < CONFIG.length; i++) {
            ddlSection.options[i] = new Option(CONFIG[i].group,CONFIG[i].group);
        }
    }
}

function PopulateVariables() {
    ClearVariables();        
    var ddlSection = document.getElementById("ddlSection");
    var ddlVariable = document.getElementById("ddlVariable");    
    //find current group
    var sectionPos = 0;

    if (CONFIG != null) {
        for(i = 0;i < CONFIG.length; i++) {
            if(CONFIG[i].group == ddlSection.value) {
                sectionPos = i;
                break;
            }
        }
        if (CONFIG.length > 0 && CONFIG[sectionPos].variables != null) {
            for (i = 0; i < CONFIG[sectionPos].variables.length; i++) {
                for (var variable in CONFIG[sectionPos].variables[i]) {
                    ddlVariable.options[i] = new Option(variable, variable);
                }
            }
        }
    }
    PopulateVariableValue();
}

function PopulateVariableValue() {
    var txtSection = document.getElementById("txtSection");
    if (txtSection.style.display == "none") { //standard mode
        var ddlSection = document.getElementById("ddlSection");
        var ddlVariable = document.getElementById("ddlVariable");    
        var txtValue = document.getElementById("txtValue");
    
        //find current group
        var sectionPos = 0;
    
        if (CONFIG != null) {
            for(i = 0;i < CONFIG.length; i++) {
                if(CONFIG[i].group == ddlSection.value) {
                    sectionPos = i;
                    break;
                }
            }
            if (CONFIG.length > 0 && CONFIG[sectionPos].variables != null) {
                for (i = 0; i < CONFIG[sectionPos].variables.length; i++) {
                    for (var variable in CONFIG[sectionPos].variables[i]) {
                        if (ddlVariable.value == variable) {
                            txtValue.value = CONFIG[sectionPos].variables[i][variable];
                        }
                    }
                }
            }
        }
    }
}

function ClearVariables() {
   var txtValue = document.getElementById("txtValue");
   var lst = document.getElementById("ddlVariable");
   for (i = lst.length; i >= 0; i--) {
      lst[i] = null;
   }
   txtValue.value = "";
}

function VariableTypeChanged(type) {
    ClearOperationResults();

    var ddlSection = document.getElementById("ddlSection");
    var ddlVariable = document.getElementById("ddlVariable");
    var txtSection = document.getElementById("txtSection");
    var txtVariable = document.getElementById("txtVariable");    
    var txtValue = document.getElementById("txtValue");    
    txtSection.value = txtVariable.value = txtValue.value = "";
    
    if (type == '1') {
        ddlSection.style.display = "inline";
        ddlVariable.style.display = "inline";
        txtSection.style.display = "none";
        txtVariable.style.display = "none";
       
        txtValue.focus();
        
        if (customVariableAdded) {  //reload CONFIG after adding custom variable(s)
            customVariableAdded = false;
            ReadConfig();
            PopulateSections();
            PopulateVariables();
        }
        PopulateVariableValue();
    } else {
        var txtSubnet = document.getElementById("txtSubnet");
        tmpSubnet = txtSubnet.value;
        ddlSection.style.display = "none";
        ddlVariable.style.display = "none";
        txtSection.style.display = "inline";
        txtVariable.style.display = "inline";        
        txtSection.focus();
    }
}

//=======================================================================
//RESTARTS

function RestartBackbone() {
    ClearOperationResults();
    if (confirm ("Are you sure you want to execute Access Point Restart ?")) {   
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            service.misc.restartApp({appName : "whaccesspoint"});
        } catch(e) {
           HandleException(e, "Error restarting Backbone !");
           return;
        }
        DisplayOperationResult("spnRestartResult", "Restart sent successfully.");
    }
}

function RestartGateway() {
    ClearOperationResults();
    if (confirm ("Are you sure you want to execute Gateway Restart ?")) {  
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            service.misc.restartApp({appName : "WHart_GW.o"});
        } catch(e) {
          HandleException(e, "Error restarting Gateway !");
          return;
        }
        DisplayOperationResult("spnRestartResult", "Restart sent successfully.");
    }
}

function RestartNetworkManager() {
    ClearOperationResults();
    if (confirm ("Are you sure you want to execute Network Manager Restart ?")) {
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
			service.misc.restartApp({appName: "WHart_NM.o", appParams: "/access_node/profile/ > /tmp/SystemManagerProcess.log 2>&1"})
        } catch(e) {
            HandleException(e, "Error restarting Network Manager !");
            return;
        }   
        DisplayOperationResult("spnRestartResult", "Restart sent successfully.");         
    }
}

function RestartMH() {
    ClearOperationResults();
    if (confirm ("Are you sure you want to execute Monitoring Host Restart ?")) {
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            service.misc.restartApp({appName : "MonitorHost"});
        } catch(e) {
           HandleException(e, "Error restarting Monitoring Host !");
           return;
        }    
        DisplayOperationResult("spnRestartResult", "Restart sent successfully.");        
    }
}

function RestartApplications() {
    ClearOperationResults();
    if (confirm ("Are you sure you want to execute Restart Applications ?")) {
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            service.misc.softwareReset();
        } catch(e) {
           HandleException(e, "Error restarting Applications !");
           return;
        }    
        DisplayOperationResult("spnRestartResult", "Restart sent successfully.");        
    }
}

function RestartVR900() {
    ClearOperationResults();
    if (confirm ("Are you sure you want to execute VR910 Hardware Restart ?")) {
       try {
           var service = new jsonrpc.ServiceProxy(serviceURL, methods);
           service.misc.hardReset();
       } catch(e) {
          HandleException(e, "Error restarting VR910 !");
          return;
       }    
       DisplayOperationResult("spnRestartResult", "Restart sent successfully.");    
    }
}

function RestartModbus() {
    ClearOperationResults();
    if (confirm ("Are you sure you want to execute MODBUS Restart ?")) {
       try {
           var service = new jsonrpc.ServiceProxy(serviceURL, methods);
           service.misc.restartApp({appName : "modbus_gw"});
       } catch(e) {
          HandleException(e, "Error restarting MODBUS !");
          return;
       }    
       DisplayOperationResult("spnRestartResult", "Restart sent successfully.");
    }
}

//=======================================================================
//VR900 SETTINGS

function GetVR900Settings() {
    ClearOperationResults();
    document.getElementById("txtNameServer").value = "";
    document.getElementById("txtNTPServer").value = "";
    var ip = document.getElementById("txtIP");
    var gateway = document.getElementById("txtGateway");	
    var mask = document.getElementById("txtMask");
    var nameServers = document.getElementById("lstNameServers");
    var ntpServers = document.getElementById("lstNTPServers");
    var mac0 = document.getElementById("txtMAC0");
    var mac1 = document.getElementById("txtMAC1");
    var service = new jsonrpc.ServiceProxy(serviceURL, methods); 
    var ckMAC = document.getElementById("ckMAC");
    ckMAC.checked = false;
    
    ClearList("lstNameServers");
    ClearList("lstNTPServers");
    EditMAC();

    try  {            
        var result0 = service.misc.getGatewayNetworkInfo();
        ip.value = result0.ip;
        gateway.value = result0.defgw;
        mask.value = result0.mask;
        mac0.value =  result0.mac0;
        mac1.value =  result0.mac1;
        for(i=0; i<result0.nameservers.length; i++){
           nameServers.options[i] = new Option(result0.nameservers[i], result0.nameservers[i]);
        };      

        var result1 = service.misc.getNtpServers();
        for(i=0; i<result1.servers.length; i++){
           ntpServers.options[i] = new Option(result1.servers[i], result1.servers[i]);
        };                                          
    } catch(e) {
        HandleException(e, "Unexpected error reading VR910 Settings !");
    };     
}

function SetVR900Settings() {
    ClearOperationResults();  
    if (ValidateVR900()) {
        var ip = document.getElementById("txtIP");
        var gateway = document.getElementById("txtGateway");	
   	    var mask = document.getElementById("txtMask");
   	    var mac0 = document.getElementById("txtMAC0");
   	    var mac1 = document.getElementById("txtMAC1");  	    
   	    var updateMac = document.getElementById("ckMAC");  	    
   	    var nameServers = document.getElementById("lstNameServers");
   	    var nameServersArray = Array();
   	    var ntpServers = document.getElementById("lstNTPServers");
   	    var ntpServersArray = Array();
   	    
   	    for (i=0; i<nameServers.length; i++){
   	        nameServersArray.push(nameServers[i].value);
   	    };
   	    for (i=0; i<ntpServers.length; i++){
   	        ntpServersArray.push(ntpServers[i].value);
   	    };
   	    
   	    var updateMACString = "false";
	    if (updateMac.checked == true){
	    	updateMACString = "true";
	    };
	    	    
	    try  {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);      
            var result0 = service.misc.setGatewayNetworkInfo({ip : ip.value,mac0 : mac0.value.trim().toUpperCase(), mac1 : mac1.value.trim().toUpperCase(), defgw : gateway.value, mask : mask.value, nameservers : nameServersArray, updateMAC : updateMACString });            
            var result1 = service.misc.setNtpServers({servers : ntpServersArray });
        
            if (result0 && result1) {
                DisplayOperationResult("spnOperationResult","Save completed successfully.");
            } else {
               alert("Error saving VR910 Settings !");
               GetVR900Settings();
            }                                                                  
       } catch(e) {
           if (e.message == "ValidationError") {
               alert("Invalid input !");
           } else {
               HandleException(e, "Unexpected error saving values !");
           }
       };     
       /*
        if (result0 != 0){            	
            switch (result0){
	            case 1: DisplayOperationResultError("spnOperationResult","Save completed successfully.");break;
	            case 2: DisplayOperationResultError("spnOperationResult","Invalid mask");break;
	            case 3: DisplayOperationResultError("spnOperationResult","Invalid gateway ip");break;
	            case 4: DisplayOperationResultError("spnOperationResult","Cannot open rc.net.info");break;
	            case 5: DisplayOperationResultError("spnOperationResult","Cannot open resolv.conf.eth");break;
            };            
            GetVR900Settings();
        };
        */          
    };   
}

function ValidateVR900() { 
    var ip = document.getElementById("txtIP");
    var gateway = document.getElementById("txtGateway");	
   	var mask = document.getElementById("txtMask");
   	var mac0 = document.getElementById("txtMAC0");
   	var mac1 = document.getElementById("txtMAC1");
   	var updateMac = document.getElementById("ckMAC");
   	
    if(!ValidateRequired(ip, "IP") || !ValidateIP(ip, "IP", true, false)) {
        return false;
    }
    
    if(!ValidateRequired(gateway, "Gateway") || !ValidateIP(gateway, "Gateway", true, false)) {
        return false;
    }
    
    if(!ValidateRequired(mask, "Mask") || !ValidateIP(mask, "Mask", true, false)) {
        return false;
    }
    if (updateMac.checked == true){
	    if (!ValidateRequired(mac0, "MAC0") || !ValidateDinamicHex(mac0, "MAC0", 12)){
	    	return false;
		};
	    if (!ValidateRequired(mac1, "MAC1") || !ValidateDinamicHex(mac1, "MAC1", 12)){
	    	return false;
		};		
    }
    return true;
}

function ClearOperationResults() {
    ClearOperationResult("spnSetResult");
    ClearOperationResult("spnOperationResult");
    ClearOperationResult("spnRestartResult");
}

function ConfigurationChanged() {
    ClearOperationResults();  
    var ddlConfiguration = document.getElementById("ddlConfiguration");
    var spnSubnet =  document.getElementById("spnSubnet");
    document.getElementById("txtSubnet").value = "";
    if (ddlConfiguration[ddlConfiguration.selectedIndex].id == "Subnet") {
        spnSubnet.style.display = "";
        ClearList("ddlSection");
        ClearVariables();
    } else {
        spnSubnet.style.display = "none";
        ReadConfig();
        if(CONFIG != null) {
            PopulateSections();
            PopulateVariables();
            GetVR900Settings();
        }
    } 
}

function SelectNameServer(){
    var lstNameServers = document.getElementById("lstNameServers");
    var txtNameServer = document.getElementById("txtNameServer");
    txtNameServer.value  = lstNameServers.options[lstNameServers.selectedIndex].value;
}

function SelectNTPServer(){
    var lstNTPServers = document.getElementById("lstNTPServers");
    var txtNTPServer = document.getElementById("txtNTPServer");
    txtNTPServer.value  = lstNTPServers.options[lstNTPServers.selectedIndex].value;
}

function SaveServerName(){
    var lstNameServers = document.getElementById("lstNameServers");
    var txtNameServer = document.getElementById("txtNameServer");
    if  ((ValidateIP(txtNameServer,'Name Server', true, false)) && (ElementIndex(txtNameServer.value,"lstNameServers") == -1)){
        lstNameServers.options[lstNameServers.length] = new Option(txtNameServer.value, txtNameServer.value);
        txtNameServer.value = ""
    }
}

function SaveNTPServer(){
    var lstNTPServers = document.getElementById("lstNTPServers");                                   
    var txtNTPServer = document.getElementById("txtNTPServer");
    if  (ValidateIP(txtNTPServer,'NTP Server', true, true) && ElementIndex(txtNTPServer.value,"lstNTPServers") == -1){
        lstNTPServers.options[lstNTPServers.length] = new Option(txtNTPServer.value, txtNTPServer.value);
        txtNTPServer.value = ""
    }
}

function DeleteServerName(){
    var lstNameServers = document.getElementById("lstNameServers");
    var txtNameServer = document.getElementById("txtNameServer");    
    var indexToDelete = ElementIndex(txtNameServer.value,"lstNameServers");
    if (indexToDelete != -1){
        lstNameServers.options[indexToDelete] = null;
        txtNameServer.value = ""
    };        
}

function DeleteNTPServer(){
    var lstNTPServers = document.getElementById("lstNTPServers");
    var txtNTPServer = document.getElementById("txtNTPServer");    
    var indexToDelete = ElementIndex(txtNTPServer.value,"lstNTPServers");
    if (indexToDelete != -1){
        lstNTPServers.options[indexToDelete] = null;
        txtNTPServer.value = ""
    };        
}

//verify if there is any element with the value val in the lstNameServers component
//if found  then the index of the element will be returned
//          else return -1    
function ElementIndex(val, list){
    var lst = document.getElementById(list);
    for(i=0; i<lst.length; i++){
        if (lst.options[i].value == val){
            return i;
        };
   };
   return -1;
}

function EditMAC(){
	var mac0 = document.getElementById("txtMAC0");
	var mac1 = document.getElementById("txtMAC1");
	var updateMac = document.getElementById("ckMAC");

	if (updateMac.checked){
		mac0.disabled = false;
		mac1.disabled = false;
	} else {
		mac0.disabled = true;
		mac1.disabled = true;
	}
}