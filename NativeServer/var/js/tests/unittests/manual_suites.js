/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var TestRunner = require('../../../../nativejscore/var/js/tests/testrunner.js');

var Suites = [
	'System/System_getOpenFileStats.js',
];

var tr = new TestsRunner();
tr.load(Suites);
tr.run(function() {
    tr.report();
});

