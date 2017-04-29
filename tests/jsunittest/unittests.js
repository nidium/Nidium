/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var TestsRunner = require('../testrunner.js');
var testsServerHost = "tests.nidium.com";
var testsServerPort = 8888
var testsServerSecurePort = 8443

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

if (args["save"]) {
    tr.setSaveVisualDiff(true);
}

if (args["tests-server"]) {
    testsServerHost = args["tests-server"];
}

if (args["tests-server-port"]) {
    testsServerPort = args["tests-server-port"];
}

if (args["tests-server-secure-port"]) {
    testsServerSecurePort = args["tests-server-secure-port"];
}

if (args["local-tests-server"]) {
    testsServerHost = "127.0.0.1";
}

const TESTS_SERVER_HOST = testsServerHost;
const TESTS_SERVER_PORT = testsServerPort;
const TESTS_SERVER_SECURE_PORT = testsServerSecurePort;

const WS_TEST_URL = `ws://${TESTS_SERVER_HOST}:${TESTS_SERVER_PORT}/ws`;
const WS_TEST_SECURE_URL = `wss://${TESTS_SERVER_HOST}:${TESTS_SERVER_SECURE_PORT}/ws`;

const HTTP_TEST_URL = `http://${TESTS_SERVER_HOST}:${TESTS_SERVER_PORT}/http`
const HTTP_TEST_SECURE_URL = `https://${TESTS_SERVER_HOST}:${TESTS_SERVER_SECURE_PORT}/http`

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
        'VM.js',
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
            'OS.js',
            'Image.js',
            'Canvas.js',
            'DB.js', // Only for frontend, because server does not support "cache://"
            'Window/__nidium__.js',
        ]);
    }
};

tr.load(Suites);
tr.run(function() {
    tr.report(exit=true);
});

