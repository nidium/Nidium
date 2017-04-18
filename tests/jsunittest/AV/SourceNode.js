/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var dsp;
var node;

Tests.register("SourceNode create", function() {
    dsp = Audio.getContext();
    node = dsp.createNode("source", 0, 2);
    Assert(node instanceof AudioNodeSource);
});

Tests.register("SourceNode with invalid input", function() {
    Assert.throws(function() {
        dsp.createNode("source", "foobar", "foobar");
    });
});

Tests.registerAsync("SourceNode open inexistent file", function(next) {
    node.addEventListener("error", function(ev) {
        // Remove ready event otherwise it will be fired by the next test
        node.removeEventListener("error");
        node.removeEventListener("ready");

        Assert.strictEqual(ev.errorCode, 1, "Invalid error code returned")

        next();
    });

    node.addEventListener("ready", function() {
        node.removeEventListener("error");
        node.removeEventListener("ready");

        throw new Error("Node fired onready callback oO");

        next();
    });

    node.open("invalidfile")
}, 5000);

Tests.registerAsync("SourceNode open invalid file", function(next) {
    node.addEventListener("error", function(ev) {
        node.removeEventListener("error");
        node.removeEventListener("ready");

        throw new Error("Node fired error callback oO");
    });

    node.addEventListener("ready", function() {
        node.removeEventListener("error");
        node.removeEventListener("ready");

        throw new Error("Node fired onready callback oO");

        next();
    });

    Assert.throws(function() {
        node.open("/tmp/foo.mp3");
    });

    node.removeEventListener("error");
    node.removeEventListener("ready");

    next();
}, 5000);

Tests.registerAsync("SourceNode open file", function(next) {
    node.open("AV/test.mp3")
    node.addEventListener("error", function(e) {
        node.removeEventListener("error");
        throw new Error("Failed to open media file : " + JSON.stringify(e));
        next();
    });

    node.addEventListener("ready", function() {
        next();
    });
}, 5000);

Tests.register("SourceNode MetaData", function() {
    Assert.equal(JSON.stringify(node.metadata), 
                '{"title":"The Sound of Epicness (ID: 358232)","artist":"larrylarrybb","album":"Newgrounds Audio Portal - http://www.newgrounds.com/audio","track":"01/01","streams":[{}]}',
                "File MetaData does not match");
});

Tests.registerAsync("SourceNode play event", function(next) {
    node.addEventListener("play", function() {
        node.removeEventListener("play");
        next();
    });

    node.play();
}, 5000);

Tests.registerAsync("SourceNode pause event", function(next) {
    node.addEventListener("pause", function() {
        node.removeEventListener("pause");
        next();
    });

    node.pause();
}, 5000);

/*
Tests.registerAsync("SourceNode seek", function(next) {
    node.position = 20;
    node.play();

    // Seeking is async
    setTimeout(function() {
        // Truncate the position, because node.position returns a float
        // and seek is not accurate to the milisecond
        Assert.strictEqual((node.position | 0), 20, "Source position is invalid. Position : " + node.position + " (was expecting 20)");
        next();
    }, 4000);
}, 5000);
*/

Tests.registerAsync("SourceNode stop", function(next) {
    node.addEventListener("stop", function() {
        next();
    });

    node.stop();
}, 5000);

Tests.registerAsync("SourceNode close", function(next) {
    node.close();
    setTimeout(function() {
        next();
    }, 1000);
}, 5000);
