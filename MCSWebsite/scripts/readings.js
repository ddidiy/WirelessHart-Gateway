function Reading() {
    this.Address64 = null;  
    this.ReadingTime = null;
    this.ChannelName = null;
    this.Command = null;
    this.Name = null;
    this.DeviceVariable = null;
    this.Value = null;
    this.Classification = null;         
    this.UnitCode = null; 
}

var CurrentPage = 1;
var TotalPages = 0;
var TotalNoOfRows = 0;
var PageSize = 10;
//filters
var deviceId = null; //might come from QS also, read QS on InitREadingsPage
var CommandNo = null;
var Name = null;
var DeviceVariable = null; 
var deviceAddress64 = null;
var showDevices = 0; 

function InitReadingsPage() {
    SetPageCommonElements();
    PageSize = 10;
    document.getElementById("ddlRowsPerPage").value = PageSize;
    //obtain deviceId;
    var url = parent.document.URL;
    deviceId = Number(url.substring(url.indexOf('?') + 10, url.length));
    if (isNaN(deviceId)) {  //make sure qs was not altered (is a number)
        deviceId = null;
    };       
    InitJSON();
    ReadFilters();
    BuildReadingsTable();
}

function ReadFilters() {
    deviceAddress64 = document.getElementById("txtDevice").value;
    if (deviceAddress64.length == 0) {
    	deviceAddress64 = null;
    }
    showDevices = document.getElementById("ddlShowDevices").value;        
    Name = document.getElementById("txtName").value;
    if (Name.length == 0) {
    	Name = null;
    };
    var txtCommandNo = document.getElementById("txtCommandNo");
    if (txtCommandNo.value.length != 0 && !ValidateNumber(txtCommandNo, "Command No", 0, 65535)){
    	return false;   	
    } else {
    	CommandNo = document.getElementById("txtCommandNo").value;
    };    
    DeviceVariable = document.getElementById("txtDeviceVariable").value;
    if (DeviceVariable.length == 0) {
    	DeviceVariable = null;
    };       
    PageSize = document.getElementById("ddlRowsPerPage").value;
    return true;
}

function Search() {
    if (ReadFilters()){
        CurrentPage = 1;
        BuildReadingsTable();    	
    };
}

//export related functions
function Export() {
    try {    
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        service.user.isValidSession();        
        var q = document.getElementById("hQuery").value;        
        document.form1.call1.value = jsonrpc.JSONRPCMethod("a").jsonRequest(1,"sqldal.getCsv", {query:q}) ;
        return 1;
    } catch (e) {
        HandleException(e, 'Unexpected error exporting data !')
    };    
    
}

function uploadComplete(text) {
    if (!text.result) {
        alert("Export failed !");
    }
}
//end export related functions

function BuildReadingsTable() {
    TotalNoOfRows = GetReadingsCount(deviceId, CommandNo, Name, DeviceVariable, deviceAddress64, showDevices);
    if (TotalNoOfRows > 0) {
        //obtain data from database
        var data = GetReadings(deviceId, CommandNo, Name, DeviceVariable, CurrentPage, PageSize, TotalNoOfRows, false, deviceAddress64, showDevices);
        if (data != null) {
            //process template 
            document.getElementById("tblReadings").innerHTML = TrimPath.processDOMTemplate("readings_jst", data);
            document.getElementById("btnExport").disabled = "";   
            document.getElementById("btnExport").style.cursor = "pointer";
            //obtain query for export & put it in hidden field        
            document.getElementById("hQuery").value = GetReadings(deviceId, CommandNo, Name, DeviceVariable, 1, 5000, TotalNoOfRows, true, deviceAddress64, showDevices);
        } else {
        	document.getElementById("spnPageNumber").innerHTML = "";
            document.getElementById("tblReadings").innerHTML = //"<span class='labels'>No records !</span>";
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"950px\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
    					"<tr><td class=\"tableSubHeader\" style=\"width: 5px; text-align: center;\">&nbsp;</td>" +
    						"<td class=\"tableSubHeader\" style=\"width: 150px; text-align: left;\">EUI-64 Address</td>" +
    						"<td class=\"tableSubHeader\" style=\"width: 150px; text-align: left;\">Timestamp</td>" +
    						"<td class=\"tableSubHeader\" style=\"width: 100px; text-align: center;\">Name</td>" +
    						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">Command</td>" +
    						"<td class=\"tableSubHeader\" style=\"width: 100px; text-align: center;\">Device Variable</td>" +
    						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">Value</td>" +
    						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">Classification</td>" +
    						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">UnitCode</td>" +
    						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">Update Period</td>" +
    						"<td class=\"tableSubHeader\" style=\"width: 100px; text-align: center;\">Last Update</td>" +
    						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">Received</td>" +
    						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">Missed</td>" +    						    						
    						"</tr>" +
                        "<tr><td colspan=\"13\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
            document.getElementById("hQuery").value = "";
            document.getElementById("btnExport").disabled = "disabled";
            document.getElementById("btnExport").style.cursor = "default";
        }
    } else {
    	document.getElementById("spnPageNumber").innerHTML = "";
        document.getElementById("tblReadings").innerHTML = //"<span class='labels'>No records !</span>";
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"950px\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
    				"<tr><td class=\"tableSubHeader\" style=\"width: 5px; text-align: center;\">&nbsp;</td>" +	
    					"<td class=\"tableSubHeader\" style=\"width: 150px; text-align: left;\">EUI-64 Address</td>" +
						"<td class=\"tableSubHeader\" style=\"width: 150px; text-align: left;\">Timestamp</td>" +
						"<td class=\"tableSubHeader\" style=\"width: 100px; text-align: center;\">Name</td>" +
						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">Command</td>" +
						"<td class=\"tableSubHeader\" style=\"width: 100px; text-align: center;\">Device Variable</td>" +
						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">Value</td>" +
						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">Classification</td>" +
						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">UnitCode</td>" +
						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">Update Period</td>" +
						"<td class=\"tableSubHeader\" style=\"width: 100px; text-align: center;\">Last Update</td>" +
						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">Received</td>" +
						"<td class=\"tableSubHeader\" style=\"width: 50px; text-align: center;\">Missed</td>" +    						    						
						"</tr>" +
                        "<tr><td colspan=\"13\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
        document.getElementById("hQuery").value = "";
        document.getElementById("btnExport").disabled = "disabled";
        document.getElementById("btnExport").style.cursor = "default";
    }
    SetPager();    
}

function PageNavigate(pageNo) {
    CurrentPage = pageNo;
    BuildReadingsTable();
}
