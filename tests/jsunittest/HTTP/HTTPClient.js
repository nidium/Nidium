/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

const HTTP_TEST_DATA = "foobar";

function testResponse(ev) {
    Assert.equal(typeof ev.error, "undefined", "Error should be undefined");
    Assert.equal(typeof ev, "object", "Invalid event object");
}

Tests.register("HTTP.request (file URL)", function() {
    Assert.throws(function() {
        new HTTP("file:///tmp/");
    });
});

Tests.registerAsync("HTTP.request (errors)", function(next) {
    var h = new HTTP("http://127.0.0.1:1245/");

    h.addEventListener("error", function(ev) {
        Assert.notEqual(typeof ev.error, "undefined", "Error should be defined");
        next();
    });

    h.addEventListener("response", function(ev) {
        throw new Error("Was not expecting a response event " + JSON.stringify(ev));
    });

    h.request();
}, 5000);

Tests.registerAsync("HTTP.request (404 error)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/404");

    h.addEventListener("error", function(ev) {
        throw new Error("Was not expecting an error event " + JSON.stringify(ev));
    });

    h.addEventListener("response", function(ev) {
        Assert.equal(ev.statusCode, 404, "Status code received should be 404");
        next();
    });

    h.request();
}, 5000);

Tests.registerAsync("HTTP.request (header event)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/echo");

    h.addEventListener("error", function(ev) {
        throw new Error("Was not expecting an error event " + JSON.stringify(ev));
    });

    h.addEventListener("headers", function(ev) {
        Assert.equal(ev.statusCode, 200, "Status code received should be 200");
        Assert.equal(ev.headers.foo, "bar", "Headers key \"foo\" value should be \"bar\"");

        h.stop();

        next();
    });

    h.request({"headers": {"foo": "bar"}});
}, 5000);

Tests.registerAsync("HTTP.request (progress with event content-length)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/progress");
    var counter = 0;

    h.addEventListener("progress", function(ev) {
        counter++;
    });

    h.addEventListener("error", function(ev) {
        throw new Error("Was not expecting an error event " + JSON.stringify(ev));
    });

    h.addEventListener("response", function(ev) {
        testResponse(ev);
        Assert(counter >= 2, "Progress event should have been called at least twice");

        next();
    });

    h.request();
}, 5000);

Tests.registerAsync("HTTP.request (progress event without content-length)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/progress-no-content-length");
    var counter = 0;

    h.addEventListener("progress", function(ev) {
        counter++;
    });

    h.addEventListener("error", function(ev) {
        throw new Error("Was not expecting an error event " + JSON.stringify(ev));
    });

    h.addEventListener("response", function(ev) {
        testResponse(ev);
        Assert(counter >= 2, "Progress event should have been called at least twice");

        next();
    });

    h.request();
}, 5000);

Tests.registerAsync("HTTP (short hand)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/hello", function(ev) {
        testResponse(ev);
        Assert.equal(ev.data, "Hello World !", "Unexpected data");
        next();
    });
}, 5000);

Tests.registerAsync("HTTP.request (short hand HTTPS)", function(next) {
    new HTTP(HTTP_TEST_SECURE_URL + "/hello", function(ev) {
        testResponse(ev);
        Assert.equal(ev.data, "Hello World !", "Unexpected data");
        next();
    });
}, 5000);

Tests.registerAsync("HTTP (short hand mode with options)", function(next) {
    new HTTP(HTTP_TEST_URL,{
        "headers": {
            "foo": "bar"
        },
        "path": "/http/echo?data=" + HTTP_TEST_DATA,
    }, function(ev) {
        testResponse(ev);

        Assert.equal(typeof ev.headers, "object", "Invalid header");
        Assert.equal(ev.headers.foo, "bar", "Invalid header value");
        Assert.equal(ev.data, "/http/echo?data=" + HTTP_TEST_DATA, "Invalid content");

        next();
    });
}, 5000);

Tests.registerAsync("HTTP.request (event response)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/hello");

    h.addEventListener("response", function(ev) {
        testResponse(ev);
        Assert.equal(ev.data, "Hello World !", "Unexpected data");
        next();
    });

    h.addEventListener("error", function(ev) {
        throw new Error("Was not expecting an error event " + JSON.stringify(ev));
    });

    h.request();
}, 5000);

Tests.registerAsync("HTTP.request (constructor options)", function(next) {
    var h = new HTTP(HTTP_TEST_URL, {
        "headers": {
            "foo": "bar"
        },
        "path": "/http/echo?data=" + HTTP_TEST_DATA,
    });

    h.addEventListener("response", function(ev) {
        testResponse(ev);

        Assert.equal(typeof ev.headers, "object", "Invalid header");
        Assert.equal(ev.headers.foo, "bar", "Invalid header value");
        Assert.equal(ev.data, "/http/echo?data=" + HTTP_TEST_DATA, "Invalid content");

        next();
    });

    h.addEventListener("error", function(ev) {
        throw new Error("Was not expecting an error event " + JSON.stringify(ev));
    });

    h.request();
}, 5000);

Tests.registerAsync("HTTP.request (GET with options)", function(next) {
    var h = new HTTP(HTTP_TEST_URL);

    h.addEventListener("response", function(ev) {
        testResponse(ev);

        Assert.equal(typeof ev.headers, "object", "Invalid header");
        Assert.equal(ev.headers.foo, "bar", "Invalid header value");
        Assert.equal(ev.data, "/http/echo?data=" + HTTP_TEST_DATA, "Invalid content");

        next();
    });

    h.addEventListener("error", function(ev) {
        throw new Error("Was not expecting an error event " + JSON.stringify(ev));
    });

    h.request({
        "headers": {
            "foo": "bar"
        },
        "path": "/http/echo?data=" + HTTP_TEST_DATA,
    });
}, 5000);

Tests.registerAsync("HTTP.request (POST with \"integer\" data)", function(next) {
    var h = new HTTP(HTTP_TEST_URL);

    h.addEventListener("response", function(ev) {
        testResponse(ev);

        Assert.equal(typeof ev.headers, "object", "Invalid header");
        Assert.equal(ev.headers.foo, "bar", "Invalid header value");
        Assert.equal(ev.data, 1234, "Invalid content");

        next();
    });

    h.addEventListener("error", function(ev) {
        throw new Error("Was not expecting an error event " + JSON.stringify(ev));
    });

    h.request({
        "headers": {
            "foo": "bar"
        },
        "method": "POST",
        "data": 1234,
        "path": "/http/echo"
    });
});

Tests.registerAsync("HTTP.request (POST with options)", function(next) {
    var h = new HTTP(HTTP_TEST_URL);

    h.addEventListener("response", function(ev) {
        testResponse(ev);

        Assert.equal(typeof ev.headers, "object", "Invalid header");
        Assert.equal(ev.headers.foo, "bar", "Invalid header value");
        Assert.equal(ev.data, HTTP_TEST_DATA, "Invalid content");

        next();
    });

    h.addEventListener("error", function(ev) {
        throw new Error("Was not expecting an error event " + JSON.stringify(ev));
    });

    h.request({
        "headers": {
            "foo": "bar"
        },
        "method": "POST",
        "data": HTTP_TEST_DATA,
        "path": "/http/echo"
    });
}, 5000);

Tests.registerAsync("HTTP.request (utf8)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/utf8", function(ev) {
        testResponse(ev);
        Assert.equal(ev.data, "♥ nidium ♥", "Unexpected data");
        next();
    });
}, 5000);

// FIXME : This test fail because nidium does not
// parse the charset in the content-type header
/*
Tests.registerAsync("HTTP.request (ASCII)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/iso", function(err, ev) {
        testResponse(err, ev);
        Assert.equal(ev.data, "\xE9", "Unexpected data");
        next();
    });
}, 5000);
*/

Tests.registerAsync("HTTP.request (json eval)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/json");

    h.addEventListener("response", function(ev) {
        testResponse(ev);
        Assert.equal(ev.data.hello, "world", "Invalid JSON data received");
        next();
    });

    h.addEventListener("error", function(ev) {
        throw new Error("Was not expecting an error event " + JSON.stringify(ev));
    });

    h.request({
        "eval": true,
    });
}, 5000);

Tests.registerAsync("HTTP.request (too small content length)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/toosmall-content-length", function(ev) {
        testResponse(ev);
        next();
    });
}, 5000);

// FIXME : This test fail because of an "http_server_disconnected" error
/*
Tests.registerAsync("HTTP.request (too large content length)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/toolarge-content-length", function(err, ev) {
        testResponse(err, ev);
        console.log(JSON.stringify(arguments));
        next();
    });
});
*/

Tests.registerAsync("HTTP.request (ArrayBuffer data)", function(next) {
    var data = new ArrayBuffer(8);
    var view = new Uint8Array(data);
    var start = 42;

    for (var i = 0; i < view.length; i++) {
        view[i] = start + i;
    }

    var h = new HTTP(HTTP_TEST_URL + "/echo", {
        method:"POST",
        data: data,
        eval: true,
        headers: {
            "content-type": "application/octet-stream" // Force binary response
        }
    }, function(ev) {
        testResponse(ev);

        Assert.equal(ev.data instanceof ArrayBuffer, true, "Response is not an ArrayBuffer " + typeof ev.data);

        var bufView = new DataView(ev.data);
        for (var i = 0; i < bufView.length; i++) {
            Assert.equal(bufView.getUint8(i), start + i, "Unexpected character at position " + i)
        }

        next();
    });
}, 5000);

Tests.registerAsync("HTTP.request (ArrayBuffer response)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/empty-content-type", function(ev) {
        testResponse(ev);

        Assert.equal(ev.data instanceof ArrayBuffer, true, "Response is not an ArrayBuffer");

        var bufView = new DataView(ev.data);
        var str = "This request has an empty content-type";
        for (var i = 0; i < str.length; i++) {
            Assert.equal(bufView.getUint8(i), str.charCodeAt(i), "Unexpected character at position " + i)
        }

        next();
    });
}, 5000);

Tests.registerAsync("HTTP.stop", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/progress");

    h.addEventListener("response", function(ev) {
        testResponse(ev);
        next();
    });

    h.addEventListener("error", function(ev) {
        Assert.equal(ev.code, 3, "Error code should be 3");
    });

    setTimeout(function() {
        h.stop();
        h.request();
    }, 20);
}, 4000);

// FIXME : Need to use a setTimeout otherwise a "disconnected" error is fired.
// It's because HTTP.cpp fire the "response" event and close the connection right after.
Tests.registerAsync("HTTP.request (multiple request)", function(next) {
    var h = new HTTP(HTTP_TEST_URL + "/hello");
    var counter = 0;

    h.addEventListener("error", function(err) {
        throw new Error("Was not expecting an error event " + JSON.stringify(err));
    });

    h.addEventListener("response", function(ev) {
        testResponse(ev);

        if (counter == 0) {
            Assert.equal(ev.data, "Hello World !", "Unexpected data");

            setTimeout(function() {
                h.request({path: "/http/echo", method: "POST", data: "hello"});
            }, 1);
        } else {
            Assert.equal(ev.data, "hello", "Unexpected data");

            next();
        }

        counter++;
    });

    h.request();
}, 4000);
