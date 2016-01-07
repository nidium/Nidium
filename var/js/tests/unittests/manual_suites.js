require('../../../../nativejscore/var/js/tests/testrunner.js');

var suites = [
    'Navigator/Navigator_language.js',
];

var tr = new TestRunner(); 
tr.do_test_suites(suites);
tr.report();
