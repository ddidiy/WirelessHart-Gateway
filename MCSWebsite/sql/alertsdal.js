var methods = ["sqldal.execute", "user.logout", "sqldal.getCsv", "user.isValidSession"];


function AppendWhereCondition(whereClause, whereCondition) {
	if (whereClause.length == 0) {
		whereClause = whereCondition;
	} else {			
		whereClause = whereClause + " AND " + whereCondition;
	}
	return whereClause;
}

function GetAlertsCount(deviceTag, eui64Address, alertType, StartTime, EndTime) {
	
	var whereClause = "";		
	if (deviceTag != null && deviceTag != "") {		
		whereClause = AppendWhereCondition(whereClause, " D.DeviceTag LIKE '%" +GetHexString(deviceTag) + "%' ");
	};
	if (eui64Address != null && eui64Address != "") {			
		eui64Address = ReplaceSpecialCharacters(eui64Address);
		whereClause = AppendWhereCondition(whereClause, " D.Address64 LIKE '%" + eui64Address + "%' ESCAPE '\\'");
	};
	if (alertType != null && alertType != "") {			
		whereClause = AppendWhereCondition(whereClause, " A.AlarmType = " + alertType + " ");
	};
	
    if (StartTime != null) {		
    	StartTime = TruncateSecondsFromDate(StartTime);    
	    whereClause = AppendWhereCondition(whereClause, " A.AlarmTime >= '" + ConvertFromJSDateToSQLiteDate(StartTime) + "' ");
    };
    if (EndTime != null) {		
    	EndTime = TruncateSecondsFromDate(AddSecondsToDate(EndTime, 60));
        whereClause = AppendWhereCondition(whereClause, " A.AlarmTime < '" + ConvertFromJSDateToSQLiteDate(EndTime) + "' ");
    };
	if (whereClause.length > 0) {
		whereClause = " WHERE " + whereClause + " ";
	};
		
	var myQuery = " SELECT COUNT(*) FROM Alarms A INNER JOIN Devices D ON A.DeviceID = D.DeviceID " + whereClause;
    var result;  
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetAlertsCount !");
        return null;
    }
	
	if (result.length != 0) {
		return result[0][0];		    
	} else {			
        return 0;
	}	
}

function GetAlertsPage(deviceTag, eui64Address, alertType, StartTime, EndTime, pageNo, pageSize, rowCount, forExport){
   	var rowOffset = 0;  
   	if (rowCount != 0 && forExport == false) {
		var maxAvailablePageNo = Math.ceil(rowCount/pageSize);
			
		if (pageNo > maxAvailablePageNo) {
			pageNo = maxAvailablePageNo;
		}
        //global var
	    TotalPages = maxAvailablePageNo;
        rowOffset = (pageNo - 1) * pageSize;
    }    	
   	
    var whereClause = "";
	if (deviceTag != null && deviceTag != "") {			
		whereClause = AppendWhereCondition(whereClause, " D.DeviceTag LIKE '%" + GetHexString(deviceTag) + "%' ");
	}
	if (eui64Address != null && eui64Address != "") {			
		eui64Address = ReplaceSpecialCharacters(eui64Address);
		whereClause = AppendWhereCondition(whereClause, " D.Address64 LIKE '%" + eui64Address + "%' ESCAPE '\\'");
	};
	if (alertType != null && alertType != "") {			
		whereClause = AppendWhereCondition(whereClause, " A.AlarmType = " + alertType + " ");
	};
	
    if (StartTime != null) {		
    	StartTime = TruncateSecondsFromDate(StartTime);    
	    whereClause = AppendWhereCondition(whereClause, " A.AlarmTime >= '" + ConvertFromJSDateToSQLiteDate(StartTime) + "' ");
    };
    if (EndTime != null) {		
    	EndTime = TruncateSecondsFromDate(AddSecondsToDate(EndTime, 60));
        whereClause = AppendWhereCondition(whereClause, " A.AlarmTime < '" + ConvertFromJSDateToSQLiteDate(EndTime) + "' ");
    };	
	if (whereClause.length > 0)	{
		whereClause = " WHERE " + whereClause + " ";
	};

    var myQuery =   " SELECT D.DeviceTag, D.Nickname, D.Address64, A.AlarmTime, " +
                    		" CASE AlarmType " +
                    			" WHEN " + AT_PathDown              + " THEN '" + GetAlertTypeName(AT_PathDown) + "' " +
                    			" WHEN " + AT_SourceRouteFailed     + " THEN '" + GetAlertTypeName(AT_SourceRouteFailed) + "' " +
                    			" WHEN " + AT_GraphRouteFailed   	+ " THEN '" + GetAlertTypeName(AT_GraphRouteFailed) + "' " +
                    			" WHEN " + AT_TransportLayerFailed  + " THEN '" + GetAlertTypeName(AT_TransportLayerFailed) + "' " +
                    			" ELSE 'UNKNOWN' " +
                    		" END AS AlertType, " +
                    		" CASE AlarmType " +
                    			" WHEN " + AT_GraphRouteFailed + " THEN A.PeerID_GraphID " +
                    			" ELSE P.Address64 " +
                    		" END AS PeerAddressGraphID, " +	
                    		" A.MIC " + 
				    " FROM Alarms A " + 
				    " INNER JOIN Devices D ON A.DeviceID = D.DeviceID " +
				    " LEFT OUTER JOIN Devices P ON A.PeerID_GraphID = P.DeviceID "
				    + whereClause  +
				    " ORDER BY A.AlarmTime DESC " +
				    " LIMIT " + pageSize + " OFFSET " + rowOffset;
    if (forExport) {
        myQuery = " SELECT 'DeviceTag', 'Nickname', 'EUI-64 Address', 'Alert Time', 'Alert Type', 'PeerAddress/GraphID', 'MIC' UNION ALL " + myQuery;
        return myQuery;        
    }
	var result;
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetAlertsPage !");
        return null;
    }
		
	if (result.length != 0) {	    
	    for (i=0;i<result.length;i++) {
	    	result[i].DeviceTag 	= GetAsciiString(result[i][0]) != "" ? GetAsciiString(result[i][0]) : NAString;
	    	result[i].DeviceTag		= result[i].DeviceTag.replace(/ /g,"&nbsp;");	    	
	    	result[i].Nickname 		= GetPadedHex(result[i][1]);
	    	result[i].EUI64Address	= result[i][2];
	    	result[i].AlertTime 	= result[i][3];
	    	result[i].AlertType 	= result[i][4];
	    	result[i].PeerAddressGraphID = result[i][5]; 
	    	result[i].MIC 			= result[i][6] != "NULL" ? result[i][6] : NAString;	    	
			if (i % 2 == 0) {
				result[i].cellClass = "tableCell2";
            } else {   
            	result[i].cellClass = "tableCell";
            }
		}		
	}
	result.alerts = result;
	return result;
}
