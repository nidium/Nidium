/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/*
var g = {
	val : 5,
	add : function(m){
		return m + this.val;
	},
	ctx : function(){
		this.zz = 40;
	}
};

var w = {
	wrapper : o,
	revoke : revoke
} = Membrane(g);


var h = new o.ctx();

echo(h.zz);
w.revoke();
echo(h.zz);

*/

function assertEquals(a, b){
	if (a === b) echo(a, "===", b);
}

/*

(function(){
	var timeouts = [];
	var messageName = "zero-timeout-message";

	function setZeroTimeout(fn){
		timeouts.push(fn);
		window.postMessage(messageName, "*");
	}

	function handleMessage(event) {
		if (event.source == window && event.data == messageName) {
			event.stopPropagation();
			if (timeouts.length > 0) {
				var fn = timeouts.shift();
				fn();
			}
		}
	}

	window.addEventListener("message", handleMessage, true);

	// Add the one thing we want added to the window object.
	window.setZeroTimeout = setZeroTimeout;
})();


        var i = 0;
        var startTime = Date.now();

        function test2() {
            if (++i == 100) {
                var endTime = Date.now();
                echo("100 iterations of setTimeout(0) took " +
                            (endTime - startTime) + " milliseconds.");
            } else {
                setTimeout(test2, 0);
            }
        }

        test2();



*/


z = [5, 8, 9, 10];

var b = [1, 2, ...z, 6];


/*
BenchThis("Native 1024x768 FillRect", 50000, function(i){
	canvas.fillRect(0, 0, 1024, 768);
});

BenchThis("Native 250x250 FillRect", 50000, function(i){
	canvas.fillRect(10, 10, 250, 250);
});
*/
