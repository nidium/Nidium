Tests.registerAsync("Socket connect", function(next) {
    var client = new Socket(TESTS_SERVER_HOST, TESTS_SERVER_PORT).connect();

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


Tests.registerAsync("Socket framing", function(next) {
    var server = new Socket("127.0.0.1", 9028).listen();
    var client = new Socket("127.0.0.1", 9028).connect();

    server.readline = true;
    client.readline = true;

    var done = 0;

    const datainc = "lastframe\n";

    server.onaccept = function(new_client) {
        new_client.write("data");
        setTimeout(function() {
            new_client.write("data2\nframe2\n");

            for (let i = 0; i < datainc.length; i++) {
                setTimeout(function(pos) {
                    new_client.write(datainc[pos]);
                }, 50 * i, i);
            }
        }, 100);
    }

    client.onread = function(data) {
        if (done == 0) {
            Assert.equal(data, "datadata2");

        } else if (done == 1) {
            Assert.equal(data, "frame2");
        } else if (done == 2) {
            Assert.equal(data, "lastframe");
            this.disconnect();
            next();
        }

        done++;
    }

    server.onread = function(new_client, data) {

    }

    client.ondisconnect = function() {

    }

}, 3000);


Tests.registerAsync("Socket lz4", function(next) {
    var server = new Socket("127.0.0.1", 9029).listen("tcp-lz4");
    var client = new Socket("127.0.0.1", 9029).connect("tcp-lz4");

    client.encoding = "utf8";
    server.encoding = "utf8";

    client.readline = true;

    const payload = "On sait depuis longtemps que travailler avec du texte lisible et contenant du sens est source de distractions, et empêche de se concentrer sur la mise en page elle-même. L'avantage du Lorem Ipsum sur un texte générique comme 'Du texte. Du texte. Du texte.' est qu'il possède une distribution de lettres plus ou moins normale, et en tout cas comparable avec celle du français standard. De nombreuses suites logicielles de mise en page ou éditeurs de sites Web ont fait du Lorem Ipsum leur faux texte par défaut, et une recherche pour 'Lorem Ipsum' vous conduira vers de nombreux sites qui n'en sont encore qu'à leur phase de construction. Plusieurs versions sont apparues avec le temps, parfois par accident, souvent intentionnellement (histoire d'y rajouter de petits clins d'oeil, voire des phrases embarassantes). On sait depuis longtemps que travailler avec du texte lisible et contenant du sens est source de distractions, et empêche de se concentrer sur la mise en page elle-même. L'avantage du Lorem Ipsum sur un texte générique comme 'Du texte. Du texte. Du texte.' est qu'il possède une distribution de lettres plus ou moins normale, et en tout cas comparable avec celle du français standard. De nombreuses suites logicielles de mise en page ou éditeurs de sites Web ont fait du Lorem Ipsum leur faux texte par défaut, et une recherche pour 'Lorem Ipsum' vous conduira vers de nombreux sites qui n'en sont encore qu'à leur phase de construction. Plusieurs versions sont apparues avec le temps, parfois par accident, souvent intentionnellement (histoire d'y rajouter de petits clins d'oeil, voire des phrases embarassantes).";

    console.log(payload.length);

    var done = 0;

    server.onaccept = function(new_client) {
        new_client.write(payload)
        new_client.write("\n");
    }

    client.onread = function(data) {
        console.log("Read", data.length);
        Assert.equal(data, payload);

        next();
    }

}, 500);
