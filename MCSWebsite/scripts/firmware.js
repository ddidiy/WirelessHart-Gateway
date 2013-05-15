function Firmware()
{
	this.FirmwareID = null;
	this.FileName = null;
	this.Version = null;
	this.Description = null;
	this.UploadDate = null;
	this.FirmwareTypeID = null;
	this.FirmwareTypeName = null;
	this.UploadStatusID = null;
	this.UploadStatusName = null;
}

/* Firmware Types*/

var FT_AquisitionBoard = 0;
var FT_NetworkManager = 1;
var FT_Gateway = 2;
var FT_BackboneRouter = 3;
var FT_Device = 10;
var FT_DeviceNonRouting = 11;

	
function GetFirmwareTypeName(firmwareType)
{
	var firmwareTypeName = null;	
	switch (firmwareType)
	{
		case  FT_BackboneRouter:
			firmwareTypeName = "Backbone Router";
			break;
		case  FT_Device:
			firmwareTypeName = "Field Router";
			break;
		case  FT_DeviceNonRouting:
			firmwareTypeName = "Device Non Routing";
			break;
		case  FT_Gateway:
			firmwareTypeName = "Gateway";
			break;
		case  FT_NetworkManager:
			firmwareTypeName = "Network Manager";
			break;
		case  FT_AquisitionBoard:
			firmwareTypeName = "Acquisition Board";
			break;
		default:
			firmwareTypeName = "Unkonwn - " + firmwareType;
			break;
	}
	return firmwareTypeName;
}

/* Upload Status */

var US_New = 0;
var US_SuccessfullyUploaded = 1;
var US_Uploading = 2;
var US_WaitRetrying = 3;
var US_Failed = 4;

	
function GetUploadStatusName(uploadStatus)
{
	var uploadStatusName = null;	
	switch (uploadStatus)
	{
		case US_New:
			uploadStatusName = "New";
			break;
		case US_Uploading:
			uploadStatusName = "Uploading";
			break;
		case US_SuccessfullyUploaded:
			uploadStatusName = "Successfully Uploaded";
			break;
		case US_WaitRetrying:
			uploadStatusName = "Wait-Retrying";
			break;
		case US_Failed:
			uploadStatusName = "Failed";
			break;			
		default:
			uploadStatusName = "Unkonwn - " + uploadStatus;
			break;
	}
	return uploadStatusName;
}