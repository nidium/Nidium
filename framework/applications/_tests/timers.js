/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var past = +new Date();

setInterval(function(){
    var now = +new Date();
    echo("Timer", now-past);
    past = now;
}, 100);

