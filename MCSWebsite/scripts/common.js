//JSON common stuff
jsolait.baseURI = "/jsolait/";
var serviceURL = "/rpc.cgi";

//controls wether exception details are shown
var DEBUG = false;
var Version = "v1.5.5"
//force SSL for login page
var EnableSSL = false;

var NAString = "N/A";
var NullString = "NULL";
var WebsitePath = "/access_node/firmware/www/wwwroot/app/";

//jsonrpc object, used on most of the pages
var jsonrpc = null;

//========================================================
// Initializes jsonrpc object
//========================================================
function InitJSON() {
    try {
        jsonrpc = imprt("jsonrpc");
    } catch (e) {
        alert(e);
    }
}

//========================================================
// Displays a green message in a given control
//========================================================
function DisplayOperationResult(controlName, msg) {
    var control = document.getElementById(controlName);
    control.className = "opResultGreen";
    control.innerHTML = msg;
}

function DisplayOperationResultError(controlName, msg) {
    var control = document.getElementById(controlName);
    control.className = "opResultRed";
    control.innerHTML = msg;
}

function ClearOperationResult(controlName) { //to do, mai elegant
    DisplayOperationResult(controlName, "");    
    var control = document.getElementById(controlName);
    control.className = "";
}

//========================================================
// Handles exceptions and session timeouts
//========================================================
function HandleException(ex, msg) {
    //session expired case
    if (ex.message != null && 
        ex.message.indexOf("login first") >= 0 ||
        ex.name == "InvalidServerResponse") {
        LoggedUser = null;
        document.location.href = "login.html?exp";
        return;
    };
    if(!DEBUG) {
        alert(msg);
    } else {
        alert(msg + "\n" + "DEBUG: " + ex);
    };
}

//=======================================================
// Sets menu content, page title, header and footer text
//=======================================================
function SetPageCommonElements() {
    document.title = "Monitoring Control System";
    
    var divHeader = document.getElementById("header");
        divHeader.innerHTML = "<table width='100%' border='0'>" +
                                "<tr>" +
                                  "<td><span style='font-size:x-large'><b>Monitoring Control System</b></span></td>" +
                                  "<td><img src='styles/images/nivis.png' width='252' height='72'></td>" +
                                  "<td><img src='styles/images/WiHART.png' width='414' height='50'></td>" +	
                                "</tr>" +
                              "</table>";
							  
    var divFooter = document.getElementById("footer");
    divFooter.innerHTML = "<p>VR910 Monitoring Control System " + Version + " &nbsp;&nbsp;&nbsp;&nbsp;&nbsp; NIVIS&reg; 2010</p>";

    var divMenu = document.getElementById("columnB");    
    if (divMenu != null) {
    	var networkID = ReadCookie("NETWORK_ID");
        if ( networkID == null || networkID == NAString) {
        	networkID = "";
        } else {
        	networkID = "(ID="+networkID+")";
        }
        	
        var menuContent = "<h3>Network&nbsp;<span style='font-size:10px;'>"+networkID+"</span></h3>" +
                          "<ul class='list1'>" +
                          	"<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='dashboard.html'>Dashboard</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='topology.html'>Topology</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='devicelist.html'>Devices</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='networkhealth.html'>Network Health</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='readings.html'>Readings</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='commandslog.html'>Commands Log</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='alertslog.html'>Alerts</a></li>" +
                          "</ul>" +
                          "<h3>Configuration</h3>" +
                            "<ul class='list1'>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='accesspoint.html'>Access Point</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='gateway.html'>Gateway</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='networkmanager.html'>Network Manager</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='devicemng.html'>Device Management</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='mhconfig.html'>Monitoring Host</a></li>"+	
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='modbus.html'>MODBUS</a>" + 
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='advanced.html'>Advanced Settings</a></li>" +
                          "</ul>" +
                          "<h3>Statistics</h3>" +
                          "<ul class='list1'>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='systemstatus.html'>System Status</a></li>" +
                          "</ul>" +   
                          "<h3>Administration</h3>" +
                          "<ul class='list1'>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='fwupgrade.html'>System Upgrade</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='customicons.html'>Custom Icons</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='devicecodes.html'>Device Codes</a></li>" +
                          "</ul>" +
                          "<h3>Session</h3>" +
                          "<ul class='list1'>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='changepassword.html'>Change Password</a></li>" +
                            "<li>&nbsp;&nbsp;&nbsp;&nbsp;<a href='javascript:Logout();'>Logout</a></li>" +
                          "</ul>" +
                          "<p></p>"; 
     
        divMenu.innerHTML = menuContent;
    }
}

//=======================================================
// Cookie management
//=======================================================
function CreateCookie(name, value, days) {
    var expires = "";
    var date = new Date();
    
	if (days) {
		date.setTime(date.getTime()+(days*24*60*60*1000));
		expires = "; expires="+date.toGMTString();
	}
	document.cookie = name + "=" + value + expires + "; path=/";
}

function ReadCookie(name) {
	var nameEQ = name + "=";
	var ca = document.cookie.split(';');
	for(var i=0; i < ca.length; i++) {
		var c = ca[i];
		while (c.charAt(0)==' ') c = c.substring(1,c.length);
		if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
	}
	return null;
}

function EraseCookie(name) {
    CreateCookie(name, "", -1);
}

// this function works only if the strDate param respects the  mm/dd/yyyy HH:mi AM/PM format
function StringToDate(strDate) {
    var d = new Date();

/*  mm/dd/yyyy HH:mi AM */
/*  0  3  6    11 14 17 */

    var hour = Number(strDate.substr(11,2));
      
    if (strDate.substr(17,2).toUpperCase() == "PM")
    {
        hour = hour + 12;
    }    

    d.setYear(strDate.substr(6,4));
    d.setMonth(Number(strDate.substr(0,2))-1, Number(strDate.substr(3,2)));
    d.setHours(hour, Number(strDate.substr(14,2)), 0, 0);

    return d;

/*  yyy-mm-dd HH:MI:ss

    d.setYear(strDate.substr(0,4));
    d.setMonth(strDate.substr(5,2)-1, strDate.substr(8,2));
    d.setHours(strDate.substr(11,2), strDate.substr(14,2), strDate.substr(17,2),0);
*/    
}

//some string manipulation functions
function trim(str, chars) {
	return ltrim(rtrim(str, chars), chars);
}

function ltrim(str, chars) {
	chars = chars || "\\s";
	return str.replace(new RegExp("^[" + chars + "]+", "g"), "");
}

function rtrim(str, chars) {
	chars = chars || "\\s";
	return str.replace(new RegExp("[" + chars + "]+$", "g"), "");
}

var STR_PAD_LEFT = 1;
var STR_PAD_RIGHT = 2;
var STR_PAD_BOTH = 3;

function pad(str, len, pad, dir) {
 
	if (typeof(len) == "undefined") { var len = 0; }
	if (typeof(pad) == "undefined") { var pad = ' '; }
	if (typeof(dir) == "undefined") { var dir = STR_PAD_RIGHT; }
 
	if (len + 1 >= str.length) {
		switch (dir){
			case STR_PAD_LEFT:
				str = Array(len + 1 - str.length).join(pad) + str;
			break;
			case STR_PAD_BOTH:
				var right = Math.ceil((padlen = len - str.length) / 2);
				var left = padlen - right;
				str = Array(left+1).join(pad) + str + Array(right+1).join(pad);
			break;
			default:
				str = str + Array(len + 1 - str.length).join(pad);
			break;
		} // switch
	}
	return str;
}

//reads datetime value from controls
function ReadDateTime(txtDate, txtHour, txtMinute, ddlAMPM, isStartDate) {
    var DT = null;
    var date = trim(document.getElementById(txtDate).value);
    var hour = trim(document.getElementById(txtHour).value);
    var minute = trim(document.getElementById(txtMinute).value);
    var ampm = trim(document.getElementById(ddlAMPM).value);
        
    if (date.length == 0) {
    	if (isStartDate) {
    		document.getElementById(txtDate).focus();
    		alert("Invalid Start date.");
    		return null;
        } else {
        	if (hour != "" || minute != ""){
        		document.getElementById(txtDate).focus();
        		alert("Invalid End date.");
        		return null;
        	} else {        		        		
        		return -1; // valid end date
        	}; 		        	
        };		
	};
    try{
    	var dateOk = f_tcalParseDate(date);	
    } catch(e){
    	if (isStartDate){
    		alert("Invalid Start date!")
    	} else {
    		alert("Invalid End date!")
    	};    	
    }
    
    if (dateOk == undefined) {
        document.getElementById(txtDate).focus();
        return null;
    }
				
	if (isStartDate) {
		if (hour != "" && minute != ""){
			if (isNaN(hour) || hour < 1 || hour > 12) {   
	            alert("Invalid Start hour: '" + hour +"'.");
	            document.getElementById(txtHour).focus();
        		return null;
		    }
	        if (isNaN(minute) || minute < 0 || minute > 59) {   
	            alert("Invalid Start minute: '" + minute +"'.");
	            document.getElementById(txtMinute).focus();
        		return null;
	        }
	        hour = pad(hour, 2, "0", STR_PAD_LEFT);
	    	minute = pad(minute, 2, "0", STR_PAD_LEFT);
	    	DT = date + " " + hour + ":" + minute + " " + ampm;					
	    	return new Date(DT)
		}	
	    if (hour != "" && minute == "")  {
	        if (isNaN(hour) || hour < 1 || hour > 12) {   
	            alert("Invalid Start hour: '" + hour +"'.");
	            document.getElementById(txtHour).focus();
        		return null;
	        }	        
	        hour = pad(hour, 2, "0", STR_PAD_LEFT);
	    	minute = "00";
	    	document.getElementById(txtMinute).value = minute;
	    	DT = date + " " + hour + ":" + minute + " " + ampm;					
	    	return new Date(DT)
	    }
		if (hour == "" && minute == ""){
        	hour = "12";
        	minute = "00";
        	ampm = "AM";
        	document.getElementById(txtHour).value = hour;
        	document.getElementById(txtMinute).value = minute;
        	document.getElementById(ddlAMPM).selectedIndex = 0;        	
        	DT = date + " " + hour + ":" + minute + " " + ampm;					
        	return new Date(DT)
		}
	    if (hour == "" && minute != "")  {
            alert("Invalid Start hour.");
            document.getElementById(txtHour).focus();
            return null;
	    }
	} else {	
	//Is End Date 
		if (hour != "" && minute != ""){
			if (isNaN(hour) || hour < 1 || hour > 12) {   
	            alert("Invalid End hour: '" + hour +"'.");
	            document.getElementById(txtHour).focus();
	            return null;
		    }
	        if (isNaN(minute) || minute < 0 || minute > 59) {   
	            alert("Invalid End minute: '" + minute +"'.");
	            document.getElementById(txtMinute).focus();
	            return null;
	        }
	        hour = pad(hour, 2, "0", STR_PAD_LEFT);
	    	minute = pad(minute, 2, "0", STR_PAD_LEFT);
	    	DT = date + " " + hour + ":" + minute + " " + ampm;					
	    	return new Date(DT)
		}	
	    if (hour != "" && minute == "")  {
	        if (isNaN(hour) || hour < 1 || hour > 12) {   
	            alert("Invalid End hour: '" + hour +"'.");
	            document.getElementById(txtHour).focus();
	            return null;
	        }	        
	        hour = pad(hour, 2, "0", STR_PAD_LEFT);
	    	minute = "59";
	    	document.getElementById(txtMinute).value = minute;
	    	DT = date + " " + hour + ":" + minute + " " + ampm;					
	    	return new Date(DT)
	    }
		if (hour == "" && minute == ""){
        	hour = "11";
        	minute = "59";
        	ampm = "PM";
        	document.getElementById(txtHour).value = hour;
        	document.getElementById(txtMinute).value = minute;
        	document.getElementById(ddlAMPM).selectedIndex = 1;        	
        	DT = date + " " + hour + ":" + minute + " " + ampm;					
        	return new Date(DT)
		}
	    if (hour == "" && minute != "")  {
            alert("Invalid End hour.");
            document.getElementById(txtHour).focus();
            return null;
	    }
	}	
}

function ConvertFromSQLiteDateToJSDate(strDateTime) {
    var strDate = strDateTime.split(' ')[0];
    var strTime = strDateTime.split(' ')[1];
    var arrDate = strDate.split('-');
    var arrTime = strTime.split(':');
    return new Date(arrDate[0], arrDate[1]-1, arrDate[2], arrTime[0], arrTime[1], arrTime[2]).format("web");
}

function ConvertFromSQLiteDateToJSDateObject(strDateTime) {
    var strDate = strDateTime.split(' ')[0];
    var strTime = strDateTime.split(' ')[1];
    var arrDate = strDate.split('-');
    var arrTime = strTime.split(':');
    return new Date(arrDate[0], arrDate[1]-1, arrDate[2], arrTime[0], arrTime[1], arrTime[2]);
}

//to do, use pad function [Istvan]
function toTwoCharacters(number) {
    return (number.toString().length == 2) ? number.toString() : '0' + number.toString();
}

function ConvertFromJSDateToSQLiteDate(date) {
    var strYear = date.getFullYear();
    var strMonth = toTwoCharacters(date.getMonth() + 1);
    var strDate = toTwoCharacters(date.getDate());
    var strHour = toTwoCharacters(date.getHours());
    var strMinute = toTwoCharacters(date.getMinutes());
    var strSecond = toTwoCharacters(date.getSeconds());
    return strYear + '-' + strMonth + '-' + strDate + ' ' + strHour + ':' + strMinute + ':' + strSecond;
}

function AddSecondsToDate(stringDate, noOfSeconds) {   
    var t = new Date(stringDate);  
    t.setTime(t.getTime() + noOfSeconds*1000);
    return t;
}

function TruncateSecondsFromDate(stringDate) {
    var t = new Date(stringDate);  
 
    t.setTime(t.getTime() - t.getSeconds()*1000);
 
    return t;
}

function ConvertArrayToSqlliteParam(arr) {	
	if (arr == null) {
		alert("Commands array is null!");	
	}
	var sqliteParam = "";
	for(i=0;i<arr.length;i++) {
		sqliteParam = sqliteParam + arr[i] + ",";
	}
	if (sqliteParam.length != 0) {
		//remove last comma
		sqliteParam = sqliteParam.substring(0, sqliteParam.length-1);

		sqliteParam = "(" + sqliteParam + ")";
	} else {
		sqliteParam = null;
	}		
	return sqliteParam;
}

function ClearList(controlName) {
   var lst = document.getElementById(controlName);

   if (lst.length == 0){
        return;
   }
   
   for (i = lst.length; i >= 0; i--) {
      lst[i] = null;
   }
}

function IfNullStr(val) {
	return val == "NULL" ? NAString : val;
}

//returns the index of an element in an array, -1 otherwise
function IndexInArray(arr, val) {
    for (var i=0; i<arr.length; i++) {
        if(arr[i] == val) { 
            return i;
        }
    }
    return -1;
}

//returns the index of an element in an array, -1 otherwise
function AddValueToArray(arr, val) {
    if (IndexInArray(arr, val) == -1) {
        arr.push(val);
    }
}

//returns the index of an element in an array, -1 otherwise
function GetPageParamValue(paramName) {
    var url = parent.document.URL;
    var nameIndex = url.indexOf(paramName);

    if (nameIndex < 0) {
        return null;
    }
    
    var beginIndex = nameIndex + paramName.length + 1;
    var endIndex = url.indexOf('&', beginIndex);

    if (endIndex < 0)
        endIndex = url.length;

    return url.substring(beginIndex, endIndex);
}

//returns the index of an element in an array, -1 otherwise
function IsParameter(paramName) {
    var url = parent.document.URL;
    var nameIndex = url.indexOf(paramName);

    if (nameIndex < 0) {
        return false;
    }
    
    return true;
}

String.prototype.trim = function () {
    return this.replace(/^\s*/, "").replace(/\s*$/, "");
}

String.prototype.trimAll = function () {
    if (this == null) {
        return null;
    }
    var trimString = "";
    for (var i=0; i<this.length; i++) {
        if (this.substr(i, 1) != " ") {
            trimString = trimString + this.substr(i, 1);
        }
    }
    return trimString;
}

function HexToDec(hexNumber) {
    return parseInt(hexNumber, 16);
}

function DecToHex(decNumber) {
    return decNumber.toString(16).toUpperCase();
}

function GetAsciiString(hexString) {
    var asciiString = "";
    
    if (hexString == null) {
        return null;
    }
    if (hexString.length % 2 != 0) {
        hexString = hexString + "0";
    }
    for (var i=0; i<hexString.length; i=i+2) {
        var intByte = HexToDec(hexString.substring(i, i+2));
        if (intByte >= 32 && intByte < 127) {
            asciiString = asciiString + String.fromCharCode(intByte);
        } else {
            return asciiString;
        }
    }
    return asciiString;
}

function GetHexString(asciiString) {
    var hexString = "";
    
    if (asciiString == null) {
        return null;
    }
    for (var i=0; i<asciiString.length; i++) {
        hexString = hexString + DecToHex(asciiString.charCodeAt(i));
    }
    return hexString;
}

function GetPadedHex(intVal) {
	return pad(DecToHex(intVal), 4, "0", STR_PAD_LEFT);
}	

function GetUTCDate()
{
    d = new Date();
    return new Date(d.getTime() + d.getTimezoneOffset() * 60000);
}

function getFileName(path){
    return path.split('\\').pop().split('/').pop();
}

function GetChannelFormatName(formatCode) {
    switch (formatCode) {
        case 0: return "UInt8";
        case 1: return "UInt16";
        case 2: return "UInt32";
        case 3: return "Int8";
        case 4: return "Int16";
        case 5: return "Int32";
        case 6: return "Float32";
        default: return formatCode;
    }
}

function sleep(millis){
	var date = new Date();
	var curDate = null;
	
	do { curDate = new Date(); } 
	while(curDate-date < millis);
} 

function roundNumber(num, dec) {
	var result = Math.round(num*Math.pow(10,dec))/Math.pow(10,dec);
	return result;
}

function GetHelp(popup, contentBurst, reference, position, width, height) {
    if (popup.div != null && 
        popup.div.style.display != "none" &&
        popup.reference == reference) {
        popup.hide();
        return;
    }
    popup.content = contentBurst; 
    popup.reference = reference;
    popup.width = width;
    popup.height = height;
    popup.position = position;
    popup.show();
}

function SetPager() {
    var anchorFirst = document.getElementById("anchorFirst");
    var anchorPrev = document.getElementById("anchorPrev");
    var anchorNext = document.getElementById("anchorNext");
    var anchorLast = document.getElementById("anchorLast");
    var spnPageNumber = document.getElementById("spnPageNumber");
    var spnTotalNoOfRows = document.getElementById("spnTotalNoOfRows");
        
    if (TotalNoOfRows > 0) {    	
        spnPageNumber.innerHTML = CurrentPage + "/" + TotalPages;
        spnTotalNoOfRows.innerHTML = " out of total " + TotalNoOfRows;
        if (CurrentPage > 1) {
            anchorFirst.className = "white";
            anchorFirst.href = "javascript:PageNavigate(1);";            
            anchorPrev.className = "white";
            anchorPrev.href = "javascript:PageNavigate(" + (CurrentPage - 1) + ");";

            SetAnchorsCursor("pointer", "pointer", null, null);            
        } else {
            anchorFirst.className = "tabLink";
            anchorFirst.href = "#";            
            anchorPrev.className = "tabLink";
            anchorPrev.href = "#";

            SetAnchorsCursor("default", "default", null, null);        
        }
        
        if (CurrentPage < TotalPages) {
            anchorNext.className = "white";
            anchorNext.href = "javascript:PageNavigate(" + (CurrentPage * 1 + 1) + ");";            
            anchorLast.className = "white";
            anchorLast.href = "javascript:PageNavigate(" + TotalPages + ");";
            
            SetAnchorsCursor(null, null, "pointer", "pointer");
        } else {
            anchorNext.className = "tabLink";
            anchorNext.href = "#";            
            anchorLast.className = "tabLink";
            anchorLast.href = "#";
            
            SetAnchorsCursor(null, null, "default", "default");            
        }   
    } else {
        spnPageNumber.innerHTML = "";
        spnTotalNoOfRows.innerHTML = "";
        
        anchorFirst.className = "tabLink";
        anchorPrev.className = "tabLink";
        anchorNext.className = "tabLink";
        anchorLast.className = "tabLink";
        
        anchorFirst.href = "#";
        anchorPrev.href = "#";
        anchorNext.href = "#";
        anchorLast.href = "#";      
        
        SetAnchorsCursor("default", "default", "default", "default");
    }        
}

function SetAnchorsCursor(firstCursor, prevCursor, nextCursor, lastCursor){
	if (firstCursor != null){
		document.getElementById("anchorFirst").style.cursor = firstCursor;	
	};
	if (prevCursor != null){
		document.getElementById("anchorPrev").style.cursor = prevCursor;
	}; 
	if (nextCursor != null){
		document.getElementById("anchorNext").style.cursor = nextCursor;
	};
	if (lastCursor != null){
		document.getElementById("anchorLast").style.cursor = lastCursor;
	};	
}
	 
function PopupValue(divId, title, content){
	document.getElementById("popupTitle").innerHTML = title;
	document.getElementById("popupContent").innerHTML = content;
    Popup.showModal(divId);
}

function GetNetworkID(){
	var networkID = NAString;
	try {
	    var service = new jsonrpc.ServiceProxy(serviceURL, methods);	  	    
	    var result = service.config.getGroupVariables({group : "ACCESS_POINT"});	    	    
	    for (var i = 0; i < result.length; i++) {
	        if(result[i].NETWORK_ID != null) {	        	
	        	networkID = result[i].NETWORK_ID;	        	
	        }                       
	    }
	} catch(e) {		
	}	
	CreateCookie("NETWORK_ID",networkID, null);	
}

function PopulateDateTime(dateControl, hoursControl, minutesControl, ampmControl, valueToSet) {
	var txtDate    = document.getElementById(dateControl);
	var txtHours   = document.getElementById(hoursControl);
	var txtMinutes = document.getElementById(minutesControl);
	txtDate.value  = (valueToSet.getMonth() + 1).toString() + "/" + valueToSet.getDate() + "/" + valueToSet.getFullYear();
	var ddlAMPM    = document.getElementById(ampmControl);
    
	if (valueToSet.getHours() > 12) {
		txtHours.value = valueToSet.getHours() % 12;
		ddlAMPM.selectedIndex = 1;
	} else {   
		txtHours.value = valueToSet.getHours();
		ddlAMPM.selectedIndex = 0;
	}
	txtMinutes.value = valueToSet.getMinutes();	    		
} 

function ElementExists(el, ar){
	for (var i=0; i<ar.length; i++){
		if (el.toString() == ar[i].toString()){
			return true;
		};
	};
	return false;
};

function ReplaceSpecialCharacters(val){
	val = val.replace("_","\\_");
	val = val.replace("%","\\%");
	val = val.replace("'","''");
	return val;
}