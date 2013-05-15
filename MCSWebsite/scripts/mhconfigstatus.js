var CurrentPage = 1;
var TotalPages = 0;
var TotalNoOfRows = 0;

var EUI64Address = null;
var PublisherState = null;
var PageSize = 10;

var OD_ASC = "ASC";
var OD_DESC = "DESC";

var OrderBy = 1;  //order column number
var OrderDirection = OD_ASC;

function InitMHConfigStatusPage() {
    SetPageCommonElements();
    InitJSON();
    ClearFilters();
    BuildTable();
}

function BuildTable(){
	TotalNoOfRows = GetMHPublishersConfigStatusCount(EUI64Address, PublisherState);
    if (TotalNoOfRows > 0) {
        //obtain data from database
        var data = GetMHPublishersConfigStatusPage(EUI64Address, PublisherState, PageSize, CurrentPage, TotalNoOfRows, OrderBy, OrderDirection)
        if (data != null) {
            document.getElementById("tblMHConfigStatus").innerHTML = TrimPath.processDOMTemplate("publishers_jst", data);
            SetOrderSignForCurrentView(OrderBy);
        } else {
            document.getElementById("tblMHConfigStatus").innerHTML =
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"950px\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
     				    "<tr>" +						  
						  "<td class=\"tableSubHeader\" style=\"width: 200px;\" align=\"center\">EUI-64 Address</td>" +
						  "<td class=\"tableSubHeader\" style=\"width: 200px;\" align=\"center\">Publisher State</td>" +
						  "<td class=\"tableSubHeader\" style=\"width: 550px;\" align=\"center\">Error</td>" +						  
		                "</tr>" +
                        "<tr><td colspan=\"3\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
        }
    } else {
    	document.getElementById("spnPageNumber").innerHTML = "";
        document.getElementById("tblMHConfigStatus").innerHTML =
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"950px\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
     				    "<tr>" +						  
						  "<td class=\"tableSubHeader\" style=\"width: 200px;\" align=\"center\">EUI-64 Address</td>" +
						  "<td class=\"tableSubHeader\" style=\"width: 200px;\" align=\"center\">Publisher State</td>" +
						  "<td class=\"tableSubHeader\" style=\"width: 550px;\" align=\"center\">Error</td>" +						  
		                "</tr>" +
                        "<tr><td colspan=\"3\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
    }
    SetPager();	
}

function Search() {
    ReadFilters();
    CurrentPage = 1;
    BuildTable();
}

function ReadFilters() {
	EUI64Address = document.getElementById("txtEUI64Address").value;
	PublisherState = document.getElementById("ddlPublisherState").value;	
    PageSize = document.getElementById("ddlRowsPerPage").value;
}

function PageNavigate(pageNo) {
    CurrentPage = pageNo;   
    BuildTable();
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
    BuildTable();
}

function SetOrderSignForCurrentView(orderBy) {
    var sign = OrderDirection == OD_ASC ? "&#9650;" : "&#9660;";
	var spnCurrentOrder = document.getElementById("col" + orderBy);
	spnCurrentOrder.innerHTML = sign;
}

function Back() {
    document.location.href = "mhconfig.html";
}

function ClearFilters(){
	EUI64Address = "";
	document.getElementById("txtEUI64Address").value = EUI64Address;
	PublisherState = "";
	var ddlPublisherState = document.getElementById("ddlPublisherState")
	ddlPublisherState.value = PublisherState;
	OrderBy = 1;
	OrderDirection = OD_ASC;
	CurrentPage = 1;	
	PageSize = 10;
    document.getElementById("ddlRowsPerPage").value = PageSize;
    var states = GetPublisherStateArray();
    ddlPublisherState.options[0] = new Option("All","");
    for (var i=0; i<states.length; i++){    	
    	ddlPublisherState.options[i+1] = new Option(states[i].TEXT,states[i].ID);
    };    
}