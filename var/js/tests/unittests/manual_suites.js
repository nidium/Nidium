var TestsRunner = require('../../../../nativejscore/var/js/tests/testrunner.js');

var Suites = [
    'Navigator/Navigator_language.js',
    'AV/getcontext.js',
    'AV/SourceNode.js',
    'AV/CustomeNode.js',
];

var tr = new TestsRunner();
tr.load(Suites);
tr.run(function() {
    tr.report(exit=true);
});

