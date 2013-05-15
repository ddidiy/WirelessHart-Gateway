function Device() {
    this.DeviceID = null;
    this.DeviceRole = null;
    this.DeviceRoleID = null;
    this.Nickname = null;
    this.Address64 = null;
    this.DeviceTag = null;
    this.DeviceStatus = null;
    this.LastRead = null;
    this.DeviceLevel = null;
    this.RejoinCount = null;
    this.DPDUsTransmitted = null;
    this.DPDUsReceived = null;
    this.DPDUsFailedTransmission = null;
    this.DPDUsFailedReception = null;
    this.PowerSupplyStatus = null;
    this.Manufacturer = null;
    this.Model = null;
    this.Revision = null;
    this.SubnetID = null;
}

function DeviceHistory() {
    this.Timestamp = null;
    this.DeviceStatus = null;
}


function HealthData(){
	this.Generated	= null;
	this.AllTx 		= null;
	this.NoACK 		= null;			
	this.Terminated	= null;
	this.AllRx 		= null;
	this.DLLFailure = null;
	this.NLFailure 	= null;
	this.CRCError 	= null;
	this.NonceLost 	= null;	
}

// device status
var DS_NotJoined = 0;                    	// Device not joined
var DS_NetworkPacketsHeard = 1;
var DS_ASNAcquired = 2;
var DS_SynchronizedToSlotTime = 4;
var DS_AdvertisementHeard = 8;
var DS_JoinRequested =16;  					//Set on transmission of first join request
var DS_Retrying = 32;	 					//Set after the first join request retry (i.e., when number of join requests > 2). Cleared when device is Authenticated or when Active Search started.
var DS_JoinFailed = 64 						//Set on transition from Active Search mode to Deep Sleep mode. Cleared on transition from Deep Sleep to Active Search.
var DS_Authenticated = 128  				//Network Key, Network Manager Session Established
var DS_NetworkJoined = 256;  				//Normal superframe and links obtained.
var DS_NegotiatingNetworkProperties = 512;  //Gateway session obtained. Initial Bandwidth requirements being negotiated with Network Manager.
var DS_NormalOperationCommencing = 1024;

function GetDeviceStatues(){
	return [DS_NotJoined, DS_JoinRequested, DS_JoinFailed, DS_Authenticated, DS_NetworkJoined, DS_NormalOperationCommencing]
} 

function GetDeviceStatusName(devStatus) {
    switch (devStatus) {
    case DS_NotJoined:
        return "NOT_JOINED";	
    case DS_NetworkPacketsHeard:
	    return "NET_PAK_HEARD";
    case DS_ASNAcquired:
	    return "ASN_ACQUIRED";
    case DS_SynchronizedToSlotTime:
	    return "SYNC_TIME";
    case DS_AdvertisementHeard:
    	return "ADV_HEARD";
    case DS_JoinRequested:    	    	
	    return "JOIN_REQ";
    case DS_Retrying:
	    return "RETRYING";
    case DS_JoinFailed:
	    //return "JOIN_FAILED"; Issue 537
    	return "DELETED";
    case DS_Authenticated:
	    return "AUTHENTICATED";
    case DS_NetworkJoined:
	    return "NET_JOINED";
    case DS_NegotiatingNetworkProperties:
	    return "NEG_NET_PROP";
    case DS_NormalOperationCommencing:
	    return "FULL_JOIN";
    default:
        return "NOT_JOINED"; //"UNKNOWN";
    }
}

function GetDeviceStatusColor(devStatus) {	
	if (devStatus == DS_NormalOperationCommencing){
		return "#008800";
	} else {//  if (devStatus == DS_NotJoined) {
    	return "#CC0000";
    }
}

function GetDeviceStatusDescription(devStatus) {
    switch (devStatus) {	
    case DS_NormalOperationCommencing:
	    return "Normal Operation Commencing";
    case DS_NotJoined:
    	return "Not Joined";
	default:
        return "Not Joined"; //"unknown";
    }
}

//  device types
//  ! VERY IMPORTANT !
//  when adding removing a device you must verify the methods below in order to update their arrays.
var DT_NetworkManager = 1;
var DT_Gateway = 2;
var DT_AccessPoint = 4; //3;
var DT_Device = 10;
var DT_DeviceNonRouting = 11;
var DT_DeviceIORouting = 12;
var DT_HartISAAdapter = 1000;
var DT_WirelessHartDevice = 1001;

function GetDeviceTypeArrayForReadingsReport(){
    return Array(DT_Device, DT_DeviceNonRouting, DT_DeviceIORouting, DT_HartISAAdapter, DT_WirelessHartDevice);
}

function GetDeviceTypeArrayForFirmwareExecution(){
    return Array(DT_Device, DT_DeviceNonRouting, DT_DeviceIORouting, DT_HartISAAdapter, DT_WirelessHartDevice, DT_AccessPoint)
}

function IsFieldDevice(deviceRole) {
	return ((deviceRole == DT_Device) || 
			(deviceRole == DT_DeviceNonRouting) || 
			(deviceRole == DT_DeviceIORouting) || 
			(deviceRole == DT_HartISAAdapter) || 
			(deviceRole == DT_WirelessHartDevice));
}

function GetDeviceRole(deviceRole) {
    $deviceRoleName = null;	
	switch (deviceRole) {
	case DT_NetworkManager:
		return "Network Manager";
	case DT_Gateway:
		return "Gateway";
	case DT_AccessPoint:
		return "Access Point";
	case DT_Device:
		return "Device";
	case DT_DeviceNonRouting:
		return "IO Device";
	case DT_DeviceIORouting:
		return "IO Router Device";
	case DT_HartISAAdapter:
		return "HART Adapter";
	case DT_WirelessHartDevice:
		return "WirelessHart Device";
	default:
		return "Unknown - " + deviceRole;
	}
}

function GetDeviceRoleColor(deviceRole) {
    $deviceRoleName = null;	
	switch (deviceRole) {
	case DT_NetworkManager:
		return "#337777";
	case DT_Gateway:
		return "#773377";
	case DT_AccessPoint:
		return "#777733";
	case DT_Device:
		return "#0000AA";
	case DT_DeviceIORouting:
		return "#AA0000";
	case DT_DeviceNonRouting:
		return "#00AA00";
	case DT_HartISAAdapter:
		return "#FFCC00";
	case DT_WirelessHartDevice:
		return "#CCFF00";
	default:
		return "#000000";
	}
}

function GetDeviceRoleImage(deviceRole) {		
	switch (deviceRole) {
		case DT_AccessPoint: 		return "BBR.gif";		
		case DT_Device: 			return "DEVICE.gif";
		case DT_DeviceNonRouting: 	return "DEVICE.gif";
		case DT_DeviceIORouting:	return "DEVICE.gif";
		case DT_HartISAAdapter:		return "DEVICE.gif";
		case DT_Gateway:			return "GW.gif";
		case DT_NetworkManager:		return "NM.gif";
		case DT_WirelessHartDevice:	return "DEVICE.gif";
		default:					return "DEVICE.gif";
	};
}  

var SETPUBLISHER_ERROR_NONE = 0,
    SETPUBLISHER_ERROR_READ_BURSTCONFIG = 1,
    SETPUBLISHER_ERROR_READ_BURSTCONFIG_CMD105_NOT_IMPL = 2,
    SETPUBLISHER_ERROR_TURNOFF_BURST = 3,
    SETPUBLISHER_ERROR_TURNOFF_BURST_CMD109_NOT_IMPL = 4,
    SETPUBLISHER_ERROR_GET_SUBDEVICEINDEX = 5,
    SETPUBLISHER_ERROR_CONFIGBURST = 6,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD102_NOT_IMPL = 7,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD103_NOT_IMPL = 8,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD104_NOT_IMPL = 9,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD106_NOT_IMPL = 10,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD107_NOT_IMPL = 11,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD108_NOT_IMPL = 12,
    SETPUBLISHER_ERROR_CONFIGBURST_CMD109_NOT_IMPL = 13,
    SETPUBLISHER_ERROR_CONFIGBURST_BUSY = 14,
    SETPUBLISHER_ERROR_CONFIGBURST_MAP_SUBDEVICE = 15,
    SETPUBLISHER_ERROR_CONFIGBURST_WRITE_UPDATE_PERIOD = 16,
    SETPUBLISHER_ERROR_CONFIGBURST_WRITE_TRIGGER = 17,
    SETPUBLISHER_ERROR_CONFIGBURST_WRITE_VARIABLES = 18,
    SETPUBLISHER_ERROR_CONFIGBURST_WRITE_COMMAND_NO = 19,
    SETPUBLISHER_ERROR_CONFIGBURST_SET_BURST_ON = 20,
    SETPUBLISHER_ERROR_FATAL = 21,
    SETPUBLISHER_ERROR_UNDEFINED = 22

function GetPublisherErrorText(errorNo){
	var err; 
	try{
		err = Number(errorNo)
	}catch(e){
		return "UNKNOWN";
	};
	
    switch (err)
    {
        case SETPUBLISHER_ERROR_NONE:                               return "";
        case SETPUBLISHER_ERROR_READ_BURSTCONFIG:                   return "Error on reading burst configuration";
        case SETPUBLISHER_ERROR_READ_BURSTCONFIG_CMD105_NOT_IMPL:   return "Command 105 is not implemented";
        case SETPUBLISHER_ERROR_TURNOFF_BURST:                      return "Error on turning off burst messages";
        case SETPUBLISHER_ERROR_TURNOFF_BURST_CMD109_NOT_IMPL:      return "Command 109 is not implemented";        
        case SETPUBLISHER_ERROR_GET_SUBDEVICEINDEX:                 return "Error on getting subdevice index";
        case SETPUBLISHER_ERROR_CONFIGBURST:                        return "Error on configuring burst message";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD102_NOT_IMPL:        return "Command 102 is not implemented";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD103_NOT_IMPL:        return "Command 103 is not implemented";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD104_NOT_IMPL:        return "Command 104 is not implemented";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD106_NOT_IMPL:        return "Command 106 is not implemented";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD107_NOT_IMPL:        return "Command 107 is not implemented";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD108_NOT_IMPL:        return "Command 108 is not implemented";
        case SETPUBLISHER_ERROR_CONFIGBURST_CMD109_NOT_IMPL:        return "Command 109 is not implemented";
        case SETPUBLISHER_ERROR_CONFIGBURST_BUSY:                   return "Device is busy";
        case SETPUBLISHER_ERROR_CONFIGBURST_MAP_SUBDEVICE:          return "Error on mapping subdevice";
        case SETPUBLISHER_ERROR_CONFIGBURST_WRITE_UPDATE_PERIOD:    return "Error on writting burst update period";
        case SETPUBLISHER_ERROR_CONFIGBURST_WRITE_TRIGGER:          return "Error on writting burst trigger";
        case SETPUBLISHER_ERROR_CONFIGBURST_WRITE_VARIABLES:        return "Error on writting burst variables";
        case SETPUBLISHER_ERROR_CONFIGBURST_WRITE_COMMAND_NO:       return "Error on writting burst command number";
        case SETPUBLISHER_ERROR_CONFIGBURST_SET_BURST_ON:           return "Error on turning on burst messege";
        case SETPUBLISHER_ERROR_FATAL:                              return "Fatal error";
        default:                                                    return "Unknown error";
    }
}    


function ParseBurstError(errors, messages) {
	var str= "";
	var burstErrors = errors.split(',');
	var burstMessages = messages.split(',');
	for (var i =0; i<burstErrors.length; i++){
		var error = burstErrors[i].split('=');
		str +=  "BURST[" +error[0] + "] = " + GetPublisherErrorText(error[1]); 
        str += ((burstMessages[i] != null && burstMessages[i] != "") ? " (" + burstMessages[i] + ")" : "") + '<br/>';
	}
	return str;
}

var	//SETPUBLISHER_STATE_NONE = 0,
	SETPUBLISHER_STATE_READ_BURSTCONFIG = 1,
	SETPUBLISHER_STATE_TURNOFF_BURST = 2,
	SETPUBLISHER_STATE_GET_SUBDEVICEINDEX = 3,
	SETPUBLISHER_STATE_CONFIGURE_BURST = 4,
	SETPUBLISHER_STATE_DONE = 5;

function GetPublisherStateName(stateVal){
    switch (stateVal)
    {
        //case SETPUBLISHER_STATE_NONE:               return "NONE";
        case SETPUBLISHER_STATE_READ_BURSTCONFIG:   return "READ BURST CONFIGURATION";
        case SETPUBLISHER_STATE_TURNOFF_BURST:      return "TURNOFF BURST MESSAGE";
        case SETPUBLISHER_STATE_GET_SUBDEVICEINDEX: return "GET SUBDEVICE INDEX";
        case SETPUBLISHER_STATE_CONFIGURE_BURST:    return "CONFIGURE BURST MESSAGE";
        case SETPUBLISHER_STATE_DONE:               return "CONFIGURED";
        default:                                    return "UNKNOWN";
    }
}

function GetPublisherStateArray(){
	return [
	        //{"ID" : SETPUBLISHER_STATE_NONE 				,"TEXT" : GetPublisherStateName(SETPUBLISHER_STATE_NONE)},
	        {"ID" : SETPUBLISHER_STATE_READ_BURSTCONFIG 	,"TEXT" : GetPublisherStateName(SETPUBLISHER_STATE_READ_BURSTCONFIG)},
	        {"ID" : SETPUBLISHER_STATE_TURNOFF_BURST 		,"TEXT" : GetPublisherStateName(SETPUBLISHER_STATE_TURNOFF_BURST)},
	        {"ID" : SETPUBLISHER_STATE_GET_SUBDEVICEINDEX 	,"TEXT" : GetPublisherStateName(SETPUBLISHER_STATE_GET_SUBDEVICEINDEX)},
	        {"ID" : SETPUBLISHER_STATE_CONFIGURE_BURST 		,"TEXT" : GetPublisherStateName(SETPUBLISHER_STATE_CONFIGURE_BURST)},
	        {"ID" : SETPUBLISHER_STATE_DONE 				,"TEXT" : GetPublisherStateName(SETPUBLISHER_STATE_DONE)}
	       ];
}