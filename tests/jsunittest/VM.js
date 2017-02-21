/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var VM = require("VM");
Tests.register("VM.run", function() {
    VM.run("1 + 1");
});

Tests.register("VM.run (with scope)", function() {
    var scope = {x: 1};
    VM.run("x++", {scope: scope});
    Assert.equal(scope.x, 2);
});

Tests.register("VM.run (with compartement)", function() {
    var obj = {x: 1, console:console};
    var c = new VM.Compartment(obj);

    VM.run("x++", {compartment: c});

    Assert.equal(c.x, 2);
});

Tests.register("VM.runInFunction (with arguments & bind)", function() {
    var bind = {x: 1};
    var args = {y: 1};

    var ret = VM.runInFunction("this.x++; Assert.equal(y, 1); return 'Hello';", {bind: bind, args: args});

    Assert.equal(bind.x, 2, "Bound value x should be 2");
    Assert.equal(ret, "Hello", "Function should have returned 'Hello'");
});
