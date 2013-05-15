var methods = ["sqldal.execute", "user.logout", "user.isValidSession"];

function GetChannelsForDevice(deviceId) {
    var myQuery =   " SELECT ChannelID, Name, CmdNo, DeviceVariable " +
    				" FROM Channels WHERE DeviceID = " + deviceId + " ORDER BY ChannelID ";
    var result;
    try  {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetChannelsForDevice!");
        return null;
    }
    var channelColl = new Array();
    for(i = 0; i<result.length; i++) {
        var channel = new DeviceChannel();
    	channel.ChannelID = result[i][0];
    	channel.Name = result[i][1];
    	channel.CmdNo = result[i][2];
    	channel.DeviceVariable = result[i][3];
    	channelColl[i] = channel;
    }
    return channelColl;
}

function GetChannelsStatistics(deviceId) {
    var myQuery =   " SELECT ByteChannelsArray " +
                    " FROM ChannelsStatistics " +
                    " WHERE	DeviceID = " + deviceId;
    var result;
    try  {
       var service = new jsonrpc.ServiceProxy(serviceURL, methods);
       result = service.sqldal.execute({query: myQuery});
    } catch(e) {
        HandleException(e, "Unexpected error running GetChannelsStatistics!");
        return null;
    }

	if (result.length != 0) {
        if (result[0][0] == "NULL") {
	       return null;
        } else { 
		  return result[0][0];
        }
	} else {			
	    return null;
    }
}