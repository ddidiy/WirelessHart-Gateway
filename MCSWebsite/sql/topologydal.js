var methods = [ "sqldal.execute", "user.logout", "file.remove", "file.create", "file.exists" ];
var resLvl;

function GetTopology() {
	var myQuery = " SELECT D.DeviceID, "
						+ " D.Address64, "
						+ " D.DeviceRole, "
						+ " D.DeviceLevel, "
						+ " C.Model, "
						+ " D.DeviceTag, "
						+ " C.Company, "
						+ "	D.NickName "
				+ " FROM Devices D "
						+ " INNER JOIN DevicesCodes C ON D.DeviceCode = C.DeviceCode "
						+ " WHERE D.DeviceStatus >= " + DS_NormalOperationCommencing + " "
				+ " ORDER BY D.DeviceID ASC";

	var result;
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);
		result = service.sqldal.execute({query: myQuery});
	} catch (e) {
		HandleException(e, "Unexpected error running GetTopology -> Get data");
		return null;
	}
    if (result.length != 0) {
        var devices = [];
        var icons = GetCustomIconFiles();
		for ( var i = 0; i < result.length; i++) {
			var modelAsciiString = GetAsciiString(result[i][4]);
			var tagAsciiString = GetAsciiString(result[i][5]);
			var manufacturerAsciiString = GetAsciiString(result[i][6]);
            devices[i] = {
				DeviceID: result[i][0],
				Address64: result[i][1],
				DeviceRole: result[i][2],
				X: 0,
				Y: 0,
				Label: ((result[i][2] == DT_Gateway) ? "GW" : 
				        (result[i][2] == DT_NetworkManager) ? "NM" : 
						(result[i][2] == DT_AccessPoint) ? "AP" :GetPadedHex(result[i][7])),
				ModelHex: result[i][4],
				Model: result[i][4],
				DeviceTag: ((result[i][5] != null) ? (tagAsciiString != "") ? tagAsciiString : NAString : NAString),
				Manufacturer: result[i][6],
				Icon: "styles/images/" + GetIconFileName(icons, result[i][4], result[i][2])
			};
		}
		return devices;
	} else {
		return null;
	}
}

function GetTopologyLinks() {
	var myQuery = " SELECT N.DeviceID, N.PeerID, N.ClockSource, N.RSL, T.DeviceRole " +				
				  " FROM ReportNeighborHealthList N " +  
					   " INNER JOIN Devices F ON N.DeviceID = F.DeviceID " +
				 	   " INNER JOIN Devices T ON N.PeerID = T.DeviceID " +
				  " WHERE F.DeviceStatus >= " + DS_NormalOperationCommencing
					  + " AND T.DeviceStatus >= " + DS_NormalOperationCommencing;
	var result;
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);		
	    result = service.sqldal.execute({query: myQuery});
	} catch (e) {
		HandleException(e, "Unexpected error on getting topology links");
		return null;
	}
	var links = [{FromDeviceID: -1, ToDeviceID: -1}];
	var idx = 0;
	if (result.length != 0) {
		for (var i = 0; i < result.length; i++) {
			var dupLink = false;
			var dupIdx = -1;
			for (var j = 0; j < links.length; j++) {
				if ((result[i][0] == links[j].FromDeviceID && result[i][1] == links[j].ToDeviceID) || 
				    (result[i][0] == links[j].ToDeviceID && result[i][1] == links[j].FromDeviceID)) {
					dupLink = true;
					dupIdx = j;
				}
			}
			if (dupLink) {
				links[dupIdx].Bidirectional = true;
				links[dupIdx].ClockSource2 = result[i][2];
				links[dupIdx].SignalQuality2 = "" + (result[i][3] != "NULL" ? result[i][3] : "");
				links[dupIdx].FromDeviceRole = result[i][4];
			} else {
				links[idx++] = {FromDeviceID:   result[i][0],
					            ToDeviceID:     result[i][1],
					            Bidirectional:  false,
					            ClockSource:    result[i][2],
					            ClockSource2:   0,
					            SignalQuality:  "" + (result[i][3] != "NULL" ? result[i][3] : ""),
					            SignalQuality2: "",
								ToDeviceRole:   result[i][4],
								FromDeviceRole: null};
			}
		}
		return links
	} else {
		return null;
	}
}

function GetServices(deviceId) {

	var myQuery = " SELECT S.ServiceID, " 
				+ "        S.DeviceID, "
				+ "        CASE DS.DeviceRole " 
				+ "             WHEN " + DT_Gateway + " THEN 'GW' "
				+ "             WHEN " + DT_AccessPoint + " THEN 'AP' "
				+ "             WHEN " + DT_NetworkManager + " THEN 'NM' "
				+ "             ELSE DS.Address64 "
				+ "		   END AS SourceAddress64, "
				+ "        CASE DD.DeviceRole "
				+ "             WHEN " + DT_Gateway + " THEN 'GW' "
				+ "             WHEN " + DT_AccessPoint + " THEN 'AP' "
				+ "             WHEN " + DT_NetworkManager + " THEN 'NM' "
				+ "             ELSE DD.Address64 "
				+ "		   END AS DestinationAddress64, "
				+ "		   R.GraphID "
				+ "   FROM Services S "
				+ "        INNER JOIN Devices DS ON S.DeviceID = DS.DeviceID AND DS.DeviceStatus >= " + DS_NormalOperationCommencing + " "
				+ "        INNER JOIN Devices DD ON S.PeerID = DD.DeviceID AND DD.DeviceStatus >= " +  DS_NormalOperationCommencing + " "
				+ "		   INNER JOIN Routes R ON S.DeviceID = R.DeviceID AND S.RouteID = R.RouteID "
				+ "  WHERE S.DeviceID = " + deviceId + " OR " + " S.PeerID = " + deviceId + " "
				+ "  ORDER BY SourceAddress64, S.ServiceID";
	var result;
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);
		result = service.sqldal.execute( {
			query : myQuery
		});
	} catch (e) {
		HandleException(e, "Unexpected error running GetServices");
		return null;
	}
	if (result.length != 0) {
		for (i = 0; i < result.length; i++) {
			result[i].ServiceID = result[i][0];
			result[i].DeviceID = result[i][1];
			result[i].SourceDestination = result[i][0] + ": " + result[i][2]
					+ " -> " + result[i][3];
			result[i].GraphID = result[i][4];
			if (i % 2 == 0) {
				result[i].cellClass = "tableCell2";
			} else {
				result[i].cellClass = "tableCell";
			}
		}
		result.services = result;
		return result;
	} else {
		return null;
	}
}

function GetServiceDetails(serviceId, deviceId) {
	if (deviceId == null || serviceId == null) {
		alert("Invalid deviceId or serviceId for GetServiceDetails");
		return null;
	}
	var myQuery = " SELECT s.ServiceID, "
					+ " ds.Address64 SrcDeviceAddress64, "
					+ " dd.Address64 DstDeviceAddress64, "
					+ " s.ApplicationDomain, " 
					+ " s.SourceFlag, "
					+ " s.SinkFlag, " 
					+ " s.IntermittentFlag, "
					+ " s.Period, " 
					+ " s.RouteID, "
					+ " s.Timestamp, "
					+ " dd.DeviceRole AS DestinationRole "	
				+ " FROM Services s "
					+ " INNER JOIN Devices ds ON ds.DeviceID = s.DeviceID "
					+ " INNER JOIN Devices dd ON dd.DeviceID = s.PeerID "
				+ " WHERE   s.DeviceID = " + deviceId + " AND "
					+ " s.ServiceID = " + serviceId;

	var result;
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);
		result = service.sqldal.execute( {
			query : myQuery
		});
	} catch (e) {
		HandleException(e, "Unexpected error running GetServiceDetails");
		return null;
	}
	if (result.length != 0) {
		var service = new Object();
		service.ServiceID = result[0][0];
		service.SrcDeviceAddress64 = result[0][1];
		service.DstDeviceAddress64 = result[0][2];
		service.ApplicationDomain = result[0][3];
		service.SourceFlag = result[0][4] == 1 ? "yes" : "no";
		service.SinkFlag = result[0][5] == 1 ? "yes" : "no";
		service.IntermittentFlag = result[0][6] == 1 ? "yes" : "no";
		service.Period = result[0][7];
		service.RouteID = result[0][8];
		service.Timestamp = result[0][9];
		service.DestinationRole = result[0][10];
	}
	return service;
}

function GetServiceElements(serviceId, deviceId, graphId, destinationRole) {
	
	var whereClause = "";		
	if (destinationRole = DT_Gateway) {
		var nmDevice = GetNetworkManagerDevice();
		whereClause = " AND G.PeerID != " + nmDevice.DeviceID; 
	}	
	var myQuery = " SELECT S.ServiceID, "
					+ "	S.DeviceID, "
					+ "	S.ApplicationDomain, "
					+ "	G.DeviceID, "
					+ "	G.PeerID "
				+ " FROM  Services S "
					+ "	INNER JOIN Routes R ON S.DeviceID = R.DeviceID AND S.RouteID = R.RouteID "
					+ "	INNER JOIN GraphNeighbors G ON R.GraphID = G.GraphID "
				+ " WHERE S.ServiceID = " + serviceId + " AND "
					+ " S.DeviceID = " + deviceId + " AND " 
					+ " G.GraphID = " + graphId + " "
					+ whereClause;		
	var result;
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);
		result = service.sqldal.execute( {
			query : myQuery
		});
	} catch (e) {
		HandleException(e, "Unexpected error running GetServiceElements");
		return null;
	}
	if (result.length != 0) {
		for (i = 0; i < result.length; i++) {
			result[i].ServiceID = result[i][0];
			result[i].DeviceID = result[i][1];
			result[i].ServiceType = result[i][2];
			result[i].FromDeviceID = result[i][3];
			result[i].ToDeviceID = result[i][4];
		}
		result.serviceLinks = result;
		return result
	} else {
		return null;
	}
}

function GetServicesElements(deviceId, mode) {
	var myQuery =   " SELECT S.ServiceID, "  
				  + 	   " S.DeviceID, "
				  + 	   " S.ApplicationDomain, "
				  +        " G.DeviceID, "
				  + 	   " G.PeerID "  
				  + " FROM  Services S " 
				  +       " INNER JOIN Routes R ON S.DeviceID = R.DeviceID AND S.RouteID = R.RouteID "
				  +       " INNER JOIN GraphNeighbors G ON R.GraphID = G.GraphID "
				  +		  " INNER JOIN Devices DS ON G.DeviceID = DS.DeviceID AND DS.DeviceStatus >= " + DS_NormalOperationCommencing + " "
				  +		  " INNER JOIN Devices DD ON G.PeerID   = DD.DeviceID AND DD.DeviceStatus >= " + DS_NormalOperationCommencing + " "
				  + " WHERE ";
	myQuery += (mode == 2) ? " S.DeviceID = " + deviceId + " " : (mode == 3) ? " S.PeerID = " + deviceId + " " : " S.DeviceID = " + deviceId + " OR S.PeerID = " + deviceId + " ";
	myQuery += " ORDER BY S.DeviceID ASC, S.ServiceID ASC ";
	
	var result;
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);
		result = service.sqldal.execute( {
			query : myQuery
		});
	} catch (e) {
		HandleException(e, "Unexpected error running GetServicesElements");
		return null;
	}
	if (result.length != 0) {
		for (i = 0; i < result.length; i++) {
			result[i].ServiceID = result[i][0];
			result[i].DeviceID = result[i][1];
			result[i].ServiceType = result[i][2];
			result[i].FromDeviceID = result[i][3];
			result[i].ToDeviceID = result[i][4];	
		}
		result.services = result;
		return result
	} else {
		return null;
	}
}

function GetLinkType(sourceDeviceId, destinationDeviceID) {
	var myQuery = "	SELECT	S.DeviceRole, "
				+ "			D.DeviceRole, "
				+ "			C.ServiceType "
				+ "	FROM	Contracts C "
				+ "			INNER JOIN Devices S ON C.SourceDeviceID = S.DeviceID AND S.DeviceID = " + sourceDeviceId + " "
				+ "			INNER JOIN Devices D ON C.DestinationDeviceID = D.DeviceID AND D.DeviceID = " + destinationDeviceID;
	var result;
	try {
		var service = new jsonrpc.ServiceProxy(serviceURL, methods);

		result = service.sqldal.execute( {
			query : myQuery
		});
	} catch (e) {
		HandleException(e, "Unexpected error running GetLinkType");
		return null;
	}

	var linkType = 0;
	if (result.length != 0) {
		result.SourceRole = result[0][0];
		result.DestinationRole = result[0][1];
		result.ServiceType = result[0][2];

		if (result.ServiceType == CST_Periodic) {
			/* SOURCE = Device and DEST = GW */
			if (IsFieldDevice(result.SourceRole)
					&& result.DestinationRole == DT_Gateway) {
				linkType = linkType | LT_PublishSubscribe;
			}
			;
			/* SOURCE = Device and DEST = Device/AP */
			if (IsFieldDevice(result.SourceRole)
					&& (result.DestinationRole == DT_AccessPoint || IsFieldDevice(result.DestinationRole))) {
				linkType = linkType | LT_LocalLoop;
			}
			;
			/* SOURCE = Device/BBR and DEST = NM */
			if ((result.SourceRole == DT_AccessPoint || IsFieldDevice(result.SourceRole))
					&& (result.DestinationRole == DT_NetworkManager)) {
				linkType = linkType | LT_PeriodicPublishing;
			}
		} else { /* Aperiodic contract service type */
			linkType = linkType | LT_Aperiodic;
		}
		return linkType;
	} else {
		return null;
	}
}
