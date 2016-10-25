Tests.registerAsync("Thread (complete)", function(next) {
    var t = new Thread(function(args) {
        return args * 100;
    });

    t.oncomplete = function(e) {
        Assert.equal(e.data, 66600) ;
        next();
    };

    t.start(666);

}, 1000);

Tests.registerAsync("Thread (message)", function(next) {
    var t = new Thread(function(args) {

        this.send(args.foo);

        return true
    });

    t.onmessage = function(ev) {
        Assert.equal(ev.data, "bar");
        next();
    }

    t.start({"foo": "bar"});

}, 1000);

Tests.register("Thread (invalid function)", function() {
    Assert.throws(function() {
        var t = new Thread(1337);
        t.start();
    });
});

// XXX : This tests does not ensure that the error is actually propagated as
// this is not easily doable. We are only throwing an error just to ensure
// that the error reported is not crashing nidium.
Tests.registerAsync("Thread (threaded errors)", function(next) {
    var t = new Thread(function() {
        throw new Error("thread error");
    });

    t.start();

    setTimeout(function() {
        next();
    }, 300);

}, 1000);
