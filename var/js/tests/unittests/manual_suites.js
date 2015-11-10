require('../../../../nativejscore/var/js/tests/testrunner.js');

var suites = [
		__dirname + 'System/System_getOpenFileStats.js',
];

var tr = new TestRunner(); 
tr.do_test_suites(suites);
tr.report();
