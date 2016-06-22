/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

//Tests.registerAsync("HTTP.HTTPClient", function(next) {
	var cases = {
	 "/" : {},
	 "/toosmall-content-length": {},
	 "/toolarge-content-length": {}
	}
	for (var path in cases) {
		var details = cases[path];
		setTimeout(function() {
			var h = new Http("http://127.0.0.1:8000/");
			function req() {
				h.request({
					headers: { },
					path: path
				}, function(ev) {
					console.log(path)
					console.log(JSON.stringify(ev));
					req();
				});
			}
			h.onerror = function(err) {
				console.log("Err", JSON.stringify(err));
			};
			req();
		}, 200);
	}
//}, 1000);
