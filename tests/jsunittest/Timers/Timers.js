Tests.registerAsync("setTimeout", function(next) {
    var start = Date.now();
    setTimeout(function() {
        var duration = Date.now() - start;
        Assert(duration > 490 && duration < 510, "setTimeout did not run at the expected time (" + duration + "ms instead of 500ms)");
        next();
    }, 500);
}, 1000);

Tests.registerAsync("clearTimeout", function(next) {
    var timer = setTimeout(function() {
        Assert(false, "clearTimeout didn't clear the timer");
    }, 100);

	setTimeout(function() {
		next();
	}, 200);

	clearTimeout(timer);
}, 1000);  

Tests.registerAsync("setInterval", function(next) {
	var counter = 0;
	var timer = setInterval(function() {
		counter++;
		if (counter == 5) {
			clearTimeout(timer);
			next();
		}
	}, 10);
}, 1000);  

Tests.registerAsync("setImmediate", function(next) {
	setImmediate(function() {
		next();
	});
}, 1000);  
