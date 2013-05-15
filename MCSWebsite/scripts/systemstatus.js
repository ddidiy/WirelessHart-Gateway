var methods = ["isa100.getSystemStatus", "user.logout"];


function InitSystemStatusPage() {
    SetPageCommonElements()
    InitJSON();
    GetData();
    start(); //start counter
}


function GetData() {
    var response;
    try {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        response = service.isa100.getSystemStatus();
    } catch(e) {
        HandleException(e, "Unexpected error reading System Status !");
        return;
    }

    var spnBackboneStatus = document.getElementById("spnBackboneStatus");
    var spnGatewayStatus = document.getElementById("spnGatewayStatus");
    var spnNetworkManagerStatus = document.getElementById("spnNetworkManagerStatus");
	var spnMODBUSStatus = document.getElementById("spnMODBUSStatus");
    var spnMonitorHostStatus = document.getElementById("spnMonitorHostStatus");
	
    for (var i=0; i<response.processes.length; i++) {
        if(response.processes[i].name == "AccessPoint") {
            if(response.processes[i].status != "Running") {
                spnBackboneStatus.className = "opResultRed";
            } else {
                spnBackboneStatus.className = "opResultGreen";    
            }
            spnBackboneStatus.innerHTML = response.processes[i].status;
            document.getElementById("spnBackboneMemory").innerHTML = roundNumber(response.processes[i].memKb / 1024, 2) + " MB (" + (100 * response.processes[i].memKb / response.sysMemTotalKb).toFixed(2) + "%)";
            document.getElementById("spnBackboneProcessor").innerHTML = response.processes[i].processorUsage + " %";
        }
        if(response.processes[i].name == "Gateway") {
           if(response.processes[i].status != "Running") {
               spnGatewayStatus.className = "opResultRed";
           } else {
               spnGatewayStatus.className = "opResultGreen";
           }
           spnGatewayStatus.innerHTML = response.processes[i].status;
           document.getElementById("spnGatewayMemory").innerHTML = roundNumber(response.processes[i].memKb / 1024, 2) + " MB (" + (100 * response.processes[i].memKb / response.sysMemTotalKb).toFixed(2) + "%)";
           document.getElementById("spnGatewayProcessor").innerHTML = response.processes[i].processorUsage + " %";
        }
        if(response.processes[i].name == "NetworkManager") {
           if(response.processes[i].status != "Running") {
               spnNetworkManagerStatus.className = "opResultRed";
           } else {
               spnNetworkManagerStatus.className = "opResultGreen";
           }
           spnNetworkManagerStatus.innerHTML = response.processes[i].status;
           document.getElementById("spnNetworkManagerMemory").innerHTML = roundNumber(response.processes[i].memKb / 1024, 2) + " MB (" + (100 * response.processes[i].memKb / response.sysMemTotalKb).toFixed(2) + "%)";
           document.getElementById("spnNetworkManagerProcessor").innerHTML = response.processes[i].processorUsage + " %";
        }
        if(response.processes[i].name == "MODBUS") {
           if(response.processes[i].status != "Running") {
               spnMODBUSStatus.className = "opResultRed";
           } else {
               spnMODBUSStatus.className = "opResultGreen";
           }
           spnMODBUSStatus.innerHTML = response.processes[i].status;
           document.getElementById("spnMODBUSMemory").innerHTML = roundNumber(response.processes[i].memKb / 1024, 2) + " MB (" + (100 * response.processes[i].memKb / response.sysMemTotalKb).toFixed(2) + "%)";
           document.getElementById("spnMODBUSProcessor").innerHTML = response.processes[i].processorUsage + " %";
        }
        if(response.processes[i].name == "MonitorHost") {
           if(response.processes[i].status != "Running") {
               spnMonitorHostStatus.className = "opResultRed";
           } else {
               spnMonitorHostStatus.className = "opResultGreen";
           }
           spnMonitorHostStatus.innerHTML = response.processes[i].status;
           document.getElementById("spnMonitorHostMemory").innerHTML = roundNumber(response.processes[i].memKb / 1024, 2) + " MB (" + (100 * response.processes[i].memKb / response.sysMemTotalKb).toFixed(2) + "%)";
           document.getElementById("spnMonitorHostProcessor").innerHTML = response.processes[i].processorUsage + " %";
        }
     }
	 
     document.getElementById("spnSystemMemoryTotal").innerHTML = roundNumber(response.sysMemTotalKb / 1024, 2) + " MB";
     document.getElementById("spnSystemMemoryUsed").innerHTML = roundNumber((response.sysMemTotalKb - response.sysMemFreeKb) / 1024, 2) + " MB (" + (100 * (response.sysMemTotalKb - response.sysMemFreeKb) / response.sysMemTotalKb).toFixed(2) + "%)";
     document.getElementById("spnSystemMemoryFree").innerHTML = roundNumber(response.sysMemFreeKb / 1024, 2) + " MB (" + (100 * response.sysMemFreeKb / response.sysMemTotalKb).toFixed(2) + "%)";
     document.getElementById("spnFlashMemoryTotal").innerHTML = roundNumber(response.sysFlashTotalKb / 1024, 2)+ " MB";
     document.getElementById("spnFlashMemoryUsed").innerHTML = roundNumber((response.sysFlashTotalKb - response.sysFlashFreeKb) / 1024, 2) + " MB (" + (100 * (response.sysFlashTotalKb - response.sysFlashFreeKb) / response.sysFlashTotalKb).toFixed(2) + "%)";
     document.getElementById("spnFlashMemoryFree").innerHTML = roundNumber(response.sysFlashFreeKb / 1024, 2) + " MB (" + (100 * response.sysFlashFreeKb / response.sysFlashTotalKb).toFixed(2) + "%)";
     document.getElementById("spnLoadAverage").innerHTML = response.load;
}
