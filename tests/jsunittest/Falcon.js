//{{{ UI.Element
Tests.register("Element", function() {
    Assert.notEqual(UI.Element, undefined, "Element should be defined");
});

Tests.register("Element (have InvalidateReason)", function() {
    Assert.equal(typeof UI.Element.prototype.InvalidateReason, "object", "Element.InvalidateReason should be defined");
});

Tests.register("Element (have events)", function() {
    var el = new UI.Element();

    Assert.equal(typeof el.addEventListener, "function", "Element.addEventListener is not a function");
    Assert.equal(typeof el.fireEvent, "function", "Element.fireEvent is not a function");
});

Tests.register("ElementStyle", function() {
    Assert.notEqual(UI.ElementStyle, undefined, "ElementStyle should be defined");
});

Tests.register("Element.style ", function() {
    var el = new UI.Element();

    Assert.notEqual(el.style, undefined, "Element.style should exist");
});

Tests.register("Element.style (properties are defined)", function() {
    var el = new UI.Element();

	// Property proxied to canvas
    Assert.notEqual(el.style.width, undefined, "Element.style.width property should exist");

	// Builtin property
    //Assert.notEqual(el.style.backgroundColor, undefined, "Element.style.backgroundColor property should exist");
});

Tests.register("Element.style (properties have default)", function() {
    var el = new UI.Element();

    Assert.equal(el.style.width, 1, "Element.style.width should be \"1\"");
});

Tests.register("Element.style (properties are permanent)", function() {
    var el = new UI.Element();
    delete el.style.width;

    Assert.notEqual(el.style.width, undefined, "Element.style.width property should not have been deleted");
});

Tests.register("Element.style (setter/getter)", function() {
    var el = new UI.Element();
    el.style.left = 4;

    Assert.equal(el.style.left, 4, "Element style left should be 4");

    // Extra safety check, 
    // make sure the properties are not shared among objects
    var el2 = new UI.Element();
    Assert.equal(el2.style.left, undefined, "Style value are shared accros instance");
});

Tests.registerAsync("Element.style.left", function(next) {
    var el = new UI.Element();
    el.style.left = 20;
    el.style.width = 100;
    el.style.height = 100;

    Assert.equal(el.style.left, 20, "Element style left should be 20");

    document.add(el);
    
    requestAnimationFrame(function() {
        Assert.equal(el.canvas.left, 20, "Canvas.left should be 20");
        next();
    });

});

// XXX : Should be visual diff !
Tests.registerAsync("Element.style.backgroundColor", function(next) {
    var el = new UI.Element();

    el.style.left = 50;
    el.style.width = 200;
    el.style.height = 200;
    el.style.backgroundColor = "red";

    document.add(el);

    Assert.equal(el.style.backgroundColor, "red", "Element style backgroundColor should be red");
 
    requestAnimationFrame(function() {
        //Assert.equal(el.canvas.left, 20, "Canvas.left should be 20");
        next();
    });

});
// }}}

// {{{ UI.NSS
Tests.register("NSS", function(next) {
    Assert.notEqual(typeof UI.NSS, "undefined", "UI.NSS should be defined");
});

Tests.register("NSS.addProperty", function(next) {
    UI.NSS.addProperty("foo", {
		default: "foo"
	});

	var el = new UI.Element();
	Assert.equal(el.style.foo, "foo", "Element.style.foo should be \"foo\"");
});

Tests.register("NSS.addProperty (re-define throws)", function(next) {
	Assert.throws(function() {
		UI.NSS.addProperty("foo", {
			default: "foo"
		});
	});
});

Tests.register("NSS.addProperty (setter)", function() {
	var setterCalled = false;
    UI.NSS.addProperty("testSetter", {
		onSet: function(key, value, private) {
			Assert.equal(key, "testSetter", "Expected key to be \"testSetter\"");
			Assert.equal(value, "42", "Expected value to be \"42\"");
    		Assert.notEqual(typeof private, "undefined", "private should be defined");
			
			setterCalled = true;
		},
		
	});

	var el = new UI.Element();

	// Bonus : No default, should be undefined
	Assert.equal(typeof el.style.testSetter, "undefined", "Element.style.testSetter should be \"undefined\"");

	el.style.testSetter = 42;

	Assert.equal(el.style.testSetter, 42, "Element.style.testSetter should be \"42\"");
	Assert.equal(setterCalled, true, "Setter has not been called");
});

Tests.registerAsync("NSS.addProperty (redraw/preDraw/postDraw)", function(next) {
	var preDraw = 0;
	var postDraw = 0;
    UI.NSS.addProperty("testRedraw", {
		redraw: true,
		preDraw: function(canvas, context, private) {
			preDraw++;
		},
		postDraw: function(canvas, context, private) {
			postDraw++;
		}
	});

	var el = new UI.Element();

	requestAnimationFrame(function() {
		el.style.testRedraw = "draw_me_a_sheep";
		requestAnimationFrame(function() {
			Assert.equal(preDraw, 2, "preDraw() callback should have been called twice");
			Assert.equal(postDraw, 2, "postDraw() callback should have been called twice");
			next();
		});
	});

	// Element must be added to be drawn a first time
	document.add(el);
});

if (0) {
// XXX : API draft for addProperty
Tests.registerAsync("NSS.addProperty", function(next) {
	var InvalidateReason = UI.Element.InvalidateReason;
    UI.NSS.addProperty("myProperty", {
        default: "bar", // Default value for the property

		redraw: true, 	// Property value update triggers redraw
		reflow: true, 	// Property value update triggers reflow

		onSet: function(key, value, private) {
			// Called when the property is set
			console.log("property set", key, value, private);
		},
		onReflow: function(ev, canvas, context, private) {
			// Called whenever : 
			// - The element is reflowed (size, padding, margin updated)
			// - The element is added / removed from his parent
			// - Direct children are added / removed / reflowed

			/*
			UI.Element.InvalidateReason.ELEMENT_REFLOW
			UI.Element.InvalidateReason.ELEMENT_ADDED
			UI.Element.InvalidateReason.ELEMENT_REMOVED
			UI.Element.InvalidateReason.CHILD_REFLOW
			UI.Element.InvalidateReason.CHILD_ADDED
			UI.Element.InvalidateReason.CHILD_REMOVED
			*/
		},
		preDraw: function(canvas, context, private) {
			// Called before the element is going to be drawn
		},
		postDraw: function(canvas, context, private) {
			// Called after the element is drawn
		}
    });

});

// XXX : API draft for addTheme
Tests.registerAsync("NSS.addTheme", function(next) {
    UI.NSS.addTheme("foo", {
		// Base Colors
		primaryColor: "#0000FF",
		altColor: "#00FF00",
		textColor: "#000000",
		altTextColor: "#FFFFFF",

		// Default to be used for any UI Element
		default: {
			textSize: 30
		},
		
		// Specific style for a class
		".className": {
			backgroundColor: "#FF0000"
		},

		// Specific drawing for UI Element
		"UI.Radio": function() {
		}
    });
});

}

// }}}
