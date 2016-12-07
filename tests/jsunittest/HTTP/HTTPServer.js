/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.register("HTTPServer constructor", function() {
    new HTTPServer("127.0.0.1", 4241);
});

/*Tests.register("HTTPServer reusePort option", function() {
    var h = new HTTPServer(4242, "0.0.0.0", {"reusePort": true});
    Assert.doesNotThrow(function() {
        new HTTPServer(4242, "0.0.0.0", {"reusePort": true})
    });
});
*/
