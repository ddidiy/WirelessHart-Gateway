var methods = ["sqldal.execute", "user.logout"];

function AppendWhereCondition(whereClause, whereCondition) {
	if (whereClause.length == 0) {
		whereClause = whereCondition;
	} else {			
		whereClause = whereClause + " AND " + whereCondition;
	}
	return whereClause;
}


function GetReadingsCount(deviceId, CommandNo, Name, DeviceVariable, deviceAddress64, showDevices) {	
	
	if (deviceId != null && deviceId.length == 0) {
		deviceId = null;
	}
	if (CommandNo != null && CommandNo.length == 0)	{
		CommandNo = null;
	}
	if (Name != null && Name.length == 0)	{
		Name = null;
	}
	if (DeviceVariable != null && DeviceVariable.length == 0)	{
		DeviceVariable = null;
	}
	if (deviceAddress64 != null && deviceAddress64.length == 0)	{
		deviceAddress64 = null;
	}
	
	
    /// work around for MH
    var whereClause = " WHERE R.ReadingTime > '1970-01-01 00:00:00' AND BC.LastUpdate > '1970-01-01 00:00:00' ";

	if (deviceId != null) {			
		whereClause = whereClause + " AND (D.DeviceID = " + deviceId + ")";
	}
	if (CommandNo != null) {			
		whereClause = whereClause + " AND (C.CmdNo = '" + CommandNo + "')";
	}
	if (Name != null) {			
		Name = ReplaceSpecialCharacters(Name);
		whereClause = whereClause + " AND (C.Name LIKE '%" + Name + "%' ESCAPE '\\')";
	}
	if (DeviceVariable != null) {			
		DeviceVariable = ReplaceSpecialCharacters(DeviceVariable);
		whereClause = whereClause + " AND (C.DeviceVariable LIKE '%" + DeviceVariable + "%' ESCAPE '\\')";
	}
	if (deviceAddress64 != null) {			
		deviceAddress64 = ReplaceSpecialCharacters(deviceAddress64);
		whereClause = whereClause + " AND (D.Address64 LIKE '%" + deviceAddress64 + "%' ESCAPE '\\')";
	}
	if (showDevices==0){
		whereClause = whereClause + " AND (D.DeviceStatus >= " + DS_NormalOperationCommencing + ") ";
	} else if (showDevices==1){
		whereClause = whereClause + " AND (D.DeviceStatus < " + DS_NormalOperationCommencing + ") ";
	}
	
	var myQuery =   " SELECT COUNT(*) " +
				    " FROM 	Readings R " +
				    " 		INNER JOIN Channels C ON R.ChannelID = C.ChannelID " +
				    " 		INNER JOIN Devices D ON C.DeviceID = D.DeviceID " +
				    " 		INNER JOIN BurstCounters BC ON C.DeviceID = BC.DeviceID AND C.BurstMessage = BC.BurstMessage AND C.CmdNo = BC.CommandNumber " +
				    " 		LEFT OUTER JOIN BurstMessages BM ON C.DeviceID = BM.DeviceID AND C.BurstMessage = BM.BurstMessage AND C.CmdNo = BM.CommandNumber " +				    
					whereClause;    
    var result;  
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetReadingsCount !");
        return null;
    }
	
	if (result.length != 0) {    	
		return result[0][0];
	} else {			
        return 0;
	}	
}
 
function GetReadings(deviceId, CommandNo, Name, DeviceVariable, pageNo, pageSize, rowCount, forExport, deviceAddress64, showDevices) {
    var rowOffset = 0;
   	if (rowCount != 0  && forExport == false) {
		var maxAvailablePageNo = Math.ceil(rowCount/pageSize);

		if (pageNo > maxAvailablePageNo) {
			pageNo = maxAvailablePageNo;
		}
        //global var
	    TotalPages = maxAvailablePageNo;
        rowOffset = (pageNo - 1) * pageSize;
    }
       
	if (deviceId != null && deviceId.length == 0) {
		deviceId = null;
	}
	if (CommandNo != null && CommandNo.length == 0)	{
		CommandNo = null;
	}
	if (Name != null && Name.length == 0)	{
		Name = null;
	}
	if (DeviceVariable != null && DeviceVariable.length == 0)	{
		DeviceVariable = null;
	}
	if (deviceAddress64 != null && deviceAddress64.length == 0)	{
		deviceAddress64 = null;
	}


    /// work around for MH
    var whereClause = " WHERE R.ReadingTime > '1970-01-01 00:00:00' AND BC.LastUpdate > '1970-01-01 00:00:00' ";
	
	if (deviceId != null) {			
		whereClause = whereClause + " AND (D.DeviceID = " + deviceId + ")";
	}
	if (CommandNo != null) {			
		whereClause = whereClause + " AND (C.CmdNo = '" + CommandNo + "')";
	}
	if (Name != null) {			
		Name = ReplaceSpecialCharacters(Name);
		whereClause = whereClause + " AND (C.Name LIKE '%" + Name + "%' ESCAPE '\\')";
	}
	if (DeviceVariable != null) {			
		DeviceVariable = ReplaceSpecialCharacters(DeviceVariable);
		whereClause = whereClause + " AND (C.DeviceVariable LIKE '%" + DeviceVariable + "%' ESCAPE '\\')";
	}
	if (deviceAddress64 != null) {			
		deviceAddress64 = ReplaceSpecialCharacters(deviceAddress64);
		whereClause = whereClause + " AND (D.Address64 LIKE '%" + deviceAddress64 + "%' ESCAPE '\\')";
	}
	if (showDevices==0){
		whereClause = whereClause + " AND (D.DeviceStatus >= " + DS_NormalOperationCommencing + ") ";
	} else if (showDevices==1){
		whereClause = whereClause + " AND (D.DeviceStatus < " + DS_NormalOperationCommencing + ") ";
	}
	
    var myQuery =   " SELECT D.Address64, R.ReadingTime, C.Name, C.CmdNo, C.DeviceVariable, R.Value, C.Classification, C.UnitCode, " +
    				" 		 BM.UpdatePeriod, BC.LastUpdate, BC.Received, BC.Missed, R.ValueType " +
				    " FROM 	Readings R " +
				    " 		INNER JOIN Channels C ON R.ChannelID = C.ChannelID " +
				    " 		INNER JOIN Devices D ON C.DeviceID = D.DeviceID " +
				    " 		INNER JOIN BurstCounters BC ON C.DeviceID = BC.DeviceID AND C.BurstMessage = BC.BurstMessage AND C.CmdNo = BC.CommandNumber " +
				    " 		LEFT OUTER JOIN BurstMessages BM ON C.DeviceID = BM.DeviceID AND C.BurstMessage = BM.BurstMessage AND C.CmdNo = BM.CommandNumber " +				    
				    whereClause  +
				    " ORDER BY 2 DESC " +
				    " LIMIT " + pageSize + " OFFSET " + rowOffset;
    if (forExport) {
        myQuery = " SELECT 'EUI-64 Address', 'ReadingTime', 'Name', 'Cmd No ', 'Device Variable', 'Value', 'Classification', 'Unit Code', 'Update Period', 'Last Update', 'Received', 'Missed' UNION ALL " + myQuery;
        return myQuery;
    }
 
	var result;
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetReadings !");
        return null;
    }
	
	var readings = new Array();		
	if (result.length != 0) {
	    for (i=0;i<result.length;i++) {
			var reading = new Reading();
			reading.Address64 = result[i][0];	
			reading.ReadingTime = result[i][1];
			reading.ChannelName = result[i][2];
    		reading.Command = result[i][3];
    		reading.DeviceVariable = result[i][4];
    		reading.Value = ((result[i][12] == 0) ? "" + result[i][5] : ((result[i][12] == 1) ? "Infinity" : "NaN" ));
    		reading.Classification = "<span id='Classification' title='" + GetClassificationName(result[i][6])+ "'>" + result[i][6] + "</a>";       		
    		reading.UnitCode = "<span id='UnitCode' title='" + GetUnitCodeName(result[i][7])+ "'>" + result[i][7] + "</a>";
    		reading.UpdatePeriod = result[i][8];
    		reading.LastUpdate = result[i][9];
    		reading.Received = result[i][10];
    		reading.Missed = result[i][11];			
    		reading.HeightSpacer = "<img src='styles/images/pixel.gif' height='31px' width='1px' border='0'>";
			if (i % 2 == 0) {
                reading.cellClass = "tableCell2";
            } else {   
                reading.cellClass = "tableCell";
            }
			readings[i] = reading;
		}		
	}
	readings.readings = readings;
	return readings;	
}
