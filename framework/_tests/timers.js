/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

var past = +new Date();

setInterval(function(){
    var now = +new Date();
    echo("Timer", now-past);
    past = now;
}, 1000);

