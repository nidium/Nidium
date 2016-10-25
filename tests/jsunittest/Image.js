Tests.registerAsync("Image.src", function(next) {
    var img = new Image();

    img.addEventListener("load", function() {
        // XXX : Should be tested in another test ?
        Assert(this.width == 128);
        Assert(this.height == 128);

        next();
    });

    img.addEventListener("error", function(ev) {
        throw new Error("Error happened during image loading : " + ev.error);
    });

    img.src = "http://tests.nidium.com/img/nidium.png";
}, 2000);

Tests.registerAsync("Image.src (onload property)", function(next) {
    var img = new Image();

    img.onload = function() {
        next();
    };

    img.onerror = function(ev) {
        throw new Error("Error happened during image loading : " + ev.error);
    };

    img.src = "http://tests.nidium.com/img/nidium.png";
}, 2000);

Tests.registerAsync("Image.src (non existent file)", function(next) {
    var img = new Image();

    img.addEventListener("load", function() {
        throw new Error("Got load event but image loaded is invalid");
    });

    img.addEventListener("error", function(ev) {
        next();
    });

    img.src = "http://nidium.com/this_file_does_not_exists.png";
}, 2000);

Tests.registerAsync("Image.src (invalid file)", function(next) {
    var img = new Image();

    img.addEventListener("load", function() {
        throw new Error("Got load event but image loaded is invalid");
    });

    img.addEventListener("error", function(ev) {
        next();
    });

    img.src = "http://nidium.com/";
}, 2000);
