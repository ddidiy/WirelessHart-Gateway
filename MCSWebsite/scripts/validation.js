function ValidateRequired(control, fieldName) {
    if (control.value == null || control.value.trim() == "") {
        alert("Field '" + fieldName + "' is required!");
        control.focus();        
        return false;
    }
    return true;
}

function ValidateRequiredRadio(control, fieldName) {
    for (var i=0; i < control.length; i++) {
        if (control[i].checked) {
            return true;
        }
    }
    alert("Field '" + fieldName + "' is required!");
    return false;
}

function ValidateEUI64(control, fieldName) {
    var regex = new RegExp("([0-9a-fA-F]{2}-){7}[0-9a-fA-F]{2}");    
    var result = regex.exec(control.value);    
    if (result == null || result[0] != control.value) {
        alert("Field '" + fieldName + "' is invalid!");
        control.focus();
        return false;
    } else {
        return true;
    }
}

function ValidateEUI64String(eui64, fieldName) {
    var regex = new RegExp("([0-9a-fA-F]{2}-){7}[0-9a-fA-F]{2}");
    var result = regex.exec(eui64);
    if (result == null || result[0] != eui64) {
        alert("Field '"+fieldName+"' is invalid!");
        return false;
    } else {        
        return true;
    }
}

/*  validateAsIP    - must be true when the control's value will be validated as numeric IP ex: (10.16.0.26)
                    - otherwise must be false
    validateAsHost  - must be true when the control's value will be validated as host IP ex: (yahoo.com)
                    - otherwise must be false
*/
function ValidateIP(control, fieldName, validateAsIP, validateAsHost) {

    var inputString = control.value;
    var rxIP   = /^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$/
    var rxHost = /^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\-]*[A-Za-z0-9])$/
    var result = false;
    
    if (validateAsIP && rxIP.test(inputString)){
        result = true;
    };

    if (validateAsHost && rxHost.test(inputString)){
        result = true;        
    };

    if (!result) {
        alert("Field '" + fieldName + "' is invalid!");
        control.focus();
    }    
    return result;    
}

function Validate16BHex(control, fieldName) {
    var regex = new RegExp("[0-9a-fA-F]{32}");
    var result = regex.exec(control.value);
    if (result == null) {
        alert("Field '" + fieldName + "' is invalid!");
        control.focus();
        return false;
    } else {
        return true;
    }
}

function ValidateDinamicHex(control, fieldName, len) {  
    var regex = new RegExp("[0-9a-fA-F]{"+len+"}");
    var result = regex.exec(control.value);    
    if (result == null || result != control.value) {
        alert("Field '" + fieldName + "' is invalid! \nYou must have "+len+" HEX characters.");
        control.focus();
        return false;
    } else {
        return true;
    }
}

function ValidateHex(control, fieldName) {
    var regex = new RegExp("[0-9a-fA-F]+");
    var result = regex.exec(control.value);
    if (result != control.value) {
        alert("Field '" + fieldName + "' is invalid!");
        control.focus();
        return false;
    } else {
        return true;
    }
}

function ValidateHexVal(val, fieldName) {
    var regex = new RegExp("[0-9a-fA-F]+");
    var result = regex.exec(val);
    if (result != val) {
        return false;
    } else {
        return true;
    }
}


function ValidateIPv6(control, fieldName) {
    var regex = new RegExp("([0-9a-fA-F]{4}:){7}[0-9a-fA-F]{4}");
    var result = regex.exec(control.value);
    if (result == null) {
        alert("Field '" + fieldName + "' is invalid!");
        control.focus();
        return false;
    } else {
        return true;
    }
}


function Validate4DigitHex(control, fieldName) {
    var val = control.value;

    var regex = new RegExp("[0-9a-fA-F]{4}");
    var result = regex.exec(val);
    
    if (result == null) {
        alert("Field '" + fieldName + "' is invalid!");
        control.focus();
        return false;
    } else {
        return true;
    }
}

function ValidateDevice(control, fieldName) {
    var validExpresion = false;
    if(!ValidateRequired(control, fieldName)) {
        return false;
    };
    var devComponents = control.value.split(",");    
    if (devComponents.length != 2){
    	alert("Incorrect number of parameters were specified for '"+fieldName+"'! \nMust have 2 parameters.");
        control.focus();
        return false;
    }
    var eui64 = devComponents[0];
    var regex = new RegExp("([0-9a-fA-F]{2}-){7}[0-9a-fA-F]{2}");
    var result = regex.exec(eui64);
    if (result == null || result[0] != eui64) {
        alert("Parameter 'EUI64' is invalid for '" + fieldName + "'");
        control.focus();
        return false;
    };
    
    var key = devComponents[1];
    var regex = new RegExp("[ ]?([0-9a-fA-F]{2}[ ]*){15}[0-9a-fA-F]{2}");    
    var result = regex.exec(key);                
    if (result == null || result[0] != key){
    	alert("Parameter 'Key' is invalid for '" + fieldName + "'");
        control.focus();
        return false;
    };    

    return true;
}

function ValidateLoggingLevel(control, fieldName) {
     var val = control.value;

    var regex = new RegExp("DEBUG|INFO|WARN|ERROR");
    var result = regex.exec(val);
    
    if (result == null || result != val) {
        alert("Field '" + fieldName + "' is invalid!");
        control.focus();
        return false;
    } else {
        return true;
    }
}


function ValidateNumber(control, fieldName, minValue, maxValue) {
    var val = control.value;
    var regex = new RegExp("[0-9]+");
    var result = regex.exec(val);
    
    if (result == null || result != val || val < minValue || val > maxValue) {
        alert("Field '" + fieldName + "' is invalid! \nMust be an integer value between " + minValue + " and " + maxValue + ".");
        control.focus();
        return false;
    } 
    return true;
}

function ValidateAnyFloat(value, maxNoOfDecimals){
    var regex = new RegExp("(0(\.[0-9]{1,"+maxNoOfDecimals+"})?)|([1-9]+[0-9]*(\.[0-9]{1,"+maxNoOfDecimals+"})?)"); 
    var result = regex.exec(value); 
    if (result == null || result[0] != value) {           
        return false;
    }
    return true;	
}

function ValidateFloatValue(value, minValue, maxValue, maxNoOfDecimals) {
    var regex = new RegExp("(0(\.[0-9]{1,"+maxNoOfDecimals+"})?)|([1-9]+[0-9]*(\.[0-9]{1,"+maxNoOfDecimals+"})?)"); 
    var result = regex.exec(value); 
    if (result == null || result[0] != value ||  (Number(minValue) != null &&  Number(value) < Number(minValue)) || (Number(maxValue) != null && Number(value) > Number(maxValue))) {           
        return false;
    }
    return true;
}


function ValidateNumberValue(value, minValue, maxValue) {
	var regex = new RegExp("(0|[1-9]+[0-9]*)");	
    var result = regex.exec(value);        
    if (result == null || result[0] != value || value < minValue || maxValue < value) {
        return false;
    } 
    return true;
}


function ValidateNumberSet(value, fieldName, valuesList) {
	var regex = new RegExp("(0|[1-9]+[0-9]*)");	
    var result = regex.exec(value);        
    if (result == null || result[0] != value){
    	alert("Field '" + fieldName + "' is invalid! \nMust be an integer in {"+valuesList+"}");
    	return false;
    };
	
    var arrNumber =  valuesList.split(',');       
    for (i=0; i<arrNumber.length ; i++){   
        if (arrNumber[i] == parseInt(value,10)) {
           return true;
        };
    };   
    alert("Field '" + fieldName + "' is invalid! \nMust be an integer in {"+valuesList+"}");
    return false;
}

function ValidateBurstMessage(control, fieldName) {
    if(!ValidateRequired(control, fieldName)) {
        return false;
    };    
    var devComponents = control.value.split(",");
    if (devComponents.length < 5 || 6 < devComponents.length){
    	alert("Incorrect number of parameters were specified this burst message! \nMust have 5 or 6 parameters.");
        control.focus();
        return false;    	    	
    }    
    var eui64 = devComponents[0];
    if (!ValidateEUI64String(eui64, "EUI64")){
        control.focus();
        return false;    	
    }    
    var cmdNo = devComponents[1].trim();    
    if (!ValidateNumberSet(cmdNo, "COMMAND NUMBER", "1, 2, 3, 9, 33, 178")) {
        control.focus();
        return false;
    }
    var burstMessage = devComponents[2].trim();    
    if (!ValidateNumberValue(burstMessage, 0, 255)) {
        alert("Field 'BURST MESSAGE' is invalid! \nMust be an integer value between 0 and 255");
        control.focus();
        return false;
    }
    var updatePeriod = devComponents[3].trim();
    if (!ValidateFloatValue(updatePeriod, 0, 3600, 2)) {
        alert("Field 'UPDATE PERIOD' is invalid! \nMust be a float value between 0 and 3600 and must have maximum 2 decimals."); 
        control.focus();
        return false;
    }
    var maxUpdatePeriod = devComponents[4].trim();
    if (!ValidateFloatValue(maxUpdatePeriod,updatePeriod, 3600, 2)) {
        alert("Field 'MAXIMUM UPDATE PERIOD' is invalid! \nMust be a float value between "+updatePeriod+" and 3600 and must have maximum 2 decimals.");;
        control.focus();
        return false;
    }    
    if (devComponents.length == 6) {
        var subdeviceMac = devComponents[5].trim();        
        if (!ValidateEUI64String(subdeviceMac, "SUBDEVICE MAC")){
            control.focus();
            return false;    	
        };    
    };    
    return true;
}

function ValidateVariable(control, fieldName) {	
    if(!ValidateRequired(control, fieldName)) {
        return false;
    };       
    var variableComponents = control.value.split(",");
    if (variableComponents.length != 5){
    	alert("Incorrect number of parameters were specified for this variable! \nMust have 5 editable parameters.");
        control.focus();
        return false;    	    	
    };    
    var devVarCode = variableComponents[0].trim();
    if (!ValidateNumberValue(devVarCode, 0, 7) &&
    	!ValidateNumberValue(devVarCode, 243, 249)) {
    	alert("Field 'DEVICE VARIABLE CODE' is invalid! \nMust be an integer value in [0-7] or in [243-249]");
        control.focus();
        return false;
    }
    var devVarName = variableComponents[1].trim();
    if (devVarName == null || devVarName.length < 1 || devVarName.length > 16) {
        control.focus();
        alert("Field 'DEVICE VARIABLE NAME' is invalid! \nMust be a string with maximum 16 characters.")
        return false;
    }
    var devVarSlot = variableComponents[2].trim();    
    if (!ValidateNumberSet(devVarSlot, "DEVICE VARIABLE SLOT", "0, 1, 2, 3, 4, 5, 6, 7")) {
        control.focus();
        return false;
    }
    var devVarClassification = variableComponents[3].trim();
    if (!ValidateNumberValue(devVarClassification, 0, 0) &&
        !ValidateNumberValue(devVarClassification, 64, 95)) {
        alert("Field 'DEVICE VARIABLE CLASSIFICATION' is invalid! \nMust be an integer in {0 or [64-95]} ");
        control.focus();
        return false;
    }
    var unitsCode = variableComponents[4].trim();    
    if (!ValidateNumberValue(unitsCode, 1, 169) &&
        !ValidateNumberValue(unitsCode, 220, 239) &&
		!ValidateNumberValue(unitsCode, 250, 250) ) {
        alert("Field 'UNITS CODE' is invalid! \nMust be an integer in [1-169] or in [220-239, 250]");
        control.focus();
        return false;
    }    
    return true;
}

function ValidateTrigger(control, fieldName) {
    var val = control.value.trim();
    if (val == "") {
        return true;
    }    
    var triggerComponents = val.split(",");
    if (triggerComponents.length != 4){
    	alert("Incorrect number of parameters were specified for the trigger! \nMust have 4 editable parameters.");
        control.focus();
        return false;    	    	
    };    
    var burstTriggerModeSel = triggerComponents[0].trim();
    if (!ValidateNumberValue(burstTriggerModeSel, 0, 4)) {
        alert("Field 'BURST TRIGGER MODE SELECTION' is invalid! \nMust be an integer in [0-4]");
        control.focus();
        return false;
    }
    var devVarClassification = triggerComponents[1].trim();
    if (!ValidateNumberValue(devVarClassification, 0, 0) &&
        !ValidateNumberValue(devVarClassification, 64, 95)) {
        alert("Field 'DEVICE VARIABLE CLASSIFICATION' is invalid! \nMust be an integer in {0, [64-95]}");
        control.focus();
        return false;
    }
    var unitsCode = triggerComponents[2].trim();
    if (!ValidateNumberValue(unitsCode, 1, 169) &&
        !ValidateNumberValue(unitsCode, 220, 239) &&
		!ValidateNumberValue(unitsCode, 250, 250) ) {
        alert("Field 'UNITS CODE' is invalid! \nMust be an integer in [1-169] or in [220-239, 250]");
        control.focus();
        return false;
    }
    var level = triggerComponents[3].trim();
    if (!ValidateAnyFloat(level, 6)) {
        alert("Field 'TRIGGER LEVEL' is invalid! \nMust be a float value with maximum 6 decimals."); 
        control.focus();
        return false;
    }    
    return true;
}

//format: <host_value>=<UnitId>,<EUI64>,<map_type>
function ValidateHost(control, fieldName) {
    if(!ValidateRequired(control, fieldName)) {
        return false;
    }
    
    var val = control.value;
    var regex = new RegExp("[0-9]+,[ ]*[0-9a-fA-F]{16},[ ]*[0-9]+");
    var result = regex.exec(val);
    
    if (result == null || result[0] != control.value) {
        alert("Field '" + fieldName + "' is invalid!");
        control.focus();
        return false;
    }
    return true;
}

function ValidateRegisterServer(control, fieldName) {
    if(!ValidateRequired(control, fieldName)) {
        return false;
    };
    var registerComps = control.value.split(","); 
    switch (registerComps[3]){
    case "device_variable" :
    	if (registerComps.length != 7){
            alert("Incorrect number of parameters were specified for the <device_variable> format! \nMust have 7 parameters.");
            control.focus();
            return false;
        }
    	var startAddress = registerComps[0].trim();
        if (!ValidateNumberValue(startAddress, 0, 65535)) {
            alert("Field 'START ADDRESS' is invalid! \nMust be an integer value between " + 0 + " and " + 65535 + ".");
        	control.focus();
            return false;
        }
        var wordCount = registerComps[1].trim();
        if (!ValidateNumberValue(wordCount, 1, 125)) {
            alert("Field 'WORD COUNT' is invalid! \nMust be an integer value between " + 1 + " and " + 255 + ".");
        	control.focus();
            return false;
        }
        var tmp = parseInt(startAddress,10) + parseInt(wordCount, 10);
        if (!ValidateNumberValue(tmp, 0, 65536)){
        	alert("'START ADDRESS + WORD COUNT' is invalid! \nMust be an integer value between " + 0 + " and " + 65536 + ".");
            return false;
        }
        var eui64 = registerComps[2];
        if (!ValidateEUI64String(eui64, "EUI64")) {
            control.focus();
            return false;
        }        
        var burstMsg = registerComps[4].trim();
        if (!ValidateNumberValue(burstMsg, 0, 255)) {
        	alert("Field 'BURST MESSAGE' is invalid! \nMust be an integer value between " + 0 + " and " + 255 + ".");
        	control.focus();
            return false;
        }
        var devVariableCode = registerComps[5].trim();
        if (!ValidateNumberValue(devVariableCode, 0, 7) &&
        	!ValidateNumberValue(devVariableCode, 243, 249)) {
        	alert("Field 'DEVICE VARIABLE CODE' is invalid! \nMust be an integer value in [0-7] or [243-249].");
            return false;
        }
        var devState = registerComps[6].trim();
        if (!ValidateNumberValue(devState, 0, 0) &&
        	!ValidateNumberValue(devState, 2, 2)) {        	
        	alert("Field 'DEVICE STATE' is invalid! \nMust be 0 or 2.");
            control.focus();
            return false;
        } 
        break;        
    case "gw_info" :
    	if (registerComps.length != 4){
            alert("Incorrect number of parameters were specified for <gw_info> format! \Must have 4 parameters.");
            control.focus();
            return false;
        }
        var startAddress = registerComps[0].trim();        
        if (!ValidateNumberValue(startAddress, 0, 65535)) {
        	alert("Field 'START ADDRESS' is invalid! \nMust be an integer value in [0-65535].");
        	control.focus();
            return false;
        }
        var wordCount = registerComps[1].trim();
        if (!ValidateNumberValue(wordCount, 5, 5)) {
        	alert("Field 'WORD COUNT' is invalid! \nMust be 5.");
        	control.focus();
            return false;
        }
        var tmp = parseInt(startAddress,10) + parseInt(wordCount, 10);
        if (!ValidateNumberValue(tmp, 0, 65536)){
        	alert("'START ADDRESS + WORD COUNT' is invalid! \nMust be an integer value between " + 0 + " and " + 65536 + ".");
            return false;
        }
        var eui64 = registerComps[2];
        var validEUI64 = ["00-1B-1E-F9-81-00-00-02","00-00-00-00-00-00-00-00","FF-FF-FF-FF-FF-FF-FF-FF"]
        if (eui64.toUpperCase() !=  validEUI64[0] && eui64.toUpperCase() !=  validEUI64[1] && eui64.toUpperCase() != validEUI64[2]) {
            alert("Field 'EUI64' is invalid! \nMust be in: '"+validEUI64[0]+"', '"+validEUI64[1]+"' or '"+validEUI64[2]+"'.");
            control.focus();
            return false;
        }    
        break;
    case "gw_code_word":  
    	if (registerComps.length != 4){
            alert("Incorrect number of parameters were specified for <gw_code_word> format! \nMust have 4 parameters.");
            control.focus();
            return false;
        }
        var startAddress = registerComps[0].trim();
        if (!ValidateNumberValue(startAddress, 0, 65535)) {
        	alert("Filed 'START ADDRESS' is invalid! \nMust be an integer value between " + 0 + " and " + 65535 + ".");
        	control.focus();
            return false;
        }
        var wordCount = registerComps[1].trim();
        if (!ValidateNumberValue(wordCount, 1, 16)) {
        	alert("Filed 'WORD COUNT' is invalid! \nMust be an integer value between " + 1 + " and " + 16 + ".");
        	control.focus();
            return false;
        }
        var tmp = parseInt(startAddress,10) + parseInt(wordCount, 10);         
        if (!ValidateNumberValue(tmp, 0, 65536)){
        	alert("'START ADDRESS + WORD COUNT' is invalid! \nMust be an integer value between " + 0 + " and " + 65536 + ".");
        	control.focus();
            return false;
        }        
        var eui64 = registerComps[2];
        var validEUI64 = ["00-1B-1E-F9-81-00-00-02","00-00-00-00-00-00-00-00","FF-FF-FF-FF-FF-FF-FF-FF"]
        if (eui64.toUpperCase() !=  validEUI64[0] && eui64.toUpperCase() !=  validEUI64[1] && eui64.toUpperCase() != validEUI64[2]) {
            alert("Field 'EUI64' is invalid! \nMust be in: '"+validEUI64[0]+"', '"+validEUI64[1]+"' or '"+validEUI64[2]+"'.");
            control.focus();
            return false;
        }    
        break;
    default:
       alert("Field 'REGISTER TYPE' is invalid! \nMust be 'device_variable', 'gw_info' or 'gw_code_word'.");
       control.focus();
       return false;
    };          
    return true;
}

function ValidateJoinKey(control, fieldName){        
    if(!ValidateRequired(control, fieldName)) {
        return false;
    }                  
    regex = new RegExp("([0-9a-fA-F]{2}[ ]*){15}[0-9a-fA-F]{2}");
    
    var result = regex.exec(control.value);                
    if (result != null && result[0] == control.value){
        return true;
    } else {
        alert("Field '" + fieldName + "' is invalid!");
        control.focus();
        return false;
    };        
}
