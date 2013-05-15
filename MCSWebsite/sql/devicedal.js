var methods = ["sqldal.execute", 
               "sqldal.close", 
			   "sqldal.open", 
			   "user.logout", 
			   "file.remove", 
			   "file.create", 
			   "file.exists", 
			   "user.isValidSession"];

function GetDeviceInformation(deviceId) {
    if (deviceId == null || deviceId.length == 0) {
        return null;
    }
    
    var myQuery = " SELECT Address64, " +
    					 " Nickname, " +
    					 " DeviceRole, " +
    					 " DeviceStatus, " +
                  		 " LastRead, " +
                  		 " PowerSupplyStatus, " +
		                 " C.Company, "+
		                 " C.Model, "+
		                 " SoftwareRevision, "+
		                 " 'dummy'	AS DPDUsTransmitted, " +
		                 " 'dummy'	AS DPDUsReceived, " +
		                 " 'dummy'	AS DPDUsFailedTransmission, " +
		                 " 'dummy'	AS DPDUsFailedReception " +
                  " FROM Devices D " +
                  	   " LEFT OUTER JOIN DevicesCodes C ON D.DeviceCode = C.DeviceCode " +
               	  " WHERE D.DeviceID = " + deviceId ;               
    var result;     
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running get device information!");
        return null;
    }
    var dev = new Device();
    dev.Address64 = result[0][0];
    dev.Nickname = GetPadedHex(result[0][1]);
    dev.DeviceRoleID = result[0][2];
    dev.DeviceRole = GetDeviceRole(result[0][2]);
    dev.DeviceStatus = result[0][3];
    dev.LastRead = result[0][4] != "NULL" ? result[0][4] : NAString;
    dev.PowerSupplyStatus = result[0][5] != "NULL" ? result[0][5] : NAString;    
    dev.Manufacturer = (result[0][6] != "NULL") ? result[0][6] : NAString;       
    dev.Model = (result[0][7] != "NULL") ? result[0][7] : NAString;    
    dev.Revision = (result[0][8] != "NULL") ? result[0][8] : NAString;
    dev.DPDUsTransmitted = result[0][9] != "NULL" ? result[0][9] : NAString;
    dev.DPDUsReceived = result[0][10] != "NULL" ? result[0][10] : NAString;
    dev.DPDUsFailedTransmission = result[0][11] != "NULL" ? result[0][11] : NAString;
    dev.DPDUsFailedReception = result[0][12] != "NULL" ? result[0][12] : NAString;
    return dev;
}

function GetDeviceChannelsCount(deviceId) {
    if (deviceId == null || deviceId.length == 0) {
        return null;
    }
    var myQuery = " SELECT  COUNT(*) " +
                  " FROM    Channels " +
               	  " WHERE   DeviceID = " + deviceId ;
    var result;
    
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetDeviceChannelsCount!");
        return;
    }
	if (result.length != 0) {
		return result[0][0];
	} else {			
	    return null;
    }
	return null;
}

function GetDeviceChannelsValues(deviceId, pageSize, pageNo, rowCount) {
    var rowOffset = 0;
	if (deviceId == null || deviceId.length == 0) {
        return null;
    }
    
   	if (rowCount != 0) {
	   var maxAvailablePageNo = Math.ceil(rowCount/pageSize);
	
	   if (pageNo > maxAvailablePageNo) {
	       pageNo = maxAvailablePageNo;
	   }
        //global var
        TotalPages = maxAvailablePageNo;
        rowOffset = (pageNo - 1) * pageSize;
    }

    var myQuery = " SELECT C.Name, C.BurstMessage, C.DeviceVariableSlot, C.DeviceVariable, C.Classification, C.UnitCode, " +
    			  " B.UpdatePeriod, B.MaxUpdatePeriod, D.ChannelNo, C.DeviceID, C.ChannelID " +
                  " FROM Channels C " +
                  " LEFT OUTER JOIN BurstMessages B ON C.DeviceID = B.DeviceID AND C.BurstMessage = B.BurstMessage AND C.CmdNo = B.CommandNumber " +
                  " LEFT OUTER JOIN Dashboard D ON C.ChannelID = D.ChannelNo " +
               	  " WHERE C.DeviceID = " + deviceId +
               	  " LIMIT " + pageSize + " OFFSET " + rowOffset;
    var result;     
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetDeviceChannelsValues!");
        return null;
    }
    
    for (i=0;i<result.length; i++) {
        result[i].Name = result[i][0];
        result[i].BurstMessage = result[i][1];
        result[i].DeviceVariableSlot = result[i][2];
        result[i].DeviceVariable = result[i][3];
        result[i].Classification = result[i][4];
        result[i].UnitCode = result[i][5];
        result[i].UpdatePeriod = result[i][6];
        result[i].MaxUpdatePeriod = result[i][7];
        result[i].ChannelNo = result[i][8];        
        result[i].DeviceID = result[i][9];
        
        if (result[i].ChannelNo == "NULL") {
        	result[i].ATD  = "<a href='adddevicetodashboard.html?channelNo=" + result[i][10] + "?deviceId=" + result[i].DeviceID + "?callerId=1'><img src='styles/images/deviceAdd.png' title='Add to dashboard'></a>";	
        } else {
        	result[i].ATD = "";
        };  
        
        if (i % 2 == 0) {
            result[i].cellClass = "tableCell";
        } else {   
            result[i].cellClass = "tableCell2";
        }
    }
    
    result.channels = result;
    return result;
}

function GetDeviceCount(showDevicesFilter, euiAddress, deviceTag) {
    var whereClause = "";
    switch (Number(showDevicesFilter)) {
        case 0: whereClause = " DeviceStatus >= " + DS_NormalOperationCommencing + " "; break;
        case 1: whereClause = " DeviceStatus < " + DS_NormalOperationCommencing + " "; break;
        default:  
    }

    if (euiAddress) {
    	euiAddress= ReplaceSpecialCharacters(euiAddress);
    	whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " Address64 LIKE '%" + euiAddress + "%' ESCAPE '\\'";
	};    
    
    if (deviceTag) {
       whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " DeviceTag LIKE '%" + deviceTag + "%' ";
    }
    if (whereClause != "") {    
        whereClause = " WHERE " + whereClause;
    }
    var myQuery = "SELECT count(*) FROM Devices " + whereClause;
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       var devCount = service.sqldal.execute({query:myQuery});
       return devCount;
    } catch(e) {
        HandleException(e, "Unexpected error running get device count !");
        return 0;
    }  
}

function GetDevicePage(pageSize, pageNo, rowCount, showDevicesFilter, sortBy, sortOrder, euiAddress, deviceTag) {
	var rowOffset = 0;
    TotalPages = 0;
   	if (rowCount != 0) {
		var maxAvailablePageNo = Math.ceil(rowCount/pageSize);
		
		if (pageNo > maxAvailablePageNo) {
			pageNo = maxAvailablePageNo;
		}
        //global var
	    TotalPages = maxAvailablePageNo;
        rowOffset = (pageNo - 1) * pageSize;
    }

    var whereClause = "";
    switch (Number(showDevicesFilter)) {
        case 0: whereClause = " DeviceStatus >= " + DS_NormalOperationCommencing + " "; break;
        case 1: whereClause = " DeviceStatus < " + DS_NormalOperationCommencing + " "; break;
        default: ;
    }
    if (euiAddress) {
       euiAddress= ReplaceSpecialCharacters(euiAddress);
       whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " Address64 LIKE '%" + euiAddress + "%' ESCAPE '\\' ";
    }
    if (deviceTag) {
       whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " DeviceTag LIKE '%" + deviceTag + "%' ";
    }
    if (whereClause != "") {    
        whereClause = " WHERE " + whereClause;
    }
   
    var myQuery = " SELECT D.DeviceID, " +
    					 " D.Address64, " +
    					 " D.Nickname, " +
    					 " D.DeviceStatus, " +
    			         " D.LastRead, " +
    			         " D.DeviceRole, " +
    			         " C.Model, " +
    			         " D.DeviceTag " +
                   " FROM Devices D " +
                        " LEFT OUTER JOIN DevicesCodes C ON D.DeviceCode = C.DeviceCode " +
                  whereClause +
                  " ORDER BY " + GetOrderByColumnName(sortBy, sortOrder) + " LIMIT " + pageSize + " OFFSET " + rowOffset;
    var result;
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running get device page !");
        return null;
    }    
    
    var icons = GetCustomIconFiles();
    for (var i=0;i<result.length; i++) {
        result[i].DeviceID = result[i][0];        
        result[i].Address64 = "<a href='deviceinformation.html?deviceId=" + result[i].DeviceID + "'>" + result[i][1] + "</a>";       
        result[i].Nickname = GetPadedHex(result[i][2]);                
        result[i].DeviceStatus = "<span style='color:"+ GetDeviceStatusColor(result[i][3]) + "'><b>" +GetDeviceStatusName(result[i][3]) + "</b></span>";
        result[i].LastRead = result[i][4] != "NULL" ? "<a href='readings.html?deviceId=" + result[i].DeviceID + "'>" + result[i][4] + "</a>" : NAString;

        var image = new Image();
		image.src = "styles/images/" + GetIconFileName(icons, result[i][6], result[i][5]);		
        result[i].Icon = "<img src='" + image.src + "' width=" + MAX_DEVICE_ICON_SIZE + " height=" + MAX_DEVICE_ICON_SIZE + " border='0'></img></div>";
        
        if (result[i][3] >= DS_NormalOperationCommencing && result[i][5] != DT_NetworkManager) {
            result[i].CommandLink = "<a href='devicecommands.html?deviceId=" + result[i].DeviceID + "' title='Run Command'><img src='styles/images/execute.png'></a>";
        };
        if (result[i][3] == DS_NotJoined){
        	result[i].CommandLink = "<a href='javascript:DeleteDevice(" + result[i].DeviceID + ");' title='Delete Device'><img src='styles/images/delete.gif'></a>"	
        };                                        
        result[i].Model = result[i][6] != "NULL" ? result[i][6] : "";
        result[i].DeviceRole = GetDeviceRole(result[i][5]) + "/" + result[i].Model;         
        devString = GetAsciiString(result[i][7]); 
        result[i].DeviceTag = (devString != "") ? devString : NAString;

        if (i % 2 == 0) {
            result[i].cellClass = "tableCell";
        } else {   
            result[i].cellClass = "tableCell2";
        }
        result[i].HeightSpacer = "<img src='styles/images/pixel.gif' height='31px' width='1px' border='0'>";
    }    
    result.devices = result;    
    return result;
}

function DeleteDeviceAndData(deviceId) {
    var qArray = new Array();
    
    qArray[0] = "DELETE FROM CommandParameters WHERE CommandID IN (SELECT CommandID FROM Commands WHERE DeviceID = " + deviceId + ")";
    qArray[1] = "DELETE FROM Commands WHERE DeviceID = " + deviceId;
    qArray[2] = "DELETE FROM Readings WHERE ChannelID IN (SELECT ChannelID FROM Channels WHERE DeviceID = " + deviceId + ")";    
    qArray[3]  = "DELETE FROM ChannelsHistory WHERE DeviceID = " + deviceId;
    qArray[4]  = "DELETE FROM Channels WHERE DeviceID = " + deviceId;
    qArray[5]  = "DELETE FROM DeviceHistory WHERE DeviceID = " + deviceId;  
    qArray[6]  = "DELETE FROM ReportNeighborHealthList WHERE DeviceID = " + deviceId + " OR PeerID = " + deviceId;
    qArray[7]  = "DELETE FROM ReportNeighborSignalLevels WHERE DeviceID = " + deviceId + " OR PeerID = " + deviceId;    
    qArray[8]  = "DELETE FROM Routes WHERE DeviceID = " + deviceId + " OR PeerID = " + deviceId;
    qArray[9] = "DELETE FROM SourceRoutes WHERE DeviceID = " + deviceId;    
    qArray[10] = "DELETE FROM Alarms WHERE DeviceID = " + deviceId;        
    qArray[11] = "DELETE FROM Services WHERE DeviceID = " + deviceId + " OR PeerID = " + deviceId; 
    qArray[12] = "DELETE FROM DeviceConnections WHERE DeviceID = " + deviceId ;            
    qArray[13] = "DELETE FROM DeviceScheduleLinks WHERE DeviceID = " + deviceId + " OR PeerID = " + deviceId;    
    qArray[14] = "DELETE FROM Superframes WHERE DeviceID = " + deviceId;    
    qArray[15] = "DELETE FROM ReportDeviceHealth WHERE DeviceID = " + deviceId;
    qArray[16] = "DELETE FROM GraphNeighbors WHERE DeviceID = " + deviceId + " OR PeerID = " + deviceId;   
    qArray[17] = "DELETE FROM BurstCounters WHERE DeviceID = " + deviceId;
    qArray[18] = "DELETE FROM BurstTriggers WHERE DeviceID = " + deviceId;   
    qArray[19] = "DELETE FROM BurstMessages WHERE DeviceID = " + deviceId;   
    qArray[20] = "DELETE FROM Devices WHERE DeviceID = " + deviceId;

    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       for(i=0; i<qArray.length; i++) {
            service.sqldal.execute({mode : "write", query: qArray[i]});
       }
    } catch(e) {
        HandleException(e, "Unexpected error running delete device !");
    } 
}			
//END DEVICE LIST FUNCTIONS


//DEVICE SETTINGS FUNCTIONS

function GetDeviceNeighbors(deviceId){
    var myQuery = " SELECT D.Nickname, T.GraphID " +
                  " FROM GraphNeighbors T INNER JOIN Devices D ON T.PeerID = D.DeviceID " +
                  " WHERE T.DeviceID = " + deviceId                 
    var result;
 
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running get device neighbors !");
        return;
    }  
    
    for (i=0;i<result.length; i++) {
        result[i].Nickname = GetPadedHex(result[i][0]);     
        result[i].GraphID  = result[i][1];   
        
        if (i % 2 == 0) {
            result[i].cellClass = "tableCell2";
        } else {   
            result[i].cellClass = "tableCell";
        }
    }
    result.neighbors = result;
    return result;
}

function GetDeviceRoutes(deviceId) {
	var myQuery = " SELECT R.RouteID, " + 
				  		 " D.Nickname, " +
				  		 " R.GraphID, " +
				  		 " CASE R.SourceRoute " +
				  		 	  " WHEN 1 THEN S.Devices " +
				  		 	  " ELSE '" + NAString + "' " +
				  		 " END AS Source, " +
				  		 " R.SourceRoute " +
                  " FROM Routes R " +
                  	   " LEFT OUTER JOIN SourceRoutes S ON R.DeviceID = S.DeviceID AND R.RouteID = S.RouteID " +
                  	   " INNER JOIN Devices D ON R.PeerID = D.DeviceID " +
                  " WHERE R.DeviceID = " + deviceId +
                  " ORDER BY R.RouteID ";
    var result; 
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running get device routes !");
        return;
    }  
    
    for (i=0;i<result.length; i++) {
        result[i].RouteId = result[i][0];
        result[i].Nickname = GetPadedHex(result[i][1]);
        result[i].GraphID = result[i][2];	
        result[i].IsSourceRoute = result[i][3];
        result[i].Source = (result[i][4] == 1) ? "Yes" : "No" ;
        
        if (i % 2 == 0) {
            result[i].cellClass = "tableCell2";
        } else {   
            result[i].cellClass = "tableCell";
        }
    }    
    
    result.routes = result;
    return result;	
}

function GetDeviceServices(deviceId) {
    var myQuery = " SELECT  ServiceID, Nickname, ApplicationDomain, SourceFlag, SinkFlag, IntermittentFlag, Period, RouteID " +
                  " FROM    Services S " +
                  "         INNER JOIN Devices D ON S.PeerID = D.DeviceID " +
                  " WHERE   S.DeviceID = " + deviceId + 
                  " ORDER BY ServiceID"; 
    var result;
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);

       result = service.sqldal.execute({query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running get device services !");
        return;
    }  
    for (i=0;i<result.length; i++) {
        result[i].ServiceID = result[i][0];
        result[i].Nickname = GetPadedHex(result[i][1]);
        result[i].ApplicationDomain = GetApplicationDomain(result[i][2]);
        result[i].SourceFlag = (result[i][3] == 1) ? "Yes" : "No"; 
        result[i].SinkFlag = (result[i][4] == 1) ? "Yes" : "No";
        result[i].IntermittentFlag = (result[i][5] == 1) ? "Yes" : "No";
        result[i].Period = result[i][6];
        result[i].RouteID = result[i][7];
        
        if (i % 2 == 0) {
            result[i].cellClass = "tableCell2";
        } else  {   
            result[i].cellClass = "tableCell";
        }
    }
    result.services = result;
    return result;
}

//END DEVICE SETTINGS FUNCTIONS
function GetIPAddressForGateway(deviceId) {
    var myQuery = "SELECT IP, Port FROM DeviceConnections WHERE DeviceID = " + deviceId;
    
    var result;
    
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running get IP address for gateway!");
        return;
    }  
    
    if (result.length > 0) {
        return result[0][0] + ":" + result[0][1];
    } else {
        return NAString;  
    }   
}

function GetDeviceListByType(deviceType, deviceStatus) {
    var	whereClause = "";

	if (deviceType != null)
	{		
		 whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " DeviceRole IN " + ConvertArrayToSqlliteParam(deviceType) + " ";
	}
	if (deviceStatus != null)
	{		
		 whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " DeviceStatus = " + deviceStatus + " ";
	}
	if (whereClause.length > 0)
	{
		whereClause = " WHERE " + whereClause;
	}

    var myQuery =   "   SELECT  DeviceID, " +
                    "           DeviceRole, " +
                    "           Address64, " +
                    "           Nickname, " +
                    "           DeviceStatus " +
					"   FROM    Devices "
					+   whereClause + 
					"   ORDER BY Address64 ";    
	var result;
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetDeviceListByType!");
        return;
    }  
    
    for (i=0;i<result.length; i++) {        
        result[i].DeviceID = result[i][0];
        result[i].DeviceRole = GetDeviceRole(result[i][1]);
        result[i].Address64 = result[i][2];
		result[i].Nickname = GetPadedHex(result[i][3]);        
    	result[i].DeviceStatus = result[i][4];
    }
    
    result.devices = result;
    return result;    
}

function GetNetworkManagerDevice() {
    var result = GetDeviceListByType(new Array(DT_NetworkManager.toString()), null);     
	if (result.devices.length == 0) {
		alert("No Network Manager present in the system!");
		return null;
	}	
	for(i=0; i<result.devices.length; i++) {
		if (result.devices[i].DeviceStatus == DS_NormalOperationCommencing) {
			return result.devices[i];
		}
	}
	return result.devices[0];
}

function GetGatewayDevice() {
    var result = GetDeviceListByType(new Array(DT_Gateway.toString()), null);     
	if (result.devices.length == 0) {
		alert("No Gateway present in the system!");
		return null;
	}	
	return result.devices[0];
}

function DeleteDeviceHistory(deviceId) {
	var myQuery = "DELETE FROM DeviceHistory WHERE DeviceID = " + deviceId; 
					
    var result;
    
    try {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        result = service.sqldal.execute({mode : "write", query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running DeleteDeviceHistory !");
        return;
    }
}

function GetDeviceHistoryCount(deviceId, startTime, endTime, deviceStatus)
{
	var whereClause = " WHERE DeviceID = " + deviceId;

    if (startTime != null) {		
        startTime = TruncateSecondsFromDate(startTime);    
	    whereClause = whereClause + " AND Timestamp >= '" + ConvertFromJSDateToSQLiteDate(startTime) + "' ";
    }

    if (endTime != null) {		
        endTime = TruncateSecondsFromDate(AddSecondsToDate(endTime, 60));
        whereClause = whereClause + " AND Timestamp < '" + ConvertFromJSDateToSQLiteDate(endTime) + "' ";
    }

	if (deviceStatus != null) {
		whereClause = whereClause + " AND DeviceStatus = " + deviceStatus;
	}
	
	var myQuery = " SELECT COUNT(*) FROM DeviceHistory " + whereClause;	
				
    var result;
    
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetDeviceHistoryCount !");
        return;
    }  		
    
	if (result.length != 0) {
		return result[0][0];
	} else {			
	    return 0;
    }
}
	
function GetDeviceHistoryPage(deviceId, startTime, endTime, deviceStatus, sortByExpression, sortOrder, pageSize, pageNo, rowCount) {
    var rowOffset = 0;
    TotalPages = 0;
    
   	if (rowCount != 0) {
		var maxAvailablePageNo = Math.ceil(rowCount/pageSize);
		if (pageNo > maxAvailablePageNo) {
			pageNo = maxAvailablePageNo;
		}
        //global var
	    TotalPages = maxAvailablePageNo;
        rowOffset = (pageNo - 1) * pageSize;
    }

	var whereClause =  " WHERE DeviceID = " + deviceId;
    if (startTime != null) {		
        startTime = TruncateSecondsFromDate(startTime);    
	    whereClause = whereClause + " AND Timestamp >= '" + ConvertFromJSDateToSQLiteDate(startTime) + "' ";
    }
    if (endTime != null) {		
        endTime = TruncateSecondsFromDate(AddSecondsToDate(endTime, 60));
        whereClause = whereClause + " AND Timestamp < '" + ConvertFromJSDateToSQLiteDate(endTime) + "' ";
    }
	if (deviceStatus != null) {
		whereClause = whereClause + " AND DeviceStatus = " + deviceStatus + " ";
	}

	var myQuery =   "   SELECT  Timestamp, " + 
	                "           DeviceStatus " +
	                "   FROM    DeviceHistory "  
	                +   whereClause  +
	                "   ORDER BY " + GetOrderByColumnNameDeviceHistory(sortByExpression) + " " + sortOrder + 
	                "   LIMIT " + pageSize + " OFFSET " + rowOffset;		 

    var result;
    
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetDeviceHistoryPage !");
        return null;
    }  		

    var histArr = Array();
	if (result.length != 0) {
        for (i=0;i<result.length;i++) {
			var deviceHistory = new DeviceHistory();
			deviceHistory.Timestamp = result[i][0];
			deviceHistory.DeviceStatus = GetDeviceStatusName(result[i][1]);
			
			if (i % 2 == 0) {
                deviceHistory.cellClass = "tableCell";
            } else {   
                deviceHistory.cellClass = "tableCell2";
            }
			histArr[i] = deviceHistory;
		}			
	}
    histArr.registrationlogs = histArr;
    return histArr;
}

function GetOrderByColumnNameDeviceHistory(sortType) {
	switch (sortType) {
		case 1:
			return	" Timestamp ";
		case 2:
			return	" CASE DeviceStatus " +			
			        " WHEN " + DS_NotJoined + " THEN '" + GetDeviceStatusName(DS_NotJoined) + "' " +
					" WHEN " + DS_NetworkPacketsHeard + " THEN '" + GetDeviceStatusName(DS_NetworkPacketsHeard) + "' " +
					" WHEN " + DS_ASNAcquired + " THEN '" + GetDeviceStatusName(DS_ASNAcquired) + "' " +
					" WHEN " + DS_SynchronizedToSlotTime + " THEN '" + GetDeviceStatusName(DS_SynchronizedToSlotTime) + "' " +
					" WHEN " + DS_AdvertisementHeard + " THEN '" + GetDeviceStatusName(DS_AdvertisementHeard) + "' " +
					" WHEN " + DS_JoinRequested + " THEN '" + GetDeviceStatusName(DS_JoinRequested) + "' " +
					" WHEN " + DS_Retrying + " THEN '" + GetDeviceStatusName(DS_Retrying) + "' " +
					" WHEN " + DS_JoinFailed + " THEN '" + GetDeviceStatusName(DS_JoinFailed) + "' " +
					" WHEN " + DS_Authenticated + " THEN '" + GetDeviceStatusName(DS_Authenticated) + "' " +
					" WHEN " + DS_NetworkJoined + " THEN '" + GetDeviceStatusName(DS_NetworkJoined) + "' " +
					" WHEN " + DS_NegotiatingNetworkProperties + " THEN '" + GetDeviceStatusName(DS_NegotiatingNetworkProperties) + "' " +
					" WHEN " + DS_NormalOperationCommencing + " THEN '" + GetDeviceStatusName(DS_NormalOperationCommencing) + "' " +
					" ELSE '" +  GetDeviceStatusName(DS_NotJoined) + "' " + 
					" END ";
  		default:
			return " Timestamp ";
	}
}

function GetOrderByColumnName(sortType, sortOrder) {
	switch (sortType) {
		case 1:
		    return " Address64 " + sortOrder;
		case 2:
            return " Nickname " + sortOrder;
        case 3:
			return " CASE DeviceRole " +
				    " WHEN " + DT_NetworkManager + " THEN '" + GetDeviceRole(DT_NetworkManager) + "' " +
					" WHEN " + DT_Gateway + " THEN '" + GetDeviceRole(DT_Gateway) + "' " +
					" WHEN " + DT_AccessPoint + " THEN '" + GetDeviceRole(DT_AccessPoint) + "' " +
					" WHEN " + DT_Device + " THEN '" + GetDeviceRole(DT_Device) + "' " +
					" WHEN " + DT_DeviceNonRouting + " THEN '" + GetDeviceRole(DT_DeviceNonRouting) + "' " +
					" WHEN " + DT_DeviceIORouting + " THEN '" + GetDeviceRole(DT_DeviceIORouting) + "' " +										
					" WHEN " + DT_HartISAAdapter + " THEN '" + GetDeviceRole(DT_HartISAAdapter) + "' " +
					" WHEN " + DT_WirelessHartDevice + " THEN '" + GetDeviceRole(DT_WirelessHartDevice) + "' " +
					" ELSE 'Unknown' " +
				   " END " + sortOrder + ", Model " + sortOrder;
        case 4:
            return " DeviceTag " + sortOrder;            
        case 5:
            return " LastRead " + sortOrder;
        default:
            return " Address64 " + sortOrder;
    }
}
        
function GetRegisteredDeviceListByType(deviceType) {
    var	whereClause = "";

	if (deviceType != null) {		
		 whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " DeviceRole IN " + ConvertArrayToSqlliteParam(deviceType) + " ";
	}
    whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " DeviceStatus >= 1 "; // + DS_NormalOperationCommencing + " ";

	if (whereClause.length > 0) {
		whereClause = " WHERE " + whereClause;
	}

    var myQuery =   "   SELECT  DeviceID, " +
                    "           DeviceRole, " +
                    "           Address64, " +
                    "           Nickname, " +
                    "           DeviceStatus " +
					"   FROM    Devices "
					+   whereClause + 
					"   ORDER BY Address64 ";    
	var result;
    
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running get Registered DeviceList by type!");
        return;
    }  
    
    for (i=0;i<result.length; i++) {        
        result[i].DeviceID = result[i][0];
        result[i].DeviceRole = GetDeviceRole(result[i][1]);
        result[i].Address64 = result[i][2];
		result[i].Nickname = GetPadedHex(result[i][3]);        
    	result[i].DeviceStatus = result[i][4];
    }
    
    result.devices = result;
    return result;    
}	

/** Custom Icons */
function GetCustomIconsList(savedIcons) {
    var myQuery =   "   SELECT  DISTINCT C.Model, D.DeviceRole " +
					"   FROM    Devices D LEFT OUTER JOIN DevicesCodes C ON D.DeviceCode = C.DeviceCode " +
                    "   WHERE   C.Model NOT IN ('NULL', 'N/A')"; 
	var result;
    try  {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetCustomIconsPage to get models!");
        return;
    }  
    var icons = new Array();
    var j = 0;
    var iconFiles = GetCustomIconFiles();
    for (var i=0; i<result.length; i++) {
        var iconFile = GetIconFile(iconFiles, result[i][0], result[i][1]); 
        if (savedIcons) {
            if (iconFile != null) {
                var objIcon = new Object();
                objIcon.DeviceModel = result[i][0];
                objIcon.DeviceRole = result[i][1];
                objIcon.DeviceRoleName = GetDeviceRole(result[i][1]);
                //objIcon.ModelAscii = GetAsciiString(result[i][0]);
                objIcon.ModelAscii = result[i][0];
		        var image = new Image();
		        image.src = "styles/images/custom/" + getFileName(iconFile);
		        objIcon.Icon = "<img src='" + image.src + "' width='" + MAX_DEVICE_ICON_SIZE + "' height='" + MAX_DEVICE_ICON_SIZE + "' border='0'></img>";
                objIcon.DelAction = "<a href='javascript:DeleteIcon(\"" + iconFile + "\");'><img src='styles/images/delete.gif'></a>";
                objIcon.cellClass = (j % 2 == 0) ? "tableCell" : "tableCell2";
                icons[j++] = objIcon;
            }
        } else {
            if (iconFile == null) {
                var objIcon = new Object();
                objIcon.DeviceModel = result[i][0];
                objIcon.DeviceRole = result[i][1];
                objIcon.DeviceRoleName = GetDeviceRole(result[i][1]);
                //objIcon.ModelAscii = GetAsciiString(result[i][0]);
                objIcon.ModelAscii = result[i][0];
                objIcon.Icon = null;
                objIcon.DelAction = null;
                objIcon.File = null;
                objIcon.cellClass = null;
                icons[j++] = objIcon;
            }
        }
    }
    icons.customicons = icons;
    if (icons == null || icons.length == 0) {
        return null;
    } else {
        return icons;    
    }
}

function GetCustomIconFiles() {
	var result;
    try  {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.file.exists({file : WebsitePath + "styles/images/custom/*.*"});
    } catch(e) {
        HandleException(e, "Unexpected error running getting Icon Files!");
        return;
    } 
    return result;
}

function GetIconFile(iconsList, deviceModel, deviceRole) {
    if (iconsList == null) {
        return null;
    }
    var iconFileName = null;
    for (var i=0; i<iconsList.length; i++) {
        iconFileName = getFileName(iconsList[i]);
        if (iconFileName.split(".")[0] == deviceModel && iconFileName.split(".")[1] == deviceRole) {
            return iconsList[i];
        }
    }
}

function GetIconFileName(iconsList, deviceModel, deviceRole) {
    if (deviceModel != null && deviceModel != "" && deviceRole != null && deviceRole != "") {
        var modelIcon = GetIconFile(iconsList, deviceModel, deviceRole);
    	if (modelIcon != null) {
    		return "custom/" + deviceModel + "." + deviceRole + "." + modelIcon.split(".")[2] + "." + modelIcon.split(".")[3];
        }
    }
    return GetDeviceRoleImage(deviceRole);
}


function GetDeviceHealthInfo(deviceId) {	
 	var myQuery =	" SELECT	"+
					"		DH.Generated, " +
					"		NH.AllTx, " +
					"		NH.NoACK, " +
					"		DH.Terminated, " +
					"		NH.AllRX, " +
					"		DH.DLLFailures, " +
					"		DH.NLFailures, " +
					"		DH.CRCErrors, " +
					"		DH.NonceLost " +
					" FROM	ReportDeviceHealth DH "  +
					"		LEFT OUTER JOIN ( SELECT DeviceID, "+
											" 		 SUM(Transmissions) AS AllTx, " +
											" 		 SUM(FailedTransmissions) AS NoACK, " +
											" 		 SUM(Receptions) AS AllRx " +											
											" FROM ReportNeighborHealthList " +
											" GROUP BY DeviceID ) NH ON DH.DeviceID = NH.DeviceID " +
					" WHERE DH.DeviceID = " + deviceId	
	var result;		
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);    
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetDeviceHealthInfo");
        return null;
    }		    
  
    var healthData = new HealthData();	
    if (result.length != 0){  
    	healthData.Generated	= result[0][0] == "NULL" ? 0 : result[0][0];
		healthData.AllTx 		= result[0][1] == "NULL" ? 0 : result[0][1];
		healthData.NoACK 		= result[0][2] == "NULL" ? 0 : result[0][2];
		healthData.Terminated	= result[0][3] == "NULL" ? 0 : result[0][3];
		healthData.AllRx 		= result[0][4] == "NULL" ? 0 : result[0][4];
		healthData.DLLFailure 	= result[0][5] == "NULL" ? 0 : result[0][5];
		healthData.NLFailure 	= result[0][6] == "NULL" ? 0 : result[0][6];
		healthData.CRCError 	= result[0][7] == "NULL" ? 0 : result[0][7];
		healthData.NonceLost 	= result[0][8] == "NULL" ? 0 : result[0][8];
		return healthData;
    }  else {
    	return null;
    }  		
}

function GetDeviceCodesCount(deviceCode, deviceModel, deviceCompany){
	var whereClause = "";
	if (deviceCode != null &&  deviceCode != "" ){
		deviceCode = ReplaceSpecialCharacters(deviceCode);
		whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " DeviceCode LIKE '%" + deviceCode + "%' ESCAPE '\\' ";
	};	
	if (deviceModel != null &&  deviceModel != "" ){
		deviceModel = ReplaceSpecialCharacters(deviceModel);
		whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " Model LIKE '%" + deviceModel + "%' ESCAPE '\\' ";
	};	
	if (deviceCompany != null &&  deviceCompany != "" ){
		deviceCompany = ReplaceSpecialCharacters(deviceCompany);
		whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " Company LIKE '%" + deviceCompany + "%' ESCAPE '\\' ";
	};	
	if (whereClause != ""){
		whereClause = " WHERE " + whereClause;
	}
	var myQuery = " SELECT COUNT(*) FROM DevicesCodes " + whereClause;
	var result;		
	try {
	var service = new jsonrpc.ServiceProxy(serviceURL, methods);    
		result = service.sqldal.execute({query:myQuery});
	} catch(e) {
		HandleException(e, "Unexpected error running GetDeviceCodesCount");
		return null;
	}		    
	
	if (result != null){
		if (result.length != 0){
			return result[0][0];
		} else {
			return 0;
		}; 	
	} else {
		return 0;
	};
}

function GetDeviceCodesPage(pageSize, pageNo, rowCount, deviceCode, deviceModel, deviceCompany, sortBy, sortOrder){
	var rowOffset = 0;
    TotalPages = 0;
    
   	if (rowCount != 0) {
		var maxAvailablePageNo = Math.ceil(rowCount/pageSize);
		
		if (pageNo > maxAvailablePageNo) {
			pageNo = maxAvailablePageNo;
		}
        //global var
	    TotalPages = maxAvailablePageNo;
        rowOffset = (pageNo - 1) * pageSize;
    }
	
	var whereClause = "";
	if (deviceCode != null &&  deviceCode != "" ){
		deviceCode = ReplaceSpecialCharacters(deviceCode);
		whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " DeviceCode LIKE '%" + deviceCode + "%' ESCAPE '\\' ";
	};	
	if (deviceModel != null &&  deviceModel != "" ){
		deviceModel = ReplaceSpecialCharacters(deviceModel);
		whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " Model LIKE '%" + deviceModel + "%' ESCAPE '\\' ";
	};	
	if (deviceCompany != null &&  deviceCompany != "" ){
		deviceCompany = ReplaceSpecialCharacters(deviceCompany);
		whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " Company LIKE '%" + deviceCompany + "%' ESCAPE '\\' ";
	};	
	if (whereClause != ""){
		whereClause = " WHERE " + whereClause;
	}
	var myQuery = " SELECT DeviceCode, Model, Company FROM DevicesCodes " +
				  whereClause +
				  " ORDER BY " + GetDeviceCodeOrderByColumnName(sortBy, sortOrder) +
				  " LIMIT " + pageSize + " OFFSET " + rowOffset;
	var result;		
	try {
	var service = new jsonrpc.ServiceProxy(serviceURL, methods);    
		result = service.sqldal.execute({query:myQuery});
	} catch(e) {
		HandleException(e, "Unexpected error running GetDeviceCodesPage");
		return null;
	}		    
	
	if (result.length != 0){  
		for (var i=0; i<result.length; i++){
			result[i].DeviceCode = "<a href='javascript:EditDeviceCode(" + result[i][0] + ")'>"+result[i][0]+"</a>";
			result[i].Model = result[i][1];
			
			var maxlen = 40;
				if (result[i].Model.length > maxlen){					
					var tmp = result[i].Model;
					var model = "";
					while (tmp.length > 0){
						model = model + tmp.substr(0,maxlen) +"</br>";				
						tmp = tmp.substr(maxlen);
					};
					result[i].Model = model;
				};														
			result[i].Company = result[i][2];
				if (result[i].Company.length > maxlen){
					var tmp = result[i].Company;
					var company = "";
					while (tmp.length > 0){
						company = company + tmp.substr(0,maxlen) +"</br>";				
						tmp = tmp.substr(maxlen);
					};		
					result[i].Company = company;
				};					
			result[i].DeleteLink = "<a href='javascript:RemoveDeviceCode(" + result[i][0] + ");'><img src='styles/images/delete.gif'></a>";
			if (i % 2 == 0) {
				result[i].cellClass = "tableCell2";
		    } else  {
		    	result[i].cellClass = "tableCell";
		    };
		} 
		result.devicecodes = result;
		return result;
	}  else {
		return null;
	};  	
}


function GetDeviceCodeOrderByColumnName(sortBy, sortOrder){
	switch (sortBy) {
	case 1:
		return	" DeviceCode " + sortOrder;
	case 2:
		return	" UPPER(Model) " +	sortOrder;		
	case 3:
		return	" UPPER(Company) " + sortOrder;			
	default:
		return " DeviceCode ASC";
	}
}

function DeleteDeviceCode(code){
	if (confirm("Are you sure you want to delete the device code <" + code + "> ?")){ 	
		var myQuery = " BEGIN TRANSACTION; DELETE FROM DevicesCodes WHERE DeviceCode = "+code+" AND NOT EXISTS(SELECT * FROM Devices WHERE DeviceCode = "+code+" ); SELECT changes(); COMMIT;";
		var result;
		try {
			var service = new jsonrpc.ServiceProxy(serviceURL, methods);    
			result = service.sqldal.execute({mode : "write", query:myQuery});
		} catch(e) {
			HandleException(e, "Unexpected error running DeleteDeviceCode");
			return null;
		}	
		
		if (result.length !=0){
			if (result[0][0] == 0) {
				alert("The device code <"+code+"> cannot be deleted! \n There is at least one device with this code.")
				return 0;
			} else {
				return 1;
			}
		} 
	}
}

function AddDeviceCode(code, model, company){
	var checkDuplicate = " SELECT 1 FROM DevicesCodes WHERE DeviceCode = " + code;
	var myQuery = " BEGIN TRANSACTION; INSERT INTO DevicesCodes (DeviceCode, Model, Company) VALUES ("+code+", '"+model+"', '"+company+"'); SELECT Changes(); COMMIT; ";
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);    		
		var result0 = service.sqldal.execute({mode : "read", query:checkDuplicate});		
		if (result0.length !=0){
			if (result0[0][0] == 1) {
				alert("The device code <"+code+"> already exists!")
				return 0;
			}; 
		} 			
		var result1 = service.sqldal.execute({mode : "write", query:myQuery});
		if (result1.length !=0){
			if (result1[0][0] == 0) {
				alert("The device code <"+code+"> cannot be added!")
				return 0;
			} else {
				return 1;
			}
		} 
	} catch(e) {
		HandleException(e, "Unexpected error running AddDeviceCode");
		return null;
	}
}

function UpdateDeviceCode(code, model, company){
	var myQuery = " UPDATE DevicesCodes SET Model = '"+model+"', Company = '"+company+"' WHERE DeviceCode = "+code;		
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);    
		var result = service.sqldal.execute({mode : "write", query:myQuery});
		return 1; 
	} catch(e) {
		HandleException(e, "Unexpected error running UpdateDeviceCode");
		return null;
	}	
}

function GetDeviceCodeDetails(code){
	var myQuery = " SELECT Model, Company FROM DevicesCodes WHERE DeviceCode = " + code	;
	var result;		
	try {
	var service = new jsonrpc.ServiceProxy(serviceURL, methods);    
		result = service.sqldal.execute({query:myQuery});
	} catch(e) {
		HandleException(e, "Unexpected error running GetDeviceCodeDetails");
		return null;
	}
	if (result.length != 0){ 
		result[0].Model = result[0][0];
		result[0].Company = result[0][1];
		return result;
	} else {
		return null;
	} 		
}

function AddDeviceInDashboard(slotNumber, deviceId, channelNo, gaugeType, minValue, maxValue){				    		
	var myQuery =  " BEGIN TRANSACTION;" +
				   " UPDATE Dashboard SET SlotNumber = SlotNumber + 1 WHERE SlotNumber >= " + slotNumber + ";" +
				   " INSERT INTO Dashboard(SlotNumber, DeviceID, ChannelNo, GaugeType, MinValue, MaxValue)" +
				   " VALUES(" + slotNumber + ", " + deviceId + ", " + channelNo + ", " + gaugeType + ", " + minValue + ", " + maxValue + ");" +
				   " COMMIT; ";
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);
		service.sqldal.execute({mode:"write", query:myQuery});
	} catch(e) {
		HandleException(e, "Unexpected error running AddDeviceInDashboard !");
		return false;
	}	
	return true;	
}

function GetChannelsForDevice(deviceId) {
    var myQuery = " SELECT C.ChannelID, C.Name " +
                  " FROM Channels C " +
                  " LEFT OUTER JOIN Dashboard S ON C.ChannelID = S.ChannelNo " +               
                  " WHERE C.DeviceID = " + deviceId + " AND S.ChannelNo is null "	
    var result;     
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetChannelsForDevice !");
        return null;
    }    
    for (i=0;i<result.length; i++) {
    	result[i].ChannelNo 	= result[i][0];
    	result[i].ChannelName	= result[i][1];
    }    
    return result;

}

function GetDashboardHashKey(){
	var myQuery =  " SELECT SlotNumber, ChannelNo FROM Dashboard ORDER BY SlotNumber ";
	var result;
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);
		result = service.sqldal.execute({query:myQuery});
	} catch(e) {
		HandleException(e, "Unexpected error running GetDashboardHashKey !");
		return null;
	}	
	
	if (result != null) {
		var hashSlots = new Array();
		var hashChannels = new Array();
		for (var i=0; i< result.length; i++){
			hashSlots.push(result[i][0]);
			hashChannels.push(result[i][1]);
		}	
		return hashSlots.toString() + "," + hashChannels.toString();	
	} else {
		return null;
	}
}

function GetDevicesForDashboard(){				    		
	var myQuery = " SELECT D.SlotNumber, D.DeviceID, D.ChannelNo, D.GaugeType, D.MinValue, D.MaxValue, "+
				  		 " R.Address64, DC.Company, DC.Model, C.Name, strftime('%H'||':'||'%M'||':'||'%S',DR.ReadingTime), DR.Value, R.DeviceRole " +
				  "	FROM Dashboard D " +
				  		" INNER JOIN Channels C ON D.ChannelNo = C.ChannelID " +
				  		" INNER JOIN Devices R ON D.DeviceID = R.DeviceID " +
				  		" INNER JOIN DevicesCodes DC ON R.DeviceCode = DC.DeviceCode " +
				  		" LEFT OUTER JOIN Readings DR ON C.ChannelID = DR.ChannelID " +
				  "	ORDER BY SlotNumber ASC ";
	var result; 
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);
		result = service.sqldal.execute({mode:"write", query:myQuery});
	} catch(e) {
		HandleException(e, "Unexpected error running GetDevicesForDashboard !");
		return false;
	}			
	var devices = [];
	var icons = GetCustomIconFiles();
	for (var i=0; i<result.length; i++){
        var image = new Image();        
        image.src = "styles/images/" + GetIconFileName(icons, result[i][8], result[i][12]);
		devices[i] = {	SlotNumber: 	result[i][0],
						DeviceID:		result[i][1],
						ChannelNo:		result[i][2],
						GaugeType:		result[i][3],
						MinValue:		result[i][4],
						MaxValue:		result[i][5],
						Address64:		result[i][6],
						Manufacturer:	result[i][7],
						Model:			result[i][8],
						ChannelName:	result[i][9],
						ReadingTime:	result[i][10],
						Value:			Number(result[i][11]).toString(),
						DeviceRole:		result[i][12],
						Icon:			image.src,
						IconWidth: 		(image.width  > MAX_DEVICE_ICON_SIZE) ? MAX_DEVICE_ICON_SIZE : image.width,
						IconHeight:		(image.height > MAX_DEVICE_ICON_SIZE) ? MAX_DEVICE_ICON_SIZE : image.height}
	}			
	if (result.length != 0){
		return devices
	}  else {
		return null;
	}
}


function GetDeviceWithChannels() {
    var myQuery = " SELECT D.Address64, D.DeviceID " +
                  " FROM Channels C INNER JOIN Devices D ON C.DeviceID = D.DeviceID " +
                  " LEFT OUTER JOIN Dashboard S ON C.ChannelID = S.ChannelNo " +
                  " WHERE S.ChannelNo is null " +
                  " ORDER BY D.Address64 ";	
    var result;     
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetDeviceWithChannels !");
        return null;
    }    
    var arrElements = [];
    for (i = 0; i < result.length; i++) {
    	result[i].Address64 	= result[i][0];
    	result[i].DeviceID 		= result[i][1];
    	if (!ElementExists(result[i], arrElements)){
    		arrElements.push(result[i]);
    	};
    };    
    return arrElements;
}

function GetNoOfDevicesInDashboard(){
	var myQuery = " SELECT COUNT(*) FROM Dashboard ";
	var result;     
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);
		result = service.sqldal.execute({query:myQuery});
	} catch(e) {
		HandleException(e, "Unexpected error running GetNoOfDevicesInDashboard !");
		return null;
	}
	result.Count = result[0][0];
	return result;	
}


function AddDeviceInDashboard(slotNumber, deviceId, channelNo, gaugeType, minValue, maxValue){				    		
	var myQuery =  " BEGIN TRANSACTION;" +
				   " UPDATE Dashboard SET SlotNumber = SlotNumber + 1 WHERE SlotNumber >= " + slotNumber + ";" +
				   " INSERT INTO Dashboard(SlotNumber, DeviceID, ChannelNo, GaugeType, MinValue, MaxValue)" +
				   " VALUES(" + slotNumber + ", " + deviceId + ", " + channelNo + ", " + gaugeType + ", " + minValue + ", " + maxValue + ");" +
				   " COMMIT; ";
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);
		service.sqldal.execute({mode:"write", query:myQuery});
	} catch(e) {
		HandleException(e, "Unexpected error running AddDeviceInDashboard !");
		return false;
	}	
	return true;	
}


function RemoveDeviceFromDashboard(slotNumber){
	var myQuery = 	" BEGIN TRANSACTION; " +
					" DELETE FROM Dashboard WHERE SlotNumber = " + slotNumber + "; " +
					" UPDATE Dashboard SET SlotNumber = SlotNumber - 1 WHERE SlotNumber > " + slotNumber + ";" +
					" COMMIT; ";
	var result;
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);
		result = service.sqldal.execute({mode:"write",query:myQuery});
	} catch(e) {
		HandleException(e, "Unexpected error running RemoveDeviceFromDashboard !");
		return false;
	}	
	return true;
}


function GetMHPublishersConfigStatusCount(eui64, pubStatus) {
	var whereClause = "";
	var newCondition = "";
    if (eui64 != null && eui64 != "") {
    	newCondition = " D.Address64 LIKE '%"+eui64+"%' ";
    	whereClause += whereClause == "" ? newCondition : " AND " + newCondition;
    };
    if (pubStatus != null && pubStatus != "") {
    	newCondition = " P.State = '"+pubStatus+"' ";
    	whereClause += whereClause == "" ? newCondition : " AND " + newCondition;  
    };
    if (whereClause != ""){
    	whereClause = " WHERE " + whereClause;
    }
    
    var myQuery = " SELECT count(*)" +
                  " FROM DeviceSetPublishersLog P " +
                  " INNER JOIN Devices D ON P.DeviceID = D.DeviceID " +
                  whereClause;                  	
    var result;
   
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetMHPublishersConfigStatusCount!");
        return;
    }
	if (result.length != 0) {
		return result[0][0];
	} else {			
	    return null;
    }
	return null;
}


function GetMHPublishersConfigStatusPage(eui64, pubStatus, pageSize, pageNo, rowCount, orderBy, orderDir) {
	var rowOffset = 0;
    TotalPages = 0;
   	if (rowCount != 0) {
		var maxAvailablePageNo = Math.ceil(rowCount/pageSize);
		
		if (pageNo > maxAvailablePageNo) {
			pageNo = maxAvailablePageNo;
		}
        //global var
	    TotalPages = maxAvailablePageNo;
        rowOffset = (pageNo - 1) * pageSize;
    }
	
	var whereClause = "";
	var newCondition = "";
    if (eui64 != null && eui64 != "") {
    	newCondition = " D.Address64 LIKE '%"+eui64+"%' ";
    	whereClause += whereClause == "" ? newCondition : " AND " + newCondition;
    };
    if (pubStatus != null && pubStatus != "") {
    	newCondition = " P.State = '"+pubStatus+"' ";
    	whereClause += whereClause == "" ? newCondition : " AND " + newCondition;  
    };
    if (whereClause != ""){
    	whereClause = " WHERE " + whereClause;
    }

    function GetOrderByString(orderBy, orderDir){
    	if (orderBy == 1){
    		return " D.Address64 " + orderDir;
    	} else {
    		return " CASE P.State WHEN " +SETPUBLISHER_STATE_READ_BURSTCONFIG+ 		" THEN '" + GetPublisherStateName(SETPUBLISHER_STATE_READ_BURSTCONFIG) + "'" +
    							" WHEN " +SETPUBLISHER_STATE_TURNOFF_BURST+ 		" THEN '" + GetPublisherStateName(SETPUBLISHER_STATE_TURNOFF_BURST) + "'" +
    							" WHEN " +SETPUBLISHER_STATE_GET_SUBDEVICEINDEX+ 	" THEN '" + GetPublisherStateName(SETPUBLISHER_STATE_GET_SUBDEVICEINDEX) + "'" +
    							" WHEN " +SETPUBLISHER_STATE_CONFIGURE_BURST+ 		" THEN '" + GetPublisherStateName(SETPUBLISHER_STATE_CONFIGURE_BURST) + "'" +
    							" WHEN " +SETPUBLISHER_STATE_DONE+ 					" THEN '" + GetPublisherStateName(SETPUBLISHER_STATE_DONE) + "'" +
    							" ELSE 'UNKNOWN' " + 
    			   " END " + orderDir;    			   	
    	};
    };    
    
    var myQuery = " SELECT D.Address64, P.State, P.Error, P.Message " +
                  " FROM DeviceSetPublishersLog P " +
                  " INNER JOIN Devices D ON P.DeviceID = D.DeviceID " +
                  whereClause + 
                  " ORDER BY " + GetOrderByString(orderBy, orderDir) + 
                  " LIMIT " + pageSize + " OFFSET " + rowOffset;
    var result;
   
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetMHPublishersConfigStatusPage!");
        return;
    }
	if (result.length != 0) {
		for (var i=0; i<result.length; i++){
			result[i].Address64 = result[i][0];
			result[i].State = GetPublisherStateName(result[i][1]);
			result[i].Error = isNaN(result[i][2]) ? 
			                         ParseBurstError(result[i][2], result[i][3]) :  
									 GetPublisherErrorText(result[i][2]) + ((result[i][3] != null && result[i][3] != "") ? " (" + result[i][3] + ")" : "");
			if (i % 2 == 0) {
				result[i].cellClass = "tableCell";
		    } else {   
		    	result[i].cellClass = "tableCell2";
		    };
		};
	};
	result.publishers = result;
	return result;
}