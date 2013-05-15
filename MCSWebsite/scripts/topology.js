var methods = ["sqldal.execute", "user.logout", "file.remove", "file.create", "file.exists"];

/** CONSTANTS ***************************************************** */
// used colors
var grayColor  = "#EEEEEE";
var blueColor  = "#0000FF";
var redColor   = "#FF0000";
var greenColor = "#00FF00";
var whiteColor = "#FFFFFF";
var blackColor = "#000000";

// Legend colors links
var LinkColor = "#000000";
var ClockSourceColor = "#FF0000";

// Legend colors links Services
var PublishServiceColor = blueColor;
var EventServiceColor = redColor;
var MaintenanceServiceColor = greenColor;
var BlockTransferServiceColor = blackColor;	

// graph parameters
var CellWidth = 50;
var CellWidthDefault = 50;
var CellWidthMin = 25;
var CellWidthMax = 75;

var SequenceOffset = 20;
var SequenceOffsetDefault = 20;
var SequenceOffsetMin = 0;
var SequenceOffsetMax = 40;

var GraphMaxLevelIndex = 5;
var GraphMaxLevel = 3;
var PaperWidth = 960;
var PaperHeight = 500;

var CLOCKSOURCE_NONE = 0;
var CLOCKSOURCE_PREFERRED = 1;

// LEVEL parameters
var LevelSequence = new Array(10); 
LevelSequence[0] = [1];
LevelSequence[1] = [1];
LevelSequence[2] = [1, 2];
LevelSequence[3] = [2, 1, 3];
LevelSequence[4] = [2, 4, 1, 3];
LevelSequence[5] = [3, 1, 5, 4, 2];
LevelSequence[6] = [3, 1, 6, 5, 2, 4];
LevelSequence[7] = [4, 1, 7, 5, 3, 6, 2];
LevelSequence[8] = [4, 1, 8, 3, 5, 6, 2, 7];
LevelSequence[9] = [5, 1, 9, 4, 6, 2, 7, 8, 3];

var LevelTopBorder = 2;
var LevelBottomBorder = 2;
var LevelLeftBorder = 50;
var LevelRightBorder = 30;
var LevelSeparatorSize = 20;

// ICON parameters
var IconMargin = 2; // px
var IconLabelHeight = 12; // px

/** GLOBAL VARIABLES ********************************************** */
// topology structures
var LEVELS = [];
var DEVICES = [];
var DEVICE_INDEX = [];
var LINKS = [];
var SERVICES = [];

var SelectedDeviceId = null;
var SelectedDeviceIdOld = null;
var ServiceSelected = new Array(null, null);
var DragingProcess = false;

var ShowAllLinks = true;
var ShowSignalQuality = true;
var ShowMode = 1;
var LineShape = "C";
var DeviceChanged = false;

var LastRefreshedString;
var LastRefreshedDate;
var RefreshDeviceInfoActive = false;

var paper = null;
var DeviceLinks = [];
var DeviceNodes = [];
var DeviceLevels = [];
var isDrag = false;
var isClickedOnly = false;

var chkLink = true;
var chkClockSource = false;

/** Device events ************************************************* */
// OnDrag
var Event_DragDevice = function (e) {
    UnTip();
    this.dx = e.clientX;
    this.dy = e.clientY;
    isDrag = this;
    this.animate({"fill-opacity" : .5}, 500);
    e.preventDefault && e.preventDefault();
};

document.onmousemove = function (e) {
    e = e || window.event;
    if (isDrag) {
        var x = isDrag.getBBox().x, dx = e.clientX - isDrag.dx
        var y = isDrag.getBBox().y, dy = e.clientY - isDrag.dy
        x = x + dx;
        y = y + dy;
        if ((x > LEVELS[isDrag.level].X + 1 && x + isDrag.getBBox().width < LEVELS[isDrag.level].X + LEVELS[isDrag.level].Width - 1) &&
            (y > LEVELS[isDrag.level].Y + 2 && y + isDrag.getBBox().height < LEVELS[isDrag.level].Y + LEVELS[isDrag.level].Height - 1)) {
            isDrag.translate(dx, dy);
            if(isDrag.text && isDrag.image) {
                isDrag.text.translate(dx, dy);
                isDrag.image.translate(dx, dy);
            }
            for (var i=0; i<DeviceLinks.length; i++) {
                paper.connection(DeviceLinks[i]);
            }
            paper.safari();
            isDrag.dx = e.clientX;
            isDrag.dy = e.clientY;
        }
    }
    isClickedOnly = false;
};


document.onmouseup = function () {
    isDrag && isDrag.animate({"fill-opacity" : 0.10}, 500);
    isDrag = false;
};


document.onmousedown = function () {
    isClickedOnly = true;
    
};


// Click
var Event_ClickOnDevice = function (e) {
    if (!isClickedOnly)
        return;
    for (var i=0; i<DEVICES.length; i++) {
        if (DeviceNodes[i] != null && DeviceNodes[i].rect.id == this.id) {
            if (SelectedDeviceId != DEVICES[i].DeviceID) {
                this.attr({"stroke-width":3});
                SelectDevice(DEVICES[i].DeviceID, true, null);
            } else {
                this.attr({"stroke-width":1});
                SelectDevice(null, true, null);
            }
        } else {
			if (DeviceNodes[i] != null)
                DeviceNodes[i].rect.attr({"stroke-width":1});
        }
    }
}


// Hover start
var Event_TipDevice = function (e) {
    for (var i=0; i<DEVICES.length; i++) {
        if (DeviceNodes[i] != null && DeviceNodes[i].rect.id == this.id) {
            var tooltipText = 
                "Address64:&nbsp;<b>" +  DEVICES[i].Address64 + "</b>" + 
                "<br /> Role:&nbsp;<b>" + GetDeviceRole(DEVICES[i].DeviceRole) + "</b>" + 
                "<br /> DeviceTag:&nbsp;<b>" + DEVICES[i].DeviceTag + "</b>" +
                "<br /> Manufacturer:&nbsp;<b>" + DEVICES[i].Manufacturer + "</b>" + 
                "<br /> Level:&nbsp;<b>" + DEVICES[i].DeviceLevel + "</b>" + 
                "<br /> LevelIndex:&nbsp;<b>" + DEVICES[i].DeviceLevelIndex + "</b>" + 
                "<br /> Model:&nbsp;<b>" + DEVICES[i].Model + "</b>"
            Tip(tooltipText);
            return;
        }
    }
}


// Hover end
var Event_UnTipDevice = function (e) {
    UnTip();
}


/** METHODS ******************************************************* */

function DrawTopology(drawDevices, drawLevels) {
    // init draw area size
	PaperWidth =  LevelLeftBorder + GraphMaxLevelIndex * CellWidth + LevelRightBorder;
    PaperWidth = PaperWidth < 940 ? 940 : PaperWidth;
    PaperHeight = PaperHeight < 480 ? 480 : PaperHeight;
    if (drawLevels) {
        if (paper == null) {
            paper = Raphael("holder", PaperWidth, PaperHeight);
        } else {
            paper.setSize(PaperWidth, PaperHeight);
        }
    }
    if (drawLevels) {
        RemoveLevels();
        DrawLevels();
    }
    if (drawDevices) {
        RemoveDevices();
        DrawDevices();
    }

    RemoveLinks();
    var lstServices = document.getElementById("lstServices");
    var selectedItem = 0; 
    if (lstServices.selectedIndex < 0) {
    	lstServices.selectedIndex = 0;
    } else {
        selectedItem = lstServices[lstServices.selectedIndex].value;
    }
    DrawLegend(ShowMode);    
    switch (ShowMode) {
    case 1:     	
        DrawLinks();        
        break;
    case 2: 
        if (selectedItem == "A" || selectedItem == "I" || selectedItem == "O") {
            DrawServices();
        } else {
            DrawService(null, null, null);
        }
        break;
    default: ;
    }
    ShowSelectedDevice();
};


/* Get the value of X coordinate */
function GetRect(deviceId) {
    if (DEVICES == null || DEVICES.length == 0 || DEVICE_INDEX[deviceId] == null || DeviceNodes[DEVICE_INDEX[deviceId]] == null) 
        return null;

    return DeviceNodes[DEVICE_INDEX[deviceId]].rect;
}


/* Get the value of X coordinate */
function GetX(deviceId) {
    if (DEVICES == null || DEVICES.length == 0 || DEVICE_INDEX[deviceId] == null) 
        return 0;

     return DEVICES[DEVICE_INDEX[deviceId]].X;
}


/* Get the value of Y coordinate */
function GetY(deviceId) {
    if (DEVICES == null || DEVICES.length == 0 || DEVICE_INDEX[deviceId] == null) 
        return 0;

    return DEVICES[DEVICE_INDEX[deviceId]].Y;
}


/* Get the maximum value of X coordinate */
function GetMaxLevelIndex() {
    if (DEVICES == null || DEVICES.length == 0) 
        return 0;

    var maxInt = 0;
    for (var i=0; i<DEVICES.length; i++) {
        if (DEVICES[i].DeviceLevelIndex > maxInt) {
            maxInt = DEVICES[i].DeviceLevelIndex;
        }
    }
    return maxInt;
}


/* Get the maximum value of Y coordinate */
function GetMaxLevel() {
    if (DEVICES == null || DEVICES.length == 0) 
        return 0;

    var maxInt = -1;
    for (var i=0; i<DEVICES.length; i++) {
        if (DEVICES[i].DeviceLevel > maxInt) {
            maxInt = DEVICES[i].DeviceLevel;
        }
    }
    return maxInt;
}


function GetSequenceIndex(maxLevelIndex) {
    var levelIndex = 1;
    if (maxLevelIndex <= 2 ) {
        levelIndex = 1;
    } else if (maxLevelIndex == 3) {
        levelIndex = 2;
    } else if (maxLevelIndex >=  3 && maxLevelIndex <=  5) {
        levelIndex = 3;
    } else if (maxLevelIndex >=  6 && maxLevelIndex <= 10) {
        levelIndex = 4;
    } else if (maxLevelIndex >= 11 && maxLevelIndex <= 15) {
        levelIndex = 5;
    } else if (maxLevelIndex >= 16 && maxLevelIndex <= 20) {
        levelIndex = 6;
    } else if (maxLevelIndex >= 21 && maxLevelIndex <= 25) {
        levelIndex = 7;
    } else if (maxLevelIndex >= 26 && maxLevelIndex <= 30) {
        levelIndex = 8;
    } else {
        levelIndex = 9;
    }
    return levelIndex;
}


function GetLevelOfsset(level, levelIndex) {
    if (LEVELS == null || LEVELS.length == 0)
	   return;

    return LevelSequence[LEVELS[level].SequenceIndex]
                        [(levelIndex - 1)% LevelSequence[LEVELS[level].SequenceIndex].length] * SequenceOffset;
}


function GetMaxIconHeight(level) {
    if (DEVICES == null || DEVICES.length == 0) 
        return 0;

    var maxHeight = 0;
    for (var i=0; i<LEVELS[level].DeviceList.length; i++) {
        var height = GetImage(DEVICES[DEVICE_INDEX[LEVELS[level].DeviceList[i]]].ModelHex,
                              DEVICES[DEVICE_INDEX[LEVELS[level].DeviceList[i]]].DeviceRole).height;
        if (height > maxHeight) {
            maxHeight = height + IconMargin + IconLabelHeight;
        }
    }
    return maxHeight;
}


function GetLevelHeight(seqIndex, level) {
    return SequenceOffset * (LevelSequence[seqIndex].length + 1) 
        + GetMaxIconHeight(level)
        + LevelTopBorder
        + LevelBottomBorder;
}


/* Get the maximum value of Index for a level */
function GetMaxIndexLvl(level) {
    if (DEVICES == null || DEVICES.length == 0) 
        return 0;

    if (level == 0 && LEVELS[level].DeviceList.length == 2) {
        DEVICES[DEVICE_INDEX[LEVELS[level].DeviceList[0]]].DeviceLevelIndex = 1;
        DEVICES[DEVICE_INDEX[LEVELS[level].DeviceList[2]]].DeviceLevelIndex = 2;
        return 2;
    }

    var maxInt = 0;
    for (var i=0; i<LEVELS[level].DeviceList.length; i++) {
        if (DEVICES[DEVICE_INDEX[LEVELS[level].DeviceList[i]]].DeviceLevelIndex > maxInt) {
            maxInt = DEVICES[DEVICE_INDEX[LEVELS[level].DeviceList[i]]].DeviceLevelIndex;
        }
    }
    return maxInt;
}


function ScaleLevelIndex(ox, level) {
    if (LEVELS == null || LEVELS.length == 0) 
        return 0;
		
    if (LEVELS[level].ScaleFactor > 1) {
        return ox * LEVELS[level].ScaleFactor - (LEVELS[level].ScaleFactor - 1) / 2;
    } else {
        return ox;
    }
}


function GetImage(deviceModel, deviceRole) {
    var imgObject = new Image();
    imgObject.src = "styles/images/" + GetIconFileName(deviceModel, deviceRole);
    return {src: imgObject.src,
		    height:((imgObject.height > MAX_DEVICE_ICON_SIZE || imgObject.height <= 0) ? MAX_DEVICE_ICON_SIZE : imgObject.height), 
            width: ((imgObject.width  > MAX_DEVICE_ICON_SIZE || imgObject.width  <= 0) ? MAX_DEVICE_ICON_SIZE : imgObject.width)};
}


function DrawLevels() {
    if (DEVICES == null || DEVICES.length == 0 || LEVELS == null || LEVELS.length ==0)
        return;
        
    for (var i = 0; i <= GraphMaxLevel; i++) {
        DeviceLevels[i] = {level: paper.rect(LEVELS[i].X, LEVELS[i].Y, LEVELS[i].Width, LEVELS[i].Height).attr({stroke:grayColor, fill:grayColor})};
        DeviceLevels[i].text = paper.text(LEVELS[i].X + 25, LEVELS[i].Y + 10, "LEVEL " + i);
        DeviceLevels[i].text.attr({font:'12px Fontin-Sans, Arial', fill:blackColor});
    }
}

function GetDeviceLevel(deviceID) {
    var level = 0;
    
    if (LINKS == null)
       return level;

    var deviceRole = -1;
    var index = 0;
    
    var historyLinks = []; // for checking infinit loops
    for (var i = 0; i < LINKS.length; i++)
       historyLinks[i] = {direct: false, reverse: false};
       
    while (deviceRole != DT_AccessPoint) {
        index = LINKS.length;
        for (var i = 0; i < LINKS.length; i++) {
            if (LINKS[i].FromDeviceID == deviceID && LINKS[i].ClockSource == 1) {
                level++;
                if (LINKS[i].ToDeviceRole == DT_AccessPoint) {
                    return level;
                } else {
                    if (historyLinks[i].direct) // There is an infinite loop between the device and backbone and therefore there is no connection between the device and backbone
                       return 0;
                    deviceID = LINKS[i].ToDeviceID;
                    deviceRole = LINKS[i].ToDeviceRole;
					historyLinks[i].direct = true;
                    break;
                }
            } else 
            if (LINKS[i].ToDeviceID == deviceID && LINKS[i].ClockSource2 == 1 && LINKS[i].Bidirectional) {
                level++;
                if (LINKS[i].FromDeviceRole == DT_AccessPoint) {
                    return level;
                } else {
                    if (historyLinks[i].reverse) // There is an infinite loop between the device and backbone and therefore there is no connection between the device and backbone
                       return 0;
                    deviceID = LINKS[i].FromDeviceID;
                    deviceRole = LINKS[i].FromDeviceRole;
					historyLinks[i].reverse = true;
                    break;                  
                }
                
            } else {
                index--;
            }
        }
        if (index <= 0) // There is no direct link between current device and Backbone router. So, there is no link between specified device and Backbone router.
            return 0;
    }
    return 0;
}

function SetDeviceLevel() {
    var maxLevel = 1;
    for (var i = 0; i < DEVICES.length; i++) {
        DEVICES[i].DeviceLevel = (DEVICES[i].DeviceRole == DT_Gateway || 
                                  DEVICES[i].DeviceRole == DT_NetworkManager || 
                                  DEVICES[i].DeviceRole == DT_AccessPoint) ? 0 : GetDeviceLevel(DEVICES[i].DeviceID);
        if (maxLevel < DEVICES[i].DeviceLevel) {
            maxLevel = DEVICES[i].DeviceLevel;
        }
    }

    // each device that has level 0 (not set) will be moved to last level. This happens when we do not have enough information about links.
    for (var i = 0; i < DEVICES.length; i++) {
        if (DEVICES[i].DeviceLevel == 0 &&
            DEVICES[i].DeviceRole != DT_Gateway &&
            DEVICES[i].DeviceRole != DT_NetworkManager &&
            DEVICES[i].DeviceRole != DT_AccessPoint) {
            DEVICES[i].DeviceLevel = maxLevel;
        }
    }

	var levelIndex = [];
	for (var i = 0; i <= maxLevel; i++) {
		levelIndex[i] = 0;
	}
	
	for (var i = 0; i < DEVICES.length; i++) {
		switch (DEVICES[i].DeviceRole) {
            case DT_Gateway:        DEVICES[i].DeviceLevelIndex = 1; break;
            case DT_AccessPoint:    DEVICES[i].DeviceLevelIndex = 2; break;
            case DT_NetworkManager: DEVICES[i].DeviceLevelIndex = 3; break;
            default:                DEVICES[i].DeviceLevelIndex = ++levelIndex[DEVICES[i].DeviceLevel];
		}
	}
}

function SetLevels() {
    if (DEVICES == null || DEVICES.length == 0)
        return;
       
    GraphMaxLevelIndex = GetMaxLevelIndex(); 
    GraphMaxLevel = GetMaxLevel();

    LEVELS = [];
    for (var i = 0; i < DEVICES.length; i++) {
        DEVICE_INDEX[DEVICES[i].DeviceID] = i;
        if (LEVELS[DEVICES[i].DeviceLevel] == null || 
            LEVELS[DEVICES[i].DeviceLevel].DeviceList == null) {
            LEVELS[DEVICES[i].DeviceLevel] = { DeviceList: [DEVICES[i].DeviceID] };
        } else {
            LEVELS[DEVICES[i].DeviceLevel].DeviceList[LEVELS[DEVICES[i].DeviceLevel].DeviceList.length] = DEVICES[i].DeviceID;
        }
    }

    var lastLevelPosition = LevelSeparatorSize;
    var seqIndex = 0;
    
    for (var l = 0; l < LEVELS.length; l++) {
    	if (LEVELS[l].DeviceList == null){
    		alert("There is no valid data to draw topology. Try to refresh...");
    		return; 
    	}             
        seqIndex = GetSequenceIndex(LEVELS[l].DeviceList.length);
        LEVELS[l] = { X:             0,
                      Y:             lastLevelPosition,
                      Width:         LevelLeftBorder + GraphMaxLevelIndex * CellWidth + LevelRightBorder,
                      Height:        GetLevelHeight(seqIndex, l),
                      SequenceIndex: seqIndex,
                      ScaleFactor:   GraphMaxLevelIndex / ((l == 0) ? 3 : LEVELS[l].DeviceList.length),
					  DeviceList:    LEVELS[l].DeviceList}
        lastLevelPosition += LEVELS[l].Height + LevelSeparatorSize;
    }
    PaperHeight = lastLevelPosition + LevelSeparatorSize;
//	var strOut = "";
//	for (i=0;i<LEVELS.length;i++) {
//		strOut += "LEVELS["+i+"] = {X:"+LEVELS[i].X+", Y:"+LEVELS[i].Y+", Width:"+LEVELS[i].Width+", Height:"+LEVELS[i].Height+", SeqIdx:"+LEVELS[i].SequenceIndex+", Scale:"+LEVELS[i].ScaleFactor+", DevLst:["+LEVELS[i].DeviceList+"]}\n";
//	}
//	alert(strOut);
}

function DrawLinks() {
    if (LINKS == null || LINKS.length == 0)
        return;

// if (!document.getElementById("chkShowLinks").checked)
// return;
    var linkColor = blackColor;
    for (var i=0; i<LINKS.length; i++) {
        if (SelectedDeviceId != null &&
            (SelectedDeviceId != LINKS[i].FromDeviceID && 
             SelectedDeviceId != LINKS[i].ToDeviceID)) {
            continue;
        }
        var rectFrom = GetRect(LINKS[i].FromDeviceID); 
        var rectTo = GetRect(LINKS[i].ToDeviceID);
        if (rectFrom != null && rectTo != null) {
            var conn, offset1 = 0; offset2 = 0, signalQuality = null;
            if (chkClockSource && LINKS[i].ClockSource  == CLOCKSOURCE_PREFERRED || 
                chkClockSource && LINKS[i].ClockSource2 == CLOCKSOURCE_PREFERRED) {
                if (LINKS[i].Bidirectional) {
                    offset1 = -2; offset2 = 2
                }
                linkColor = (LINKS[i].ClockSource == CLOCKSOURCE_PREFERRED) ? ClockSourceColor : LinkColor;               
                signalQuality = ShowSignalQuality ? LINKS[i].SignalQuality : null;
                conn = paper.connection(rectFrom, rectTo, linkColor, true, false, null, offset1, signalQuality);
                DeviceLinks.push(conn);
                if (LINKS[i].Bidirectional) {                
                    linkColor = (LINKS[i].ClockSource2 == CLOCKSOURCE_PREFERRED) ? ClockSourceColor : LinkColor;
                    signalQuality = ShowSignalQuality ? LINKS[i].SignalQuality2 : null;
                    conn = paper.connection(rectTo, rectFrom, linkColor, true, false, null, offset2, signalQuality);
                    DeviceLinks.push(conn);
                }       
            } else {
            	signalQuality = ShowSignalQuality ? LINKS[i].SignalQuality : null;
                conn = paper.connection(rectFrom, rectTo, LinkColor, true, LINKS[i].Bidirectional, null, null, signalQuality);
                DeviceLinks.push(conn);
            }
        }
    }
}

function DrawServices() {
    if (SERVICES == null || SERVICES.length == 0)
        return;

    /*
	 * var PublishServiceColors = ["#347C2C", "#437C17", "#41A317", "#4AA02C"];
	 * var EventServiceColors = ["#E41B17", "#F62817", "#E42217", "#C11B17"];
	 * var MaintenanceServiceColors = ["#00FF00"] var BlockTransferServiceColors =
	 */
    
    var linesSpace = 4;
    var services = [];
    var serviceId = null, k = 0;
    // get services list
    for (var i=0; i<SERVICES.length; i++) {
        if (serviceId == null || serviceId != SERVICES[i].ServiceID) {
        	serviceId = SERVICES[i].ServiceID;
        	services[k++] = {ServiceID: SERVICES[i].ServiceID, ServiceType: SERVICES[i].ServiceType};
        }
    }
    // draw service
    for (k=0; k<services.length; k++) {
        var serviceColor;
        var serviceOffset;
        switch (services[k].ServiceType) {
        	case AD_PUBLISH:
        		// PublishServiceColors[services[k].ServiceID %
				// PublishServiceColors.length];
        		serviceColor = PublishServiceColor;        		
        		serviceOffset = -2;
        		break;
        	case AD_EVENT:		
               	// serviceColor = EventServiceColors[services[k].ServiceID %
				// EventServiceColors.length];
        		serviceColor = EventServiceColor;
               	serviceOffset = -1;
               	break;
        	case AD_MAINTENANCE:		
               	// serviceColor = MaintenanceServiceColors[services[k].ServiceID
				// % MaintenanceServiceColors.length];
        		serviceColor = MaintenanceServiceColor;
               	serviceOffset = 0;
               	break;               	
        	case AD_BLOCK_TRANSFFER:		
               	// serviceColor =
				// BlockTransferServiceColors[services[k].ServiceID %
				// BlockTransferServiceColors.length];
        		serviceColor = BlockTransferServiceColor;
               	serviceOffset = 1;
               	break;
            default:;   	
        }               	
        
        // var serviceOffset = (services[k].ServiceID %
		// PublishServiceColors.length - 2) * linesSpace;
        DrawService(services[k].ServiceID, serviceColor, serviceOffset);
    }
}

function DrawService(serviceId, serviceColor, serviceOffset) {	
    if (SERVICES == null || SERVICES.length == 0) 
        return;
        
    var rectFrom = null, rectTo = null;
    if (serviceColor == null)
    	switch (SERVICES[0].ServiceType) 
    	{
    		case AD_PUBLISH:
    			serviceColor = PublishServiceColor;
    			break;
    		case AD_EVENT:
    			serviceColor = EventServiceColor;
    			break;
    		case AD_MAINTENANCE:
    			serviceColor = MaintenanceServiceColor;
    			break;
    		case AD_BLOCK_TRANSFFER:
    			serviceColor = BlockTransferServiceColor;
    			break;
    		default:;	
    	} 
    if (serviceOffset == null)
        servicetOffset = 0;
    
    for (var i=0; i<SERVICES.length; i++) {
        rectFrom = GetRect(SERVICES[i].FromDeviceID);           
        rectTo   = GetRect(SERVICES[i].ToDeviceID);
        if (rectFrom != null && rectTo != null) {
        	DeviceLinks.push(paper.connection(rectFrom, rectTo, serviceColor, true, false, null, serviceOffset));
        }
    }
}

function ShowLink() {
    chkLink = !chkLink;
    DrawTopology(false, false);
}

function ShowClockSource() {
    chkClockSource = !chkClockSource;
    DrawTopology(false, false);
}

// Draw the legend of the topology
function DrawLegend(mode) {
    var divLegend = document.getElementById("divLegend");
    switch (mode) {
    case 1: 
        divLegend.innerHTML =
            "<span class=\"labels\"><b>Links legend:</b></span><br />" +
            "<input type='checkbox' id='chkLink' checked='checked' disabled /><span style=\"border-style:solid;border-width:1px;background:" + LinkColor + "\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span> Link<br />" +
            "<input type='checkbox' id='chkClockSource' " + ((chkClockSource) ? "checked='checked'" : "") + " onclick='JavaScript:ShowClockSource();' /><span style=\"border-style:solid;border-width:1px;background:" + ClockSourceColor + "\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span> ClockSource";
        break;
    case 2:
        divLegend.innerHTML =
            "<span class=\"labels\"><b>Service legend:</b></span><br />" +
            "<span style=\"border-style:solid;border-width:1px;background:" + PublishServiceColor + "\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span> Publish Service <br />" +
            "<span style=\"border-style:solid;border-width:1px;background:" + EventServiceColor + "\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span> Event Service <br />" +
            "<span style=\"border-style:solid;border-width:1px;background:" + MaintenanceServiceColor + "\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span> Maintenance Service <br />" +        
            "<span style=\"border-style:solid;border-width:1px;background:" + BlockTransferServiceColor + "\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span> BlockTransfer Service";                 
        break;
    default: ;
    }
    
}


// Detects the width and height for an image (assigned to a device)
function GetIconSize(iconSource) {
    var image = new Image();
    image.src = iconSource;
    return {height:((image.height > MAX_DEVICE_ICON_SIZE || image.height <= 0) ? MAX_DEVICE_ICON_SIZE : image.height), 
            width: ((image.width  > MAX_DEVICE_ICON_SIZE || image.width <= 0) ? MAX_DEVICE_ICON_SIZE : image.width)};
}

// Draw a device on the screen
function DrawDevice(x, y, iconSrc, label, deviceRole, level) {
    var color = GetDeviceRoleColor(deviceRole); 
    var icon = GetIconSize(iconSrc);
    var i = paper.image(iconSrc, x, y, icon.width, icon.height);
    var t = paper.text(i.attrs.x + Math.ceil(icon.width/2)-1, i.attrs.y + icon.height + IconLabelHeight - IconMargin, label);
    t.attr({font:'9px Fontin-Sans, Arial', fill:blackColor});
    var r = paper.rect(i.attrs.x - IconMargin, 
	                   i.attrs.y - IconMargin, 
                       i.attrs.width + 2*IconMargin, 
					   i.attrs.height + IconLabelHeight + 2*IconMargin, 
					   2);
    r.attr({fill:color, stroke:color, "fill-opacity":0.10, "stroke-width":1});
    r.node.style.cursor = "pointer";
    r.mousedown(Event_DragDevice);
    r.click(Event_ClickOnDevice);
    r.hover(Event_TipDevice, Event_UnTipDevice);
    r.image = i;
    r.text = t;
    r.level = level;
    r.toFront();
    return r
}


// Draw all devices from the topology
function DrawDevices() {
    if (DEVICES == null || DEVICES.length == 0)
        return;

    for (var i=0; i<DEVICES.length; i++) {
        if (DEVICES[i].X > 0 && DEVICES[i].Y > 0) {
            DeviceNodes[i] = {rect: DrawDevice(DEVICES[i].X, 
			                                   DEVICES[i].Y, 
											   DEVICES[i].Icon, 
											   DEVICES[i].Label, 
			                                   DEVICES[i].DeviceRole, 
											   DEVICES[i].DeviceLevel)};
        }
    }
}

// Refresh device object on the screen
function RefreshDevice(deviceRect) {
    var d = (new Date()).getTime();
    var imgAttrs = deviceRect.image.attrs;
    imgAttrs.src = imgAttrs.src.split("?")[0] + "?d=" + d;
    var icon = GetIconSize(imgAttrs.src);
    imgAttrs.width = icon.width;
    imgAttrs.height = icon.height;
    var txtAttrs = deviceRect.text.attrs;
    txtAttrs.x = imgAttrs.x + Math.ceil(icon.width / 2) - 1;
    txtAttrs.y = imgAttrs.y + icon.height + IconLabelHeight - IconMargin;
    var rectAttrs = deviceRect.attrs;
    rectAttrs.x = imgAttrs.x - IconMargin;
    rectAttrs.y = imgAttrs.y - IconMargin;
    rectAttrs.width = imgAttrs.width + 2 * IconMargin;
    rectAttrs.height = imgAttrs.height + IconLabelHeight + 2 * IconMargin;
}


// refresh image source for each device in topology
function RefreshDevices () {
    if (DeviceNodes == null || DeviceNodes.length == 0)
        return;
    
    for (var i=0; i<DeviceNodes.length; i++) {
        if (DeviceNodes[i] != null) {
            var imgAttrs = DeviceNodes[i].rect.image.attrs;
            if ((imgAttrs.width <= 0 || imgAttrs.height <= 0) || 
                (imgAttrs.width == MAX_DEVICE_ICON_SIZE && imgAttrs.height == MAX_DEVICE_ICON_SIZE)) {
                RefreshDevice(DeviceNodes[i].rect);
            }
        }
    }
}


// Clean all graphic objects for links. It's necessary when you want to refresh
// the topology.
function RemoveLinks() {
    if (DeviceLinks == null || DeviceLinks.length == 0)
        return;
	
    for (var i=0; i<DeviceLinks.length; i++) {
        DeviceLinks[i].line.remove();
        DeviceLinks[i].arrow && DeviceLinks[i].arrow.remove();
        DeviceLinks[i].label && DeviceLinks[i].label.remove();
    }
    DeviceLinks = [];
}


// Clean all graphic objects for devices. It's necessary when you want to
// refresh the topology.
function RemoveDevices() {
    if (DeviceNodes == null || DeviceNodes.length == 0)
        return;

    for (var i=0; i<DeviceNodes.length; i++) {
		if (DeviceNodes[i] == null)
            continue;
        DeviceNodes[i].rect.image.remove();
        DeviceNodes[i].rect.text.remove();
        DeviceNodes[i].rect.remove();
    }
    DeviceNodes = [];
}


// Clean all graphic objects for levels. It's necessary when you want to refresh
// the topology.
function RemoveLevels() {
    if (DeviceLevels == null || DeviceLevels.length == 0)
        return;

    for (var i=0; i<DeviceLevels.length; i++) {
        DeviceLevels[i].level.remove();
        DeviceLevels[i].text.remove();
    }
    DeviceLevels = [];
}


// Translate device position into screen coordinates
function SetScreenCoordinates() {
    if (DEVICES == null || DEVICES.length == 0)
        return;
    
    for (var i=0; i<DEVICES.length; i++) {
        var icon = GetIconSize(DEVICES.Icon);
        if (DEVICES[i].DeviceLevelIndex > 0 && DEVICES[i].DeviceLevel >= 0) {
            DEVICES[i].X = Math.floor(LevelLeftBorder + CellWidth * (ScaleLevelIndex(DEVICES[i].DeviceLevelIndex, DEVICES[i].DeviceLevel) - 1));
            DEVICES[i].Y = Math.floor(LEVELS[DEVICES[i].DeviceLevel].Y + GetLevelOfsset(DEVICES[i].DeviceLevel, DEVICES[i].DeviceLevelIndex));
        } else {
            DEVICES[i].X = 0;
            DEVICES[i].Y = 0;
        }
    }
}


// Select a device
function SelectDevice(deviceId, draw, prevAction) {
    var chkShow = document.getElementById("chkShow");
    var chkShowAllLinks = document.getElementById("chkShowAllLinks");
    var chkShowSignalQuality = document.getElementById("chkShowSignalQuality")
    var lstServices = document.getElementById("lstServices");
    if (prevAction != "DevicesChanged") {
        document.getElementById("lstDevices").selectedIndex = 0;
    }
    var prevRole = GetDeviceRoleById(SelectedDeviceId);
    SelectedDeviceId = deviceId;
    
    switch (ShowMode) {
    case 1: /* Links */
        chkShowAllLinks.checked = (SelectedDeviceId) ? false : true;
        ShowAllLinks = (SelectedDeviceId) ? false : true;
        chkShowSignalQuality.checked = ShowSignalQuality;
        // remove service selection
        lstServices.selectedIndex = 0;
        document.getElementById("tblServiceDetail").innerHTML = "<span class='labels'><b>Service&nbsp;details:</b></span>";
        break;
    case 2: /* Services */
        chkShowAllLinks.checked = false;
        ShowAllLinks = false;
        chkShowSignalQuality.checked = false;
        ShowSignalQuality = false;
        var prevOption = lstServices.selectedIndex;
        PopulateServicesTable(); 
        // remove service selection
        var devRole = GetDeviceRoleById(deviceId);
        if (devRole != DT_Gateway && 
            devRole != DT_AccessPoint && 
            devRole != DT_NetworkManager &&
            (prevOption == 1 || prevOption == 2 || prevOption == 3)) {
            if (prevRole == DT_Gateway || 
                prevRole == DT_AccessPoint || 
                prevRole == DT_NetworkManager) {
            	lstServices.selectedIndex = 0;
                SERVICES = null;
            } else {
            	lstServices.selectedIndex = prevOption;
                SERVICES = GetServicesElements(deviceId, prevOption);
                draw = true;
            }
        } else {
        	lstServices.selectedIndex = 0;
            SERVICES = null;
        }
        document.getElementById("tblServiceDetail").innerHTML = "<span class='labels'><b>Service&nbsp;details:</b></span>";
        break;
    default: ;
    }
    // select device in combobox
    var lstDevices = document.getElementById("lstDevices");
    for (var i=0; i<lstDevices.length; i++) {
        if (lstDevices[i].value == SelectedDeviceId) {
            lstDevices.selectedIndex = i;
            break;
        }
    }
    if (draw) {
        DrawTopology(false, false);
    }
}


// Makes the selected device more visible
function ShowSelectedDevice() {
    if (DEVICES == null || DEVICES.length == 0)
        return;

    for (var i=0; i<DEVICES.length; i++) {
        if (SelectedDeviceId == DEVICES[i].DeviceID) {
			if (DeviceNodes[i] != null)
                DeviceNodes[i].rect.attr({"stroke-width":3});
        } else {
			if (DeviceNodes[i] != null)
                DeviceNodes[i].rect.attr({"stroke-width":1});
        }
    }
}


// Set device coordinates
function SetDevicePosition(deviceId, x, y) {
    if (DEVICES == null || DEVICES.length == 0)
        return;

    DEVICES[DEVICE_INDEX[deviceId]].X = x;
    DEVICES[DEVICE_INDEX[deviceId]].Y = y;
}


function ZoomInTopologyW() {
    var btnZoomInW = document.getElementById("btnZoomInW");
    var btnZoomOutW = document.getElementById("btnZoomOutW")
    if (CellWidth < CellWidthMax) {
        CellWidth += 5;
        SetLevels(); 
        SetScreenCoordinates();
        DrawTopology(true, true);
        if (btnZoomOutW.disabled) {
            btnZoomOutW.disabled = false;
            btnZoomOutW.style.cursor = "pointer";
        }
    }
    if (CellWidth >= CellWidthMax) {
        btnZoomInW.disabled = true;
        btnZoomInW.style.cursor = "default";
    }
}


function ZoomOutTopologyW() {
    var btnZoomInW = document.getElementById("btnZoomInW");
    var btnZoomOutW = document.getElementById("btnZoomOutW")
    if (CellWidth > CellWidthMin) {
        CellWidth -= 5;
        SetLevels(); 
        SetScreenCoordinates();
        DrawTopology(true, true);
        if (btnZoomInW.disabled) {
            btnZoomInW.disabled = false;
            btnZoomInW.style.cursor = "pointer";
        }
    }
    if (CellWidth <= CellWidthMin) {
        btnZoomOutW.disabled = true;
        btnZoomOutW.style.cursor = "default";
    }
}


function ZoomInTopologyH() {
    var btnZoomInH = document.getElementById("btnZoomInH");
    var btnZoomOutH = document.getElementById("btnZoomOutH")
    if (SequenceOffset < SequenceOffsetMax) {
        SequenceOffset += 5;
        SetLevels();
        SetScreenCoordinates();
        DrawTopology(true, true);
        if (btnZoomOutH.disabled) {
            btnZoomOutH.disabled = false;
            btnZoomOutH.style.cursor = "pointer";
        }
    }
    if (SequenceOffset >= SequenceOffsetMax) {
        btnZoomInH.disabled = true;
        btnZoomInH.style.cursor = "default";
    }
}


function ZoomOutTopologyH() {
    var btnZoomInH = document.getElementById("btnZoomInH");
    var btnZoomOutH = document.getElementById("btnZoomOutH")
    if (SequenceOffset > SequenceOffsetMin) {
        SequenceOffset -= 5;
        SetLevels();
        SetScreenCoordinates();
        DrawTopology(true, true);
        if (btnZoomInH.disabled) {
            btnZoomInH.disabled = false;
            btnZoomInH.style.cursor = "pointer";
        }
    }
    if (SequenceOffset <= SequenceOffsetMin) {
        btnZoomOutH.disabled = true;
        btnZoomOutH.style.cursor = "default";
    }
}


function ZoomFitToWindow() {
    var paperWidth = PaperWidth;
    var paperHeight = PaperHeight;
    var topologyDiv = document.getElementById("networkTopology");
    var divWidth = parseInt(topologyDiv.style.width);
    var divHeight = parseInt(topologyDiv.style.height);
    var wasZoomed = false;

    CellWidth = CellWidthDefault;
    paperWidth = LevelLeftBorder + GraphMaxLevelIndex * CellWidth + LevelRightBorder;
    SequenceOffset = SequenceOffsetDefault;
    SetLevels();
    paperHeight = 0;
    for (var i = 0; i < LEVELS.length; i++) {
        paperHeight += LEVELS[i].Height + ((i < LEVELS.length - 1) ? LevelSeparatorSize : 0);
    }
    
    while (divWidth < paperWidth || divHeight < paperHeight) {
        var widthZoomed = false, heightZoomed = false;
        if (CellWidth > CellWidthMin && divWidth < paperWidth) {
            CellWidth -= 5;
            paperWidth = LevelLeftBorder + GraphMaxLevelIndex * CellWidth + LevelRightBorder;
            widthZoomed = true;
        }
        if (SequenceOffset > SequenceOffsetMin && divHeight < paperHeight) {
            SequenceOffset -= 5;
            heightZoomed = true;
        }
        
        if (widthZoomed || heightZoomed) {
            SetLevels();
            paperHeight = 0;
            for (var i = 0; i < LEVELS.length; i++) {
                paperHeight += LevelSeparatorSize + LEVELS[i].Height;
            }
            wasZoomed = true;
        } else {
            break;
        }
    }
    
    if (wasZoomed) {
        SetScreenCoordinates();
        DrawTopology(true, true);
        document.getElementById("btnZoomOutH").disabled = (SequenceOffset <= SequenceOffsetMin);
        document.getElementById("btnZoomOutW").disabled = (CellWidth <= CellWidthMin);
    }
}


function ZoomNormalSize() {
    document.getElementById("btnZoomInH").disabled = false;
    document.getElementById("btnZoomOutH").disabled = false;
	document.getElementById("btnZoomInW").disabled = false;
    document.getElementById("btnZoomOutW").disabled = false;
    SequenceOffset = SequenceOffsetDefault;
    CellWidth = CellWidthDefault;
    SetLevels(); 
    SetScreenCoordinates();
    DrawTopology(true, true);
}


function SetDefaultWidth() {
    document.getElementById("btnZoomInW").disabled = false;
    document.getElementById("btnZoomOutW").disabled = false;
    CellWidth = CellWidthDefault;
    SetLevels(); 
    SetScreenCoordinates();
    DrawTopology(true, true);
}


function SetDefaultHeight() {
    document.getElementById("btnZoomInH").disabled = false;
    document.getElementById("btnZoomOutH").disabled = false;
    SequenceOffset = SequenceOffsetDefault;
    SetLevels(); 
    SetScreenCoordinates();
    DrawTopology(true, true);
}


function RefreshTopology() {
    if (AddGetTopologyCommand() != null) {
        AutoRefresh();    
    }
}

function RefreshTopologyGraph() {
	DEVICES = null;
	LINKS = null;
	LEVELS = null;
    DEVICES = GetTopology();
    if (DEVICES != null) {
        LINKS = GetTopologyLinks();
		SetDeviceLevel();
        SetLevels(); 
        //SetDeviceOrder();
        SetScreenCoordinates();
        if (SelectedDeviceId == null)
            SelectedDeviceId = GetDeviceID(DT_AccessPoint);
    }
    PopulateDevicesList();
    SelectDevice(SelectedDeviceId, false, null);
    
    document.getElementById("chkShowLinks").checked = true;
    document.getElementById("chkShowAllLinks").checked = false;
    ShowAllLinks = false;
    document.getElementById("chkShowSignalQuality").checked = false;
    ShowSignalQuality = false;
    document.getElementById("chkCurveLines").checked = true;
    LineShape = "C";
    ShowModeChanged(1, false); // Show Links
    
    DrawTopology(true, true);
}

function PopulateDevicesList() {
    if (DEVICES == null || DEVICES.length == 0)
        return;

    var lstDevices = document.getElementById("lstDevices");
    ClearList("lstDevices");
    lstDevices.options[0] = new Option("<None>", "N");
    var sortedDevices = SortDevicesList();
    for (var i=0; i<DEVICES.length; i++) {
        var devOptions = sortedDevices[i].split(",");
        lstDevices.options[i+1] = new Option(devOptions[0], devOptions[1]);
    }
}


function DevicesChanged() {
    var lstDevices = document.getElementById("lstDevices");
    var deviceId = lstDevices[lstDevices.selectedIndex].value;
    deviceId = (lstDevices[lstDevices.selectedIndex].value == "N") ? null : deviceId;
    SelectDevice(deviceId, true, "DevicesChanged");
}

/* Init page */
function InitTopologyPage() {
    SetPageCommonElements();
    InitJSON();
    DEVICES = GetTopology();
    if (DEVICES != null) {
        LINKS = GetTopologyLinks();
        SetDeviceLevel();
        SetLevels();
        SetScreenCoordinates();
    }
    PopulateDevicesList();
    SelectDevice(GetDeviceID(DT_AccessPoint), false, null);
    
    document.getElementById("chkShowLinks").checked = true;
    document.getElementById("chkShowAllLinks").checked = false;
    ShowAllLinks = false;
    document.getElementById("chkShowSignalQuality").checked = false;
    ShowSignalQuality = false;
    document.getElementById("chkCurveLines").checked = true;
    LineShape = "C";
    ShowModeChanged(1, false); // Show Links
 
    DrawTopology(true, true);
    SetRefreshLabel();
    setInterval("DisplaySecondsAgo()", 1000);
	// refresh images at every 10 seconds
	RefreshDevices();
}

	
function AutoRefresh() { 
    var cmdLastValidCmdInProgress = GetLastCommandInProgress(CTC_RequestTopology);
    if (cmdLastValidCmdInProgress != null) {                    
       RefreshDeviceInfoActive = true;  
    } else {                        
        RefreshDeviceInfoActive = false;      
    }
               
    // display last refresh date
    SetRefreshLabel();
	
    if (RefreshDeviceInfoActive) {      
        setTimeout(AutoRefresh, RefreshInterval);
    } else {
		RefreshTopologyGraph();
	}
	
}


function SetRefreshLabel() {   
    var lastRespondedCommand = GetLastCommandResponded(CTC_RequestTopology);
    if (lastRespondedCommand != null && lastRespondedCommand != NullString) {
        LastRefreshedDate = ConvertFromSQLiteDateToJSDateObject(lastRespondedCommand.TimeResponded);
        LastRefreshedString = lastRespondedCommand.TimeResponded;
    } else {
        LastRefreshedString = NAString;
    }
}
	
function GetDeviceID(role) {
    if (DEVICES == null || DEVICES.length == 0)
        return;

    for (var i=0; i<DEVICES.length; i++) {
        if (DEVICES[i].DeviceRole == role) {
            return DEVICES[i].DeviceID;
        }
    }
    return null;
}


function GetDeviceRoleById(deviceId) {
    if (deviceId == null || 
        DEVICE_INDEX[deviceId] == null || 
        DEVICES == null)
        return null;
    return DEVICES[DEVICE_INDEX[deviceId]].DeviceRole;
}

function ShowChanged(mode, draw) {
    ShowModeChanged(mode, false);
    SelectDevice(SelectedDeviceId, draw, null);
}

function ShowModeChanged(mode, draw) {
    var chkShow = document.getElementById("chkShow");
    var chkShowAllLinks = document.getElementById("chkShowAllLinks");
    var spnShowAllLinks = document.getElementById("spnShowAllLinks");
    var chkShowSignalQuality = document.getElementById("chkShowSignalQuality");
    var spnShowSignalQuality = document.getElementById("spnShowSignalQuality");
    var spnService = document.getElementById("spnService");
    var lstServices = document.getElementById("lstServices");
    ShowMode = mode;
    switch (ShowMode) {
    case 1: // Links
        // show ShowAllLinks chekbox disabled
        chkShowAllLinks.disabled = false;
        spnShowAllLinks.style.display = "";
        chkShowAllLinks.checked = false;
        ShowAllLinks = false;
        // show ShowSignalQuality checkbox disabled
        chkShowSignalQuality.disabled = false;
        spnShowSignalQuality.style.display = "";
        chkShowSignalQuality.checked = false;
        // hide Services combobox
        spnService.style.display = "none";
        lstServices.style.display = "none";
        document.getElementById("tblServiceDetail").innerHTML = "<span class='labels'><b>Service&nbsp;details:</b></span>";
        // set Access Point as selected device
        if (SelectedDeviceId == null) {
            SelectedDeviceId = GetDeviceID(DT_AccessPoint);
        }
        break;
    case 2: // Services
        // remove service selection
        ServiceSelected[0] = null;
        ServiceSelected[1] = null;
        // alert('ShowModeChanged SelectedDeviceId=' + SelectedDeviceId);
        SERVICES = GetServicesElements(SelectedDeviceId);
        // alert("SERVICES[0].ServiceID = " + SERVICES[0].ServiceID + "
		// SERVICES[0].DeviceID = " + SERVICES[0].DeviceID);
        // alert("SERVICES[1].ServiceID = " + SERVICES[1].ServiceID + "
		// SERVICES[1].DeviceID = " + SERVICES[1].DeviceID);
        // hide ShowAllLinks chekbox
        chkShowAllLinks.disabled = true;
        chkShowAllLinks.checked = false;
        ShowAllLinks = false;
        // hide ShowSignalQuality checkbox
        chkShowSignalQuality.disabled = true;
        chkShowSignalQuality.checked = false;
        ShowSignalQuality = false;
        // show Services combobox
        spnService.style.display = "";
        lstServices.style.display = "";
        document.getElementById("tblServiceDetail").innerHTML = "<span class='labels'><b>Service&nbsp;details:</b></span>";
        // set Backbone Router as selected device
        if (SelectedDeviceId == null) {
            SelectedDeviceId = GetDeviceID(DT_NetworkManager);
        }
        // get services for Backbone Router
        PopulateServicesTable();
        break;
    default: ;
    }
    if (draw) {
        DrawTopology(false, false);
    }
}


function ShowAllLinksChanged() {
    ShowAllLinks = document.getElementById("chkShowAllLinks").checked;
    if (ShowAllLinks) {
        // remove device selection
        SelectedDeviceIdOld = SelectedDeviceId;
        SelectedDeviceId = null;
    } else {
        // set Backbone Router as selected device
		if (SelectedDeviceIdOld != null) {
			SelectedDeviceId = SelectedDeviceIdOld;
		} else {
			SelectedDeviceId = GetDeviceID(DT_AccessPoint);
		}
    }
    document.getElementById("tblServiceDetail").innerHTML = "<span class='labels'><b>Service&nbsp;details:</b></span>";
    SelectDevice(SelectedDeviceId, false, null);

    DrawTopology(false, false);
}

function ShowSignalQualityChanged() {
    ShowSignalQuality = document.getElementById("chkShowSignalQuality").checked;
    DrawTopology(false, false);
}

function LineShapeChanged() {
    LineShape = (document.getElementById("chkCurveLines").checked) ? "C" : "L";
    DrawTopology(false, false);
}

function PopulateServicesTable() {
    var lstServices = document.getElementById("lstServices");   
    var servicesData = GetServices(SelectedDeviceId);
    ClearList("lstServices");
    var devRole = GetDeviceRoleById(SelectedDeviceId);
    lstServices.options[0] = new Option("<None>", "N");
    if (servicesData != null) {
        var idx = 1;
        if (devRole != DT_Gateway && 
            devRole != DT_AccessPoint &&
            devRole != DT_NetworkManager) {
        	lstServices.options[1] = new Option("<All>", "A");
        	lstServices.options[2] = new Option("<Inbound>", "I");
        	lstServices.options[3] = new Option("<Outbound>", "O");
            idx = 4;
        }
        for(var i = 0; i < servicesData.length; i++) {
        	lstServices.options[i+idx] = new Option(servicesData[i].SourceDestination, servicesData[i].ServiceID + "," + servicesData[i].DeviceID + "," + servicesData[i].GraphID);
        }
    }
}

function ServiceChanged() {
    // display selected service
    var lstServices = document.getElementById("lstServices");
    var serviceKey = lstServices[lstServices.selectedIndex].value;
    if (serviceKey == "N" || serviceKey == "A" || serviceKey == "I" || serviceKey == "O") {
        document.getElementById("tblServiceDetail").innerHTML = "<span class='labels'><b>Service&nbsp;details:</b></span>";
        switch (serviceKey) {
        case "A": 
            SERVICES = GetServicesElements(SelectedDeviceId, 1);
            break;
        case "I":
        	SERVICES = GetServicesElements(SelectedDeviceId, 2);
            break;
        case "O":
        	SERVICES = GetServicesElements(SelectedDeviceId, 3);
            break;
        default:
        	SERVICES = null;
        }
    } else {
        var serviceId = serviceKey.split(",");
        ServiceSelected[0] = serviceId[0]; // ServiceID
        ServiceSelected[1] = serviceId[1]; // DeviceID
        ServiceSelected[2] = serviceId[2]; // GraphID       
        var serviceDetail = GetServiceDetails(serviceId[0], serviceId[1]);
        if (serviceDetail) {
            document.getElementById("tblServiceDetail").innerHTML = 
            "<span class='labels'><b>Service&nbsp;details:</b></span><br/>" +
            "<span class='labels'>Service&nbsp;ID:</span>" + serviceDetail.ServiceID + ",<br/> " +
            "<span class='labels'>Application&nbsp;Domain:</span>" + GetApplicationDomain(serviceDetail.ApplicationDomain) + ", " +
            serviceDetail.SrcDeviceAddress64 + " -> " +
            serviceDetail.DstDeviceAddress64 + ", <br/>" +
            "<span class='labels'>SourceFlag&nbsp;:</span>" + serviceDetail.SourceFlag + ", " +
            "<span class='labels'>SinkFlag&nbsp;:</span>" + serviceDetail.SinkFlag + ", " +
            "<span class='labels'>IntermittentFlag&nbsp;:</span>" + serviceDetail.IntermittentFlag + ", <br/>" +
            "<span class='labels'>Period&nbsp;:</span>" + serviceDetail.Period + ", " +
            "<span class='labels'>RouteID&nbsp;:</span>" + serviceDetail.RouteID + ", " +
            "<span class='labels'>Timestamp&nbsp;:</span>" + serviceDetail.Timestamp;
        } else {
            document.getElementById("tblServiceDetail").innerHTML = "";
        }        
        SERVICES = GetServiceElements(serviceId[0], serviceId[1], serviceId[2], serviceDetail.DestinationRole);
    }
    DrawTopology(false, false);
}


Raphael.fn.connection = function (obj1, obj2, line, oriented, bidirectional, bg, offset, label) {
    if (offset == null) offset = 0;
    if (obj1.line && obj1.from && obj1.to) {
        line = obj1;
        obj1 = line.from;
        obj2 = line.to;
        oriented = line.oriented;
        bidirectional = line.bidirectional;
        offset = line.offset;
        label = line.label;
    }
    var bb1 = obj1.getBBox();
    var bb2 = obj2.getBBox();
    var p = [
        {x : bb1.x + bb1.width / 2 + offset, y : bb1.y - 1},
        {x : bb1.x + bb1.width / 2 + offset, y : bb1.y + bb1.height + 1},
        {x : bb1.x - 1, y : bb1.y + bb1.height / 2 + offset},
        {x : bb1.x + bb1.width + 1, y : bb1.y + bb1.height / 2 + offset},
        {x : bb2.x + bb2.width / 2 + offset, y : bb2.y - 1},
        {x : bb2.x + bb2.width / 2 + offset, y : bb2.y + bb2.height + 1},
        {x : bb2.x - 1, y : bb2.y + bb2.height / 2 + offset},
        {x : bb2.x + bb2.width + 1, y : bb2.y + bb2.height / 2 + offset}];

    var d = {};
    var dis = [];
    for (var i = 0; i < 4; i ++ ) {
        for (var j = 4; j < 8; j ++ ) {
            var dx = Math.abs(p[i].x - p[j].x);
            var dy = Math.abs(p[i].y - p[j].y);
            if ((i == j - 4) || 
                (((i != 3 && j != 6) || p[i].x < p[j].x) && 
                 ((i != 2 && j != 7) || p[i].x > p[j].x) && 
                 ((i != 0 && j != 5) || p[i].y > p[j].y) && 
                 ((i != 1 && j != 4) || p[i].y < p[j].y))) {
                dis.push(dx + dy);
                d[dis[dis.length - 1]] = [i, j];
            }
        }
    }
    if (dis.length == 0) {
        var res = [0, 4];
    } else {
        var res = d[Math.min.apply(Math, dis)];
    }
    
    var x1 = p[res[0]].x;
    var y1 = p[res[0]].y;
    var x4 = p[res[1]].x;
    var y4 = p[res[1]].y;
    
    var dx, dy, x2, y2, x3, y3, path, arrow, xt, yt, text;
    var arrowP1, arrowP2;
    if (LineShape == "C") {
        dx = Math.max(Math.abs(x1 - x4) / 2, 10);
        dy = Math.max(Math.abs(y1 - y4) / 2, 10);
        x2 = [x1, x1, x1 - dx, x1 + dx][res[0]].toFixed(3);
        y2 = [y1 - dy, y1 + dy, y1, y1][res[0]].toFixed(3);
        x3 = [0, 0, 0, 0, x4, x4, x4 - dx, x4 + dx][res[1]].toFixed(3);
        y3 = [0, 0, 0, 0, y1 + dy, y1 - dy, y4, y4][res[1]].toFixed(3);
        path = ["M", x1.toFixed(3), y1.toFixed(3), "C", x2, y2, x3, y3, x4.toFixed(3), y4.toFixed(3)].join(",");
    } else {
        path = ["M", x1.toFixed(3), y1.toFixed(3), "L", x4.toFixed(3), y4.toFixed(3)].join(","); 
        if (oriented) {
            arrowP1 = GetArrowPoints(x1, y1, x4, y4);
            if (bidirectional) {
                arrowP2 = GetArrowPoints(x4, y4, x1, y1);
                arrow = ["M", arrowP1.x1, arrowP1.y1, "L", x4.toFixed(3), y4.toFixed(3), "L", arrowP1.x2, arrowP1.y2, "L", arrowP1.x1, arrowP1.y1,
                         "M", arrowP2.x1, arrowP2.y1, "L", x1.toFixed(3), y1.toFixed(3), "L", arrowP2.x2, arrowP2.y2, "L", arrowP2.x1, arrowP2.y1].join(",");
            } else {
                arrow = ["M", arrowP1.x1, arrowP1.y1, "L", x4.toFixed(3), y4.toFixed(3), "L", arrowP1.x2, arrowP1.y2, "L", arrowP1.x1, arrowP1.y1].join(",");
            }
        }
    }

    /** CONSTANTS */
    var lineSizePercent = 0.1;
    var lineSizeMin = 7;
    
    /** For label offset */
    if (label != null) {
        var theta = Math.abs((y4 - y1) / (x4 - x1 == 0 ? 0.1 : x4 - x1));
        if (theta <= 2) {
            dxt = (bidirectional) ? 0 : 4*(offset==0 ? 1 : offset); 
            dyt = 4 * (offset==0 ? 1 : offset);
        } else {
            dxt = 4 * (offset==0 ? 1 : offset); 
            dyt = (bidirectional) ? 0 : 4*(offset==0 ? 1 : offset);
        }
/*
 * var dm = Math.sqrt((x4-x1)*(x4-x1)+(y4-y1)*(y4-y1)) var st = pt.y/dm; var ct =
 * pt.x/dm; var dt = 5; var xt = pt.x-dt*st; var yt = pt.y+dt*ct; if (x4<x1) yt +=
 * 12;
 * 
 */  }

    var p1, p2;
    if (line && line.line) {
        line.bg && line.bg.attr({path : path});
        var l = line.line.attr({path : path});
        var curveLength = l.getTotalLength();
        if (LineShape == "C") {
            if (oriented) {
                var arcLength = Math.floor(curveLength * lineSizePercent);
                arcLength = (arcLength < lineSizeMin) ? lineSizeMin : arcLength;
                var p1 = l.getPointAtLength(curveLength - arcLength);
                arrowP1 = GetArrowPoints(p1.x, p1.y, x4, y4);
                if (bidirectional) {
                    var p2 = l.getPointAtLength(arcLength);
                    arrowP2 = GetArrowPoints(p2.x, p2.y, x1, y1);
                    arrow = ["M", arrowP1.x1, arrowP1.y1, "L", x4.toFixed(3), y4.toFixed(3), "L", arrowP1.x2, arrowP1.y2, "L", arrowP1.x1, arrowP1.y1, 
                             "M", arrowP2.x1, arrowP2.y1, "L", x1.toFixed(3), y1.toFixed(3), "L", arrowP2.x2, arrowP2.y2, "L", arrowP2.x1, arrowP2.y1].join(",");
                } else {
                    arrow = ["M", arrowP1.x1, arrowP1.y1, "L", x4.toFixed(3), y4.toFixed(3), "L", arrowP1.x2, arrowP1.y2, "L", arrowP1.x1, arrowP1.y1].join(",");
                }
            }
        }
        var pt;
        if (label != null) {
            pt = l.getPointAtLength(Math.floor(curveLength / 2));
            line.label.attr({x: pt.x - dxt, y: pt.y - dyt});
        }
        line.arrow.attr({path : arrow});
    } else {
        var color = typeof line == "string" ? line : "#000";
        var b = bg && bg.split && this.path(path).attr({stroke : bg.split("|")[0], fill : "none", "stroke-width" : bg.split("|")[1] || 3});
        var l = this.path(path).attr({stroke : color, fill : "none"});
        var curveLength = l.getTotalLength();
        if (LineShape == "C") {
            if (oriented) {
                var arcLength = Math.floor(curveLength * lineSizePercent);
                arcLength = (arcLength < lineSizeMin) ? lineSizeMin : arcLength;
                var p1 = l.getPointAtLength(curveLength - arcLength);
                arrowP1 = GetArrowPoints(p1.x, p1.y, x4, y4);
                if (bidirectional) {
                    var p2 = l.getPointAtLength(arcLength);
                    arrowP2 = GetArrowPoints(p2.x, p2.y, x1, y1);
                    arrow = ["M", arrowP1.x1, arrowP1.y1, "L", x4.toFixed(3), y4.toFixed(3), "L", arrowP1.x2, arrowP1.y2, "L", arrowP1.x1, arrowP1.y1, 
                             "M", arrowP2.x1, arrowP2.y1, "L", x1.toFixed(3), y1.toFixed(3), "L", arrowP2.x2, arrowP2.y2, "L", arrowP2.x1, arrowP2.y1].join(",");
                }
                else {
                    arrow = ["M", arrowP1.x1, arrowP1.y1, "L", x4.toFixed(3), y4.toFixed(3), "L", arrowP1.x2, arrowP1.y2, "L", arrowP1.x1, arrowP1.y1].join(",");
                }
            }
        }
        var pt, lb = null;
        if (label != null) {
            pt = l.getPointAtLength(Math.floor(curveLength / 2));
            lb = this.text(pt.x - dxt, pt.y - dyt, label).attr({fill: color, font: '9px Fontin-Sans, Arial'});
        }
        return {
            bg: b,
            line: l,
            arrow: this.path(arrow).attr({stroke: color, fill : color}),
            from: obj1,
            to: obj2,
            oriented: oriented,
            bidirectional: bidirectional,
            offset: offset,
            label: lb
            };
    }
};


function AddGetTopologyCommand() {
    var gateway = GetGatewayDevice();
    if (gateway == null) {
        return null;
    }
// if (gateway.DeviceStatus < DS_NormalOperationCommencing) {
// alert("Gateway not registered !");
// return null;
// }
    var cmd = new Command();
    cmd.DeviceID = gateway.DeviceID;
    cmd.CommandTypeCode = CTC_RequestTopology;
    cmd.CommandStatus = CS_New;
    cmd.TimePosted = ConvertFromJSDateToSQLiteDate(GetUTCDate());
    cmd.ParametersDescription = "";
            
    return AddCommand(cmd, "");
}


function DisplaySecondsAgo() {
    if (RefreshDeviceInfoActive){
        document.getElementById("spnRefreshDate").innerHTML = ' refreshing ...';
    } else {
        document.getElementById("spnRefreshDate").innerHTML = LastRefreshedString +  (LastRefreshedString != NAString ? "  (" + Math.ceil((GetUTCDate().getTime() - LastRefreshedDate.getTime()) / 1000) + " seconds ago)" : ""); 
    };
}


function SortDevicesList() {
    if (DEVICES == null) 
       return;
    var sortedDevices = new Array(DEVICES.length);
    for (var i=0; i<DEVICES.length; i++) {
        sortedDevices[i] = DEVICES[i].Address64 + "," + DEVICES[i].DeviceID;
    }
    return sortedDevices.sort();
}


function GetArrowPoints(x, y, xx, yy ) {
  var arrowWidth = 3.0; // change these two values
  var theta = 0.323;    // to change the size of the head of the arrow
  var xPoints = [];
  var yPoints = [];
  var vecLine = [];
  var vecLeft = [];
  var fLength;
  var th;
  var ta;
  var baseX, baseY;

  xPoints[0] = xx;
  yPoints[0] = yy;

  // build the line vector
  vecLine[0] = xPoints[0] - x;
  vecLine[1] = yPoints[0] - y;

  // build the arrow base vector - normal to the line
  vecLeft[0] = -vecLine[1];
  vecLeft[1] = vecLine[0];

  // setup length parameters
  fLength = Math.sqrt(vecLine[0] * vecLine[0] + vecLine[1] * vecLine[1]) ;
  th = arrowWidth / (2.0 * fLength);
  ta = arrowWidth / (2.0 * (Math.tan(theta) / 2.0) * fLength);

  // find the base of the arrow
  baseX = xPoints[0] - ta * vecLine[0];
  baseY = yPoints[0] - ta * vecLine[1];

  // build the points on the sides of the arrow
  xPoints[1] = baseX + th * vecLeft[0];
  yPoints[1] = baseY + th * vecLeft[1];
  xPoints[2] = baseX - th * vecLeft[0];
  yPoints[2] = baseY - th * vecLeft[1];
  
  return {x1:xPoints[1], y1:yPoints[1], x2:xPoints[2], y2:yPoints[2]};
}  

function GetDeviceLevelAvarage(deviceId, level) {
    if (LINKS == null || LINKS.length ==0)
       return;
       
    var sumParentDevices = 0;
    var sumCount = 0;
    for (var i=0; i<LINKS.length; i++) {
        if (LINKS[i].FromDeviceID == deviceId) {
            if (DEVICES[DEVICE_INDEX[LINKS[i].FromDeviceID]].DeviceLevel == level) {
                sumParentDevices += DEVICES[DEVICE_INDEX[LINKS[i].FromDeviceID]].DeviceLevelIndex;
                sumCount++;
            }
        }
    }
    return sumParentDevices/sumCount;
}

function SetDeviceOrderByParents() {
    if (DEVICES == null || DEVICES.length == 0 || LEVELS == null || LEVELS.length == 0)
       return;

    for (var j=1; j<LEVELS.length; j++) {
        for (var i=0; i<LEVELS[j].DeviceList.length; i++) {
            DEVICES[DEVICE_INDEX[LEVELS[j].DeviceList[i]]].AvgLevelIndex = GetDeviceLevelAvarage(LEVELS[j].DeviceList[i], j);
        }
    }
}

function SetDeviceOrder() {
    if (DEVICES == null || DEVICES.length == 0)
       return;

    SetDeviceOrderByParents();
    for (var i=0; i<DEVICES.length-1; i++) {
        var min = i;
        for (var j = i+1; j<DEVICES.length; j++) {
            if (DEVICES[j].AvgLevelIndex < DEVICES[min].AvgLevelIndex) {
                min = j;
            }
        }
        if (i != min) {
            var swapIndex = DEVICES[i].DeviceLevelIndex;
            DEVICES[i].DeviceLevelIndex = DEVICES[min].DeviceLevelIndex;
            DEVICES[min].DeviceLevelIndex = swapIndex;
            var swapDEVICE = DEVICES[i];
            DEVICES[i]= DEVICES[min];
            DEVICES[min] = swapDEVICE;
            DEVICE_INDEX[DEVICES[i].DeviceID] = min;
            DEVICE_INDEX[DEVICES[min].DeviceID] = i;
        }
    }
}

