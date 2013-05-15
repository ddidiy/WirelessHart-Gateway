function InitCustomIconsAddPage() {
    SetPageCommonElements();
    InitJSON();    
    document.getElementById("btnAdd").disabled = false;
    if (PopulateModels()==0){
        DisplayOperationResult("spnOperationResultAdd", "There is no Device Model (Role) without assigned icon.");
        document.getElementById("btnAdd").disabled = true;
    }
	document.getElementById("spnMaxIconSize").innerHTML = "Maximum icon size: " + MAX_DEVICE_ICON_SIZE + "x" + MAX_DEVICE_ICON_SIZE + " pixels.";
    document.getElementById("spnMaxIconFileSize").innerHTML = "Maximum icon file size: " + MAX_DEVICE_ICON_FILE_SIZE + " Kb.";
    document.getElementById("spnImageFileType").innerHTML = "You have to upload only image files (ex: .jpg .png etc)";
}

function Add() {
    var ddlModelRole = document.getElementById("ddlModelRole");
        var selectedModel = ddlModelRole[ddlModelRole.selectedIndex].value;
    if (selectedModel == null || selectedModel == "") {
        alert("Please select a model!");
        return;
    };

    var fileIcon = document.getElementById("fileIcon").value;    
    if (fileIcon == null || fileIcon == "") {
        alert("Please select an icon file!");
        return;
    };
    var imgToUpload = new Image();
    imgToUpload.src = fileIcon;     
        
    var params = "\"" + getFileName(fileIcon) + "\" \"" + selectedModel + "." + Math.round(Math.random()*10000) + "\"";
    document.form1.call1.value = jsonrpc.JSONRPCMethod("a").jsonRequest(1, "misc.fileUpload", {script: "/access_node/firmware/www/uploadIcon.sh", scriptParams: params});
    return 1 ;
}

function Back() {
    location.href = "customicons.html";
}

// returns the number of items populated in ddlModelRole
function PopulateModels() {
    var ddlModelRole = document.getElementById("ddlModelRole");   
    ClearList("ddlModelRole");
    var modelsData = GetCustomIconsList(false);
    var optionName = "";
    var optionID = null;
    
    if (modelsData != null) {   
        for(i = 0; i < modelsData.length; i++) {
            optionName = trim(modelsData[i].ModelAscii) + ' (' + GetDeviceRole(modelsData[i].DeviceRole) + ')';
            optionID   = modelsData[i].DeviceModel + '.' + modelsData[i].DeviceRole;
            ddlModelRole.options[i] = new Option(optionName, optionID);
        };
        return  modelsData.length;  
    } else {
        return 0;
    }
}

function operationDoneListener(text) {
	var spnOperationResultActivate = document.getElementById("spnOperationResultAdd");
    var noOfItems = PopulateModels();
	if (text.result) {
	    if (noOfItems == 0) {
		    DisplayOperationResult("spnOperationResultAdd", "Add completed successfully. </br> There is no Device Model (Role) without assigned icon.");
		    document.getElementById("btnAdd").disabled = true;
        } else {
            DisplayOperationResult("spnOperationResultAdd", "Add completed successfully.");
        }		
	} else {
        if (text.error) {
            DisplayOperationResult("spnOperationResultAdd", "Add icon failed: "+ text.error);
        };
	};	
}
