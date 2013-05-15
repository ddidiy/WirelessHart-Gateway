//controls page refresh
var RefreshDeviceInfoActive = false; 
var LastRefreshedString;
var LastRefreshedDate;

// page parameter
var deviceId = null;
// chart view
var chart = null;

function InitChannelsStatisticsPage() {
    SetPageCommonElements();
    InitJSON();

    deviceId = GetPageParamValue("deviceId");
    if (!isNaN(deviceId)) {  //make sure qs was not altered (is a number)
        dev = GetDeviceInformation(deviceId);
        if (dev != null) {
            SetData(dev);
            
            var runTabVisible = (dev.DeviceStatus >= DS_NormalOperationCommencing && dev.DeviceRoleID != DT_Gateway && dev.DeviceRoleID != DT_NetworkManager);
            SetDeviceTabs(deviceId, 5, runTabVisible);    
        }

        BuildChannelsStatisticsChart();

        SetRefreshLabel(deviceId);
        setInterval("DisplaySecondsAgo()", 1000);
    } 
}

function AutoRefresh() {     
    var LastCommand = GetLastCommand(CTC_ChannelsStatistics, CPC_ChannelsStatistics_DeviceID, deviceId);
	if (LastCommand.CommandStatus == CS_New || LastCommand.CommandStatus == CS_Sent) {					
        RefreshDeviceInfoActive = true;					
    } else {
        if (LastCommand.CommandStatus == CS_Failed) {
            SetRefreshLabel(deviceId);
        } else {            
            RefreshDeviceInfoActive = false;
            LastRefreshedDate   = ConvertFromSQLiteDateToJSDateObject(LastCommand.TimeResponded);
            LastRefreshedString = LastCommand.TimeResponded;
        }    
        BuildChannelsStatisticsChart();
	}
	if (RefreshDeviceInfoActive) {		
		setTimeout(AutoRefresh, RefreshInterval);
	}			
}

function RefreshPage() {
    if (AddChannelsStatisticsCommand(deviceId) != null) {
        AutoRefresh();   
    }
}

function DrawGrid(canvas, x, y, w, h, wv, hv, color) {
    color = color || "#000";
    var path = ["M", x, y, "L", x + w, y, x + w, y + h, x, y + h, x, y];
    var rowHeight = h / hv;
    var columnWidth = w / wv;
    for (var i = 1; i < hv; i ++ ) {
        path = path.concat(["M", x, y + i * rowHeight, "L", x + w, y + i * rowHeight]);
    }
    for (var i = 1; i < wv; i ++ ) {
        path = path.concat(["M", x + i * columnWidth, y, "L", x + i * columnWidth, y + h]);
    }
    return canvas.path(path.join(",")).attr({stroke : color});
}

function BuildChannelsStatisticsChart() {
    var deviceChannels = GetChannelsStatistics(deviceId);
    var channelsData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    var oxLabels = [11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26];
    var oyLabels = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100];
    if (deviceChannels) {
        for (var i=0; i<deviceChannels.length/2; i++) {
            channelsData[i] = HexToDec(deviceChannels.substr(2*i, 2));
        }
    }
    
    var widthChart = 800;
    var heightChart = 250;
    var leftGutter = 30;
    var bottomGutter = 20;
    var topGutter = 30;
    var X = (widthChart - leftGutter) / oxLabels.length;
    var Y = (heightChart - bottomGutter - topGutter) / Math.max.apply(Math, oyLabels);
    
    if (chart) {
        chart.remove();
    }
    chart = Raphael("channelsStatistics", widthChart, heightChart);

    chart.g.txtattr.font = "12px 'Fontin Sans', Fontin-Sans, sans-serif";
    chart.g.text(400, 15, "Clear Channel  Assesment");
//    if (!deviceChannels) {
//        for (var i=0; i<16; i++) {
//            channelsData[i] = Math.floor((101)*Math.random());
//        }
//        chart.g.text(400, 15, "Channels Statistics (No data. Displayed chart is only for test)");
//    }

    DrawGrid(chart, leftGutter + X * .5, topGutter, widthChart - leftGutter - X, heightChart - topGutter - bottomGutter, 0, 10, "#333");
    if (Math.max.apply(Math, channelsData) > 0) {
        var fin = function () { 
                    this.flag = chart.g.popup(this.bar.x, this.bar.y, this.bar.value || "0").insertBefore(this); 
                  };
        var fout = function () { 
                    this.flag.animate({opacity: 0}, 300, function () {this.remove();}); 
                  };
        chart.g.barchart(leftGutter + X * .5, topGutter - bottomGutter, widthChart - leftGutter - X, heightChart - topGutter + bottomGutter - 1, [channelsData], {to: 100}).hover(fin, fout);
    }
    
    for (var i = 0; i < oxLabels.length; i++) { 
        var x = Math.round(leftGutter + (X - 4)* i) + 47;
        chart.g.text(x, heightChart - 6, "" + oxLabels[i]); 
    }
    for (var i = 0; i< oyLabels.length; i++) { 
        var y = Math.round(heightChart - bottomGutter - Y * (oyLabels[i] - .8));
        chart.g.text(leftGutter, y, oyLabels[i]+"%"); 
    }
}


function SetData(dev) {
    document.getElementById("spnEUI64").innerHTML = dev.Address64;
    document.getElementById("spnDeviceRole").innerHTML = dev.DeviceRole;
}


function BackButtonClicked() {
    document.location.href = "devicelist.html?setState";
}


function AddChannelsStatisticsCommand(deviceId) {
	var networkManager = GetNetworkManagerDevice();
	if (networkManager == null) {
		return null;
	}
	if (networkManager.DeviceStatus < DS_NormalOperationCommencing) {
	   alert("Network Manager not registered !");
	   return null;
    }   
    var params = Array(1);
    var cmdParam = new CommandParameter();
    cmdParam.ParameterCode = CPC_ChannelsStatistics_DeviceID;
    cmdParam.ParameterValue = deviceId;
    params[0] = cmdParam;
    
	var cmd = new Command();
	cmd.DeviceID = networkManager.DeviceID;
	cmd.CommandTypeCode = CTC_ChannelsStatistics;
	cmd.CommandStatus = CS_New;
	cmd.TimePosted = ConvertFromJSDateToSQLiteDate(GetUTCDate());
    cmd.ParametersDescription = GetParameterName(Number(cmd.CommandTypeCode), Number(cmdParam.ParameterCode)) + ": " + dev.Address64;			

	return AddCommand(cmd, params);
}

function DisplaySecondsAgo() {
    if (RefreshDeviceInfoActive){
        document.getElementById("spnRefreshDate").innerHTML = ' refreshing ...';
    } 
    else {
        document.getElementById("spnRefreshDate").innerHTML = LastRefreshedString +  (LastRefreshedString != NAString ? "  (" + Math.ceil((GetUTCDate().getTime() - LastRefreshedDate.getTime()) / 1000) + " seconds ago)" : ""); 
    };
}

function SetRefreshLabel(deviceId){   
    var LastCommand = GetLastCommandResponded(CTC_ChannelsStatistics, CPC_ChannelsStatistics_DeviceID, deviceId);
    if (LastCommand == null) {	
        RefreshDeviceInfoActive	= false;			
        LastRefreshedString = NAString;
    } else {						        
        LastRefreshedDate   = ConvertFromSQLiteDateToJSDateObject(LastCommand.TimeResponded);
        LastRefreshedString = LastCommand.TimeResponded;
	}    
}
