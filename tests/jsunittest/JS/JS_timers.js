/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

//Timeout
/*FIXME:
Tests.register("Global.setTimeout string 100", function() {
    var a = 0;
    var t = setTimeout('function() {
        Assert.equal(a, 1);
    }');
    Assert.notEqual(t, 0);
    a++;
});
*/

Tests.register("Global.setTimeout FN 100", function() {
    var a = 0;
    var t = setTimeout(function() {
        Assert.equal(a, 1);
    });
    Assert.notEqual(t, 0);
    a++;
});

Tests.register("Global.setTimeout FN 110", function() {
    var a = 0;
    var t = setTimeout(function() {
        Assert.equal(a, 1);
    }, 500);
    Assert.notEqual(t, 0);
    a++;
});

Tests.register("Global.setTimeout FN 111", function() {
    var a = 0;
    var t = setTimeout(function(arg) {
        Assert.equal(a, arg);
    }, 500, 1);
    Assert.notEqual(t, 0);
    a++;
});



//Interval
/*FIXME:
Tests.register("Global.setInterval string 100", function() {
    var a = 0;
    var t = setTimeout('function() {
        Assert.equal(a, 1);
    }');
    Assert.notEqual(t, 0);
    a++;
});
*/

Tests.register("Global.setInterval FN 100", function() {
    var a = 0;
    var t = setInterval(function() {
        Assert.equal(a, 1);
        clearTimeout(t);
    });
    Assert.notEqual(t, 0);
    a++;
});

Tests.register("Global.setInterval FN 110", function() {
    var a = 0;
    var b = 0;
    var t = setInterval(function() {
        Assert.equal(a, 1);
        b++
        if (b > 5 ) {
            clearTimeout(t);
        }
    }, 50);
    Assert.notEqual(t, 0);
    a++;
});

Tests.register("Global.setInterval FN 111", function() {
    var a = 0;
    var b = 0
    var t = setInterval(function(arg) {
        Assert.equal(a, arg);
        b++;
        if (b > 5 ) {
            clearTimeout(t);
        }
    }, 50, 1);
    Assert.notEqual(t, 0);
    a++;
});

//clear
Tests.register("Global.ClearTimeout FN 111", function() {
    var a = 0;
    var t = setTimeout(function(arg) {
        a = args;
    }, 600, 666);
    Assert.notEqual(t, 0);
    clearTimeout(t);
    Assert.equal(a, 0);
});

Tests.register("Global.ClearInterval FN 111", function() {
    var a = 0;
    var t = setInterval(function(arg) {
        a = args;
    }, 600, 666);
    Assert.notEqual(t, 0);
    clearTimeout(t);
    Assert.equal(a, 0);
});


