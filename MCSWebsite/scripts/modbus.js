var methods = ["isa100.modbus.server.setRow",
               "isa100.modbus.server.delRow",
               "misc.applyConfigChanges",
               "config.getConfig",
               "config.getConfigPart",
               "file.create",
               "user.logout",
               "user.isValidSession",
               "file.exists"];

//config file content
var CONTENT = null;

var helpPopup = new Popup();
	helpPopup.autoHide = false;
	helpPopup.offsetLeft = 10;
	helpPopup.constrainToScreen = true;
	helpPopup.position = 'above adjacent-right';
	helpPopup.style = {'border':'2px solid black','backgroundColor':'#EEE'};

var headerInputRegister = '<b><u>INPUT REGISTER</u>:</b><br/>';
var headerHoldingRegister = '<b><u>HOLDING REGISTER</u>:</b><br/>';
var headerAllRegisters = '<b><u>INPUT/HOLDING REGISTER</u>:</b><br/>';
var contentRegister =  
        '<b>Format1:</b> <i>{&lt;START ADDRESS&gt;, &lt;WORD COUNT&gt;, &lt;EUI64&gt;,<br/>' +
	        '&lt;REGISTER TYPE&gt;, &lt;BURST MESSAGE&gt;, &lt;DEVICE VARIABLE CODE&gt;,<br/>' +
	        '&lt;DEVICE STATE&gt;}</i><br/>' + 
	        '&nbsp;&nbsp;&nbsp;&nbsp;<b>START ADDRESS:</b> integer in [0-65535]<br />' + 
	        '&nbsp;&nbsp;&nbsp;&nbsp;<b>WORD COUNT:</b> integer in [1-125]<br />' + 
	        '&nbsp;&nbsp;&nbsp;&nbsp;<b>EUI64:</b> 8 bytes grouped by 2, represented as hex, separated by minus<br />' + 
	        '&nbsp;&nbsp;&nbsp;&nbsp;<b>REGISTER TYPE:</b> device_variable<br />' + 
	        '&nbsp;&nbsp;&nbsp;&nbsp;<b>BURST MESSAGE:</b> integer in [0-255]<br />' + 
	        '&nbsp;&nbsp;&nbsp;&nbsp;<b>DEVICE VARIABLE CODE:</b> integer in [0-7] or [243-249]<br />' + 
	        '&nbsp;&nbsp;&nbsp;&nbsp;<b>DEVICE STATE:</b> integer in {0, 2}<br /><br />' +
        '<b>Format2:</b> <i>{&lt;START ADDRESS&gt;, &lt;WORD COUNT&gt;, &lt;EUI64&gt;, &lt;REGISTER TYPE&gt;}</i><br/>' +
        	'&nbsp;&nbsp;&nbsp;&nbsp;<b>START ADDRESS:</b> integer in [0-65535]<br />' +        
        	'&nbsp;&nbsp;&nbsp;&nbsp;<b>WORD COUNT:</b> 5<br />' +
        	'&nbsp;&nbsp;&nbsp;&nbsp;<b>EUI64:</b> 00-1B-1E-F9-81-00-00-02, 00-00-00-00-00-00-00-00 or FF-FF-FF-FF-FF-FF-FF-FF<br />' +
        	'&nbsp;&nbsp;&nbsp;&nbsp;<b>REGISTER TYPE:</b> gw_info<br /><br />'+
        '<b>Format3:</b> <i>{&lt;START ADDRESS&gt;, &lt;WORD COUNT&gt;, &lt;EUI64&gt;, &lt;REGISTER TYPE&gt;}</i><br/>' +
        	'&nbsp;&nbsp;&nbsp;&nbsp;<b>START ADDRESS:</b> integer in [0-65535]<br />' +        
        	'&nbsp;&nbsp;&nbsp;&nbsp;<b>WORD COUNT:</b> integer in [1-16]<br />' +
        	'&nbsp;&nbsp;&nbsp;&nbsp;<b>EUI64:</b> 00-1B-1E-F9-81-00-00-02, 00-00-00-00-00-00-00-00 or FF-FF-FF-FF-FF-FF-FF-FF<br />' +        
        	'&nbsp;&nbsp;&nbsp;&nbsp;<b>REGISTER TYPE:</b> gw_code_word<br />';        

var closeLink = 
        '&nbsp;&nbsp;<a href="#" onclick="helpPopup.hide();return false;"><b>Close</b></a>';                
 

function InitModbusPage() {
    SetPageCommonElements();
    InitJSON();    
    ClearPageControls();
    ReadConfigFromDisk();
    PopulateInputRegistersList();
    PopulateHoldingRegistersList();
    
    var registerTitle = 
    	"Format1: {<START ADDRESS>, <WORD COUNT>, <EUI64>, device_variable, <BURST MESSAGE>, <DEVICE VARIABLE CODE>, <DEVICE STATE>} \n" + 
    	"Format2: {<START ADDRESS>, <WORD COUNT>, <EUI64>, gw_info} \n" + 
    	"Format3: {<START ADDRESS>, <WORD COUNT>, <EUI64>, gw_code_word}"  
    	
    document.getElementById("txtInputRegister").title = registerTitle; 
    document.getElementById("txtHoldingRegister").title = registerTitle;   	   
}

function ReadConfigFromDisk() {
	try {
    	var service = new jsonrpc.ServiceProxy(serviceURL, methods);
    	var response = service.file.exists({file: FILE_MODBUS_INI});
    	if (response.length == 0){    		
    		alert("There is no "+FILE_MODBUS_INI+" file on the VR")
    		return;
    	};	
    } catch (e) {
    	HandleException(e, "Unexpected error reading "+FILE_MODBUS_INI);
    	return;
    };  
    
    try {    
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        // create if not exists
        service.file.create({file: FILE_MODBUS_INI});
        var maxVarsNo = 1000;
        CONTENT = service.config.getConfigPart({configFile: FILE_MODBUS_INI, maxVarsNo: maxVarsNo});
        var contentLength = GetVarsNo(CONTENT);
        if (maxVarsNo == contentLength) 
            alert("The loaded registers exceed the maximum limit of " + maxVarsNo + ".\nThe list is limited to " + maxVarsNo + " registers.");
    } catch(e) {
        HandleException(e, "Unexpected error reading data from " + FILE_MODBUS_INI + "!");
    }
}

function GetVarsNo(content) {
    if (content == null)
        return 0;

    var len = 0;
    for (var i = 0; i < content.length; i++) {   
        if (content[i].group != null) {
            len += content[i].variables.length;
        }
    }
    return len;
}

function PopulateInputRegistersList() {
    ClearList("lstInputRegisters");
    document.getElementById("txtInputRegister").value = "";    
    var rowCounter = 0;
    var lst = document.getElementById("lstInputRegisters");
    if (CONTENT != null) {
        for (var i = 0; i < CONTENT.length; i++) {   
            if (CONTENT[i].group != null && 
                CONTENT[i].group == "INPUT_REGISTERS") {
                for (var j = 0; j < CONTENT[i].variables.length; j++) {
                    if (CONTENT[i].variables[j].REGISTER != null) {
                        var op = new Option(CONTENT[i].variables[j].REGISTER, CONTENT[i].variables[j].REGISTER);
                        if ( rowCounter % 2 == 1) {
                            op.className = "listAlternateItem";
                        }
                        lst[lst.length] = op;
                        rowCounter++;
                    }
                }
            }
        }
    }
}

function PopulateHoldingRegistersList() {
    ClearList("lstHoldingRegisters");
    document.getElementById("txtHoldingRegister").value = "";
    var rowCounter = 0;
    var lst = document.getElementById("lstHoldingRegisters");
    if (CONTENT != null) {
        for (var i = 0; i < CONTENT.length; i++) {   
            if (CONTENT[i].group != null && 
                CONTENT[i].group == "HOLDING_REGISTERS") { 
                for (var j = 0; j < CONTENT[i].variables.length; j++) {
                    if (CONTENT[i].variables[j].REGISTER != null) {
                        var op = new Option(CONTENT[i].variables[j].REGISTER, CONTENT[i].variables[j].REGISTER);
                        if ( rowCounter % 2 == 1) {
                            op.className = "listAlternateItem";
                        }
                        lst[lst.length] = op;
                        rowCounter++;
                    }
                }
            }
        }
    }
}

function EditInputRegister() {
    var txtInputRegister = document.getElementById("txtInputRegister");
    txtInputRegister.value = document.getElementById("lstInputRegisters").value;
    ClearOperationResults();
}

function SaveInputRegister() {
    ClearOperationResults(); 
    document.getElementById("txtInputRegister").value = document.getElementById("txtInputRegister").value.trimAll();
    var txtInputRegister = document.getElementById("txtInputRegister");
    if (ValidateRegisterServer(txtInputRegister, "Input Register")) {
        try {   
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            var response = service.isa100.modbus.server.setRow({section: "INPUT_REGISTERS", rowValue: txtInputRegister.value});
            if(response) {
                ReadConfigFromDisk();
                PopulateInputRegistersList();
                DisplayOperationResult("spnOperationResultInputRegister", "Input register saved successfully.");
            } else {
                alert ("Error saving 'Input register'!");
            }
        } catch(e) {
            HandleException(e, "Unexpected error saving 'Input register'!");
            return;
        }
    }
}

function DeleteInputRegister() {
    ClearOperationResults();
    var lstInputRegisters = document.getElementById("lstInputRegisters");
	if (lstInputRegisters.length == 0) {
		alert("The 'Input Registers' list is empty!")
		return;
	}
	if (lstInputRegisters.selectedIndex < 0) {
		alert("Please select an 'Input Register' first!")
		return;
	}
    if (confirm("Are you sure you want to delete the Input Register?")){                 
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            var response = service.isa100.modbus.server.delRow({section: "INPUT_REGISTERS", rowValue: lstInputRegisters.value});
            if(response) {
                ReadConfigFromDisk();
                PopulateInputRegistersList();
                DisplayOperationResult("spnOperationResultInputRegister", "Input register deleted successfully.");
            };
        } catch(e) {
            HandleException(e, "'Input Register' not deleted or not found !");
            return;
        };
    };        
}

function EditHoldingRegister() {
    var txtHoldingRegister = document.getElementById("txtHoldingRegister");
    txtHoldingRegister.value = document.getElementById("lstHoldingRegisters").value;
    ClearOperationResults();
}

function SaveHoldingRegister() {
    ClearOperationResults();
    document.getElementById("txtHoldingRegister").value = document.getElementById("txtHoldingRegister").value.trimAll();
    var txtHoldingRegister = document.getElementById("txtHoldingRegister");
    if (ValidateRegisterServer(txtHoldingRegister, "Holding Register")) {
        try {   
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            var response = service.isa100.modbus.server.setRow({section: "HOLDING_REGISTERS", rowValue: txtHoldingRegister.value});
            if(response) {
                ReadConfigFromDisk();
                PopulateHoldingRegistersList();
                DisplayOperationResult("spnOperationResultHoldingRegister", "Holding register saved successfully.");
            } else {
                alert ("Error saving 'Holding Register' !");
            }
        } catch(e) {
            HandleException(e, "Unexpected error saving 'Holding Register' !");
            return;
        }
    }
}

function DeleteHoldingRegister() {
    ClearOperationResults();
    var lstHoldingRegisters = document.getElementById("lstHoldingRegisters");
    if (lstHoldingRegisters.length == 0) {
        alert("The 'Holding Registers' list is empty!")
        return;
    }
    if (lstHoldingRegisters.selectedIndex < 0) {
        alert("Please select a 'Holding Register' first!")
        return;
    }
    if (confirm("Are you sure you want to delete the Holding Register?")){
        try {
            var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            var response = service.isa100.modbus.server.delRow({section: "HOLDING_REGISTERS", rowValue: lstHoldingRegisters.value});
            if(response) {
                ReadConfigFromDisk();
                PopulateHoldingRegistersList();
                DisplayOperationResult("spnOperationResultHoldingRegister", "Holding register deleted successfully.");
            };
        } catch(e) {
            HandleException(e, "'Holding Register' not deleted or not found !");
            return;
        };
    };
}

function ClearOperationResults() {
    ClearOperationResult("spnOperationResultInputRegister");
    ClearOperationResult("spnOperationResultHoldingRegister");
    ClearOperationResult("spnOperationResultActivate");
}

function Activate() {
    ClearOperationResults();
    try {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        var response = service.misc.applyConfigChanges({module: "modbus_gw"});
        if(response) {
            DisplayOperationResult("spnOperationResultActivate", "Activate completed successfully.");
        } else {
            alert ("Error activating registers!");
        }
    } catch(e) {
        HandleException(e, "Unexpected error activating registers!");
        return;
    }
}

function ClearPageControls() {
    ClearList("lstInputRegisters");
    ClearList("lstHoldingRegisters");  
    document.getElementById("txtInputRegister").value = "";
    document.getElementById("txtHoldingRegister").value = "";
}

function Download() {
    ClearOperationResults();
    try {    
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        service.user.isValidSession();            
	    document.form1.call1.value = jsonrpc.JSONRPCMethod("a").jsonRequest(1, "misc.fileDownload", {file: FILE_MODBUS_INI});
        return 1 ;
    } catch (e) {
        HandleException(e, 'Unexpected error downloading file !')
    };        
}

function Upload() {
    ClearOperationResults();
    var file = document.getElementById("modbusFile");
    if (file.value == ""){
        alert('There is no selected file. Please select a file!');
        return false;    	
    };    
    if (getFileName(file.value) != FILE_MODBUS_INI){
        alert('The name of the uploaded file must be ' + FILE_MODBUS_INI);
        return false;
    } else {   
        if (confirm("The previous host list will be lost. Are you sure you want to replace the file?")){
            try {    
                var service = new jsonrpc.ServiceProxy(serviceURL, methods);
                service.user.isValidSession();            
                document.form1.call1.value = jsonrpc.JSONRPCMethod("a").jsonRequest(1,"misc.fileUpload", {file: FILE_MODBUS_INI});
                return 1 ;
            } catch (e) {
                HandleException(e, 'Unexpected error downloading file !')
            };        
        } else {
            return false;
        };
    };    
}

function ShowHideHelp(op) {
    var divHelp = document.getElementById("divHelp");
    if (op == "open") {
        divHelp.style.display = "";
    } else {
        divHelp.style.display = "none";
    }
}

function operationDoneListener(text) {
    if (text.error) {
    	if (text.error == "No such file."){
    		alert("There is no " + FILE_MODBUS_INI + " file on the VR");	
    	} else {
    		alert("Error uploading " + FILE_MODBUS_INI + " file !");
    	};                
    } else {
        DisplayOperationResult("spnOperationResultActivate", "Upload completed successfully.");
        // refresh data from disk
	    ClearPageControls();
	    ReadConfigFromDisk();
	    PopulateInputRegistersList();
	    PopulateHoldingRegistersList();
    }
}
