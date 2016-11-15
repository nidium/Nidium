// Nidium frontend can have at most 16ms of delay for the timers because of vsync
// Under some condition it can even be up to 50ms
// See : https://github.com/nidium/Nidium/issues/22
const TIMEOUT_THRESHOLD = 16*8;

Tests.registerAsync("Global.setTimeout", function(next) {
    var start = Date.now();
    setTimeout(function() {
        var duration = Date.now() - start;
        console.log(`diff=${duration - 500}ms`);
        Assert(duration > 499 && duration < (500 + TIMEOUT_THRESHOLD), 
                `setTimeout did not run at the expected time (${duration}ms instead of 500ms)`);
        next();
    }, 500);
}, 1000);


Tests.registerAsync("Global.setInterval", function(next) {
    var count = 0;
    const speed = 100;

    var t = +new Date();
    var i = setInterval(function() {
        count++;

        var diff = Math.abs((+new Date() - t) - speed);

        console.log(`diff=${diff}ms`);
        Assert.equal(diff < TIMEOUT_THRESHOLD, true, 
                `Set interval has more than ${TIMEOUT_THRESHOLD} of delay (diff=${diff}ms)`);

        if (count == 10) {
            clearInterval(i);
            next();
        }
        t = +new Date();
    }, speed);
}, 2000);


Tests.registerAsync("Global.setTimeout (clear)", function(next) {
    var t = setTimeout(function() {
        throw new Error("Timeout should have been cleared");
    }, 100);

    setTimeout(function() {
        clearTimeout(t);
    }, 20);

    setTimeout(function() {
        next();
    }, 100 + TIMEOUT_THRESHOLD);

}, 1000);

Tests.registerAsync("setImmediate", function(next) {
    setImmediate(function() {
        next();
    });
}, 1000);  
