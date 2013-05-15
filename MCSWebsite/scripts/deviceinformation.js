var CurrentPage = 1;
var PageSize = 10;
var TotalPages = 0;
var TotalNoOfRows = 0;
var deviceId = null;
var dev = null;
var healthData = null;

function InitDeviceInformationPage() {
    SetPageCommonElements();    
    InitJSON();    
    deviceId = GetPageParamValue("deviceId");
    if (!isNaN(deviceId)) { //make sure qs was not altered (is a number)
        var dev = GetDeviceInformation(deviceId);
        if (dev != null) {                                    
            SetDeviceTabs(deviceId, dev.DeviceStatus, dev.DeviceRoleID, 0)
            SetData(dev);
            BuildHeaderValuesTable(deviceId);
            BuildProcessValuesTable();
        }
    }    
}

function SetData(dev) { 
    document.getElementById("spnEUI64").innerHTML = dev.Address64;        
    document.getElementById("spnNickname").innerHTML = dev.Nickname;
    document.getElementById("spnDeviceRole").innerHTML = dev.DeviceRole;
    document.getElementById("spnDeviceStatus").innerHTML = GetDeviceStatusName(dev.DeviceStatus);
    document.getElementById("spnLastRead").innerHTML = dev.LastRead;
    document.getElementById("spnPowerStatus").innerHTML = dev.PowerSupplyStatus;
    
    document.getElementById("spnManufacturer").innerHTML = dev.Manufacturer;
    document.getElementById("spnModel").innerHTML = dev.Model;
    document.getElementById("spnRevision").innerHTML = dev.Revision;           
}

function SetHealthData(healthData){
	if (healthData != null){		
	    document.getElementById("spnGenerated").innerHTML = healthData.Generated;    
	    document.getElementById("spnAllTx").innerHTML = healthData.AllTx;
	    document.getElementById("spnNoACK").innerHTML = healthData.NoACK;
	    document.getElementById("spnTerminated").innerHTML = healthData.Terminated;   
	    document.getElementById("spnAllRx").innerHTML = healthData.AllRx;
	    document.getElementById("spnDLLFailures").innerHTML = healthData.DLLFailure;
	    document.getElementById("spnNLFailures").innerHTML = healthData.NLFailure;
	    document.getElementById("spnCRCError").innerHTML = healthData.CRCError;
	    document.getElementById("spnNonceLost").innerHTML = healthData.NonceLost;
	} else {
	    document.getElementById("spnGenerated").innerHTML = 0;    
	    document.getElementById("spnAllTx").innerHTML = 0;
	    document.getElementById("spnNoACK").innerHTML = 0;
	    document.getElementById("spnTerminated").innerHTML = 0;   
	    document.getElementById("spnAllRx").innerHTML = 0;
	    document.getElementById("spnDLLFailures").innerHTML = 0;
	    document.getElementById("spnNLFailures").innerHTML = 0;
	    document.getElementById("spnCRCError").innerHTML = 0;
	    document.getElementById("spnNonceLost").innerHTML = 0;		
	}
}

function BackButtonClicked() {
    document.location.href = "devicelist.html?setState";
}

function BuildProcessValuesTable() {
    TotalNoOfRows = GetDeviceChannelsCount(deviceId);    
    if (TotalNoOfRows > 0) {
        var data = GetDeviceChannelsValues(deviceId, PageSize, CurrentPage, TotalNoOfRows); 
        document.getElementById("tblChannels").innerHTML = TrimPath.processDOMTemplate("channels_jst", data);
    } else {
    	document.getElementById("spnPageNumber").innerHTML = "";
        document.getElementById("tblChannels").innerHTML =
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"890px\"><tr><td>" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
     					"<tr >" +
     					  "<td class=\"tableSubHeader\" style=\"width:110px;\" align=\"center\">Name</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:110px;\" align=\"center\">Burst Message</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:110px;\" align=\"center\">Device Variable Slot</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:110px;\" align=\"center\">Device Variable</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:110px;\" align=\"center\">Classification</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:110px;\" align=\"center\">Unit Code</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:110px;\" align=\"center\">Update Period</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:110px;\" align=\"center\">Max Update Period</td>" +
                        "<tr><td colspan=\"8\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
                
    }
    SetPager();    
}

function PageNavigate(pageNo) {
	PageSize = document.getElementById("ddlRowsPerPage").value;
    CurrentPage = pageNo;  
    BuildProcessValuesTable();
}

function BuildHeaderValuesTable(deviceId){
     if (!isNaN(deviceId)) {
        dev = GetDeviceInformation(deviceId);   
        if (dev != null) {
            SetData(dev);            
            healthData = GetDeviceHealthInfo(deviceId);                        	
            SetHealthData(healthData);	                                    
        };
    };
}