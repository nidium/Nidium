var vm = require("vm");

Tests.register("vm.runInScope", function() {
    var obj = {foo: "bar"};

    vm.runInScope(`
        if (1 == 1) {
            console.log("1 is 1");
        }

        if (foo == "bar") {
            var newprop = 42;
        }
    `, obj);

    Assert.equal(obj.newprop, 42);
});
