var methods = ["sqldal.execute", "user.logout", "sqldal.open", "sqldal.close"];
var devices = null;
var networkManager = null;

function InitFWCommandsPage() {    
    SetPageCommonElements();
    InitJSON();
    networkManager = GetNetworkManagerDevice();
    SetData();
}

function SetData() {
    var cmds = GetFirmwareCommandsArray();
    var ddlCommands = document.getElementById("ddlCommands");    
    ddlCommands.options[0] = new Option("<select>","");
    for(i = 0;i < cmds.length; i++) {
        ddlCommands.options[i+1] = new Option(GetCommandName(cmds[i]),cmds[i]);
    }
}

function CommandSelectionChanged() {
    ClearOperationResult("spnExecuteResult");
    //document.getElementById("spnSelectedDevice").style.display = "none";
    HideCommandParameters();
    
    var commandId = Number(document.getElementById("ddlCommands").value);
    
    //var spnDevice = document.getElementById("spnDevice");
    //var spnTargetDevice = document.getElementById("spnTargetDevice");
    var spnFileName = document.getElementById("spnFileName");
    
    var ddlDevice = document.getElementById("ddlDevice");
    //var ddlTargetDevice = document.getElementById("ddlTargetDevice");
    var ddlFileName = document.getElementById("ddlFileName");    

    //devices
    var devTypeArray = new Array();
    devices = GetRegisteredDeviceListByType(GetDeviceTypeArrayForFirmwareExecution());
    ddlDevice.options[0] = new Option("<select>", "");
    if (devices != null) {
        for (i = 0; i < devices.length; i++) {
            ddlDevice.options[i+1] = new Option(devices[i].Address64, devices[i].DeviceID);
        }
    }
        
    //target devices    
/*    
    var targetDevices = GetRegisteredDeviceListByType(Array(DT_Device, DT_DeviceNonRouting, DT_AccessPoint));    
    ddlTargetDevice.options[0] = new Option("<select>", "");
    if (targetDevices != null) {
        for (i = 0; i < targetDevices.length; i++) {
            ddlTargetDevice.options[i+1] = new Option(targetDevices[i].Address64, targetDevices[i].DeviceID);
            ddlTargetDevice.options[i+1].title = targetDevices[i].Address64;
        }
    }
*/    
    switch(commandId) {
        case CTC_CancelFirmwareUpdate:
            //spnDevice.style.display = "";
            //spnTargetDevice.style.display = "";
            break;
        case CTC_FirmwareUpdate:
            //spnDevice.style.display = "";
            //spnTargetDevice.style.display = "";
            spnFileName.style.display = "";            
            var files = GetFirmwareFiles();
            ddlFileName.options[0] = new Option("<select>", "");
            if (files != null) {
                for (i = 0; i < files.length; i++) {
                    ddlFileName.options[i+1] = new Option(files[i].FileName, files[i].FileName);
                }
            }
            break;
        case CTC_GetFirmwareUpdateStatus:
            //spnDevice.style.display = "";
            //spnTargetDevice.style.display = "";
            break;
        case CTC_GetFirmwareVersion:
            //spnDevice.style.display = "";
            //spnTargetDevice.style.display = "";
            break;

        default:
            ;    
    }
}

/*
function NetworkManagerCheck()   //trebe sau nu ?
{
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
*/

function ExecuteCommand() {
    var ddlCommands = document.getElementById("ddlCommands");
    if(ddlCommands.value == "") {
        alert("Field 'Command' is required !");
        return;   
    }
    
    var ddlDevice = document.getElementById("ddlDevice");
    //var ddlTargetDevice = document.getElementById("ddlTargetDevice");
    var ddlFileName = document.getElementById("ddlFileName");  
    
    var trackingNo;
    
    var cmd = new Command();
    var cmdParam = new CommandParameter();
    var curentDate = new Date();

    // validate input parameters    
    if(ddlDevice.value == "") {
        alert("Field 'Device' is required!");
        ddlDevice.focus();
        return;
    }
    /*
    if(ddlTargetDevice.value == "") {
        alert("Field 'Target Device' is required!");
        ddlTargetDevice.focus();
        return;
    }
    */
    switch (Number(ddlCommands.value)) {
    case CTC_CancelFirmwareUpdate:
        // read parameters values
        cmdParam.ParameterCode = CPC_CancelFirmwareUpdate_DeviceID;
        cmdParam.ParameterValue = ddlDevice.value;
        // read command attributes
        //cmd.TimePosted =  curentDate.format(APP_DateTimeFormat);
        cmd.TimePosted =  ConvertFromJSDateToSQLiteDate(GetUTCDate());        
        cmd.DeviceID = networkManager.DeviceID;
        cmd.CommandTypeCode = ddlCommands.value;
        cmd.CommandStatus = CS_New;
        cmd.ParametersDescription = GetParameterName(Number(cmd.CommandTypeCode), Number(cmdParam.ParameterCode)) + ": " + ddlDevice.options[ddlDevice.selectedIndex].text;
        //if (NetworkManagerCheck()) {
        var cmdParams = new Array(1);
        cmdParams[0] = cmdParam;
        trackingNo = AddCommand(cmd, cmdParams);
        //}
        break;
    case CTC_FirmwareUpdate:
        if(ddlFileName.value == "") {
            alert("Field 'File Name' is required!");
            ddlFileName.focus();
            return;
        }
        // read parameters values
        cmdParam.ParameterCode = CPC_FirmwareUpdate_DeviceID;
        cmdParam.ParameterValue = ddlDevice.value;
        
        var cmdParam1 = new CommandParameter();
        cmdParam1.ParameterCode = CPC_FirmwareUpdate_FileName;
        cmdParam1.ParameterValue = ddlFileName.value;
        //read command attributes
        //cmd.TimePosted =  curentDate.format(APP_DateTimeFormat);
        cmd.TimePosted =  ConvertFromJSDateToSQLiteDate(GetUTCDate());
        cmd.DeviceID = networkManager.DeviceID;
        cmd.CommandTypeCode = ddlCommands.value;
        cmd.CommandStatus = CS_New;
        cmd.ParametersDescription = GetParameterName(Number(cmd.CommandTypeCode), Number(cmdParam.ParameterCode)) + ": " + 
                                    ddlDevice.options[ddlDevice.selectedIndex].text;
        cmd.ParametersDescription = cmd.ParametersDescription + ", " + 
                                    GetParameterName(Number(cmd.CommandTypeCode), Number(cmdParam1.ParameterCode)) + ": " + 
                                    cmdParam1.ParameterValue;
        //if (NetworkManagerCheck()) {
        var cmdParams = new Array(2);
        cmdParams[0] = cmdParam;
        cmdParams[1] = cmdParam1;
        trackingNo = AddCommand(cmd, cmdParams);
        //}
        break;
    case CTC_GetFirmwareUpdateStatus:
        // read parameters values
        cmdParam.ParameterCode = CPC_GetFirmwareUpdateStatus_DeviceID;
        cmdParam.ParameterValue = ddlDevice.value;
        // read command attributes
        //cmd.TimePosted =  curentDate.format(APP_DateTimeFormat);
        cmd.TimePosted = ConvertFromJSDateToSQLiteDate(GetUTCDate()); 
        cmd.DeviceID = networkManager.DeviceID;
        cmd.CommandTypeCode = ddlCommands.value;
        cmd.CommandStatus = CS_New;
        cmd.ParametersDescription = GetParameterName(Number(cmd.CommandTypeCode), Number(cmdParam.ParameterCode)) + ": " + ddlDevice.options[ddlDevice.selectedIndex].text;
        //if (NetworkManagerCheck()) {
        var cmdParams = new Array(1);
        cmdParams[0] = cmdParam;
        trackingNo = AddCommand(cmd, cmdParams);
        //}
        break;
    case CTC_GetFirmwareVersion:
        // read parameters values
        cmdParam.ParameterCode = CPC_GetFirmwareVersion_DeviceID;
        cmdParam.ParameterValue = ddlDevice.value;
        // read command attributes
        //cmd.TimePosted =  curentDate.format(APP_DateTimeFormat);
        cmd.TimePosted = ConvertFromJSDateToSQLiteDate(GetUTCDate()); 
        cmd.DeviceID = networkManager.DeviceID;
        cmd.CommandTypeCode = ddlCommands.value;
        cmd.CommandStatus = CS_New;
        cmd.ParametersDescription = GetParameterName(Number(cmd.CommandTypeCode), Number(cmdParam.ParameterCode)) + ": " + ddlDevice.options[ddlDevice.selectedIndex].text;
        //if (NetworkManagerCheck()) {
        var cmdParams = new Array(1);
        cmdParams[0] = cmdParam;
        trackingNo = AddCommand(cmd, cmdParams);
        //}
        break;
    default: ;
    }
    
    if (trackingNo != null) {
        DisplayOperationResult("spnExecuteResult", "Command sent successfully. <br />  Tracking no: " + trackingNo + ". Go to <a href='commandslog.html'>Commands Log.</a>");
    }
}


function CancelClicked() {
    HideCommandParameters();
    document.getElementById("ddlCommands").selectedIndex = 0;
    document.getElementById("ddlDevice").selectedIndex = 0;
    ClearOperationResult("spnExecuteResult");
}


function HideCommandParameters() {
    //document.getElementById("spnDevice").style.display = "none";
    //document.getElementById("spnTargetDevice").style.display = "none";
    document.getElementById("spnFileName").style.display = "none";
}


function BackButtonClicked() {
    document.location.href = "devicefw.html";
}

/*
function DeviceSelectionChanged() {
    var ddlDevice = document.getElementById("ddlDevice");
    var spnSelectedDevice = document.getElementById("spnSelectedDevice");
    
    if (ddlDevice[ddlDevice.selectedIndex].value != "") {
        spnSelectedDevice.style.display = "";
    } else {
        spnSelectedDevice.style.display = "none";
    }
    
    
    var spnDeviceIPV6Address = document.getElementById("spnDeviceIPV6Address");
    var spnDeviceType = document.getElementById("spnDeviceType");
    for (i = 0; i < devices.length; i++) {
        if (ddlDevice[ddlDevice.selectedIndex].value == devices[i].DeviceID) {
            spnDeviceIPV6Address.innerHTML = devices[i].Nickname;
            spnDeviceType.innerHTML = devices[i].DeviceRole;
            return;
        }
    }
}
*/