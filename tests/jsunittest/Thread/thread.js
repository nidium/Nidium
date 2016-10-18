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

