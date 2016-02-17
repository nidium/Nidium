Tests.register("Global.btoa", function() {
	Assert.equal(btoa("Hello Nidium"), "SGVsbG8gTmlkaXVt");
	Assert.equal(btoa("hello nidium"), "aGVsbG8gbmlkaXVt");
});

