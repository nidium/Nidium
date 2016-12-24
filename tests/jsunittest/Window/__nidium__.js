/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.register("Window.__nidium__", function() {
    var nidium = window.__nidium__;
    console.log(nidium.build.length);

    Assert.equal(typeof nidium, "object", "Window.__nidium__ should be a object");
    Assert.equal(typeof nidium.version, "string", "__nidium__.version should  be \"string\"");
    Assert.notEqual(nidium.version.indexOf('.'), -1, "__nidium__.version should  have at least one '.'");
    Assert.equal(typeof nidium.build, "string", "__nidium__.build should  be \"string\"");
    Assert.equal(nidium.build.length, 40, "__nidium__.build should  be 40 characters long.");
    Assert.equal(typeof nidium.revision, "string", "__nidium__.revision should  be \"string\"");
    Assert.equal(nidium.revision.length, 40, "__nidium__.revision should  be 40 characters long.");
});


