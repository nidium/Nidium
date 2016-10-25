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

var tr = new TestsRunner();
var Suites = [];

if (args["regex"]) {
    tr.setTestRegex(args["regex"]);
}

if (args["file"]) {
    Suites.push(args["file"]);
} else {
    Suites = [
        'Global/Global_require.js',
        'File/File.js',
        'Process/process.js',
        'HTTP/HTTPServer.js',
        'HTTP/HTTPClient.js',
        //'Modules/Module_libHello.js',

        'Thread.js',

        'JS/JS_btoa.js',
        'JS/JS_timers.js',
        'Socket/basic.js',
        'Socket/websocket-client.js',
        'Socket/websocket-server.js'
    ];

    if (args["frontend"]) {
        Suites = Suites.concat([
            'AV/AudioContext.js',
            'AV/SourceNode.js',
            'AV/CustomeNode.js',
            'AV/CustomSourceNode.js',
            'AV/AudioThread.js',
            'AV/GainNode.js',
            'AV/Video.js',
            'Image.js',
            'DB.js', // Only for frontend, because server does not support "cache://"
        ]);
    }
};

tr.load(Suites);
tr.run(function() {
    tr.report(exit=true);
});

