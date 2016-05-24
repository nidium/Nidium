/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var TestsRunner = require('../testrunner.js');

var args = {};
var gotKey = null;
var idx = 0;
for (var i = 0; i < process.argv.length; i++) {
    var value = process.argv[i];

    if (i == 0) {
        args["_file"] = value;
    } else if (value[0] == "-") {
        var skip = value[1] == "-" ? 2 : 1;
        gotKey = value.substr(skip, value.length);
        args[gotKey] = true;
    } else if (gotKey) {
        args[gotKey] = value;
        gotKey = null;
    } else {
        args[idx] = value;
        idx++;
    }

}

var Suites = [
    'File/File_listFiles.js',
    'File/File_properties.js',
    'Process/process.js',

    //'Thread/Thread_simple.js',
    //'Thread/Thread_complex.js',

    'JS/JS_btoa.js',
    'JS/JS_timers.js'
];

if (args["frontend"]) {
    Suites = Suites.concat([
        'Navigator/Navigator_language.js',
        'AV/AudioContext.js',
        'AV/SourceNode.js',
        'AV/CustomeNode.js',
        'AV/CustomSourceNode.js',
        'AV/AudioThread.js',
        'AV/GainNode.js',
        'AV/Video.js',
    ]);
}

var tr = new TestsRunner();
tr.load(Suites);
tr.run(function() {
    tr.report(exit=true);
});

