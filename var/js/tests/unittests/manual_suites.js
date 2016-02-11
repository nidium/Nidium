var TestsRunner = require('../../../../nativejscore/var/js/tests/testrunner.js');

var suites = [
    'Navigator/Navigator_language.js',
    'AV/getcontext.js',
    'AV/SourceNode.js',
    'AV/CustomeNode.js',
];

var tr = new TestsRunner();
tr.load(suites);
tr.run(function() {
    tr.report();
});
