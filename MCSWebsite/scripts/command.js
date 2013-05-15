function Command() {
    this.TrackingNo = null;
    this.DeviceID = null;
    this.CommandTypeCode = null;
    this.CommandStatus = null;
    this.TimePosted = null;
    this,TimeResponded = null;
    this.DeviceAddress64 = null;
    this.DeviceAddress128 = null;
    this.Response = null;
    this.ParametersDescription = null;
}

function CommandParameter() {
    this.ParameterCode = null;
    this.ParameterValue = null;
}

//command status
var CS_New = 0;
var CS_Sent = 1;
var CS_Responded = 2;
var CS_Failed = 3;
	
function GetStatusName(stat) {		
	switch (stat) {
	case CS_New:
	  return "New";
	case CS_Sent:
	  return "Sent";
	case CS_Responded:
	  return "Responded";
	case CS_Failed:
	  return "Failed";			
	default:
	  return NAString;
	}
}

//command type codes and command parameters codes 
var CTC_RequestTopology = 0;
var CTC_ReadValue = 1;
	var CPC_ReadValue_Channel =  10;
	var CPC_ReadValue_BypassIOCache = 1300;
var CTC_SubscribeForBurstNotifications = 3
var CTC_SubscribeForTopologyNotification = 9;
var CTC_WHGeneralCommand = 11;
	var CPC_CommandNo = 92;
	var CPC_DataBuffer = 93;
	var CPC_WHGeneralCommand_BypassIOCache = 1300;
var CTC_GetRoutes = 120;
var CTC_GetServices = 121;
var CTC_GetSuperframes = 122;
var CTC_GetScheduleLinks = 123;
var	CTC_RequestDeviceNeighborsHealth = 124;
	var CPC_RequestDeviceNeighborsHealth_DeviceID = 62;	
var	CTC_RequestDeviceHealth = 125;
	var CPC_RequestDeviceHealth_DeviceID = 63;
var CTC_AutoDetectBurstConfig = 126;
    var CPC_AutoDetectBurstConfig_DeviceID = 64;

function GetCommandName(commandCode, commandType) {
    switch(commandCode) {
    case CTC_RequestTopology: 
        return "Request Topology";
    case CTC_ReadValue: 
        return "Read Value";
    case CTC_SubscribeForBurstNotifications: 
        return "Subscribe for Burst Notifications";
    case CTC_SubscribeForTopologyNotification: 
        return "Subscribe for Topology Notification";
    case CTC_WHGeneralCommand: 
        return "WH General Command";
    case CTC_GetSuperframes:        
    	return "Request Superframes";
    case CTC_GetScheduleLinks:        
		return "Request Schedule Links";
    case CTC_RequestDeviceNeighborsHealth: 
        return "Request Device Neighbors Health";            
    case CTC_RequestDeviceHealth: 
        return "Request Device Health";
    case CTC_GetRoutes: 
        return "Request Routes and SourceRoutes";
    case CTC_GetServices: 
        return "Request Services";
	case CTC_AutoDetectBurstConfig:
        return "Auto Detect Burst Configuration";
    default: 
        return NAString;
    }
}

function GetParameterName(commandCode, parameterCode) {		
	switch (commandCode) {
	case CTC_RequestTopology:
		return NAString;
	case CTC_ReadValue:
		switch (parameterCode) {					
		case CPC_ReadValue_Channel:
            return "Variable";		
		case CPC_ReadValue_BypassIOCache:
			return "bypassIOCache";
		default:
			return NAString;
		}
	case CTC_SubscribeForBurstNotifications:
		return NAString;
	case CTC_SubscribeForTopologyNotification:
		return NAString;
    case CTC_WHGeneralCommand:
        switch (parameterCode) {
        case CPC_CommandNo:
            return "CommandNo";
        case CPC_DataBuffer:    
            return "DataBuffer";
		case CPC_WHGeneralCommand_BypassIOCache:
			return "bypassIOCache";            
        default: 
            return NAString;     
        }	
    case CTC_RequestDeviceNeighborsHealth:
    	switch(parameterCode){
    	case CPC_RequestDeviceNeighborsHealth_DeviceID:
    		return "Target Device ID";
        default: 
            return NAString;         		
    	}
    case CTC_RequestDeviceHealth:	
    	switch(parameterCode){
    	case CPC_RequestDeviceHealth_DeviceID:
    		return "Target Device ID";
        default: 
            return NAString;         		
    	}
	default:
		return NAString;			
	}
}

function GetCommandsArray() {
    var commArray = [];
    commArray.push(CTC_WHGeneralCommand);
    /* commArray.push(CTC_ReadValue);  ISSUE 510  
     * commArray.push(CTC_RequestDeviceHealth);
     * commArray.push(CTC_RequestDeviceNeighborsHealth); ISSUE 694 
     */
    commArray.push(CTC_GetRoutes);    
    commArray.push(CTC_GetScheduleLinks);
    commArray.push(CTC_GetServices);
    commArray.push(CTC_GetSuperframes);               
    commArray.push(CTC_RequestTopology);    
    commArray.push(CTC_SubscribeForBurstNotifications);
    commArray.push(CTC_SubscribeForTopologyNotification);
    return commArray;
}

function GetFirmwareCommandsArray() {
    return Array(/* TBD */);
}

// RestartTypes
var RT_WarmRestart = 2;
var RT_RestartAsProvisioned = 3;
var RT_ResetToFactoryDefaults = 4;

function GetRestartTypeName(type) {
	switch(type) {
	case RT_WarmRestart:
		return "Warm Restart";
	case RT_RestartAsProvisioned:
		return "Restart as provisioned";
	case RT_ResetToFactoryDefaults:
		return "Reset to factory defaults";
	default:
		return type;
	}
}
