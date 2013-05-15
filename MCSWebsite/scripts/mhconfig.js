var methods = ["config.getGroupVariables",
               "isa100.mh.setPublisher",
               "isa100.mh.delPublisher",
               "misc.applyConfigChanges",
               "config.getConfig",
               "user.logout",
               "user.isValidSession",
               "file.exists"];

var CONTENT = null;

var pendingChangesOn = false;
var currentBurstMsg = null;
var currentTrigger = null;

var helpPopup = new Popup();
	helpPopup.autoHide = false;
	helpPopup.offsetLeft = 10;
	helpPopup.constrainToScreen = true;
	helpPopup.position = 'above adjacent-right';
	helpPopup.style = {'border':'2px solid black','backgroundColor':'#EEE'};

var contentBurst =                
        '<b>Burst Messages Format:</b><br><i>{' + 
        '&lt;EUI64&gt;, &lt;COMMAND NUMBER&gt;, &lt;BURST MESSAGE&gt;,' +
		'&lt;UPDATE PERIOD&gt;, <br>&lt;MAXIMUM UPDATE PERIOD&gt;[, &lt;SUBDEVICE MAC&gt;]}</i><br />' + 
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>EUI64:</b> 8 bytes grouped by 2, represented as hex, separated by minus<br />' + 
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>COMMAND NUMBER:</b> integer from set {1, 2, 3, 9, 33, 178}<br />' + 
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>BURST MESSAGE:</b> index of the burst message, integer [0 - 255]<br />' + 
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>UPDATE PERIOD:</b> integer in [0 - 3600] seconds<br />' + 
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>MAXIMUM UPDATE PERIOD:</b> integer in [UPDATE PERIOD - 3600] seconds<br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>SUBDEVICE MAC:</b> 8 bytes hex represented (16 characters)<br />';
var contentVariable =
        '<b>Variable Format:</b> <i>{' +   
        '&lt;COMMAND NUMBER&gt;, &lt;BURST MESSAGE&gt;,<br />' +
		'&lt;DEVICE VARIABLE CODE &gt;, &lt;NAME&gt;, &lt;DEVICE VARIABLE SLOT&gt;,<br />' +
		'&lt;DEVICE VARIABLE CLASIFICATION&gt;, &lt;UNITS CODE&gt;}</i><br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>COMMAND NUMBER:</b> integer from set {1, 2, 3, 9, 33, 178}<br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>BURST MESSAGE:</b> index of the burst message, integer [0 - 255]<br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>DEVICE VARIABLE CODE:</b> integer in range [0-7] or [243-249]<br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>NAME:</b> character string (maximum 16 characters)<br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>DEVICE VARIABLE SLOT:</b> integer from set {0, 1, 2, 3, 4, 5, 6, 7}<br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>DEVICE VARIABLE CLASIFICATION:</b> integer in range [64-95] or 0<br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>UNITS CODE:</b> integer in range [1-169] or [220-239, 250]<br />';
var contentTrigger =
        '<b>Trigger Format:</b> <i>{' +
        '&lt;COMMAND NUMBER&gt;, &lt;BURST MESSAGE&gt;,<br />' +
		'&lt;BURST TRIGGERT MODE SELECTION&gt;, &lt;DEVICE VARIABLE CLASIFICATION&gt;,<br />' +
		'&lt;UNITS CODE&gt;, &lt;TRIGGER LEVEL&gt;}</i><br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>COMMAND NUMBER:</b> integer from set {1, 2, 3, 9, 33, 178}<br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>BURST MESSAGE:</b> index of the burst message, integer [0 - 255]<br />' +                                       
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>BURST TRIGGER MODE SELECTION:</b> integer: 0-Continuous, <br/>' +
        '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1-Window, 2-Rising, 3-Falling, 4-On-Change<br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>DEVICE VARIABLE CLASIFICATION:</b> integer in range [64-95] or 0<br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>UNITS CODE:</b> integer in range [1-169] or [220-239, 250]<br />' +
        '&nbsp;&nbsp;&nbsp;&nbsp;<b>TRIGGER LEVEL:</b> float trigger value<br />'
var closeLink = 
        '&nbsp;&nbsp;<a href="#" onclick="helpPopup.hide();return false;"><b>Close</b></a>';


function InitMHConfigPage() {
    SetPageCommonElements();
    InitJSON();
	ClearBurstMessages();
    ClearVariables();    
    ClearTrigger();
    if (!ReadConfigFromDisk()){
    	return;
    };    
    PopulateBurstMesseges();     
    var lstBurstMessages = document.getElementById("lstBurstMessages");
    lstBurstMessages.selectedIndex = lstBurstMessages.options.length-1;
    EditBurstMessages();    
}

function ReadConfigFromDisk() {
    try {
    	var service = new jsonrpc.ServiceProxy(serviceURL, methods);
    	var response = service.file.exists({file: FILE_MH_PUBLISHERS_CONF});
    	if (response.length == 0){    		
    		alert("There is no "+FILE_MH_PUBLISHERS_CONF+" file on the VR")
    		return false;
    	};	
    } catch (e) {
    	HandleException(e, "Unexpected error reading "+FILE_MH_PUBLISHERS_CONF);
    	return false;
    };    
	
    try {    
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        CONTENT = service.config.getConfig({configFile: FILE_MH_PUBLISHERS_CONF});
        if (CONTENT == null || CONTENT == ""){
        	alert("The "+FILE_MH_PUBLISHERS_CONF+" file content is empty!");
        	return false;
        } 
    } catch(e) {
        HandleException(e, "Error reading " + FILE_MH_PUBLISHERS_CONF + " file!");
        return false;
    };
    return true;
}
 
function DeleteBurstMessage() {
    ClearOperationResults();
    var txtBurstMessage = document.getElementById("txtBurstMessage");
    if (txtBurstMessage.value != ""){
	    if (confirm("Are you sure you want to delete the Burst Message?")){      
	        if (ValidateBurstMessage(txtBurstMessage, "BurstMessage")) {
	            var eui64 = txtBurstMessage.value.split(",")[0];	            
	            try {
	                var service = new jsonrpc.ServiceProxy(serviceURL, methods);
					var txtBurstMessageValue = formatCSVString(txtBurstMessage.value, 1)
	                var response = service.isa100.mh.delPublisher({eui64: eui64, burst: txtBurstMessageValue});
	                    
	                if(response) {
	                    ReadConfigFromDisk();                    
	                    ClearVariables();
	                    ClearTrigger(); 
	                    PopulateBurstMesseges();
	                    DisplayOperationResult("spnOperationResultBurstMessages", "Burst Message deleted successfully.");
	                };
	            } catch(e) {
	                HandleException(e, "Burst Message not deleted or not found !");
	            	ClearBurstMessages();
	                ClearVariables();    
	                ClearTrigger();
	                return;
	            };
	        };
	    };
    } else {
    	alert("No Burst Message selected. Please select a Burst Message!")
    };     
}


function EditBurstMessages() {
	currentBurstMsg = document.getElementById("lstBurstMessages").selectedIndex;
	if(pendingChangesOn){
		if (confirm("There are pending changes. Do you want to save it!")){
			document.getElementById("lstBurstMessages").selectedIndex = previousBurstMsg;								
			SaveBurstMessage();
			pendingChangesOn = false;
			return;
		} else {
			pendingChangesOn = false;		
		}			
	}
	previousBurstMsg = currentBurstMsg;	
	
    ClearOperationResults();    
    ClearVariables();
    ClearTrigger();
	
    var txtBurstMessage = document.getElementById("txtBurstMessage");
	txtBurstMessage.value = document.getElementById("lstBurstMessages").value;
    var group = txtBurstMessage.value.split(",");

    var lstVariables = document.getElementById("lstVariables");
    var txtVariableId = document.getElementById("txtVariableId");
    var txtTrigger = document.getElementById("txtTrigger");
	var txtTriggerId = document.getElementById("txtTriggerId");
    txtVariableId.value = group[1].trim() + ", " + group[2].trim() + ",";
    txtTriggerId.value = group[1].trim() + ", " + group[2].trim() + ",";
	
    //find the publisher and get the variables
    for (var i=0; i<CONTENT.length; i++) {
        if (CONTENT[i].group == group[0]) {
			if (CONTENT[i].variables != null) {
				for (var j = 0; j < CONTENT[i].variables.length; j++) {
					var value = CONTENT[i].variables[j];
					if (value.VARIABLE != null) {
						var valArray = value.VARIABLE.split(",");
						if (valArray[0].trim() == group[1].trim() && valArray[1].trim() == group[2].trim()) {
							var op = new Option(value.VARIABLE, value.VARIABLE);
							op.className = (j % 2 == 0) ? "listAlternateItem" : op.className;
							lstVariables[lstVariables.length] = op;
							
						}
					} else if (value.TRIGGER != null) {
						var valArray = value.TRIGGER.split(",");
						if (valArray[0].trim() == group[1].trim() && valArray[1].trim() == group[2].trim()) {
							txtTrigger.value = valArray[2];
							for (var k = 3; k<valArray.length; k++) {
								txtTrigger.value = txtTrigger.value + "," + valArray[k];
							}
						}
					}
				}
			}
            break;
        }
    }
}

function EditVariable() {
    ClearOperationResults();    
    var txtVariable = document.getElementById("txtVariable");
	var lstVariables = document.getElementById("lstVariables");
    var txtVariableId = document.getElementById("txtVariableId");
	txtVariable.value = formatCSVString(lstVariables.value, 2);
}

function formatCSVString(csvString, beginIndex, endIndex) {
	if (csvString == null)
	   return null;
	var csvArray = csvString.split(",");
	beginIndex = (beginIndex != null && beginIndex <= csvArray.length) ? beginIndex : 0;
	endIndex = (endIndex != null && endIndex < csvArray.length) ? endIndex : csvArray.length-1;
    var csvResult = "";
	for (var i = beginIndex; i <= endIndex; i++){
	   csvResult += csvArray[i].trim() + ((i < endIndex) ? ", " : "");
    }
	return csvResult;
}

function SaveBurstMessage() {
	pendingChangesOn = false;	
	ClearOperationResults();

    var lstBurstMessages = document.getElementById("lstBurstMessages");
    var txtBurstMessage = document.getElementById("txtBurstMessage");
    var txtBurstMessageValue = formatCSVString(txtBurstMessage.value);
    var origIndex = lstBurstMessages.selectedIndex;

    var lstVariables = document.getElementById("lstVariables");
    var txtVariable = document.getElementById("txtVariable");
    var txtTrigger = document.getElementById("txtTrigger");
    var txtTriggerId = document.getElementById("txtTriggerId");

    if (ValidateBurstMessage(txtBurstMessage, "BurstMessage") &&
        ValidateTrigger(txtTrigger, "Trigger")) {
        var lstKey = new Array(lstBurstMessages.length);
        for (var i=0; i<lstBurstMessages.length; i++) {
			lstKey[i] = formatCSVString(lstBurstMessages[i].value, 0, 2);
        }
        var valKey = formatCSVString(txtBurstMessage.value, 0, 2);
        
        //check wether key exists already
        var newIndex = IndexInArray(lstKey, valKey);
        
        if (newIndex == -1) {
			var userMsg = "";
			
			var newDevice = formatCSVString(valKey,0,0).toUpperCase();
			var newBurst  = formatCSVString(valKey,2,2).toUpperCase();
			for(var i=0; i<lstKey.length; i++){
				var devAddress = formatCSVString(lstKey[i],0,0).toUpperCase(); 
				var burstMsg   = formatCSVString(lstKey[i],2,2).toUpperCase();
				if (devAddress == newDevice && burstMsg == newBurst){
					alert("Burst Message " + burstMsg + " already exists for " + devAddress + " !");
		            ClearVariables();
		            ClearTrigger();
		            lstBurstMessages.selectedIndex = origIndex;
		            EditBurstMessages();
					return;
				};  
			};
			
			if (origIndex >= 0) {
				if (!confirm("Dou you want to add a new 'Burst Message'?")) {
					txtBurstMessage.value = lstBurstMessages.selectedIndex >= 0 ? lstBurstMessages[lstBurstMessages.selectedIndex].value : "";
		            ClearVariables();
		            ClearTrigger();
		            lstBurstMessages.selectedIndex = origIndex;
		            EditBurstMessages();
					return;
				};
			};						
			newIndex = lstBurstMessages.length;
            lstBurstMessages[newIndex] = new Option(txtBurstMessageValue, txtBurstMessageValue);
			ClearVariables();
			ClearTrigger();
            lstBurstMessages.selectedIndex = newIndex;
			EditBurstMessages();
        } else {
			if (newIndex != origIndex) {
				alert("A Burst Message with this key [" + valKey + "] already exists!");
				lstBurstMessages.selectedIndex = origIndex;
                txtBurstMessage.value = lstBurstMessages[origIndex].value;
                EditBurstMessages();                
                return;
			} else {
                lstBurstMessages[newIndex] = new Option(txtBurstMessage.value, txtBurstMessage.value);
				lstBurstMessages.selectedIndex = newIndex;
			};
        }	
		// write Burst Message to disk
        var eui64 = formatCSVString(txtBurstMessageValue, 0, 0); 
        var burst = formatCSVString(txtBurstMessageValue, 1); 
        var variables = Array();
        for (var i = 0; i < lstVariables.length; i++) {
            variables.push(lstVariables[i].value.trim());
        }
        var trigger = (txtTrigger.value == "") ? "" : formatCSVString(txtTriggerId.value + txtTrigger.value);
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            var response = service.isa100.mh.setPublisher({eui64: eui64.trim(), burst: burst.trim(), variables: variables, trigger: trigger.trim()});
            if(response) {
                if (ReadConfigFromDisk()){
                    ClearVariables();     
                    ClearTrigger();     
                    PopulateBurstMesseges();                                      
                    lstBurstMessages.selectedIndex = (origIndex == -1 ? 0 : origIndex);
                    EditBurstMessages();
                    DisplayOperationResult("spnOperationResultBurstMessages", "Save completed successfully.");
                }
            } else {
                alert ("Error saving Burst Message !");
            }
        } catch(e) {
            HandleException(e, "Unexpected error saving Burst Message !");
        	ClearBurstMessages();
            ClearVariables();    
            ClearTrigger();
            return;
        };
    }
}

function PopulateBurstMesseges() {
	ClearList("lstBurstMessages");
	document.getElementById("txtBurstMessage").value = "";
	
    var lst = document.getElementById("lstBurstMessages");
	var index = 0;
    if (CONTENT != null) {
        for (i = 0; i < CONTENT.length; i++) {   
            if (CONTENT[i].group != null) {
				var burstCount = -1;
				for (var j = 0; j < CONTENT[i].variables.length; j++) {
					if (CONTENT[i].variables[j].BURST_NO_TOTAL_CMD_105 != null) {
						burstCount = parseInt(CONTENT[i].variables[j].BURST_NO_TOTAL_CMD_105);
					}
				}
				var burstError = [];
                for (var k = 0; k < burstCount; k++) {
					burstError[k] = false;
                    for (var j = 0; j < CONTENT[i].variables.length; j++) {
						var stateVar = eval("CONTENT[i].variables[j].BURST_SET_STAT_" + k);
                        if (stateVar != null && stateVar.indexOf("ERROR") >= 0) {
							burstError[k] = true;
							break;
						}
                    }
                }
				for (var j = 0; j < CONTENT[i].variables.length; j++) {
					if (CONTENT[i].variables[j].BURST != null) {
                        var op = new Option(CONTENT[i].group + ", " + CONTENT[i].variables[j].BURST, 
                                            CONTENT[i].group + ", " + CONTENT[i].variables[j].BURST);
                        var valArray = CONTENT[i].variables[j].BURST.split(",");
                        if (burstCount >= 0 && burstError[parseInt(valArray[1])]) {
                            op.className = (index % 2 == 1) ? "listAlternateErrorItem" : "listErrorItem";
						} else {
							op.className = (index % 2 == 1) ? "listAlternateItem" : op.className;
						}
						index++;
						lst[lst.length] = op;
					}
                }
            }
        }
    }    
}

function ClearBurstMessages() {
	ClearList("lstBurstMessages");
	document.getElementById("txtBurstMessage").value = "";       
}

function ClearVariables() {
	ClearList("lstVariables");
	document.getElementById("txtVariable").value = "";    
	document.getElementById("txtVariableId").value = "";    
}

function ClearTrigger() {
	document.getElementById("txtTrigger").value = "";
    document.getElementById("txtTriggerId").value = "";
}

function ChangeVariable() {
    ClearOperationResults();
    
    var lstVariables = document.getElementById("lstVariables");
    var txtVariable = document.getElementById("txtVariable");
    var txtVariableId = document.getElementById("txtVariableId");

    if(ValidateVariable(txtVariable, "Variable")) {
        var lstKey = new Array(lstVariables.length);
        for (var i=0; i<lstVariables.length; i++) {
            lstKey[i] = formatCSVString(lstVariables[i].value, 0, 2);
        }
        var valKey = formatCSVString(txtVariableId.value + txtVariable.value, 0, 2); 
		
		//check wether key exists already
        var index = IndexInArray(lstKey, valKey);
		var txtVariableValue = formatCSVString(txtVariableId.value + txtVariable.value);
        if (index == -1) {
            lstVariables[lstVariables.length] = new Option(txtVariableValue, txtVariableValue);
        } else {
            lstVariables[index] = new Option(txtVariableValue, txtVariableValue);
        }         
        txtVariable.value = "";
        pendingChangesOn = true;
    };    
    lstVariables.selectedIndex = -1;
    txtVariable.focus();
}

function DeleteVariable() {
    ClearOperationResults();    
    var txtVariable = document.getElementById("txtVariable");
    if (txtVariable.value != ""){
    	if (confirm("Are you sure you want to delete the variable?")){
	        var lstVariables = document.getElementById("lstVariables");
	        if (lstVariables.selectedIndex != -1){
	            for (var i = lstVariables.selectedIndex; i < lstVariables.length-1; i++) {    	
	                lstVariables[i] = new Option(lstVariables[i+1].value, lstVariables[i+1].value);
	            }    	
	        }    	
	        if (lstVariables.length > 0){
	        	lstVariables[lstVariables.length-1] = null;	
	        }                    
	        txtVariable.value = "";
	        pendingChangesOn = true;
    	}    
    } else {
    	alert("No variable selected. Please select a variable!")
    };
    txtVariable.focus();
}

function DeleteTrigger() {
	var txtTrigger = document.getElementById("txtTrigger");
	if (txtTrigger.value != ""){
		if (confirm("Are you sure you want to delete the trigger?")){
			document.getElementById("txtTrigger").value = "";
			pendingChangesOn = true;			
		};
	} else {
		alert("No trigger selected. Please select a trigger!")
	};
	txtTrigger.focus();
}

function Activate() {
    ClearOperationResults();    
    try {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        var response1 = service.misc.applyConfigChanges({module: "MonitorHost"});
        var response2 = service.misc.applyConfigChanges({module: "modbus_gw"});
        if(response1) {
			if (response2) {
				DisplayOperationResult("spnOperationResultActivate", "Activate completed successfully.");
			} else {
                alert ("Error activating Publisher list for MODBUS!");
			}
        } else {
            alert ("Error activating Publisher list for MonitorHost!");
        }
    } catch(e) {
        HandleException(e, "Unexpected error activating Publisher list!");
        return;
    }
}

function Download() {
    ClearOperationResults();
    try {    
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        service.user.isValidSession();        
	    document.form1.call1.value = jsonrpc.JSONRPCMethod("a").jsonRequest(1, "misc.fileDownload", {file: FILE_MH_PUBLISHERS_CONF});
        return 1 ;
    } catch (e) {
        HandleException(e, 'Unexpected error downloading file !')
    };        
}

function Upload() {
    ClearOperationResults();
    var file = document.getElementById("mhFile");
    if (file.value == ""){
        alert('There is no selected file. Please select a file!');
        return false;    	
    };    
    if (getFileName(file.value) != FILE_MH_PUBLISHERS_CONF)  {
        alert('The name of the uploaded file must be ' + FILE_MH_PUBLISHERS_CONF);
        return false; 
    } else {   
        if (confirm("The previous publisher list will be lost. Are you sure you want to replace the file?")){
            try {    
                var service = new jsonrpc.ServiceProxy(serviceURL, methods);
                service.user.isValidSession();        
                document.form1.call1.value = jsonrpc.JSONRPCMethod("a").jsonRequest(1,"misc.fileUpload", {file: FILE_MH_PUBLISHERS_CONF});
                return 1 ;
            } catch (e) {
                HandleException(e, 'Unexpected error uploading file !')
            };    
        } else {
            return false;
        };
    };    
}

function ClearOperationResults() {
    ClearOperationResult("spnOperationResultBurstMessages");
    ClearOperationResult("spnOperationResultActivate");
}

function operationDoneListener(text) {
    if (text.error) {
    	if (text.error == "No such file."){
    		alert("There is no " + FILE_MH_PUBLISHERS_CONF + " file on the VR");	
    	} else {
    		alert("Error uploading " + FILE_MH_PUBLISHERS_CONF + " file !");
    	};        
	} else {
        DisplayOperationResult("spnOperationResultActivate", "Upload completed successfully.");
        // refresh data from disk
	    ReadConfigFromDisk();    
	    ClearVariables();    
	    ClearTrigger();    
	    PopulateBurstMesseges();
	}
}

function SeePublishersConfigStatus(){
	   document.location.href = "mhconfigstatus.html";
} 