# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

ClassDoc("DB", """Key/Value store backed by LevelDB""", 
    NO_Sees,
    [ExampleDoc("""var DB = require("DB");
var db = new DB("mydb");
db.set("foo", "bar");
console.log(db.get("foo"));""")])

ConstructorDoc("DB", """Open a database
        
> You cannot open more than once the same DB""",
    NO_Sees,
    [ExampleDoc("""var DB = require("DB");
var db = new DB("mydb");
db.set("foo", "bar");
console.log(db.get("foo"));""")])

FunctionDoc("DB.set", "Set the value for a key", 
    NO_Sees,
    [ExampleDoc("""var DB = require("DB");
var db = new DB("mydb");
db.set("foo", "bar");

// You can store complex JS object too
db.set("foo", {"foo": "bar"}); 
console.log(JSON.stringify(db.get("foo")));

db.set("foo", function() { console.log("hello world") });
var fn = db.get("foo");
fn(); // Will print "hello world" """)],
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc("key", "the key to set", "string", IS_Obligated),
        ParamDoc("value", "the value to set", "mixed", IS_Obligated),
    ])

FunctionDoc("DB.get", "Get the value for a key", 
    NO_Sees,
    [ExampleDoc("""var DB = require("DB");
var db = new DB("mydb");
db.set("foo", "bar");
console.log(db.get("foo"));""")],
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc("key", "the key to get", "string", IS_Obligated)
    ],
    ReturnDoc("value", "mixed"))

FunctionDoc("DB.close", """Force the database to be closed

> The `DB` instance will automatically be closed when the DB instance is GCed""", 
    NO_Sees,
    [ExampleDoc("""var DB = require("DB");
var db = new DB("mydb");
db.set("foo", "bar");
db.close();

try {
    db.get("foo");
} catch (e) {
    // Operation on a closed DB throws exception
    console.log(e);
}

// It's okay to re-use the same DB once it has been closed
var otherInstance = new DB("mydb");""")])

FunctionDoc("DB.drop", "Drop the database (delete it)", 
    NO_Sees,
    [ExampleDoc("""var DB = require("DB");
var db = new DB("mydb");
db.drop();

try {
    db.get("foo");
} catch (e) {
    // Operation on a dropped DB throws exception
    console.log(e);
}""")])
