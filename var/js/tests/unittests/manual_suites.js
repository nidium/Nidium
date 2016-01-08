require('../testrunner.js');


var suites = [
//		__dirname + 'File/File_listDirs.js',
		__dirname + 'File/File_properties.js',
];
var tr = new TestRunner(); 
tr.do_test_suites(suites);
tr.report();
