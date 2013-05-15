var CurrentPage = 1;
var TotalPages = 0;

//filters
var deviceId = null;
var Cmd = null;
var CmdStatus = null;
var PageSize = 10;

function InitDeviceFwPage() {
    SetPageCommonElements();
    InitJSON();    
    PopulateFilters();
    ReadFilters();
    BuildFirmwareCommandsTable();
}

function BuildFirmwareCommandsTable() {
    var rowCount = GetCommandsCount(null, null, deviceId, CmdStatus, Cmd, true);
    if (rowCount > 0) {
        //obtain data from database
        var data = GetCommandsPage(null, null, deviceId, CmdStatus, Cmd, true, CurrentPage, PageSize, rowCount, false);
        
        //process template
        document.getElementById("tblFirmwareCommands").innerHTML = TrimPath.processDOMTemplate("fwcommands_jst", data);
        document.getElementById("btnExport").disabled = "";
        SetPager();
         //obtain query for export & put it in hidden field        
        document.getElementById("hQuery").value = GetCommandsPage(null, null, deviceId, CmdStatus, Cmd, true, 1, 5000, rowCount, true);
    } else {
    	document.getElementById("spnPageNumber").innerHTML = "";
        document.getElementById("tblFirmwareCommands").innerHTML = //"<span class='labels'>No records !</span>";
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"950px\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
     					"<tr><td class=\"tableSubHeader\" style=\"width:  25px;\" align=\"center\">&nbsp;</td>" +
						  "<td class=\"tableSubHeader\" style=\"width: 100px;\" align=\"center\">Tracking No.</td>" +
						  "<td class=\"tableSubHeader\" style=\"width: 100px;\" align=\"center\">EUI-64 Address</td>" +
						  "<td class=\"tableSubHeader\" style=\"width: 100px;\" align=\"center\">Command</td>" +
						  "<td class=\"tableSubHeader\" style=\"width: 220px;\" align=\"center\">Parameters</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:  60px;\" align=\"center\">Status</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:  80px;\" align=\"center\">Posted Time</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:  80px;\" align=\"center\">Response Time</td>" +
		                  "<td class=\"tableSubHeader\" style=\"width: 19   0px;\" align=\"center\">Response</td></tr>" +
                        "<tr><td colspan=\"9\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
        document.getElementById("hQuery").value = "";
        document.getElementById("btnExport").disabled = "disabled";
    }
}

function SetPager() {
    var anchorFirst = document.getElementById("anchorFirst");
    var anchorPrev = document.getElementById("anchorPrev");
    var anchorNext = document.getElementById("anchorNext");
    var anchorLast = document.getElementById("anchorLast");
    var spnPageNumber = document.getElementById("spnPageNumber");
    
    if (TotalPages > 0) {
        spnPageNumber.innerHTML = CurrentPage + "/" + TotalPages;
    }
    
    if (CurrentPage > 1) {
        anchorFirst.className = "white";
        anchorFirst.href = "javascript:PageNavigate(1);";
        
        anchorPrev.className = "white";
        anchorPrev.href = "javascript:PageNavigate(" + (CurrentPage - 1) + ");";
    } else {
        anchorFirst.className = "tabLink";
        anchorPrev.className = "tabLink";   
    }
    if (CurrentPage < TotalPages) {
        anchorNext.className = "white";
        anchorNext.href = "javascript:PageNavigate(" + (CurrentPage * 1 + 1) + ");";
        
        anchorLast.className = "white";
        anchorLast.href = "javascript:PageNavigate(" + TotalPages + ");";
    } else {
        anchorNext.className = "tabLink";
        anchorLast.className = "tabLink";   
    }
}


function PageNavigate(pageNo) {
    CurrentPage = pageNo;
    BuildFirmwareCommandsTable();
}

//export related functions
function Export() {
    var q = document.getElementById("hQuery").value;
    document.form1.call1.value = jsonrpc.JSONRPCMethod("a").jsonRequest(1,"sqldal.getCsv", {query:q}) ;
    return 1;
}

function uploadComplete(text) {
    if (!text.result) {
        alert("Export failed !");
    }
}

function PopulateFilters() {
    //devices
    var ddlDevice = document.getElementById("ddlDevice");   
    var devices = GetDeviceListByType(null, null);
    ddlDevice.options[0] = new Option("All", "");
    if (devices != null) {
        for(i = 0; i < devices.length; i++) {
            ddlDevice.options[i+1] = new Option(devices[i].Address64, devices[i].DeviceID);
        }
    }

    //commands
    var ddlCommand = document.getElementById("ddlCommand");
    var cmdArr = GetFirmwareCommandsArray();
    ddlCommand.options[0] = new Option("All","");    
    if (cmdArr != null) {
        for (i = 0; i < cmdArr.length; i++) {
            ddlCommand.options[i+1] = new Option(GetCommandName(cmdArr[i]),cmdArr[i]);
        }
    }

    //commands status
    var ddlCommandStatus = document.getElementById("ddlCommandStatus");
    ddlCommandStatus.options[0] = new Option("All","");    
    ddlCommandStatus.options[1] = new Option(GetStatusName(CS_New), CS_New);      
    ddlCommandStatus.options[2] = new Option(GetStatusName(CS_Sent), CS_Sent);  
    ddlCommandStatus.options[3] = new Option(GetStatusName(CS_Responded), CS_Responded);
    ddlCommandStatus.options[4] = new Option(GetStatusName(CS_Failed), CS_Failed);

    //items per page
    var ddlRowsPerPage = document.getElementById("ddlRowsPerPage");   
    ddlRowsPerPage.options[0] = new Option("10","10");    
    ddlRowsPerPage.options[1] = new Option("15","15");      
    ddlRowsPerPage.options[2] = new Option("25","25");  
    ddlRowsPerPage.options[3] = new Option("100", "100");
}


function ReadFilters() {
    deviceId = document.getElementById("ddlDevice").value;
    if (deviceId.length == 0) {
        deviceId = null;
    }
      
    //we need an array for query param
    Cmd = document.getElementById("ddlCommand").value;
    if (Cmd.length == 0) {
        Cmd = GetFirmwareCommandsArray();    
    } else {
        var cmdArr = new Array(1); 
        cmdArr[0] = Cmd;
        Cmd = cmdArr;
    }
    
    CmdStatus = document.getElementById("ddlCommandStatus").value;
    if (CmdStatus.length == 0) {
        CmdStatus = null;
    }
       
    PageSize = document.getElementById("ddlRowsPerPage").value;
}

function Search() {
    ReadFilters();
    CurrentPage = 1;
    BuildFirmwareCommandsTable();
}

function NavigateToExecute() {
    document.location.href = "fwcommands.html";
}

function NavigateToFWFiles() {
    document.location.href = "fwfilelist.html";
}
