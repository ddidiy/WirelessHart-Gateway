var channelNo = null;
var deviceId  = null;
var NoOfDevices  = 0;
var NoOfChannels = 0;
var NoOfSlots    = 0;

function ReadParameters(){
    var url = parent.document.URL;
	channelNo = Number(url.substring(url.indexOf('?channelNo=') + 11, url.indexOf('?deviceId=')));
    deviceId  = Number(url.substring(url.indexOf('?deviceId=') + 10, url.indexOf('?callerId=')));
    callerId  = Number(url.substring(url.indexOf('?callerId=') + 10, url.length));
}

function InitAddDeviceToDashboardPage() {
    SetPageCommonElements();    
    InitJSON();    
    ReadParameters();
    var ddlDevices = document.getElementById("ddlDevices");
    var ddlChannels = document.getElementById("ddlChannels");
      
    if (channelNo){
    	PopulateDevices();
    	for(var i=0; i<NoOfDevices; i++){
    		if (ddlDevices[i].value == deviceId){
    			ddlDevices.selectedIndex = i;
    			PopulateChannels(deviceId);
    			for (var j=0; j<NoOfChannels; j++){
    				if (ddlChannels[j].value == channelNo) {
    					ddlChannels.selectedIndex = j;	
    				}    				
    			}
    			PopulateNoOfSlots();
    			break;
    		} 
    	}
    } else {
    	 PopulateFilters();
    }
}

function Cancel() {
	if (callerId == 1){
		document.location.href = "deviceinformation.html?deviceId="+deviceId;	
	} else {
		document.location.href = "dashboard.html";
	}		   
}

function PopulateFilters(){
	PopulateDevices();
	if (NoOfDevices !=0){
		var ddlDevices  = document.getElementById("ddlDevices");
		var deviceId = ddlDevices[ddlDevices.selectedIndex].value; 
		PopulateChannels(deviceId);
		PopulateNoOfSlots();		
	}
}

function PopulateDevices(){
    var ddlDevices  = document.getElementById("ddlDevices");   
    var ddlChannels = document.getElementById("ddlChannels");
    ClearList("ddlDevices");    
    ClearList("ddlChannels");

    var data=GetDeviceWithChannels()
    for (var i=0; i<data.length; i++){
    	ddlDevices.options[i] = new Option(data[i].Address64, data[i].DeviceID);
    }
    NoOfDevices = data.length;
    if (NoOfDevices==0){
    	ddlDevices.options[0] = new Option("<none>", "");
    	ddlChannels.options[0] = new Option("<none>", "");
    }
}

function PopulateChannels(deviceId) {    
    var ddlChannels = document.getElementById("ddlChannels");    
    ClearList("ddlChannels");
    if (NoOfDevices != 0){
        var data=GetChannelsForDevice(deviceId)
        for (var i=0; i<data.length; i++){
        	ddlChannels.options[i] = new Option(data[i].ChannelName, data[i].ChannelNo);
        }    
        NoOfChannels = data.length;    	
    } else {
    	ddlChannels.options[0] = new Option("<none>", "");
    }
}

function DeviceChanged(){
	var ddlDevices  = document.getElementById("ddlDevices");
	if (ddlDevices.selectedIndex != -1){
		var deviceId = ddlDevices[ddlDevices.selectedIndex].value; 
		PopulateChannels(deviceId);			
	}
}

function PopulateNoOfSlots(){
	var ddlSlotNumber = document.getElementById("ddlSlotNumber");
	ClearList("ddlSlotNumber");
	var data = GetNoOfDevicesInDashboard();
	for(var i=1; i<=data.Count+1; i++){
		ddlSlotNumber.options[i-1] = new Option(i, i);
	}
	NoOfSlots = data.Count;
	ddlSlotNumber.selectedIndex = NoOfSlots; 
}

function AddDeviceToDashboard(){
	var deviceId 	= document.getElementById("ddlDevices").value;
	var channelNo 	= document.getElementById("ddlChannels").value;
	var slotNumber 	= document.getElementById("ddlSlotNumber").value;
	var gaugeTypeArr = document.getElementsByName("rbGaugeType");	
	var gaugeType = null;
	
	var devices = GetNoOfDevicesInDashboard();
	if (devices.Count == 9){
		alert("You can add only 9 device readings in dashboard !");
		return;		
	}
	
	if (deviceId == "" || channelNo == ""){
		alert("Invalid device/channel !")
		return;
	}	
	
	if (gaugeTypeArr[0].checked){
		gaugeType = 1;
	};
	if (gaugeTypeArr[1].checked){
		gaugeType = 2;
	};	

	var minValue = document.getElementById("txtMinValue");
	var maxValue = document.getElementById("txtMaxValue") ;  
	if (!ValidateRequired(minValue,"Min value")){
		return;
	};	
	if (isNaN(minValue.value)){
		alert("Invalid value for Min value!")
		minValue.focus();
		return;
	};
	if(!ValidateRequired(maxValue,"Max value")){
		return;
	};		
	if (isNaN(maxValue.value)){
		alert("Invalid value for Max value!")
		maxValue.focus();
		return;
	};	
	if (Number(minValue.value) >= Number(maxValue.value)){
		alert("Min value must be less then Max value!")		
		return;
	}
		
	AddDeviceInDashboard(slotNumber, deviceId, channelNo, gaugeType, minValue.value, maxValue.value)
	PopulateNoOfSlots();

	if (callerId == 1){
		document.location.href = "deviceinformation.html?deviceId="+deviceId;	
	} else {
		document.location.href = "dashboard.html";
	}		   
}
