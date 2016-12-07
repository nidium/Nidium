/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.register("System.getOpenFileStats", function() {
    var stats = System.getOpenFileStats();
    Assert.equal(stats.hasOwnProperty('cur'), true);
    Assert.equal(stats.hasOwnProperty('max'), true);
    Assert.equal(stats.hasOwnProperty('open'), true);
    Assert.equal(stats.hasOwnProperty('sockets'), true);
    Assert.equal(stats.hasOwnProperty('files'), true);
});

