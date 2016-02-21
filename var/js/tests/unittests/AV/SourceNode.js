var dsp;
var node;

Tests.register("SourceNode create", function() {
    dsp = Audio.getContext();
    node = dsp.createNode("source", 0, 2);
    Assert(node instanceof AudioNode);
});

Tests.register("SourceNode with invalid input", function() {
    try {
        dsp.createNode("source", "foobar", "foobar");
    } catch (e) {
        return;
    }

    Assert("Exception was expected");
});

Tests.registerAsync("SourceNode open invalid file", function(next) {
    node.open("invalidfile")
    node.onerror = function(code, err) {
        // XXX : NativeStream triggers two error
        // workaround this issue otherwise callback is executed twice
        node.onerror = null;
        Assert.strictEqual(code, 1, "Invalid error code returned")
        next();
    };

    node.onready = function() {
        Assert(false, "Node fired onready callback oO");
        next();
    }
}, 5000);

Tests.registerAsync("SourceNode open file", function(next) {
    node.open("AV/test.mp3")
    node.onerror = function(e) {
        node.onerror = null;
        Assert(false, "Failed to open media file");
        next();
    };

    node.onready = function() {
        next();
    }
}, 5000);

Tests.register("SourceNode MetaData", function() {
    Assert(JSON.stringify(node.metadata) == '{"title":"The Sound of Epicness (ID: 358232)","artist":"larrylarrybb","album":"Newgrounds Audio Portal - http://www.newgrounds.com/audio","track":"01/01"}',
        "File MetaData does not match");
});

Tests.registerAsync("SourceNode play event", function(next) {
    node.onplay = function() {
        next();
    }

    node.play();
}, 5000);

Tests.registerAsync("SourceNode pause event", function(next) {
    node.onpause = function() {
        next();
    }

    node.pause();
}, 5000);

Tests.registerAsync("SourceNode seek", function(next) {
    node.position = 20;

    // Seeking is async
    setTimeout(function() {
        // Truncate the position, because node.position returns a float
        // that is not accurate to the miliseconds 
        Assert.strictEqual((node.position | 0), 20, "Source position is invalid");
        next();
    }, 200);
}, 5000);

Tests.registerAsync("SourceNode stop", function(next) {
    node.onstop = function() {
        next();
    }

    node.stop();
}, 5000);
