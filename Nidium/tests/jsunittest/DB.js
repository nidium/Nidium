/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var DB = require("DB");

Tests.register("DB", function() {
    var db = new DB("private://tests");
    db.close();
});

Tests.register("DB.close", function() {
    var db = new DB("private://tests");

    db.close();

    // Once the DB is closed, operations on the db should throw an error
    Assert.throws(function() {
        db.set("foo", "bar");
    });

    Assert.throws(function() {
        db.get("foo");
    });

    Assert.throws(function() {
        db.delete("foo");
    });
});

Tests.register("DB.set/get(string)", function() {
    var db = new DB("private://tests");

    db.set("foo", "bar");

    Assert(db.get("foo") == "bar");

    db.close();
});

Tests.register("DB.set/get(object)", function() {
    var db = new DB("private://tests");

    db.set("foo", {"foo":"bar"});

    Assert(db.get("foo").foo == "bar");

    db.close();
});

Tests.register("DB.delete", function() {
    var db = new DB("private://tests");

    db.set("fooDel", "bar");
    Assert(db.get("fooDel") == "bar");

    db.delete("fooDel");
    Assert(db.get("fooDel") == undefined);

    db.close();
});

Tests.register("DB.drop", function() {
    var db = new DB("private://tests");

    db.drop();

    Assert.throws(function() {
        db.get("foo");
    });

    db.close(); // noop
});
