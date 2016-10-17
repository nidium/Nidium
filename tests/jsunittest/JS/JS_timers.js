Tests.registerAsync("Global.setTimeout", function(next) {
    setTimeout(function() {
        next();
    }, 100);
}, 110);


Tests.registerAsync("Global.setInterval", function(next) {
    var count = 0;
    const speed = 100;

    var t = +new Date();
    var i = setInterval(function() {
        count++;

        var diff = Math.abs((+new Date() - t) - speed);

        Assert.equal(diff < 19, true);

        if (count == 10) {
            clearInterval(i);
            next();
        }
        t = +new Date();
    }, speed);
}, 2000);


Tests.registerAsync("Global.setTimeout (clear)", function(next) {
    var t = setTimeout(function() {
        Assert.equal(true, false);
    }, 100);

    setTimeout(function() {
        clearTimeout(t);
    }, 20);

    setTimeout(function() {
        next();
    }, 150);

}, 200);