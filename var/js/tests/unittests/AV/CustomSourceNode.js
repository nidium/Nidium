var dsp;
var source;
var customProcessor;
Tests.register("CustomSourceNode.create", function() {
    dsp = Audio.getContext();
    source = dsp.createNode("custom-source", 0, 2);
    customProcessor = dsp.createNode("custom", 2, 2);
    var target = dsp.createNode("target", 2, 0);

    Assert(source instanceof AudioNode);
    
    dsp.connect(source.output[0], customProcessor.input[0]);
    dsp.connect(source.output[1], customProcessor.input[1]);

    dsp.connect(customProcessor.output[0], target.input[0]);
    dsp.connect(customProcessor.output[1], target.input[1]);
});

Tests.registerAsync("CustomSourceNode.play", function(next) {
    source.onplay = function() {
        source.onplay = null;
        next();
    }

    source.play();
}, 5000);

Tests.registerAsync("CustomSourceNode.pause", function(next) {
    source.onpause = function() {
        source.onpause = null;
        next();
    }

    source.pause();
}, 5000);

Tests.registerAsync("CustomSourceNode.stop", function(next) {
    source.onstop = function() {
        source.onstop = null;
        next();
    }

    source.stop();
}, 5000);

Tests.registerAsync("CustomSourceNode.assignInit", function(next) {
    source.onmessage = function(msg) {
        source.stop();
        Assert.equal(msg.data, "init");
        next();
    }

    source.assignInit(function() {
        this.send("init");
    });

    // Custom node init method is lazy-called when the node 
    // needs to perform some work, thus call play() to 
    // for the init function to be called
    source.play();
}, 5000);

Tests.registerAsync("CustomSourceNode.assignSetter & set", function(next) {
    source.onmessage = function(msg) {
        source.onmessage = null;

        Assert.equal(msg.data.key, "foo");
        Assert.equal(msg.data.value, "bar");

        next();
    }

    source.assignSetter(function(key, value) {
        this.send({"key": key, "value": value});
    });

    source.set("foo", "bar");

}, 5000);

Tests.registerAsync("CustomSourceNode.set & get (threaded)", function(next) {
    source.onmessage = function(msg) {
        source.onmessage = null;

        Assert.equal(msg.data.set, true);
        Assert.equal(msg.data.get, true); 

        next();
    }

    source.assignSetter(function(key, value) {
        if (key == "test") {
            console.log("got test");
            var out = {};

            this.set("foo", "bar");

            if (this.get("test") == "me") {
                out["set"] = true;
            }

            if (this.get("foo") == "bar") {
                out["get"] = true;
            }

            this.send(out);
        }
    });

    source.set("test", "me");

}, 5000);

Tests.registerAsync("CustomSourceNode.assignProcessor", function(next) {
    customProcessor.onmessage = function(msg) {
        customProcessor.onmessage = null;
        customProcessor.assignProcessor(null);
        source.assignProcessor(null);

        Assert.equal(msg.data, true);

        source.stop();

        next();
    }

    customProcessor.assignProcessor(function(frames, scope) {
        var ok = false;
        for (var i = 0; i < frames.data.length; i++) {
            if (frames.data[i][frames.size - 1] == 42) {
                ok = true;
            }
        } 
        this.send(ok);
    });

    source.assignProcessor(function(frames, scope) {
        for (var i = 0; i < frames.data.length; i++) {
            frames.data[i][frames.size - 1] = 42;
        } 
    });

    source.play();

}, 5000);

Tests.registerAsync("CustomSourceNode.assignSeek", function(next) {
    source.onmessage = function(msg) {
        source.onmessage = null;
        source.assignSeek(null);

        source.stop();

        Assert.equal(msg.data.seek, 15);

        next();
    }

    source.assignSeek(function(time, scope) {
        this.send({"seek": time});
    });

    setTimeout(function() {
        source.position = 15.0;
    }, 500);

    source.play();
}, 5000);
