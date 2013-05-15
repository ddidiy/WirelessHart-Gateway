var deviceId = null;
var deviceRoleId = null;

function InitDeviceCommandsPage() {
    SetPageCommonElements();   
    InitJSON();    
    //obtain deviceId;
    deviceId = GetPageParamValue("deviceId");
    if (!isNaN(deviceId)) {  //make sure qs was not altered (is a number)
        dev = GetDeviceInformation(deviceId);
        if (dev != null) {
            SetData(dev);                           
            SetDeviceTabs(deviceId, dev.DeviceStatus, dev.DeviceRoleID, 6);    
        }
    }
}


function SetData(dev) {
    document.getElementById("spnEUI64").innerHTML = dev.Address64;
    document.getElementById("spnNickname").innerHTML = dev.Nickname;
    var cmds = [CTC_WHGeneralCommand];
    var ddlCommands = document.getElementById("ddlCommands");
    ddlCommands.options[0] = new Option("<select>","");    
    if (cmds) {
        var k=1;
        for(var i=0; i<cmds.length; i++) {
        	ddlCommands.options[k++] = new Option(GetCommandName(cmds[i], null),cmds[i]);
        }
    }
}


function CommandSelectionChanged() {
    ClearOperationResult("spnExecuteResult");
    HideCommandParameters();
    ClearCommandsParametersFields();
    
    var commandCode = document.getElementById("ddlCommands").value;
    var commandId = Number(commandCode.split(".")[0]);
    switch(commandId) {

    case CTC_WHGeneralCommand:
    	var spnCommandNo = document.getElementById("spnCommandNo");
    	var spnBypassIOCache = document.getElementById("spnBypassIOCache");
    	spnCommandNo.style.display = "";
    	spnBypassIOCache.style.display = "";
    	break;    
                   	   
    };	
}

function NetworkManagerCheck() {
    var networkManager = GetNetworkManagerDevice();
	if (networkManager == null) {
		alert("No registered Network Manager !");
		return false;
	}
	if (networkManager.DeviceStatus < DS_NormalOperationCommencing) {
	    alert("No registered Network Manager !");
  		return false;
    }
    return true;
}

function ExecuteCommand() {
    var ddlCommands = document.getElementById("ddlCommands");
    if(ddlCommands.value == "") {
        alert("Field 'Command' is required !");
        return;   
    }
    var ddlBypassIOCache = document.getElementById("ddlBypassIOCache");
    
    var trackingNo;
    var cmd = new Command();
    var cmdParam = new CommandParameter();
    var cmdParam1 = new CommandParameter();
    var cmdParam2 = new CommandParameter();
    var cmdParam3 = new CommandParameter();
    var cmdParam4 = new CommandParameter();
    var cmdParam5 = new CommandParameter();
    var cmdParam6 = new CommandParameter();
    var cmdParam7 = new CommandParameter();
    
    var spnDeviceVariable = document.getElementById("spnDeviceVariable");
    var networkManager = GetNetworkManagerDevice();
    if (networkManager == null) {
        return;
    }
    
    var commandCode = Number(ddlCommands.value);
    switch(commandCode) {
    case CTC_WHGeneralCommand:    	
    	var txtCommandNo = document.getElementById("txtCommandNo");
		if (!ValidateRequired(txtCommandNo, "Command No") || 
		    !ValidateNumber(txtCommandNo, "Command No", 0, 65535)) {
            return;
        }

        var txtCommandHex = document.getElementById("txtCommandHex");
        if (txtCommandHex.value != "" && 
		    !ValidateHex(txtCommandHex,'Command (hex)')){
                HexToDec(txtCommandHex.value);
                return;
        }
        cmdParam.ParameterCode = CPC_CommandNo;
        cmdParam.ParameterValue = txtCommandNo.value;        
        cmdParam1.ParameterCode = CPC_DataBuffer;
        cmdParam1.ParameterValue = txtCommandHex.value;        
        
        cmd.DeviceID = deviceId;
        cmd.CommandTypeCode = commandCode;
        cmd.CommandStatus = CS_New;        
        cmd.TimePosted = ConvertFromJSDateToSQLiteDate(GetUTCDate());        
        cmd.ParametersDescription = GetParameterName(Number(cmd.CommandTypeCode), Number(cmdParam.ParameterCode)) + ":" + cmdParam.ParameterValue + ', ' + 
        							GetParameterName(Number(cmd.CommandTypeCode), Number(cmdParam1.ParameterCode)) + ":" + cmdParam1.ParameterValue;
        var cmdParams = new Array();
        cmdParams[0] = cmdParam;
        cmdParams[1] = cmdParam1;

        cmdParam2.ParameterCode = CPC_WHGeneralCommand_BypassIOCache;
        cmdParam2.ParameterValue = ddlBypassIOCache.value;
    	var paramVal = "No";
    	if (cmdParam2.ParameterValue==1){
    		paramVal = "Yes"
    	}        	
        cmd.ParametersDescription = cmd.ParametersDescription + ', ' +        
		GetParameterName(Number(cmd.CommandTypeCode), Number(cmdParam2.ParameterCode)) + ":" + paramVal;
        cmdParams[2] = cmdParam2;
        
        trackingNo = AddCommand(cmd, cmdParams);
        break;    
    /*    
    case CTC_ReadValue:
        var txtDeviceVariable = document.getElementById("txtDeviceVariable");
        if(txtDeviceVariable.value == "") {
            alert("Field 'Device Variable' is required!");
            return;
        }                
        // command parameters
        cmdParam.ParameterCode = CPC_ReadValue_Channel;
        cmdParam.ParameterValue = txtDeviceVariable.value;
        
        // ReadValue command   
        cmd.DeviceID = deviceId;
        cmd.CommandTypeCode = commandCode;
        cmd.CommandStatus = CS_New;
        cmd.TimePosted = ConvertFromJSDateToSQLiteDate(GetUTCDate());              
        cmd.ParametersDescription = GetParameterName(Number(cmd.CommandTypeCode), Number(cmdParam.ParameterCode)) + ": " + txtDeviceVariable.value;
       
        var cmdParams = new Array();
        cmdParams[0] = cmdParam;

        cmdParam1.ParameterCode = CPC_ReadValue_BypassIOCache;
        cmdParam1.ParameterValue = ddlBypassIOCache.value;
    	var paramVal = "No";
    	if (cmdParam1.ParameterValue==1){
    		paramVal = "Yes"
    	} 
    	cmd.ParametersDescription = cmd.ParametersDescription + ', ' +
		GetParameterName(Number(cmd.CommandTypeCode), Number(cmdParam1.ParameterCode)) + ":" + paramVal;
        cmdParams[1] = cmdParam1;
                
        trackingNo = AddCommand(cmd, cmdParams);
        break;
    */
    default: 
    	return;
    }
   
    if (trackingNo != null && trackingNo > 0) {
        DisplayOperationResult("spnExecuteResult", "Command sent successfully. <br />  Tracking no: " + trackingNo + ". Go to <a href='commandslog.html'>Commands Log.</a>");
    } else {
        DisplayOperationResult("spnExecuteResult", "<font color='#FF0000'>Command sent error!</font>");
    }    
}

function CancelClicked() {
    HideCommandParameters();
    document.getElementById("ddlCommands").selectedIndex = 0;
    document.getElementById("ddlBypassIOCache").selectedIndex = 0;
    ClearOperationResult("spnExecuteResult");
}

function HideCommandParameters() {
    var spnRestartType = document.getElementById("spnRestartType");
    spnRestartType.style.display = "none";
    var spnCommandNo = document.getElementById("spnCommandNo");
    spnCommandNo.style.display = "none";
    var spnDeviceVariable = document.getElementById("spnDeviceVariable");
    spnDeviceVariable.style.display = "none";
    var spnBypassIOCache = document.getElementById("spnBypassIOCache");
    spnBypassIOCache.style.display = "none";
}

function BackButtonClicked() {
    document.location.href = "devicelist.html?setState";
}

function ClearCommandsParametersFields() {
    document.getElementById("txtCommandNo").value = "";
    document.getElementById("txtCommandHex").value = "";
    document.getElementById("ddlBypassIOCache").selectedIndex = 0;
}
