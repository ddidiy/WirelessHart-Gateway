function InitCustomIconsPage() {
    SetPageCommonElements();    
    InitJSON();
    BuildCustomIconsTable();
}


function BuildCustomIconsTable() {
    var data = GetCustomIconsList(true);
    if (data != null) {
        document.getElementById("tblCustomIcons").innerHTML = TrimPath.processDOMTemplate("customicons_jst", data);
    } else {
        document.getElementById("tblCustomIcons").innerHTML = //"<span class='labels'>No records !</span>";
        		    "<table cellpadding=\"0\" cellspacing=\"0\" class=\"containerDiv\" width=\"550px\" align=\"left\"><tr><td align=\"left\">" +
                        "<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\">" +
					       "<tr><td class=\"tableHeader\" style=\"width: 300px;\" align=\"left\">Model</td>" +
					       "<td class=\"tableHeader\" style=\"width: 150px;\" align=\"left\">Role</td>" +
    					   "<td class=\"tableHeader\" style=\"width: 50px;\" align=\"left\">Icon</td>" +
						   "<td class=\"tableHeader\" style=\"width: 50px;\">&nbsp;</td></tr>" +
                        "<tr><td class=\"labels\" colspan=\"3\" style=\"text-align:center;\">No records!</td></tr>" +
					"</table></td></tr></table>";


    }
}


function AddIcon(deviceId) {
    if (!confirm("Are you sure you want to delete the device and all the related data ?")) {
        return;
    }
    DeleteCustomIcon(deviceId);
    BuildCustomIconsTable();
}


function DeleteIconFile(deviceModel) {
    try {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        var response = service.file.remove({file: deviceModel});
        if(response) {
            location.href = "customicons.html";
        } else {
            alert ("Error deleting the selected icon!");
        }
    } catch(e) {
        HandleException(e, "Unexpected error to delete the selected icon!");
        return;
    }
}


function DeleteIcon(deviceModel) {
    if (!confirm("Are you sure you want to delete the selected icon?")) {
        return;
    }
    DeleteIconFile(deviceModel);
    BuildCustomIconsTable();
}

function AddIcon() {
    location.href = "customiconsadd.html"
}
