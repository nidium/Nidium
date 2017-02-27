/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
VM_TEST_GLOBAL_VAR = true;

const VM = require("VM");

Tests.register("VM.run", function() {
    VM.run("1 + 1");
});

Tests.register("VM.run (capture args from current env)", function() {
    VM.run("Assert(VM_TEST_GLOBAL_VAR, 'VM_TEST_GLOBAL_VAR should be defined')");
});

Tests.register("VM.run (with scope)", function() {
    var scope = {x: 1};
    VM.run("x++", {scope: scope});
    Assert.equal(scope.x, 2);
});

Tests.register("VM.run (with sandbox)", function() {
    var sandbox = {x: 1};

    VM.run("x++", {sandbox: sandbox});

    Assert.equal(sandbox.x, 2);
});

Tests.register("VM.run (sandbox capture unqualifed args)", function() {
    var sandbox = {};

    VM.run("x = 15", {sandbox: sandbox});

    Assert.equal(sandbox.x, 15);
});
