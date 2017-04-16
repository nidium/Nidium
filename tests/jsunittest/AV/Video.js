/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var c = new Canvas(1280, 1024);
var ctx = c.getContext("2d");
var dsp = Audio.getContext();
var target = dsp.createNode("target", 2, 0);
document.canvas.add(c);
var video = new Video(c);

Tests.registerAsync("Video.open(invalid file)", function(next) {
    video.addEventListener("error", function(ev) {
        video.removeEventListener("error");
        Assert.strictEqual(ev.errorCode, 1, "Invalid error code returned")
        next();
    });

    video.open("invalid");
}, 5000);

Tests.registerAsync("Video.open", function(next) {
    video.close();

    video = new Video(c);

    video.addEventListener("ready", function() {
        video.removeEventListener("ready");
        video.removeEventListener("error");
        next();
    });
    video.addEventListener("error", function(ev) {
        video.removeEventListener("ready");
        video.removeEventListener("error");

        Assert("Failed to open video");
    });
    video.open("AV/video.ogg");
}, 5000);

Tests.register("Video.duration", function() {
    Assert.strictEqual(video.duration, 6, "Video duration is incorrect");
});

Tests.register("Video.bitrate", function() {
    Assert.strictEqual(video.bitrate, 671356, "Unexpected bitrate");
});

Tests.register("Video.metadata", function() {
    Assert(JSON.stringify(video.metadata) == '{"streams":[{"ENCODER":"ffmpeg2theora 0.19"},{"ENCODER":"ffmpeg2theora 0.19"}]}',
        "File MetaData does not match");
});

Tests.register("Video.getAudioNode", function() {
    var node = video.getAudioNode();
    Assert(node instanceof AudioNodeSourceVideo, "Video should have an audio node");
}, 5000);

Tests.registerAsync("Video.play", function(next) {
    video.addEventListener("play", function() {
        video.removeEventListener("play");
        next();
    });

    video.play();
}, 5000);

Tests.registerAsync("Video.addEventListener(\"frame\")", function(next) {
    video.addEventListener("frame", function() {
        video.removeEventListener("frame");
        next();
    });
}, 5000);

Tests.registerAsync("Video.pause", function(next) {
    video.addEventListener("pause", function() {
        video.removeEventListener("pause");
        next();
    });

    video.pause();
}, 5000);

Tests.registerAsync("Video.position", function(next) {
    video.position = 1;
    setTimeout(function() {
        // Seek is done to the closest keyframe, 1.240 for our video
        Assert.strictEqual(video.position, 1.24, "Video position is invalid. Position :" + video.position + " (was expecting 1.24)");
        next();
    }, 500);
}, 5000);

Tests.registerAsync("Video.nextFrame", function(next) {
    video.nextFrame();
    video.addEventListener("frame", function() {
        video.removeEventListener("frame");
        Assert.strictEqual(video.position, 1.28, "Video position is invalid. Position " + video.position + " (was expecting 1.28)");
        next();
    }, 500);
}, 5000);

// XXX : prevFrame is jumping one frame :/
/*
Tests.registerAsync("Video.prevFrame", function(next) {
    video.prevFrame();
    video.addEventListener("frame", function() {
        video.removeEventListener("frame");
        Assert.strictEqual(video.position, 1.24, "Video position is invalid");
        next();
    }, 200);
}, 5000);
*/

Tests.registerAsync("Video.stop", function(next) {
    video.addEventListener("stop", function() {
        video.removeEventListener("stop");
        next();
    });

    video.stop();
}, 5000);

Tests.registerAsync("Video.close", function(next) {
    video.close();
    setTimeout(function() {
        next();
    }, 1000);
}, 5000);
