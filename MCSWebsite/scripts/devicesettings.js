var deviceId = null;

function InitDeviceSettingsPage() {
    SetPageCommonElements();
    InitJSON();
    deviceId = GetPageParamValue("deviceId"); 
    if (!isNaN(deviceId)) { //make sure qs was not altered (is a number)
        var dev = GetDeviceInformation(deviceId);
        if (dev != null) {            
            SetData(dev);            
            SetDeviceTabs(deviceId, dev.DeviceStatus, dev.DeviceRoleID, 1)
            BuildServicesTable();
            BuildNeighborsTable();            
            BuildRoutesTable();
        }
    }
}

function SetData(dev) {
    document.getElementById("spnEUI64").innerHTML = dev.Address64;
    document.getElementById("spnNickname").innerHTML = dev.Nickname;
}

function BuildServicesTable(){
    //obtain data from database
    var data = GetDeviceServices(deviceId);
    //process template 
    document.getElementById("tblServices").innerHTML = TrimPath.processDOMTemplate("services_jst", data);
}

function BuildNeighborsTable() {
    //obtain data from database
    var data = GetDeviceNeighbors(deviceId);
    //process template 
    document.getElementById("tblNeighbors").innerHTML = TrimPath.processDOMTemplate("neighbors_jst", data);
}

function BuildRoutesTable() {
    //obtain data from database
    var data = GetDeviceRoutes(deviceId);    
    //process template   
    document.getElementById("tblRoutes").innerHTML = TrimPath.processDOMTemplate("routes_jst", data);
}

function BackButtonClicked() {
    document.location.href = "devicelist.html?setState";
}
