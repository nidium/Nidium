
Tests.register("Canvas.properties.Canvas.scrollTop.0", function( ) {
	var dummy = 0;
		var canvas = new Canvas(200, 400);
		var ctx = canvas.getContext("2d");
		document.canvas.add(canvas)
		ctx.fillStyle = "red";
		ctx.fillRect(0, 0, 100, 100);
		canvas.top = 100;
		document.canvas.offsetTop = 10;

	Assert.equal(dummy, 0);
});

Tests.register("Canvas.properties.Canvas.position.0", function( ) {
	var dummy = 1;
		var canvas = new Canvas(200, 400);
		var ctx = canvas.getContext("2d");
		document.canvas.add(canvas);
		
		ctx.fillStyle = "red";
		ctx.fillRect(0, 0, 100, 100);
		
		canvas.left = 100; // 100px relative to window.canvas
		canvas.top = 50; // 50px relative to window.canvas
		
		// at this stage, canvas is now positioned at (100,50) relative to its parent (window.canvas)
		
		canvas.position = "absolute";
		
		// at this stage, canvas is now positioned at (100,50) relative to the application window

	Assert.equal(dummy, 1);
});

Tests.register("Canvas.properties.Canvas.top.0", function( ) {
	var dummy = 2;
		var canvas = new Canvas(200, 400);
		var ctx = canvas.getContext("2d");
		document.canvas.add(canvas);
		ctx.fillStyle = "red";
		ctx.fillRect(0, 0, 100, 100);
		canvas.top = 100;

	Assert.equal(dummy, 2);
});

Tests.register("Canvas.properties.Canvas.coating.0", function( ) {
	var dummy = 3;
		var canvas = new Canvas(100, 100);
		canvas.coating = 100;
		canvas.left = 300;
		canvas.top = 300;
		
		var ctx = canvas.getContext("2d");
		ctx.fillStyle = "red";
		ctx.fillRect(0, 0, canvas.width, canvas.height);
		
		ctx.strokeStyle = "blue";
		ctx.strokeRect(-100, -100, canvas.width+200, canvas.height+200);
		
		document.canvas.add(canvas);

	Assert.equal(dummy, 3);
});

Tests.register("Canvas.properties.Canvas.left.0", function( ) {
	var dummy = 4;
		var canvas = new Canvas(200, 400);
		var ctx = canvas.getContext("2d");
		document.canvas.add(canvas);
		ctx.fillStyle = "red";
		ctx.fillRect(0, 0, 100, 100);
		canvas.left = 100;

	Assert.equal(dummy, 4);
});

Tests.register("Canvas.properties.Canvas.height.0", function( ) {
	var dummy = 5;
		var canvas = new Canvas(100, 100);
		canvas.height = 150;

	Assert.equal(dummy, 5);
});

Tests.register("Canvas.properties.Canvas.scrollLeft.0", function( ) {
	var dummy = 6;
		var canvas = new Canvas(200, 400);
		var ctx = canvas.getContext("2d");
		document.canvas.add(canvas)
		ctx.fillStyle = "red";
		ctx.fillRect(0, 0, 100, 100);
		canvas.left = 100;
		document.canvas.offsetLeft = 10;

	Assert.equal(dummy, 6);
});

Tests.register("Canvas.properties.Canvas.clientHeight.0", function( ) {
	var dummy = 7;
		var canvas = new Canvas(100, 100); // create a 100x100 canvas
		canvas.coating = 30; // add a coating (padding) of 30 outside the canvas
		console.log(canvas.height) // ---> 100
		console.log(canvas.clientHeight) // ---> 160 (30 coating top + 100 height + 30 coating bottom = 160)

	Assert.equal(dummy, 7);
});

Tests.register("Canvas.properties.Canvas.clientWidth.0", function( ) {
	var dummy = 8;
		var canvas = new Canvas(100, 100); // create a 100x100 canvas
		canvas.coating = 30; // add a coating (padding) of 30 outside the canvas
		console.log(canvas.width) // ---> 100
		console.log(canvas.clientWidth) // ---> 160 (30 coating left + 100 width + 30 coating right = 160)

	Assert.equal(dummy, 8);
});

Tests.register("Canvas.properties.Canvas.overflow.0", function( ) {
	var dummy = 9;
		var canvas = new Canvas(100, 100);
		document.canvas.add(canvas);
		var canvas2 = new Canvas(50, 50);
		canvas.add(canvas2);
		canvas2.left = 50;
		canvas2.top = -10;
		canvas.overflow = false;

	Assert.equal(dummy, 9);
});

Tests.register("Canvas.properties.Canvas.width.0", function( ) {
	var dummy = 10;
		var canvas = new Canvas(100, 100);
		canvas.width = 150;

	Assert.equal(dummy, 10);
});

Tests.register("Canvas.constructors.Canvas.0", function( ) {
	var dummy = 11;
		// Create a new 200x100 canvas (logical pixels)
		var myCanvas = new Canvas(200, 100);
		// at this stage, myCanvas is an orphaned canvas
		// now we add myCanvas to the root canvas (with the add() method)
		document.canvas.add(myCanvas);
		// at this stage, myCanvas is "rooted" and document.canvas becomes its parent.

	Assert.equal(dummy, 11);
});

Tests.register("Canvas.methods.Canvas.getContext.0", function( ) {
	var dummy = 12;
		var canvas = new Canvas(200, 100);
			var context = canvas.getContext("2d");
		context.fillStyle = "red";
		context.fillRect(0, 0, 200, 100);

	Assert.equal(dummy, 12);
});

Tests.register("Canvas.methods.Canvas.getPrevSibling.0", function( ) {
	var dummy = 13;
		var canvas1 = new Canvas(200, 100);
			var canvas2 = new Canvas(200, 100);
			var canvas3 = new Canvas(200, 100);
			document.canvas.add(canvas1);
		document.canvas.add(canvas2);
		document.canvas.add(canvas3);
		var prev = canvas2.getPrevSibling(); // returns canvas1
		var nothing = canvas1.getPrevSibling(); // returns null

	Assert.equal(dummy, 13);
});

Tests.register("Canvas.methods.Canvas.removeFromParent.0", function( ) {
	var dummy = 14;
		var canvas = new Canvas(200, 100);
		canvas.removeFromParent();

	Assert.equal(dummy, 14);
});

Tests.register("Canvas.methods.Canvas.bringToFront.0", function( ) {
	var dummy = 15;
		var canvas = new Canvas(200, 100);
		document.canvas.add(canvas);
		var canvas2 = new Canvas(10, 10);
		document.canvas.add(canvas2);
		canvas.bringToFront();

	Assert.equal(dummy, 15);
});

Tests.register("Canvas.methods.Canvas.clear.0", function( ) {
	var dummy = 16;
		var canvas = new Canvas(200, 400);
		var ctx = canvas.getContext("2d");
		ctx.fillStyle = "red";
		ctx.fillRect(10, 10, 100,100);
		canvas.clear();

	Assert.equal(dummy, 16);
});

Tests.register("Canvas.methods.Canvas.setCoordinates.0", function( ) {
	var dummy = 17;
		var canvas = new Canvas(200, 400);
		canvas.position = 'absolute';
		canvas.setCoordinates(30, 50);
		//alternative way
		canvas.left = 30;
		canvas.top = 50;

	Assert.equal(dummy, 17);
});

Tests.register("Canvas.methods.Canvas.add.0", function( ) {
	var dummy = 18;
		var canvas = new Canvas(200, 100);
		document.canvas.add(canvas);
		
		var canvas2 = new Canvas(10, 10);
		canvas.add(canvas2);

	Assert.equal(dummy, 18);
});

Tests.register("Canvas.methods.Canvas.setZoom.0", function( ) {
	var dummy = 19;
		var canvas = new Canvas(200, 400);
		canvas.setZoom(2);

	Assert.equal(dummy, 19);
});

Tests.register("Canvas.methods.Canvas.setSize.0", function( ) {
	var dummy = 20;
		var canvas = new Canvas(200, 400);
		var ctx = canvas.getContext("2d");
		canvas.setSize(300, 300);

	Assert.equal(dummy, 20);
});

Tests.register("Canvas.methods.Canvas.getChildren.0", function( ) {
	var dummy = 21;
		var canvas1 = new Canvas(200, 400);
		var canvas2 = new Canvas(200, 400);
		var canvas3 = new Canvas(200, 400);
		document.canvas.add(canvas1);
		document.canvas.add(canvas2);
		document.canvas.add(canvas3);
		var list = document.canvas.getChildren(); // returns [canvas1, canvas2, canvas3]
		canvas2.bringToFront(); // brings canvas2 to the front of the display list.
		var list = document.canvas.getChildren(); // returns [canvas1, canvas3, canvas2]

	Assert.equal(dummy, 21);
});

Tests.register("Canvas.methods.Canvas.getLastChild.0", function( ) {
	var dummy = 22;
		var canvas = new Canvas(200, 100);
		var child = canvas.getLastChild();

	Assert.equal(dummy, 22);
});

Tests.register("Canvas.methods.Canvas.getFirstChild.0", function( ) {
	var dummy = 23;
		var canvas = new Canvas(200, 100);
		var child = canvas.getFirstChild();

	Assert.equal(dummy, 23);
});

Tests.register("Canvas.methods.Canvas.getParent.0", function( ) {
	var dummy = 24;
		var canvas = new Canvas(200, 100);
			var parent = canvas.getParent();

	Assert.equal(dummy, 24);
});

Tests.register("Canvas.methods.Canvas.sendToBack.0", function( ) {
	var dummy = 25;
		var canvas = new Canvas(200, 100);
		document.canvas.add(canvas);
		
		var canvas2 = new Canvas(10, 10);
		document.canvas.add(canvas2);
		
		canvas2.sendToBack();

	Assert.equal(dummy, 25);
});

Tests.register("Canvas.methods.Canvas.getNextSibling.0", function( ) {
	var dummy = 26;
		var canvas1 = new Canvas(200, 100);
		var canvas2 = new Canvas(200, 100);
		var canvas3 = new Canvas(200, 100);
		document.canvas.add(canvas1);
		document.canvas.add(canvas2);
		document.canvas.add(canvas3);
		var next = canvas2.getNextSibling(); // returns canvas3
		var nothing = canvas3.getNextSibling(); // returns null

	Assert.equal(dummy, 26);
});

Tests.register("Canvas.base.Canvas.0", function( ) {
	var dummy = 27;
		// Create a new 200x100 canvas (logical pixels)
		var canvas = new Canvas(200, 100);
		// Add it to the root hierarchy
		document.canvas.add(canvas);
		// Get its 2D context
		var context = canvas.getContext("2d");

	Assert.equal(dummy, 27);
});

Tests.register("NativeDocument.methods.NativeDocument.showFPS.0", function( ) {
	var dummy = 28;
		var canvas = new Canvas(200, 100);
		var ctx = canvas.getContext("2d");
		document.showFPS(true);

	Assert.equal(dummy, 28);
});

Tests.register("NativeDocument.methods.NativeDocument.loadFont.0", function( ) {
	var dummy = 29;
		document.loadFont({
		    file: "private://assets/fonts/onesize.ttf",
		    name: "OneSize"
		});

	Assert.equal(dummy, 29);
});

Tests.register("NativeDocument.methods.NativeDocument.loadFont.1", function( ) {
	var dummy = 30;
		document.loadFont({
		    file : "private://modules/fontawesome/fontawesome.ttf",
		    name : "fontAwesome"
		});

	Assert.equal(dummy, 30);
});

Tests.register("Window.properties.Window.__nidium__.build.0", function( ) {
	var dummy = 31;
		console.log('build is now: '  + window.__nidium__.build);

	Assert.equal(dummy, 31);
});

Tests.register("Window.properties.Window.__nidium__.version.0", function( ) {
	var dummy = 32;
		console.log('version is now: '  + window.__nidium__.version);

	Assert.equal(dummy, 32);
});

Tests.register("Window.properties.Window.navigator.0", function( ) {
	var dummy = 33;
		console.log(JSON.stringify(window.navigator));

	Assert.equal(dummy, 33);
});

Tests.register("Window.properties.Window.__nidium__.0", function( ) {
	var dummy = 34;
		console.log(JSON.stringify(window.navigator));

	Assert.equal(dummy, 34);
});

Tests.register("Window.properties.Window.__nidium__.revision.0", function( ) {
	var dummy = 35;
		console.log('revision is now: '  + window.__nidium__.revision);

	Assert.equal(dummy, 35);
});

Tests.register("Image.base.Image.0", function( ) {
	var dummy = 36;
		document.background = "#333333";
		document.opacity = 0;
		
		var pic = new UI.element(document, {
		    visible : false,
		    width : 1,
		    height : 1,
		    radius : 6,
		    shadowBlur : 6,
		    shadowColor : "black"
		});
		
		var img = new Image();
		
		img.onload = function(){
		    pic.setBackgroundImage(img);
		    pic.width = img.width;
		    pic.height = img.height;
		    pic.visible = true;
		    pic.center();
		    document.fadeIn(850);
		};
		
		img.src = "http://www.nidium.com/static/img/island.png";

	Assert.equal(dummy, 36);
});

Tests.register("Navigator.properties.Navigator.appVersion.0", function( ) {
	var dummy = 37;
		console.log('appVersion is now: ' +  window.navigator.appVersion);

	Assert.equal(dummy, 37);
});

Tests.register("Navigator.properties.Navigator.vibrate.0", function( ) {
	var dummy = 38;
		console.log('vibrate is now: ' +  window.navigator.vibrate);

	Assert.equal(dummy, 38);
});

Tests.register("Navigator.properties.Navigator.userAgent.0", function( ) {
	var dummy = 39;
		console.log('userAgent is now: ' +  window.navigator.userAgent);

	Assert.equal(dummy, 39);
});

Tests.register("Navigator.properties.Navigator.platform.0", function( ) {
	var dummy = 40;
		console.log('platform is now: ' +  window.navigator.platform);

	Assert.equal(dummy, 40);
});

Tests.register("Navigator.properties.Navigator.language.0", function( ) {
	var dummy = 41;
		console.log('language is now: ' +  window.navigator.language);

	Assert.equal(dummy, 41);
});

Tests.register("Navigator.properties.Navigator.appName.0", function( ) {
	var dummy = 42;
		console.log('appName is now: ' +  window.navigator.appName);

	Assert.equal(dummy, 42);
});

Tests.register("Navigator.base.Navigator.0", function( ) {
	var dummy = 43;
		console.log(JSON.stringify(window.navigator));

	Assert.equal(dummy, 43);
});

