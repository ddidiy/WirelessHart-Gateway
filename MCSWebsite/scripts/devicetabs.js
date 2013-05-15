function SetDeviceTabs(deviceId, deviceStatus, deviceRoleID, activeTabNumber) {
    var tab0 = document.getElementById("tab0");
    var tab1 = document.getElementById("tab1");
    var tab2 = document.getElementById("tab2");
    var tab3 = document.getElementById("tab3");
    var tab4 = document.getElementById("tab4");
    var tab6 = document.getElementById("tab6");

    //Information
    var tab0Visible = (deviceStatus >= DS_NormalOperationCommencing);
    //Settings
    var tab1Visible = (deviceStatus >= DS_NormalOperationCommencing);
    //Registration Log
    var tab2Visible = (deviceStatus >= DS_NormalOperationCommencing);    
    //Neighbors Health
    var tab3Visible = (deviceStatus >= DS_NormalOperationCommencing && deviceRoleID != DT_NetworkManager && deviceRoleID != DT_Gateway);
    //Schedule Report
    var tab4Visible = (deviceStatus >= DS_NormalOperationCommencing && deviceRoleID != DT_NetworkManager && deviceRoleID != DT_Gateway);
    //Run Commands
    var tab6Visible = (deviceStatus >= DS_NormalOperationCommencing && deviceRoleID != DT_NetworkManager);
    
    if (activeTabNumber == 0) {
        tab0.className = "selectedTabButton";
        tab0.innerHTML = "Information";
    } else if (tab0Visible){
        tab0.className = "tabButton";
        tab0.innerHTML = "<a href='deviceinformation.html?deviceId=" + deviceId + "' class='tabLink'>Information</a>";
        tab0.disabled = false;
    } else {
    	tab0.className = "tabButton";    
    	tab0.innerHTML = "Information";
    	tab0.disabled = true;    	
    }        
        
    if (activeTabNumber == 1) {
        tab1.className = "selectedTabButton";
        tab1.innerHTML = "Settings";
    } else if (tab1Visible) {
        tab1.className = "tabButton";
        tab1.innerHTML = "<a href='devicesettings.html?deviceId=" + deviceId + "' class='tabLink'>Settings</a>";
        tab1.disabled = false;
    } else {
    	tab1.className = "tabButton";    
    	tab1.innerHTML = "Settings";
    	tab1.disabled = true;    	    	
    }    
    
    if (activeTabNumber == 2) {
        tab2.className = "selectedTabButton";
        tab2.innerHTML = "Registration Log";
    } else if (tab2Visible) {
        tab2.className = "tabButton";
        tab2.innerHTML = "<a href='registrationlog.html?deviceId=" + deviceId + "' class='tabLink'>Registration Log</a>";
        tab2.disabled = false;
    } else {
    	tab2.className = "tabButton";    
    	tab2.innerHTML = "Registration Log";
    	tab2.disabled = true;    	    	    	
    }
    
    if (activeTabNumber == 3) {
        tab3.className = "selectedTabButton";
        tab3.innerHTML = "Neighbors Health";
    } else if (tab3Visible) {
        tab3.className = "tabButton";
        tab3.innerHTML = "<a href='neighborshealth.html?deviceId=" + deviceId + "' class='tabLink'>Neighbors Health</a>";
        tab3.disabled = false;
    } else {
    	tab3.className = "tabButton";    	
    	tab3.innerHTML = "Neighbors Health";
    	tab3.disabled = true;
    }

    if (activeTabNumber == 4) {
        tab4.className = "selectedTabButton";
        tab4.innerHTML = "Schedule Report";
    } else if (tab4Visible) {
        tab4.className = "tabButton";
        tab4.innerHTML = "<a href='schedulereport.html?deviceId=" + deviceId + "' class='tabLink'>Schedule Report</a>";
        tab4.disabled = false;        
    } else {
    	tab4.className = "tabButton";    
    	tab4.innerHTML = "Schedule Report";
    	tab4.disabled = true;    	    	    	    	
    }
       
    if (activeTabNumber == 6) {
        tab6.className = "selectedTabButton";
        tab6.innerHTML = "Run Commands";
    } else if (tab6Visible) {
        tab6.className = "tabButton";
        tab6.innerHTML = "<a href='devicecommands.html?deviceId=" + deviceId + "' class='tabLink'>Run Commands</a>";
        tab6.disabled = false;
    } else {
    	tab6.className = "tabButton";
    	tab6.innerHTML = "Run Commands";
    	tab6.disabled = true;    	    	
    }          
}
