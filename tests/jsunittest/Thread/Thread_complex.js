/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.register("Thread.complex", function() {
	var running = false;

	var t = new Thread(function(...n) {
		var p = 0;
		console.log("starting thread");
		for (var i = 0; i < 20000000; i++) {
			if (i % 10000 == 0) this.send(i);
			p++;
		}
		return n;
	});

	t.onmessage = function(e) {
		Assert.equal(running, false);	
		var i = e.data, perc = i * 100 / 20000000;
		console.log( Math.round(perc) + "%", v );
	};

	t.oncomplete = function(e) {
		console.log("complete");
		if (e.data){
			Assert.equal(e.data, 20000000);
			running = false;
			console.log("done");
		}
	};

	running = true;
	t.start(5, 6, 6, 9);
	while(running) {
		//loop
	}
	Assert.equal(running, false);
});

