function Alert() {
    this.DeviceID = null;
    this.Address64 = null;
    this.Time = null;    
    this.Type = null;
    this.PeerID_GraphID = null;
    this.Mic = null;    
}

// alert types
var AT_PathDown = 788;
var AT_SourceRouteFailed = 789;
var AT_GraphRouteFailed = 790;
var AT_TransportLayerFailed = 791;

function GetAlertTypeName(alertType) {
    switch (alertType) {
    case AT_PathDown:
        return "Path Down";
	case AT_SourceRouteFailed:
		return "Source Route Failed";
	case AT_GraphRouteFailed:
		return "Graph Route Failed";	
	case AT_TransportLayerFailed:
		return "Transport Layer Failed";	
	default:
        return "UNKNOWN";
    }
}

function GetAlertTypesArray(){
    return Array(AT_PathDown, AT_SourceRouteFailed, AT_GraphRouteFailed, AT_TransportLayerFailed);
}