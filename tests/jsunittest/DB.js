var DB = require("DB");

Tests.register("DB", function() {
    var db = new DB("cache://tests");
    db.close();
});

Tests.register("DB.close", function() {
    var db = new DB("cache://tests");

    db.close();

    // Once the DB is closed, operations on the db should throw an error
    Assert.throws(function() {
        db.set("foo", "bar");
    });

    Assert.throws(function() {
        db.get("foo");
    });

    Assert.throws(function() {
        db.del("foo");
    });
});

Tests.register("DB.set/get(string)", function() {
    var db = new DB("cache://tests");

    db.set("foo", "bar");

    Assert(db.get("foo") == "bar");

    db.close();
});

Tests.register("DB.set/get(object)", function() {
    var db = new DB("cache://tests");

    db.set("foo", {"foo":"bar"});

    Assert(db.get("foo").foo == "bar");

    db.close();
});

Tests.register("DB.del", function() {
    var db = new DB("cache://tests");

    db.set("fooDel", "bar");
    Assert(db.get("fooDel") == "bar");

    db.del("fooDel");
    Assert(db.get("fooDel") == undefined);

    db.close();
});

Tests.register("DB.drop", function() {
    var db = new DB("cache://tests");

    db.drop();

    Assert.throws(function() {
        db.get("foo");
    });

    db.close(); // noop
});
