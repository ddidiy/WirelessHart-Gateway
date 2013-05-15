
var methods = ["sqldal.execute", "user.logout"];

function GetNeighborsForScheduleReportLinksPage(deviceId, superframeId){  
 	var myQuery =	" SELECT DISTINCT Nickname, PeerID " +
					" FROM DeviceScheduleLinks L " +
					" LEFT OUTER JOIN Devices D ON L.PeerID = D.DeviceID " + 
					" WHERE	L.DeviceID = " + deviceId + " AND L.SuperframeID = " + superframeId;
    var result;    
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);   
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetNeighborsForScheduleReportLinksPage");
        return null;
    }    
	if (result.length != 0) {
		for (i=0;i<result.length;i++) {
			result[i].Nickname = result[i][0] != "NULL" ? GetPadedHex(result[i][0]) : "FFFF";
			result[i].PeerID = result[i][1];
		}
		return result;
	} else {
		return null;
	}
}

function GetNeighborsHealth(deviceId, pageSize, pageNo, rowCount, sortType, sortOrder, neighbor, startTime, endTime) {    
	var rowOffset = 0;
    TotalPages = 0;
    
   	if (rowCount != 0) {
		var maxAvailablePageNo = Math.ceil(rowCount/pageSize);	
		if (pageNo > maxAvailablePageNo) {
			pageNo = maxAvailablePageNo;
		}
	    TotalPages = maxAvailablePageNo;		
        rowOffset = (pageNo - 1) * pageSize;
    }
	var whereClause = " WHERE N.DeviceID = " + deviceId;
    if (startTime != null) {		
    	startTime = TruncateSecondsFromDate(startTime);    
	    whereClause = whereClause + " AND Timestamp >= '" + ConvertFromJSDateToSQLiteDate(startTime) + "' ";
    }
    if (endTime != null) {		
    	endTime = TruncateSecondsFromDate(AddSecondsToDate(endTime, 60));
        whereClause = whereClause + " AND Timestamp < '" + ConvertFromJSDateToSQLiteDate(endTime) + "' ";
    }
	if (neighbor != "") {
		neighbor = ReplaceSpecialCharacters(neighbor);
		whereClause = whereClause + " AND D.Address64 LIKE '%" + neighbor + "%' ESCAPE '\\'";
	}	
   	
    var myQuery =	"	SELECT	D.Address64, " +
					"			N.TimeStamp, " +
					"			N.ClockSource, " +
					"			N.Transmissions, " +
					"			N.FailedTransmissions, " +
					"			N.Receptions, " + 
					"			N.RSL " +
					"	FROM	ReportNeighborHealthList N " +
					"			INNER JOIN Devices D ON N.PeerID = D.DeviceID " 
					+ whereClause +
					"     ORDER BY " + GetOrderByColumnNameNeighborsHealth(sortType) + " " + sortOrder + 
					"	  LIMIT " + pageSize + " OFFSET " + rowOffset;
    var result;
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);   
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetNeighborsHealth");
        return null;
    }    
	if (result.length != 0) {
		for (i=0;i<result.length;i++) {
			result[i].NeighborAddress64 = result[i][0];
			result[i].TimeStamp = result[i][1];            
			result[i].Flags = result[i][2] == 1 ? "yes" : "no"
			result[i].Transmitted = IfNullStr(result[i][3]);
			result[i].Failed = IfNullStr(result[i][4]);
			result[i].Received = IfNullStr(result[i][5]);		
			result[i].SignalLevel = " <span id=\"spnSignalQuality\" style=\"color:" + GetSignalQualityColor(result[i][6]) + "\">" + GetSignalQuality(result[i][6]) + "</span> (" + result[i][6] + ")";
		    if (i % 2 == 0) {
                result[i].cellClass = "tableCell";
            } else {   
                result[i].cellClass = "tableCell2";
            }
		}
		result.neighborshealth = result;
		return result;
	} else {
		return null;
	}
}

function GetNeighborsHealthCount(deviceId, neighbor, startTime, endTime) {
	var whereClause = " WHERE N.DeviceID = " + deviceId;
    if (startTime != null) {		
    	startTime = TruncateSecondsFromDate(startTime);    
	    whereClause = whereClause + " AND Timestamp >= '" + ConvertFromJSDateToSQLiteDate(startTime) + "' ";
    }
    if (endTime != null) {		
    	endTime = TruncateSecondsFromDate(AddSecondsToDate(endTime, 60));
        whereClause = whereClause + " AND Timestamp < '" + ConvertFromJSDateToSQLiteDate(endTime) + "' ";
    }
	if (neighbor != "") {
		neighbor = ReplaceSpecialCharacters(neighbor);
		whereClause = whereClause + " AND D.Address64 LIKE '%" + neighbor + "%' ESCAPE '\\'";
	}		
	var myQuery = " SELECT COUNT(*) " +
				  " FROM ReportNeighborHealthList N " +
				  	   " INNER JOIN Devices D ON N.PeerID = D.DeviceID "
				  + whereClause;
    var result; 
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);   
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetNeighborsHealthCount");
        return null;
    }    
	if (result.length != 0) {
		return result[0][0];
	} else {
		return 0;
	}
}

function GetDeviceScheduleReportPage(deviceId, pageSize, pageNo, rowCount) {	
    var rowOffset = 0;
    TotalPages = 0;
    
   	if (rowCount != 0) {
		var maxAvailablePageNo = Math.ceil(rowCount/pageSize);	
		if (pageNo > maxAvailablePageNo) {
			pageNo = maxAvailablePageNo;
		}
	    TotalPages = maxAvailablePageNo;		
        rowOffset = (pageNo - 1) * pageSize;
    }        
	var myQuery = "	SELECT S.DeviceID, " +
				  		 " S.SuperframeID, " +
						 " S.NumberOfTimeSlots, " +
						 " S.Active, " +
						 " S.HandheldSuperframe, " +
						 " L.NoOfLinks " +
				  "	FROM Superframes S " +
				  	   " LEFT OUTER JOIN (SELECT SuperframeID, DeviceID, COUNT(*) AS NoOfLinks FROM DeviceScheduleLinks GROUP BY SuperframeID, DeviceID) L ON S.SuperframeID = L.SuperframeID AND S.DeviceID = L.DeviceID " +
				  "	WHERE S.DeviceID = " + deviceId +
				  "	LIMIT " + pageSize + " OFFSET " + rowOffset;
    var result;    
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);   
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetDeviceScheduleReportPage");
        return null;
    }    

	if (result.length != 0)	{
		for (var i=0;i<result.length;i++) {
			result[i].DeviceID = result[i][0];
			result[i].SuperframeID = result[i][1];
			result[i].NumberOfTimeSlots = result[i][2];
			result[i].Active = (result[i][3] == 1) ? "Yes" : "No";
			result[i].HandheldSuperframe = (result[i][4] == 1) ? "Yes" : "No";
			if (result[i][5] != 'NULL'){ 
				result[i].NoOfLinks = "<a href='schedulereportlinks.html?deviceId=" + result[i].DeviceID + "&superframeId=" + result[i].SuperframeID + "'>" + result[i][5] + "</a>";
			} else {
				result[i].NoOfLinks = 0;
			} 
		    if (i % 2 == 0) {
                result[i].cellClass = "tableCell";
            } else {   
                result[i].cellClass = "tableCell2";
            }
		}
		result.schedulereport = result;
		return result;
	} else {
		return null;
	}
}

function GetDeviceScheduleReportCount(deviceId) {	
	var myQuery = "	SELECT COUNT(*) " +
				  "	FROM Superframes " +
				  " WHERE DeviceID = " + deviceId ;
    var result;    
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);   
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetDeviceScheduleReportCount");
        return null;
    }    
	if (result.length != 0) {		
		return result[0][0];
	} else {
		return 0;
	}
}


function GetDeviceScheduleReportLinksPage(deviceId, superframeId, peerID, linkType, pageSize, pageNo, rowCount) {
	var rowOffset = 0;
    TotalPages = 0;
    
   	if (rowCount != 0) {
		var maxAvailablePageNo = Math.ceil(rowCount/pageSize);	
		if (pageNo > maxAvailablePageNo) {
			pageNo = maxAvailablePageNo;
		}
	    TotalPages = maxAvailablePageNo;		
        rowOffset = (pageNo - 1) * pageSize;
    }
        
    var whereClause = "	WHERE	L.DeviceID = " + deviceId + " AND L.SuperframeID = " + superframeId;
    
    if (peerID != null) {
		whereClause = whereClause + " AND L.PeerID = " + peerID;
    }
    if (linkType != null) {
		whereClause = whereClause + " AND L.LinkType = " + linkType + " ";
    } 
 	var myQuery =	"	SELECT	D.Nickname, " +
					"			L.SlotIndex, " +
					"			L.ChannelOffset, " +
					"			L.Transmit, " +
					"			L.Receive, " +
					"			L.Shared, " +
					"			L.LinkType " +
					"	FROM	DeviceScheduleLinks L " +
					"			LEFT OUTER JOIN Devices D ON L.PeerID = D.DeviceID " 
					+	whereClause +
					"	ORDER BY L.SlotIndex ASC " +
					"	LIMIT " + pageSize + " OFFSET " + rowOffset;

    var result;    
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);   
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetDeviceScheduleReportLinksPage");
        return null;
    }    
	if (result.length != 0) {
		for (i=0;i<result.length;i++) {			
			result[i].Nickname = result[i][0] != "NULL" ? GetPadedHex(result[i][0]) : "FFFF";
			result[i].SlotIndex = result[i][1];
			result[i].ChannelOffset = result[i][2];			
			result[i].Transmit = (result[i][3] == 1) ? "Yes" : "No";
			result[i].Receive = (result[i][4] == 1) ? "Yes" : "No";
			result[i].Shared = (result[i][5] == 1) ? "Yes" : "No";
			result[i].LinkType = GetLinkTypeName(result[i][6]);

		    if (i % 2 == 0) {
                result[i].cellClass = "tableCell";
            } else {   
                result[i].cellClass = "tableCell2";
            }
		}
		result.schedulereportlinks = result;
		return result;
	} else {
		return null;
	}
}


function GetDeviceScheduleReportLinksCount(deviceId, superframeId, peerID, linkType) {          
	var whereClause = "	WHERE	L.DeviceID = " + deviceId + " AND L.SuperframeID = " + superframeId;   
    if (peerID != null) {
		whereClause = whereClause + " AND L.PeerID = " + peerID;
    }
    if (linkType != null) {
		whereClause = whereClause + " AND L.LinkType = " + linkType + " ";
    } 
	var myQuery =	"	SELECT	COUNT(*) " +
					"	FROM	DeviceScheduleLinks L " +
					"			LEFT OUTER JOIN Devices D ON L.PeerID = D.DeviceID " 
					+	whereClause ;
    var result;    
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);   
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetDeviceScheduleReportLinksCount");
        return null;
    }    
	if (result.length != 0) {
		return result[0][0];
	} else {
		return null;
	}
}

function GetNetworkHealthReportHeader(showDevices) {
	var whereClause = " WHERE D.DeviceRole NOT IN (" + DT_NetworkManager + ", " + DT_Gateway + ") ";
	if (showDevices == 0){
		whereClause = whereClause + " AND D.DeviceStatus >= " + DS_NormalOperationCommencing + " ";
	} else if (showDevices==1){
		whereClause = whereClause + " AND D.DeviceStatus < " + DS_NormalOperationCommencing + " ";
	}
	var myQuery1 = 
		  " SELECT "
		+	" COUNT(D.DeviceID) AS DeviceCount, " 
		+	" SUM(D.RejoinCount) AS JoinCount, " 			  			 					 		
		+	" datetime('now') AS CurrentDate, " 				  	
		+	" SUM(DH.Generated) AS Generated, " 
		+	" NH.AllTx, " 
		+	" NH.NoACK, " 
		+	" NH.AllRx, "				
		+	" SUM(DH.Terminated) AS Terminated, " 
		+	" SUM(DH.DLLFailures) AS DLLFailures, " 
		+	" SUM(DH.NLFailures) AS NLFailures, " 
		+	" SUM(DH.CRCErrors) AS CRCError, " 
		+	" SUM(DH.NonceLost) AS NonceLost " 
		+ " FROM "
		+	" Devices D "
		+ 	" LEFT OUTER JOIN ReportDeviceHealth DH ON D.DeviceID = DH.DeviceID "
		+ 	" LEFT OUTER JOIN (SELECT SUM(R.Transmissions) AS AllTx, "
						   +		" SUM(R.FailedTransmissions) AS NoACK, "
						   +		" SUM(R.Receptions) AS AllRx "
						   + " FROM Devices D "
						   + 	  " LEFT OUTER JOIN ReportNeighborHealthList R ON D.DeviceID = R.DeviceID "
						   + whereClause + ") NH" 			 		
		+ whereClause;
	
	var result1;		
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);    
       result1 = service.sqldal.execute({query:myQuery1});
    } catch(e) {
        HandleException(e, "Unexpected error running GetNetworkHealthReportHeader");
        return null;
    }		    
    
    var myQuery2 = " SELECT min(Timestamp) FROM DeviceHistory H INNER JOIN Devices D ON H.DeviceID = D.DeviceID AND D.DeviceRole = "+DT_NetworkManager+" AND H.DeviceStatus = "+DS_NormalOperationCommencing;
	var result2;		
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);    
       result2 = service.sqldal.execute({query:myQuery2});
    } catch(e) {
        HandleException(e, "Unexpected error running GetNetworkHealthReportHeader");
        return null;
    }		    

    var net = new Array();        
    net.DeviceCount	= result1[0][0] == "NULL" ? 0 : result1[0][0];
    net.JoinCount	= result1[0][1] == "NULL" ? 0 : result1[0][1];
    net.StartDate 	= IfNullStr(result2[0][0]);	
    net.CurrentDate	= IfNullStr(result1[0][2]);
    net.Generated 	= result1[0][3] == "NULL" ? 0 : result1[0][3];
    net.AllTx 		= result1[0][4] == "NULL" ? 0 : result1[0][4];
    net.NoACK 		= result1[0][5] == "NULL" ? 0 : result1[0][5];
    net.AllRx 		= result1[0][6] == "NULL" ? 0 : result1[0][6];
    net.Terminated 	= result1[0][7] == "NULL" ? 0 : result1[0][7];
    net.DLLFailures	= result1[0][8] == "NULL" ? 0 : result1[0][8];
    net.NLFailures 	= result1[0][9] == "NULL" ? 0 : result1[0][9];
    net.CRCError 	= result1[0][10] == "NULL" ? 0 : result1[0][10];
    net.NonceLost 	= result1[0][11] == "NULL" ? 0 : result1[0][11];
    return net;
}

function GetNetworkHealthReportPageCount(showDevices) {
	var whereClause = " WHERE DeviceRole NOT IN (" + DT_NetworkManager + ", " + DT_Gateway + ") ";
	if (showDevices == 0){
		whereClause = whereClause + " AND DeviceStatus >= " + DS_NormalOperationCommencing + " ";
	} else if (showDevices==1){
		whereClause = whereClause + " AND DeviceStatus < " + DS_NormalOperationCommencing + " ";
	}
		
	var myQuery = "SELECT COUNT(*) FROM Devices " + whereClause;					
    var result;    
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);   
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetNetworkHealthReportPageCount");
        return null;
    }    
	if (result.length != 0) {
		return result[0][0];
	} else {
		return 0;
	}
}

function GetNetworkHealthReportPage(pageSize, pageNo, rowCount, showDevices) {	
    var rowOffset = 0;
    TotalPages = 0;
    
   	if (rowCount != 0) {
		var maxAvailablePageNo = Math.ceil(rowCount / pageSize);	
		if (pageNo > maxAvailablePageNo) {
			pageNo = maxAvailablePageNo;
		}
	    TotalPages = maxAvailablePageNo;		
        rowOffset = (pageNo - 1) * pageSize;
    }
   	var whereClause = " WHERE D.DeviceRole NOT IN (" + DT_NetworkManager + ", " + DT_Gateway + ") ";
	if (showDevices == 0){
		whereClause = whereClause + " AND D.DeviceStatus >= " + DS_NormalOperationCommencing + " ";
	} else if (showDevices==1){
		whereClause = whereClause + " AND D.DeviceStatus < " + DS_NormalOperationCommencing + " ";
	}
	
	var myQuery =    " SELECT D.Address64, "
					+ 	"	D.RejoinCount, "
					+	"	DH.PowerStatus, "
					+	"	DH.Generated, "										
					+	"	DH.Terminated, "					
					+	"	DH.DLLFailures, "
					+	"	DH.NLFailures, "
					+	"	DH.CRCErrors, "
					+	"	DH.NonceLost "
				    +" FROM Devices D"
					+	"	LEFT OUTER JOIN ReportDeviceHealth DH ON D.DeviceID = DH.DeviceID "
				    + whereClause							
				    + " ORDER BY D.Address64 ASC "
                    + " LIMIT " + pageSize + " OFFSET " + rowOffset;
	var result;		
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);    
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetNetworkHealthReportPage");
        return null;
    }		    
			    	
    if (result.length != 0)	{
		for(var i=0;i<result.length;i++) {
			result[i].Address64 	= result[i][0] == "NULL" ? 0 : result[i][0];
			result[i].JoinCount 	= result[i][1] == "NULL" ? 0 : result[i][1];
			result[i].PowerStatus 	= result[i][2] == "NULL" ? 0 : result[i][2];			
			result[i].Generated 	= result[i][3] == "NULL" ? 0 : result[i][3];
			result[i].Terminated 	= result[i][4] == "NULL" ? 0 : result[i][4];
			result[i].DLLFailure 	= result[i][5] == "NULL" ? 0 : result[i][5];
			result[i].NLFailure 	= result[i][6] == "NULL" ? 0 : result[i][6];
			result[i].CRCError 		= result[i][7] == "NULL" ? 0 : result[i][7];
			result[i].NonceLost 	= result[i][8] == "NULL" ? 0 : result[i][8];			
		    if (i % 2 == 0) {
                result[i].cellClass = "tableCell";
            } else {   
                result[i].cellClass = "tableCell2";
            }
		}
		result.networkhealthdevices = result;
		return result;
    } else {
		return null;
    }

    var myQuery2 =	" SELECT D.DeviceID, "
		+	"		SUM(N.Transmissions) AS AllTx, "
		+	"		SUM(N.FailedTransmissions) AS NoACK, "
		+	" 		SUM(N.Receptions) AS AllRx "											
		+	" FROM  Devices D "
		+	"		INNER JOIN ReportNeighborHealthList N " 
		+ whereClause
		+ " GROUP BY D.DeviceID "

	var result2;		
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);    
		result2 = service.sqldal.execute({query:myQuery2});
	} catch(e) {
		HandleException(e, "Unexpected error running GetNetworkHealthReportPage");
		return null;
	}    

	for(var i=0; i<result2.length; i++){
		resukt
	}
}

function GetOrderByColumnNameNeighborsHealth(sortType) {
	switch (sortType) {
   		case 1:
			return	" Address64 ";
		case 2:
			return	" Timestamp ";
  		default:
			return	" Address64 ";
	}
}

function GetNeighborList(deviceId){
	var myQuery =	" SELECT DISTINCT D.Address64, D.DeviceID " +
					" FROM 	ReportNeighborHealthList N INNER JOIN Devices D ON N.PeerID = D.DeviceID " +
					" WHERE N.DeviceID = " + deviceId;
	var result; 
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);   
		result = service.sqldal.execute({query:myQuery});
	} catch(e) {
		HandleException(e, "Unexpected error running GetNeighborList");
		return null;
	}    
	
	for (var i=0; i<result.length; i++ ){
		result[i].NeighborAddress = result[i][0]; 
		result[i].DeviceID = result[i][1];
	}
	return result;
}
