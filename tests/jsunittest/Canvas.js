Tests.registerVisual("Canvas.fillRect", function(shot) {
    var c = new Canvas(200, 200);
    var ctx = c.getContext("2d");

    document.canvas.add(c);

    ctx.fillStyle = "blue";
    ctx.fillRect(0, 0, 200, 200);

    shot();
});

Tests.registerVisual("Canvas.left", function(shot) {
    var c = new Canvas(200, 200);
    var ctx = c.getContext("2d");

    document.canvas.add(c);

    ctx.fillStyle = "blue";
    ctx.fillRect(0, 0, 200, 200);
	
	c.left = 20;

	Assert.equal(c.left, 20, "Unexpected left value for canvas");

    shot();
});

