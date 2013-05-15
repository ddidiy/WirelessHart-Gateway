//order direction
var OD_ASC = "ASC";
var OD_DESC = "DESC";

var CurrentPage = 1;
var PageSize = 15;
var TotalPages = 0;

function InitFirmwareFileListPage() {
    SetPageCommonElements();
    InitJSON();
    
    var url = parent.document.URL;
    qsparam = url.substring(url.indexOf('?') + 1 , url.length);
    if (qsparam == "setState") {
        LoadPageState();
    }
    BuildFirmwareTable();    
}


function BuildFirmwareTable() {
    var rowCount = GetFirmwareCount();
    
    if (rowCount > 0) {
        //obtain data from database
        var data = GetFirmwaresPage(CurrentPage, PageSize, rowCount, false);
        //process template 
        document.getElementById("tblFirmwares").innerHTML = TrimPath.processDOMTemplate("firmwares_jst", data);
    
        SetPager();
        SavePageState();
    } else {
    	document.getElementById("spnPageNumber").innerHTML = "";
        document.getElementById("tblFirmwares").innerHTML = //"<span class='labels'>No records !</span>";
                "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"945px\"><tr><td align=\"left\">" +
    				"<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
     					"<tr><td class=\"tableHeader\" style=\"width:190px;\" align=\"center\">File Name</td>" +
						  "<td class=\"tableHeader\" style=\"width:70px;\" align=\"center\">Version</td>" +
						  "<td class=\"tableHeader\" style=\"width:200px;\" align=\"center\">Firmware Type</td>" +
						  "<td class=\"tableHeader\" style=\"width:150px;\" align=\"center\">Upload Status</td>" +
						  "<td class=\"tableHeader\" style=\"width:190px;\" align=\"center\">Description</td>" +
						  "<td class=\"tableHeader\" style=\"width:100px;\" align=\"center\">Upload Date</td></tr>" +
                        "<tr><td colspan=\"6\" class=\"labels\" style=\"text-align:center;\">No records!</td></tr></table>" +
                "</td></tr></table>";
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
    BuildFirmwareTable();
}


function SavePageState() {
    CreateCookie("FIRMWAREFILELIST_CURRENTPAGE", CurrentPage, 1); 
}


function Refresh() {
    document.location.href = "fwfilelist.html";
}


function Back() {
    document.location.href = "devicefw.html";
}


function NavigateToUpload() {
    document.location.href = "fwdetails.html";
}


function LoadPageState() {
    if (ReadCookie("FIRMWAREFILELIST_CURRENTPAGE") != null) {
        CurrentPage = ReadCookie("FIRMWAREFILELIST_CURRENTPAGE");    
    }
}
   
