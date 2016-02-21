var TestsRunner = require('../testrunner.js');
var Suites = [
		'File/File_listDirs.js',
		'File/File_properties.js',

		//'Thread/Thread_simple.js',
		//'Thread/Thread_complex.js',

		'JS/JS_btoa.js',
		'JS/JS_timers.js',
];

var tr = new TestsRunner();
tr.load(Suites);
tr.run(function() {
    tr.report(exit=true);
});

