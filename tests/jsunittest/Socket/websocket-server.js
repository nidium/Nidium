Tests.registerAsync("Websocket - Echo local server (plain)", function(next) {
    var server = new WebSocketServer("ws://127.0.0.1:9099/");
    var done = false;

    server.onopen = function(client) {
        client.send("hello !");
    }

    server.onmessage = function(client, ev) {
        Assert.equal(ev.data, "hello !");

        done = true;
        client.close();
    }

    server.onclose = function(client) {
        Assert.equal(done, true);
        next();
    }

    var wsclient = new WebSocket("ws://127.0.0.1:9099/");

    wsclient.onopen = function() {
        
    }

    wsclient.onmessage = function(ev) {
        Assert.equal(ev.data, "hello !");
        wsclient.send(ev.data);
    }

    wsclient.onclose = function() {
    }

}, 3000);


Tests.registerAsync("Websocket - Many clients (plain)", function(next) {
    var server = new WebSocketServer("ws://127.0.0.1:9101/");
    var count = 0;
    var close = 0;
    var total = 100;

    server.onopen = function(client) {
        client.send("hello !");
    }

    server.onmessage = function(client, ev) {
        Assert.equal(ev.data, "hello !");

        count++;
        client.close();
    }

    server.onclose = function(client) {
        
        close++;

        if (count == total && close == total) {
            next();
        }
    }

    for (var i = 0; i < total; i++) {
        let wsclient = new WebSocket("ws://127.0.0.1:9101/");

        wsclient.onopen = function() {
            
        }

        wsclient.onmessage = function(ev) {
            Assert.equal(ev.data, "hello !");

            this.send(ev.data);
        }

        wsclient.onclose = function() {

        }
    };


}, 1000);


/*
Tests.registerAsync("Websocket - Echo local server (Secure)", function(next) {
    var server = new WebSocketServer("wss://127.0.0.1:9102/");
    var done = false;

    server.onopen = function(client) {
        client.send("hello !");
    }

    server.onmessage = function(client, ev) {
        Assert.equal(ev.data, "hello !");

        done = true;
        client.close();
    }

    server.onclose = function(client) {
        Assert.equal(done, true);
        next();
    }

    var wsclient = new WebSocket("wss://127.0.0.1:9102/");

    wsclient.onopen = function() {
        
    }

    wsclient.onmessage = function(ev) {
        Assert.equal(ev.data, "hello !");
        wsclient.send(ev.data);
    }

    wsclient.onclose = function() {
    }

}, 3000);
*/