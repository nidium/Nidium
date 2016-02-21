Tests.register("Audio.getContext()", function() {
    var ctx = Audio.getContext();
    Assert(ctx instanceof AudioContext);
});

Tests.register("Audio.getContext() with params", function() {
    var ctx = Audio.getContext(1024, 2, 22050);
    Assert(ctx instanceof AudioContext);
});

Tests.register("Audio.getContext() with invalid buffer size", function() {
    try {
        var ctx = Audio.getContext(-1024);
    } catch (e) {
        Assert(e.message.indexOf("Unsuported buffer size") == 0);
        return;
    }

    Assert(false, "Exception was expected");
});

Tests.register("Audio.getContext() with invalid channels", function() {
    try {
        var ctx = Audio.getContext(1024, 64);
    } catch (e) {
        Assert(e.message.indexOf("Unsuported channels number") == 0);
        return;
    }

    Assert(false, "Exception was expected");
});

Tests.register("Audio.getContext() with invalid sample rate", function() {
    try {
        var ctx = Audio.getContext(1024, 32, 11025);
    } catch (e) {
        Assert(e.message.indexOf("Unsuported sample rate") == 0);
        return;
    }

    Assert(false, "Exception was expected");
});
