var firmwareId = null;

function InitFirmwareDetailsPage() {
    SetPageCommonElements();
    
    InitJSON();
    
    var url = parent.document.URL;
    firmwareId = Number(url.substring(url.indexOf('?') + 12, url.length));
    
    var spnAddOrEdit = document.getElementById("spnAddOrEdit");
    var spnTypeEdit = document.getElementById("spnTypeEdit");
    var spnVersionEdit = document.getElementById("spnVersionEdit");
    
    if (isNaN(firmwareId)) { //add mode
        PopulateFirmwareTypeList();
        spnAddOrEdit.innerHTML = "Add";
        document.getElementById("spnUploadControl").style.display = "";
        spnTypeEdit.style.display = "";
        spnVersionEdit.style.display = "";
    } else {   //edit mode
        spnAddOrEdit.innerHTML = "Edit";
        document.getElementById("spnDetails").style.display = "";
        var fw = GetFirmwareDetails(firmwareId);
        if (fw != null) {
            var spnFileName = document.getElementById("spnFileName");
            var spnVersion = document.getElementById("spnVersion");
            var spnFirmwareType = document.getElementById("spnFirmwareType");
            var txtDescription = document.getElementById("txtDescription");
            
            spnFileName.innerHTML = fw.FileName;
            spnVersion.innerHTML = fw.Version;
            spnFirmwareType.innerHTML = fw.FirmwareTypeName;
            txtDescription.value = fw.Description;
            
        }
    }
}


function ValidateInput() {
    if (!ValidateRequired(document.getElementById("figure2"), "Firmware file")) {
        return false;
    }
    if(!ValidateRequired(document.getElementById("txtVersion"), "Version")) {
        return false;
    }
    var fileName = getFileName(document.getElementById("figure2").value);
    try {
        var cntFwFile = GetFirmwareFilename(fileName);
        if (cntFwFile > 0) {
            alert("The firmware file '" + fileName + "' already uploaded!");
            return false;
        }
    } catch (e) {
        alert("Error on checking the firmware file!");
        return false;
    }
    return true;
}


function Save() {
    if (isNaN(firmwareId)) { //add mode
        if (ValidateInput()) {
            //#1 upload file    
            try {    
                var service = new jsonrpc.ServiceProxy(serviceURL, methods);
                service.user.isValidSession();            
                var fileName = getFileName(document.getElementById("figure2").value); 
                document.form1.call1.value = jsonrpc.JSONRPCMethod("a").jsonRequest(1,"misc.fileUpload", {script: "/access_node/firmware/web_device_fw_upload.sh", scriptParams: fileName});
                return 1;
            } catch (e) {
                HandleException(e, 'Unexpected error downloading file !')
            };        
            //#2 execute db insert - when upload completes (operationDoneListener)
        }
    } else {   //edit mode
        var txtDescription = document.getElementById("txtDescription");
        UpdateFirmware(firmwareId, txtDescription.value);
        document.location.href = "fwfilelist.html";
    }
}


function operationDoneListener(text) {
    if (text) {
		if (text.result) {
			 SaveNewFirmwareToDatabase();
		} else if (text.error) {
			alert("Error uploading firmware file !");
		}
	}
}

function SaveNewFirmwareToDatabase() {
    var firmware = new Firmware();
                
    firmware.FileName = getFileName(document.getElementById("figure2").value); 
    firmware.Version = document.getElementById("txtVersion").value;
    firmware.Description = document.getElementById("txtDescription").value;
    firmware.UploadDate = GetUTCDate();
    firmware.FirmwareType = document.getElementById("ddlFirmwareTypes").value;    
    AddFirmware(firmware);
    document.location.href = "app/fwfilelist.html"; //[HACK] - to investige why app/ is needed here
}


function Cancel() {
    document.location.href = "fwfilelist.html";
}


function PopulateFirmwareTypeList() {

    var ddlFirmwareTpes = document.getElementById("ddlFirmwareTypes");    
    ClearList("ddlFirmwareTypes");

    //ddlFirmwareTpes.options[0] = new Option(GetFirmwareTypeName(FT_AquisitionBoard), FT_AquisitionBoard);
    ddlFirmwareTpes.options[0] = new Option(GetFirmwareTypeName(FT_BackboneRouter), FT_BackboneRouter);
    ddlFirmwareTpes.options[1] = new Option(GetFirmwareTypeName(FT_Device), FT_Device);
    ddlFirmwareTpes.options[2] = new Option(GetFirmwareTypeName(FT_DeviceNonRouting), FT_DeviceNonRouting);
}
