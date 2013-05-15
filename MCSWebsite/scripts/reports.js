
/* Link Status for NeighborsHealthReport*/
var LS_Unavailable = 0;
var LS_Available = 1;

function GetLinkStatusName(linkStatus)
{
	switch (linkStatus)
	{
		case LS_Unavailable:
		  return "Unavailable";
		case LS_Available:
		  return "Available";
		default:
		  return linkStatus;
	}
}

/* Link Direction for ScheduleLinkReport*/
var SLD_Reception = 0;
var SLD_Transmission = 1;

function GetLinkDirectionName(linkDirection)
{
	switch (linkDirection)
	{
		case SLD_Reception:
		  return "Reception";
		case SLD_Transmission:
		  return "Transmission";
		default:
		  return linkDirection;
	}
}

/* Lynk Type for ScheduleLinkReport*/

var SLT_Normal = 0;
var SLT_Discovery = 1;
var SLT_Broadcast = 2;
var SLT_Join = 3;

function GetLinkTypeName(linkType)
{
	switch (linkType)
	{
		case SLT_Normal:
		  return "Normal";
		case SLT_Discovery:
		  return "Discovery";
		case SLT_Broadcast:
		  return "Broadcast";
		case SLT_Join:
		  return "Join";
		default:
		  return linkType;
	}
}

/*ApplicationDomain*/
var AD_PUBLISH = 0;
var AD_EVENT = 1;
var AD_MAINTENANCE = 2;
var AD_BLOCK_TRANSFFER = 3;

function GetApplicationDomain(applicationDomain)
{
	switch (applicationDomain)
	{
		case AD_PUBLISH:
		  return "Publish";
		case AD_EVENT:
		  return "Event";
		case AD_MAINTENANCE:
		  return "Maintenance";
		case AD_BLOCK_TRANSFFER:
		  return "Block Transfer";
		default:
		  return applicationDomain;
	}
}

function GetSignalQuality(val){
    if ( -99 < val && val <= -85 )
        {return 'Poor'};
    if ( -85 < val && val <= -73)
        {return 'Fair'};
    if (-73 < val && val <= -60)
        {return 'Good'};
    if (-60 < val && val <= -10)
        {return 'Excellent'}
    else
        {return 'Out of range'};
}

function GetSignalStrength(val){ 

    if (-192 <= val && val <= 63 ){
        return val;
    }
    else{
        return 'Out of range';
    };
}


function GetSignalQualityColor(val){

    //if ( val == 0 ) 
    //    {return '#000000'};
    if ( -99 <= val && val <= -85 )
        {return '#FF7800'};
    if ( -85 < val && val <= -73)
        {return '#00B6FF'};
    if (-73 < val && val <= -60)
        {return '#0000FF'};
    if (-60 < val && val <= -10)
        {return '#007700'}
    else
        {return '#FF0000'};
}