var code;
var operation;

function InitDeviceCodeAddPage() {
    SetPageCommonElements();
    InitJSON();
    code = GetPageParamValue("codeToEdit");
    ClearOperationResult("spnOperationResultAdd");
    if (code != null){
    	operation = "edit";
    	var devicecode = GetDeviceCodeDetails(code);    	
    	document.getElementById("winHeader").innerHTML = "Edit Device Code";
        document.getElementById("txtCode").value = code;
    	document.getElementById("txtCode").disabled = true; 
    	document.getElementById("txtModel").value = devicecode[0].Model;
    	document.getElementById("txtCompany").value = devicecode[0].Company;
    } else {    	
    	operation = "add";    	
    	document.getElementById("winHeader").innerHTML = "Add Device Code"
    		
    	document.getElementById("txtCode").disabled = false;
    	document.getElementById("txtModel").value = "";
    	document.getElementById("txtCompany").value = "";
    };    
}

function AddEdit() {
    var txtCode = document.getElementById("txtCode");
    var txtModel = document.getElementById("txtModel");
    var txtCompany = document.getElementById("txtCompany");
   
    if (txtCode.value == "" || txtCode.value == null){
    	alert("Field Code is required!");
    	ClearOperationResult("spnOperationResultAdd");
    	return;
    };    
    if (txtModel.value == "" || txtModel.value == null || txtModel.value.length>100 ) {
    	alert("Field Model is required and must have maximum 100 caracters!")
    	ClearOperationResult("spnOperationResultAdd");
    	return;
    };
    
    if (txtCompany.value == "" || txtCompany.value == null || txtCompany.value.length>100){
    	alert("Field Company is required and must have maximum 100 caracters!!")
    	ClearOperationResult("spnOperationResultAdd");
    	return;
    };
    
    if (operation == "add"){
    	if (!ValidateNumber(txtCode, "Code", 0, 65535)){
        	ClearOperationResult("spnOperationResultAdd");
        	return;
        } 
    	if (AddDeviceCode(txtCode.value, txtModel.value, txtCompany.value) == 1){
    		DisplayOperationResult("spnOperationResultAdd", "Device Code <"+txtCode.value+"> was successfully added!")
    	} 
    } else {
    	if (GetDeviceCodeDetails(txtCode.value) == null){		
    		alert("The device code <"+txtCode.value+"> no longer exists! \n You'll be redirected to Device Codes page!");
    		location.href = "devicecodes.html";
    		return;
    	};	
    	if (UpdateDeviceCode(txtCode.value, txtModel.value, txtCompany.value) == 1){
    		DisplayOperationResult("spnOperationResultAdd", "Device Code <"+txtCode.value+"> was successfully updated!")
    	};	
    };
}

function Back() {
    location.href = "devicecodes.html";
    ClearOperationResult("spnOperationResultAdd");
}
