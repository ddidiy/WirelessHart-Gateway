var methods = ["config.getGroupVariables",
               "config.setVariable",
               "isa100.sm.setDevice",
               "isa100.sm.delDevice",
               "misc.applyConfigChanges",
               "misc.fileDownload",
               "user.logout",
               "user.isValidSession",
               "file.exists"];
var joinKeyPerNetwork;
var helpPopup = new Popup();
    helpPopup.autoHide = false;
    helpPopup.offsetLeft = 10;
    helpPopup.constrainToScreen = true;
    helpPopup.position = 'above adjacent-right';
    helpPopup.style = {'border':'2px solid black','backgroundColor':'#EEE'};

var contentDevice =
	 		    '<b>&nbsp;Gateway,&nbsp;Access Point,&nbsp;Device:&nbsp;</b><i>{&lt;EUI64&gt;, &lt;KEY&gt;}</i><br/>' +                
                '&nbsp;&nbsp;&nbsp;&nbsp;<b>EUI64:</b> 8 bytes grouped by 2, represented as hex, separated by minus<br />' + 
                '&nbsp;&nbsp;&nbsp;&nbsp;<b>KEY:</b> 16 bytes, represented as hex, can be separated by spaces<br />' +                  
				'<br />' +
                '&nbsp;&nbsp;<b>Examples:</b><br/>' + 
				'&nbsp;&nbsp;<b>1.</b> 62-02-03-04-05-06-FC-00, C0 C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF <br />' +                			
				'&nbsp;&nbsp;<b>2.</b> 62-02-03-04-05-06-FC-00, C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF <br/>';                  
var closeLink = '&nbsp;&nbsp;<a href="#" onclick="helpPopup.hide();return false;"><b>Close</b></a>';                

var GatewayList = [];

function InitDeviceMngPage() {
    SetPageCommonElements();
    InitJSON();
    PopulateAllDeviceLists();
}


function DeleteAccessPoint() {
    ClearOperationResults();
    var txtAccessPoint = document.getElementById("txtAccessPoint");
    var lstAccessPoints = document.getElementById("lstAccessPoints");
    if (txtAccessPoint.value != ""){
	    if (confirm("Are you sure you want to delete the Access Point?")){
	        try {
	            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
				/// TODO: ACCESS_POINT
	            var response = service.isa100.sm.delDevice({deviceType: "BACKBONE", deviceValue : txtAccessPoint.value});
	            if(response) {
	                PopulateAllDeviceLists();
	                DisplayOperationResult("spnOperationResultAccessPoint", "Access Point deleted successfully.");
	            };
	        } catch(e) {
	            HandleException(e, "Access Point not deleted or not found !");	   
	            lstAccessPoints.selectedIndex = -1;
	            txtAccessPoint.select();	
	            return;
	        };
	    };
    } else {
    	alert("No Access Point selected. Please select an Access Point!");
    	lstAccessPoints.selectedIndex = -1;
    }     
}


function DeleteDevice() {
    ClearOperationResults();
    var txtDevice = document.getElementById("txtDevice");
    if (txtDevice.value != ""){
        if (confirm("Are you sure you want to delete the Device?")){    
            try {
                var service = new jsonrpc.ServiceProxy(serviceURL, methods);
                var response = service.isa100.sm.delDevice({deviceType: "DEVICE", deviceValue : txtDevice.value});
                if(response) {
                    PopulateAllDeviceLists();
                    DisplayOperationResult("spnOperationResultDevice", "Device deleted successfully.");
                };
            } catch(e) {
                HandleException(e, "Device not deleted or not found !");
                return;
            };
        };    	
    } else {
    	alert("No device selected. Please select a device!")
    }
}


function deleteOldGateway(service) {
    try {
        for (var i=0; i<GatewayList.length; i++) {
            var response = service.isa100.sm.delDevice({deviceType: "GATEWAY", deviceValue: GatewayList[i]});
        };
    } catch(e) {
        //HandleException(e, "Gateway not deleted or not found !");
        return;
    };
}


function EditDevice() {
    var txtDevice = document.getElementById("txtDevice");
    txtDevice.value = document.getElementById("lstDevices").value;
    ClearOperationResults();
}


function EditAccessPoint() {
    var txtAccessPoint = document.getElementById("txtAccessPoint");
    txtAccessPoint.value = document.getElementById("lstAccessPoints").value;
    ClearOperationResults();
}


function SaveAccessPoint() {
    ClearOperationResults();
    var txtAccessPoint = document.getElementById("txtAccessPoint");
    if (ValidateDevice(txtAccessPoint, "AccessPoint")) {
        try {   
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
			/// TODO: ACCESS_POINT
            var response = service.isa100.sm.setDevice({deviceType: "BACKBONE", deviceValue : txtAccessPoint.value});
            
            if(response) {
                PopulateAllDeviceLists();
                DisplayOperationResult("spnOperationResultAccessPoint", "Save completed successfully.");
            } else {
                alert ("Error saving Access Point !");
            }
        } catch(e) {
            HandleException(e, "Unexpected error saving Access Point !");
            return;
        }
    }
}


function SaveGateway() {
    ClearOperationResults();
    var txtGateway = document.getElementById("txtGateway");
    if (ValidateDevice(txtGateway, "Gateway")) {
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
			deleteOldGateway(service);
            var response = service.isa100.sm.setDevice({deviceType: "GATEWAY", deviceValue : txtGateway.value});
                
            if(response) {
                PopulateAllDeviceLists();
                DisplayOperationResult("spnOperationResultGateway", "Save completed successfully.");
            } else {
                alert ("Error saving Gateway !");
            }
        } catch(e) {
            HandleException(e, "Unexpected error saving gateway !");
            return;
        }
    }
}


function SaveDevice() {
    ClearOperationResults();
    var txtDevice = document.getElementById("txtDevice");
    if (ValidateDevice(txtDevice, "Device")) {
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            var response = service.isa100.sm.setDevice({deviceType: "DEVICE", deviceValue : txtDevice.value});
            if(response) {
                PopulateAllDeviceLists();
                DisplayOperationResult("spnOperationResultDevice", "Save completed successfully.");
                
            } else {
                alert ("Error saving device !");
            }
        } catch(e) {
            HandleException(e, "Unexpected error saving device !");
            return;
        }
    }
}


function PopulateAllDeviceLists() {
    ClearAllDeviceLists();   
    var lstDevices = document.getElementById("lstDevices");
    var lstAccessPoints = document.getElementById("lstAccessPoints");
    var txtGateway = document.getElementById("txtGateway");
    var txtJoinKey = document.getElementById("txtJoinKey");
    var rbJoinKey  = document.getElementsByName("rbJoinKey");
    var spnDeviceList = document.getElementById("spnDeviceList");
    var spnJoinKey = document.getElementById("spnJoinKey");    
           
    try {
    	var service = new jsonrpc.ServiceProxy(serviceURL, methods);
    	var response = service.file.exists({file: FILE_SYSTEMMANAGER_INI});
    	if (response.length == 0){    		
    		alert("There is no "+FILE_SYSTEMMANAGER_INI+" file on the VR")
    		return;
    	};	
    } catch (e) {
    	HandleException(e, "Unexpected error reading "+FILE_SYSTEMMANAGER_INI);
    	return;
    };
    
    try {       
    	var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        var response = service.config.getGroupVariables({configFile: FILE_SYSTEMMANAGER_INI, group : 'SECURITY_MANAGER'});
    } catch(e) {    	
        HandleException(e, "Unexpected error reading "+FILE_SYSTEMMANAGER_INI);
        return;
    }
    var op;
        
    if (response){
    for (var i = 0; i < response.length; i++) {   
        if(response[i].DEVICE != null) {
            op = new Option(response[i].DEVICE, response[i].DEVICE);
            lstDevices[lstDevices.length] = op;
			if (lstDevices.length % 2 == 1) {
				lstDevices[lstDevices.length - 1].className = "listAlternateItem";
			}
        }
        /// TODO: ACCESS_POINT
        if(response[i].BACKBONE != null) {
            op = new Option(response[i].BACKBONE, response[i].BACKBONE);
            lstAccessPoints[lstAccessPoints.length] = op;
			if (lstAccessPoints.length % 2 == 1) {
				lstAccessPoints[lstAccessPoints.length - 1].className = "listAlternateItem";
			}
        }
        if(response[i].GATEWAY != null) {
			GatewayList[GatewayList.length] = response[i].GATEWAY;
            txtGateway.value = response[i].GATEWAY;
        }
        
        if(response[i].NETWORK != null) {        	
        	var tmp = response[i].NETWORK;        	
        	txtJoinKey.value = tmp.substring(0, tmp.indexOf(','));        	
        	if (trim(tmp.substring(tmp.indexOf(',')+1),' ') == "true"){        		
        		rbJoinKey[0].checked = true;
        		rbJoinKey[1].checked = false;
        		spnDeviceList.style.display = "none";           		
        		spnJoinKey.style.display = "";
        	} else {
        		rbJoinKey[0].checked = false;
        		rbJoinKey[1].checked = true;
        		spnDeviceList.style.display = "";
        		spnJoinKey.style.display = "none";
        	}
        }	
    }
    }
}


function ClearAllDeviceLists() {
   document.getElementById("txtGateway").value= ""; 
   ClearList("lstAccessPoints");
   document.getElementById("txtAccessPoint").value = "";
   ClearList("lstDevices");
   document.getElementById("txtDevice").value = "";
}


function Activate() {
    ClearOperationResults();
    try {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        var response = service.misc.applyConfigChanges({module: "WHart_NM.o"});
        if(response) {
            DisplayOperationResult("spnOperationResultActivate", "Activate completed successfully.");
        } else {
            alert ("Error activating device list !");
        }
    } catch(e) {
        HandleException(e, "Unexpected error activating device list !");
        return;
    }
}


function Upload() {
    ClearOperationResults();
    var file = document.getElementById("smFile");
    if (file.value == ""){
        alert('There is no selected file. Please select a file!');
        return false;    	
    };
    if (getFileName(file.value) != FILE_SYSTEMMANAGER_INI){
        alert('The name of the uploaded file must be ' + FILE_SYSTEMMANAGER_INI);
        return false;
    } else {   
        if (confirm("The previous device list will be lost. Are you sure you want to replace the file?")){
            try {    
                var service = new jsonrpc.ServiceProxy(serviceURL, methods);
                service.user.isValidSession();        
                document.form1.call1.value = jsonrpc.JSONRPCMethod("a").jsonRequest(1,"misc.fileUpload", {file: FILE_SYSTEMMANAGER_INI});
                return 1;
            } catch (e) {
                HandleException(e, 'Unexpected error uploading file !')
            };    
        } else {
            return false;
        };
    };    
}


function Download() {
    ClearOperationResults();
    try {    
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        service.user.isValidSession();        
        document.form1.call1.value = jsonrpc.JSONRPCMethod("a").jsonRequest(1, "misc.fileDownload", {file: FILE_SYSTEMMANAGER_INI});
        return 1 ;
    } catch (e) {
        HandleException(e, 'Unexpected error downloading file !')
    };        
}


function operationDoneListener(text) {
    if (text.error) {
    	if (text.error == "No such file."){
    		alert("There is no "+FILE_SYSTEMMANAGER_INI+" file on the VR");	
    	} else {
    		alert("Error uploading " + FILE_SYSTEMMANAGER_INI + " file !");
    	}	
    } else {
        DisplayOperationResult("spnOperationResultActivate", "Upload completed successfully.");
        PopulateAllDeviceLists();
    }
}


function ClearOperationResults() {
    ClearOperationResult("spnOperationResultAccessPoint");
    ClearOperationResult("spnOperationResultGateway");
    ClearOperationResult("spnOperationResultDevice");
    ClearOperationResult("spnOperationResultActivate");
}


function ShowHideHelp(op) {
    var divHelp = document.getElementById("divHelp");
    if (op == "open") {
        divHelp.style.display = "";
    } else {
        divHelp.style.display = "none";
    }
}

function ActivateJoinKey(){
	ClearOperationResults();
    var txtJoinKey = document.getElementById("txtJoinKey");
    var rbJoinKey  = document.getElementsByName("rbJoinKey");
    
    if (ValidateJoinKey(txtJoinKey, "Join Key")) {
    	try {
        	var joinKeyToSave = txtJoinKey.value + ", " + rbJoinKey[0].checked.toString();
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            var response = service.config.setVariable({configFile : FILE_SYSTEMMANAGER_INI, group : "SECURITY_MANAGER", varName : "NETWORK", varValue: joinKeyToSave})
            if(response) {
                PopulateAllDeviceLists();                
                DisplayOperationResult("spnOperationResultDevice", "Save completed successfully.");
            } else {
                alert ("Error saving Join Key !");
            };
        } catch(e) {
            HandleException(e, "Unexpected error saving Join Key !");
            return;
        };
    };   
}

function ChangeJoinKeyMode(){
	  var rbJoinKey  = document.getElementsByName("rbJoinKey");
	//  var btnActivateJoinKey  = document.getElementById("btnActivateJoinKey");
      var spnDeviceList = document.getElementById("spnDeviceList");
	  var spnJoinKey = document.getElementById("spnJoinKey");
	  
	  if (rbJoinKey[0].checked == true){
		  rbJoinKey[0].checked == false;
		  rbJoinKey[1].checked == true;
		//  btnActivateJoinKey.disabled = false;
   		  spnDeviceList.style.display = "none";           		
		  spnJoinKey.style.display = "";		  
	  } else {
		  rbJoinKey[0].checked == true;
		  rbJoinKey[1].checked == false;
		 // btnActivateJoinKey.disabled = false;
  		  spnDeviceList.style.display = "";           		
		  spnJoinKey.style.display = "none";
	  };	  
}