var methods = ["sqldal.execute", "user.logout", "sqldal.open", "sqldal.close", "user.isValidSession"];

function GetCommandsCount(startDate, endDate, deviceId, commandStatus, commandTypeCode, showSystemCommands) {
    var whereClause = ""; 

    if (deviceId != null) {     
        whereClause = whereClause + (whereClause == "" ? " " : " AND ") +  " ((d.DeviceID = " + deviceId + ")) ";
    }
    if (commandStatus != null) {        
         whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " c.CommandStatus = '" + commandStatus + "' ";
    }
    if (commandTypeCode != null) {      
        whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " c.CommandCode IN " + ConvertArrayToSqlliteParam(commandTypeCode) + " ";   
    }
    if (!showSystemCommands) {
        whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " c.Generated = 0 ";
    }
    if (whereClause.length > 0) {
        whereClause = " WHERE " + whereClause + " ";
    }            
    var myQuery =   "   SELECT  COUNT(DISTINCT c.CommandID) " +
                    "   FROM    Commands c " +
					"           INNER JOIN CommandSet cs ON c.CommandCode= cs.CommandCode " +
                    "           INNER JOIN Devices d ON c.DeviceID = d.DeviceID " +
                    "           LEFT OUTER JOIN CommandParameters cp on c.CommandID = cp.CommandID " +
                    whereClause;
    var result;
	try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetCommandsCount !");
        return 0;
    }
    if (result.length == 0) {
        return 0;
    } 
    else {  
        return result[0][0];
    }   
}


function GetCommandsPage(startDate, endDate, deviceId, commandStatus, commandTypeCodeArr, showSystemCommands, pageNo, pageSize, rowCount, forExport) {
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
    if (startDate != null) {		
        startDate = TruncateSecondsFromDate(startDate);    
	    whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " TimePosted >= '" + ConvertFromJSDateToSQLiteDate(startDate) + "' ";
    }
    if (endDate != null) {		
        endDate = TruncateSecondsFromDate(AddSecondsToDate(endDate, 60));
        whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " TimePosted < '" + ConvertFromJSDateToSQLiteDate(endDate) + "' ";
    }
    if (deviceId != null) {	
        whereClause = whereClause + (whereClause == "" ? " " : " AND ") +  " ((d.DeviceID = " + deviceId + ")) ";
    }
    if (commandStatus != null) {		
	     whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " c.CommandStatus = '" + commandStatus + "' ";
    }
    if (commandTypeCodeArr != null) {		
        whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " c.CommandCode IN " + ConvertArrayToSqlliteParam(commandTypeCodeArr) + " ";
    }
    if (!showSystemCommands) {
	    whereClause = whereClause + (whereClause == "" ? " " : " AND ") + " c.Generated = 0 ";
    }
    if (whereClause.length > 0) {
	    whereClause = " WHERE " + whereClause + " ";
    }  
    var myQuery =   "   SELECT  DISTINCT " +
                    "           CASE WHEN c.Generated = 1 THEN 'yes' ELSE 'no' END AS Generated, " +
                    "           c.CommandID, d.Address64, cs.CommandName, " +
                    "           CASE WHEN c.ParametersDescription is null THEN '' ELSE c.ParametersDescription END ParametersDescription, " +
                    "           CASE c.CommandStatus " +
                    "               WHEN " + CS_New + " THEN 'New' " +
                    "               WHEN " + CS_Sent + " THEN 'Sent' " +
                    "               WHEN " + CS_Responded + " THEN 'Responded' " +
                    "               WHEN " + CS_Failed + " THEN 'Failed' " +
                    "               ELSE 'N/A' " + 
                    "           END AS CommandStatus," +
                    "           c.TimePosted, " +
                    "           CASE WHEN c.TimeResponsed  is null THEN '' ELSE c.TimeResponsed END AS TimeResponsed, " +
                    "           CASE c.CommandStatus " +
                    "               WHEN " + CS_Failed + " THEN 'Error Reason: ' || CASE WHEN c.Response is null THEN c.ErrorReason ELSE c.Response END " +
                    "               ELSE CASE WHEN c.Response is null THEN '' ELSE c.Response END " +
                    "           END AS Response " + 
                    "   FROM    Commands c " +
                    "           INNER JOIN CommandSet cs ON c.CommandCode= cs.CommandCode " +
                    "           INNER JOIN Devices d ON c.DeviceID = d.DeviceID " + 
                    "           LEFT OUTER JOIN CommandParameters cp on c.CommandID = cp.CommandID " + 
                    whereClause +
                    " ORDER BY  c.CommandID DESC " +
                    " LIMIT " + pageSize + " OFFSET " + rowOffset;
                    
    if (forExport) {
        myQuery = " SELECT 'System Generated', 'Tracking No.', 'EUI-64 Address', 'Command', 'Command Parameters', 'Status', 'Posted Time', 'Response Time', 'Response' UNION ALL " + myQuery;
        return myQuery;
    }

    var result;
    try {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
     
       result = service.sqldal.execute({query:myQuery});
    }  catch(e) {
        HandleException(e, "Unexpected error running get commands !");
        return null;
    }
    
    var commands = new Array();		
	
	if (result.length != 0) {
	    for (i=0;i<result.length;i++) {
			
			var command = new Command();
            
            command.Icon = result[i][0] == "yes" ? "<img src='styles/images/syscomm.gif' width='18px' height='18px'>" : "";
			command.TrackingNo = result[i][1];	
			command.Address64 = result[i][2];
	    	command.CommandName = result[i][3];

			if (result[i][4].length <= 30){
				command.Parameters = result[i][4];
			} else {
				command.Parameters = "<a href='JavaScript:PopupValue(\"modalPopup\", \"Parameters\", \"" + result[i][4] + "\")'>" + result[i][4].substring(0,30) + "...</a>";
			};
			
    		command.Status = result[i][5];
			command.PostedTime = result[i][6];
			command.ResponseTime = result[i][7];
			command.Response = result[i][8] == "0" ? null : result[i][8];
											
			if( command.Response.length > 40){
				var tmp = command.Response;
				var k = 0;
				var finalResponse = "";
				while (tmp != ""){
				  finalResponse = finalResponse + tmp.substr(0,40) + "<br>" ;
				  tmp = tmp.substr(40);
				  k = k + 1;
				} 
				command.Response = finalResponse; 
			}
			
			if (i % 2 == 1) {
                command.cellClass = "tableCell2";
            } else {   
                command.cellClass = "tableCell";
            }
			commands[i] = command;
		}		
	}
	commands.commands = commands;
	return commands;        
}

function AddCommand(command, parameters){

	var myCommand = "   BEGIN TRANSACTION; " +
	                "   INSERT INTO Commands (DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode, ParametersDescription) " +
	                "   VALUES ( " + command.DeviceID + ", " + command.CommandTypeCode + ", " + command.CommandStatus + ", '" + command.TimePosted + "', 0, '" + command.ParametersDescription + "'); " +
	                "   SELECT last_insert_rowid(); ";
    if (parameters.length != 0){
        myCommand =  myCommand + " INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue) " +
                                 " SELECT l.lastrowid, t.paramcode, t.paramvalue FROM ("; 
        var myParams = '';
        for (var i=0; i<parameters.length; i++) {
           myParams =  myParams + " SELECT " + parameters[i].ParameterCode + " as paramcode, '" + parameters[i].ParameterValue + "' as paramvalue UNION ";      
        }; 
        myParams = myParams.substring(0,myParams.length-6);
        myCommand = myCommand + myParams + ") t, (SELECT last_insert_rowid() as lastrowid) l; ";
    };        
    myCommand = myCommand + "   COMMIT; ";
    var result;
    try {        
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);     
        result = service.sqldal.execute({mode:"write",query:myCommand});
    } 
    catch(e) {
        HandleException(e, "Error running Add Command!");
        return null;
    }    
	if (result.length != 0){
	    return result[0][0];
	}     
    else{
        return null;
    }        
}


function GetLastCommandInProgress(cmdCode, paramCode, paramValue) {
    var lastValidTimeResponsed = GetUTCDate();
    lastValidTimeResponsed.setTime(lastValidTimeResponsed.getTime()-(APP_LastValidDeviceInformationInterval*60*1000));

    var whereConditions = "";
    if (paramCode != null && paramValue != null) {
       whereConditions = " AND EXISTS (SELECT 1 FROM CommandParameters WHERE CommandID = c.CommandID AND ParameterCode = " + paramCode +" AND ParameterValue = " + paramValue + ")";
    };     		
	var myQuery =   "   SELECT  c.CommandID, c.CommandCode, c.CommandStatus, c.Response, c.TimePosted, c.TimeResponsed, c.ErrorCode, c.ErrorReason " +
	                "   FROM    Commands c " +
	                "   WHERE   c.CommandCode = " + cmdCode + " AND " +
	                "           c.CommandStatus IN (" + CS_Sent + ", " + CS_New + ") AND " +
	                "           c.TimePosted  > '" + ConvertFromJSDateToSQLiteDate(lastValidTimeResponsed) + "' " 
	                + whereConditions +
					"   ORDER BY c.CommandID DESC "
					"   LIMIT 1 OFFSET 0 ";
    var result;    
    try {          
        var service = new jsonrpc.ServiceProxy(serviceURL, methods); 
        result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetLastCommandInProgress !");
        return null;
    }					
	
	if (result.length != 0) {
	    var	command = new Command();			    				
	    command.TrackingNo = result[0][0];
        command.CommandCode = result[0][1];	
	    command.CommandStatus = result[0][2];
	    if (command.CommandStatus != CS_Failed) {
		    command.Response = result[0][3];			
	    } 
	    else {
		    command.Response = "Error Code: " + result[0][6] + ". Error Reason: " + result[0][7];
	    }	    
	    command.TimePosted = result[0][4];
	    command.TimeResponded = result[0][5];	
	    return command;
    }
    return null;	
}

function GetLastCommandResponded(cmdCode, paramCode, paramValue)
{
    var whereConditions = "";
    if (paramCode != null && paramValue != null) {
       whereConditions = " AND EXISTS (SELECT 1 FROM CommandParameters WHERE CommandID = c.CommandID AND ParameterCode = " + paramCode +" AND ParameterValue = " + paramValue + ")";
    };
	var myQuery =  "   SELECT  c.CommandID, c.CommandCode, c.CommandStatus, c.Response, c.TimePosted, c.TimeResponsed, c.ErrorCode, c.ErrorReason " +
	               "   FROM    Commands c " +
				   "   WHERE   c.CommandCode = " + cmdCode + " AND " +
				   "           c.CommandStatus = " + CS_Responded  
				   + whereConditions +
				   "   ORDER BY c.CommandID DESC " + 
				   "   LIMIT 1 OFFSET 0 ";
    var result;   
    try 
    {          
        var service = new jsonrpc.ServiceProxy(serviceURL, methods); 
        result = service.sqldal.execute({query:myQuery});
    }
    catch(e)
    {
        HandleException(e, "Unexpected error running GetLastCommandResponded !");
        return null;
    }					
	
	if (result.length != 0)
	{
	    var	command = new Command();						
	    command.TrackingNo = result[0][0];	    
        command.CommandCode = result[0][1];	
	    command.CommandStatus = result[0][2];
	    if (command.CommandStatus != CS_Failed)
	    {
		    command.Response = result[0][3];			
	    }
	    else {
		    command.Response = "Error Code: " + result[0][6] + ". Error Reason: " + result[0][7];
	    }
	    command.TimePosted = result[0][4];
	    command.TimeResponded = result[0][5];	
	    return command;
	}
	return null;   
}

function GetCommandParameters(commandId) {
	var myQuery =   "   SELECT  CommandID, ParameterCode, ParameterValue " +
	                "   FROM    CommandParameters " +
	                "   WHERE   CommandID = " + commandId;		
    var result;
    
    try {          
        var service = new jsonrpc.ServiceProxy(serviceURL, methods); 
        result = service.sqldal.execute({query:myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetCommandParameters !");
        return null;
    }					

    if (result.length != 0) {
        for (i=0;i<result.length; i++) {
	        result[i].CommandID = result[i][0];
	        result[i].ParameterCode = result[i][1];
	        result[i].ParameterValue = result[i][2];
        }        
	    return result;
	} 
	else {
	    return null;    
	}    
}

function GetLastCommand(cmdCode, paramCode, paramValue)
{
    var whereConditions = "";
    if (paramCode != null && paramValue != null) {
       whereConditions = " AND EXISTS (SELECT 1 FROM CommandParameters WHERE CommandID = c.CommandID AND ParameterCode = " + paramCode +" AND ParameterValue = " + paramValue + ")";
    };
	var myQuery =  "   SELECT  c.CommandID, c.CommandStatus, c.TimeResponsed" +
	               "   FROM    Commands c " +
				   "   WHERE   c.CommandCode = " + cmdCode
				   + whereConditions +
				   "   ORDER BY c.CommandID DESC " + 
				   "   LIMIT 1 OFFSET 0 ";
    var result;   
    try{          
        var service = new jsonrpc.ServiceProxy(serviceURL, methods); 
        result = service.sqldal.execute({query:myQuery});
    }
    catch(e){
        HandleException(e, "Unexpected error running GetLastCommandStatus !");
        return null;
    };					
	
	if (result.length != 0){
	    result.TrackingNo = result[0][0];	    
	    result.CommandStatus = result[0][1];
	    result.TimeResponded = result[0][2];
	    return result;
	};
	return null;   
}