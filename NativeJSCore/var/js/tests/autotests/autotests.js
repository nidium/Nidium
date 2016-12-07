var TestsRunner = require('../../../../../nativejscore/var/js/tests/testrunner.js');
var Suites = ['auto_suites.js'];

var tr = new TestsRunner();
tr.load(Suites);
tr.run(function() {
	tr.report(exit=true);
});
