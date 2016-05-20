/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var running = false;

Tests.register("Thread.simple", function() {
	var t = new Thread(function( args ) {
		//something loud and heavy;
		return args * 100;
	});

	t.oncomplete = function(e) {
		Assert.equal(running, true);
		Assert.equal(e.data, 66600) ;
		running = false;
	};

	running = true;
	t.start(666); // start the new job with "666"" as a parameter
	while(running) {
		//loop
	}
	Assert.equal(running, false);
});

