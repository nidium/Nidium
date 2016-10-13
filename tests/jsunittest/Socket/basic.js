Tests.registerAsync("Socket connect", function(next) {
    var client = new Socket("127.0.0.1", 8000).connect();

    client.onconnect = function() {
        Assert.equal(client, this);
        client.disconnect();
        next();
    }
}, 500);

Tests.registerAsync("Socket listen", function(next) {
    var server = new Socket("127.0.0.1", 9027).listen();

    var client = new Socket("127.0.0.1", 9027).connect();
    var done = false;

    server.onaccept = function(new_client) {
        new_client.foo = "bar";
        new_client.write("data");

        Assert.equal(new_client.ip, "127.0.0.1");
    }

    client.onread = function(data) {
        Assert.equal(data, "data");
        this.write("reply");
    }

    server.onread = function(new_client, data) {
        Assert.equal(data, "reply");
        Assert.equal(new_client.foo, "bar");

        new_client.disconnect();
        done = true;
    }

    client.ondisconnect = function() {
        Assert.equal(done, true);
        next();
    }

}, 500);