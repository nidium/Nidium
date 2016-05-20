/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var TestsRunner = require('../testrunner.js');

var Suites = [
    /* Nidium Core */
    'File/File_listFiles.js',
    'File/File_properties.js',
    'Process/process.js',

    //'Thread/Thread_simple.js',
    //'Thread/Thread_complex.js',

    'JS/JS_btoa.js',
    'JS/JS_timers.js',

    /* Nidium */
    'Navigator/Navigator_language.js',
    'AV/AudioContext.js',
    'AV/SourceNode.js',
    'AV/CustomeNode.js',
    'AV/CustomSourceNode.js',
    'AV/AudioThread.js',
    'AV/GainNode.js',
    'AV/Video.js',
];

var tr = new TestsRunner();
tr.load(Suites);
tr.run(function() {
    tr.report(exit=true);
});

