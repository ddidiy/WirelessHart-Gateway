var methods = ["sqldal.execute", "user.logout", "sqldal.open", "sqldal.close", "user.isValidSession"];
var CurrentPage = 1;
var TotalPages = 0;
var TotalNoOfRows = 0;


//filters
var deviceTag = null;
var eui64Address = null;
var alertType = null;
var StartTime = null;
var EndTime = null;
var PageSize = 10;

function InitAlertsLogPage() {
    SetPageCommonElements();   
    InitJSON();    
    ClearFilters();
    PopulateFilters();
    ReadFilters();        
    BuildAlertsTable();
}

function ClearFilters(){
	document.getElementById("txtDeviceTag").value = "";
	document.getElementById("txtEUI64Address").value = "";
	document.getElementById("ddlAlertType").selectedIndex = 0;
	
	document.getElementById("txtStartDate").value = "";
	document.getElementById("txtStartDateHours").value = "";
	document.getElementById("txtStartDateMinutes").value = "";
	document.getElementById("ddlStartDateAMPM").value = "AM";
	document.getElementById("txtEndDate").value = "";
	document.getElementById("txtEndDateHours").value = "";
	document.getElementById("txtEndDateMinutes").value = "";
	document.getElementById("ddlEndDateAMPM").value = "AM";
}

function BuildAlertsTable() {    	
    TotalNoOfRows = GetAlertsCount(deviceTag, eui64Address, alertType, StartTime, EndTime);
    if (TotalNoOfRows > 0) {
        var data = GetAlertsPage(deviceTag, eui64Address, alertType, StartTime, EndTime, CurrentPage, PageSize, TotalNoOfRows, false)
        if (data != null) {                    
            document.getElementById("tblAlertsLog").innerHTML = TrimPath.processDOMTemplate("alerts_jst", data);            
            document.getElementById("btnExport").disabled = "";
            document.getElementById("btnExport").style.cursor = "pointer";
            document.getElementById("hQuery").value = GetAlertsPage(deviceTag, eui64Address, alertType, StartTime, EndTime, 1, 5000, TotalNoOfRows, true)
        } else {
        	document.getElementById("spnPageNumber").innerHTML = "";
            document.getElementById("tblAlertsLog").innerHTML =
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"950px\"><tr><td align=\"left\">" +
    			"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
    			  "<tr><td class=\"tableSubHeader\" style=\"width: 250px;\" align=\"center\">Device Tag</td>" +						  
    				  "<td class=\"tableSubHeader\" style=\"width: 100px;\" align=\"left\">Nickname</td>" +
    				  "<td class=\"tableSubHeader\" style=\"width: 150px;\" align=\"left\">EUI-64 Address</td>" +
    				  "<td class=\"tableSubHeader\" style=\"width: 150px;\" align=\"left\">Alert Time</td>" +						  						  					  
    				  "<td class=\"tableSubHeader\" style=\"width: 150px;\" align=\"left\">Alert Type</td>" +
    				  "<td class=\"tableSubHeader\" style=\"width: 120px;\" align=\"center\">PeerAddress/GraphID</td>" +
    				  "<td class=\"tableSubHeader\" style=\"width: 30px;\" align=\"center\">MIC</td>" +						  
                    "<tr><td colspan=\"7\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
            "</td></tr></table>";
            document.getElementById("hQuery").value = "";
            document.getElementById("btnExport").disabled = "disabled";
            document.getElementById("btnExport").style.cursor = "default";
        }
    } else {
        document.getElementById("tblAlertsLog").innerHTML =
            "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"950px\"><tr><td align=\"left\">" +
			"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
			  "<tr><td class=\"tableSubHeader\" style=\"width: 250px;\" align=\"center\">Device Tag</td>" +						  
				  "<td class=\"tableSubHeader\" style=\"width: 100px;\" align=\"left\">Nickname</td>" +
				  "<td class=\"tableSubHeader\" style=\"width: 150px;\" align=\"left\">EUI-64 Address</td>" +
				  "<td class=\"tableSubHeader\" style=\"width: 150px;\" align=\"left\">Alert Time</td>" +						  						  					  
				  "<td class=\"tableSubHeader\" style=\"width: 150px;\" align=\"left\">Alert Type</td>" +
				  "<td class=\"tableSubHeader\" style=\"width: 120px;\" align=\"center\">PeerAddress/GraphID</td>" +
				  "<td class=\"tableSubHeader\" style=\"width: 30px;\" align=\"center\">MIC</td>" +						  
                "<tr><td colspan=\"7\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
        "</td></tr></table>";
        document.getElementById("hQuery").value = "";
        document.getElementById("btnExport").disabled = "disabled";
        document.getElementById("btnExport").style.cursor = "default";
    }    
    SetPager();    
}

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

function PageNavigate(pageNo) {
    CurrentPage = pageNo;   
    BuildAlertsTable();
}

function PopulateFilters() {
   //alert types
   var ddlAlertType = document.getElementById("ddlAlertType");
   ddlAlertType.options[0] = new Option("All","");    
   var alarmTypeArr = GetAlertTypesArray();
   for (i = 0; i < alarmTypeArr.length; i++) {
	   ddlAlertType.options[i+1] = new Option(GetAlertTypeName(alarmTypeArr[i]),alarmTypeArr[i]);
   }

   //set start date = now - 24 hour
   var dateNow = GetUTCDate()
   dateNow.setHours(dateNow.getHours() - 24);
   PopulateDateTime("txtStartDate", "txtStartDateHours", "txtStartDateMinutes", "ddlStartDateAMPM", dateNow);
}

function ReadFilters() {
	PageSize = document.getElementById("ddlRowsPerPage").value;
	deviceTag	= document.getElementById("txtDeviceTag").value;   
    eui64Address = document.getElementById("txtEUI64Address").value;   
	alertType	= document.getElementById("ddlAlertType").value;            

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
	if (ReadFilters()){
		CurrentPage = 1;
		BuildAlertsTable();		
	};
}