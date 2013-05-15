/*var methods = ["config.getGroupVariables",
               "isa100.mh.setPublisher",
               "isa100.mh.delPublisher",
               "misc.applyConfigChanges",
               "config.getConfig",
               "user.logout"];*/

//??? content
var CONTENT = null;

function InitOPCPage()
{
    SetPageCommonElements();
 
    //InitJSON();

    //ReadConfigFromDisk();
    
    //ClearChannels();
    
    //PopulatePublisherList();
}


function ShowHideHelp(op)
{
    var divHelp = document.getElementById("divHelp");
    
    if (op == "open")
    {
        divHelp.style.display = "";
    }
    else
    {
        divHelp.style.display = "none";
    }
}