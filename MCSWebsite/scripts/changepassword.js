function InitChangePasswordPage() 
{
    SetPageCommonElements();
    InitJSON();
    
    document.getElementById("txtOldPassword").focus();
}


function SaveData()
{
    if (ValidateInput())
    {
        var txtOldPassword = document.getElementById("txtOldPassword");
        var txtNewPassword = document.getElementById("txtNewPassword");
        var txtConfirmNewPassword = document.getElementById("txtConfirmNewPassword");
        
        if(ChangePassword(txtOldPassword.value, txtNewPassword.value))
        {
            DisplayOperationResult("spnOperationResult", "Password changed successfully. Go to <a href='login.html'>Login</a>.");
        }        
    }
}


function ClearData()
{
    document.getElementById("txtOldPassword").value = "";
    document.getElementById("txtNewPassword").value = "";
    document.getElementById("txtConfirmNewPassword").value = "";
    
    document.getElementById("txtOldPassword").focus();
    
    ClearOperationResult("spnOperationResult");
}


function ValidateInput()
{
    var txtOldPassword = document.getElementById("txtOldPassword");
    var txtNewPassword = document.getElementById("txtNewPassword");
    var txtConfirmNewPassword = document.getElementById("txtConfirmNewPassword");

    if (!ValidateRequired(txtOldPassword, "Old password"))
    {
        return false;
    }
    
    if (txtOldPassword.value.length < 8)
    {
        alert("Field 'Old password' must be at least 8 characters length !");
        return false;
    }
    
    if (!ValidateRequired(txtNewPassword, "New password"))
    {
        return false;
    }
    
    if (txtNewPassword.value.length < 8)
    {
        alert("Field 'New password' must be at least 8 characters length !");
        return false;
    }
    
    if (!ValidateRequired(txtConfirmNewPassword, "Confirm new password"))
    {
        return false;
    }
    
    if (txtConfirmNewPassword.value.length < 8)
    {
        alert("Field 'Confirm new password' must be at least 8 characters length !");
        return false;
    }
    
    if (txtNewPassword.value != txtConfirmNewPassword.value)
    {
        alert("Field 'New password' must be equal to field 'Confirm new password' !");
        txtConfirmNewPassword.focus();
        return false;
    }
    
    return true;
}
