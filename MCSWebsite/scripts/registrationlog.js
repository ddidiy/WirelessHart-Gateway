var deviceId = null;

//filters
var RegistrationStatus = null;
var StartTime = null;
var EndTime = null;
var PageSize = 10;

//order direction
var OD_ASC = "ASC";
var OD_DESC = "DESC";

var CurrentPage = 1;
var TotalPages = 0;
var TotalNoOfRows = 0;
var OrderBy = 1;  //order column number
var OrderDirection = OD_ASC;

function InitRegistrationLogPage() {
    SetPageCommonElements();
    InitJSON();
    var url = parent.document.URL;
    deviceId = Number(url.substring(url.indexOf('?') + 10, url.length));    
    if (!isNaN(deviceId)) {  //make sure qs was not altered (is a number)
        dev = GetDeviceInformation(deviceId);
        if (dev != null) {
            SetData(dev);            
            SetDeviceTabs(deviceId, dev.DeviceStatus, dev.DeviceRoleID, 2)            
        }
        ClearFilters();
        PopulateFilters();
        BuildRegistrationLogTable();
    }
}

function ClearFilters(){
	document.getElementById("txtStartDate").value = "";
	document.getElementById("txtStartDateHours").value = "";
	document.getElementById("txtStartDateMinutes").value = "";
	document.getElementById("ddlStartDateAMPM").value = "AM";
	document.getElementById("txtEndDate").value = "";
	document.getElementById("txtEndDateHours").value = "";
	document.getElementById("txtEndDateMinutes").value = "";
	document.getElementById("ddlEndDateAMPM").value = "AM";
}

function Search() {    
    if (ReadFilters()){
    	CurrentPage = 1;
        BuildRegistrationLogTable();        	
    };
}

function DeleteHistory() {
    if (confirm("Are you sure you want to delete device history ?")) {
    	DeleteDeviceHistory(deviceId);
    };
    CurrentPage = 1;
    BuildRegistrationLogTable();
}

function ReadFilters() {
    RegistrationStatus = document.getElementById("ddlRegistrationStatus").value;
    if (RegistrationStatus.length == 0) {
        RegistrationStatus = null;
    }    
	StartTime 	= ReadDateTime("txtStartDate", "txtStartDateHours", "txtStartDateMinutes", "ddlStartDateAMPM", true);
	EndTime 	= ReadDateTime("txtEndDate", "txtEndDateHours", "txtEndDateMinutes", "ddlEndDateAMPM", false);
	
    if (StartTime == null || EndTime == null) {
    	EndTime = null;
    	return false;    	
    };
    if (StartTime != null){
    	if (EndTime == -1) {
    		EndTime = null;
    		return true;
    	};   	
    	if (StartTime > EndTime && EndTime != null ) { 
            alert("Start Time must be less than or equal to End Time !");
            return false;
    	};        	
    }
    PageSize = document.getElementById("ddlRowsPerPage").value;
    return true;    
}

function BuildRegistrationLogTable() {
    //document.getElementById("btnDelete").style.display = "";
    document.getElementById("btnDelete").disabled = false;
    TotalNoOfRows = GetDeviceHistoryCount(deviceId, StartTime, EndTime, RegistrationStatus);
    if (TotalNoOfRows > 0) {
        //obtain data from database
        var data = GetDeviceHistoryPage(deviceId, StartTime, EndTime, RegistrationStatus, OrderBy, OrderDirection, PageSize, CurrentPage, TotalNoOfRows);
        //process template 
        document.getElementById("tblRegistrationLog").innerHTML = TrimPath.processDOMTemplate("registrationlog_jst", data);
        //display order direction arrow
        SetOrderSignForCurrentView(OrderBy);        
    } else {
    	document.getElementById("spnPageNumber").innerHTML = "";
        document.getElementById("btnDelete").disabled = true;
        document.getElementById("tblRegistrationLog").innerHTML = //"<span class='labels'>No records !</span>";
                 "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"600px\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
     					"<tr><td class=\"tableSubHeader\" style=\"width:300px;\" align=\"center\">Timestamp</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:300px;\" align=\"center\">Device Status</td></tr>" +
                        "<tr><td colspan=\"2\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
   }
   SetPager();
}

function PopulateFilters() {
    //registration status
    var ddlRegistrationStatus = document.getElementById("ddlRegistrationStatus");
    ddlRegistrationStatus.options[0] = new Option("All", "");
    var data = GetDeviceStatues();
    for (var i=0; i<data.length; i++){
        ddlRegistrationStatus.options[i+1] = new Option(GetDeviceStatusName(data[i]), data[i]);    	
    }

    //items per page
    var ddlRowsPerPage = document.getElementById("ddlRowsPerPage");   
    ddlRowsPerPage.options[0] = new Option("10","10");    
    ddlRowsPerPage.options[1] = new Option("15","15");      
    ddlRowsPerPage.options[2] = new Option("25","25");  
    ddlRowsPerPage.options[3] = new Option("100", "100");
  
    //set start date = now - 6 hour
	var dateNow = GetUTCDate()
	dateNow.setHours(dateNow.getHours() == 6 ? 12 : dateNow.getHours() - 6);
	PopulateDateTime("txtStartDate", "txtStartDateHours", "txtStartDateMinutes", "ddlStartDateAMPM", dateNow);
}

function SetData(dev) {
    document.getElementById("spnEUI64").innerHTML = dev.Address64;
    document.getElementById("spnNickname").innerHTML = dev.Nickname;
}

function PageNavigate(pageNo) {
	PageSize = document.getElementById("ddlRowsPerPage").value;	
    CurrentPage = pageNo;
    BuildRegistrationLogTable();
}

function ChangeOrderBy(colNumber) {
    //set the column number we are ordering by
    OrderBy = colNumber;
    if (OrderDirection == OD_ASC) {
        OrderDirection = OD_DESC;
    } else {
        OrderDirection = OD_ASC;
    }
    CurrentPage = 1;
    BuildRegistrationLogTable();
}

function SetOrderSignForCurrentView(orderBy) {
    var sign = OrderDirection == OD_ASC ? "&#9650;" : "&#9660;";
	var spnCurrentOrder = document.getElementById("col" + orderBy);
	spnCurrentOrder.innerHTML = sign;
}


function BackButtonClicked() {
    document.location.href = "devicelist.html?setState";
}