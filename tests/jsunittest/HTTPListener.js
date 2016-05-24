Tests.register("HTTPServer constructor", function() {
    new HTTPListener(4241, "127.0.0.1");
});

Tests.register("HTTPServer reusePort option", function() {
    var h = new HTTPListener(4242, "0.0.0.0", {"reusePort": true});
    Assert.doesNotThrow(function() {
        new HTTPListener(4242, "0.0.0.0", {"reusePort": true})
    });
});
