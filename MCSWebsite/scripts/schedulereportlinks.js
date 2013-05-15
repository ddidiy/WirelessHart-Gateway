var deviceId = null;
var superframeId = null;

//filters variables
var NeighborDevice = null;
var LinkType = null;
var Direction = null;
var PageSize = 10;
var TotalNoOfRows = 0;

// paging variables
var CurrentPage = 1;
var TotalPages = 0;


function InitScheduleReportLinksPage() {
	PageSize = 10;
	SetPageCommonElements();    
    InitJSON();    
    //obtain deviceId;
    deviceId = GetPageParamValue("deviceId");
    superframeId = GetPageParamValue("superframeId");

    if (!isNaN(deviceId) ) { //make sure qs was not altered (is a number)
        dev = GetDeviceInformation(deviceId);
        if (dev != null) {
            SetData(dev);                
            SetDeviceTabs(deviceId, dev.DeviceStatus, dev.DeviceRoleID, 4);
        }        
        PopulateFilters();        
        BuildScheduleReportLinksTable();
    }    
    BuildRFChannelsTable();
}

function PopulateFilters() {
    // Ppopulate Neighbor device list
    var ddlNeighborDevice = document.getElementById("ddlNeighborDevice");
    var devices = GetNeighborsForScheduleReportLinksPage(deviceId,superframeId);
    ddlNeighborDevice.options[0] = new Option("All", "");
    if (devices != null) {
        for(i = 0; i < devices.length; i++) {
            ddlNeighborDevice.options[i+1] = new Option(devices[i].Nickname, devices[i].PeerID);
        }
    }

    //Populate Items per page list
    var ddlLinkType = document.getElementById("ddlLinkType");   
    ddlLinkType.options[0] = new Option("All", "");
    ddlLinkType.options[1] = new Option(GetLinkTypeName(SLT_Normal), SLT_Normal);
    ddlLinkType.options[2] = new Option(GetLinkTypeName(SLT_Discovery), SLT_Discovery);
    ddlLinkType.options[3] = new Option(GetLinkTypeName(SLT_Broadcast), SLT_Broadcast);
    ddlLinkType.options[4] = new Option(GetLinkTypeName(SLT_Join), SLT_Join);
    
    //Populate Items per page list
    var ddlRowsPerPage = document.getElementById("ddlRowsPerPage");   
    ddlRowsPerPage.options[0] = new Option("10","10");    
    ddlRowsPerPage.options[1] = new Option("15","15");      
    ddlRowsPerPage.options[2] = new Option("25","25");  
    ddlRowsPerPage.options[3] = new Option("100", "100");
}

function BuildScheduleReportLinksTable() {
    TotalNoOfRows = GetDeviceScheduleReportLinksCount(deviceId, superframeId, NeighborDevice, LinkType);
    if (TotalNoOfRows > 0) {
        var data = GetDeviceScheduleReportLinksPage(deviceId, superframeId, NeighborDevice, LinkType, PageSize, CurrentPage, TotalNoOfRows);
        document.getElementById("tblScheduleReportLinks").innerHTML = TrimPath.processDOMTemplate("schedulereportlinks_jst", data);
    } else {
    	document.getElementById("spnPageNumber").innerHTML = "";
        document.getElementById("tblScheduleReportLinks").innerHTML = //"<span class='labels'>No records !</span>";
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"99%\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
    					"<tr>" +
    						"<td class=\"tableSubHeader\" style=\"width:130px;\" align=\"center\">Peer</td>" +
    						"<td class=\"tableSubHeader\" style=\"width:80px;\" align=\"center\">Slot Index</td>" +
    						"<td class=\"tableSubHeader\" style=\"width:100px;\" align=\"center\">Channel Offset</td>" +
    						"<td class=\"tableSubHeader\" style=\"width:80px;\" align=\"center\">Transmit</td>" +
    						"<td class=\"tableSubHeader\" style=\"width:80px;\" align=\"center\">Receive</td>" +
    						"<td class=\"tableSubHeader\" style=\"width:100px;\" align=\"center\">Shared</td>" +
    						"<td class=\"tableSubHeader\" style=\"width:250px;\" align=\"center\">Link Type</td>" + 						
    					"</tr>" +
                        "<tr><td colspan=\"7\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
    }
    SetPager();    
}

function BuildRFChannelsTable() {
/*    var data = GetRFChannels();
    var stringRFChannels = "RF Channels: ";

    if (data != null) {
        for (var i=0; i<data.length; i++) {
            stringRFChannels = stringRFChannels + "<span style='background:" + GetRFChannelColor(data[i].ChannelStatus) + 
            ";color:#000000;border-style:solid;border-width:1px;'>&nbsp;&nbsp;<font color='#FFFFFF'>" + 
            data[i].ChannelNumber + "</font>&nbsp;&nbsp;</span>&nbsp;"
        }
        document.getElementById("divRFChannels").innerHTML = stringRFChannels;
    } else {
        document.getElementById("divRFChannels").innerHTML = "RF Channels: <span class='labels'>No records !</span>";
    }
*/    
}

function GetRFChannelColor(channelStatus) {
    switch (channelStatus) {
        case 0:  return "#FF3737";
        case 1:  return "#3737FF";
        default: return "#000000";
    }
}

function SetData(dev) {
    document.getElementById("spnEUI64").innerHTML = dev.Address64;
    document.getElementById("spnNickname").innerHTML = dev.Nickname;
    document.getElementById("spnSuperframeID").innerHTML = GetPageParamValue("superframeId");
}

function Search() {
    ReadFilters();
    CurrentPage = 1;
    BuildScheduleReportLinksTable();
}

function ReadFilters() {
    NeighborDevice = document.getElementById("ddlNeighborDevice").value;
    if (NeighborDevice.length == 0) NeighborDevice = null;

    LinkType = document.getElementById("ddlLinkType").value;
    if (LinkType.length == 0) LinkType = null;
   
    PageSize = document.getElementById("ddlRowsPerPage").value;
}

function PageNavigate(pageNo) {
    CurrentPage = pageNo;
    BuildScheduleReportLinksTable();
}

function BackButtonClicked() {
    document.location.href = "schedulereport.html?deviceId=" + GetPageParamValue("deviceId");
}
