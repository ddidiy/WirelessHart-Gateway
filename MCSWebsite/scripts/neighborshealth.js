// page parameter
var deviceId = null;
var Neighbors = null;
var StartTime = null;
var EndTime = null;

//order direction
var OD_ASC = "ASC";
var OD_DESC = "DESC";
var OrderBy = 1;  //order column number
var OrderDirection = OD_ASC;

var TotalNoOfRows = 0;
var CurrentPage = 1;
var TotalPages = 0;
var PageSize = 10;


function InitNeighborsHealthPage() {
    SetPageCommonElements();
    InitJSON();
    deviceId = GetPageParamValue("deviceId");
    if (!isNaN(deviceId)) {  //make sure qs was not altered (is a number)
        dev = GetDeviceInformation(deviceId);
        if (dev != null) {
            SetData(dev);                        
            SetDeviceTabs(deviceId, dev.DeviceStatus, dev.DeviceRoleID, 3);     
        }
        ClearFilters();
        PopulateFilters();        
        ReadFilters();
        BuildNeighborsHealthTable();
    }
}

function ClearFilters(){
	document.getElementById("txtNeighbors").value = "";
	document.getElementById("txtStartDate").value = "";
	document.getElementById("txtStartDateHours").value = "";
	document.getElementById("txtStartDateMinutes").value = "";
	document.getElementById("ddlStartDateAMPM").value = "AM";
	document.getElementById("txtEndDate").value = "";
	document.getElementById("txtEndDateHours").value = "";
	document.getElementById("txtEndDateMinutes").value = "";
	document.getElementById("ddlEndDateAMPM").value = "AM";
}

function BuildNeighborsHealthTable() {
    TotalNoOfRows = GetNeighborsHealthCount(deviceId, Neighbors, StartTime, EndTime);
    if (TotalNoOfRows > 0) {
        var data = GetNeighborsHealth(deviceId, PageSize, CurrentPage, TotalNoOfRows, OrderBy, OrderDirection, Neighbors, StartTime, EndTime);        
        document.getElementById("tblNeighborsHealth").innerHTML = TrimPath.processDOMTemplate("neighborshealth_jst", data);
        SetOrderSignForCurrentView(OrderBy);
    } else {
    	document.getElementById("spnPageNumber").innerHTML = "";
        document.getElementById("tblNeighborsHealth").innerHTML = 
        		"<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"100%\" border=\"0\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
     				  "<tr><td class=\"tableSubHeader\" style=\"width:150px;\" align=\"center\">Neighbor</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:150px;\" align=\"center\">Timestamp</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:100px;\" align=\"center\">Flags</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:140px;\" align=\"center\">Transmitted</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:140px;\" align=\"center\">Failed</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:120px;\" align=\"center\">Received</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:100px;\" align=\"center\">Signal Level</td></tr>" +
                        "<tr><td colspan=\"7\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
    }
    SetPager();
}

function PageNavigate(pageNo) {
	PageSize = document.getElementById("ddlRowsPerPage").value;	
    CurrentPage = pageNo;   
    BuildNeighborsHealthTable();
}

function ChangeOrderBy(colNumber) {
    OrderBy = colNumber;    
    if (OrderDirection == OD_ASC) {
        OrderDirection = OD_DESC;
    } else {
        OrderDirection = OD_ASC;
    }    
    CurrentPage = 1;
    BuildNeighborsHealthTable();
}

function SetData(dev) {
    document.getElementById("spnEUI64").innerHTML = dev.Address64;
    document.getElementById("spnNickname").innerHTML = dev.Nickname;
}

function SetOrderSignForCurrentView(orderBy) {
    var sign = OrderDirection == OD_ASC ? "&#9650;" : "&#9660;";
	var spnCurrentOrder = document.getElementById("col" + orderBy);
	spnCurrentOrder.innerHTML = sign;
}

function BackButtonClicked() {
    document.location.href = "devicelist.html?setState";
}

function PopulateFilters() {
	var txtNeighbors = document.getElementById("txtNeighbors");
	txtNeighbors.value = "";
	var data = GetNeighborList(deviceId);	
	var dateNow = GetUTCDate()
	dateNow.setHours(dateNow.getHours() - 6 == 0 ? 12 : dateNow.getHours() - 6);
	PopulateDateTime("txtStartDate", "txtStartDateHours", "txtStartDateMinutes", "ddlStartDateAMPM", dateNow);	
}

function ReadFilters() {
    PageSize = document.getElementById("ddlRowsPerPage").value;
	Neighbors 	= document.getElementById("txtNeighbors").value;
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
    return true;    
}

function Search() {
	if (ReadFilters()) {
        CurrentPage = 1;
        BuildNeighborsHealthTable();	        
	};
}
