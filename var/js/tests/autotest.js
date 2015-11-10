var COUNTERS = { examples: 0, fails : 0};
try {

test_Console_methods_Console_log_0 = function() {
console.log( 'Nidium' );
};

test_Console_methods_Console_info_0 = function() {
console.info( 'Nidium, A new breed of browser' );
};

test_Console_methods_Console_error_0 = function() {
console.error( 'Nidium, Cannot display HTML' );
};

test_Console_methods_Console_warn_0 = function() {
console.warn( 'Nidium, Improving the web' );
};

test_Console_base_Console_0 = function() {
console.log( 'Nidium' );
};

test_System_methods_System_getOpenFileStats_0 = function() {
console.log( JSON.stringify( System.getOpenFileStats() ));
};

} catch( err ) {
	COUNTERS.fails = 1;
	console.log('Syntax error in example code; Go fix that!' + err.message );
}
if ( ! COUNTERS.fails ) {
	try {
		var fns = ['test_Console_methods_Console_log_0', 'test_Console_methods_Console_info_0', 'test_Console_methods_Console_error_0', 'test_Console_methods_Console_warn_0', 'test_Console_base_Console_0', 'test_System_methods_System_getOpenFileStats_0'];
		for (var i in fns ) {
			console.log('running: ' + fns[i] );
			global[fns[i]]();
			COUNTERS.examples++;
		}
	} catch ( err ) {
		console.log( err.message );
		COUNTERS.fails++;
	}
	if ( COUNTERS.fails > 0 ) {
		console.log( COUNTERS.fails + ' examples did not run correctly! Go fix them!' );
	} else {
		console.log( "These " + COUNTERS.examples + " examples seem to be ok!" );
	}
}
