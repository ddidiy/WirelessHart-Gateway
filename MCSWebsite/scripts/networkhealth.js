var CurrentPage 	= 1;
var TotalPages 		= 0;
var PageSize 		= 10;
var TotalNoOfRows 	= 0;
var ShowDevices 	= 0;

function InitNetworkHealth() {
	SetPageCommonElements();	
	AddValueToArray(methods, "user.logout");
	InitJSON();
	PageSize    = document.getElementById("ddlRowsPerPage").value;
	ShowDevices = document.getElementById("ddlShowDevices").value;
    BuildNetworkHealthHeaderTable();	
	BuildNetworkHealthTable();	    
}

function ShowItems(){
	CurrentPage = 1;
    PageSize 	= document.getElementById("ddlRowsPerPage").value;
    ShowDevices = document.getElementById("ddlShowDevices").value;
    BuildNetworkHealthHeaderTable();	
	BuildNetworkHealthTable();	    
}

function BuildNetworkHealthHeaderTable() {
    var net = GetNetworkHealthReportHeader(ShowDevices);
	if (net != null) {	    
	    document.getElementById("spnDeviceCount").innerHTML = net.DeviceCount;
	    document.getElementById("spnJoinCount").innerHTML = net.JoinCount;	    
	    document.getElementById("spnStartDate").innerHTML = net.StartDate;
	    document.getElementById("spnCurrentDate").innerHTML = net.CurrentDate;
	    
	    document.getElementById("spnGenerated").innerHTML = net.Generated;
	    document.getElementById("spnAllTx").innerHTML = net.AllTx;
	    document.getElementById("spnNoACK").innerHTML = net.NoACK;
	    document.getElementById("spnTerminated").innerHTML = net.Terminated;
	    document.getElementById("spnAllRx").innerHTML = net.AllRx;
	    
	    document.getElementById("spnDLLFailures").innerHTML = net.DLLFailures;
	    document.getElementById("spnNLFailures").innerHTML = net.NLFailures;
	    document.getElementById("spnCRCError").innerHTML = net.CRCError;
	    document.getElementById("spnNonceLost").innerHTML = net.NonceLost;
	    
    } else {	    
	    document.getElementById("spnDeviceCount").innerHTML = 0;
	    document.getElementById("spnJoinCount").innerHTML = 0;	 	    
	    document.getElementById("spnStartDate").innerHTML = NAString;
	    document.getElementById("spnCurrentDate").innerHTML = NAString;
	    
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

function BuildNetworkHealthTable() {
    TotalNoOfRows = GetNetworkHealthReportPageCount(ShowDevices);
    if (TotalNoOfRows > 0) {
        var data = GetNetworkHealthReportPage(PageSize, CurrentPage, TotalNoOfRows, ShowDevices);  
        if (data != null) {
            document.getElementById("tblNetDevices").innerHTML = TrimPath.processDOMTemplate("netdevices_jst", data);
        } else {
            document.getElementById("tblNetDevices").innerHTML =
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"950px\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
     					"<tr>" +
	     					"<td class=\"tableSubHeader\" style=\"width: 110px;\" align=\"center\">EUI-64 Address</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">Join Count</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">Power Status</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">Generated</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">All Tx</td>"+
	     	    			"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">No ACK</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">Terminated</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">All Rx</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">DLL Failure</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">NL Failure</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">CRC Error</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">Nonce Lost</td>"+
	     				"</tr>"+	
                        "<tr><td colspan=\"12\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
        }
    } else {
    	document.getElementById("spnPageNumber").innerHTML = "";
        document.getElementById("tblNetDevices").innerHTML =
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"950px\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
     					"<tr>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 110px;\" align=\"center\">EUI-64 Address</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">Join Count</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">Power Status</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">Generated</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">All Tx</td>"+
	     	    			"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">No ACK</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">Terminated</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">All Rx</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">DLL Failure</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">NL Failure</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">CRC Error</td>"+
	     					"<td class=\"tableSubHeader\" style=\"width: 70px;\" align=\"center\">Nonce Lost</td>"+
     					"</tr>" +
                        "<tr><td colspan=\"12\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
    }
    SetPager();
}

function PageNavigate(pageNo) {
    CurrentPage = pageNo;    
    BuildNetworkHealthTable();
}
