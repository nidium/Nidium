var running = false;

function test_Thread_complex() {
	var t = new Thread(function(...n) {
		var p = 0;
		for (var i = 0; i < 20000000; i++) {
			if (i % 10000 == 0) this.send(i);
			p++;
		}
		return n;
	});

	t.onmessage = function(e) {
		var i = e.data, v = i * 100 / 20000000;
		console.log( Math.round(v) + "%", v );
	};

	t.oncomplete = function(e) {
		console.log("complete");
		if (e.data){
			console.log("i'm done with", e.data);
		}
	};


	running = true;
	t.start(5, 6, 6, 9);
	while(running) {
		//loop
	}
	return true;
}


run_unit_tests = function( ) {
	return test_Thread_complex( );
}
