var running = false;

function test_Thread_simple() {

	var t = new Thread(function(foo) {
	   console.log("something loud and heavy on " +  foo );
	   return foo + "'ed";
	});

	t.oncomplete = function(e) {
	   // executed when the job is done 
		running = false;
	   console.log(e.data);
	};

	running = true;
	t.start("bar"); // start the new job with "bar" as a parameter
	while(running) {
		//loop
	}
	return true;
}


run_unit_tests = function( ) {
	return test_Thread_simple( );
}
