var refreshinterval = 60;

var starttime;
var nowtime;
var reloadseconds = 0;
var secondssinceloaded = 0;
var timer;

function enableDisableRefresh(isChecked)
{
    if (isChecked)
    {
        start();   
    }
    else 
    {
        clearTimeout(timer);
    }
}

function start()
 {
    starttime = new Date();
    starttime = starttime.getTime();
    countdown();
}

function countdown() 
{
    nowtime = new Date();
    nowtime = nowtime.getTime();
    secondssinceloaded = (nowtime - starttime) / 1000;
    reloadseconds = Math.round(refreshinterval - secondssinceloaded);
    if (refreshinterval >= secondssinceloaded) 
    {
        timer = setTimeout("countdown()", 1000);
     
        //window.status = "Page refreshing in " + reloadseconds + " seconds";
    }
    else 
    {
       clearTimeout(timer);
       window.location.reload(true);
    }
}
