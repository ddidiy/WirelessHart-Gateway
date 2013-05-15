var OD_ASC = "ASC";
var OD_DESC = "DESC";
var OrderBy = 1;
var OrderDirection = OD_ASC;
var CurrentPage = 1;
var PageSize = 10;
var TotalNoOfRows = 0;
var Code;
var Model;
var Company;

function InitDeviceCodesPage() {
    SetPageCommonElements();    
    InitJSON();
    ResetFilters();
    Search();
}

function BuildDeviceCodesTable() {
	TotalNoOfRows = GetDeviceCodesCount(Code, Model, Company);    
	if (TotalNoOfRows > 0) {		
	    var data = GetDeviceCodesPage(PageSize, CurrentPage, TotalNoOfRows, Code, Model, Company, OrderBy, OrderDirection);
	    if (data != null) {
	        document.getElementById("tblDeviceCodes").innerHTML = TrimPath.processDOMTemplate("devicecodes_jst", data);
	        SetOrderSignForCurrentView(OrderBy);
	    } else {
	        document.getElementById("tblDeviceCodes").innerHTML = //"<span class='labels'>No records !</span>";
			    "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"650px\" align=\"left\"><tr><td align=\"left\">" +
	                "<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
				       "<tr><td class=\"tableSubHeader\" style=\"width: 100px;\" align=\"center\">Code</td>" +
				       "<td class=\"tableSubHeader\" style=\"width: 250px;\" align=\"left\">Model</td>" +
					   "<td class=\"tableSubHeader\" style=\"width: 250px;\" align=\"left\">Company</td>" +
					   "<td class=\"tableSubHeader\" style=\"width: 50px;\">&nbsp;</td></tr>" +
	                "<tr><td class=\"labels\" colspan=\"3\" style=\"text-align:center;\">No records!</td></tr>" +
				"</table></td></tr></table>";
	    }
	} else {
        document.getElementById("tblDeviceCodes").innerHTML = //"<span class='labels'>No records !</span>";
		    "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"650px\" align=\"left\"><tr><td align=\"left\">" +
                "<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
			       "<tr><td class=\"tableSubHeader\" style=\"width: 100px;\" align=\"center\">Code</td>" +
			       "<td class=\"tableSubHeader\" style=\"width: 250px;\" align=\"left\">Model</td>" +
				   "<td class=\"tableSubHeader\" style=\"width: 250px;\" align=\"left\">Company</td>" +
				   "<td class=\"tableSubHeader\" style=\"width: 50px;\">&nbsp;</td></tr>" +
                "<tr><td class=\"labels\" colspan=\"3\" style=\"text-align:center;\">No records!</td></tr>" +
			"</table></td></tr></table>";		
	};
	SetPager();
}


function ChangeOrderBy(colNumber) {
    OrderBy = colNumber;
    if (OrderDirection == OD_ASC) {
        OrderDirection = OD_DESC;
    } else {
        OrderDirection = OD_ASC;
    }
    CurrentPage = 1;
    BuildDeviceCodesTable();
}

function SetOrderSignForCurrentView(orderBy) {
    var sign = OrderDirection == OD_ASC ? "&#9650;" : "&#9660;";
	var spnCurrentOrder = document.getElementById("col" + orderBy);
	spnCurrentOrder.innerHTML = sign;
}

function AddDeviceCode(){
	 location.href = "devicecodeadd.html";
}

function RemoveDeviceCode(deviceCode){
	if (GetDeviceCodeDetails(deviceCode) == null){	
		alert("The device code <"+deviceCode+"> no longer exists! \n The page will be refreshed!");
		BuildDeviceCodesTable();
		return;
	}; 	
	if (DeleteDeviceCode(deviceCode) != 0) {
		BuildDeviceCodesTable();	
	};
}

function EditDeviceCode(deviceCode){
	if (GetDeviceCodeDetails(deviceCode) == null){		
		alert("The device code <"+deviceCode+"> no longer exists! \n The page will be refreshed!")
		BuildDeviceCodesTable();
	} else {
		location.href = "devicecodeadd.html?codeToEdit="+deviceCode;
	};
}

function Search() {
    ReadFilters();
    CurrentPage = 1;
    BuildDeviceCodesTable();
}

function ReadFilters() {
	Code = document.getElementById("txtCode").value;
	Model = document.getElementById("txtModel").value;
	Company = document.getElementById("txtCompany").value;
	PageSize = document.getElementById("ddlRowsPerPage").value;	
}

function ResetFilters(){
	document.getElementById("txtCode").value = "";
	document.getElementById("txtModel").value = "";
	document.getElementById("txtCompany").value = "";
	document.getElementById("ddlRowsPerPage").value = 10;
}

function PageNavigate(pageNo) {
    CurrentPage = pageNo;
    BuildDeviceCodesTable();
}