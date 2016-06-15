/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

Tests.register("process.setOwner (invalid calls)", function() {
    try {
        process.setOwner();
    } catch (e) {
        Assert(("" + e).indexOf("TypeError") == 0);
    }

    try {
        process.setOwner("foobar1337");
    } catch(e) {
        Assert(("" + e).indexOf("Error") == 0);
    }

    var owner = process.getOwner();
    if (owner.uid != 0) {
        try {
            process.setOwner("root");
        } catch(e) {
            Assert(("" + e).indexOf("Operation not permitted") != -1);
        }
    }
});

Tests.register("process.setOwner(uid, gid)/getOwner", function() {
    if (owner.uid == 0) {
        console.log("Setting owner with uid & gid = 1000");
        process.setOwner(1000, 1000);

        Assert.equal(process.getOwner().uid, 1000);
        Assert.equal(process.getOwner().gid, 1000);
    } else {
        console.log("Setting same user (non root caller)");
        var owner = process.getOwner();

        process.setOwner(owner.uid, owner.gid);

        Assert.equal(process.getOwner().uid, owner.uid);
        Assert.equal(process.getOwner().gid, owner.gid);
    }
});

Tests.register("process.setOwner(user, group)/getOwner", function() {
    var owner = process.getOwner();

    process.setOwner(owner.user, owner.group);

    Assert.equal(process.getOwner().user, owner.user);
    Assert.equal(process.getOwner().group, owner.group);
});

Tests.register("process.cwd", function() {
    Assert.equal(global.__dirname.substr(0, global.__dirname.length -8), process.cwd());
});
