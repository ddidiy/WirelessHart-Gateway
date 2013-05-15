var methods = ["sqldal.execute", "user.logout", "sqldal.open", "sqldal.close", "user.isValidSession"];

var paper = null;
var DASHBOARD_WIDTH;
var DASHBOARD_HEIGHT;

var GAUGE_WIDTH		  = 150;
var GAUGE_HEIGHT	  = 150;
var LEFT_INFO_WIDTH	  = 150;
var RIGHT_INFO_WIDTH  = 7;
var TOP_INFO_WIDTH	  = 20;
var BOTTOM_INFO_WIDTH = 0;
var DEVICE_WIDTH 	  = LEFT_INFO_WIDTH + GAUGE_WIDTH + RIGHT_INFO_WIDTH; 
var DEVICE_HEIGHT 	  = TOP_INFO_WIDTH + GAUGE_HEIGHT + BOTTOM_INFO_WIDTH;

var GAUGES_XSPACE = 11;
var GAUGES_YSPACE = 11;
var GAUGES_PER_ROW  = 3;
var AUTO_REFRESH_INTERVAL = 10; 

//USED FOR REFRESH
var starttime;
var nowtime;
var secondssinceloaded = 0;
var timer;

var	DEVICES = [{channelNo:"", 
			deviceId:"", 
			slotNumber:"", 
			address:"", 
			manufacturer:"", 
			model:"", 
			channelName:"", 
			lastRead:"", 
			lastValue:"", 
			drawSet:"",
			gaugeType:"", 
			minValue:"",
			maxValue:"",
			deltaDegrees:"",
			icon:"",
			posX:"",
			posY:"",
			closeButton:"",
			gauge:"",
			txtOutOfRange:""}]  

function InitDashboardPage() {	
    SetPageCommonElements();       
    InitJSON();    
    ResetFilters();
    SetPaperSize();
    DrawDashboard();
    start();
	SetHashKey();
}

function start(){
	starttime = new Date();
    starttime = starttime.getTime();
    countdown();
}

function countdown(){
    nowtime = new Date();
    nowtime = nowtime.getTime();
    secondssinceloaded = (nowtime - starttime) / 1000; 
    if (AUTO_REFRESH_INTERVAL >= secondssinceloaded){
    	timer = setTimeout("countdown()", 1000);     
    } else {
    	clearTimeout(timer);
    	RefreshDevices();       
    }
}

function SetPaperSize(){
	var NoOfDevices = GetNoOfDevicesInDashboard();
	DASHBOARD_WIDTH = GAUGES_XSPACE + GAUGES_PER_ROW * (DEVICE_WIDTH + GAUGES_XSPACE); 
	DASHBOARD_HEIGHT = GAUGES_YSPACE + Math.ceil(NoOfDevices/GAUGES_PER_ROW) * (DEVICE_HEIGHT + GAUGES_YSPACE);
    if (paper == null) {
        paper = Raphael("daskboard", DASHBOARD_WIDTH, DASHBOARD_HEIGHT);
    } else {
        paper.setSize(DASHBOARD_WIDTH, DASHBOARD_HEIGHT);
    }            
}

function ResetFilters(){
	var ddlAutoRefresh   = document.getElementById("ddlAutoRefresh");
	ddlAutoRefresh.selectedIndex = 0;	
	AUTO_REFRESH_INTERVAL = ddlAutoRefresh.value; 
}

function DrawDashboard(){
	DEVICES = [];
	var data = GetDevicesForDashboard();
	if (data != null){
		for (var i=0; i<data.length; i++){				
			var pos = GetDevicePosition(data[i].SlotNumber);
			DEVICES.push(DrawDevice(pos.x, pos.y, data[i]));			
			var devIndex = DEVICES[i].slotNumber-1;
			DrawDeviceValue(devIndex, DEVICES[i].lastValue);			
		}		
	}
}

function TranslateDevices(slotNumber){
	for(var i=slotNumber-1; i<DEVICES.length; i++){
		var pos = GetDevicePosition(i+1);		
		DEVICES[i].drawSet.translate(pos.x - DEVICES[i].posX, pos.y - DEVICES[i].posY);
		var slot = DEVICES[i].slotNumber-1;
		DEVICES[i].slotNumber = slot;
		DEVICES[i].posX = pos.x;
		DEVICES[i].posY = pos.y;
		DEVICES[i].closeButton.slotNumber = slot; 
	}	
}

function DrawDeviceValue(idx, value){
	var minValue  = DEVICES[idx].minValue;
	var maxValue  = DEVICES[idx].maxValue;
	var deltaDegrees = DEVICES[idx].deltaDegrees;
	var rx = DEVICES[idx].gauge.rx;
	var ry = DEVICES[idx].gauge.ry;

	if (value < minValue){
		value = (90+deltaDegrees/2);
	} else if (value > maxValue) {
		value = (90+deltaDegrees/2)+Math.round(360-deltaDegrees);
	} else {
		value = (90+deltaDegrees/2)+Math.round(100*(value-minValue)/(maxValue-minValue)*(360-deltaDegrees)/100)	
	}
	//	rotate the manos according to your value	
	DEVICES[idx].gauge[0].rotate(value,rx,ry);
	var lastValue = DEVICES[idx].lastValue;
	if (lastValue <  minValue || lastValue > maxValue){
		DEVICES[idx].txtOutOfRange.show();
	} else {
		DEVICES[idx].txtOutOfRange.hide();
	}		
}

function RefreshDevice(idx, data){
	
	DEVICES[idx].drawSet[5].attr({text:data.Address64});	
	DEVICES[idx].drawSet[6].attr({text:data.Manufacturer.substring(0,20)});	
	DEVICES[idx].drawSet[7].attr({text:data.Model.substring(0,30)});
	DEVICES[idx].drawSet[11].attr({text:data.ChannelName.substring(0,12)});
	DEVICES[idx].drawSet[12].attr({text:data.ReadingTime});
	DEVICES[idx].drawSet[13].attr({text:data.Value.substring(0,12)});
	
	DEVICES[idx].address = data.Address64;
	DEVICES[idx].manufacturer = data.Manufacturer;
	DEVICES[idx].model = data.Model;
	DEVICES[idx].channelName = data.ChannelName;
	DEVICES[idx].lastRead = data.ReadingTime;
	DEVICES[idx].lastValue = data.Value;
	
	DrawDeviceValue(idx, data.Value);		
}

function RefreshDevices(){
	if (ValidateHaskKey()){
		var data = GetDevicesForDashboard();
		if (data != null){		
			for (var i=0; i<data.length; i++){							
				RefreshDevice(i,data[i]);									
			}		
		}		
	} else {
		paper.clear();
		SetPaperSize();
	    DrawDashboard();
	}
	start();
}


function GetDevicePosition(slotNumber){
	var pos = {x:"", y:""};
	pos.x = GAUGES_XSPACE + ((GAUGES_XSPACE + DEVICE_WIDTH)*((slotNumber-1)%GAUGES_PER_ROW))
	pos.y = GAUGES_YSPACE + ((GAUGES_YSPACE + DEVICE_HEIGHT)*(Math.ceil(slotNumber/GAUGES_PER_ROW)-1));	
	return pos;
}

function DrawDevice(posX, posY, data){
	var deviceId 		= data.DeviceID; 
	var channelNo 		= data.ChannelNo;
	var minValue 		= data.MinValue;
	var maxValue 		= data.MaxValue;
	var deviceAddress 	= data.Address64;
	var manufacturer 	= data.Manufacturer.substring(0,20);
	var model 			= data.Model.substring(0,30);
	var gaugeType 		= data.GaugeType;
	var icon 			= data.Icon;
	var channelName 	= data.ChannelName.substring(0,12);
	var lastRead 		= data.ReadingTime;
	var lastValue 		= data.Value.substring(0,12);
	var slotNumber 		= data.SlotNumber; 
	var iconWidth 		= data.IconWidth;
	var iconHeight 		= data.IconHeight;  	
	
	var drawSet = paper.set();	
	
	var textSize = Math.ceil(LEFT_INFO_WIDTH*8/100);
	var mainRectangle = paper.rect(posX, posY, DEVICE_WIDTH, DEVICE_HEIGHT);	
	var headerRectagle = paper.rect(posX, posY, DEVICE_WIDTH, TOP_INFO_WIDTH).attr({fill:"#B8C0CA"});
	var deviceIcon = paper.image(icon, posX+2 , posY+TOP_INFO_WIDTH+2, iconWidth, iconHeight);
	var gaugeImage;
	
	switch (gaugeType) {
		case 1 : gaugeImage = paper.image("styles/images/gauges/full.png", posX + LEFT_INFO_WIDTH-1, posY + TOP_INFO_WIDTH - 1, GAUGE_WIDTH, GAUGE_HEIGHT); break;
		case 2 : gaugeImage = paper.image("styles/images/gauges/half.png", posX + LEFT_INFO_WIDTH-1, posY + TOP_INFO_WIDTH - 1, GAUGE_WIDTH, GAUGE_HEIGHT); break;
	};	
	
	var closeButton = paper.image("styles/images/close.png",posX + DEVICE_WIDTH - TOP_INFO_WIDTH*75/100, posY + TOP_INFO_WIDTH*25/100, TOP_INFO_WIDTH*50/100, TOP_INFO_WIDTH*50/100);
		closeButton.click(EventClickOnDevice);		
		closeButton.mouseover(EventOnMouseOver);
		closeButton.mouseout(EventOnMouseOut);				
		closeButton.slotNumber = slotNumber;												
		
	var Address = paper.text(posX+10 , posY + GAUGE_HEIGHT*8/100, deviceAddress);		
	var Manufacturer = paper.text(posX + iconWidth+10, posY + TOP_INFO_WIDTH + GAUGE_HEIGHT*20/100, manufacturer);	
	var Model = paper.text(posX + iconWidth+10, posY + TOP_INFO_WIDTH + GAUGE_HEIGHT*10/100, model);
		Address.attr({font:textSize+'px Fontin-Sans, Arial', fill:"#273A54"}).attr({x:Address.attr("x") + Address.getBBox().width/2});		
		Manufacturer.attr({font:textSize+'px Fontin-Sans, Arial', fill:"#273A54"}).attr({x:Manufacturer.attr("x") + Manufacturer.getBBox().width/2});	
		Model.attr({font:textSize+'px Fontin-Sans, Arial', fill:"#273A54"}).attr({x:Model.attr("x") + Model.getBBox().width/2});				
		
	var txtChannel = paper.text(posX + LEFT_INFO_WIDTH*3/100, posY + TOP_INFO_WIDTH + GAUGE_HEIGHT*70/100, "Channel:")	
	var txtLastRead = paper.text(posX + LEFT_INFO_WIDTH*3/100, posY + TOP_INFO_WIDTH + GAUGE_HEIGHT*80/100, "Last Read:");
	var txtValue = paper.text(posX + LEFT_INFO_WIDTH*3/100, posY + TOP_INFO_WIDTH + GAUGE_HEIGHT*90/100, "Value:");
			
		txtChannel.attr({font:textSize+'px Fontin-Sans, Arial', fill:"#273A54"}).attr({x:txtChannel.attr("x") + txtChannel.getBBox().width/2});
		txtLastRead.attr({font:textSize+'px Fontin-Sans, Arial', fill:"#273A54"}).attr({x:txtLastRead.attr("x") + txtLastRead.getBBox().width/2});
		txtValue.attr({font:textSize+'px Fontin-Sans, Arial', fill:"#273A54"}).attr({x:txtValue.attr("x") + txtValue.getBBox().width/2});
		
	var ChannelName = paper.text(posX + LEFT_INFO_WIDTH*45/100, posY + TOP_INFO_WIDTH + GAUGE_HEIGHT*70/100, channelName);
	var LastReadTime = paper.text(posX + LEFT_INFO_WIDTH*45/100, posY + TOP_INFO_WIDTH + GAUGE_HEIGHT*80/100, lastRead);
	var LastValue = paper.text(posX + LEFT_INFO_WIDTH*45/100, posY + TOP_INFO_WIDTH + GAUGE_HEIGHT*90/100, lastValue);	
		ChannelName.attr({font:textSize+'px Fontin-Sans, Arial', fill:"#273A54"}).attr({x:ChannelName.attr("x") + ChannelName.getBBox().width/2});
		LastReadTime.attr({font:textSize+'px Fontin-Sans, Arial', fill:"#273A54"}).attr({x:LastReadTime.attr("x") + LastReadTime.getBBox().width/2});
		LastValue.attr({font:textSize+'px Fontin-Sans, Arial', fill:"#273A54"}).attr({x:LastValue.attr("x") + LastValue.getBBox().width/2});				
		
	var Gauge;
	switch (gaugeType){
		case 1 : Gauge = DrawFullGauge(posX + LEFT_INFO_WIDTH, posY + TOP_INFO_WIDTH, minValue, maxValue);break;
		case 2 : Gauge = DrawHalfGauge(posX + LEFT_INFO_WIDTH, posY + TOP_INFO_WIDTH, minValue, maxValue);break;
		default:
	};	
	var deltaDegrees = Gauge.deltaDegrees;		
	
	var txtOutOfRange = paper.text(posX + LEFT_INFO_WIDTH + GAUGE_WIDTH*50/100, posY + TOP_INFO_WIDTH + GAUGE_HEIGHT*93/100, "Out of range!");
		txtOutOfRange.attr({font:textSize+'px Fontin-Sans, Arial', fill:"#FF0000"});
		txtOutOfRange.hide();

	drawSet.push(mainRectangle);	//[0]
	drawSet.push(headerRectagle);	//[1]
	drawSet.push(deviceIcon);		//[2]
	drawSet.push(gaugeImage);		//[3]
	drawSet.push(closeButton);		//[4]
	drawSet.push(Address);			//[5]
	drawSet.push(Manufacturer);		//[6]
	drawSet.push(Model);			//[7] 
	drawSet.push(txtChannel);		//[8]
	drawSet.push(txtLastRead);		//[9]
	drawSet.push(txtValue);			//[10]
	drawSet.push(ChannelName);		//[11]
	drawSet.push(LastReadTime);		//[12]
	drawSet.push(LastValue);		//[13]
	drawSet.push(Gauge);			//[14]	
	drawSet.push(txtOutOfRange);	//[15]	
	
	var result = {};
		result.channelNo	= channelNo;
		result.deviceId		= deviceId;
		result.slotNumber 	= slotNumber;
		result.address 		= deviceAddress;
		result.manufacturer = manufacturer; 
		result.model 		= model;
		result.channelName	= channelName;
		result.lastRead 	= lastRead;
		result.lastValue 	= lastValue;	
		result.gaugeType 	= gaugeType; 
		result.minValue		= minValue;
		result.maxValue		= maxValue;
		result.deltaDegrees = deltaDegrees;
		result.icon			= icon;
		result.posX			= posX;
		result.posY			= posY;
		result.drawSet		= drawSet;	
		result.closeButton	= closeButton;
		result.gauge		= Gauge;
		result.txtOutOfRange = txtOutOfRange;
	return result;	
	
}

function DrawFullGauge(posX, posY, minValue, maxValue){
	var gauge = paper.set();

	var deltaDegrees = 92;
	var delta = 2*Math.PI*deltaDegrees/360;
	var alfa0 = (Math.PI/2) + (delta/2);	
	var pointsArray = [];
	var noOfPoints = 13;	
	var rx = GAUGE_WIDTH/2-1;
	var ry = GAUGE_HEIGHT/2+GAUGE_HEIGHT*6/100+1;
	var textSize = Math.ceil(GAUGE_WIDTH*6/100); 
	var interval = (maxValue - minValue) / (noOfPoints-1)
	if (noOfPoints > maxValue - minValue){
		for (var i=0; i<noOfPoints; i++) {		
			pointsArray.push(Math.round((minValue+interval*i)*100)/100);		
		}				
	} else {
		for (var i=0; i<noOfPoints; i++) {		
			pointsArray.push(Math.round(minValue+interval*i));		
		}				
	}
	var step = (2*Math.PI-delta) / (noOfPoints-1);
	var r = 45/100*GAUGE_WIDTH;	
	var RX = rx + posX;
	var RY = ry + posY;
 		
	gauge.push(paper.rect(RX, RY, GAUGE_WIDTH*30/100, 1, 100).attr({fill:"#273A54"})); 
	
	for(var i = 0; i < noOfPoints; i++ ){		
		var alfa = alfa0 + step*i;
		var x = RX + r*Math.cos(alfa); 
		var y = RY + r*Math.sin(alfa);		
		var color = "#273A54"
		if (i >= noOfPoints/2){color = "#FFFFFF"}
		var t = paper.text(x, y, pointsArray[i].toString())
		t.attr({font:textSize+'px Fontin-Sans, Arial', fill:color});
		gauge.push(t);
	}	
	
	gauge.deltaDegrees = deltaDegrees;
	gauge.rx = posX + rx;
	gauge.ry = posY + ry;
	return gauge; 
}

function DrawHalfGauge(posX, posY, minValue, maxValue){
	var gauge = paper.set();
	
	var deltaDegrees = 180
	var delta = 2*Math.PI*deltaDegrees/360;
	var alfa0 = (Math.PI/2) + (delta/2);			
	var pointsArray = [];
	var noOfPoints = 9;	
	var rx = GAUGE_WIDTH/2-1;
	var ry = GAUGE_HEIGHT/2+GAUGE_HEIGHT*23/100+1;
	var textSize = Math.ceil(GAUGE_WIDTH*6/100);
	var interval = (maxValue - minValue) / (noOfPoints-1)  
	if (noOfPoints > maxValue - minValue){
		for (var i=0; i<noOfPoints; i++) {		
			pointsArray.push(Math.round((minValue+interval*i)*100)/100);		
		}				
	} else {
		for (var i=0; i<noOfPoints; i++) {		
			pointsArray.push(Math.round(minValue+interval*i));		
		}				
	};			
	var step = (2*Math.PI-delta) / (noOfPoints-1);
	var r = 43/100*GAUGE_WIDTH;		
	var RX = rx + posX;
	var RY = ry + posY;

	gauge.push(paper.rect(RX, RY, GAUGE_WIDTH*30/100, 1, 100).attr({fill:"#273A54"})); 				

	for(var i = 0; i < noOfPoints; i++ ){		
		var alfa = alfa0 + step*i;
		var x = RX + r*Math.cos(alfa); 
		var y = RY + r*Math.sin(alfa);
		var t = paper.text(x, y, pointsArray[i].toString())
		var color = "#273A54"
		if (i >= noOfPoints/2){
			color = "#FFFFFF"
		}
		t.attr({font:textSize+'px Fontin-Sans, Arial', fill:color});
		gauge.push(t);
	}

	gauge.deltaDegrees = deltaDegrees;
	gauge.rx = posX + rx;
	gauge.ry = posY + ry;
	return gauge; 
}

function ChangeDevicesPerRow(){
	paper.clear();
	SetPaperSize();
	DrawDashboard();	
}

function AddDeviceToDashboard(){
	document.location.href = "adddevicetodashboard.html?callerId=2";	
} 

function ChangeAutoRefreshInterval(){
	AUTO_REFRESH_INTERVAL = document.getElementById("ddlAutoRefresh").value;
}

function RemoveDevice(slotNumber){				
	if (RemoveDeviceFromDashboard(slotNumber)){
		DEVICES[slotNumber-1].drawSet.remove();
		DEVICES.splice(slotNumber-1,1);			
	};	
}

var EventClickOnDevice = function(e) {
	if (confirm("Are you sure you want to remove the device from dashboard?")){
		ShowRefreshInProgress(true);	
		var slot = this.slotNumber;		
		RemoveDevice(slot);	
	    SetPaperSize();      
	    TranslateDevices(slot);           
	    ShowRefreshInProgress(false);    		
	}
}

var EventOnMouseOver = function(e){
	this.attr({cursor:"pointer"});
}

var EventOnMouseOut = function(e){	
	this.attr({cursor:"default"});	
}

function ShowRefreshInProgress(show) {
    if (show) {
    	//document.body.style.cursor = "url('styles/images/loader_arrows.gif')"
    	document.body.style.cursor = "wait";
    } else {
    	document.body.style.cursor = "default";        
    }
}

function SetHashKey(){
	var hashSlots = new Array();
	var hashChannels= new Array();
	for (var i=0; i<DEVICES.length; i++){
		hashSlots.push(DEVICES[i].slotNumber);
		hashChannels.push(DEVICES[i].channelNo);
	}	
	CreateCookie("DASHBOARD_HASH_KEY", hashSlots.toString()+","+hashChannels.toString(), null);
}

function ValidateHaskKey(){
	if (GetDashboardHashKey() == ReadCookie("DASHBOARD_HASH_KEY")){
		SetHashKey();
		return true;
	} else {		
		SetHashKey();
		return false;
	}
}