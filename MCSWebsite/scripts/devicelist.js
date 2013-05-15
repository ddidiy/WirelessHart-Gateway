var LastRefreshedString;
var LastRefreshedDate;
var RefreshDeviceInfoActive = false; 

//order direction
var OD_ASC = "ASC";
var OD_DESC = "DESC";

var CurrentPage = 1;
var PageSize = 10;
var TotalNoOfRows = 0;
var ShowDevicesFilter = 0;
var OrderBy = 1;  //order column number
var OrderDirection = OD_ASC;

// filtred fields
var EUI64Address = null;
var DeviceTag = null;
 

function InitDeviceListPage() {
    SetPageCommonElements();
    InitJSON();

    if (IsParameter("setState")) {
        LoadPageState();
    }
    BuildDeviceTable();
}

function ShowDevicesChanged() {
    ShowDevicesFilter = parseInt(document.getElementById("ddlShowDevices").value);
    ddlShowDevices.selectedIndex = ShowDevicesFilter;
    Search();
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
    BuildDeviceTable();
}

function SetOrderSignForCurrentView(orderBy) {
    var sign = OrderDirection == OD_ASC ? "&#9650;" : "&#9660;";
	var spnCurrentOrder = document.getElementById("col" + orderBy);
	spnCurrentOrder.innerHTML = sign;
}

function BuildDeviceTable() {	
    TotalNoOfRows = GetDeviceCount(ShowDevicesFilter, EUI64Address, GetHexString(DeviceTag));    
    if (TotalNoOfRows > 0) {
        // obtain data from database    	
        var data = GetDevicePage(PageSize, CurrentPage, TotalNoOfRows, ShowDevicesFilter, OrderBy, OrderDirection, EUI64Address, GetHexString(DeviceTag));
    	if (data != null) {
            // process template
            document.getElementById("tblDevices").innerHTML = TrimPath.processDOMTemplate("devices_jst", data);
            // display order direction arrow
            SetOrderSignForCurrentView(OrderBy);
            // set paging controls
            SavePageState();
        } else {
            document.getElementById("tblDevices").innerHTML = //"<span class='labels'>No records !</span>";
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"950px\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
     					"<tr><td class=\"tableSubHeader\" style=\"width:50px;\" align=\"center\">&nbsp;</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:150px;\" align=\"center\">EUI-64 Address</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:60px;\" align=\"center\">Nickname</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:150px;\" align=\"center\">Device Tag</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:225px;\" align=\"center\">Device Role/Model</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:75px;\" align=\"center\">Status</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:150px;\" align=\"center\">Last read</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:25px;\" align=\"center\">Commands</td>" +
                        "<tr><td colspan=\"8\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
        }
    } else {
    	document.getElementById("spnPageNumber").innerHTML = "";
        document.getElementById("tblDevices").innerHTML = //"<span class='labels'>No records !</span>";
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"950px\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
     					"<tr><td class=\"tableSubHeader\" style=\"width:50px;\" align=\"center\">&nbsp;</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:150px;\" align=\"center\">EUI-64 Address</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:60px;\" align=\"center\">Nickname</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:150px;\" align=\"center\">Device Tag</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:225px;\" align=\"center\">Device Role/Model</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:75px;\" align=\"center\">Status</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:150px;\" align=\"center\">Last read</td>" +
						  "<td class=\"tableSubHeader\" style=\"width:25px;\" align=\"center\">Commands</td>" +
						  "<td class=\"tableSubHeader\">&nbsp;</td></tr>" +
                        "<tr><td colspan=\"8\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
    }
    SetPager();
}

function DeleteDevice(deviceId) {
    if (!confirm("Are you sure you want to delete the device and all the related data ?")) {
        return;
    }
    DeleteDeviceAndData(deviceId);
    BuildDeviceTable();
}

function PageNavigate(pageNo) {
    CurrentPage = pageNo;
    BuildDeviceTable();
}

function SavePageState() {
    CreateCookie("DEVICELIST_CURRENTPAGE", CurrentPage, 1); 
    CreateCookie("DEVICELIST_SHOWDEVICESFILTER", ShowDevicesFilter, 1);
    CreateCookie("DEVICELIST_ORDERBY", OrderBy, 1); 
    CreateCookie("DEVICELIST_ORDERDIRECTION", OrderDirection, 1); 
}

function LoadPageState() {
    if (ReadCookie("DEVICELIST_CURRENTPAGE") != null) {
        CurrentPage = ReadCookie("DEVICELIST_CURRENTPAGE");    
    }
    if (ReadCookie("DEVICELIST_SHOWDEVICESFILTER") != null) {
    	var ddlShowDevices = document.getElementById("ddlShowDevices");
    	ShowDevicesFilter = ReadCookie("DEVICELIST_SHOWDEVICESFILTER")       
        ddlShowDevices.selectedIndex = ShowDevicesFilter;        
    }
    if (ReadCookie("DEVICELIST_ORDERBY") != null) {
        OrderBy = ReadCookie("DEVICELIST_ORDERBY");
    }
    if (ReadCookie("DEVICELIST_ORDERDIRECTION") != null) {
        OrderDirection = ReadCookie("DEVICELIST_ORDERDIRECTION");
    }
}

function ReadFilters() {
	var txtEUI64Address = document.getElementById("txtEUI64Address");
    txtEUI64Address.value = txtEUI64Address.value.trim()
    EUI64Address = txtEUI64Address.value;
    if (EUI64Address.length == 0) {
        EUI64Address = null;
    }
	var txtDeviceTag = document.getElementById("txtDeviceTag");
    txtDeviceTag.value = txtDeviceTag.value.trim()
    DeviceTag = txtDeviceTag.value;
    if (DeviceTag.length == 0) {
        DeviceTag = null;
    }  
    PageSize = document.getElementById("ddlRowsPerPage").value;
	ShowDevicesFilter = parseInt(document.getElementById("ddlShowDevices").value);
}

function Search() {
    ReadFilters();
    CurrentPage = 1;
    SavePageState();
    BuildDeviceTable();
}

function Reset() {
    EUI64Address = null;
    DeviceTag = null;
    PageSize = 10;
	ShowDevicesFilter = 1;
	
    document.getElementById("txtEUI64Address").value = "";
    document.getElementById("txtDeviceTag").value = "";
    document.getElementById("ddlRowsPerPage").selectedIndex = 0;
    document.getElementById("ddlShowDevices").selectedIndex = 0;	
	Search();
}
