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

console.log(h.zz);
w.revoke();
console.log(h.zz);

*/

function assertEquals(a, b){
	if (a === b) console.log(a, "===", b);
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
                console.log("100 iterations of setTimeout(0) took " +
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




/* console.log unit test */

var l = {
	x : 5,
	y : 7,
	z : function(){
		console.log("fdsf");
	}
};

l.parent = {
	h : 45,
	g : 95,

	zref : l.z,

	child : {
		k : 9,
		p : null,
		u : undefined,
		a : [5, 8, {k:5, cc:function(){console.log("fdf");}}, 9, 8],
		b : true,
		mm : false,

		child : {
			q : "fsdf",

			r : function(){
				return false;
			},
			y : l,
			m : Math.random
		}
	}

};

console.log(l);






