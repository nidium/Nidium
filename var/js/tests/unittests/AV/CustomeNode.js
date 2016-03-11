var dsp, node;
Tests.register("CustomNode.create", function() {
    dsp = Audio.getContext();
    node = dsp.createNode("custom", 2, 2);
    Assert(node instanceof AudioNode);
});

Tests.registerAsync("CustomNode.set(key, value)", function(next) {
    node.assignSetter(function(key, value) {
        this.send({"key": key, "value": value});
    })

    node.set("foo", "bar");
    node.addEventListener("message", function(msg) {
        node.assignSetter(null);
        Assert.equal(msg.data.key, "foo", "Key isn't \"foo\"")
        Assert.equal(msg.data.value, "bar", "Value isn't \"bar\"");
        next()
    });
}, 5000);

Tests.registerAsync("CustomNode.set(object)", function(next) {
    node.assignSetter(function(key, value) {
        this.send({"key": key, "value": value});
    });

    node.set({"ILove": "Rock'n'Roll"});

    node.addEventListener("message", function(msg) {
        node.setter = null;
        Assert(msg.data.key == "ILove" && msg.data.value == "Rock'n'Roll");
        next();
    });
}, 5000);

Tests.register("CustomNode.set(invalid data)", function() {
    var thrown = false;
    try {
        node.set(null);
    } catch (e) {
        thrown = true;
    }

    Assert(thrown, ".set(null) didn't throw an error");


    thrown = false;
    try {
        node.set(null, "foo");
    } catch (e) {
        thrown = true;
    }

    Assert(thrown, ".set(null, \"foo\") didn't throw an error");
});
