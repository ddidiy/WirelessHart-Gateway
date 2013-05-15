var deviceId = null;
var dev = null;
var CurrentPage = 1;
var TotalPages = 0;
var TotalNoOfRows = 0;
var PageSize = 10;

function InitScheduleReportPage() {
	var PageSize = 10;
    SetPageCommonElements();  
    InitJSON();    
    deviceId = GetPageParamValue("deviceId");
    if (!isNaN(deviceId)) { //make sure qs was not altered (is a number)
        dev = GetDeviceInformation(deviceId);
        if (dev != null) {
            SetData(dev);    
            SetDeviceTabs(deviceId, dev.DeviceStatus, dev.DeviceRoleID, 4);
        }
        BuildScheduleReportTable();
    }
}

function BuildScheduleReportTable() {	
    TotalNoOfRows = GetDeviceScheduleReportCount(deviceId);
    if (TotalNoOfRows > 0) {
        var data = GetDeviceScheduleReportPage(deviceId, PageSize, CurrentPage, TotalNoOfRows);
        document.getElementById("tblScheduleReport").innerHTML = TrimPath.processDOMTemplate("schedulereport_jst", data);    
    } else {
    	document.getElementById("spnPageNumber").innerHTML = "";
        document.getElementById("tblScheduleReport").innerHTML = //"<span class='labels'>No records !</span>";
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"500px\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
     					"<tr><td class=\"tableSubHeader\" style=\"width:100px;\" align=\"center\">Superframe ID</td>" +
						    "<td class=\"tableSubHeader\" style=\"width:100px;\" align=\"center\">Time Slots</td>" +
						    "<td class=\"tableSubHeader\" style=\"width:100px;\" align=\"center\">Is Active</td>"+
						    "<td class=\"tableSubHeader\" style=\"width:100px;\" align=\"center\">Is Handheld</td>"+
						    "<td class=\"tableSubHeader\" style=\"width:100px;\" align=\"center\">Links</td></tr>" +
                        "<tr><td colspan=\"5\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
    }
    SetPager();
}

function GetRFChannelColor(channelStatus) {
    switch (channelStatus) {
        case 0: return "#FF3737";
        case 1: return "#3737FF";
        default: return "#000000";
    }
}

function SetData(dev) {
    document.getElementById("spnEUI64").innerHTML = dev.Address64;
    document.getElementById("spnNickname").innerHTML = dev.Nickname;
}

function PageNavigate(pageNo) {
	PageSize = document.getElementById("ddlRowsPerPage").value;	
    CurrentPage = pageNo;
    BuildNeighborsHealthTable();
}

function BackButtonClicked() {
    document.location.href = "devicelist.html?setState";
}
