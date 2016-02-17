var TestRunner = require('../../../../nativejscore/var/js/tests/testrunner.js');

var Suites = [
	'System/System_getOpenFileStats.js',
];

var tr = new TestsRunner();
tr.load(Suites);
tr.run(function() {
    tr.report();
});

