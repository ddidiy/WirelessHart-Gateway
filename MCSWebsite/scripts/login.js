var methods = ["user.login",
               "user.logout",
               "user.update",
               "file.exists", 
               "sqldal.open",
               "sqldal.close",
               "config.getGroupVariables"];

function InitLoginPage() {
    SetPageCommonElements();    
    InitJSON();
    
    var url = parent.document.URL;
    var sessionExpired = url.substring(url.indexOf('?') + 1, url.length) == "exp" ? true : false;
    if(sessionExpired) {
        var spnSessionExpired = document.getElementById("spnSessionExpired");
        spnSessionExpired.style.display = "";
    }
    
    var userId = document.getElementById("txtUser");
    var passwd = document.getElementById("txtPassword");
    //todo remove me for release
    if (DEBUG) {
        userId.value = "admin";
        passwd.value = "adminadmin";
    } else {
        userId.value = "";
        passwd.value = "";
    }
    userId.focus();    
}

function Login(event) {
     var keyCodeEntered = (event.which) ? event.which : window.event.keyCode;
     
     if ( keyCodeEntered == 13 || keyCodeEntered == 0 || keyCodeEntered == 1) { //enter on textboxes (13) or mouse click on Login button (0,1 - browser dependent)
        ClearOperationResult("spnSessionExpired");
    
        var userId = document.getElementById("txtUser");
        var password = document.getElementById("txtPassword");

        if(!ValidateRequired(userId, "User Name")) {
            return;
        }
	    if(!ValidateRequired(password, "Password")) {
            return;
        }
    
        try {
	        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
            var response = service.user.login({user: userId.value, pass: password.value});
            if (response) {
                var openResult = service.sqldal.open({dbFile:"/tmp/Monitor_Host.db3"});
 	   	        if(!openResult) {
                    return;
                }
                
                //used to display the logged user (common.js/menu section)
                CreateCookie("loggedUser", userId.value, null);
                GetNetworkID();
                document.location.href = "devicelist.html";
            }
        } catch(e) {
            HandleException(e, "Login failed !");
            userId.focus();
        }
    }
}

function Logout() {
    try {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        var response = service.user.logout();
        if (response) {
            EraseCookie("loggedUser");
            document.location.href = "login.html";
        }
    } catch(e) {
        document.location.href = "login.html";
    }
}

function ChangePassword(oldPassword, newPassword) {
    //verify old password
    try {
        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
        var response = service.user.login({user: ReadCookie("loggedUser"), pass: oldPassword});
        if (response) {
   	        //ok to change password
   	        try {
    	        var service = new jsonrpc.ServiceProxy(serviceURL, methods);
                var response = service.user.update({pass: newPassword});
                if (response) {
 	           	    return true;
                } else {
                    return false;
                }
            } catch(e) {
                HandleException(e, "Password update failed !");
                return false;
            }
        }
    } catch(e) {
        HandleException(e, "Invalid old password !");
    }
}
