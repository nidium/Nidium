var test_System_getOpenFileStats = function(){
	var success = false;
	var count = 0;
	var stats = System.getOpenFileStats();
	for (var k in stats) {
		if (typeof stats[k] === typeof 0 ) {
			count++;
		}
	}
	success = (count == 5 );
	success = success && stats.hasOwnProperty('cur');
	success = success && stats.hasOwnProperty('max');
	success = success && stats.hasOwnProperty('open');
	success = success && stats.hasOwnProperty('sockets');
	success = success && stats.hasOwnProperty('files');

	return success;
}

run_unit_tests = function( ) {
	return test_System_getOpenFileStats();
}

