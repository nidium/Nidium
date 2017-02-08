HTTP.get = function(url, callback) {
    var h = new HTTP(url);

    h.on("error", (err) => {
        callback(err, null);
    });

    h.on("response", (ev) => {
        callback(undefined, ev);
    });

    h.request({
        method: 'GET'
    });
}