require('../testrunner.js');


var suites = [
//		__dirname + 'File/File_listDirs.js',
		__dirname + 'File/File_properties.js',

		__dirname + 'Thread/Thread_simple.js',
		__dirname + 'Thread/Thread_complex.js',

		__dirname + 'JS/JS_btoa.js',
		__dirname + 'JS/JS_timers.js',

];
var tr = new TestRunner(); 
tr.do_test_suites(suites);
tr.report();
